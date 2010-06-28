/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"

#include <stdio.h>

#include <vector>
#include <set>

#include <algorithm>
#include "MC2SimpleString.h"

#include <stdlib.h>

#include <math.h>

#include "ClipUtil.h"

#include "TileMapHandler.h"
#include "TileMapParams.h"
#include "TileMapFormatDesc.h"
#include "TileMapUtil.h"
#include "TileArgNames.h"
#include "TileMap.h"
#include "DBufRequester.h"
#include "MemoryDBufRequester.h"
#include "MC2BoundingBox.h"
#include "MapPlotter.h"
#include "TileMapContainer.h"
#include "TileMapLayerInfo.h"

#include "GfxConstants.h"

#include "MC2Point.h"

#include "GunzipUtil.h"
#include "RouteID.h"
#include "TileMapTextHandler.h"

#include "RandomFuncs.h"
#include "GfxUtility.h"

// For the user defined features.
#include "UserDefinedFeature.h"
#include "UserDefinedBitMapFeature.h"
#include "UserDefinedScaleFeature.h"
#include "DirectedPolygon.h"

// For events
#include "TileMapEvent.h"
#include "TileMapEventListener.h"

#include "InsideUtil.h"
#include "TileMapTextSettings.h"

#include "TileMapParamTypes.h"

#if defined __SYMBIAN32__ && !defined SYMBIAN_9
#include <hal.h>
#endif

#include "TMKTimingInfo.h"
#include "TileMapInfoCallback.h"

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

using namespace std;
using namespace isab;


static inline
MC2SimpleString
createServerString( const char* name,
                    const MC2Coordinate& coord,
                    pair<const TileMap*,
                    const TilePrimitiveFeature*>& mapAndFeature );


class TileMapHandlerReleaseChecker : public DBufReleaseChecker {
public:
   TileMapHandlerReleaseChecker( TileMapHandler* handler ) {
      m_handler = handler;
   }

   bool releaseToNextAllowed( const MC2SimpleString& descr,
                              const DBufRequester& req ) {
      bool res = true;
      if ( req.getType() == DBufRequester::MEMORY ) {
         if ( TileMapParamTypes::getParamType( descr ) ==
              TileMapParamTypes::TILE ) {
            if ( m_handler->m_mapFormatDesc != NULL &&
                 !m_handler->m_mapFormatDesc->allowedOnDisk( descr ) ) {
               res = false;
            }
         }
      }
      mc2dbg << "[TMHRC]: Returning "
             << res
             << " for desc "
             << descr << endl;
      return res;
   }
   
private:
   /** 
    *    The TileMapHandler to call when something happens.
    */
   TileMapHandler* m_handler;   
};

/**
 *   Class that it used as DBufRequestListener for easy change.
 */
class TileMapHandlerDefaultDBListener : public DBufRequestListener {
public:

   /**
    *    Constructor.
    */
   TileMapHandlerDefaultDBListener(TileMapHandler* handler) :
         m_handler(handler) {}
   
   /**
    *    Called when a BitBuffer is received from a
    *    requester.
    */
   void requestReceived(const MC2SimpleString& descr,
                        BitBuffer* dataBuffer,
                        const DBufRequester& /*origin*/ ) {
      m_handler->dataBufferReceived(descr, dataBuffer);
   }
private:
   /** 
    *    The TileMapHandler to call when something happens.
    */
   TileMapHandler* m_handler;
};

/**
 *   Class that handles stuff like checking that maps aren't
 *   requested twice in a row, timeouts etc.
 */
class TileMapHandlerDBBufRequester : public DBufRequester, DBufRequestListener{
public:
   /**
    *   Constructor.
    *   @param realRequester The requester to request the maps from.
    *   @param toolkit Toolkit to get the timer from.
    */
   TileMapHandlerDBBufRequester(TileMapHandler* handler,
                                DBufRequester* realRequester,
                                TileMapToolkit* toolkit,
                                DBufRequestListener* defListener);

   /**
    *   Destructor.
    */
   virtual ~TileMapHandlerDBBufRequester() {};
   
   /**
    *   Called by the Requester when a request is done.
    *   @param descr          The descr used to get the object.
    *   @param receivedObject The received object.
    */
   inline void requestReceived(const MC2SimpleString& descr,
                               BitBuffer* dataBuffer,
                               const DBufRequester& origin );

   /**
    *   The Requester must keep the 
    *   release is called.
    *   @param descr  The description.
    *   @param caller The RequestListener that wants an update
    *                 if a delayed answer comes back.
    *   @param onlyCached True if only cached maps should be requested.
    */ 
   inline void request(const MC2SimpleString& descr,
                       DBufRequestListener* caller,
                       request_t whereFrom );
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer.
    *   Default implementation deletes the BitBuffer and removes
    *   it from the set.
    */ 
   inline void release(const MC2SimpleString& descr,
                       BitBuffer* obj);

   /**
    *   Calls cancelAll in the parent.
    */
   inline void cancelAll();
   
private:

   /**
    *   Data for one request.
    */
   class ReqData {
   public:
      /// Should only be used internally in map.      
      ReqData() {}
      ReqData(uint32 timep, DBufRequestListener* reqListener,
              DBufRequester::request_t whereFrom )
            : m_time(timep), m_reqListener(reqListener),
              m_whereFrom( whereFrom ) 
         {
            m_block  = true;
            m_failedTime = 0;
         }
      /// Time of request
      uint32 m_time;
      /// Listener of request.
      DBufRequestListener* m_reqListener;
      /// Where the request may be requested from.
      DBufRequester::request_t m_whereFrom;
      /// True if there should be no more requests for the buffer
      bool m_block;
      /// Time for last failuer.
      uint32 m_failedTime;
   };
   
   /// The toolkit
   TileMapToolkit* m_toolkit;
   /// Type of map to store data in
   typedef map<MC2SimpleString, ReqData> storage_t;
   /// Map containing descriptions and last request time in ms.
   storage_t m_requested;

   /**
    *   Minimum time to wait between re-requests in ms.
    */
   uint32 m_reRequestTimeSec;

   /**
    *   The absolute minimum time to wait before re-requesting.
    */
   uint32 m_minReRequestTimeSec;
   
   /**   
    *   The absolute minimum time to wait before re-requesting.
    */
   uint32 m_maxReRequestTimeSec;

   /**
    *   Time of first failed request.
    */
   uint32 m_firstFailedTimeSec;

   /**
    *   Default listener to send something to when strange stuff happens.
    */
   DBufRequestListener* m_defaultListener;

   /// TileMapHandler is needed to get the mapformatdesc
   TileMapHandler* m_handler;
};

TileMapHandlerDBBufRequester::
TileMapHandlerDBBufRequester(TileMapHandler* handler,
                             DBufRequester* realRequester,
                             TileMapToolkit* toolkit,
                             DBufRequestListener* defaultListener)
      : DBufRequester(realRequester)
{
   m_defaultListener = defaultListener;
   m_toolkit   = toolkit;
   m_reRequestTimeSec    = 5;
   m_minReRequestTimeSec = 5;
   m_maxReRequestTimeSec = 60;
   m_firstFailedTimeSec  = 0; // No failures yet
   m_handler             = handler;
}


inline void
TileMapHandlerDBBufRequester::request(const MC2SimpleString& descr,
                                      DBufRequestListener* caller,
                                      request_t whereFrom )
{
   MC2SimpleString cmp("G+Cho+U1Y");

   if(descr == cmp) {
      int ap = 22;
      ap++;
   }
   
   uint32 curTimeSec = TileMapUtil::currentTimeMillis() / 1000;
   mc2dbg8 << "[THM]: m_firstFailedTimeSec = "
          << m_firstFailedTimeSec << endl;
   mc2dbg8 << "[TMH]: m_requested.size() = " << m_requested.size()
          << endl;
   if ( whereFrom != onlyCache && m_requested.size() >= 30 ) {
      if ( m_firstFailedTimeSec == 0 ) {
         return;
      } else if ( (curTimeSec - m_firstFailedTimeSec) > m_reRequestTimeSec ) {
         // Don't return
      } else {
         return;
      }
   }
   storage_t::iterator it = m_requested.find(descr);
   // Only request maps if not waiting for it.
   // Later we will add timeout that removes old stuff.
   bool update = false;
   
   if ( it == m_requested.end() ) {      
      update = true;
   } else if ( it->second.m_block == false ) {
      update = true;
   } else if ( it->second.m_whereFrom != whereFrom ) {
      mc2dbg << "[TMHBBR]: whereFrom changed." << endl;
      update = true;
   } else if ( (curTimeSec - it->second.m_time) > m_reRequestTimeSec ) {
      mc2dbg << "[TMHBBR]: Ok to request map again - timediff "
             << (curTimeSec - it->second.m_time) << " s" << endl;
      // Waited too long for the map.      
      update = true;
      // Wait one more second for next one.
      m_reRequestTimeSec = MIN(m_reRequestTimeSec, m_maxReRequestTimeSec);
      m_firstFailedTimeSec = 0;
      for ( storage_t::iterator it = m_requested.begin();
            it != m_requested.end();
            ++it ) {
         m_firstFailedTimeSec = MAX(m_firstFailedTimeSec,
                                    it->second.m_failedTime);
      }
   }
   if ( update ) {
      mc2log << info << "[TMH]: Requested " << descr << endl;

      m_requested[descr] = ReqData(
         curTimeSec,
         caller, 
         whereFrom );
      m_parentRequester->request(descr, this, whereFrom );
   } else {
      // TODO: Make a requester that fails after x seconds.
      // m_timerRequester->request(descr, this);
   }
}

inline void
TileMapHandlerDBBufRequester::requestReceived(const MC2SimpleString& descr,
                                              BitBuffer* dataBuffer,
                                              const DBufRequester& origin )
{
   if ( origin.getType() & DBufRequester::EXTERNAL ) {
      // Now we must fixup the traffic maps since they are from the
      // in ter net.
      if ( m_handler->m_mapFormatDesc ) {
         dataBuffer =
            TileMap::writeReceiveTime( *m_handler->m_mapFormatDesc,
                                       descr,
                                       dataBuffer );
      }
   } else if(origin.getType() & DBufRequester::PERMANENT) {
      if(dataBuffer == NULL) {

         //m_handler->m_tileMapCont->addNullMap(descr);   
//         return;
      }
   }
   
   storage_t::iterator it = m_requested.find(descr);
   if ( it != m_requested.end() ) {
      mc2dbg << "[TMH]: Received " << descr << endl;
      DBufRequestListener* listener = it->second.m_reqListener;
      if ( dataBuffer == NULL ) {
         // Failed - release the buffer.
         // But first wait some time.
         it->second.m_failedTime = TileMapUtil::currentTimeMillis() / 1000;
         if ( m_firstFailedTimeSec == 0 ) {
            m_firstFailedTimeSec = it->second.m_failedTime;               
         }
      } else {
         // Decrease the waiting time
         m_reRequestTimeSec = MAX(m_reRequestTimeSec -1 ,
                                  m_minReRequestTimeSec);         
         m_requested.erase(it);
      }
      listener->requestReceived(descr, dataBuffer, origin);
   } else {
      // Wake up the TileMapHandler.
      // Special for the description format
      if ( descr[0] == 'D' ) {
         m_defaultListener->requestReceived( descr, dataBuffer, origin );
      } else {
         // Do not bother the TileMapHandler with the buffer.
         m_defaultListener->requestReceived( descr, NULL, origin );
         release(descr, dataBuffer);
      }
      mc2log << warn << "[TMH]: req recv: Could not find req data"
             << " for " << descr << endl;
   }
}

inline void
TileMapHandlerDBBufRequester::release(const MC2SimpleString& descr,
                                      BitBuffer* dataBuffer)
{
   if ( dataBuffer == NULL ) {
      return;
   }
   m_requested.erase(descr);
   
   m_parentRequester->release(descr, dataBuffer);

}

inline void
TileMapHandlerDBBufRequester::cancelAll()
{
   // Allow all requests to be resent.
   m_requested.clear();
   m_parentRequester->cancelAll();
}

// --------------------************ TileMapHandler *****************------

class LayerSorter {
public:
   inline bool operator()(const int a, const int b) const {
      if ( a == TileMapTypes::c_routeLayer ) {
         return true;
      } else if ( b == TileMapTypes::c_routeLayer ) {
         return false;
      } else {
         return a < b;
      }
   }
};

typedef TileMapContainer::MIt MIt;
typedef TileMapContainer::PIt PIt;

TileMapHandler::TileMapHandler(MapPlotter* plotter,
                               DBufRequester* requester,
                               TileMapToolkit* platform,
                               TileMapEventListener* eventListener )
      : m_plotter(plotter),
        m_toolkit(platform),
        m_mapFormatDesc(NULL),
        // We are not painting.
        m_painting(0),
        m_garbage( platform ),
        m_tileMapCont( new TileMapContainer( this, &m_garbage ) ),
        m_highlightPoint(-1, -1),
        m_screenCoords( m_realScreenCoords, 0, 0 ),
        m_copyrightPos( 0, 0 ),
        m_showCopyright( false )
{
   m_isDrawing = true;
   
   //setShouldCheckLevels( false );
   setDetectMovementBySelf( false );
   m_clickInfo = new TileMapHandlerClickResult();
   m_releaseChecker = new TileMapHandlerReleaseChecker( this );
   m_waitingForInfoString.second = NULL;
   m_outlinesDrawnLastTime = false;
   m_mapArrivedSinceRepaint = true;
   m_descCRCReceived = false;
   m_serverDescCRC   = MAX_UINT32;
   m_tmkTimingInfo = NULL;
   m_rerequestTimer = 0;
   m_dxxBuffer = NULL;
   m_realScreenCoords.reserve( 128 );
  
   m_showHighlight = false;
   if ( eventListener ) {
      m_eventListeners.push_back( eventListener );
   }
   // Create polygon to show when an item can be selected
   m_outlinePolygon = new DirectedPolygon( vector<MC2Point>(),
                                           MC2Point(-10,-10),
                                           false,
                                           0,
                                           3 );
   m_outlinePolygon->setClickable( false );
   m_outlinePolygon->setAngle( 0, false );
   m_repaintAfterMapReceptionTimer = 0;
   srand(TileMapUtil::currentTimeMillis());
   // Create random characters for the description.
   {
      char* randString = RandomFuncs::newRandString(3);
      m_randChars = randString;
      delete [] randString;
   }

   m_userDefinedFeatures = NULL;
   
   m_textHandler = new TileMapTextHandler(m_plotter, 
                                          *this, // MapProjection
                                          (*m_tileMapCont).endMapsToDraw(),
                                          *this ); // TileMapHandler
   m_textHandlerIdleID = 0;

   
   // Default layers are always added (now they will come in the DXXX,
   // but it is possible that there is an old DXXX.
   m_layersToDisplay.insert( TileMapTypes::c_mapLayer );
   m_layersToDisplay.insert( TileMapTypes::c_routeLayer );
   m_layersToDisplay.insert( 2 );
//     m_layersToDisplay.insert( 3 );

   m_routeID = NULL;

   // TESTA!
#if 0
  m_routeID = new RouteID("12_40617C3F");
  //m_routeID = new RouteID("88_4064371D");
#endif

   m_repaintTimerRequested = 0;
   m_lastRepaintTime = TileMapUtil::currentTimeMillis();
   // Create requester with 256k of memory.
   
   m_dataBufferListener = new TileMapHandlerDefaultDBListener(this);
   m_requester = new TileMapHandlerDBBufRequester(this,
                                                  requester, platform,
                                                  m_dataBufferListener);

   setLanguage( LangTypes::english );

   // Set the release checker.
   m_requester->setReleaseChecker( m_releaseChecker );
   
   m_detailLevel = MAX_INT32;
   m_matrixChangedSinceDraw = true;
   m_detailRepaintTimerRequested = 0;
   m_detailRepaintTimerHasBeenRun = true;


   // Use gunzip if available.
   m_useGzip = GunzipUtil::implemented();

   // Get screensize
   m_plotter->getMapSizePixels(m_lastScreenSize);
   setScreenSize( MC2Point(m_lastScreenSize.getHeight(),
                           m_lastScreenSize.getWidth() ) );

   // Set start bbox and scale
   // This is somewhere around spolegatan
   // MC2Coordinate lower( 664609150, 157263143 );
   // MC2Coordinate upper( 664689150, 157405144 );
   // Strange street here.
   //   MC2Coordinate lower( 664380226,156217749 );
	//   MC2Coordinate upper( 664357053,156258858 );
   //   Malm�   
   MC2Coordinate lower( 663428099, 154983536 );
   MC2Coordinate upper( 663394172, 155043599 );

   // Vischan
//   MC2Coordinate lower( 663521299, 157193433 );
   // MC2Coordinate upper( 663498395, 157193577 );

   
   //Lund
	//   MC2Coordinate lower( 663428099, 154983536 );
	//   MC2Coordinate upper( 663394172, 155043599 );

   
//   // Europe
//   MC2Coordinate lower( 756634992, -175799752 );
//   MC2Coordinate upper( 405535631, 278927496 );

//   // Helsinki
//   MC2Coordinate lower( 717870134, 297503083 );
//   MC2Coordinate upper( 717839305, 297556052 );

   MC2BoundingBox bbox(lower, upper);
   
   setBoundingBox(bbox);
   setAngle(0);

//   MC2Coordinate paris( 582876115, 28066212 );
//   setCenter( paris );
//   setScale( 5.0 );
   
#if 0
   // Test putting the pin in the map
   m_userDefinedFeatures = new vector<UserDefinedFeature*>;
   UserDefinedBitMapFeature* mappin =
      new UserDefinedBitMapFeature( lower,
                                    "mappin");
   m_userDefinedFeatures->push_back( mappin );
#endif

}

TileMapHandler::~TileMapHandler()
{
   if ( m_dxxBuffer != NULL ) {
      m_requester->release( "DXXX", new BitBuffer ( *m_dxxBuffer ) );
      m_requester->release( "DYYY", m_dxxBuffer );
   }
   delete m_outlinePolygon;
   delete m_requester;
   delete m_dataBufferListener;

#ifndef QUICK_SHUTDOWN    
   {
      for ( bitMapMap_t::iterator it = m_bitMaps.begin();
            it != m_bitMaps.end();
            ++it ) {
         m_plotter->deleteBitMap( (*it).second );
      }
   }

   for ( vector<TileMapFormatDesc*>::iterator it = m_oldFormatDescs.begin();
         it != m_oldFormatDescs.end();
         ++it ) {
      delete *it;
   }

   delete m_mapFormatDesc;
#endif
   delete m_releaseChecker;
   delete m_tileMapCont;
   delete m_clickInfo;
}

void
TileMapHandler::matrixUpdated()
{
   // Checks and updates the parameters.
   updateParams();

   // We cannot continue drawing where we left off.
   m_matrixChangedSinceDraw = true;

   // Cancel detail repaint timer
   if ( m_detailRepaintTimerRequested ) {
      m_toolkit->cancelTimer(this, m_detailRepaintTimerRequested);
      m_detailRepaintTimerRequested = 0;
   }

   m_textHandler->mapVectorChanged( (*m_tileMapCont).beginMapsToDraw(),
                                    (*m_tileMapCont).endMapsToDraw(),
                                    !m_movementHelper.isMoving() );
   m_textHandlerIdleID = 0;
} 

void
TileMapHandler::toggleLayerToDisplay(int layerNo)
{
  // Why a toggle method?
  if(isLayerVisible(layerNo)) {
     /* layer is present, erase it */
     m_layersToDisplay.erase( layerNo );
  } else { /* add the layer to the display list */
     m_layersToDisplay.insert( layerNo );
  }
  /* request a repaint of the screen */
  requestRepaint();
  return;
}
   
VectorProxy<MC2Point>& 
TileMapHandler::getScreenCoords( TilePrimitiveFeature& feature )
{
   m_screenCoords.clear();
   if ( feature.isDrawn() || !m_isDrawing) {
      // The feature is drawn, i.e. calculate it's coordinates.
      switch ( feature.getType() ) {
         case FeatureType::line:
         case FeatureType::polygon:
            // Polylines and polygons uses prepareCoordinates.
            prepareCoordinates( feature, getBoundingBox() );
            break;
         case FeatureType::bitmap:
         case FeatureType::circle: {
            // Bitmaps uses getXYFromCoordArg.
            MC2Point center(0,0);
            getXYFromCoordArg( feature, getBoundingBox(), 
                               center.getX(), center.getY() );
            m_screenCoords.push_back( center );
            break;
         } 
      }
   }
   return m_screenCoords;
}

void
TileMapHandler::rotateLeftDeg(int angleDeg)
{
   setAngle(getAngle() + angleDeg);
}

void
TileMapHandler::clearRouteID()
{
   delete m_routeID;
   m_routeID = NULL;
   updateParams();
}

void
TileMapHandler::setRouteID(const RouteID& routeID)
{
   // First clear the route id so that the params are really updated
   clearRouteID();
   // Then set the new one.
   m_routeID = new RouteID(routeID);
   updateParams();
}

void 
TileMapHandler::setLanguage( LangTypes::language_t lang )
{
   m_requester->setPreferredLang( lang );
   m_lang = lang;
   // Ok to request new CRC if language differs.
   m_descCRCReceived = false;
}   

void 
TileMapHandler::setNav2Language( uint32 nav2Lang )
{
   setLanguage( LangTypes::getNavLangAsLanguage( nav2Lang ) );
}
  
const char* 
TileMapHandler::setLanguageAsISO639( const char* isoStr )
{
   // Convert to LangTypes:language_t.
   LangTypes::language_t langType = 
      LangTypes::getISO639AsLanguage( isoStr );
   if ( langType == LangTypes::invalidLanguage ) {
      // Default to english if the language was not supported.
      langType = LangTypes::english;
   } 
  
   // Set the language.
   setLanguage( langType );
   // And return the used language str..
   bool two = strlen( isoStr ) == 2;
   return LangTypes::getLanguageAsISO639( langType, two );
}


#define SQUARE(a) ((a)*(a))

int64
TileMapHandler::sqDistanceCoordToLine(const TileMapCoord& coord,
                                      const TileMapCoord& lineFirst,
                                      const TileMapCoord& lineSecond,
                                      float cosF )
{
   int32 xP = coord.getLon();
   int32 yP = coord.getLat();
   int32 x1 = lineFirst.getLon();
   int32 y1 = lineFirst.getLat();
   int32 x2 = lineSecond.getLon();
   int32 y2 = lineSecond.getLat();
   
   //ex and ey are unitvectors
   //
   //Vector V1 = (x2 - x1) * ex + (y2 - y1) * ey
   //Vector V2 = (xP -Removed all  x1) * ex + (yP - y1) * ey

   //Calculate the scalarproduct V1 * V2
   int64 scalarProd = int64( cosF * (x2 - x1) * cosF * (xP - x1) +
                             float( y2 - y1 ) * float( yP - y1 ));
   
   int64 dist;
   if( (x1 - x2 == 0) && (y1 == y2) )
   {
      //V1 has a length of zero, use plain pythagoras
      dist = SQUARE( int64( cosF*(xP - x1) ) ) + 
             SQUARE( int64( yP - y1 ) );
   }
   else if ( scalarProd < int64(0) )
   {
      //The angle between V1 and V2 is more than pi/2,
      //this means that (xP, yP) is closest to (x1, y1) 
      dist = SQUARE( int64( cosF*(xP - x1) ) ) +
             SQUARE( int64( yP - y1 ) );
   }
   else if ( scalarProd > int64( SQUARE( int64( cosF*(x2 - x1) ) ) +
                                 SQUARE( int64( y2 - y1 ) ) ) )
   {
      //The scalarproduct V1 * V2 is larger than the length of V1
      //this means that (xP, yP) is closest to (x2, y2) 
      dist = SQUARE( int64( cosF*(xP - x2) ) ) +
             SQUARE( int64( yP - y2 ) );
   }
   else 
   {
      //Use the formula for minimumdistance from a point to a line
      //dist = ( V2x * V1y - V2y * V1x )^2 / ( (V1x ^ 2) + (V1y ^ 2) )
      dist = int64( SQUARE( float(y2 - y1) * cosF*(xP - x1) + cosF*(x1 - x2) * float(yP - y1) ) /
             ( SQUARE( float(y2 - y1) ) + SQUARE( cosF*(x1 - x2) ) ) );
   }

   return dist;
}


int64
TileMapHandler::getMinSQDist(const TileMapCoord& coord,
                             const TilePrimitiveFeature& prim,
                             float cosLat)
{
   if ( prim.getType() == FeatureType::polygon ) {
      // Don't use the polygons, since the border isn't interesting there
      return MAX_INT64;
   }
   const CoordsArg* coords =
      static_cast<const CoordsArg*>(prim.getArg(TileArgNames::coords));
   if ( coords ) {
      CoordsArg::const_iterator it = coords->begin();
      CoordsArg::const_iterator lastit = it++;
      int64 minDist = MAX_INT64;
      while ( it != coords->end() ) {
         int64 curDist = sqDistanceCoordToLine(coord, *it, *lastit, cosLat);
         if ( curDist < minDist ) {
            minDist = curDist;
         }
         ++it;
         ++lastit;
      }
      return minDist;
   } else {
      // FIXME: Change this into inside the bounding box (bitmaps, that is)
      const CoordArg* coordArg =
         static_cast<const CoordArg*>(prim.getArg(TileArgNames::coord));
      // One coordinate
      if ( coordArg ) {
         return sqDistanceCoordToLine(coord,
                                      coordArg->getCoord(),
                                      coordArg->getCoord(),
                                      cosLat);
      }
   }
   // No coordinates at all.
   return MAX_INT64;
}

void
TileMapHandler::getClosestPrimCoord(const TileMapCoord& coord,
                                    const TilePrimitiveFeature& prim,
                                    float cosLat,
                                    MC2Coordinate& closestPrimCoord)
{
   if ( prim.getType() == FeatureType::polygon ) {
      // Don't use the polygons, since the border isn't interesting there
      closestPrimCoord = MC2Coordinate(0, 0);
   }
   const CoordsArg* coords =
      static_cast<const CoordsArg*>(prim.getArg(TileArgNames::coords));
   if ( coords ) {
      CoordsArg::const_iterator it = coords->begin();
      CoordsArg::const_iterator lastit = it++;
      int64 minDist = MAX_INT64;
      while ( it != coords->end() ) {
         int64 curDist = sqDistanceCoordToLine(coord, *it, *lastit, cosLat);
         if ( curDist < minDist ) {
            closestPrimCoord = *it;
         }
         ++it;
         ++lastit;
      }
   } else {
      // FIXME: Change this into inside the bounding box (bitmaps, that is)
      const CoordArg* coordArg =
         static_cast<const CoordArg*>(prim.getArg(TileArgNames::coord));
      // One coordinate
      if ( coordArg ) {
         closestPrimCoord = coordArg->getCoord();
      }
   }
}


pair<const TileMap*, const TilePrimitiveFeature*>
TileMapHandler::getFirstInside(const TileMapCoord& coord) const
{
   MC2Coordinate useCoord(coord.getLat(), coord.getLon());
   const TilePrimitiveFeature* closestSoFar = NULL;
   const TileMap* closestSoFarMap = NULL;

   // Loop again. Try to find the first polygon that has been clicked
   // inside.
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();
   for( int level = m_mapFormatDesc->getMaxLevel();
        level >= m_mapFormatDesc->getMinLevel();
        --level ) {
      for( MapsToDrawIt mIt = (*m_tileMapCont).beginMapsToDraw();
           mIt != endIt; ++mIt ) {
         TileMap* curMap = *mIt;
         if ( curMap == NULL ) {
            continue;
         }
         const TileMap::primVect_t& prims =
            curMap->getPrimitives(*m_mapFormatDesc,
                                  level);
         for( TileMap::primVect_t::const_iterator it = prims.begin();
              it != prims.end(); ++it ) {
            const TilePrimitiveFeature* curPrim = &(*it);
            if (curPrim->getType() == FeatureType::polygon ) {
               const CoordsArg* coords =
                  static_cast<const CoordsArg*>
                  (curPrim->getArg(TileArgNames::coords));
               if ( coords->getBBox().contains( useCoord ) ) {
                  // Real inside check here.
                  if ( InsideUtil::inside( coords->begin(),
                                           coords->end(),
                                           coord ) ) {                     
                     return make_pair(curMap, curPrim);
                  }
               }
            }
         }
      }
   }
   
   return make_pair(closestSoFarMap, closestSoFar);
}

/**
 *  Used to put small circles around 
 *  detected points (crossings etc).
 *  Only on unix, where the debug client
 *  resides.
 */

#ifdef __unix__
vector<MC2Coordinate> 
getCircle(TileMapHandler* mapHandler, MC2Coordinate center, int radius) 
{
   vector<MC2Coordinate> ret;
   MC2Point centerPoint(0,0);
   
   float cur = 0.0f;
   float step = 0.2f;

   float pixScale = mapHandler->getPixelScale();

   mapHandler->transformPointInternalCosLat( centerPoint.getX(),
                                             centerPoint.getY(),
                                             center );

   while(cur <= (2*M_PI + step)) {
      float curX = centerPoint.getX() + cos(cur) * radius / pixScale;
      float curY = centerPoint.getY() + sin(cur) * radius / pixScale;
      int xCoord = static_cast<int>(curX + 0.5f);
      int yCoord = static_cast<int>(curY + 0.5f);
      
      MC2Coordinate coord;
      mapHandler->inverseTranformUsingCosLat( coord, xCoord, yCoord);

      ret.push_back(coord);
      cur+=step;
   }

   return ret;
}

void TileMapHandler::markDetectedPoints(vicVec_t& points) {
   m_detectedPoints.clear();
   
   for(vicVec_t::iterator vt = points.begin();
       vt!=points.end(); 
       vt++) {
      MC2Coordinate curCoord = (*vt)->Coord();
      vector<MC2Coordinate> vmc = getCircle(this, curCoord, 18);
      m_detectedPoints.push_back(vmc);
   }

   requestRepaint();
}
#endif

void
TileMapHandler::getPOIsAndUDFsAlongRoad( const MC2Point& userPosition,
                                         const MC2SimpleString& roadName,
                                         vicVec_t& retVec)
{
   const int cutOff = 200;
   
   RoadCollection* allRoads = getAllRoads(userPosition,
                                          cutOff);

   RoadCollection onlyUsers;

   for(RoadCollection::iterator vi = allRoads->begin();
      vi != allRoads->end(); vi++)
   {
      Road& cur = *vi;
      if(cur.name() == roadName) {
         onlyUsers.push_back(cur);
      }
   }

   delete allRoads;

   vicVec_t pois;
   vicVec_t udfs;
   getPOIsInVicinity(userPosition, pois, cutOff);
   getUDFsInVicinity(userPosition, udfs, cutOff);
   
   //Only return pois and udfs which are along userRoad
   
   for(RoadCollection::iterator ri = onlyUsers.begin();
       ri!=onlyUsers.end();
       ri++)
   {
      for(vicVec_t::iterator vi = pois.begin(); vi!=pois.end(); vi++)
      {
         if(isAlongRoad(*ri, *vi)) {
            retVec.push_back(*vi);
         }
      }

      {for(vicVec_t::iterator vi = udfs.begin(); vi!=udfs.end(); vi++)
      {
         if(isAlongRoad(*ri, *vi)) {
            retVec.push_back(*vi);
         }
      }}
   }
}

bool
TileMapHandler::isAlongRoad(Road& road, VicinityItem* vItem) {
   const int ALONG_ROAD_MAX_DISTANCE = 20;
   float cosLat = getCosLat();
   
   for(Road::RoadSegmentVector::const_iterator rsv = road.begin();
       rsv!=road.end();
       rsv++)
   {
      MC2Point vItemPoint(0, 0);
      
      transformPointInternalCosLat( vItemPoint.getX(),
                                    vItemPoint.getY(),
                                    vItem->Coord() );

      int curDistance = getDistanceMeters(vItemPoint,
                                          rsv->startPosition(),
                                          rsv->endPosition(),
                                          cosLat);

      if(curDistance < ALONG_ROAD_MAX_DISTANCE) {
         return true;
      }
   }
       
   return false;
}



void
TileMapHandler::getCrossingsAlongRoad( const MC2Point& userPosition,
                                       const MC2SimpleString& roadName,
                                       vicVec_t& retVec,
                                       bool overRideCutOff,
                                       int cutOff)
{
   // Start with 200 meters as cutoff and then move up.
   int cutoffMeters = START_CUTOFF_CROSSINGS;
   
   if(overRideCutOff) {
      cutoffMeters = cutOff - 1;
   }
   
   while ( cutoffMeters <= cutOff && retVec.size() < 15 ) {
      VicinityItem::ClearAndDelete(retVec);

      getCrossingsInVicinityLimited( userPosition,
                                     roadName,
                                     retVec,
                                     cutoffMeters );
   
      mc2dbg << "Found " << retVec.size() << " crossings within " 
             << cutoffMeters << " meters." << endl;
      cutoffMeters *= 2;
   }
}


void
TileMapHandler::getCrossingsInVicinity(const MC2Point& userPosition, 
                                       vicVec_t& retVec, 
                                       bool overRideCutOff,
                                       int cutOff) 
{
   // Start with 200 meters as cutoff and then move up.
   int cutoffMeters = START_CUTOFF_CROSSINGS;

   if(overRideCutOff) {
      cutoffMeters = cutOff - 1;
   }


   MC2SimpleString userRoad; //Empty since we want to search all roads
   
   while ( cutoffMeters <= cutOff && retVec.size() < 15 ) {
      VicinityItem::ClearAndDelete(retVec);

      getCrossingsInVicinityLimited( userPosition,
                                     userRoad,
                                     retVec,
                                     cutoffMeters );
   
      mc2dbg << "Found " << retVec.size() << " crossings within " 
             << cutoffMeters << " meters." << endl;
      cutoffMeters *= 2;
   }
}

RoadCollection*
TileMapHandler::getAllRoads(const MC2Point& centerPos,
                            int cutOffMeters)
{
   // Get the coordinate for the point
   MC2Coordinate centerCoord;
   inverseTranformUsingCosLat( centerCoord,
                               centerPos.getX(),
                               centerPos.getY() );

   
   MC2BoundingBox includeBox( centerCoord, cutOffMeters ); 

   RoadCollection* allCollections = new RoadCollection;

   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();
   
   for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
       it != endIt; ++it )
   {
      TileMap* curMap = *it;

      if ( curMap == NULL ) {
         continue;
      }      

      if(curMap->getParams()->getLayer()!=TileMapTypes::c_mapLayer) {
         continue;
      }                

      for(  int level = m_mapFormatDesc->getMaxLevel();
            level >= m_mapFormatDesc->getMinLevel();
            --level ) 
      {

         RoadCollection* r2 = getRoadsInTileMap(curMap, 
                                                level, 
                                                includeBox);

         std::copy(     r2->begin(), 
                        r2->end(), 
                        std::back_inserter(*allCollections));

         delete r2;
      }
   }
   
   return allCollections;
}

void
TileMapHandler::getCrossingsInVicinityLimited(const MC2Point& userPosition,
                                              const MC2SimpleString& roadName,
                                              vicVec_t& retVec,
                                              int cutoffMeters) 
{
   if ( m_mapFormatDesc == NULL ) {
      return ;
   }

   // Get the coordinate for the point
   MC2Coordinate coord;
   inverseTranformUsingCosLat( coord,
                               userPosition.getX(),
                               userPosition.getY() );

   RoadCollection* allCollections = getAllRoads(userPosition,
                                                cutoffMeters);
   
   if(roadName.empty()) {
      findIntersections(retVec,
                        *allCollections,
                        coord,
                        cutoffMeters);
   }
   else {
      findIntersections(retVec,
                        roadName,
                        *allCollections,
                        coord,
                        cutoffMeters);
   }      
   
   delete allCollections;
}

RoadCollection* 
TileMapHandler::getRoadsInTileMap (TileMap* tileMap,
                                   int mapLevel,
                                   const MC2BoundingBox& includeBox) 
{
   RoadCollection* roads = new RoadCollection;

   const TileMap::primVect_t& prims =
      tileMap->getPrimitives(    *m_mapFormatDesc,
                                mapLevel);


   bool entered = false;
   //Take all types
   for( TileMap::primVect_t::const_iterator it = prims.begin(); 
        it != prims.end(); 
        ++it ) 
   {   
      if( it->getType() != FeatureType::line )
         continue;

      entered = true;
      
      const CoordsArg* coords = static_cast<const CoordsArg*>(
         (*it).getArg(TileArgNames::coords));
      if ( coords == NULL ) {
         continue;
      }
      if ( ! coords->getBBox().overlaps( includeBox ) ) {
         continue;
      }
      
      Road r = createRoad(tileMap, *it);
      if(!r.isEmpty())
         roads->push_back(r);
   }

   return roads;
}

MC2SimpleString
TileMapHandler::getServerInfo(MC2Point point) 
{
   MC2SimpleString ret("");
   
   TileMapHandlerClickResult clickRes;
   getInfoForFeatureAt( clickRes, point );

   return MC2SimpleString( clickRes.getServerString() );
}



int TileMapHandler::getFeatureId(TileMap* theMap,
                                 const TilePrimitiveFeature* feature)
{
   const TileFeature* parent =
      theMap->getParentFeature(feature);
                  
   if(parent) {
      return parent->getType();
   }  else {
      return FeatureType::nonexisting;
   }
}


std::vector<MC2SimpleString> 
TileMapHandler::getLayeredPointInfo(MC2Point point) 
{  
   std::vector<MC2SimpleString> ret;
   
   if ( m_mapFormatDesc == NULL ) {
      return ret;
   }             
   
   TilePrimitiveFeature roadStandingOn;
   TileMap*             roadStandingOnMap = NULL;
   
   int curDistanceToRoad = MAX_INT32;
   
   // Loop
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();
      
   for( int level = m_mapFormatDesc->getMinLevel();
        level <=m_mapFormatDesc->getMaxLevel();
        ++level ) {

      for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
          it != endIt; it++ ) {
         
         TileMap* curMap = *it;
         if ( curMap == NULL ) {
            continue;
         }

         const TileMap::primVect_t& prims =
               curMap->getPrimitives(*m_mapFormatDesc,
                                      level);

         for(TileMap::primVect_t::const_iterator curPrim = prims.begin();
             curPrim!=prims.end();
             curPrim++) 
         {
            if(primitiveOverlaps(point, *curPrim, curDistanceToRoad))
            {
               const TilePrimitiveFeature& curFeature = *curPrim;
               const char* name = 
                  getNameForFeature(*curMap, curFeature, NULL );
               
               if(!name) {
                  continue;
               }
               
               if(curPrim->getType() == FeatureType::line) {
                  roadStandingOnMap = curMap;
                  roadStandingOn = *curPrim;
               } else if(curPrim->getType() == FeatureType::polygon) {
                  if(getFeatureId(curMap, &curFeature)==FeatureType::land) {
                     continue;
                  }
                  
                  //Some complex polygons are appearantly split up,
                  //so their names appear twice.
                  if(ret.empty()) {
                     ret.push_back(name);
                  } else if(ret.back()!=name) {
                     ret.push_back(name);
                  }
               }
            }
         }
      }
   }
   
   if(roadStandingOnMap && curDistanceToRoad < ROAD_DISTANCE_CUTOFF) {
      const char* roadName = 
         getNameForFeature( *roadStandingOnMap, roadStandingOn, NULL );
      
      MC2SimpleString road (roadName);
      road.append(" ");
      
      ret.push_back( road );
   }


   return ret;
}


bool
TileMapHandler::primitiveOverlaps(const MC2Point& centerPoint,
                                  const TilePrimitiveFeature& prim,
                                  int& smallestLineDist)
{   
   float cosLat = getCosLat();

   // Get the coordinate for the point
   MC2Coordinate centerCoord;
   inverseTranformUsingCosLat( centerCoord,
                               centerPoint.getX(),
                               centerPoint.getY() );
   
   bool hitOverlap = false;
   
   if (prim.getType() == FeatureType::polygon) {
       const CoordsArg* coords =
          static_cast<const CoordsArg*>
          (prim.getArg(TileArgNames::coords));

       //Quick throwaway check. If there is no overlap of
       //the bounding boxes there cannot be any overlap

       if ( !coords->getBBox().contains( centerCoord ) ) {
         return false;
       }
       
       TileMapCoord center( centerCoord.lat,
                            centerCoord.lon );


       std::vector<TileMapCoord> args;

       for(CoordsArg::const_iterator a = coords->begin();
           a!=coords->end();
           a++) {
          TileMapCoord tc ( a->lat, a->lon );
          
          args.push_back(tc);
       }

       if(args.front()!=args.back()) {
          //  args.push_back(args.front());
       }
       
       
       //Do the complete checking
       return
          InsideUtil::inside( args.begin(),
                              args.end(),
                              center);
   }
   else if(prim.getType() == FeatureType::line) {
      TileMapCoord center( centerCoord.lat,
                           centerCoord.lon );
      
      const CoordsArg* coords =  
         static_cast<const CoordsArg*>
         (prim.getArg(TileArgNames::coords));
      
      CoordsArg::const_iterator it = coords->begin();
      CoordsArg::const_iterator lastit = it++;

      while ( it != coords->end() ) {
         int64 curDist = sqDistanceCoordToLine(center,
                                               *it,
                                               *lastit,
                                               cosLat);
         
         float curDistMeterf = 
            sqrt( float( curDist ) ) *
            GfxConstants::MC2SCALE_TO_METER;
   
         int curDistMeter =
            static_cast<int64>( curDistMeterf + 0.5f );
         
         if ( curDistMeter < smallestLineDist ) {
            smallestLineDist = curDistMeter;
            hitOverlap = true;
         }
         ++it;
         ++lastit;
      }
   }
   else if (prim.getType()==FeatureType::bitmap) {
      //Since the return data is supposed to be used as auxillary
      //"Where am I"-functionality in a context where the closest
      //POI's, crossings and UDF's are displayed, this would be
      //unnecessary data
      
      /*if(prim.isDrawn() &&
         getPixelBoxForBitMap( prim ).pointInside ( centerPoint ))
      {
         hitOverlap = true;
         }*/
   }

   return hitOverlap;
}

MC2SimpleString
TileMapHandler::getRoadAt(MC2Point point) {
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();

   Road curRoad;
   int smallestDist = MAX_INT32;
   
   for( int level = m_mapFormatDesc->getMaxLevel();
        level >= m_mapFormatDesc->getMinLevel();
        --level ) {
      for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
          it != endIt; ++it ) {
         TileMap* curMap = *it;
         if ( curMap == NULL ) {
            continue;
            }
         
         if(   curMap->getParams()->getLayer() != TileMapTypes::c_mapLayer) {
            continue;
         }
         
         TileMap::primVect_t& prims =
            curMap->getPrimitives(*m_mapFormatDesc,
                                  level);
         
         TileMap::primVect_t::iterator theEnd = prims.end();
         
         for ( TileMap::primVect_t::iterator it = prims.begin();
               it != theEnd;
               ++it ) 
         {
            if(it->getType() == FeatureType::line) {
               //Calculate distance and notice if it is lower.

               const TilePrimitiveFeature& feat = *it;
               
               int distance = getDistanceMeters(point, feat, getCosLat());
               
               if(distance<smallestDist) {
                  smallestDist = distance;
                  curRoad = createRoad(curMap, *it);
               }
            }
         }
      }
   }

   return curRoad.name();
}

void 
TileMapHandler::findIntersections(vicVec_t& retVector,
                                  const MC2SimpleString& roadName,
                                  RoadCollection& roads, 
                                  const MC2Coordinate& coord, 
                                  int cutoff) 
{
   CrossingsMap crossCollection;

   for(vector<Road>::iterator vi = roads.begin();
      vi != roads.end(); vi++)
   {
      Road& outer = *vi;
      
      if(outer.name() != roadName) {
         continue;
      }
      
      for(  vector<Road>::iterator innerV = roads.begin();
            innerV != roads.end();
            innerV++) 
      {
         Road& inner = *innerV;

         if(!inner.sameRoad(outer))
            outer.getCrossings(inner, crossCollection, this);
      }
   }

   crossCollection.getAllCrossings(this, coord, cutoff, retVector);
}


void 
TileMapHandler::findIntersections(vicVec_t& retVector, 
                                  RoadCollection& roads, 
                                  const MC2Coordinate& coord, 
                                  int cutoff) 
{
   CrossingsMap crossCollection;

   for(vector<Road>::iterator vi = roads.begin();
      vi != roads.end(); vi++) {
      Road& outer = *vi;
         
      vector<Road>::iterator step = vi + 1;

      if(vi == roads.end())
         break;

      for(  vector<Road>::iterator innerV = step;
            innerV != roads.end();
            innerV++) 
      {
         Road& inner = *innerV;

         if(!inner.sameRoad(outer))
            outer.getCrossings(inner, crossCollection, this);
      }
   }

   crossCollection.getAllCrossings(this, coord, cutoff, retVector);
}

Road 
TileMapHandler::createRoad(const TileMap* tileMap, 
                           const TilePrimitiveFeature& feature) 
{
   const CoordsArg* coords = static_cast<const CoordsArg*>(
      feature.getArg(TileArgNames::coords));

   const SimpleArg* argStartLevel = 
      static_cast<const SimpleArg*>(feature.getArg(TileArgNames::level));
   const SimpleArg* argEndLevel = 
      static_cast<const SimpleArg*>(feature.getArg(TileArgNames::level_1));

   const int startLevel = argStartLevel ? argStartLevel->getValue() : 0;
   const int endLevel =   argEndLevel ? argEndLevel->getValue() : 0; 

   if ( coords == NULL ) {
      return Road("", startLevel, endLevel);
   }

   const char* name = 
      getNameForFeature( *tileMap, feature, NULL );

   if(!name)
      return Road("", startLevel, endLevel);

   Road ret(name, startLevel, endLevel);

   if ( feature.getType() != FeatureType::line ) 
      return ret;
   
   MC2Point prevPoint(0, 0);


   for ( CoordsArg::const_iterator it ( coords->begin() );
         it != coords->end();
         ++it )
      {
         MC2Point curPoint(0, 0);
         transformPointInternalCosLat( curPoint.getX(),
                                       curPoint.getY(),
                                       *it );
         
         if(it!=coords->begin()) {
            RoadSegment r(prevPoint, curPoint);
            ret.addRoadSegment(r);
         }         

         prevPoint = curPoint;
      }

   return ret;
}


void
TileMapHandler::getPOIsInVicinity(const MC2Point& userPosition, 
                                  vicVec_t& retVec,
                                  int cutOff) 
{
   if ( m_mapFormatDesc == NULL ) {
      return ;
   }   
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();


   for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
            it != endIt; ++it ) 
   {      
      TileMap* curMap = *it;
      
      if ( curMap == NULL ) {
         continue;
      }

      if( curMap->getParams()->getLayer() == TileMapTypes::c_mapLayer ||
          curMap->getParams()->getLayer() == TileMapTypes::c_routeLayer ) {
         continue;
      }        

      for( int level = m_mapFormatDesc->getMaxLevel();
           level >= m_mapFormatDesc->getMinLevel();
           --level ) {
         
         
         addPOIsInTileMapLevel(curMap,
                               level, 
                               userPosition,
                               cutOff, 
                               retVec);
      }            
   }
}

inline void
TileMapHandler::addPOIsInTileMapLevel( TileMap* tileMap,
                                       int level,
                                       const MC2Point& userPosition,
                                       int cutoff,
                                       vicVec_t& retVec) 
{
   
   const TileMap::primVect_t& prims =
      tileMap->getPrimitives(*m_mapFormatDesc, level);


   ///Evaluate all primitives on level.
   for( TileMap::primVect_t::const_iterator it = prims.begin(); 
        it != prims.end(); 
        ++it ) 
      {
         evaluatePrimitiveAsPOI(tileMap, 
                                *it, 
                                userPosition,
                                retVec,
                                cutoff);
      }
}

void
TileMapHandler::evaluatePrimitiveAsPOI(TileMap* tileMap,
                                       const TilePrimitiveFeature& primitive,
                                       const MC2Point& userPosition,
                                       vicVec_t& retVec,
                                       int cutOff)
{
   if( primitive.getType() != FeatureType::bitmap ||
       forbiddenFeature( *tileMap, primitive) )
   {
      return;
   }

       float cosLat = getCosLat();

   // Get the coordinate for the point
   MC2Coordinate userCoord;
   inverseTranformUsingCosLat( userCoord, 
                               userPosition.getX(), 
                               userPosition.getY() );

   TileMapCoord tileUserCoord( userCoord.lat, userCoord.lon);
   
   int64 curDist = getMinSQDist( tileUserCoord, primitive, cosLat );
   float curDistMeter = 
      sqrt( float( curDist ) ) * GfxConstants::MC2SCALE_TO_METER;
   
   int curDist_meter = static_cast<int64>( curDistMeter + 0.5f );
   if( curDist_meter > cutOff )
      return;
   
   const char* name = 
      getNameForFeature( *tileMap, primitive, NULL );
   
   if (!name)
      return;

   
   MC2Coordinate closestPrimCoord;
   getClosestPrimCoord( tileUserCoord, 
                        primitive, 
                        cosLat, 
                        closestPrimCoord );

   const MC2Coordinate cTmp(closestPrimCoord);

   pair<const TileMap*,
      const TilePrimitiveFeature*> mapAndFeature(tileMap, &primitive);

   MC2SimpleString serverName =
      createServerString( name,
                          cTmp,
                          mapAndFeature);
   
   float64 angleFromNorthFloat = 
      GfxUtility::getAngleFromNorthDegrees(  userCoord.lat, 
                                             userCoord.lon,
                                             closestPrimCoord.lat, 
                                             closestPrimCoord.lon);

   int featureId  = getFeatureId(tileMap, &primitive);

   MC2SimpleString category;
         
   if ( m_mapFormatDesc ) {
      category = m_mapFormatDesc->getCategoryNameForFeature( featureId );
   }
   
   int angleFromNorthInt =
      static_cast<int>( angleFromNorthFloat + 0.5f );


   
   retVec.push_back( new VicinityItem( curDist_meter, 
                                       name,
                                       serverName,
                                       closestPrimCoord, 
                                       MC2Direction(angleFromNorthInt),
                                       category) );
}

void TileMapHandler::getUDFsInVicinity( const MC2Point& userPosition,
                                        vicVec_t& retVec,
                                        int cutOff)  
{
   MC2Coordinate userCoord;
   inverseTranformUsingCosLat( userCoord,
                               userPosition.getX(),
                               userPosition.getY());

   if ( m_userDefinedFeatures == NULL ) {
      return;
   }
   
   float cosLat = getCosLat();

   // Go from the top down and check if there are any UserDefined features
   // which the point is inside (bitmaps only)

   for ( int i = (*m_userDefinedFeatures).size() - 1;
         i >= 0;
         --i ) {
      UserDefinedFeature* curFeat = (*m_userDefinedFeatures)[i];
      // First some checks for what cannot be done
      if ( curFeat == NULL) continue;
      if ( curFeat->getType() != UserDefinedFeature::bitmap ) continue;
      if ( curFeat->isClickable() == false ) continue;

      // Ok
      UserDefinedBitMapFeature* bmp =
         static_cast<UserDefinedBitMapFeature*>(curFeat);

      MC2Coordinate mc = bmp->getCenter().getWorldCoord();
      float curDist = 
         float( GfxUtility::squareP2Pdistance_linear(userCoord, mc, cosLat));
      
      float curDistMeter = sqrt(float(curDist));
      int curDist_meter = static_cast<int64>(curDistMeter + 0.5f);
      
      if ( curDist_meter < cutOff) {
         const MC2SimpleString name = 
            curFeat->getName();

         float64 angleFromNorth =   GfxUtility::
            getAngleFromNorthDegrees(  userCoord.lat, 
                                       userCoord.lon,
                                       mc.lat, mc.lon );

         int angleFromNorthInt =
            static_cast<int> (angleFromNorth + 0.5f);
         
         if(!name.empty()) {
            retVec.push_back(
               new UDFItem(   curDist_meter, 
                              name,
                              mc, 
                              MC2Direction(angleFromNorthInt),
                              curFeat)
               );
         }
      }         
   }
}

inline const TilePrimitiveFeature*
TileMapHandler::getClosest(const MC2Point& point,
                           const MC2Coordinate& coord,
                           TileMap::primVect_t::const_iterator begin,
                           TileMap::primVect_t::const_iterator end,
                           int64& mindist,
                           int primType ) const {
   TileMapCoord tileCoord( coord.lat, coord.lon);
   const TilePrimitiveFeature* closestFeature = NULL;
   float cosLat = getCosLat();
   mindist = MAX_INT64;
   for(  TileMap::primVect_t::const_iterator it = begin; it != end; ++it ) {
      if ( ( primType != MAX_INT32 ) && 
           ( it->getType() != primType ) ) {
         // Skip this feature. Was not of the desired type.
         continue;
      }
      if ( it->getType() == FeatureType::bitmap ) {
         // Check if it has been drawn.
         if ( (*it).isDrawn() || !m_isDrawing) {
            if ( getPixelBoxForBitMap( *it ).pointInside ( point ) ) {
               mindist = 0;
               return &(*it);
            }
         }
      } else {
         int64 curDist = getMinSQDist(tileCoord, (*it), cosLat);
         if ( curDist < mindist ) {
            mindist = curDist;
            closestFeature = &(*it);
         }
      }
   }
   return closestFeature;
}

pair<const TileMap*, const TilePrimitiveFeature*>
TileMapHandler::getFeatureAt(const MC2Point& point,
                             uint32* /*distance*/, 
                             int32 primType )
{
   const pair<const TileMap*, const TilePrimitiveFeature*>
      nullPair(NULL, NULL);
   
   if ( m_mapFormatDesc == NULL ) {
      return nullPair;
   }                                     
   // Get the coordinate for the point
   MC2Coordinate coord;
   inverseTranformUsingCosLat( coord, point.getX(), point.getY() );
   
   // Make the bounding box some pixels wide.
   const int nbrPixels = 5;
   MC2Coordinate corner1;
   inverseTranformUsingCosLat( corner1,
                               point.getX() + nbrPixels,
                               point.getY() + nbrPixels );
   MC2Coordinate corner2;
   inverseTranformUsingCosLat( corner2,
                               point.getX() + nbrPixels,
                               point.getY() + nbrPixels );

   const TilePrimitiveFeature* closestSoFar = NULL;
   const TileMap* closestSoFarMap = NULL;
   int64 minDist = MAX_INT64;
   // Loop
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();
   for( int level = m_mapFormatDesc->getMaxLevel();
        level >= m_mapFormatDesc->getMinLevel();
        --level ) {
      for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
          it != endIt; ++it ) {
         TileMap* curMap = *it;
         if ( curMap == NULL ) {
            continue;
         }
         const TileMap::primVect_t& prims =
               curMap->getPrimitives(*m_mapFormatDesc,
                                      level);
//           set<TilePrimitiveFeature*> overlappingBBox;
//           for ( TileMap::primVect_t::const_iterator it = prims.begin();
//                 it != prims.end();
//                 ++it ) {
//              const TilePrimitiveFeature* curPrim = *it;
//              if ( insideBBox(bbox, *curPrim) ) {
//                 overlappingBBox.insert(curPrim);
//              }
//              const TilePrimitiveFeature* prim =
//                 getClosest(coord, overlappingBBox, mindist);
//           }
         int64 curDist;
         const TilePrimitiveFeature* curPrim =
            getClosest(point, coord, prims.begin(), prims.end(), 
                       curDist, primType );
         if ( curDist < minDist ) {
            minDist         = curDist;
            closestSoFar    = curPrim;
            closestSoFarMap = curMap;
         }                                   
      }
   }

   float minDistPixels = sqrt(float(minDist)) / getPixelScale() *
      GfxConstants::MC2SCALE_TO_METER;

   mc2dbg << "[TMH]: Distance in pixels = " << minDistPixels << endl;

   // Check that we are interested in polygons.
   if ( (( primType == MAX_INT32 ) || 
         (primType == FeatureType::polygon )) &&
        minDistPixels > 10 ) {
      pair<const TileMap*, const TilePrimitiveFeature*> closestInside =
         getFirstInside(TileMapCoord(coord.lat, coord.lon));
      closestSoFarMap = closestInside.first;
      closestSoFar = closestInside.second;
   }
   
   if ( closestSoFarMap ) {
      /*mc2dbg << "[TMH]: Closest feature is " << MC2HEX((uint32)closestSoFar)
             << " with type = "
             << ( (closestSoFar != NULL) ? closestSoFar->getType() : -65536)
             << " and dist " << minDist << " featnbr = "
             << closestSoFar->getFeatureNbr() << endl;*/
      // Set closestSoFar to its parent so I can check what type it has.
      //closestSoFar = closestSoFarMap->getParentFeature(closestSoFar);
   }
   
   return make_pair(closestSoFarMap, closestSoFar);
}

const char*
TileMapHandler::getNameForFeature( const TileMap& theMap,
                                   const TilePrimitiveFeature& theFeature, 
                                   TileMapInfoCallback* infoCallback )
{
   // Moved from another function.
   const TileMap* closestSoFarMap           = &theMap;
   const TilePrimitiveFeature* closestSoFar = &theFeature;
  
   // Copy the params and change it into string map params.
   const TileMapParams* stringParams =
      closestSoFarMap->getStringMapParams(m_lang);
   mc2dbg8 << "[TMH]: New params : " << *stringParams << endl;
   const TileMap* stringMap = getMap(*stringParams);      
   if ( stringMap == NULL ) {
      // Not found or not loaded yet.
      // Notify the tilemap info callback when this map is received.
      m_waitingForInfoString.first = stringParams->getAsString();
      m_waitingForInfoString.second = infoCallback;
      
      requestFromCacheOrForReal(*stringParams);
      mc2dbg8 << "[TMH]: No string map - requesting " << endl;         
      return NULL;
   } else {
      // No callback wanted
      m_waitingForInfoString.second = NULL;
      const char* str = stringMap->getStringForFeature(
         closestSoFar->getFeatureNbr());
      if ( str ) {
         return str;
      } else {
         mc2dbg8 << "[TMH]: No string found in map" << endl;
         return NULL;
      }
   }      
}

const char*
TileMapHandler::getNameForFeatureAt(const MC2Point& point,
                                    uint32* distance )
{
   if ( m_mapFormatDesc == NULL ) {
      return NULL;
   }   
   
   TileMapHandlerClickResult clickRes;
   getInfoForFeatureAt( clickRes, point );
   if ( clickRes.getClickedUserFeature() != NULL ) {
      // A user feature was clicked.
      if ( clickRes.getClickedUserFeature()->getName().empty() ) {
         return NULL;
      } else {
         return clickRes.getClickedUserFeature()->getName().c_str();
      }
   }

   pair<const TileMap*, const TilePrimitiveFeature*> mapAndFeature =
      getFeatureAt(point, distance);
   if ( mapAndFeature.first ) {
      return getNameForFeature( *mapAndFeature.first,
                                *mapAndFeature.second );
   } else {
      return NULL;
   }
}


static
MC2SimpleString
createServerString( const char* name,
                    const MC2Coordinate& coord,
                    pair<const TileMap*,
                    const TilePrimitiveFeature*>& mapAndFeature )
{
   const bool closed =
      mapAndFeature.second->getType() == FeatureType::polygon;
   
   static const int latLonLength = 11*2 + 2;
   static const int extraLength = 4 + 100; // Colons and more.
   char* tempString = new char[strlen(name) + latLonLength + extraLength];
   sprintf(tempString, "C:%ld:%ld:%ld:%s",
           (long int)coord.lat, (long int)coord.lon,
           (long int)closed, name);
   // Return and delete the string
   return MC2SimpleStringNoCopy( tempString );
}

static inline
void setDirectedPolyFromPixelBox( DirectedPolygon& poly,
                                  const PixelBox& pixBox )
{
   MC2Point center( pixBox.getCenterPoint() );
   poly.setCenter( center );
   vector<MC2Point> tmpVec;
   for( int i = 0; i < 5; ++i ) {
      tmpVec.push_back( pixBox.getCorner(i) - center );
   }
   poly.swapPoints( tmpVec );
   
}

PixelBox 
TileMapHandler::getPixelBoxForBitMap( 
            const TilePrimitiveFeature& bitmap ) const 
{ 
   MC2_ASSERT( bitmap.getType() == FeatureType::bitmap );

   MC2Point center(0,0);
   getXYFromCoordArg( bitmap, getBoundingBox(), 
         center.getX(), center.getY() );

   if(!m_isDrawing) {
      PixelBox pb(32); //If we aren't drawing, we cannot get
                       //useful information.
      pb.moveCenterTo(center);
      return pb;
   }
   
   const StringArg* bitMapName =
      static_cast<const StringArg*>
      (bitmap.getArg(TileArgNames::image_name));

   MC2_ASSERT( bitMapName != NULL );
   
   bitMapMap_t::const_iterator it = 
      m_bitMaps.find( bitMapName->getValue() );
   MC2_ASSERT( it != m_bitMaps.end() );

   // Fetch bitmap size from the plotter.
   isab::Rectangle rect;
   m_plotter->getBitMapAsRectangle( rect, center, it->second );
   return rect;
}

void
TileMapHandler::getInfoForFeatureAt( TileMapHandlerClickResult& res,
                                     const MC2Point& point,
                                     bool highlightable,
                                     TileMapInfoCallback* infoCallback )
   
{
   // Reset the waiting for info callback to avoid crash when exiting
   // VectorMapContainer.
   m_waitingForInfoString.second = NULL;
   
   // Reset the result
   res = TileMapHandlerClickResult();
   
   if ( m_mapFormatDesc == NULL ) {
      return;
   }   

   if ( m_userDefinedFeatures != NULL ) {
      // Go from the top down and check if there are any UserDefined features
      // which the point is inside (bitmaps only)
      UserDefinedFeature* foundUserDef = NULL;
      PixelBox foundPixelBox;
      for ( int i = (*m_userDefinedFeatures).size() - 1;
            i >= 0;
            --i ) {
         UserDefinedFeature* curFeat = (*m_userDefinedFeatures)[i];
         // First some checks for what cannot be done
         if ( curFeat == NULL) continue;
         if ( curFeat->getType() != UserDefinedFeature::bitmap ) continue;
         if ( curFeat->isClickable() == false ) continue;
         
         // Ok
         UserDefinedBitMapFeature* bmp =
            static_cast<UserDefinedBitMapFeature*>(curFeat);
         // Check the pixelbox for the feature (by using the bitmap)
         if ( bmp->getBitMapName() != NULL ) {
            isab::BitMap* bitMap =
               getOrRequestBitMap( bmp->getBitMapName() );
            if ( bitMap != NULL ) {
               // Ok to get the pixelboxel
               PixelBox tmpBox( m_plotter->getBitMapAsRectangle(
                  bmp->getScreenCoords().front(),
                  bitMap ) );
               if ( tmpBox.pointInside( point ) ) {
                  foundUserDef  = curFeat;
                  foundPixelBox = tmpBox;
                  break;
               }
            }
         }
      }
      if ( foundUserDef != NULL ) {
         // Make outline, then we're done.
         setDirectedPolyFromPixelBox( *m_outlinePolygon,
                                      foundPixelBox );
         m_outlinePolygon->setVisible(true);
         res.m_selectionOutline = m_outlinePolygon;
         res.m_clickedFeature = foundUserDef;
         res.m_name = foundUserDef->getName();
         return; // RETURN - all is done
      }
   }


   // Second part. Try to find feature in the map.
   // If only highlightable features should be found, then set
   // the last param to FeatureType::bitmap, otherwise
   // MAX_INT32.
   pair<const TileMap*, const TilePrimitiveFeature*> mapAndFeature =
      getFeatureAt(point, &res.m_distance, 
             highlightable ? FeatureType::bitmap : MAX_INT32 );
   if ( mapAndFeature.first ) {
      const char* name = getNameForFeature( *mapAndFeature.first,
                                            *mapAndFeature.second,
                                            infoCallback );
      // Everything needs the name of the feature.
      if ( name ) {
         // Check if it is possible to create an outline
         if ( mapAndFeature.second->getType() ==
              FeatureType::bitmap ) {
            if ( mapAndFeature.second->isDrawn() || !m_isDrawing) {
               

               setDirectedPolyFromPixelBox( *m_outlinePolygon,
                     getPixelBoxForBitMap( *mapAndFeature.second ) );
               m_outlinePolygon->setVisible(true);
               // All is ok - set the stuff
               res.m_selectionOutline = m_outlinePolygon;
            } 
         } else {
            // Not bitmap - cannot create selection outline.
            m_outlinePolygon->setVisible(false);
         }
         // Set name and also server string.
         res.m_name = name;
         // Translate the clicked point
         MC2Coordinate coord;
         if ( mapAndFeature.second->getType() == FeatureType::bitmap ) {
            // Point feature. Add the point of the feature instead of
            // the clicked coord.
            const CoordArg* coordArg =
               static_cast<const CoordArg*>
                  (mapAndFeature.second->getArg(TileArgNames::coord));
            coord.lat = coordArg->getCoord().getLat();
            coord.lon = coordArg->getCoord().getLon();
         } else {
            // Non point feature. Add the clicked coord.
            inverseTranformCosLatSupplied( 
                                    coord, point.getX(), point.getY(),
                                    getCosLat(getCenter().lat ) );
         }
         // Get server string.
         res.m_serverString = createServerString( name,
                                                  coord,
                                                  mapAndFeature );
         return;
      }
   }
   // Clear the result.
   res = TileMapHandlerClickResult();
   m_outlinePolygon->setVisible(false);
}

const ClickInfo&
TileMapHandler::getInfoForFeatureAt( const MC2Point& point,
                                     bool onlyPois,
                                     TileMapInfoCallback* infoCallback )
{
   // Reset the waiting for info callback to avoid crash when exiting
   // VectorMapContainer.
   m_waitingForInfoString.second = NULL;

   getInfoForFeatureAt( *m_clickInfo,
                        point,
                        onlyPois,
                        infoCallback );
   return *m_clickInfo;
}

int
TileMapHandler::
getServerIDStringForFeaturesAt( vector<MC2SimpleString>& strings,
                                const MC2Point& point )
{

#if 0
   if ( m_mapFormatDesc == NULL ) {
      return 0;
   }
   
   MC2Coordinate coord;
   inverseTranformCosLatSupplied( coord, point.getX(), point.getY(),
                                     getCosLat(getCenter().lat ) );
   uint32 distance;
   pair<const TileMap*, const TilePrimitiveFeature*> mapAndFeature =
      getFeatureAt(point, &distance);

   if ( ( mapAndFeature.first != NULL ) && ( mapAndFeature.second != NULL ) ){ 
      // Should really use a point on the feature.
      const char* name = getNameForFeature( *mapAndFeature.first,
                                            *mapAndFeature.second );
      if ( name == NULL ) {
         return 0;
      }

      const bool closed =
         mapAndFeature.second->getType() == FeatureType::polygon;

      static const int latLonLength = 11*2 + 2;
      static const int extraLength = 4 + 100; // Colons and more.
      char* tempString = new char[strlen(name) + latLonLength + extraLength];
      sprintf(tempString, "C:%ld:%ld:%ld:%s",
              (long int)coord.lat, (long int)coord.lon,
              (long int)closed, name);
      // Copy the string into the vector.
      strings.push_back( tempString );
      delete [] tempString;  
   } 
   return strings.size();
#else
   // New version, the one to keep, probably.
   TileMapHandlerClickResult res;
   getInfoForFeatureAt( res, point );
   if ( ! res.m_serverString.empty() ) {
      strings.push_back( res.m_serverString );
   }
   return strings.size();
#endif
}

void
TileMapHandler::setUserDefinedFeatures( vector<UserDefinedFeature*>* features )
{
   m_userDefinedFeatures = features;
}

const TileMapLayerInfoVector*
TileMapHandler::
getLayerIDsAndDescriptions() const
{
   if ( m_mapFormatDesc ) {
      return &(m_mapFormatDesc->getLayerIDsAndDescriptions());
   } else {
      return NULL;
   }
}
uint32
TileMapHandler::getDetailLevelForLayer(uint32 layerID) const
{
   return m_mapFormatDesc->getDetailLevelFromLayerID( 
                                          layerID, uint16(getPixelScale()) );
}


class TileCoordsXYHelper {
public:
   inline int32 getX(const TileMapCoord& coord) const {
      return coord.getLon();
   }
   
   inline int32 getY(const TileMapCoord& coord) const {
      return coord.getLat();
   }
   
};

void
TileMapHandler::requestRepaint()
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   // Check if the screen size has changed.
   isab::Rectangle curScreenSize;
   m_plotter->getMapSizePixels(curScreenSize);
   bool sizeChanged = curScreenSize != m_lastScreenSize;
   
   if ( sizeChanged ) {
      // Force repaint next time since the size can change back
      // again, but then the backbuffer might have been cleared.
      m_lastScreenSize = isab::Rectangle(0,0,0,0);
   }
   if ( !m_repaintTimerRequested ) {
      // XXX: If there is more than one
      mc2dbg << "[TMH]: Requested repaint timer" << endl;
      m_repaintTimerRequested =
         m_toolkit->requestTimer(this,
                                 20);
   }
}
   
PixelBox 
TileMapHandler::getMapSizePixels() const
{
   isab::Rectangle rect;
   m_plotter->getMapSizePixels( rect );
   return rect;
}

inline TileMap*
TileMapHandler::getMap(const MC2SimpleString& params) const
{
   return (*m_tileMapCont).getMap( params );
}

inline void
TileMapHandler::dumpMapSizes() const
{
#ifdef __unix__
//   (*m_tileMapCont).dumpMapSizes(); 
#endif
}

inline bool
TileMapHandler::haveMap(const MC2SimpleString& params) const
{
   return getMap(params);
}

void
TileMapHandler::dumpParams()
{
#ifdef __unix__
#endif
}


inline bool
TileMapHandler::tileMapReceived(const MC2SimpleString& descr,
                                BitBuffer* buffer,
                                vector<TileMapParams>& removeFromCache )
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   // Add the map using the parameters. Can be different
   // inside the map because of gzip.
   bool mapOK = false;
   bool cancelRequestedMaps = false;
   MapsToDrawIt it = (*m_tileMapCont).addMap( 
         descr, *buffer, removeFromCache, mapOK, cancelRequestedMaps );
   if ( ! mapOK ) {
      return false;
   }
   if ( cancelRequestedMaps ) {
      mc2log << "TMH: Calling cancelAll() since empty maps found "
             << "among the requested." << endl;
      m_requester->cancelAll();
      // Request repaint so that the maps are requested again.
      requestRepaint();
   }
   
   // Request another timer to repaint the stuff after everything
   // has arrived
   if ( m_repaintAfterMapReceptionTimer == 0) {
      m_repaintAfterMapReceptionTimer = m_toolkit->requestTimer(this, 1000);
   }

   if ( it != (*m_tileMapCont).endMapsToDraw() ) {
      m_textHandler->newMapArrived( it, 
                                    (*m_tileMapCont).beginMapsToDraw(),
                                    (*m_tileMapCont).endMapsToDraw() );
      if ( ! m_outlinesDrawnLastTime ) {
         // Do not do the needful until the outlines have been drawn.
         m_textHandlerIdleID = 0;
      }
   }
   m_mapArrivedSinceRepaint = true;

   // Check if this new map that arrived is a string map that was
   // requested by getNameForFeature. 
   if ( m_waitingForInfoString.second != NULL && 
        m_waitingForInfoString.first == descr ) {
      m_waitingForInfoString.second->notifyInfoNamesAvailable(); 
      // Clear the old one, 
      // i.e. we are not waiting for any strings now any more.
      m_waitingForInfoString.second = NULL;
   }

   return true;
}

MC2SimpleString
TileMapHandler::bitMapDescToBitMapName(const MC2SimpleString& desc)
{
   // Remove the leading 'B'
   // Create a temporary string to work with. It will be shorter than
   // the original.
   char* tmpStr = new char[desc.length()];
   // Go from after the B to the first '.'
   for( uint32 i = 1; i < desc.length(); ++i ) {
      tmpStr[i-1] = desc.c_str()[i];
      if ( desc.c_str()[i] == '.' ) {
         // Found the dot
         tmpStr[i-1] = '\0';
         break;
      }
   }
   return MC2SimpleStringNoCopy(tmpStr);
}

MC2SimpleString
TileMapHandler::bitMapNameToDesc( const MC2SimpleString& name )
{
   // Add B etc. Make room for the B and the name and a 10 char ext
   // (and a dot).
   char* tmpStr = new char[ 1 + name.length() + 10 + 1 + 1];
   sprintf(tmpStr, "%c%s%c%s",
           m_plotter->returnsVisibleSizeOfBitmaps() ? 'b' : 'B',
           name.c_str(), '.',
           m_plotter->getBitMapExtension());
   return MC2SimpleStringNoCopy(tmpStr);
}

bool
TileMapHandler::requestTextHandlerIdle()
{
   if ( m_textHandlerIdleID == 0 ) {
      if ( m_textHandler->needsToRun() ) {
         m_textHandlerIdleID = m_toolkit->requestIdle(this);
         return true;
      } else {
         return false;
      }
   }
   return true;
}

void
TileMapHandler::runIdleTask(uint32 id)
{
   //mc2dbg << "[TMH]: runIdleTask(" << id << ")" << endl;
   if ( id == m_textHandlerIdleID ) {
      mc2dbg2 << "[TMH]: Texthandler-> do stuff " << endl;
      m_textHandler->doTheNeedful();
      m_textHandlerIdleID = 0;
      // Request a new one if needed.
      if ( requestTextHandlerIdle() == false ) {
         if ( m_outlinesDrawnLastTime ) {
            // Only request repaint if the outlines have been
            // drawn. Else there will come a repaint.
            
            // Make the repaint method repaint everything correctly.
            m_detailRepaintTimerHasBeenRun = true;
            m_outlinesDrawnLastTime = false;
            requestRepaint();
         }
      }
   } 
}

void
TileMapHandler::timerExpired(uint32 id)
{
   //mc2dbg << "[TMH]: The timer " << id << " expired" << endl;
   if ( id == m_repaintTimerRequested ) {
      mc2dbg << "[TMH]: repaint timer" << endl;
      m_repaintTimerRequested = 0;
      repaint(20);
   } else if ( id == m_repaintAfterMapReceptionTimer ) {
      // We would like to request a repaint now. All maps may
      // be downloaded.
      m_repaintAfterMapReceptionTimer = 0;
      mc2dbg << "[TMH]: map reception timer" << endl;
      requestRepaint();
   } else if ( id == m_detailRepaintTimerRequested ) {
      
      m_detailRepaintTimerHasBeenRun = true;
      m_detailRepaintTimerRequested = 0;
      mc2dbg << "[TMH]: detail timer" << endl;
      // Do not request a repaint if the details have been drawn
      // already.
      if ( m_outlinesDrawnLastTime == false ) {
         requestRepaint();
      }
   } else if ( id == m_rerequestTimer ) {
      if ( m_rerequestTimerValue < c_maxRerequestTimerValue ) {
         // Multiply the timer value with 1.5
         m_rerequestTimerValue = m_rerequestTimerValue +
                                 (m_rerequestTimerValue >> 1);
         
         m_rerequestTimer =
            m_toolkit->requestTimer( this, m_rerequestTimerValue );
         mc2log << "[TMH]: re-requestTimer new timer ("
                <<  m_rerequestTimerValue << ")" << endl;
      } else {
         // Too long time. No more re-request timers
         m_rerequestTimer = 0;
         mc2log << "[TMH]: re-requestTimer - no new timers" << endl;
      }
      
      requestRepaint();
   }
}

void
TileMapHandler::setHighlight( bool onoff )
{
   m_showHighlight = onoff;
}

void
TileMapHandler::setHighlightPoint( const MC2Point& hightlightPoint )
{
   m_highlightPoint = hightlightPoint;
}

void
TileMapHandler::setTextSettings( const TileMapTextSettings& settings )
{
   m_textHandler->setTextSettings( settings );
}

void
TileMapHandler::sendEvent( const TileMapEvent& event )
{
   for ( vector<TileMapEventListener*>::iterator it = m_eventListeners.begin();
         it != m_eventListeners.end();
         ++it ) {
      (*it)->handleTileMapEvent( event );
   }
}

void
TileMapHandler::removeEventListener( TileMapEventListener* listener )
{
   for ( vector<TileMapEventListener*>::iterator it = m_eventListeners.begin();
         it != m_eventListeners.end(); ) {
      if ( *it == listener ) {
         it = m_eventListeners.erase(it);
      } else {
         ++it;
      }
   }
}

void
TileMapHandler::addEventListener( TileMapEventListener* listener )
{
   if ( listener == NULL ) {
      return;
   }
   removeEventListener( listener );
   m_eventListeners.push_back( listener );
}

void
TileMapHandler::dataBufferReceived(const MC2SimpleString& descr,
                                   BitBuffer* dataBuffer,
                                   bool requestMore)
{  
   // Request another timer to repaint the stuff after everything
   // has arrived
   if ( m_repaintAfterMapReceptionTimer == 0) {
      m_repaintAfterMapReceptionTimer = m_toolkit->requestTimer(this, 1000);
   }

   // Also request the re-request timer if the buffer is NULL
   // and the re-request timer is not running already.
   if ( dataBuffer == NULL && m_rerequestTimer == 0 ) {
      // Take the lowest value for the timer.
      m_rerequestTimerValue = c_minRerequestTimerValue;
      m_rerequestTimer =
         m_toolkit->requestTimer( this, m_rerequestTimerValue );
   }

  
   if ( dataBuffer == NULL ) {
      if ( requestMore ) {
         // Request max one new map or similar.
         // It might be time to wake up.
         requestMaps(1);
      }    
      // FIXME: Do something about it?
      return;
   }
 
   TileMapParamTypes::param_t paramType = 
            TileMapParamTypes::getParamType( descr.c_str() );
   vector<TileMapParams> removeFromCache;
   
   switch ( paramType ) {
      case TileMapParamTypes::TILE:
      { 
         // Map coming!
         if ( m_mapFormatDesc != NULL ) {
            m_garbage.checkGarbage();
            if ( ! tileMapReceived(descr, dataBuffer, removeFromCache) ) {
               mc2log << "TMH: Tilemap could not be decoded. "
                      << "Requesting new tmfd." << endl;
               // The tilemap could not be decoded. Request a new mapdesc.
               // Request the real new one
               // Create new random characters
               m_randChars = 
                  MC2SimpleStringNoCopy(RandomFuncs::newRandString(3));
               requestFromCacheOrForReal(
                     TileMapFormatDesc::createParamString( m_lang,
                                               m_toolkit->getIDString(),
                                               m_randChars.c_str() ) );
            
            }
            m_garbage.checkGarbage();
         }
      } break;

      case TileMapParamTypes::BITMAP: {
         // Bitmap. Type not used.
         isab::BitMap* bmp =
            m_plotter->createBitMap( isab::bitMapType(0),
                                     dataBuffer->getBufferAddress(),
                                     dataBuffer->getBufferSize(),
                                     getDPICorrectionFactor() );
         if ( bmp != NULL ) {
            // Store the bitmap. Remove the 'B'
            MC2SimpleString bmpName( bitMapDescToBitMapName( descr ) );
            // Delete old bitmap. (Default should be NULL here, I hope).
            m_plotter->deleteBitMap(m_bitMaps[bmpName]);
            // Set new bitmap.
            m_bitMaps[bmpName] = bmp;
            m_neededBitMaps.erase(bmpName);
         }
      }                                     
      break;

      case TileMapParamTypes::FORMAT_DESC_CRC : {
         // CRC
         TileMapFormatDescCRC crcObj;
         crcObj.load( *dataBuffer );
         m_serverDescCRC = crcObj.getCRC();
         m_descCRCReceived = true;
         mc2log << "[TMH]: CRC buffer received. CRC = "
                << MC2HEX(m_serverDescCRC) << endl;
      } break;
      
      case TileMapParamTypes::FORMAT_DESC : {
         // Description coming!
         // Only exchange the desc if it is the real one or we don't have
         // any already.
         // Load the description
         TileMapFormatDesc* tmpNewDesc = new TileMapFormatDesc();
         // Load it. Transfer stuff from the old one.            
         tmpNewDesc->load( *dataBuffer, m_mapFormatDesc );
         mc2log << "[TMH]: TMFD: timeStamp (" << descr
                << ")" 
                << tmpNewDesc->getTimeStamp() << endl;
         // Use the new desc if it is newer or we don't have one
         // already.
         if ( ( m_mapFormatDesc == NULL  ) ||
              ( m_mapFormatDesc->getTimeStamp() <=
                tmpNewDesc->getTimeStamp() ) ) {
            // Cannot delete the format description now,
            // there may be references
            if ( m_mapFormatDesc ) {
               m_oldFormatDescs.push_back( m_mapFormatDesc );
            }
            mc2log << "[TMH]: TMFD is set to " << descr << endl;
            m_mapFormatDesc = tmpNewDesc;
            m_mapFormatDesc->updateLayersToDisplay( m_layersToDisplay );
            // Real repaint needed
            m_matrixChangedSinceDraw = true;
            (*m_tileMapCont).setMapDesc( m_mapFormatDesc );
            sendEvent ( TileMapEvent(
                           TileMapEvent::NEW_CATEGORIES_AVAILABLE ) );
            // Save the currently used desc
            delete m_dxxBuffer;
            m_dxxBuffer = new BitBuffer( *dataBuffer );
            
            // Copy the description to another buffer and save it as "DXXX".
            m_requester->release("DXXX", new BitBuffer( *dataBuffer ) );
            // Copy the description to another buffer and save it as "DYYY".
            m_requester->release("DYYY", new BitBuffer( *dataBuffer ) ); 

            // Make it ok to request a new format desc if 
            // e.g. the language is wrong.
            if ( descr != "DXXX" && descr != "DYYY" ) {
               // Temporary until the real crc is implemented
               m_randChars = 
                  MC2SimpleStringNoCopy(RandomFuncs::newRandString(3));
            }
            
         } else {
            // The format desc was no good.
            delete tmpNewDesc;
         }
         updateParams();
      } break;
      case TileMapParamTypes::UNKNOWN: {
         MC2_ASSERT(false);
         break;
      }
   }
   
   if ( removeFromCache.empty() ) {
         // We will not use the BitBuffer anymore. Add to cache.
      m_requester->release(descr, dataBuffer);
   } else {
      // Don't add the buffer to the cache. Delete instead.
      mc2log << "TMH: Remove from cache:" << endl;
      delete dataBuffer;
      for ( uint32 i = 0; i < removeFromCache.size(); ++i ) {
         mc2log << "TMH: Removing " << removeFromCache[ i ] 
                << " from cache." << endl;
         m_requester->removeBuffer( removeFromCache[ i ] );
      }
   }
   
   if ( requestMore ) {
      // Request max one new map or similar.
      requestMaps(1);
   }
}

bool
TileMapHandler::requestFromCacheAndCreate(const MC2SimpleString& desc)
{
   BitBuffer* bitBuf = m_requester->requestCached(desc);
   if ( bitBuf ) {     
      dataBufferReceived(desc, bitBuf, false);     
   }
   return bitBuf != NULL;
}

bool
TileMapHandler::requestFromCacheOrForReal(const MC2SimpleString& desc,
                                 const DBufRequester::request_t& reqType )
{
   if ( ! requestFromCacheAndCreate( desc ) ) {
      // Request for real.
      m_requester->request( desc,
                            m_dataBufferListener,
                            reqType );
      return false; // Could not get it from cache.
   } else {
      return true;  // Got it from cache.
   }
}

void
TileMapHandler::requestMaps(int maxNbr)
{
   // Counter for the maxNbr
   int nbrReq = 0;

   // Request the missing format description.
   if ( ( m_mapFormatDesc == NULL ) || ( m_descCRCReceived == false ) ||
        ( m_mapFormatDesc->getCRC() != m_serverDescCRC ) ||
        ( !m_mapFormatDesc->okLanguage( m_lang ) ) ) {
      // Request the real new one
      // Don't create new random characters unless the language is wrong
      if ( m_mapFormatDesc != NULL ) {
         // Don't create new random characters at all.
         //m_randChars = MC2SimpleStringNoCopy(RandomFuncs::newRandString(3));
      }
      if ( m_descCRCReceived == false || ( m_mapFormatDesc && m_mapFormatDesc->getCRC() != m_serverDescCRC ) ) {
         requestFromCacheOrForReal(
            TileMapFormatDescCRC::createParamString( m_lang,
                                                     m_toolkit->getIDString(),
                                                     m_randChars.c_str() ) );
      }

      if ( m_mapFormatDesc != NULL ) {
         if ( m_descCRCReceived ) {
            // onlt 
            if ( ( m_mapFormatDesc->getCRC() != m_serverDescCRC ) ||
                 ( !m_mapFormatDesc->okLanguage( m_lang ) ) ) {
               mc2log << "[TMH]: Wrong lang in TMFD "
                      << int(m_mapFormatDesc->getLanguage()) << " wanted "
                      << int(m_lang) << " or wrong CRC "
                      <<  m_mapFormatDesc->getCRC() << " wanted "
                      <<  m_serverDescCRC << endl;
               // The CRC was wrong
               requestFromCacheOrForReal(
                  TileMapFormatDesc::createParamString(
                     m_lang,
                     m_toolkit->getIDString(),
                     m_randChars.c_str() ) );
            }
         }
      }
            
      if ( m_mapFormatDesc == NULL ) {
         // Also request the previously saved one
         requestFromCacheOrForReal( "DXXX" );
         // Also request one from the readonly cache too.
         requestFromCacheOrForReal( "DYYY" );
      }
   }

   // Print the maps to request
   
   
   
   // Request the missing maps.
   {
      //(*m_tileMapCont).m_mapsToReq.dump();      
      PIt pIt = (*m_tileMapCont).beginParamsToReq();
      for ( MIt mIt = (*m_tileMapCont).beginMapsToReq(); 
            mIt != (*m_tileMapCont).endMapsToReq(); ++pIt, ++mIt ) {
         // WARNING: The requester can call dataBufferReceived directly
         // here if no threads are present.
         DBufRequester::request_t requestType = DBufRequester::cacheOrInternet;

         if ( (*m_tileMapCont).toRequestMap( mIt, requestType ) ) {
            const TileMapParams* params = &(*pIt);
            // FIXME: Rotate the requested params to get more requests.
            if ( nbrReq < maxNbr ) {
               bool cached = requestFromCacheOrForReal( *params, 
                                                        requestType );
               if ( ! cached ) {
                  ++nbrReq;
               }   
            } else {
               // Only cached maps are allowed now.               
               requestFromCacheAndCreate( *params );
            }
         }         
      }
   }   
   // Request the reserve maps.
   {
      
      PIt pIt = (*m_tileMapCont).beginReserveParamsToReq();
      for ( MIt mIt = (*m_tileMapCont).beginReserveMapsToReq(); 
            mIt != (*m_tileMapCont).endReserveMapsToReq(); ++pIt, ++mIt ) {
         // WARNING: The requester can call dataBufferReceived directly
         // here if no threads are present.
         const TileMap* curMap = *mIt;
         if ( curMap == NULL ) {
            const TileMapParams* params = &(*pIt);
            mc2dbg8 << "Requested param " << params->getAsString()
               << endl;
            // FIXME: Rotate the requested params to get more requests.
            if ( nbrReq < maxNbr ) {
               bool cached = requestFromCacheOrForReal( *params );
               if ( ! cached ) {
                  ++nbrReq;
               }   
            } else {
               // Only cached maps are allowed now.               
               requestFromCacheAndCreate( *params );
            }
         }         
      }
   }
   // Request the missing bitmaps.
   // Copy the set so that there will not be trouble if it was cached.
   set<MC2SimpleString> tmpNeeded ( m_neededBitMaps );
   for ( set<MC2SimpleString>::const_iterator it = tmpNeeded.begin();
         it != tmpNeeded.end(); ++it ) {
      if ( m_bitMaps.find( *it ) == m_bitMaps.end() ) {
         // Make temp string
         bool cached = requestFromCacheOrForReal(
            bitMapNameToDesc(*it) );
         if ( ! cached ) {
            if ( nbrReq++ > maxNbr ) {
               // No more requests allowed.
               return;
            }       
         }
      } else {
         // Already there. An error really.
         m_neededBitMaps.erase( *it );
      }
   }
}

void
TileMapHandler::repaint(uint32 /*maxTimeMillis*/)
{
   if ( m_painting ) {
      // Already painting.
      return;
   }


   // FIXME: Adapt this time to the real time.
#ifdef THIS_CODE_IS_OMITTED_FOR_TESTING
   const int32 minPaintTime = 1*20;
   const int32 timeDiff = TileMapUtil::currentTimeMillis() -
                               m_lastRepaintTime;

   mc2dbg << "[TMH]: Timediff = " << timeDiff << endl;
   
   if ( timeDiff < minPaintTime ) {
      if ( !m_repaintTimerRequested ) {
      // XXX: If there is more than one
         mc2dbg << "[TMH]: Requested repaint timer" << endl;
         m_repaintTimerRequested =
            m_toolkit->requestTimer(this,
                                    minPaintTime - timeDiff);
      }
      mc2dbg << "[TMH]: No repaint. Too little diff" << endl;
      return;
   }
#endif  // THIS_CODE_IS_OMITTED_FOR_TESTING
   
   // We are painting
   ++m_painting;


   // FIXME on another monday
   // FIXME: 1. Paint until time expires importance levels -> paint bitmap
   // FIXME: 2. At every importance level -> update bitmap.
   // FIXME: 3. Save where we were.

   {
      isab::Rectangle curScreenSize;
      m_plotter->getMapSizePixels(curScreenSize);
      setScreenSize(MC2Point(curScreenSize.getWidth(),
                             curScreenSize.getHeight()) );
      if ( m_lastScreenSize != curScreenSize ) {
         m_matrixChangedSinceDraw = true;
      }
      m_lastScreenSize = curScreenSize;
   }
   
   // FIXME: Move this to the correct place. Where?
   requestMaps(10);
   
   // FIXME: Better way to determine which requests to send.

   // Check if there is a complete level.
   plotWhatWeGot( );
   
   // We are (maybe) not painting (we could have gotten here recursively).
   --m_painting;

   // Check if we need to repaint again. (E.g. a new, better map may have
   // arrived).
   m_lastRepaintTime = TileMapUtil::currentTimeMillis();
   
   //(*m_tileMapCont).m_mapsToReq.dump();
   m_mapArrivedSinceRepaint = false;
}

inline bool
TileMapHandler::coordsInside(const CoordsArg& /*coords*/,
                             const MC2BoundingBox& /*bbox*/)
{
   // This is now checked in the loop when drawing polygons or polylines
   return true;
}


uint32
TileMapHandler::getTotalMapSize() const
{
   uint32 totSize = 0;
#ifdef __unix__
   totSize = (*m_tileMapCont).getTotalMapSize();
#endif
   return totSize;
}

inline bool
TileMapHandler::
getXYFromCoordArg( const TilePrimitiveFeature& primWithCoordArg,
                   const MC2BoundingBox& bbox,
                   int& x,
                   int& y ) const
{
   // FIXME: This may mean that the feature will not be plotted
   // if the center coordinate is outside the map.
   const TileFeatureArg* arg = primWithCoordArg.getArg(TileArgNames::coord);
   const CoordArg* coordArg = static_cast<const CoordArg*>(arg);
   if ( arg == NULL ) {
      return false;
   }
   if ( bbox.inside( coordArg->getCoord().getLat(),
                     coordArg->getCoord().getLon() ) ) {
      transformPointInternalCosLat( x, y,
                                    coordArg->getCoord() );
      return true;
   }
   return false;
}

inline int
TileMapHandler::plotCircleFeature( TilePrimitiveFeature& prim,
                                   const MC2BoundingBox& bbox,
                                   int /*pass*/,
                                   int /*moving*/ )
{
   // Must have a coord arg.
   int x;
   int y;
   if ( ! getXYFromCoordArg( prim, bbox, x, y ) ) {
      return 0;
   }
   const SimpleArg* radiusArg =
      static_cast<const SimpleArg*>(prim.getArg(TileArgNames::radius));
   if ( radiusArg == NULL ) {
      return 0;
   }
   const SimpleArg* colorArg =
      static_cast<const SimpleArg*>(prim.getArg(TileArgNames::color));
   if ( colorArg ) {
      m_plotter->setPenColor( colorArg->getValue() );
   } else {
      return 0;
   }

   uint32 radius = radiusArg->getValue();
   uint32 halfRadius = radius >> 1;
   m_plotter->drawArc( true, isab::Rectangle(x - halfRadius, y - halfRadius,
                                       radius, radius), 0, 360);
   
   return 1;
}

isab::BitMap*
TileMapHandler::getOrRequestBitMap( const MC2SimpleString& bitMapName )
{
   bitMapMap_t::const_iterator it = m_bitMaps.find( bitMapName );
   if ( it != m_bitMaps.end() ) {
      return it->second;
   } else {
      // Will be requested later.
      m_neededBitMaps.insert( bitMapName );
   }
   return NULL;
}

inline bool 
TileMapHandler::forbiddenFeature( const TileMap& curMap,
                                  const TilePrimitiveFeature& prim ) const 
{
   // Check if it is in a forbidden category
   const TileFeature* parent = curMap.getParentFeature(&prim);

   if ( parent != NULL &&
        ( m_disabledParentTypes.find(parent->getType()) !=
          m_disabledParentTypes.end() ) ) {
      // Yep, among the forbidden ones.
      return true;
   }
   // Not forbidden.
   return false;
}

inline int
TileMapHandler::plotBitMapFeature( TileMap& curMap,
                                   TilePrimitiveFeature& prim,
                                   const MC2BoundingBox& bbox,
                                   int pass,
                                   int /*moving*/ )
{
   // Should not be drawn on pass 0
   if ( pass == 0 ) {
      return 0;
   }
   
   // Check if it is in a forbidden category
   if ( forbiddenFeature( curMap, prim ) ) {
      return 0;
   }

   // Must have a coord arg.
   MC2Point center(0,0);
   if ( ! getXYFromCoordArg( prim, bbox, center.getX(), center.getY() ) ) {
      return 0;
   }
   
   const StringArg* bitMapName =
      static_cast<const StringArg*>(prim.getArg(TileArgNames::image_name));

   if ( bitMapName == NULL ) {
      return 0;
   }

   // FIXME: Use getOrRequestBitMap instead
   BitMap* curBitMap = getOrRequestBitMap( bitMapName->getValue() );
   if ( curBitMap != NULL ) {      
      // Fetch bitmap size from the plotter.
      isab::Rectangle rect;
      m_plotter->getBitMapAsRectangle( rect, center, curBitMap );
      PixelBox pixelBox = rect;
      
      // Debug
#if 0
      m_plotter->setPenColor(0xffff00);
      m_plotter->drawRect(false,
                          rect);
#endif 
      
      if ( m_overlapDetector.addIfNotOverlapping( pixelBox ) ) {
         // Set coordinate
         m_screenCoords.push_back(center);         
         m_plotter->drawBitMap( m_screenCoords.back(), curBitMap );
      } else {
         return 0;
      }
   } else {
      // We will fetch it some other time
      return 0;
   }

   return 1; // Number of coords plotted.
}

int
TileMapHandler::plotBBox( const MC2BoundingBox& bbox,
                          const MC2BoundingBox& /*screenBBox*/ )
{ 
//   if( ! screenBBox.overlaps( bbox ) ) {
//       return 0;
//    }
   vector<MC2Coordinate> coords;
   coords.push_back( MC2Coordinate(bbox.getMinLat(), bbox.getMinLon()) );
   coords.push_back( MC2Coordinate(bbox.getMinLat(), bbox.getMaxLon()) );
   coords.push_back( MC2Coordinate(bbox.getMaxLat(), bbox.getMaxLon()) );
   coords.push_back( MC2Coordinate(bbox.getMaxLat(), bbox.getMinLon()) );
   coords.push_back( MC2Coordinate(bbox.getMinLat(), bbox.getMinLon()) );
   vector<MC2Point> points;
   points.reserve( coords.size() );
   MC2Point curPoint(0,0);
   vector<MC2Coordinate>::const_iterator the_end ( coords.end() );
   for ( vector<MC2Coordinate>::const_iterator it ( coords.begin() );
         it != the_end;
         ++it ) {
      transformPointInternalCosLat( curPoint.getX(),
                                    curPoint.getY(),
                                    *it );
      points.push_back(curPoint);
   }
   m_plotter->drawPolyLine( points.begin(), points.end() );
   return points.size();
}

inline int
TileMapHandler::plotPolyLines(const TilePrimitiveFeature& /*feat*/,
                              uint32 color,
                              int lineWidth )
{
   m_plotter->drawPolyLineWithColor( m_screenCoords.begin(),
                                     m_screenCoords.end(), 
                                     color,
                                     lineWidth );
   return m_screenCoords.size();
}

inline int
TileMapHandler::plotPolygon( const TilePrimitiveFeature& /*feat*/,
                             uint32 colorVal )
{
   m_plotter->drawPolygonWithColor( m_screenCoords.begin(),
                                    m_screenCoords.end(), 
                                    colorVal );
   return m_screenCoords.size();
}

inline uint32
TileMapHandler::getPolylineInnerPixelWidth( 
                              const TilePrimitiveFeature& prim ) const
{
   MC2_ASSERT( prim.getType() == FeatureType::line );
   uint32 widthPixels = 1;
      
   // Get the width argument given in pixels
   uint32 tmpWidthPixels = MAX_UINT32;
   const SimpleArg* widthArg = 
      static_cast<const SimpleArg*>((prim.getArg(TileArgNames::width)));
   if ( widthArg != NULL ) {
      tmpWidthPixels = widthArg->getValue( m_scaleIndex );
   }

   // Check for meters arg first, since we must send the
   // old arg for the old clients.
   const SimpleArg* widthMetersArg =
      static_cast<const SimpleArg*>(
            (prim.getArg(TileArgNames::width_meters)));

   bool useMeters = false;
   if ( widthMetersArg != NULL ) {
      uint32 tmpWidthMeters = widthMetersArg->getValue( m_scaleIndex );
      if ( ( tmpWidthMeters & MAX_UINT8 ) != MAX_UINT8 ) {
         useMeters = true;
         widthPixels = uint32(tmpWidthMeters / getDPICorrectedScale());
         // Make sure the width is at least the normal width argument
         if ( ( tmpWidthPixels & MAX_UINT8 ) != MAX_UINT8 ) {
            widthPixels = MAX(widthPixels, tmpWidthPixels);
         }
         if ( widthPixels == 0 ) {
            // Must have odd width or more than zero. But zero is even
            // so it will also get here.
            ++widthPixels;
         }
      }
   }
   if ( ! useMeters ) {
      if ( ( tmpWidthPixels & MAX_UINT8 ) != MAX_UINT8 ) {
         widthPixels = tmpWidthPixels;
      }
   }
  
   widthPixels = widthPixels * getDPICorrectionFactor();
   
   return widthPixels;
}

uint32
TileMapHandler::getPolylineOuterPixelWidth( 
                              const TilePrimitiveFeature& prim ) const
{
   uint32 width = getPolylineInnerPixelWidth( prim );
   // Outline
   const SimpleArg* borderColorArg = 
      static_cast<const SimpleArg*>(
            (prim.getArg(TileArgNames::border_color)));
   if ( borderColorArg != NULL ) {
      uint32 borderColor = 
         borderColorArg->getValue( m_scaleIndex );
      if ( VALID_TILE_COLOR( borderColor ) ) {
         width += 2;
      }
   }
   return width;   
}

inline int
TileMapHandler::plotPolygonOrLineFeature( const TilePrimitiveFeature& prim,
                                          int pass,
                                          int moving )
{
   int nbrDrawn = 0;
   //MC2_ASSERT( int(curPrim->getType()) < 0 );
   
   const SimpleArg* color = static_cast<const SimpleArg*>(
                                         (prim.getArg(TileArgNames::color)));
      
   if ( prim.getType() == FeatureType::polygon ) {
      mc2dbg8 << "[TMH]: Filling " << endl;
      if ( color && ( pass == 0 ) ) {
         nbrDrawn += plotPolygon( prim, color->getValue( m_scaleIndex ) );
      }
   } else {
      uint32 widthPixels = getPolylineInnerPixelWidth( prim );
      if ( color ) {
         if ( pass == 1 ) {
            // Inside
            nbrDrawn += plotPolyLines(prim,
                                      color->getValue( m_scaleIndex ),
                                      widthPixels );
         } else {
            // FIXME: Remove this when streets are sent
            //        correctly.
            if ( ( pass == 0 ) && ( moving == 0 ) ) {
               // Outline
               const SimpleArg* borderColorArg = 
                  static_cast<const SimpleArg*>(
                                (prim.getArg(TileArgNames::border_color)));
               if ( borderColorArg != NULL ) {
                  uint32 borderColor = 
                     borderColorArg->getValue( m_scaleIndex );
                  if ( VALID_TILE_COLOR( borderColor ) ) {
                     nbrDrawn += plotPolyLines(prim, borderColor,
                                               widthPixels + 2 );
                  }
               }
            }
         }
      }
   }
   return nbrDrawn;
}

inline int
TileMapHandler::checkScale( const TilePrimitiveFeature& feature ) const
{
   const TileFeatureArg* maxScale = feature.getArg( TileArgNames::max_scale );
   if ( maxScale != NULL ) {
      if ( TileFeatureArg::simpleArgCast( maxScale )->getValue() <= 
           getDPICorrectedScale() ) {
         return false;
      } else {
         return true;
      }
   }
   return true;
}

inline void
TileMapHandler::clearCoords( TilePrimitiveFeature& feat )
{
   feat.setDrawn( false );
   m_screenCoords.clear();
}

inline int
TileMapHandler::prepareCoordinates( TilePrimitiveFeature& feat,
                                    const MC2BoundingBox& bbox)
{
   // Get the coords.
   const CoordsArg* coords = static_cast<const CoordsArg*>(
      feat.getArg(TileArgNames::coords));

   if ( coords == NULL ) {
      return 0;
   }
 
   if ( !bbox.overlaps(coords->getBBox() ) ) {
      // Outside completely. And there should be no more coordinates.
      // FIXME: Maybe we could re-use them?
      return 0;
   }

#if 0
   m_plotter->setLineWidth( 1 );
   m_plotter->setPenColor( 0 );
   plotBBox( coords->getBBox(), bbox );
#endif
   
   // For now we will use the same amount of points as in the
   // pointvector. Could be different when clipping etc is implemented.
   MC2Point curPoint(0,0);
   // Version with pre-calculated cosLat.
   // The size of the VectorProxy should be the same as the size of
   // the coords vector, or else we're ...
  
   MC2Point lastPoint( MAX_INT32, MAX_INT32 );
   
   uint32 nbrCoords = coords->end() - coords->begin();
   
   m_realScreenCoords.reserve( nbrCoords );
   vector<MC2Coordinate> clipCoords;

   if ( feat.getType() == FeatureType::polygon &&
        ClipUtil::clipPolyToBBoxFast( bbox,
                                    clipCoords,
                                    &(*coords->begin()), &(*coords->end()))) { 
      
      m_realScreenCoords.reserve( clipCoords.size() );
      for ( vector<MC2Coordinate>::const_iterator it = clipCoords.begin();
            it != clipCoords.end();
            ++it ) {
         transformPointInternalCosLat( curPoint.getX(),
                                       curPoint.getY(),
                                       *it );
         if ( curPoint != lastPoint ) {
            m_screenCoords.push_back(curPoint);
            lastPoint = curPoint;
         }
      }
   } else {
      // Don't clip.
      CoordsArg::const_iterator the_end ( coords->end() );
      for ( CoordsArg::const_iterator it ( coords->begin() );
            it != the_end;
            ++it ) {
         transformPointInternalCosLat( curPoint.getX(),
                                       curPoint.getY(),
                                       *it );
         if ( curPoint != lastPoint ) {
            m_screenCoords.push_back(curPoint);
            lastPoint = curPoint;
         }
      }
   }
   
   return m_screenCoords.size();
}


inline int
TileMapHandler::plotPrimitive( TileMap& curMap,
                               TilePrimitiveFeature& curPrim,
                               const MC2BoundingBox& bbox,
                               int pass,
                               int moving )
{
   int curNbrDrawn = 0;

   switch ( curPrim.getType() ) {
      case FeatureType::line:
      case FeatureType::polygon:
         if ( prepareCoordinates(curPrim, bbox) == 0 ) {
            return 0;
         }
         break;
   }
   
   switch ( curPrim.getType() ) {
      case FeatureType::line:
      case FeatureType::polygon:
         curNbrDrawn =
            plotPolygonOrLineFeature( curPrim, pass, moving );
         break;
      case FeatureType::bitmap:
         if ( ! checkScale( curPrim ) ) {
            return 0;
         }
         curNbrDrawn = plotBitMapFeature( curMap, curPrim, bbox,
                                          pass, moving  );
         break;
      case FeatureType::circle:
         if ( pass == 1 ) {
            curNbrDrawn += plotCircleFeature( curPrim, bbox,
                                              pass, moving );
         }
      default:
         // Unknown stuff.
         break;
   }
   // Mark if the primitive was drawn.
   curPrim.setDrawn( curNbrDrawn > 0 );



   return curNbrDrawn;
}

void
TileMapHandler::drawProgressIndicator()
{
   // Only draw progress indicator if we have a valid pixelbox for the
   // string.
   if ( ! m_progressIndicatorBox.isValid() ) {
      return;
   }
   
   // How many percent of the maps do we have?
   uint32 percentMapsReceived = (*m_tileMapCont).getPercentageMapsReceived();
   
   // Don't draw any progress indicator if everything is completed.
   if ( percentMapsReceived >= 100 ) {
      MC2_ASSERT( percentMapsReceived == 100 );
      return;
   }
      
   // Get font name. 
   const STRING* fontName = m_textHandler->getPlotterStr( 
         m_textHandler->getTextSettings().
            getProgressIndicatorFont().first );
   // And font size.
   int fontSize = 
      m_textHandler->getTextSettings().getProgressIndicatorFont().second;

   m_plotter->setFont( *fontName, fontSize );
   
   char tmpStr[40];
   const char percent = '%';
#ifdef __unix__
   sprintf( tmpStr, "%u%c %u/%u", 
         (unsigned int) percentMapsReceived, percent,
         (unsigned int) (*m_tileMapCont).m_mapsToReqCount.first, 
         (unsigned int) (*m_tileMapCont).m_mapsToReqCount.second );
#else
   sprintf( tmpStr, "%u%c",  
         (unsigned int) percentMapsReceived, percent );
#endif
   STRING* str = m_plotter->createString( tmpStr );
   
#if 0   
   m_plotter->setLineWidth( 1 );
   m_plotter->setFillColor( 231, 221, 187 );
   m_plotter->setPenColor( 231, 221, 187 );
   m_plotter->drawRect( true, m_progressIndicatorBox );
#endif 

   m_plotter->setPenColor(0,0,0);
   m_plotter->drawText( *str, m_progressIndicatorBox.getCenterPoint() );
   
   m_plotter->deleteString( str );
}

void 
TileMapHandler::plotCopyrightString() 
{
   if ( m_mapFormatDesc == NULL ) {
      return;
   }
   if ( ! m_showCopyright ) {
      return;
   }
   
   // Get font name. 
   const STRING* fontName = m_textHandler->getPlotterStr( 
         m_textHandler->getTextSettings().
            getCopyrightFont().first );
   // And font size.
   int fontSize = 
      m_textHandler->getTextSettings().getCopyrightFont().second;

   m_plotter->setFont( *fontName, fontSize );

   m_plotter->setPenColor(0,0,0);
   STRING* str = m_plotter->createString( 
         m_mapFormatDesc->getCopyrightString( getBoundingBox() ).c_str() );
   
   m_plotter->drawTextAtBaselineLeft( *str, m_copyrightPos );
   m_plotter->deleteString( str );
}

class ColorDispenser {
public:
   ColorDispenser() {
      m_allocator = 0;
      setupColors();
   }

   int getColor(const char* id) {
      if(m_colorMap.end() == m_colorMap.find(id)) {
         int index = m_allocator;
         m_colorMap.insert(make_pair(id, m_colors[index]));
         m_allocator = (m_allocator + 1) % m_colors.size();
      }

      return m_colorMap[id];
   }

   static uint32 translate(int red, int green, int blue) {
      return (red << 16) | (green << 8) | (blue << 0);
   }
   
private:
   void setupColors() {
      int steps = 4;
      int step = 255 / steps;

      for(int r = 2; r<=steps; r++) {
         for(int g = 2; g<=steps; g++) {
            for(int b = 2; b<=steps; b++) {
               m_colors.push_back( translate(r * step, 
                                             g * step, 
                                             b * step) );
            }
         }
      }
      std::random_shuffle(m_colors.begin(), m_colors.end());
   }
  

   unsigned int                  m_allocator;
   std::vector<unsigned int>     m_colors;
   std::map<MC2SimpleString, unsigned int>   m_colorMap;


};

inline void
TileMapHandler::plotWhatWeGotInBBox( const MC2BoundingBox& bbox,
                                     int skipOutlines,
                                     int& nbrFeatures,
                                     int& nbrDrawn )
{
   
   if ( m_mapFormatDesc == NULL || !m_isDrawing) {
      return;
   }
        
   m_plotter->setLineWidth( 1 );
   
   m_plotter->setFillColor(255, 0, 0);
   m_plotter->setPenColor(0, 0, 0);

   // Divide the bbox by two to check the removal of features outside
   // the bbox.
   //bbox *= 0.5;
   
   // Paint what we've got.
   
   
   MapsToDrawIt endIt = (*m_tileMapCont).endMapsToDraw();
   for( int pass = 0; pass < 2; ++pass ) {
      // FIXME: The pass needs fixing. Check it before converting points.
      

      for( int level = m_mapFormatDesc->getMinLevel();
           level <= m_mapFormatDesc->getMaxLevel();
           ++level ) {

         for( MapsToDrawIt mIt = (*m_tileMapCont).beginMapsToDraw();
              mIt != endIt; ++mIt ) {
            TileMap* tileMap = *mIt;
         
            if ( tileMap == NULL ) {
               continue;
            }
              TileMap::primVect_t& prims =
               tileMap->getPrimitives(*m_mapFormatDesc,
                                      level);

            TileMap::primVect_t::iterator theEnd = prims.end();
            for ( TileMap::primVect_t::iterator it = prims.begin();
                  it != theEnd;
                  ++it ) {
#ifdef __unix__ //then we draw them later
               if(it->getType() == FeatureType::bitmap)
                  continue;
#endif               
               // FIXME: Remove this and check if it needs updating.
               clearCoords( *it );
               int curNbrDrawn = plotPrimitive(*tileMap,
                                               *it, bbox, pass,
                                               skipOutlines );
               if ( curNbrDrawn > 0 ) {
                  ++nbrFeatures;
               }

               nbrDrawn += curNbrDrawn;
            }
         }
      }
      mc2dbg8 << "[TMH]: Level done. Number features "
         << nbrFeatures << ", Number of points: " << nbrDrawn << endl;

   }
   
   #ifndef __unix__
      return;
   #endif
   

#ifdef __unix__
   //Do debug-drawing of roads...
   for(int pass = 0; pass<2; pass++) {
      for( int level = m_mapFormatDesc->getMaxLevel();
           level >= m_mapFormatDesc->getMinLevel();
           --level ) {
         for(MapsToDrawIt it = (*m_tileMapCont).beginMapsToDraw();
             it != endIt; ++it ) {
            TileMap* curMap = *it;
            if ( curMap == NULL ) {
               continue;
            }

            if(   curMap->getParams()->getLayer() != TileMapTypes::c_mapLayer) {
               //   continue;
            }

            TileMap::primVect_t& prims =
               curMap->getPrimitives(*m_mapFormatDesc,
                                     level);
               
            TileMap::primVect_t::iterator theEnd = prims.end();

            for ( TileMap::primVect_t::iterator it = prims.begin();
                  it != theEnd;
                  ++it ) 
            {
               if(it->getType() == FeatureType::line && pass==0) {
                  clearCoords( *it );
                  plotDebugRoad(*curMap, *it, bbox);
               } else if(it->getType() == FeatureType::bitmap && pass==1) {
                  clearCoords( *it );
                  plotPrimitive(*curMap,
                                *it, bbox, 1,
                                false );                     
               }
            }
         }
      }
   }
   //Print crossings
      m_plotter->setPenColor(0, 255, 0);
      vector<MC2Point> tempVector;

      for(  vector<vector<MC2Coordinate> >::iterator 
            curCrossingVec =  m_detectedPoints.begin();
            curCrossingVec != m_detectedPoints.end();
            curCrossingVec++)
      {
         for(  vector<MC2Coordinate>::iterator 
               curCrossing = curCrossingVec->begin();
               curCrossing!= curCrossingVec->end();
               curCrossing++) 
         {
            MC2Point curPoint(0, 0);
            transformPointInternalCosLat( curPoint.getX(),
                                          curPoint.getY(),
                                          *curCrossing );
            tempVector.push_back(curPoint);
         }
         m_plotter->drawPolyLineWithColor(   tempVector.begin(),
                                 tempVector.end(),
                                 ColorDispenser::translate(0, 0, 0),
                                 6);

         m_plotter->drawPolyLineWithColor(   tempVector.begin(),
                                 tempVector.end(),
                                 ColorDispenser::translate(0, 255, 0),
                                 4);

         tempVector.clear();
      }
#endif
}

#ifdef __unix__

ColorDispenser colors;

void 
TileMapHandler::plotDebugRoad(   TileMap& curMap,
                                 TilePrimitiveFeature& prim,
                                 const MC2BoundingBox& bbox )
{



   const char* name = 
      getNameForFeature( curMap, prim, NULL );
   
//   if(!name)
   //    return;

   
   bool isSelected = false;
   if(name) {
      MC2SimpleString compString(name);
      isSelected = !compString.empty() && compString == m_selectedRoad;
   }

   if ( prepareCoordinates(prim, bbox) == 0 ) {
      return;
   }
   
   const SimpleArg* color = static_cast<const SimpleArg*>(
                                         (prim.getArg(TileArgNames::color)));
      
   uint32 widthPixels = getPolylineInnerPixelWidth( prim ) / 4;

   if (!color ) {
      return;
   }


   uint32 debugColor;
   if(name)
      debugColor = colors.getColor(name);
   else
      debugColor = 0;
   
   int red =      (debugColor >> 16) & 0xff;
   int green =    (debugColor >>  8) & 0xff;
   int blue =     (debugColor >>  0) & 0xff;

   int outerColor = colors.translate(100, 100, 100);
   if(isSelected)
      outerColor = colors.translate(0, 0, 0);

   int outerWidth = widthPixels + 2;
   if(isSelected)
      outerWidth+=2;
   
   plotPolyLines(prim, outerColor, outerWidth);
   plotPolyLines(prim, debugColor, widthPixels);
   m_plotter->setPenColor(100, 100, 100);
   m_plotter->setFillColor(red, green, blue);

   prim.setDrawn(true);

   int rectSize = 1;
   int def_outerRectSize = rectSize + 2;
   for ( vector<MC2Point>::const_iterator it = m_screenCoords.begin();
         it != m_screenCoords.end();
         ++it ) 
   {
      int outerRectSize = def_outerRectSize;
      
      isab::Rectangle r(it->getX() - outerRectSize / 2, 
                        it->getY() - outerRectSize / 2,
                        outerRectSize,
                        outerRectSize);

      isab::Rectangle r_inner(it->getX() - rectSize / 2, 
                              it->getY() - rectSize / 2,
                              rectSize,
                              rectSize);

      m_plotter->setPenColor(50, 50, 50);
      m_plotter->drawRect(false, r, 0);
      m_plotter->setPenColor(red, green, blue);
      m_plotter->drawRect(false, r_inner, 0);
   }
}
#endif

void
TileMapHandler::plotWhatWeGot()
{
   if ( m_mapFormatDesc == NULL || !m_isDrawing ) {
      return;
   }
   
   int nbrDrawn = 0;
   int nbrFeatures = 0;
   
   mc2dbg << "[TMH]: Plotting STARTS" << endl;

   // Only redraw for outlines if we didn't do it last time.
   bool detailRepaint = m_detailRepaintTimerHasBeenRun &&
                              !(m_outlinesDrawnLastTime);

   m_movementHelper.updateIfToDrawOutlines( detailRepaint );
   
   if ( (m_matrixChangedSinceDraw || detailRepaint ) ||
        m_plotter->nbrMaskedExtraBitmaps() < 1 ||
        m_mapArrivedSinceRepaint ) {
      
      m_matrixChangedSinceDraw = false;
      
      m_plotter->prepareDrawing();
      // Set white
      uint32 backColor = m_mapFormatDesc->getDefaultBackgroundColor();
      m_plotter->setBackgroundColor( (backColor >> 16 ) & 0xff,
                                     (backColor >> 8  ) & 0xff,
                                     (backColor >> 0  ) & 0xff );
      m_plotter->clearScreen();
      m_overlapDetector.clear();
      m_scaleIndex = 
         m_mapFormatDesc->getScaleIndexFromScale( 
            uint32(getDPICorrectedScale()) );
      
      // Test if it is possible to divide the bounding boxes into
      // smaller parts and draw several times.
      
      // Divide the bounding box into several new ones
      vector<MC2BoundingBox> bboxVec;
      
      getDrawingBBoxes( bboxVec );

      // If the detail repaint timer hasn't been run yet, then
      // skip the outlines so that moving can be done faster.
      // If tracking is on, always draw outlines.
      bool drawOutlines =
         // Means that we have already painted the map once without
         // the outlines and the user now has stopped moving the map.
         m_detailRepaintTimerHasBeenRun ||
         // When tracking is on we should always draw the outlines.
         !m_movementHelper.isMoving();
      
      m_movementHelper.updateIfToDrawOutlines( drawOutlines );
      
      const bool skipOutlines = ! drawOutlines;

      m_outlinesDrawnLastTime = drawOutlines;

      // Start the text handling if the outlines have been drawn
      if ( drawOutlines ) {
         requestTextHandlerIdle();
      }
      
      // Plot the parts
      uint32 i;   
      for ( i = 0; i < bboxVec.size(); ++i ) {      
         plotWhatWeGotInBBox( bboxVec[i], skipOutlines,
                              nbrFeatures, nbrDrawn );
      }
           
      // Request a new detail repaint timer so that we can decide if
      // we should draw the details the next time.
      if ( m_movementHelper.isMoving() ) {
         // FIXME: Do no request so many timers, always cancel one
         if ( m_detailRepaintTimerRequested ) {
            m_toolkit->cancelTimer(this, m_detailRepaintTimerRequested);
            m_detailRepaintTimerRequested = 0;
         }
         m_detailRepaintTimerHasBeenRun = false;
         mc2dbg << "[TMH]: Requested detail repaint timer" << endl;
         
         if ( m_movementHelper.detectMovementBySelf() ) {
            // Sigh, start a timer to check if the map has stopped moving.
            m_detailRepaintTimerRequested =
               m_toolkit->requestTimer(this,
                                       400);
         }
      }

      
#if 0
      // Plot all the bounding boxes.
      m_plotter->setLineWidth( 2 );
      m_plotter->setPenColor( 0xff00ff );
      for ( i = 0; i < bboxVec.size(); ++i ) {
         plotBBox( bboxVec[i], bbox );
      }
#endif
      
      drawProgressIndicator();
   
      // Add the map count here.
      if ( (*m_tileMapCont).getPercentageMapsReceived() >= 100 ) {     
         mc2log << "[TMH]: No missing maps - turning off the re-request timer"
                << endl;
         if ( m_rerequestTimer != 0 ) {
            m_toolkit->cancelTimer( this, m_rerequestTimer );
            m_rerequestTimer = 0;
         }
      }
      
      drawTexts();
   }

   m_bitmapMover.reset();
   m_plotter->makeScreenSnapshot();
   
   plotCopyrightString(); 
   // This can be drawn on a separate bitmap
   drawUserDefinedFeatures();
   
   {
      if ( false && m_tmkTimingInfo != NULL ) {
         vector<MC2SimpleString> texts;
         char tmpStr[80];
         sprintf( tmpStr, "R: %ld", (long int)m_tmkTimingInfo->m_nbrRunsLeft );
         texts.push_back( tmpStr );
         sprintf( tmpStr, "D: %ld", (long int)m_tmkTimingInfo->m_nbrRunsDone );
         texts.push_back( tmpStr );
         sprintf( tmpStr, "Avg: %lu",
                  (long unsigned int)m_tmkTimingInfo->m_avgTimePerRun );
         texts.push_back( tmpStr );
         sprintf( tmpStr, "Tim: %lu",
                  (long unsigned int)m_tmkTimingInfo->m_elapsedTime );
         texts.push_back( tmpStr );
         sprintf( tmpStr, "L: %lu",
                  (long unsigned int)m_tmkTimingInfo->m_lastTime );
         texts.push_back( tmpStr );
         printDebugStrings( texts );
      }
   }

   drawDebugStrings();

   if ( m_showHighlight ) {
      isab::Rectangle tmpRect;
      m_plotter->getMapSizePixels(tmpRect);
      TileMapHandlerClickResult tmpRes;
      if ( m_highlightPoint == MC2Point( -1, -1 ) ) {
         getInfoForFeatureAt( tmpRes, PixelBox(tmpRect).getCenterPoint(),
                           true ); // Only highlightable features wanted.
      } else {
         getInfoForFeatureAt( tmpRes, m_highlightPoint,
                           true ); // Only highlightable features wanted.
      }
      UserDefinedFeature* highlight = tmpRes.getOutlineFeature();
      if ( highlight ) {
         drawOneUserDefinedFeature(*highlight);
      }
   }
   m_tileMapCont->emptyMapInMapsToReq();
   m_tileMapCont->countMapsInMapsToReq();
   
   mc2log << "[TMH]: Finished plotting   ENDS Number features "
          << nbrFeatures << ", Number of points: " << nbrDrawn << endl;

   
   m_plotter->drawingCompleted();      
}


// Inlined so that it can be auto-removed in releases.
inline void
TileMapHandler::printDebugStrings(const vector<MC2SimpleString>& strings)
{
   if ( strings.empty() ) {
      return;
   }
   // Black text s.v.p.
#ifdef __SYMBIAN32__
   STRING* fontName = m_plotter->createString("Swiss");
#else
   STRING* fontName =
      m_plotter->createString("-*-times-*-r-*-*-12-*-*-*-*-*-*-*");
#endif
   m_plotter->setFont( *fontName, 18 );
   m_plotter->deleteString( fontName );
   isab::Rectangle lastScreenSize;
   m_plotter->getMapSizePixels(lastScreenSize);
   PixelBox pix(lastScreenSize);
   MC2Point curPoint( pix.getCenterPoint() );
   // Start at y = height-24
   curPoint.getY() = lastScreenSize.getHeight() - 24;
   for ( int i = strings.size() - 1; i >= 0; -- i ) {
      STRING* str = m_plotter->createString(strings[i].c_str());
      m_plotter->setPenColor(255,255,255);
      MC2Point backPoint ( curPoint );
      backPoint.getX()--;
      backPoint.getY()--;
      m_plotter->drawText( *str, backPoint );
      m_plotter->setPenColor(0,0,0);      
      m_plotter->drawText( *str, curPoint );
      // Back up y
      curPoint.getY() -=
         m_plotter->getStringAsRectangle( *str,
                                          curPoint ).getHeight() + 1;
      m_plotter->deleteString(str);
   }
}


void
TileMapHandler::drawDebugStrings()
{   

#if 0 // Enable this for some debug output.
   while ( m_drawTimes.size() > 20 ) {
      m_drawTimes.pop_front();
   }
   vector<MC2SimpleString> strings;
//   for ( list<pair<char, uint32> >::const_iterator it = m_drawTimes.begin();
//         it != m_drawTimes.end();
//         ++it ) {
//      char* tmpStr = new char[1024];
//      unsigned long int thetime = it->second;
//      sprintf(tmpStr, "%c %ld ms", it->first, thetime);
//
//      strings.push_back(tmpStr);
//      delete [] tmpStr;
//   }

   // Print available memory also.
   char avail[80];
   uint32 biggestBlock;
   sprintf(avail, "Av: %lu",
           (unsigned long int)m_toolkit->availableMemory(biggestBlock));
   strings.push_back( avail );
#ifndef __unix__
 
   
   sprintf(avail, "Ac: %lu",
         (unsigned long int)User::CountAllocCells());
   strings.push_back( avail );

   int allocSize;
   User::AllocSize( allocSize );
   sprintf(avail, "As: %lu", (unsigned long int)allocSize );
   strings.push_back( avail );

   int memFree = 0;
#if !(defined NAV2_CLIENT_SERIES60_V3 || defined NAV2_CLIENT_UIQ3)
   HAL::Get(HALData::EMemoryRAMFree, memFree );

   if ( memFree < 1024*1024 ) {
      User::CompressAllHeaps();
      sprintf( avail, "User::compressAllHeaps() called!" );
      strings.push_back( avail );
   }
#endif
   
   sprintf(avail, "HAL+As: %lu", (unsigned long int)allocSize + memFree );
   strings.push_back( avail );
   
   sprintf(avail, "HAL: %lu", 
           (unsigned long int) memFree);
   strings.push_back( avail );
   sprintf(avail, "G: %lu", 
           (unsigned long int) m_garbage.size());
   strings.push_back( avail );
#endif
   printDebugStrings(strings);
#else
   m_drawTimes.clear();
#endif
}


void
TileMapHandler::drawTexts()
{
    /* 
    * the code below is purly for debug. draws al the overlaytest boxes 
    * so that we can se that they are correct.
    */
#if 0
   vector<PixelBox> boxes;
   m_textHandler->getOverlapVector( boxes );
   m_plotter->setLineWidth( 1 );  
   for( int i = 0;
         i < (int)boxes.size();
         i++ ) {
      const PixelBox pb = boxes[ i ];
      const Rectangle rect( 
            pb.getMinLon(), pb.getMinLat(), 
            pb.getWidth(), pb.getHeight() );
      m_plotter->setPenColor( 0, 255, 0 );
      m_plotter->drawRect( false, rect, 1 );
   }
#endif
   for( int i = 0;
         i < (int)m_textHandler->getTextResults().size();
         i++ ) {

      

      const TextResult* tr = m_textHandler->getTextResults()[i];
      const isab::Rectangle rect = tr->getStringAsRect();
      if ( tr->getTypeOfString() == TileMapNameSettings::on_roundrect ) {
         m_plotter->setLineWidth( 1 );
         m_plotter->setFillColor( 31, 31, 255 );
         m_plotter->setPenColor( 31, 31, 255 );
         m_plotter->drawRect( true, rect, 2 );
         m_plotter->setPenColor( 255, 255, 255 );
         m_plotter->drawRect( false, rect, 2 );
      }
      mc2dbg2 << "In TileMapHandler::drawTexts()" << endl;

      m_plotter->setFont( tr->getFontName(), tr->getFontSize() );

      unsigned int r;
      unsigned int g;
      unsigned int b;
      tr->getFontColor( r, g, b );
      m_plotter->setPenColor( r, g, b );
      for( int j = 0; j < (int)tr->getTPN().size(); j++ ) {
         TextPlacementNotice tpn = tr->getTPN()[j];
    
         mc2dbg8 << "namedraw2: " <<  tr->getString() << endl;
         mc2dbg8 << " si " << tpn.m_startIndex <<
                "  " << tpn.m_endIndex << endl;
    
         m_plotter->drawText( tr->getString(), tpn.m_point,
            tpn.m_startIndex, tpn.m_endIndex, tpn.m_angle );
      }
   }
}

inline void
TileMapHandler::drawOneUserDefinedFeature( UserDefinedFeature& userFeat )
{
   // Note that the screen coordinates are updated inside the switch
   // statement.
      
   switch ( userFeat.getType() ) {
      // Filled or unfilled polygon.
      case UserDefinedFeature::directed_poly: {
         if ( ! userFeat.isVisible() ) {
            return;
         }
         // Update the screen coordinates.
         userFeat.updateScreenCoords(*this);
         DirectedPolygon* poly = static_cast<DirectedPolygon*>(&userFeat);
         m_plotter->setLineWidth( poly->getPenSize() );
         if ( poly->isFilled() ) {
            m_plotter->setFillColor( poly->getColor() );
            m_plotter->drawPolygon( poly->getScreenCoords().begin(),
                                    poly->getScreenCoords().end() );
         } else {
            m_plotter->setPenColor( poly->getColor() );
            m_plotter->drawPolyLine( poly->getScreenCoords().begin(),
                                     poly->getScreenCoords().end() );
         }
      }
      break;
      // BitMap
      case UserDefinedFeature::bitmap: {
         // Update the user defined features before checking the BB
         userFeat.updateScreenCoords(*this);
         if ( ! getBoundingBox().contains( 
                  userFeat.getCenter().getWorldCoord() ) ) {
            return;
         }
         UserDefinedBitMapFeature* bmp =
            static_cast<UserDefinedBitMapFeature*>(&userFeat);
         if ( bmp->getBitMapName() != NULL ) {
            // Load the bitmap even if the feature is not visible.
            isab::BitMap* bitMap =
               getOrRequestBitMap( bmp->getBitMapName() );
            if ( ! userFeat.isVisible() ) {
               return;
            }
            if ( bitMap != NULL ) {
               // Update the screen coordinates.
               userFeat.updateScreenCoords(*this);
               m_plotter->drawBitMap( bmp->getScreenCoords().front(),
                                      bitMap );
            }
         }
      }
      break;

      case UserDefinedFeature::scale: {
         // Update the screen coordinates.
         userFeat.updateScreenCoords(*this);
         UserDefinedScaleFeature* scaleFeat =
            static_cast<UserDefinedScaleFeature*>(&userFeat);
         scaleFeat->draw( *this, *m_plotter );
      }
      break;
       
      default:
         // Do you want to know what it is?
         break;
   }
}

void
TileMapHandler::drawUserDefinedFeatures()
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   if ( m_userDefinedFeatures == NULL ) {
      return;
   }

   if ( m_plotter->nbrMaskedExtraBitmaps() >= 1 ) {
      for ( int i = 0; i < 2; ++i ) {
         // Switch to extra bitmap, then to extra bitmap mask
         if ( m_plotter->switchBitmap( 1, i ) ) {
            // Success. Clear the screen.
            m_plotter->clearScreen();
         }
         for ( vector<UserDefinedFeature*>::iterator it =
                  m_userDefinedFeatures->begin();
               it != m_userDefinedFeatures->end();
               ++it ) {
            if ( *it != NULL ) {
               drawOneUserDefinedFeature( **it );
            }
         }
      }

      m_plotter->switchBitmap( 0, false );
   } else {
      // Must draw on the only bitmap
      for ( vector<UserDefinedFeature*>::iterator it =
               m_userDefinedFeatures->begin();
            it != m_userDefinedFeatures->end();
            ++it ) {
         if ( *it != NULL ) {
            drawOneUserDefinedFeature( **it );
         }
      }
   }
}

void
TileMapHandler::updateParams()
{
   mc2dbg << "[TMH]: Updateparams 1" << endl;
   // If no mapformatdesc then request it.
   if ( m_mapFormatDesc == NULL ) {
      requestMaps( 1 );
      return;
   }
   
   if ( (*m_tileMapCont).updateParams( *m_mapFormatDesc,
                                    *this, m_useGzip,
                                    m_lang, m_layersToDisplay, 
                                    m_routeID) ) {
      
      // Cancel the re-request timer so that the time will start
      // anew.
      if ( m_rerequestTimer != 0 ) {
         m_toolkit->cancelTimer( this, m_rerequestTimer );
         m_rerequestTimer = 0;
      }

      m_requester->cancelAll();
      m_textHandler->mapVectorChanged( (*m_tileMapCont).beginMapsToDraw(),
                                       (*m_tileMapCont).endMapsToDraw(),
                                       !m_movementHelper.isMoving() );
   }
}

const vector<const TileCategory*>*
TileMapHandler::getCategories() const
{
   if ( m_mapFormatDesc == NULL ) {
      return NULL;
   } else {
      return & ( m_mapFormatDesc->getCategories() );
   }
}

bool
TileMapHandler::setCategoryEnabled( int id, bool enabled )
{
   if ( m_mapFormatDesc == NULL ) {
      return false;
   } else {
      bool retVal = m_mapFormatDesc->setCategoryEnabled(id, enabled);
      if ( retVal ) {
         // Update the list of disabled item types.
         m_disabledParentTypes.clear(); 
         m_mapFormatDesc->getFeaturesDisabledByCategories(
            m_disabledParentTypes);
      }
      return retVal;
   }
}

bool
TileMapHandler::updateLayerInfo( TileMapLayerInfoVector& info )
{
   if ( m_mapFormatDesc == NULL ) {
      // No change
      return false;
   }
   const bool layers_changed = m_mapFormatDesc->updateLayers( info );
   const bool visibility_changed =
      m_mapFormatDesc->updateLayersToDisplay( m_layersToDisplay );
   if ( visibility_changed ) {
      updateParams();
      requestRepaint();
   }
   return layers_changed;
}
   
void 
TileMapHandler::setProgressIndicatorBox( const PixelBox& box )
{
   m_progressIndicatorBox = box;
}
   
DBufRequestListener* 
TileMapHandler::getDBufRequestListener() const
{
   return m_dataBufferListener;
}


void
TileMapHandler::setBBoxFromParamStr( const MC2SimpleString& paramString ) {
   if ( m_mapFormatDesc == NULL ) {
      return;
   }

   TileMapParams param( paramString );
   mc2dbg << param << endl;
   MC2BoundingBox bbox;
   m_mapFormatDesc->getBBoxFromTileIndex( param.getLayer(), bbox,
                                          param.getDetailLevel(),
                                          param.getTileIndexLat(),
                                          param.getTileIndexLon() );
   // Tilemaps can be off the world
   if ( bbox.getMaxLat() > MAX_INT32/2 ) {
      bbox.setMaxLat( MAX_INT32/2 );
   } 
   if ( bbox.getMinLat() > MAX_INT32/2 ) {
      bbox.setMinLat( MAX_INT32/2 );
   } 
   if ( bbox.getMinLat() < -MAX_INT32/2 ) {
      bbox.setMinLat( -MAX_INT32/2 );
   }
   if ( bbox.getMaxLat() < -MAX_INT32/2 ) {
      bbox.setMaxLat( -MAX_INT32/2 );
   }
   bbox.updateCosLat();
   setBoundingBox( bbox );
//   mc2dbg << bbox << endl;

   if ( param.getRouteID() != NULL ) {
      mc2dbg << " RouteID: X_X";
      setRouteID( *param.getRouteID() );
   }

   mc2dbg << endl;
}

   
bool 
TileMapHandler::getNextHighlightPoint( MC2Point& point ) 
{
   const TilePrimitiveFeature* highlightFeature = 
      (*m_tileMapCont).getNextHighlightFeature();
   if ( highlightFeature != NULL ) {
      // Got something to highlight!

      // We "know" that it must be a bitmap.
      MC2_ASSERT( highlightFeature->getType() == FeatureType::bitmap );

      // Get the coordinate.
      const CoordArg* coordArg =
         static_cast<const CoordArg*>
         (highlightFeature->getArg(TileArgNames::coord));
      MC2Coordinate coord;
      coord.lat = coordArg->getCoord().getLat();
      coord.lon = coordArg->getCoord().getLon();

      // And transform it to screen coordinates.
      transformPointInternalCosLat( point, coord );

      // Move it to the middle of the poi.
      const StringArg* bitMapName =
         static_cast<const StringArg*>
         (highlightFeature->getArg(TileArgNames::image_name));

      MC2_ASSERT( bitMapName != NULL );
     
      bitMapMap_t::const_iterator it = 
         m_bitMaps.find( bitMapName->getValue() );
      
      if ( it == m_bitMaps.end() ) {
         // Should not happen, but if it does,
         // the cursor will not be centered.
         return true;
      }

      // Fetch bitmap size from the plotter.
      PixelBox box = m_plotter->getBitMapAsRectangle( point, it->second );
      // And then move to the center of that.
      point = box.getCenterPoint();      
      
      return true;
   } else {
      // Nothing to highlight.
      return false;
   }
}

void
TileMapHandler::setMoving( bool moving ) 
{
   m_movementHelper.setMoving( moving );
   if ( ! moving ) {
      // Stopped moving, then time to draw the outlines, ey.
      requestRepaint();
   }
}

void 
TileMapHandler::setDetectMovementBySelf( bool detectSelf )
{
   m_movementHelper.setDetectMovementBySelf( detectSelf );
}

uint32 
TileMapHandler::getPercentageMapsReceived() const
{
   // Return how many percent of the maps we have
   return (*m_tileMapCont).getPercentageMapsReceived();
}

void 
TileMapHandler::setCopyrightPos( const MC2Point& pos )
{
   m_copyrightPos = pos;
}

void 
TileMapHandler::showCopyright( bool show )
{
   m_showCopyright = show;
}

bool 
TileMapHandler::canHandleScreenAsBitmap() const
{
   return m_plotter->snapshotHandlingImplemented();
}

void 
TileMapHandler::moveBitmap(int deltaX, int deltaY)
{
   m_bitmapMover.move( MC2Point( deltaX, deltaY ) );
   m_plotter->prepareDrawing();
   m_plotter->moveScreenSnapshot( m_bitmapMover.getMoveOffset() );
   
   // Draw the rest of map.
   plotCopyrightString(); 
   drawUserDefinedFeatures();

   m_plotter->drawingCompleted();
}

void 
TileMapHandler::setPointBitmap( const MC2Point& screenPoint )
{
   m_bitmapMover.moveTo( screenPoint );
   m_plotter->prepareDrawing();
   m_plotter->moveScreenSnapshot( m_bitmapMover.getMoveOffset() );
   
   // Draw the rest of map.
   plotCopyrightString(); 
   drawUserDefinedFeatures();

   m_plotter->drawingCompleted();
}

void
TileMapHandler::setBitmapDragPoint( const MC2Point& dragPoint )
{
   m_bitmapMover.setDragPoint( dragPoint );
}

void 
TileMapHandler::zoomBitmapAtPoint( double factor, 
                                   const MC2Point& screenPoint )
{
   m_bitmapMover.zoom( factor ); 
   m_plotter->prepareDrawing();
   m_plotter->zoomScreenSnapshot( m_bitmapMover.getZoomFactor(), 
                                  screenPoint );
   
   // Draw the rest of map.
   plotCopyrightString(); 
   drawUserDefinedFeatures();

   m_plotter->drawingCompleted();

}
void 
TileMapHandler::zoomBitmapAtCenter( double factor )
{
   isab::Rectangle rect;
   m_plotter->getMapSizePixels(rect);
   zoomBitmapAtPoint( factor, MC2Point( rect.getWidth() / 2, 
                                        rect.getHeight() / 2 ) );
}

bool 
TileMapHandler::mapFullyLoaded() const 
{
   if ( m_tileMapCont->getPercentageMapsReceived() == 100 && 
        m_mapFormatDesc ) {
      return true;
   }
   return false;
}

void TileMapHandler::setIsDrawing(bool isDrawing)
{
   m_isDrawing = isDrawing;
}

#ifdef __unix__
void TileMapHandler::setSelectedRoad(MC2SimpleString selectedRoad)
{
   m_selectedRoad = selectedRoad;
   requestRepaint();
}
#endif

int TileMapHandler::getDistanceMeters(const MC2Point& point,
                                      const MC2Point& lineBegin,
                                      const MC2Point& lineEnd,
                                      float cosLat)
{
   MC2Coordinate mc2Point;
   MC2Coordinate mc2LineBegin;
   MC2Coordinate mc2LineEnd;
   
   inverseTranformUsingCosLat( mc2Point,
                               point.getX(),
                               point.getY() );
   
   inverseTranformUsingCosLat( mc2LineBegin,
                               lineBegin.getX(),
                               lineBegin.getY() );

   inverseTranformUsingCosLat( mc2LineEnd,
                               lineEnd.getX(),
                               lineEnd.getY() );

   return getDistanceMeters(mc2Point, mc2LineBegin, mc2LineEnd, cosLat);
}

int TileMapHandler::getDistanceMeters(const MC2Point& point,
                                      const TilePrimitiveFeature& feat,
                                      float cosLat)
{
   MC2Coordinate mc2Point;
   
   inverseTranformUsingCosLat( mc2Point,
                               point.getX(),
                               point.getY() );

   TileMapCoord tPoint(mc2Point.lat, mc2Point.lon);
   
   int64 dist = getMinSQDist( tPoint, feat, cosLat );
   float distMeterF = 
      sqrt( float( dist ) ) * GfxConstants::MC2SCALE_TO_METER;
   
   int distMeter = static_cast<int64>( distMeterF + 0.5f );

   return distMeter;
}

int TileMapHandler::getDistanceMeters(const MC2Coordinate& coord,
                                      const MC2Coordinate& lineBegin,
                                      const MC2Coordinate& lineEnd,
                                      float cosLat)
{
   TileMapCoord tPoint(coord.lat, coord.lon);
   TileMapCoord tLineBegin(lineBegin.lat, lineBegin.lon);
   TileMapCoord tLineEnd(lineEnd.lat, lineEnd.lon);
   
   int64 curDist =  sqDistanceCoordToLine(tPoint,
                                          tLineBegin,
                                          tLineEnd,
                                          cosLat);
   
   float curDistMeterf = sqrt( float( curDist ) ) *
      GfxConstants::MC2SCALE_TO_METER;
   
   int curDistMeter = static_cast<int64>( curDistMeterf + 0.5f );

   return curDistMeter;
}

