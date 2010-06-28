/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRACKING_H
#define TRACKING_H


#include "arch.h"
#include "Nav2Error.h"
#include "PositionState.h"


namespace isab{

   class ModuleQueue;
   class NavServerComProviderPublic;
   class ParameterProviderPublic;
   class Buffer;
   class NParamBlock;
   class TrackPINList;
   class NavReplyPacket;
   class ErrorObj;

   /**
    * Class for holding a Tracking point.
    */
   class TrackingPoint {
      public:
         TrackingPoint( int32 lat, int32 lon, uint32 dist, uint16 heading,
                        uint16 speed, uint32 time, uint32 gpsTime )
            : m_lat( lat ), m_lon( lon ), m_dist( dist ), 
              m_heading( heading ), m_speed( speed ), m_time( time ),
              m_src( 0 ), m_gpsTime( gpsTime )
            {}


         TrackingPoint()
            : m_lat( MAX_INT32 ), m_lon( MAX_INT32 ), m_dist( MAX_UINT32 ),
              m_heading( MAX_UINT16 ), m_speed( MAX_UINT16 ), m_time( 0 ),
              m_src( 0 ), m_gpsTime( 0 )
            {}

         operator PositionState() const;


         int32&  lat();
         int32&  lon();
         uint32& dist();
         uint16& heading();
         uint16& speed();
         uint32& time();
         uint32& src();
         uint32& gpsTime();

         const int32&  lat() const;
         const int32&  lon() const;
         const uint32& dist() const;
         const uint16& heading() const;
         const uint16& speed() const;
         const uint32& time() const;
         const uint32& src() const;
         const uint32& gpsTime() const;

      private:
         int32  m_lat;
         int32  m_lon;
         uint32 m_dist;
         uint16 m_heading;
         uint16 m_speed;
         uint32 m_time;
         uint32 m_src;
         uint32 m_gpsTime;
   };


   inline int32&  TrackingPoint::lat()      { return m_lat; }
   inline int32&  TrackingPoint::lon()      { return m_lon; }
   inline uint32& TrackingPoint::dist()     { return m_dist; }
   inline uint16& TrackingPoint::heading()  { return m_heading; }
   inline uint16& TrackingPoint::speed()    { return m_speed; }
   inline uint32& TrackingPoint::time()     { return m_time; }
   inline uint32& TrackingPoint::src()      { return m_src; }
   inline uint32& TrackingPoint::gpsTime()  { return m_gpsTime; }

   inline const int32&  TrackingPoint::lat()     const { return m_lat; }
   inline const int32&  TrackingPoint::lon()     const { return m_lon; }
   inline const uint32& TrackingPoint::dist()    const { return m_dist; }
   inline const uint16& TrackingPoint::heading() const { return m_heading;}
   inline const uint16& TrackingPoint::speed()   const { return m_speed; }
   inline const uint32& TrackingPoint::time()    const { return m_time; }
   inline const uint32& TrackingPoint::src()     const { return m_src; }
   inline const uint32& TrackingPoint::gpsTime() const { return m_gpsTime;}



   /**
    * This a a Module wannabe, currenly it hooks into an existing
    * Module. This is because a new Module would cost much memory.
    *
    * This class needs some more documentation.
    */
   class Tracking {
      public:
         Tracking( ModuleQueue* queue,
                   NavServerComProviderPublic* navServComProvPubl,
                   ParameterProviderPublic* paramProvPubl );

         ~Tracking();

         void decodedStartupComplete();

         void decodedShutdownPrepare( int16 upperTimeout );

         bool isdecodedTimerOk( uint16 timerID );

         void decodedExpiredTimer( uint16 timerID );
         
         void decodedPositionState( 
            const struct PositionState &p );

         bool isdecodedParamNoValueOk( uint32 param );

         void decodedParamNoValue( uint32 param );

         bool isdecodedParamValueOk( uint32 param );

         void decodedParamValue( uint32 param, const int32* data, 
                                 int32 numEntries );

         void decodedParamValue( uint32 param, const uint8* data, 
                                 int size );

         void reply( const uint8* data, int size, uint32 src ); 

         void solicitedError( const ErrorObj& err, uint32 src );


         /**
          * Adds current PIN data to param block.
          *
          * @param pinBuff The buffer with the PIN data.
          * @param params  The param block to add data to.
          */
         void addPINData( NParamBlock& params );


         /**
          * Get the current crc for PIN data.
          */
         uint32 getPINDataCRC() const;


         /**
          * Check if this is a new crc and send PIN sync if not already 
          * sent.
          */
         void newPINCRC( uint32 crc );


         /**
          * Calculates the distance between 1 and 2 in centimeters.
          */
         uint32 distancePoints( int32 lat1, int32 lon1, 
                                int32 lat2, int32 lon2 ) const;


         /**
          * Set the time from server, the UTC time.
          */
         void setServerTime( uint32 t );


      private:
         void defineTimer();

         void setTrackDataParam();


         /**
          * Sync with server list.
          */
         void sync( const NavReplyPacket* r );


         /**
          * If the PINs are changed, compared with the crc.
          */
         bool changedPINs( uint32 crc ) const;


         void storePos( const PositionState &p, uint32 now );


         ModuleQueue* m_queue;
         NavServerComProviderPublic* m_nscProvPubl;
         ParameterProviderPublic* m_paramProvPubl;

         /// Time interval between stored tracks.
         uint32 m_positionTime;

         /// Time interval between sending tracks to server.
         uint32 m_trackingTime;

         /// The tracking timer.
         uint16 m_trackingTimer;

         /// Last coord from GPS.
         TrackingPoint m_lastCoord;

         /// Last stored coord.
         TrackingPoint m_lastStoredCoord;

         typedef std::vector< TrackingPoint > TrackingPoints;
         /// The stored coords.
         TrackingPoints m_points;

         /// If startup done(loaded Trackpoints from param)
         bool m_startupCompletete;

         /// If loaded PINs param.
         bool m_initializedPINs;

         /// The PINs
         TrackPINList* m_pinData;

         /// The id of outstanding PIN sync.
         uint32 m_pinSyncID;

         /// If counting distance for current track
         bool m_distcounting;

         /// The ID of outstanding tracking req.
         uint32 m_trackingID;


         /// If has tracking right.
         bool m_hastracking;


         /// The time of last sent tracking request.
         uint32 m_lastSentTime;


         /// If fleet right.
         bool m_hasFleet;

         /// If has time diff.
         bool m_timeDiffSet;

         /// The time diff
         int32 m_timeDiff;
   };


} // End isab namespace

#endif // TRACKING_H
