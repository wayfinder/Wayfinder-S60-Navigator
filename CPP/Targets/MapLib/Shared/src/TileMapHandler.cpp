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
#include "PlotterAdapter3d.h"
#include "PositionInterpolator.h"
#include "InterpolationCallback.h"

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
#include <e32cmn.h>
#endif

#include "TMKTimingInfo.h"
#include "TileMapInfoCallback.h"
#include "PlotterAdapter3d.h"
#include "ImageBlender.h"

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

//#define ENABLE_INTERPOLATION_DEBUGGING

using namespace std;
using namespace isab;


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
   uint32 curTimeSec = TileMapUtil::currentTimeMillis() / 1000;
   mc2dbg << "[THM]: m_firstFailedTimeSec = "
          << m_firstFailedTimeSec << endl;
   mc2dbg << "[TMH]: m_requested.size() = " << m_requested.size()
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
      if ( descr[0] == 'D' || descr[0] == 'd' ) {
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

class ScopedCounter {
public:
   ScopedCounter( int& var ) : m_var( var ) {
      ++m_var;
   }

   ~ScopedCounter() {
      --m_var;
   }

private:
   int& m_var;
};

typedef TileMapContainer::MIt MIt;
typedef TileMapContainer::PIt PIt;

TileMapHandler::TileMapHandler(MapPlotter* plotter,
                               DBufRequester* requester,
                               TileMapToolkit* platform,
                               TileMapEventListener* eventListener )
      : m_plotter2d( plotter ),
        m_plotter3d( new PlotterAdapter3d( plotter ) ),
        m_toolkit(platform),
        m_mapFormatDesc(NULL),
        // We are not painting.
        m_painting(0),
        m_garbage( platform ),
        m_tileMapCont( new TileMapContainer( this, &m_garbage ) ),
        m_highlightPoint(-1, -1),
        m_screenCoords( m_realScreenCoords, 0, 0 ),
        m_copyrightPos( 0, 0 ),
        m_showCopyright( false ),
        m_nightMode( false ),
        m_3dOn( false ),
        m_outlinesIn3dEnabled( false ),
        m_acpModeEnabled( true )
{
   m_interpolatingPositions = false;
   
   m_posInterpolator = new PositionInterpolator;
   m_posInterpolator->setAllowedCatchUpFactor( 0.5 );
   m_positionInterpolationRequested = 0;
   m_interpolationCallback = NULL;
   m_inRequestingMaps = 0;
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
   m_horizonHeight = 0;
   m_plotter = m_plotter2d;
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

   m_routeID = NULL;

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
//     MC2Coordinate lower( 664609150, 157263143 );
//     MC2Coordinate upper( 664689150, 157405144 );
   // Strange street here.
//   MC2Coordinate lower( 664380226,156217749 );
//   MC2Coordinate upper( 664357053,156258858 );
   //   Malm�   
   MC2Coordinate lower( 663428099, 154983536 );
   MC2Coordinate upper( 663394172, 155043599 );
   
//   // Europe
//   MC2Coordinate lower( 756634992, -175799752 );
//   MC2Coordinate upper( 405535631, 278927496 );

//   // Helsinki
//   MC2Coordinate lower( 717870134, 297503083 );
//   MC2Coordinate upper( 717839305, 297556052 );

   MC2BoundingBox bbox(lower, upper);
   
   setBoundingBox(bbox);
   setAngle(0);
}

TileMapHandler::~TileMapHandler()
{
   delete m_dxxBuffer;
   delete m_outlinePolygon;
   delete m_requester;
   delete m_dataBufferListener;

   for ( bitMapMap_t::iterator it = m_bitMaps.begin();
         it != m_bitMaps.end();
         ++it ) {
      m_plotter->deleteBitMap( (*it).second );
   }

#ifndef QUICK_SHUTDOWN    
   delete m_mapFormatDesc;
#endif
   delete m_releaseChecker;
   delete m_tileMapCont;
   delete m_clickInfo;
}

MC2Point
TileMapHandler::getCenterPoint() const
{
   isab::Rectangle sizePixels;
   m_plotter->getMapSizePixels( sizePixels );

   MC2Point centerPoint = MC2Point( sizePixels.getWidth() / 2,
         sizePixels.getHeight() / 2 );

   return centerPoint;
}


MC2Pointf
TileMapHandler::getPoint2Df( const MC2Point& screenPoint ) const
{
   MC2Pointf point2D = { screenPoint.getX(),
                         screenPoint.getY() };

   if ( m_3dOn ) {
      // Get the 2D point corresponding to the 3d point.
      point2D = m_plotter3d->transform3DTo2D_slowf( screenPoint );
   }

   return point2D;
}

MC2Point 
TileMapHandler::getPoint2D( const MC2Point& screenPoint ) const
{
   MC2Point point2D = screenPoint;
   if ( m_3dOn ) {
      // Get the 2D point corresponding to the 3d point.
      point2D = m_plotter3d->transform3DTo2D_slow( point2D );
   }

   return point2D;
}

void 
TileMapHandler::transform( MC2Point& point,
                          const MC2Coordinate& coord ) const 
{

   MapProjection::transformPointInternalCosLat( point, coord );
   if ( m_3dOn ) {
      // Transform 2d point to 3d.
      point = m_plotter3d->transform2DTo3D( point );
   }
}

void 
TileMapHandler::inverseTransform( MC2Coordinate& coord,
                                 const MC2Point& point ) const 
{
   MapProjection::inverseTranformUsingCosLat( coord,
                                              getPoint2D( point ) );
}

const MC2Coordinate& 
TileMapHandler::getCenter() const 
{
   if ( m_3dOn ) {
      // Going to 2D from 3D. Keep center coordinate.
      MC2Coordinate centerCoord;
      inverseTransform( centerCoord, getCenterPoint() );

      const_cast<TileMapHandler*> (this)->m_tmpCoord = centerCoord;
      return m_tmpCoord;
   } else {
      return MapProjection::getCenter();
   }
}

void 
TileMapHandler::setCenter( const MC2Coordinate& newCenter ) 
{
   if (! m_3dOn ) {
      MapProjection::setCenter( newCenter );
   } else {
      MapProjection::setPointf( newCenter, getPoint2Df( getCenterPoint() ) );
   }
}

void 
TileMapHandler::setPoint(const MC2Coordinate& newCoord,
                        const MC2Point& screenPoint ) 
{
   MapProjection::setPoint( newCoord, getPoint2D( screenPoint ) );
}

void 
TileMapHandler::setPoint(const MC2Coordinate& newCoord,
                         const MC2Point& screenPoint,
                         double angleDegrees ) 
{
   MapProjection::setPoint( newCoord, getPoint2D( screenPoint ), angleDegrees );
}

void 
TileMapHandler::setAngle( double angleDegrees,
                      const MC2Point& rotationPoint ) 
{
   MapProjection::setAngle( angleDegrees, getPoint2D( rotationPoint ) );
}

void 
TileMapHandler::setAngle(double angleDegrees) 
{
   // Means set angle of the center.
   if ( m_3dOn ) {
      setAngle( angleDegrees, getCenterPoint() );      
   } else {
      MapProjection::setAngle( angleDegrees );
   }
}

double
TileMapHandler::zoom( double factor, 
                    const MC2Coordinate& zoomCoord,
                    const MC2Point& zoomPoint ) {
   return MapProjection::zoom( factor, zoomCoord, getPoint2D( zoomPoint ) );
}

double 
TileMapHandler::zoom( double factor ) 
{
   if ( m_3dOn ) {
      // Zoom means zooming in the center of the screen.
      return zoom( factor, getCenter(), getCenterPoint() );
   } else {
      return MapProjection::zoom( factor );
   }
}

void 
TileMapHandler::setPixelBox( const MC2Point& oneCorner,
                             const MC2Point& otherCorner ) 
{
   MapProjection::setPixelBox( getPoint2D( oneCorner ), 
                               getPoint2D( otherCorner ) );
}
      
void 
TileMapHandler::setWorldBox( const MC2Coordinate& oneCorner,
                            const MC2Coordinate& otherCorner ) 
{
   MC2BoundingBox box( oneCorner, otherCorner );
   MapProjection::setBoundingBox( box );
   // Set center so that center of box will be centered also in 3d. 
   setCenter( box.getCenter() );
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
   if ( m_textHandlerIdleID ) {
      m_toolkit->cancelIdle(this, m_textHandlerIdleID );
      m_textHandlerIdleID = 0;
   }
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
TileMapHandler::getScreenCoords( TileMap& tileMap, TilePrimitiveFeature& feature )
{
   m_screenCoords.clear();
   if ( feature.isDrawn() ) {
      // The feature is drawn, i.e. calculate it's coordinates.
      switch ( feature.getType() ) {
         case TilePrimitiveFeature::line:
         case TilePrimitiveFeature::polygon:
            // Polylines and polygons uses prepareCoordinates.
            prepareCoordinates( tileMap, feature, getBoundingBox() );
            break;
         case TilePrimitiveFeature::bitmap:
         case TilePrimitiveFeature::circle: {
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

const RouteID*
TileMapHandler::getRouteID() const
{
   return m_routeID;
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
TileMapHandler::getMinSQDist(TileMap& tileMap,
                             const TileMapCoord& coord,
                             const TilePrimitiveFeature& prim,
                             float cosLat)
{
   if ( prim.getType() == TilePrimitiveFeature::polygon ) {
      // Don't use the polygons, since the border isn't interesting there
      return MAX_INT64;
   }
   const CoordsArg* coords =
      static_cast<const CoordsArg*>(prim.getArg(TileArgNames::coords));
   if ( coords ) {
#ifndef SLIM_TILE_COORDS
      CoordsArg::const_iterator it = coords->begin();
#else
      CoordsArg::const_iterator it = coords->begin( tileMap );
#endif
      CoordsArg::const_iterator lastit = it++;
      int64 minDist = MAX_INT64;
#ifndef SLIM_TILE_COORDS
      while ( it != coords->end() ) {
#else
      while ( it != coords->end(tileMap) ) {
#endif
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
            if (curPrim->getType() == TilePrimitiveFeature::polygon ) {
               const CoordsArg* coords =
                  static_cast<const CoordsArg*>
                  (curPrim->getArg(TileArgNames::coords));
               if ( coords->getBBox().contains( useCoord ) ) {
                  // Real inside check here.
#ifndef SLIM_TILE_COORDS
                  if ( InsideUtil::inside( coords->begin(),
                                           coords->end(),
                                           coord ) ) {
#else
                  if ( InsideUtil::inside( coords->begin(*curMap),
                                           coords->end(*curMap),
                                           coord ) ) {
#endif
                     return make_pair(curMap, curPrim);
                  }
               }
            }
         }
      }
   }
   return make_pair(closestSoFarMap, closestSoFar);
}

inline const TilePrimitiveFeature*
TileMapHandler::getClosest(TileMap& tileMap,
                           const MC2Point& point,
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
      if ( it->getType() == TileFeature::bitmap ) {
         // Check if it has been drawn.
         if ( (*it).isDrawn() ) {
            if ( getPixelBoxForBitMap( *it ).pointInside ( point ) ) {
               mindist = 0;
               return &(*it);
            }
         }
      } else {
         int64 curDist = getMinSQDist(tileMap, tileCoord, (*it), cosLat);
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
   inverseTransform( coord, point );
   
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
            getClosest(*curMap, point, coord,
                       prims.begin(), prims.end(), 
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
         (primType == TileFeature::polygon )) &&
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
   mc2dbg << "[TMH]: New params : " << *stringParams << endl;
   const TileMap* stringMap = getMap(*stringParams);      
   if ( stringMap == NULL ) {
      // Not found or not loaded yet.
      // Notify the tilemap info callback when this map is received.
      m_waitingForInfoString.first = stringParams->getAsString();
      m_waitingForInfoString.second = infoCallback;
      
      requestFromCacheOrForReal(*stringParams);
      mc2dbg << "[TMH]: No string map - requesting " << endl;         
      return NULL;
   } else {
      // No callback wanted
      m_waitingForInfoString.second = NULL;
      const char* str = stringMap->getStringForFeature(
         closestSoFar->getFeatureNbr());
      if ( str ) {
         return str;
      } else {
         mc2dbg << "[TMH]: No string found in map" << endl;
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

static inline
MC2SimpleString
createServerString( const char* name,
                    const MC2Coordinate& coord,
                    pair<const TileMap*,
                    const TilePrimitiveFeature*>& mapAndFeature )
{
   const bool closed =
      mapAndFeature.second->getType() == TilePrimitiveFeature::polygon;
   
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
   MC2_ASSERT( bitmap.getType() == TileFeature::bitmap );

   MC2Point center(0,0);
   getXYFromCoordArg( bitmap, getBoundingBox(), 
         center.getX(), center.getY() );

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

// This is the one doing the stuff.
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
         if ( ! curFeat->wasDrawn() ) continue;
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
   // the last param to TilePrimitiveFeature::bitmap, otherwise
   // MAX_INT32.
   pair<const TileMap*, const TilePrimitiveFeature*> mapAndFeature =
      getFeatureAt(point, &res.m_distance, 
             highlightable ? TilePrimitiveFeature::bitmap : MAX_INT32 );
   if ( mapAndFeature.first ) {
      const char* name = getNameForFeature( *mapAndFeature.first,
                                            *mapAndFeature.second,
                                            infoCallback );
      bool dataToReturn = false;
      // Check if it is possible to create an outline
      if ( mapAndFeature.second->getType() ==
           TilePrimitiveFeature::bitmap ) {
         if ( mapAndFeature.second->isDrawn() ) {
            setDirectedPolyFromPixelBox( *m_outlinePolygon,
                                         getPixelBoxForBitMap( *mapAndFeature.second ) );
            m_outlinePolygon->setVisible(true);
            // All is ok - set the stuff
            res.m_selectionOutline = m_outlinePolygon;
            dataToReturn = true;
         }
      } else {
         // Not bitmap - cannot create selection outline.
         m_outlinePolygon->setVisible(false);
      }
      // Everything needs the name of the feature.
      if ( name ) {
         // Set name and also server string.
         res.m_name = name;
         // Translate the clicked point
         MC2Coordinate coord;
         if ( mapAndFeature.second->getType() == TileFeature::bitmap ) {
            // Point feature. Add the point of the feature instead of
            // the clicked coord.
            const CoordArg* coordArg =
               static_cast<const CoordArg*>
                  (mapAndFeature.second->getArg(TileArgNames::coord));
            coord.lat = coordArg->getCoord().getLat();
            coord.lon = coordArg->getCoord().getLon();
         } else {
            // Non point feature. Add the clicked coord.
            inverseTransform( coord, point );
         }
         // Get server string.
         res.m_serverString = createServerString( name,
                                                  coord,
                                                  mapAndFeature );
         dataToReturn = true;
      }
      if(dataToReturn) {
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
   // New version, the one to keep, probably.
   TileMapHandlerClickResult res;
   getInfoForFeatureAt( res, point );
   if ( ! res.m_serverString.empty() ) {
      strings.push_back( res.m_serverString );
   }
   return strings.size();
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
TileMapHandler::forceRedrawOnNextRepaint()
{  
   // Simulate that matrix has changed in order to force a redraw
   // next time the map is repainted.
   m_matrixChangedSinceDraw = true;
}

void TileMapHandler::requestPositionInterpolation() {   
   if( !m_positionInterpolationRequested &&
       m_posInterpolator->hasUsefulInterpolationData() ) {
      
      m_positionInterpolationRequested =
         m_toolkit->requestTimer( this, 10 );
   }
}

void TileMapHandler::positionInterpolationTimerExpired() {
   m_positionInterpolationRequested = 0;
   
   if( !m_posInterpolator->hasUsefulInterpolationData() ||
       !getInterpolatingPositions() )
   {
      return;
   }

   unsigned int curTime = TileMapUtil::currentTimeMillis();
   
   if( !m_posInterpolator->withinRange( curTime ) ) {
      return;
   }

   InterpolatedPosition ip =
      m_posInterpolator->getInterpolatedPosition( curTime );
      
   if ( m_interpolationCallback ) {
      m_interpolationCallback->positionInterpolated( ip.coord,
                                                     ip.velocityMPS,
                                                     ip.directionDegrees );
   }
   
   m_lastInterpolatedCenter = ip.coord;
}

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
                                BitBuffer& buffer,
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
         descr, buffer, removeFromCache, mapOK, cancelRequestedMaps );
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
         if ( m_textHandlerIdleID ) {
            m_toolkit->cancelIdle(this, m_textHandlerIdleID );
            m_textHandlerIdleID = 0;
         }
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
   } else if( id == m_positionInterpolationRequested ) {
      positionInterpolationTimerExpired();
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
            if ( ! tileMapReceived(descr, *dataBuffer, removeFromCache) ) {
               mc2log << "TMH: Tilemap could not be decoded. "
                      << "Requesting new tmfd." << endl;
               // The tilemap could not be decoded. Request a new mapdesc.
               // Request the real new one
               // Create new random characters
               m_randChars = 
                  MC2SimpleStringNoCopy(RandomFuncs::newRandString(3));
               
               // Request more => request a new tmfd.
               m_descCRCReceived = false;
            
            }
            m_garbage.checkGarbage();
         }
      } break;

      case TileMapParamTypes::BITMAP: {
         // Bitmap. Type not used.

         MC2SimpleString bmpName( bitMapDescToBitMapName( descr ) );

         bool shouldScale = false;
         std::map<MC2SimpleString, ImageRequest>::iterator itr =
            m_neededBitMaps.find(bmpName);
         
         if(itr != m_neededBitMaps.end()) {
            shouldScale = itr->second.shouldScale;
         }

         isab::BitMap* bmp =
            m_plotter->createBitMap( isab::bitMapType(0),
                                     dataBuffer->getBufferAddress(),
                                     dataBuffer->getBufferSize(),
                                     shouldScale,
                                     getDPICorrectionFactor() );

         if ( bmp != NULL ) {
            // Store the bitmap. Remove the 'B'
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
         
         // We don't want a TMFD if we are currently requesting maps,
         // since we use iterators in TileMapContainer which will be
         // invalidated when getting a new TMFD.
         if ( m_inRequestingMaps > 0 ) {
            // Release the TMFD to memory cache below
            mc2dbg << "[TMH]: Unsolicited TMFD received when"
                   << " requesting tilemaps. Releasing to memory cache "
                   << "to avoid crash." << endl;
            break;
         }
                                               
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

         // Use it if:
         // No existing TMFD       or
         // No crc received yet    or
         // New tmfd crc matches.
         bool okToUse = ( m_mapFormatDesc == NULL ) ||
                        ( !m_descCRCReceived ) ||
                        ( m_serverDescCRC == tmpNewDesc->getCRC() );
         
         if ( okToUse ) {
            mc2log << "[TMH]: TMFD is set to " << descr << endl;
            // Delete the format description now,
            // there may not be references.
            delete m_mapFormatDesc;
            m_mapFormatDesc = tmpNewDesc;
            m_mapFormatDesc->updateLayersToDisplay( m_layersToDisplay, m_acpModeEnabled );
            // Real repaint needed
            m_matrixChangedSinceDraw = true;
            (*m_tileMapCont).setMapDesc( m_mapFormatDesc );

            // Update the copyright handler with data from tmfd.
            updateCopyrightHandler( *m_mapFormatDesc );

            m_textHandler->setTextColor( m_mapFormatDesc->getTextColor() );
            sendEvent ( TileMapEvent(
                           TileMapEvent::NEW_CATEGORIES_AVAILABLE ) );
            // Save the currently used desc
            delete m_dxxBuffer;
            m_dxxBuffer = new BitBuffer( *dataBuffer );

            if( descr.c_str()[0] == 'd' ){
               // Copy the description to another buffer and save it as "dXXX".
               m_requester->release("dXXX", new BitBuffer( *dataBuffer ) );
               // Copy the description to another buffer and save it as "dYYY".
               m_requester->release("dYYY", new BitBuffer( *dataBuffer ) );
            }
            else {
               // Copy the description to another buffer and save it as "DXXX".
               m_requester->release("DXXX", new BitBuffer( *dataBuffer ) );
               // Copy the description to another buffer and save it as "DYYY".
               m_requester->release("DYYY", new BitBuffer( *dataBuffer ) );
            }

         } else {
            // The format desc was no good.
            // Check if we need to fetch a new crc.
            if ( m_mapFormatDesc != NULL &&
                 m_descCRCReceived ) {
               if ( m_serverDescCRC != tmpNewDesc->getCRC() &&
                    m_serverDescCRC != m_mapFormatDesc->getCRC() ) {
                  // Fetch new crc, since neither the old or new
                  // tmfd matched the server crc.
                  m_randChars = 
                     MC2SimpleStringNoCopy(RandomFuncs::newRandString(3));
                  m_descCRCReceived = false;
                  mc2dbg << "[TMH]: Requesting new TMFD CRC since mismatch"
                         << " between old and new tmfd against existing CRC." 
                         << endl;
               }
            }
                 
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
TileMapHandler::requestCachedTMFD()
{
   if( m_nightMode ){
      // Also request the previously saved one
      requestFromCacheOrForReal( "dXXX", DBufRequester::onlyCache );
      // Also request one from the readonly cache too.
      requestFromCacheOrForReal( "dYYY", DBufRequester::onlyCache );
   } else {
      // Also request the previously saved one
      requestFromCacheOrForReal( "DXXX", DBufRequester::onlyCache );
      // Also request one from the readonly cache too.
      requestFromCacheOrForReal( "DYYY", DBufRequester::onlyCache );
   }
}

void
TileMapHandler::requestMaps(int maxNbr)
{
   // Counter for the maxNbr
   int nbrReq = 0;

   // Request the missing format description.

   bool myTMFDHasWrongCRC = m_mapFormatDesc && m_mapFormatDesc->getCRC() != m_serverDescCRC;

   // If no CRC has arrived or we already have a TMFD with the wrong CRC 
   // - request new CRC
   // Either the CRC or the TMFD is wrong
   if ( m_descCRCReceived == false || myTMFDHasWrongCRC ) {
      requestFromCacheOrForReal(
         TileMapFormatDescCRC::createParamString( m_lang,
                                                  m_toolkit->getIDString(),
                                                  m_randChars.c_str(),
                                                  m_nightMode ) );
      mc2dbg << "[TMH]: Requesting TMFD CRC because:" << endl;
      if ( ! m_descCRCReceived ) {
         mc2dbg << "does not have a valid TMFD CRC" << endl;
      } else {
         mc2dbg << "my tmfd has wrong crc." << endl;
      }
   }

   if ( m_descCRCReceived ) {
      if ( ( m_mapFormatDesc == NULL ) ||
           ( m_mapFormatDesc->getCRC() != m_serverDescCRC ) ) {
         // The CRC was wrong
         requestFromCacheOrForReal(
            TileMapFormatDesc::createParamString(
               m_lang,
               m_toolkit->getIDString(),
               m_randChars.c_str(),
               m_nightMode ) );
         mc2dbg << "[TMH]: Requesting TMFD because:" << endl;
         if ( m_mapFormatDesc == NULL ) {
            mc2dbg << "does not have any TMFD." << endl;
         } else {
            mc2dbg << "tmfd crc mismatch." << endl;
         }
      }
   }
         
   if ( m_mapFormatDesc == NULL ) {
      requestCachedTMFD();
   }

   // Print the maps to request
   
   ScopedCounter ctr( m_inRequestingMaps );
   
   mc2dbg << "[TMH]: Starting to request missing maps." << endl; 
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
            mc2dbg << "[TMH]: Requesting " <<  pIt->getAsString() << endl;
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
   mc2dbg << "[TMH]: Finished requesting missing maps." << endl; 
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

   typedef std::map<MC2SimpleString, ImageRequest> ImageReqMap;
   
   // Request the missing bitmaps.
   // Copy the set so that there will not be trouble if it was cached.
   ImageReqMap tmpNeeded(m_neededBitMaps);
   
   // set<MC2SimpleString> tmpNeeded ( m_neededBitMaps );
   
   for ( ImageReqMap::const_iterator it = tmpNeeded.begin();
         it != tmpNeeded.end(); ++it ) {
      if ( m_bitMaps.find( it->first ) == m_bitMaps.end() ) {
         // Make temp string
         bool cached = requestFromCacheOrForReal(
            bitMapNameToDesc(it->first) );
         if ( ! cached ) {
            if ( nbrReq++ > maxNbr ) {
               // No more requests allowed.
               return;
            }       
         }
      } else {
         // Already there. An error really.
         m_neededBitMaps.erase( it->first );
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
   
   if( getInterpolatingPositions() ) {
      requestPositionInterpolation();
   }
   
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
TileMapHandler::getOrRequestBitMap( const MC2SimpleString& bitMapName,
                                    bool shouldScale )
{
   bitMapMap_t::const_iterator it = m_bitMaps.find( bitMapName );
   if ( it != m_bitMaps.end() ) {
      return it->second;
   } else {
      // Will be requested later.
      ImageRequest r = { shouldScale };
      m_neededBitMaps[bitMapName]  = r;
   }
   
   return NULL;
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
   const TileFeature* parent = curMap.getParentFeature(&prim);

   if ( parent != NULL &&
        ( m_disabledParentTypes.find(parent->getDrawType()) !=
          m_disabledParentTypes.end() ) ) {
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
//    if ( ! screenBBox.overlaps( bbox ) ) {
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
TileMapHandler::plotPolygon( const MC2BoundingBox& bbox,
                             TileMap& tilemap,
                             const TilePrimitiveFeature& feat,
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
   MC2_ASSERT( prim.getType() == TileFeature::line );
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
TileMapHandler::plotPolygonOrLineFeature( const MC2BoundingBox& bbox,
                                          TileMap& tilemap,
                                          const TilePrimitiveFeature& prim,
                                          int pass,
                                          int moving )
{
   int nbrDrawn = 0;
   //MC2_ASSERT( int(curPrim->getType()) < 0 );
   
   const SimpleArg* color = static_cast<const SimpleArg*>(
                                         (prim.getArg(TileArgNames::color)));
      
   if ( prim.getType() == TilePrimitiveFeature::polygon ) {
      mc2dbg8 << "[TMH]: Filling " << endl;
      if ( color && ( pass == 0 ) ) {
         nbrDrawn += plotPolygon( bbox, tilemap, prim, color->getValue( m_scaleIndex ) );
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
TileMapHandler::prepareCoordinates( TileMap& tileMap,
                                    TilePrimitiveFeature& feat,
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

   // For now we will use the same amount of points as in the
   // pointvector. Could be different when clipping etc is implemented.
   MC2Point curPoint(0,0);
   // Version with pre-calculated cosLat.
   // The size of the VectorProxy should be the same as the size of
   // the coords vector, or else we're ...
  
   MC2Point lastPoint( MAX_INT32, MAX_INT32 );

   uint32 nbrCoords = coords->size();
   
   m_realScreenCoords.reserve( nbrCoords );
   vector<MC2Coordinate> clipCoords;

#ifndef SLIM_TILE_COORDS
   if ( feat.getType() == TileFeature::polygon &&
        ClipUtil::clipPolyToBBoxFast( bbox,
                                    clipCoords,
                                    &(*coords->begin()),
                                    &(*coords->end()))) 
#else
   if ( feat.getType() == TileFeature::polygon &&
        ClipUtil::clipPolyToBBoxFast( bbox,
                                    clipCoords,
                                    &(*coords->begin(tileMap)),
                                    &(*coords->end(tileMap)))) 
#endif     
   { // Bracket is here in order to avoid confusing for vim.
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
#ifndef SLIM_TILE_COORDS
      CoordsArg::const_iterator the_end ( coords->end() );
      for ( CoordsArg::const_iterator it ( coords->begin() );
#else
      CoordsArg::const_iterator the_end ( coords->end(tileMap) );
      for ( CoordsArg::const_iterator it ( coords->begin(tileMap) );
#endif
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
      case TilePrimitiveFeature::line:
      case TilePrimitiveFeature::polygon:
         if ( prepareCoordinates( curMap, curPrim, bbox) == 0 ) {
            return 0;
         }
         break;
   }
   
   switch ( curPrim.getType() ) {
      case TilePrimitiveFeature::line:
      case TilePrimitiveFeature::polygon:
         curNbrDrawn =
            plotPolygonOrLineFeature( bbox,curMap, curPrim, pass, moving );
         break;
      case TilePrimitiveFeature::bitmap:
         if ( ! checkScale( curPrim ) ) {
            return 0;
         }
         curNbrDrawn = plotBitMapFeature( curMap, curPrim, bbox,
                                          pass, moving  );
         break;
      case TilePrimitiveFeature::circle:
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
   
   m_plotter->setPenColor(0,0,0);
   m_plotter->drawText( *str, m_progressIndicatorBox.getCenterPoint() );
   
   m_plotter->deleteString( str );
}

void 
TileMapHandler::plotCopyrightString( const char* copyrightString ) 
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

   m_plotter->setPenColor( m_mapFormatDesc->getTextColor() );
   STRING* str = m_plotter->createString( 
         copyrightString );
   
   m_plotter->drawTextAtBaselineLeft( *str, m_copyrightPos );
   m_plotter->deleteString( str );
}

void 
TileMapHandler::plotLastCopyrightString()
{
   char copyrightString[ 80 ];
   m_copyrightHandler.getCopyrightString( copyrightString, 80 );
   plotCopyrightString( copyrightString );
}

inline void
TileMapHandler::plotWhatWeGotInBBox( const MC2BoundingBox& bbox,
                                     int skipOutlines,
                                     int lowQualityDrawing,
                                     int& nbrFeatures,
                                     int& nbrDrawn )
{
   
   if ( m_mapFormatDesc == NULL ) {
      return;
   }
       
   m_plotter->enableLowQualityDrawing( lowQualityDrawing );

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
   
   m_plotter->enableLowQualityDrawing( false );
}

void
TileMapHandler::plotWhatWeGot()
{
   if ( m_mapFormatDesc == NULL ) {
      return;
   }
   
   int nbrDrawn = 0;
   int nbrFeatures = 0;
   
   mc2dbg << "[TMH]: Plotting STARTS" << endl;

   // Only redraw for outlines if we didn't do it last time.
   bool detailRepaint = m_detailRepaintTimerHasBeenRun &&
                              !(m_outlinesDrawnLastTime);

   m_movementHelper.updateIfToDrawOutlines( detailRepaint );

   // Test if it is possible to divide the bounding boxes into
   // smaller parts and draw several times.
   
   // Divide the bounding box into several new ones
   vector<MC2BoundingBox> bboxVec;
   getDrawingBBoxes( bboxVec );
   
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

      if ( m_3dOn ) { 
         // If 3d is on, draw a horizon at the top of 
         // the screen.
         isab::Rectangle screenRect;
         m_plotter->getMapSizePixels( screenRect );
         uint32 horizTopColor = m_mapFormatDesc->getHorizonTopColor();
         uint32 horizBottomColor = m_mapFormatDesc->getHorizonBottomColor();

         std::vector<uint32> colors;
         ImageBlender::blendImage( horizTopColor, horizBottomColor, m_horizonHeight, colors );

         std::vector<MC2Point> points;
         points.push_back( MC2Point( 0, 0 ) );
         points.push_back( MC2Point( screenRect.getWidth(), 0) );
         for ( uint32 i = 0; i < colors.size(); ++i ) {
            points[0].getY() = i;
            points[1].getY() = i;
            m_plotter2d->drawPolyLineWithColor( points.begin(), points.end(), colors[i], 1 );
         }
      }

      m_overlapDetector.clear();
      m_scaleIndex = 
         m_mapFormatDesc->getScaleIndexFromScale( 
            uint32(getDPICorrectedScale()) );
      
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
      
      // If the map should be rendered in low quality (read fast) mode.
      bool lowQualityDrawing = !drawOutlines;
    
      if ( get3dMode() && !getOutlinesIn3dEnabled() ) {
         drawOutlines = false;
      }

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
                               lowQualityDrawing,
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
  
   char copyrightString[ 80 ];
   m_copyrightHandler.getCopyrightString( 
      bboxVec, copyrightString, 80 );
   plotCopyrightString( copyrightString );

   // This can be drawn on a separate bitmap
   drawUserDefinedFeatures();
   
   {
      if ( false && m_tmkTimingInfo != NULL ) {
         vector<MC2SimpleString> texts;
         char tmpStr[80];
         sprintf( tmpStr, "R: %ld", (long int)m_tmkTimingInfo->m_nbrRunsLeft );
         texts.push_back( tmpStr );
         if( m_nightMode ){
            sprintf( tmpStr, "d: %ld", (long int)m_tmkTimingInfo->m_nbrRunsDone );
         }
         else{
            sprintf( tmpStr, "D: %ld", (long int)m_tmkTimingInfo->m_nbrRunsDone );
         }
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
   
   mc2log << "[TMH]: Finished plotting   ENDS Number features "
          << nbrFeatures << ", Number of points: " << nbrDrawn << endl;

#ifdef ENABLE_MAPPERFORMANCE_DEBUGGING
   // Draw fps.
   drawFps();
#endif

#ifdef ENABLE_INTERPOLATION_DEBUGGING
   drawInterpolationDebugInfo();
#endif

   m_plotter->drawingCompleted();      

}

void TileMapHandler::drawPoint( MC2Point center,
                                unsigned int color,
                                unsigned int size)
{
   std::vector<MC2Point> screenCoords;
   
   center.getX() -= size / 2;
   screenCoords.push_back( center );
   center.getX() += size ;
   screenCoords.push_back( center );
   
   m_plotter->drawPolyLineWithColor( screenCoords.begin(),
                                     screenCoords.end(), 
                                     color,
                                     size );

   screenCoords.clear();
}

void TileMapHandler::drawPoint( MC2Coordinate center,
                                unsigned int color,
                                unsigned int size)
{
   MC2Point curPoint(0,0);
   
   transformPointInternalCosLat( curPoint.getX(),
                                 curPoint.getY(),
                                 center );
   
   drawPoint( curPoint, color, size );
}

void TileMapHandler::drawInterpolationDebugInfo()
{
   drawFps();


   std::vector<MC2Point> screenCoords;
   MC2Point curPoint(0,0);

   // /**
   //  *   Draw stored information
   //  */  

   // for( InterpolationDebugVector::iterator iI
   //         = m_interpolationDebugInfo.begin(); iI !=
   //         m_interpolationDebugInfo.end();
   //      iI++ )
   // {
    
   //    transformPointInternalCosLat( curPoint.getX(),
   //                                  curPoint.getY(),
   //                                  iI->coord );
      
   //    screenCoords.push_back( curPoint );
   // }

   // if( !screenCoords.empty() ) {
   //    m_plotter->drawPolyLineWithColor( screenCoords.begin(),
   //                                      screenCoords.end(), 
   //                                      0x20cc20,
   //                                      2 );
   // }

   // char num[64];
   
   // for( InterpolationDebugVector::iterator iI
   //         = m_interpolationDebugInfo.begin(); iI !=
   //         m_interpolationDebugInfo.end();
   //      iI++ )
   // {
    
   //    transformPointInternalCosLat( curPoint.getX(),
   //                                  curPoint.getY(),
   //                                  iI->coord );

   //    sprintf( num, "%d", iI->timeDiffMillis );
   //    drawDebugText( num, curPoint );
   // }
   
   /**
    *   Draw interpolation path.
    */

   std::vector<PositionInterpolator::InterpolationNode> nodes =
      m_posInterpolator->getInterpolationVector();
   
   if ( nodes.empty() ) {
      return;
   }

#ifndef __unix__
   nodes.clear(); // We still want to see them on linux, where we have no route.
#endif
   
   
   for ( vector<PositionInterpolator::InterpolationNode>::const_iterator it =
            nodes.begin(); it != nodes.end(); ++it )
   {
      
      transformPointInternalCosLat( curPoint.getX(),
                                    curPoint.getY(),
                                    it->coord );
      
      screenCoords.push_back( curPoint );
   }

   if( !screenCoords.empty() ) { 
      m_plotter->drawPolyLineWithColor( screenCoords.begin(),
                                        screenCoords.end(), 
                                        0x20cc20,
                                        2 );      
   }
   
   screenCoords.clear();

   char num[64];
   MC2Point centerPoint = getCenterPoint();
   centerPoint.getX() = 65;
      
   sprintf( num, "Dx: %.02fm", m_posInterpolator->getPositionDeltaMeters() );
   drawDebugText( num, centerPoint );
   centerPoint.getY() += 20;
   
   sprintf( num, "Cl: %.02fkmph",
            m_posInterpolator->getPrevCalcSpeedKMPH() );
   drawDebugText( num, centerPoint );
   centerPoint.getY() += 20;
   
   sprintf( num, "Cr: %.02fkmph",
            m_posInterpolator->getPrevCorrectSpeedKMPH() );
   drawDebugText( num, centerPoint );

   centerPoint.getY() += 20;

   char config = m_posInterpolator->getConfiguration() + 'A';
   
   sprintf( num, "Configuration: %c", config );

   drawDebugText( num, centerPoint );
}

void TileMapHandler::drawDebugText( const char* text,
                                    const MC2Point& position )
{         
   STRING* str = m_plotter->createString( text );
#ifdef __SYMBIAN32__
   STRING* fontName = m_plotter->createString("Swiss");
#elif UNDER_CE
   STRING* fontName = m_plotter->createString("tahoma");
#else
   STRING* fontName =
      m_plotter->createString("-*-times-*-r-*-*-12-*-*-*-*-*-*-*");
#endif
   
   m_plotter->setPenColor( 0, 0, 0 );   
         
   m_plotter->setFont( *fontName, 18 );
   m_plotter->deleteString( fontName );
   m_plotter->drawText( *str, position );
   m_plotter->deleteString( str );
}

// Inlined so that it can be auto-removed in releases.
inline void
TileMapHandler::printDebugStrings(const vector<MC2SimpleString>& strings,
                                  const MC2Point& pos )
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
   
   MC2Point curPoint = pos;

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

inline void
TileMapHandler::printDebugStrings(const vector<MC2SimpleString>& strings )
{
   isab::Rectangle lastScreenSize;
   m_plotter->getMapSizePixels(lastScreenSize);
   PixelBox pix(lastScreenSize);
   MC2Point curPoint( pix.getCenterPoint() );
   // Start at y = height-24
   curPoint.getY() = lastScreenSize.getHeight() - 24;
   printDebugStrings( strings, curPoint ); 
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
   if ( m_3dOn ) {
      return;
   }

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
   
   // Not drawn until proven guilty.
   userFeat.setDrawn( false );
  
   MapPlotter* plotter;
   if ( userFeat.getAlways2d() ) {
      // If the feature always should be plottered in 2d, use
      // the 2d plotter.
      plotter = m_plotter2d;
   } else {
      // If feature should be in 3d when map is in 3d, use the
      // activated plotter, if in 3d mode the 3d plotter will be used
      // otherwise the 2d plotter will be used.
      plotter = m_plotter;
   }

   switch ( userFeat.getType() ) {
      // Filled or unfilled polygon.
      case UserDefinedFeature::directed_poly: {
         if ( ! userFeat.isVisible() ) {
            return;
         }
         // Update the screen coordinates.
         bool mode3d = m_3dOn;
         m_3dOn = false;
         // Najs.
         userFeat.updateScreenCoords(*this);
         m_3dOn = mode3d;
         DirectedPolygon* poly = static_cast<DirectedPolygon*>(&userFeat);
         plotter->setLineWidth( poly->getPenSize() );
         userFeat.setDrawn( true );
         if ( poly->isFilled() ) {
            plotter->setFillColor( poly->getColor() );
            plotter->drawPolygon( poly->getScreenCoords().begin(),
                                  poly->getScreenCoords().end() );
         } else {
            plotter->setPenColor( poly->getColor() );
            plotter->drawPolyLine( poly->getScreenCoords().begin(),
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
               getOrRequestBitMap( bmp->getBitMapName(),
                                   bmp->shouldScale() );
            
            if ( ! userFeat.isVisible() ) {
               return;
            }
            if ( bitMap != NULL ) {
               int width = bmp->getWidth();
               int height = bmp->getHeight();
               
               if(plotter && plotter->supportsBitMapScaling() &&
                  width != 0 && height != 0)
               {
                  plotter->updateBitmapDimensions(bitMap,
                                                  bmp->getWidth(),
                                                  bmp->getHeight());
               }
               
               // Update the screen coordinates.
               userFeat.updateScreenCoords(*this);
               userFeat.setDrawn( true );
               plotter->drawBitMap( bmp->getScreenCoords().front(),
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
         scaleFeat->draw( *this, *plotter, 
                          m_mapFormatDesc->getTextColor() );
         userFeat.setDrawn( true );
      }
      break;
       
      default:
         // Do you want to know what it is? No, I'm fine.
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
   sendEvent(TileMapEvent(TileMapEvent::USER_DEFINED_FEATURES_REDRAWN));
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
      m_mapFormatDesc->updateLayersToDisplay( m_layersToDisplay, m_acpModeEnabled );
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
      MC2_ASSERT( highlightFeature->getType() == TileFeature::bitmap );

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
   plotLastCopyrightString(); 
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
   plotLastCopyrightString(); 
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
   plotLastCopyrightString(); 
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

void 
TileMapHandler::setNightModeL( bool aOn )
{
   if( aOn != m_nightMode ){
      m_nightMode = aOn;

      requestCachedTMFD();

      m_descCRCReceived = false;
      requestMaps( 1 );      
   }
}

void 
TileMapHandler::set3dMode( bool on )
{
   MC2Coordinate oldCenter = getCenter();
   if ( on ) {
      m_plotter = m_plotter3d;
   } else {
      m_plotter = m_plotter2d;
   }
   if ( m_3dOn != on ) {
      m_3dOn = on;
      forceRedrawOnNextRepaint();
      requestRepaint();
   }

   setCenter( oldCenter );
}

bool 
TileMapHandler::get3dMode( )
{
   return m_3dOn;
}

void
TileMapHandler::setOutlinesIn3dEnabled( bool enabled )
{
   m_outlinesIn3dEnabled = enabled;
}

bool
TileMapHandler::getOutlinesIn3dEnabled( ) const
{
   return m_outlinesIn3dEnabled;
}

void
TileMapHandler::setHorizonHeight( uint32 height )
{
   m_plotter3d->setHorizonHeight( height );
  
   bool changed = m_horizonHeight != height;
   m_horizonHeight = height;
   if ( changed && get3dMode() ) {
      forceRedrawOnNextRepaint();
      requestRepaint();
   }
}

// XXX: Remove this method once the 3d parameters are tweaked.
Settings3D&
TileMapHandler::getSettings3D()
{
   return m_plotter3d->m_settings;
}

void
TileMapHandler::drawFps()
{
   uint32 currTimeStamp = TileMapUtil::currentTimeMillis();
   uint32 millisSinceDraw = currTimeStamp - m_lastDrawTimeStamp;
   m_lastDrawTimeStamp = currTimeStamp;
   double fps = 0;
   if ( millisSinceDraw != 0 ) {
      fps = 1000.0 / millisSinceDraw;
   }

   char tmpStr[40];
   sprintf( tmpStr, "fps: %.2f, s: %.2f", fps, getScale()  );
   STRING* str = m_plotter->createString( tmpStr );
#ifdef __SYMBIAN32__
   STRING* fontName = m_plotter->createString("Swiss");
#elif UNDER_CE
   STRING* fontName = m_plotter->createString("tahoma");
#else
   STRING* fontName =
      m_plotter->createString("-*-times-*-r-*-*-12-*-*-*-*-*-*-*");
#endif
   m_plotter->setFont( *fontName, 18 );
   m_plotter->deleteString( fontName );
   m_plotter->setPenColor( 0, 0, 0 );
   m_plotter->drawTextAtBaselineLeft( *str, MC2Point(60,60) );
   m_plotter->deleteString( str );
}

void 
TileMapHandler::updateCopyrightHandler( const TileMapFormatDesc& tmfd ) 
{
   if ( tmfd.getCopyrightHolder() != NULL ) {
      // Set the CopyrightHolder to the CopyrightHandler.
      m_copyrightHandler.setCopyrightHolder( tmfd.getCopyrightHolder() );
   } else {
      // Set the old version of copyright data from TMFD.
      m_copyrightHandler.setStaticCopyrightString( tmfd.getStaticCopyrightString() );
   }
}

bool
TileMapHandler::isInitialized() const
{
   return m_mapFormatDesc != NULL;
}

bool 
TileMapHandler::isACPModeEnabled() const
{
   return m_acpModeEnabled;
}

void 
TileMapHandler::setACPModeEnabled( bool enable )
{
   m_acpModeEnabled = enable;
   if ( m_mapFormatDesc ) {
      // A race can occure here when having no internet access point, entering the map and 
      // imediately goes back and enters settings. This function will be called and the
      // m_mapFormatDesc has not yet been read from the cache and therefore is NULL.
      const bool visibility_changed =
         m_mapFormatDesc->updateLayersToDisplay( m_layersToDisplay, m_acpModeEnabled );
      if ( visibility_changed ) {
         updateParams();
         requestRepaint();
      }
   }
}

PositionInterpolator* TileMapHandler::getPositionInterpolator()
{
   return m_posInterpolator;
}

void TileMapHandler::setInterpolationCallback( InterpolationCallback* callback )
{
   m_interpolationCallback = callback; 
}

void TileMapHandler::clearInterpolationCallback( InterpolationCallback* callback )
{
   MC2_ASSERT( callback == m_interpolationCallback );
   m_interpolationCallback = NULL;
}

bool TileMapHandler::getInterpolatingPositions() const
{
   return m_interpolatingPositions;
}

void TileMapHandler::setInterpolatingPositions( bool enabled )
{
   m_interpolatingPositions = enabled;
}

