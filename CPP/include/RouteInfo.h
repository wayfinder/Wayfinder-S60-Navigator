/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef ROUTEINFO_H
#define ROUTEINFO_H
#include "arch.h"
#include "RouteEnums.h"
#include <vector>
namespace isab{
   using namespace RouteEnums;
   class Buffer;
   class Log;

   namespace RouteInfoParts {
      class Segment {
         public:
         const char *streetName;
         int16 speedLimit;
         uint8 flags;
         bool  valid;
         bool  changed;
         
         bool isHighway() const { return (flags & 0x01) != 0; }
         bool isLeftTraffic() const { return (flags & 0x02) != 0; }

         private:
         bool ownStreetName;

         public:
         Segment();
         Segment(const Segment &oldseg);
         Segment(Buffer *buf);
         ~Segment();
         Segment& operator= (const Segment &old);
         void writeToBuf(Buffer *buf) const;
         void readFromBuf(Buffer *buf);
         int log(class Log* log, const char* name) const;
      };

      struct Crossing {
         uint32 action;
         uint32 distToNextCrossing;
         uint8  crossingType;
         uint8  exitCount;

         /** coordinates for the crossing */
         int32 lat;
         int32 lon;

         bool   valid;
         bool  changed;

         void writeToBuf(Buffer *buf) const;
         void readFromBuf(Buffer *buf);

         int log(class Log* log, const char* name) const;
      };

      class Landmark {
         public:         
            enum LandmarkSide {
               Left = 0,
               Right = 1,
               Undefined = 2
            };

            enum LandmarkType {
               builtUpAreaLM=0,
               railwayLM,
               areaLM,
               poiLM,
               signPostLM,
               countryLM,
               countryAndBuiltUpAreaLM,
               passedStreetLM,
               accidentLM,
               roadWorkLM,
               cameraLM,
               speedTrapLM,
               policeLM,
               weatherLM,
               trafficOtherLM
            };

            enum StartStopQuery {
               ActiveNow = 0,
               ActiveOnWpt,
               Begins,
               Ends
            };
               
                                
            const char *info;
            enum LandmarkSide side;
            enum LandmarkType type;
            bool disturbedRoute;
            bool start;
            bool stop;
            int32 startDistance;    // -1 if start==false
            int32 stopDistance;     // -1 if stop==false

         private:
            bool ownInfo;

         public:
            Landmark(const char *ainfo, bool astart, bool astop, bool adetour, 
                     enum LandmarkSide aside, enum LandmarkType atype, 
                     int32 astartDistance, int32 astopDistance);
            Landmark(Buffer *);
            Landmark(const Landmark& other);
            Landmark& operator= ( const Landmark& other );
            ~Landmark();
            bool matchesStartStop(enum Landmark::StartStopQuery q, int32 currDist) const;
            bool isDetour(enum Landmark::StartStopQuery q, int32 currDist) const;
            bool isSpeedCamera(enum Landmark::StartStopQuery q, int32 currDist) const;
            const char * getDescription() const;
            void writeToBuf(Buffer *buf) const;
            void readFromBuf(Buffer *buf);
            int log(class Log* log, int order) const;

      };

      class Lane{
         public:

            enum Direction{
               NO_DIRECTION = 0,
               LEFT90       = 2,
               LEFT45       = 3,
               STRAIGTH     = 4,
               RIGHT45      = 5,
               RIGHT90      = 6
            };
            
            Lane( uint8 lane );
            
            Lane( Buffer *buf );

            Lane(const Lane& other);

            bool isPreferred();

            bool isAllowed();

            Direction direction();

            void writeToBuf(Buffer *buf) const;

            void readFromBuf(Buffer *buf);

            int log(class Log* log, int order) const;

            uint8 m_lane;
      };

      class Lanes{
         public:
 
            bool noLanes;

            int32 distance;

            typedef std::vector<Lane*> rip_lane_vector;
            rip_lane_vector m_laneVector;

      };

      struct RGBColor {
         uint8 red;
         uint8 green;
         uint8 blue;
      };

      class SignPost{
         public:
            SignPost();

            SignPost(const char *atext, int32 adist, 
                     uint8 atextRed, uint8 atextGreen, uint8 atextBlue, 
                     uint8 abgRed, uint8 abgGreen, uint8 abgBlue, 
                     uint8 afgRed, uint8 afgGreen, uint8 afgBlue);

            SignPost(Buffer*);

            ~SignPost();
            
            SignPost& operator=( const SignPost& other );

            const char* getText() const;

            void writeToBuf(Buffer *buf) const;

            void readFromBuf(Buffer *buf);

            int log(class Log* log, int order) const;

            const char* m_text;
            RGBColor m_textColor;
            RGBColor m_backColor;
            RGBColor m_frontColor;
            int32 m_distance;

         private:
            bool ownInfo;

      };

      class RouteListCrossing {
         public:
         int16 wptNo;

         /** Heading to the target */
         uint8 toTarget;

         /** Distance from this crossing to goal. */
         int32 distToGoal;
         /** Travel time from this crossing to goal. */
         int32 timeToGoal;

         /** The crossing information. */
         struct Crossing crossing;

         /** Segments between this crossing and the next.*/
         typedef std::vector<Segment*> segment_container ;
         typedef segment_container::const_reverse_iterator segment_cr_iterator;
         segment_container segments;
         /** Landmarks between this crossing and the next. */
         typedef std::vector<Landmark*> landmark_container;
         landmark_container rlc_landmarks;

         RouteInfoParts::Lanes rlc_lanes;

         SignPost rlc_signpost;

         /**
          * Destructor. Deletes all objects pointed to by the segments
          * and landmarks containers.
          */
         ~RouteListCrossing();
         /**
          * Store a serialized representation of this
          * RouteListCrossing object in a Buffer.
          * @param buf The buffer to write into.
          */
         void writeToBuf(Buffer *buf) const;
         /**
          * Read a serialized representation of a RouteListCrossing
          * object from a Buffer.  Change this objects state to be a
          * copy of the serialized object.
          * @param buf The buffer to read from.
          */
         void readFromBuf(Buffer *buf);

         // Ownership of the returned string is not passed and remains
         // with this object.
         const char * hasDetourLandmark(enum Landmark::StartStopQuery q = Landmark::ActiveOnWpt) const;
         const char * hasSpeedCameraLandmark(enum Landmark::StartStopQuery q = Landmark::ActiveOnWpt) const;
      };
   }

   class RouteInfo {
      public:

      /// Default constructor
      RouteInfo() {}

      /// Copy constructor
      RouteInfo( const RouteInfo& other );

      /// Assignment operator
      RouteInfo& operator=(const RouteInfo& other);
      
      /// Deletes the landmarks
      ~RouteInfo();

      /* On track, off track, goal, wrong direction.  */
      enum OnTrackEnum onTrackStatus; 

      uint16 simInfoStatus; 

      /* Estimated time to the goal */
      int32 timeToGoal;

      /* Distance to current waypoint (the one we're navigating to) */
      int32 distToWpt; 

      /* Distance from the current position to the goal. */
      int32 distToGoal;

      /* Distance to the track (similar to crosstrack error) */
      int32 distToTrack;
   
      /** coordinates for the crossing */
      int32 lat;
      int32 lon;

      /** How to turn when at the track to get on track.
       * Only in off track.  */
      uint8 toOnTrackTurn;

      /** How to drive to get to the track.
       *  Only in off track.  */
      uint8 toTrackTurn;

      /** Estimated time until the next waypoint, in 1/10 second
       * Not including the effects of latency listed below.
       * Not sent in off track.  */
      uint16 timeToWpt;

      /** Estimated latency. The data above is _not_ corrected
       * for the expected latency. Useful mainly (only?) for timing
       * of user interface presentation. Unit: 1/10 second.  */
      uint16 latency;

      /** Current speed in m/s */
      uint16 speed;

      /** Currently driving over the speed limit? */
      bool overSpeed;

      /** Heading to the target */
      uint8 toTarget;

      /** The current crossing number. Crossings are numbered starting 
       *  at 0 (for the start crossing) */
      int16 crossingNo;
      
      RouteInfoParts::Segment  currSeg;
      /* distToAltAttribSegment is set to < 0 when there is no alternate 
       * attribute version of the current segment. */
      int32    distToAltAttribSegment;
      RouteInfoParts::Segment  altAttribSegment;
      RouteInfoParts::Crossing currCrossing;
      RouteInfoParts::Segment  nextSeg1;
      RouteInfoParts::Crossing nextCrossing1;
      RouteInfoParts::Segment  nextSeg2;

      typedef std::vector<RouteInfoParts::Landmark*> landmark_container;
      landmark_container rip_landmarks;

      RouteInfoParts::Lanes rip_lanes;

      RouteInfoParts::SignPost rip_signpost;

      void writeToBuf(Buffer *buf) const;
      void readFromBuf(Buffer *buf);

      // Ownership of the returned string is not passed and remains with
      // this object.
      const char * hasDetourLandmark(enum RouteInfoParts::Landmark::StartStopQuery q = RouteInfoParts::Landmark::ActiveNow) const;
      const char * hasSpeedCameraLandmark(enum RouteInfoParts::Landmark::StartStopQuery q = RouteInfoParts::Landmark::ActiveNow) const;
      int log(class Log* log) const;
   };

   /**
    */
   class RouteList {
      public:
      typedef std::vector <RouteInfoParts::RouteListCrossing *> container;
      typedef container::const_iterator const_iterator;
      typedef container::const_reverse_iterator const_reverse_iterator;
      /**
       * Container of pointers to crossings. These crossings are what
       * the route it made of.
       */
      container crossings;

      /**
       * Construct a RouteList object from a serialized representation
       * stored in a Buffer.
       * @param buf The Buffer to read from.
       */
      explicit RouteList(Buffer *buf);
      /**
       * Store a serialized representation of this RouteList object in
       * a Buffer.
       * @param buf The buffer to store the serialization in.
       */
      void writeToBuf(Buffer *buf) const;
      /**
       * Default constructor. 
       */
      RouteList() { };
      /**
       * Destructor. Will delete all RouteListCrossings stored in
       * crossings.
       */
      ~RouteList();
   };
}
#endif
