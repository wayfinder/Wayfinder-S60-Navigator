/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The meat of the navigation system. This is the algorithm that 
 * actually follows the position of the car.
 */

//#include "GuiFileOperation.h"
#ifndef MODULE_NavTaskInternal_H
#define MODULE_NavTaskInternal_H
#include "HintNode.h"

namespace isab {

   namespace RouteInfoParts {
      struct Crossing;
   }

   /**
    * This class is a Nav2 Module that is connected directly to the
    * CtrlHub Module. It handles the downloading of routes by
    * referring them to the NavServerCom Module. It accepts
    * positioning packets from an underlying GpsProvider and matches
    * them to a loaded route and then sends route-matched packages to
    * consumers.
    */
   class NavTask : public Module,
                   public GpsConsumerInterface, 
                   public CtrlHubAttachedInterface, 
                   public NavTaskProviderInterface,
                   public ParameterConsumerInterface,
                   public NavServerComConsumerInterface
   {
      friend class GuiCommandHandler;

   public:
      /** @name Constructors and destructor. */
      //@{
      /**
       * Constructor. 
       * @param p Pointer to a GpsProviderPoublic object. 
       */
      NavTask(class GpsProviderPublic *p);
      /** Virtual destructor. */
      virtual ~NavTask();
      //@}

      /** 
       * Creates a new CtrlHubAttachedPublic object used to connect
       * this module to the CtrlHub.
       * @return A new CtrlHubAttachedPublic object connected to the
       *         queue.
       */
      class CtrlHubAttachedPublic * newPublicCtrlHub();

   protected:
#ifndef _MSC_VER
      /** Distance at which to show goal reached */
      static const int GOAL_DIST = 25;
      /** ?????? FIXME - what is this? Fix comment.  */
      static const int32 MAX_DIST = 60;
      /** How many off track before it shows up on display */
      static const int OFF_TRACK_HOLDOFF_VAL = 3;
      /** How many wrong dir before it shows up on display */
      static const int WRONG_DIR_HOLDOFF_VAL = 4;
#else
      enum { 
         /** Distance at which to show goal reached */
         GOAL_DIST = 25,
         /** ?????? FIXME - what is this? Fix comment.  */
         MAX_DIST = 60, 
         /** How many off track before it shows up on display */
         OFF_TRACK_HOLDOFF_VAL = 6,
         /** How many wrong dir before it shows up on display */
         WRONG_DIR_HOLDOFF_VAL = 4
      };
#endif

      /** 
       * Tuning variables used by the off track detection algorithm
       * Perhaps we could brand these depending on GPS device (position quality)
       * and location (map quality)?
       */

      // -------------------------------------------------------------

      enum misc_offtrack_constants {
         /**
          * Maximum off track penalty. We are off track if
          * iOffTrackPenalty >= MAX_OFF_TRACK_PENALTY
          */

         MAX_OFF_TRACK_PENALTY = 12000,

         /**
          * Maximum penalty per position. This is used to limit
          * the influence of a single extreme position.
          */

         MAX_PENALTY_PER_POSITION =
            MAX_OFF_TRACK_PENALTY / 2,

         /**
          * Minimum speed in knots (1 knot ~= 1.8 km/h) 
          * required for off track detection
          */

         MIN_SPEED_FOR_OFFTRACK = 5,

         /**
          * Minimum penalty for a position required to 
          * increase the total penalty iOffTrackPenalty,
          * otherwise it is reset
          */

         MIN_OFF_TRACK_PENALTY = 150,

         /**
          * Distance to route (in meters) limit for starting 
          * adding extra penalty, i.e. if we are further away
          * than this from the route, extra penalty is added
          */

         MIN_DIST_FOR_EXTRA_PENALTY = 20,

         /**
          * Minimum change in distance to route between consecutive
          * positions for off track detection
          */

         MIN_PER_DIFF = 2,

         /**
          * Maximum change in distance to route between consecutive
          * positions for off track detection
          */

         MAX_PER_DIFF = 35,

         /**
          * Limit on the angle error for determining if we 
          * are driving in the wrong direction
          * 
          * Java implementation uses 360 degrees, Nav2 256 degrees.
          * Java implementation says 120 degrees => 120/360*256 nav2 degrees.
          */
         MIN_ANGLE_ERROR_FOR_OFF_TRACK = 85,

         /**
          * Upper distance limit in meters for on track
          */

         MAX_DISTANCE_FOR_ON_TRACK = 200,

         // ------------------------------------------------------------

         /**
          * Maximum distance to route to start snapping
          */

         MAX_DISTANCE_FOR_SNAP = 15,

         /**
          * Maximum angle error to start snapping 
          * Java implementation uses 360 degrees, Nav2 256 degrees.
          * Java implementation says 30 degrees => 30/360*256 nav2 degrees.
          */

         MAX_ANGLE_ERROR_FOR_SNAP = 21,

      };

      /** Enumerates the route following state. */
      enum OnTrackEnum { 
         /** The position matches the route on track.*/
         OnTrack = 0, 
         /** 
          * The posision matches the route but travels in the wrong
          * direction. 
          */
         OnTrackWrongWay, 
         /** The position is to far away from the route. */
         OffTrack 
      };
      
      /** The object that sends position information. */
      class GpsProviderPublic * m_gps;

      /** @name From Module. */
      //@{
      /**
       * Decode a received packet by sending to different decoders
       * until someone wants it.
       * @param buf The received package.
       * @return NULL if the package was accepted, otherwise buf. 
       */
      virtual class MsgBuffer * dispatch(class MsgBuffer *buf);

      // Generic order to (dis)connect to the peer module
      virtual void decodedStartupComplete();

      virtual void decodedConnectionCtrl(enum ConnectionCtrl, 
                                         const char *method, uint32 src);

      virtual void decodedSolicitedError( const ErrorObj& err, 
                                          uint32 src, uint32 dst );

      /** 
       * Specialized ExpiredTimer method to decode the timer for the
       * simulation.
       * When we get a timeout from the timer we reset the timer and
       * call the function to generate the next simulated GPS
       * position.
       * 
       * @param timerid The ID of the timer that timedout.
       */
      virtual void decodedExpiredTimer(uint16 timerid);

      //@}

      /**
       * @name From GpsConsumerInterface. 
       * Some of the functions have helpers that do the actual
       * work. This is used while simulating.
       */
      //@{
      /** 
       * Stub method for the real GPS Sat Info handler
       * decodedGpsSatelliteInfoFunc.
       * Is used to be able to discard GPS Sat info when we are
       * simulating.
       * 
       * @param satInfo   The current satellite information.
       * @param src       The source of the message.
       */
      virtual void decodedGpsSatelliteInfo(
                           GpsSatInfoHolder* satInfo,
                           uint32 src);
      void decodedGpsSatelliteInfoFunc(GpsSatInfoHolder* satInfo,
                                       uint32 src);

      /** 
       * Stub method for the real GPS Position handler decodedGpsPositionVelocityFunc.
       * Is used to be able to discard GPS positions when we are simulating.
       * 
       * @param v   The current GPS velocity vector (our GPS position).
       * @param src The source of the message.
       */
      virtual void decodedGpsPositionVelocity(
            const struct GpsConsumerPublic::GpsPositionVelocityVector &v,
            uint32 src);
      void decodedGpsPositionVelocityFunc(
            const struct GpsConsumerPublic::GpsPositionVelocityVector &v,
            uint32 src);

      /** 
       * Stub method for the real GPS Status handler decodedGpsStatusFunc.
       * Is used to be able to discard GPS sat info when we are simulating.
       * 
       * @param t                Currently a DUMMY.
       * @param sum_status       The current GPS Status.
       * @param error_conditions Possible errors.
       * @param pdop             Position Dilution of Precision.
       * @param src              The source of the message.
       */
      virtual void decodedGpsStatus(isabTime t, 
                           enum GpsStatus sum_status,
                           uint16 error_conditions,
                           float pdop,
                           uint32 src);
      void decodedGpsStatusFunc(isabTime t, 
                           enum GpsStatus sum_status,
                           uint16 error_conditions,
                           float pdop,
                           uint32 src);

      virtual void decodedGpsTime(isabTime t, 
                          float tow,
                          int weekno,
                          float utc_offset,
                          uint32 src);

      //@}

      /** @name From NavTaskProviderInterface. */
      //@{
      virtual void decodedCommand(enum NavTaskProviderPublic::CommandType command,
            uint32 src);
      virtual void decodedFileOperation(class GuiFileOperation* op,
            uint32 src);
      virtual void decodedNTRouteToGps(int32 toLat, int32 toLon,
            int32 fromLat, int32 fromLon,
            uint32 src);

      virtual void decodedNewRoute(bool routeOrigFromGPS,
                                   int32 origLat, int32 origLon,
                                   int32 destLat, int32 destLon, 
                                   int packetLength,
                                   uint32 src);

      virtual void decodedGetFilteredRouteList(int16 startWpt, int16 numWpts,
                                   uint32 src);

      virtual void decodedRouteChunk(bool failedRoute, int chunkLength, const uint8 *chunkData,
            uint32 src);
      virtual void decodedKeepRoute(int64 routeId, uint32 src, uint32 dst);

      //@}

      /** 
       * ParameterConsumer functions.
       */
      //@{
      virtual void decodedParamNoValue(uint32 paramId,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const int32 * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const float * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId,
                                     const char * const * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst);
      virtual void decodedParamValue(uint32 paramId, 
                                     const uint8* data, 
                                     int size,
                                     uint32 src,
                                     uint32 dst);
      //@}

      void switchToNewRoute();

      void reRoute(bool internal,
                   enum NavServerComEnums::RerouteReason reason);

      bool offTrackDetection(int dist_to_seg, int seg_angle, uint32 dist_to_goal);

      /** 
       * Performs the calculation of the snapped position.
       */
      void calculateSnappedPosition( int32 x1, int32 y1,
                                     int32 x2, int32 y2,
                                     int32 xP, int32 yP,
                                     int32 &snap_lat,
                                     int32 &snap_lon );

      void navigate_gps();


      void dumpHintNodeVector( uint32 timeMillis,
                               const std::vector<HintNode>& nodes );
      /**
       *   Calculates the speed using current and previous position.
       */
      
      float getCalculatedSpeed();

      /**
       *   Sets up a vector of HintNodes, where the first node contains
       *   the current GPS position and velocity. All other nodes are points
       *   on the route, containing no velocity weights. The returned data
       *   should be processed by a HintNode algorithm - to fill it with
       *   interpolation data.
       */
      
      std::vector<HintNode> prepareInterpolationVector(int32 lat, int32 lon);
      
      uint8 calc_heading_to_target(float lat, float lon);

      void FIXME_fixup_roundabout_crossings(RouteInfoParts::Crossing & c);

      /** 
       * Setup the simulation of GPS positions.
       * Called from the UI when the user selects simulation.
       */
      void setupGpsSimulation();

      /**
       * Stops the GPS simulation and returns the current position
       * to the first waypoint in the route.
       */
      void stopGpsSimulation();

      /**
       * Resets the GPS simulation, returns the current position
       * to the first waypoint in the route. But lets the timer
       * keep running.
       */
      void resetGpsSimulation();

      /** 
       * Generates the next simulated GPS position to be displayed while
       * simulating a route.
       */
      void generateNextSimulatedGpsPosition();

      /**
       * Called from gui when user wants to stop the simulation.
       */
      void setSimStop();

      /**
       * Called from gui when user wants to pause the simulation.
       */
      void setSimPaused();

      /**
       * Called from gui when user wants to resume a paused simulation.
       */
      void resumeSimPaused();

      /**
       * Called from gui when user wants to increase the 
       * simulation speed (doubles the current speed).
       */
      void incSimSpeed();

      /**
       * Called from gui when user wants to decrease the 
       * simulation speed (halves the speed).
       */
      void decSimSpeed();

      /**
       * Called from gui when user wants to repeat the 
       * simulation when reaching the goal.
       */
      void setSimRepeat();

      /**
       * Called from gui when user wants to stop repeating 
       * the simulation when reaching the goal.
       */
      void stopSimRepeat();
      
      /** Convenience function to get the index read from 
       * the parameter file. */
      int32 getRouteLastUsedIndex();

      /** Convenience function to get the file name read from 
       * the parameter file. */
      char* getRouteFileName();

      /**
       * Uses the different PTUI intervals to decide which to use and
       * the set the PTUI timer. Note that even if the interval hasn't
       * changed since the last time the timer will be reset to the
       * full interval.
       */
      void resetPtuiTimer();

      /**
       * This function does the actual work of setServerPtui and
       * setUserPtui. 
       *
       * Check if the lower 24 bits of user_ptui are less than the
       * lower 24 bits of server_ptui. If so, replace the lower 24
       * bits of user_ptui with the lower 24 bits of server_ptui,
       * keeping the top 8 bits of user_ptui. 
       *
       * If the result of this calculation is different than the
       * current value of m_user_ptui, the m_user_ptui variable is set
       * to the calculated value and the Ptui timer is
       * reset. Otherwise nothing happens.
       * @param user_ptui User PTUI candidate.
       * @param server_ptui Server PTUI value. 
       * @return The calculated user_ptui value.
       */
      int32 setUserPtuiParameter(int32 user_ptui, int32 server_ptui);

      /**
       * Handle changes to the server PTUI. If ptui is larger than
       * m_user_ptui the m_user_ptui value will be set to a valid
       * value and the ptui timer reset.
       * @param ptui New value of server ptui.
       */
      void setServerPtui(int32 ptui);

      /**
       * Handle changes to the user PTUI.  If ptui is less than
       * m_server_ptui, the NT_UserTrafficUpdatePeriod parameter value
       * will be set to the current value of m_server_ptui. If ptui is
       * valid (larger then or equal to m_server_ptui) the value will
       * be stored in m_user_ptui and the PTUI timer will be reset.
       * @param ptui The new value of the user PTUI. 
       */
      void setUserPtui(int32 ptui);

      /** 
       * Called when timer has expired and we are waiting for a gps
       * signal for calculating a route based on a "route to" request.
       */
      void HandleRouteToGpsRequest();

public:
      // from NavServerComConsumerInterface
      virtual void decodedBinaryUploadReply(uint32 src, uint32 dst) {};

      virtual void decodedBinaryDownload(const uint8* data, size_t length, 
                                         uint32 src, uint32 dst) {};

      virtual void decodedWhereAmIReply(const char* country, 
                                        const char* municipal, 
                                        const char* city, 
                                        const char* district, 
                                        const char* streetname, int32 lat,
                                        int32 lon, uint32 src, 
                                        uint32 dst) {};

      virtual void decodedSearchReply(unsigned nAreas, 
                                      const class SearchArea* const *sa,
                                      unsigned nItems, 
                                      const class FullSearchItem* const *fsi,
                                      uint16 begindex, uint16 total,
                                      uint32 src, uint32 dst) {};

      virtual void decodedSearchInfoReply(uint16 num, const char* const* id,
                                          const unsigned* index,
                                          unsigned numInfo,
                                          const class AdditionalInfo*const* info,
                                          uint32 src, uint32 dst) {};

      virtual void decodedRouteReply(uint32 src, uint32 dst) {};

      virtual void decodedMapReply(const class BoxBox& bb, uint32 realWorldWidth,
                                   uint32 realWorldHeight, uint16 imageWidth,
                                   uint16 imageHeight,
                                   enum MapEnums::ImageFormat imageType, 
                                   uint32 imageBufferSize, 
                                   const uint8* imageBuffer, uint32 dst) {};

      virtual void decodedVectorMapReply(const char* request, uint32 size,
                                         const uint8* data, uint32 dst) {};

      virtual void decodedMulitVectorMapReply(uint32 size,
                                              const uint8* data, 
                                              uint32 dst) {};

      virtual void decodedForceFeedMuliVectorMapReply( uint32 size,
                                                       const uint8* data, 
                                                       uint32 dst ) {};
                                                       
      virtual void decodedCellConfirm(uint32 size, const uint8* data,
                                      uint32 dst) {};

      /** Receives progress messages from NavServerCom. Since these
       * messages are multicasted, any module that expects to receive
       * progress messages must join the multicast channel
       * Module::NSCProgressMessages.
       * @param status indicates progress in quantified steps. 
       * @param done   parts done.
       * @param of     total number of parts to do. 
       * @param src    the sender of this message, probably NavServerCom.
       * @param dst    the address of the receiver, probaly the 
       *               multicast address.
       */
      virtual void decodedProgressMessage(isab::NavServerComEnums::ComStatus status, 
                                          GuiProtEnums::ServerActionType type,
                                          uint32 done, uint32 of,
                                          uint32 src, uint32 dst);
                                          
      virtual void decodedMessageReply(const uint8* data, unsigned length,
                                       uint32 src, uint32 dst) {};
      virtual void decodedLicenseReply(bool keyOk, bool phoneOk, 
                                       bool regionOk, bool nameOk, 
                                       bool emailOk,
                                       GuiProtEnums::WayfinderType type,
                                       uint32 src, uint32 dst) {};
      virtual void decodedLatestNews(uint32 checksum, const uint8* data, 
                                     unsigned length, uint32 src, 
                                     uint32 dst) {};
      virtual void decodedParamSyncReply(GuiProtEnums::WayfinderType wft, 
                                         uint32 src, uint32 dst) {};
      virtual void decodedReply( 
         navRequestType::RequestType type, class NParamBlock& params, 
         uint8 requestVer, uint8 statusCode, const char* statusMessage,
         uint32 src, uint32 dst ) {};

protected:

      struct RouteData {
         int32 toLat;
         int32 toLon;
         int32 fromLat;
         int32 fromLon;
         uint32 routeRequestorId;
      };

      std::vector<HintNode> m_interpolationHints;

      int32 m_prevLat;
      int32 m_prevLon;
      uint32 m_prevTimeMillis;
      
      /** Decoder for GpsConsumer-messages */
      class GpsConsumerDecoder m_gpsDecoder;
      /** Decoder for CtrlHub-messages */
      class CtrlHubAttachedDecoder m_ctrlHubAttachedDecoder;
      /** Decoder for NavTask-messages */
      class NavTaskProviderDecoder m_navTaskProviderDecoder;
      /** Decoder for Parameter-messages */
      class ParameterConsumerDecoder m_parameterConsumerDecoder;

      /** Encoder for NavTask-messages */
      class NavTaskConsumerPublic * m_navTaskConsumer;
      /** Encoder for NavServerCom-messages */
      class NavServerComProviderPublic * m_navServerCom;
      /** Encoder for Parameter-messages */
      class ParameterProviderPublic * m_parameter;

      /** The route we are following. This is only a temporary
       * solution used during initial development and debugging. */
      class Route *m_route;

      /** The route we are downloading. This is only a temporary
       * solution used during initial development and debugging. */
      class Route *m_receiveRoute;

      /** The handler that performs the actions from 
       * decodedFileOperation. */
      class GuiCommandHandler *m_guiCommandHandler;

      /** Time Logging utility for meassuring delays in  
       * Nav2 while debugging. */
      class TimeLoggingUtil* m_timeLogger;

      /** Are we following a route right now? 
       * This is only set to true by decodedRouteChunk. */
      bool m_followingRoute;

      /** Is the route in m_route a valid route? 
       * Set on switchToNewRoute() and never cleared for now */
      bool m_routeValid;

      /** Enum to know if we're currently in simulation mode or paused.
       * This gets set when the user selects simulation from the GUI. */
      uint16 m_simInfoStatus;

      /** The current simulated gps position is needed so we can 
       * keep track of where we are now and what should be our
       * next position. */
      struct GpsConsumerPublic::GpsPositionVelocityVector m_currentSimGpsPosition;

      /** Variables to keep the state between invocations of the 
       * function generateNextSimulatedGpsPos(); */
      class Point m_currentSimGpsPoint;
      class Point m_nextSimGpsPoint;
      int32 m_distFromPrevWpt;

      /** Variable to keep the state between invocations of the 
       * functions increase and decrease sim speed. */
      int32 m_simGpsStatus;
      
      /** The current speed that the simulated gps is traveling
       * with, in m/s. Maximum is the speed limit on the road. */
      uint8 m_simulatedSpeed;
      
      /** Variable used as a constant when calculating our next 
       * simulated position. Used to variate the speed of the simulation. */
      uint8 m_simSpeed;

      /** The timer to know when we should get new simulation data.
       * The decoder function for the timer calls a function
       * to get the next simulated position. */
      uint16 m_simTimeoutId;

      /** The timer to know when to check for gps signa so that
       * users route requeste can be granted. */
      uint16 m_waitForSignalId;

      /** Variables used to store index and filename read from 
       * the parameter file. */
      int32 m_routeLastUsedIndex;
      char* m_routeLastFileName;

      /* *************************************************
       * The state variables
       *
       * Maybe they should be combined with m_lastPosState? Or 
       * m_lastPosState could be constructed from them? FIXME.
       * Think this one through more.
       */
      float last_gps_lat;
      float last_gps_lon;
      float last_gps_alt;
      float last_gps_speed;
      uint8 last_gps_head;
      bool last_gps_head_valid;

      float m_gpsLatency;

      enum Quality m_lastGpsQuality;
      struct PositionState m_lastPosState;

      enum OnTrackEnum m_carOnTrack;
      class Point m_firstPoint;
      int m_lastCurrSegCrossingNo;
      class Point m_lastCurrWpt;
      class Point m_lastCurrSeg;
      bool m_lastCurrSegValid;
      bool m_lastShowStartWithUturn;
      const char * m_lastCurrSegText;
      int16 m_lastCurrSegSpeed;
      uint8 m_lastCurrSegFlags;


      /* Legacy code that has not been de-haired yet */
      int m_holdoff_wrong_dir;
      int m_holdoff_off_track;

      /** The request id that was sent to NavServerCom */
      uint32 m_nscRouteReqId;

      /** The request id that caused us to start downloading a new
         route. */
      uint32 m_routeRequestorId;

      /** File operation requester id. */
      uint32 m_fileOperationRequester;

      /** Internally generated route request. Replies are unsolicited. */
      bool m_internalRouteRequest;

      /** There is currently an outstanding route request to NSC */
      bool m_awaitingRouteReply;

      /** We have received an abandon route command, and need to */
      /** ignore any outstanding route commands to server. */
      bool m_stopNextInternalRoute;

      /** The last automatic reroute (end of truncated route or
       *  an offtrack with auto reroute set) failed and no 
       *  manual route has been succsesful since. */
      bool m_autoRerouteFailed;

      /** This parameter is set from the WayfinderType, and set
       * to false on startup and for silver customers.
       */
      bool m_useGPSNavigation;

      uint32 m_last_time;

      uint32 m_prev_dist_to_goal;
    	uint32 m_prev_dist_to_seg;

      /**
       * @name Variables used for the Periodic Traffic Information
       *       Update (PTUI). 
       */
      //@{
      /** User set interval in minutes. */
      int32 m_user_ptui;
      /** Server set minimal allowed interval in minutes. */
      int32 m_server_ptui;
      /** Timer id of the PTUI timer. */
      uint16 m_ptuiTimer;
      /** Nbr of times the timer should time out when waiting for gps
       * signal before sending error to gui */
      uint16 m_nbrRetries;
      RouteData m_routeData;
      //@}

      /// Current off track penalty.
      int m_offTrackPenalty;
      
      NavServerComConsumerDecoder  m_nscDecoder;     // XXX: not set!
      
      /** Indicates whether the connection is available for rerouting or not. */
      bool m_canReroute;
   };

} /* namespace isab */

#endif /* MODULE_NavTaskInternal_H */
