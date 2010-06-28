/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#include <math.h>
#include <vector>
#include <algorithm>

#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "Tracking.h"
#include "TraceMacros.h"

#include "Buffer.h"
#include "MsgBufferEnums.h"
#include "Module.h"
/* #include "Matches.h" */
#include "NavServerComEnums.h"
#include "MapEnums.h"
/* #include "GuiProt/GuiProtEnums.h" */
/* #include "GuiProt/Favorite.h" */
/* #include "ErrorModule.h" */
#include "NavPacket.h"

#include "GuiProt/ServerEnums.h"
#include "GuiProt/ServerEnumHelper.h"
#include "NavServerCom.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "Buffer.h"
#include "TrackPIN.h"
#include "NavPacket.h"
#include "nav2util.h"
#include "TimeUtils.h"

using namespace std;

namespace isab {

#define TRACK_DATA_VER 1
#define TRACK_DATA_VER_2 2


   TrackingPoint::operator PositionState() const {
      PositionState s;
      s.lat = lat();
      s.lon = lon();
      s.speed = speed();
      s.heading = heading();      
      return s;
   }


   Tracking::Tracking( ModuleQueue* queue,
                       NavServerComProviderPublic* navServComProvPubl,
                       ParameterProviderPublic* paramProvPubl )
         : m_queue( queue ), m_nscProvPubl( navServComProvPubl ),
           m_paramProvPubl( paramProvPubl ),  m_positionTime( 0 ),
           m_trackingTime( 0 ), m_trackingTimer( 0 ),
           m_startupCompletete( false ), m_initializedPINs( false ),
           m_pinData( NULL ), m_pinSyncID( 0 ), m_distcounting( true ),
           m_trackingID( 0 ), m_hastracking( false ), 
           m_lastSentTime( TimeUtils::time( NULL ) ), m_hasFleet( false ),
           m_timeDiffSet( false ), m_timeDiff( 0 )
   {
   }


   Tracking::~Tracking() {
      delete m_nscProvPubl;
      delete m_pinData;
   }


   void
   Tracking::decodedStartupComplete() {

//      TRACE_FUNC1( "Tracking decodedStartupComplete" );
      m_paramProvPubl->getParam( ParameterEnums::TR_trackLevel );
      m_paramProvPubl->getParam( ParameterEnums::TR_trackdata );
      m_paramProvPubl->getParam( ParameterEnums::TR_trackPIN );
      m_paramProvPubl->getParam( ParameterEnums::NSC_userRights );
   }


   void
   Tracking::decodedShutdownPrepare( int16 /*upperTimeout*/ )
   {
      // Stop timer
      m_queue->removeTimer( m_trackingTimer );
      m_trackingTimer = 0;
   }


   bool 
   Tracking::isdecodedTimerOk( uint16 timerID ) {
      return (m_trackingTimer != 0 && timerID == m_trackingTimer);
   }
   
   
   void 
   Tracking::decodedExpiredTimer( uint16 timerID ) {
      if ( m_trackingTimer == timerID ) {
//         TRACE_FUNC1( "Tracking timer received" );
         m_queue->removeTimer( m_trackingTimer );
         m_trackingTimer = 0;
         defineTimer();

         uint32 now = TimeUtils::time( NULL );
         uint32 timesincelast = now - m_lastSentTime;
         if ( now < m_lastSentTime ) {
            timesincelast = now;
         }
         
         if ( m_trackingID == 0 && (timesincelast +3 >= m_trackingTime ) )
         {
            // No outstanding tracking request and enough time passed

            // Sync store and sending if last stored is old 
            if ( now  > m_lastStoredCoord.time() + m_positionTime/4 &&
                 m_lastCoord.lat() != MAX_INT32 && 
                 now < m_lastCoord.time() + 3 ) 
            {
               storePos( m_lastCoord, now - 1 );
            }

            // Send stored coords
            NParamBlock params;
            for ( uint32 i = 0 ; i < m_points.size() ; ++i ) {
               NParam& p = params.addParam( NParam( 5200 ) );
               p.addInt32( m_points[ i ].lat() );
               p.addInt32( m_points[ i ].lon() );
               p.addInt32( m_points[ i ].dist() );
               p.addUint16( m_points[ i ].speed() );
               p.addUint16( m_points[ i ].heading() );
#ifndef USE_GPS_TIME
               uint32 timeToSend = now - m_points[ i ].time();
               if ( m_points[ i ].time() > now ) {
                  timeToSend = now;
               }
               if ( m_timeDiffSet ) {
//                  TRACE_DBG( "Stored time %"PRIu32" Diff %"PRId32, m_points[ i ].time(), m_timeDiff );
                  timeToSend = uint32(int32(m_points[ i ].time()) + m_timeDiff);
               } else {
//                  TRACE_DBG( "m_timeDiffSet NOT set" );
               }
               p.addUint32( timeToSend );
#else
               p.addUint32( m_points[ i ].gpsTime() );
#endif
               p.addString( "" );
            }
            // Resend PIN change? (If error on prevoius try.)
            bool sendPINs = false;
            if ( m_initializedPINs && m_pinSyncID == 0 &&
                 changedPINs( getPINDataCRC() ) )
            {
               sendPINs = true;
               addPINData( params );
            }
            uint32 src = 0;
            if ( m_points.size() > 0 ) {
//               TRACE_FUNC1( "Tracking timer received sending coords!" );
               src = m_nscProvPubl->sendRequest( 
                  navRequestType::NAV_TRACK_REQ, params );
               m_lastSentTime = now;
            }

            for ( uint32 j = 0 ; j < m_points.size() ; ++j ) {
               m_points[ j ].src() = src;
            }
            m_trackingID = src;
            if ( sendPINs ) {
               m_pinSyncID = src;
            }
         } // End if trackingID == 0
      } // End if timeID is trackingTimer
   }


   void
   Tracking::decodedPositionState( 
      const PositionState &p ) 
   {
//      TRACE_FUNC1( "Position received" );
      uint32 now = TimeUtils::time( NULL );

      if ( m_positionTime > 0 && p.lat != MAX_INT32 && 
           (m_hastracking || m_hasFleet) ) 
      {
         // First add dist (from last coord stored) using this coord and 
         // m_last if last not MAX_INT32 (If dist > 100m or something.)
         if ( m_lastCoord.lat() != MAX_INT32 && m_distcounting ) {
            // We can calculate last to this
            uint32 thisDist = distancePoints( 
               m_lastCoord.lat(), m_lastCoord.lon(), p.lat, p.lon );
            // Sanity. 180 and 100000 in constants
            if ( now - m_lastCoord.time() < 180 &&
                 thisDist < 100000 )
            {
               if ( m_lastCoord.dist() == MAX_UINT32 ) {
                  m_lastCoord.dist() = thisDist;
               } else {
                  m_lastCoord.dist() += thisDist;
               }
            } else {
               // Too long time/distance between coords
               m_lastCoord.dist() = MAX_UINT32;
               // Set m_distcounting to false (reset when storing coord)
               m_distcounting = false;
            }
         } // End if m_lastCoord is valid and m_distcounting is true

         // Check if time to store another coord (m_lastStoredCoordTime?)
         if ( now >= (m_lastStoredCoord.time() + m_positionTime) ) {
            storePos( p, now );   
         }

      } // Else we don't use Tracking or not ok coord

      // Store this locally for use with next coord
      m_lastCoord.lat()      = p.lat;
      m_lastCoord.lon()      = p.lon;
      m_lastCoord.heading()  = p.heading;
      m_lastCoord.speed()    = p.speed;
      m_lastCoord.time()     = now;
      m_lastCoord.gpsTime()  = p.time.millis();
   }


   bool
   Tracking::isdecodedParamNoValueOk( uint32 param ) {
      if ( param == ParameterEnums::TR_trackLevel ||
           param == ParameterEnums::TR_trackdata ||
           param == ParameterEnums::TR_trackPIN ||
           param == ParameterEnums::NSC_userRights )
      {
         return true;
      } else {
         return false;
      }
   }


   void
   Tracking::decodedParamNoValue( uint32 param ) {
      switch ( param ) {
         case ParameterEnums::TR_trackLevel :
//            TRACE_FUNC1( "Tracking got NO param for trackLevel" );
            if ( !m_hasFleet ) {
               m_trackingTime = 0;
               m_positionTime = 0;
               if ( m_trackingTimer != 0 ) {
                  defineTimer();
               }
            }
            break;

         case ParameterEnums::TR_trackdata :
//            TRACE_FUNC1( "Tracking got NO param for trackdata" );
            // Ok, no data.
            if ( !m_startupCompletete ) {
               defineTimer();
            }
            m_startupCompletete = true;

            break;

         case ParameterEnums::TR_trackPIN :
//            TRACE_FUNC1( "Tracking got NO param for trackPIN" );
            // Ok, no PIN set.
            m_initializedPINs = true;
            break;

         default:
            // Param we don't care about
            break;
      }
   }


   bool
   Tracking::isdecodedParamValueOk( uint32 param ) {
      return isdecodedParamNoValueOk( param );
   }


   void
   Tracking::decodedParamValue( uint32 param, const int32* data, 
                                int32 numEntries )
   {
//      TRACE_FUNC1( "Tracking got int param" );
      switch ( param ) {
         case ParameterEnums::TR_trackLevel :
//            TRACE_FUNC1( "Tracking got int param trackLevel!" );
            if ( m_hasFleet ) { // Then don't set
               break;
            }
            switch ( *data ) {
               case GuiProtEnums::tracking_level_none :
                  m_trackingTime = 0;
                  m_positionTime = 0;
                  break;
               case GuiProtEnums::tracking_level_minimal :
                  m_trackingTime = 24*60*60;
                  m_positionTime = 15*60;
                  break;
               case GuiProtEnums::tracking_level_log :
                  m_trackingTime = 2*60*60;
                  m_positionTime = 5*60;
                  break;
               case GuiProtEnums::tracking_level_log_often :
                  m_trackingTime = 30*60;
                  m_positionTime = 2*60;
                  break;
               case GuiProtEnums::tracking_level_often :
                  m_trackingTime = 5*60;
                  m_positionTime = 1*60;
                  break;
               case GuiProtEnums::tracking_level_live :
                  m_trackingTime = 1*60;
                  m_positionTime = 30;
                  break;
               default:
                  m_trackingTime = 0;
                  m_positionTime = 0;
                  break;
            }
            defineTimer();
            break;
         case ParameterEnums::NSC_userRights : {
//            TRACE_FUNC1( "Got UserRights param" );
            UserRights r;
            r.setUserRights( data, numEntries );
            m_hastracking = r.hasUserRight( GuiProtEnums::UR_POSITIONING );
//            TRACE_DBG( "POS %d", m_hastracking );
            bool changedTime = false;
            if ( r.hasUserRight( GuiProtEnums::UR_FLEET ) ) {
               m_hasFleet = true;
               const uint32 fleetTime = 1*60;
               if ( m_trackingTime != fleetTime ) {
                  changedTime = true;
               }
               m_trackingTime = fleetTime;
               m_positionTime = fleetTime;
            } else if ( m_hasFleet ) {
               m_hasFleet = false;
               // Changing from fleet. Set the Level again.
               m_paramProvPubl->getParam( ParameterEnums::TR_trackLevel );
            }
            if ( m_trackingTimer == 0 || changedTime ) {
               defineTimer();
            }
         }  break;

         default:
            // Param we don't care about
            break;
      }
   }


   void
   Tracking::decodedParamValue( uint32 param, const uint8* data, 
                                int size )
   {
      switch ( param ) {
         case ParameterEnums::TR_trackdata : {
            // Load the stored track points
            Buffer buff( const_cast<uint8*>( data ), size, size );
            uint8 ver = buff.readNext8bit();
            if ( !m_startupCompletete && 
                 (ver == TRACK_DATA_VER || ver == TRACK_DATA_VER_2) ) 
            {
               m_startupCompletete = true;
               TrackingPoints newPoints;
               m_points.swap( newPoints );
               TrackingPoint p;
               uint32 now = TimeUtils::time( NULL );
               while ( buff.remaining() >= 20 ) {
                  p.lat()     = buff.readNextUnaligned32bit();
                  p.lon()     = buff.readNextUnaligned32bit();
                  p.dist()    = buff.readNextUnaligned32bit();
                  p.heading() = buff.readNextUnaligned16bit();
                  p.speed()   = buff.readNextUnaligned16bit();
                  p.time()    = buff.readNextUnaligned32bit();
                  if ( ver == TRACK_DATA_VER ) {
                     p.gpsTime() = p.time();
                  } else {
                     p.gpsTime() = buff.readNextUnaligned32bit();
                  }
                  m_points.push_back( p );
//                  TRACE_FUNC1( "Loaded stored coord" );
               }
               if ( !m_points.empty() ) {
                  // Sanity on times
                  uint32 lastTime = m_points.back().time();
                  uint32 firstTime = m_points.front().time();

                  if ( firstTime > now || lastTime > now ) {
                     // Ok, set new times
                     uint32 diffTime = lastTime - firstTime;
                     if ( firstTime > lastTime ) {
                        diffTime = 0;
                     }
                     uint32 theTime = now - diffTime;
                     diffTime /= m_points.size();
                     for ( uint32 i = 0 ; i < m_points.size() ; ++i ) {
                        m_points[ i ].time() = theTime;
                        theTime += diffTime;
                     }
                  }

                  // Set last stored time too (first stored point oldest)
                  m_lastStoredCoord.time() = m_points.front().time();
                  m_lastSentTime = m_points.front().time();
               }
               m_points.insert( m_points.end(), 
                                newPoints.begin(), newPoints.end() );
               defineTimer();
            }
            buff.releaseData();
         }  break;

         case ParameterEnums::TR_trackPIN : {
            uint32 crc = 0;
            if ( m_pinData != NULL ) {
               crc = m_pinData->getCRC();
            }
            //TRACE_DBG( "Got trackPIN oldcrc %"PRId32, crc );
            if ( m_pinSyncID == 0 ) {
               delete m_pinData;
               m_pinData = new TrackPINList( data, size );
               //m_pinData->TRACE();
            } else {
               // Sync sent have to wait for it, skipping this
               // But it is set in paramfile so GO NUTS!!!!
               // Changing back to old
               Buffer buf;
               if ( m_pinData != NULL ) {
                  m_pinData->packInto( &buf );
               }
               //TRACE_DBG( "GO NUTS!" );
               m_paramProvPubl->setParam(
                  ParameterEnums::TR_trackPIN,
                  (uint8 *)buf.accessRawData( 0 ), buf.getLength() );
            }
            if ( !m_initializedPINs ) {
               // From paramfile
               m_initializedPINs = true;
            } else {
               newPINCRC( crc );
            }
         }  break;

         default:
            // Param we don't care about
            break;
      }      
   }


   void
   Tracking::reply( const uint8* data, int size, uint32 src ) {
      //TRACE_DBG( "reply size %d src %"PRIu32, size, src );
      // Ok to remove track points
      // XXX: If reply is not ok?
      TrackingPoints::iterator startIndex = m_points.end();
      TrackingPoints::iterator stopIndex = m_points.begin();
      for ( TrackingPoints::iterator i = m_points.begin() ; 
            i != m_points.end() ; ++i ) {
         if ( (*i).src() == src ) {
            // Ok to remove
            if ( startIndex == m_points.end() ) {
               startIndex = i;
            }
            stopIndex = i;
         } else if ( startIndex != m_points.end() ) {
            // End of points with same src
            break;
         }
      }
      //TRACE_DBG( "reply " );
      if ( startIndex != m_points.end() ) {
         m_points.erase( startIndex, 
                         stopIndex + 1 );
         setTrackDataParam();
      }
      //TRACE_DBG( "reply " );

      // Check if this is the pin sync reply
      if ( src == m_pinSyncID ) {
         //TRACE_DBG( "PIN REPLY" );
         m_pinSyncID = 0;
      }
      if ( src == m_trackingID ) {
         m_trackingID = 0;
      }

      // Modify m_pinData
      //TRACE_DBG( "Got reply %d", NavReplyPacket::getRequestType( data, size ) );
      if ( NavReplyPacket::getRequestType( data, size ) == 
           navRequestType::NAV_TRACK_REPLY ) 
      {
         //TRACE_DBG( "Got TRACK! reply %d", NavReplyPacket::getRequestType( data, size ) );
         NavReplyPacket r( data, size );
         uint32 crc = 0;
         uint32 origSize = 0;
         if ( m_pinData != NULL ) {
            crc = m_pinData->getCRC();
            origSize = m_pinData->size();
         }
         if ( m_pinData != NULL ) {
            //TRACE_DBG( "Calling sync! " );
            sync( &r );
            //TRACE_DBG( "sync done!" );
         } else {
            m_pinData = new TrackPINList( );
            //TRACE_DBG( "New PIN data!" );
            // 5303 PINAdd    byte array 
            if ( r.getParamBlock().getParam( 5303 ) != NULL ) {
               const NParam* p = r.getParamBlock().getParam( 5303 );
               uint32 pos = 0;
               while ( pos < p->getLength() ) {
                  uint32 id = p->getUint32( pos );
                  pos += 4;
                  const char* pin = p->getString( pos );
                  pos += strlen( pin ) + 1;
                  const char* comment = p->getString( pos );
                  pos += strlen( comment ) + 1;
                  m_pinData->addPIN( pin, comment, id );
               }
            }
            if ( r.getParamBlock().getParam( 5304 ) != NULL ) {
               m_pinData->setCRC( 
                  r.getParamBlock().getParam( 5304 )->getUint32() );
            }
            //TRACE_FUNC1( "New PINs done!" );
         }
         //TRACE_DBG( "crc %"PRId32" newcrc %"PRId32, crc, m_pinData->getCRC() );
         if ( crc != m_pinData->getCRC() || origSize != m_pinData->size() )
         {
            // Changed set param
            Buffer buf;
            m_pinData->packInto( &buf );
            //m_pinData->TRACE();
            m_paramProvPubl->setParam(
               ParameterEnums::TR_trackPIN,
               (uint8 *)buf.accessRawData( 0 ), buf.getLength() );
         }
      }
      
   }


   void 
   Tracking::solicitedError( const ErrorObj& err, uint32 src ) {
      // For now unset sent status and hope it will be sent again
      if ( src == m_pinSyncID ) {
         // If initiated from GUI we should send an error...
         m_pinSyncID = 0;
      }
      if ( src == m_trackingID ) {
         m_trackingID = 0;
      }
   }


   void
   Tracking::addPINData( NParamBlock& params )
   {
      NParam& psync = params.addParam( 5201 );
      if ( m_pinData != NULL ) {
         params.addParam( NParam( 5204, m_pinData->getCRC() ) );
         NParam& padd = params.addParam( 5203 );
         NParam& pdel = params.addParam( 5202 );
         for ( TrackPINList::iterator it = m_pinData->begin() ; 
               it != m_pinData->end() ; ++it ) {
            if ( (*it)->getID() == 0 ) {
               // Added   5203
               padd.addUint32( (*it)->getID() );
               padd.addString( (*it)->getPIN() );
               padd.addString( (*it)->getComment() );
            } else if ( GET_UINT32_MSB( (*it)->getID() ) ) {
               // Deleted 5202
               pdel.addUint32( REMOVE_UINT32_MSB( (*it)->getID() ) );
               psync.addUint32( REMOVE_UINT32_MSB( (*it)->getID() ) );
            } else {
               // Sync    5201
               psync.addUint32( (*it)->getID() );
            }
         }// End for each in m_pinData
      } // End if m_pinData not NULL
      else {
         params.addParam( NParam( 5204, uint32(0) ) );
      }
   }


   uint32
   Tracking::getPINDataCRC() const {
      return m_pinData ? m_pinData->getCRC() : 0;
   }


   void
   Tracking::newPINCRC( uint32 crc ) {
      //TRACE_DBG( "newPINCRC %"PRIu32, crc );
      if ( m_pinSyncID == 0 && !m_hasFleet ) {
         if ( changedPINs( crc ) ) {
            // Changed
            NParamBlock params;
            addPINData( params );
            //TRACE_DBG( "Send PIN request" );
            m_pinSyncID = m_nscProvPubl->sendRequest( 
               navRequestType::NAV_TRACK_REQ, params );
         }
      } // Else already syncing wait for it or fleet 
      //        mode without pin in server
   }

   #define EARTH_RADIUS 6378137.0
   #define NAV2_COORD_RAD_FACTOR 100000000
   #define SCALE_TO_CENTIMETER (EARTH_RADIUS / NAV2_COORD_RAD_FACTOR * 100)

   uint32
   Tracking::distancePoints( int32 lat1, int32 lon1, 
                             int32 lat2, int32 lon2 ) const
   {
      float64 cosLat = cos( float64((lat1 + lat2)/2) / 
                            NAV2_COORD_RAD_FACTOR );
      float64 dlatsq = square( (float64)  (lat2 - lat1) );
      float64 dlonsq = square( ((float64) (lon2 - lon1)) * cosLat);
      return uint32( sqrt(dlatsq + dlonsq) * SCALE_TO_CENTIMETER );
   }


   void 
   Tracking::setServerTime( uint32 t ) {
      uint32 lt = TimeUtils::time( NULL );
//      TRACE_DBG( "Server UTC %"PRIu32" Local %"PRIu32, t, lt );
      m_timeDiffSet = true;
      m_timeDiff = t - lt;
   }

   //------------ Private methods ------------------------------

   void
   Tracking::defineTimer() {
      if ( m_trackingTime > 0 && (m_hastracking || m_hasFleet) ) {
//         TRACE_FUNC1( "Tracking timer set" );
         //      Check time of first stored track point compared to now
         //      and subtract that (but at least 1s timeout)
         uint32 trackingTime = m_trackingTime;
         if ( m_points.size() > 0 ) {
            uint32 now = TimeUtils::time( NULL );
            uint32 diffTime = now - m_points[ 0 ].time();
            if ( m_points[ 0 ].time() > now ) {
               diffTime = 0;
            }
            if ( trackingTime > diffTime ) {
               trackingTime -= diffTime;
            } else {
               trackingTime = 1;
            }
         }
         //TRACE_DBG( "trackingTime %"PRIu32" = %"PRId32, trackingTime, (trackingTime*1000) );
         // Max in microseconds (2147)
         trackingTime = min(trackingTime, uint32(2000));
         //TRACE_DBG( "setting trackingTime %"PRIu32" = %"PRId32, trackingTime, (trackingTime*1000) );
         if ( m_trackingTimer == 0 ) {
            m_trackingTimer = m_queue->defineTimer( trackingTime * 1000 );
         } else {
            m_queue->setTimer( m_trackingTimer, trackingTime * 1000 );
         }
      }
   }


   void
   Tracking::setTrackDataParam() {
      Buffer buff( 20 + m_points.size()*20 );
      buff.writeNext8bit( TRACK_DATA_VER_2 );
      for ( uint32 i = 0 ; i < m_points.size() ; ++i ) {
         buff.writeNextUnaligned32bit( m_points[ i ].lat() );
         buff.writeNextUnaligned32bit( m_points[ i ].lon() );
         buff.writeNextUnaligned32bit( m_points[ i ].dist() );
         buff.writeNextUnaligned16bit( m_points[ i ].heading() );
         buff.writeNextUnaligned16bit( m_points[ i ].speed() );
         buff.writeNextUnaligned32bit( m_points[ i ].time() );
         buff.writeNextUnaligned32bit( m_points[ i ].gpsTime() );
//         TRACE_FUNC1( "Storing coord" );
      }
      m_paramProvPubl->setParam( ParameterEnums::TR_trackdata,
            buff.accessRawData( 0 ), buff.getLength()  );
   }



   void
   Tracking::sync( const NavReplyPacket* r ) {
      if ( r->getParamBlock().getParam( 5304 ) != NULL ) {
         // Have data
         // Remove all localy added
         m_pinData->removePIN( uint32(0) );

         const NParamBlock& params = r->getParamBlock();
         // 5302 PINIDDel  uint32 array 
         if ( params.getParam( 5302 ) != NULL ) {
            const NParam* p = params.getParam( 5302 );
            uint32 pos = 0;
            while ( pos < p->getLength() ) {
               uint32 id = p->getUint32( pos );
               pos += 4;
//               TRACE_DBG( "Remove %"PRId32" size %d", id,
//                          m_pinData->size() );
               m_pinData->removePIN( id );
//               TRACE_DBG( "Removed size %d", m_pinData->size() );
            }
         }
         
         // 5303 PINAdd    byte array 
         if ( params.getParam( 5303 ) != NULL ) {
            const NParam* p = params.getParam( 5303 );
            uint32 pos = 0;
            while ( pos < p->getLength() ) {
               uint32 id = p->getUint32( pos );
               pos += 4;
               const char* pin = p->getString( pos );
               pos += strlen( pin ) + 1;
               const char* comment = p->getString( pos );
               pos += strlen( comment ) + 1;
               m_pinData->addPIN( pin, comment, id );
//               TRACE_DBG( "Add %"PRId32" PIN %s size %d", 
//                          id, pin,
//                          m_pinData->size() );
               
            }
         }
         
         // 5304 PINCRC     uint32 
         // Set new crc
         m_pinData->setCRC( params.getParam( 5304 )->getUint32() );
      }
   }


   bool
   Tracking::changedPINs( uint32 crc ) const {
      uint32 newCrc = getPINDataCRC();
      uint32 size = m_pinData ? m_pinData->size() : 0;
      if ( crc != newCrc || (newCrc == 0 && size > 0) ) {
         return true;
      } else {
         return false;
      }
   }


   void
   Tracking::storePos( const PositionState &p, uint32 now ) {
      m_lastStoredCoord.lat()      = p.lat;
      m_lastStoredCoord.lon()      = p.lon;
      m_lastStoredCoord.heading()  = p.heading;
      m_lastStoredCoord.speed()    = p.speed;
      m_lastStoredCoord.time()     = now;
      m_lastStoredCoord.gpsTime()  = p.time.millis();
      m_lastStoredCoord.dist()     = m_lastCoord.dist();

      m_points.push_back( m_lastStoredCoord );
      
      m_lastCoord.dist() = MAX_UINT32;
      m_distcounting = true;
      
      // Save as TR_trackdata
      setTrackDataParam();
   }

} // End isab namespace
