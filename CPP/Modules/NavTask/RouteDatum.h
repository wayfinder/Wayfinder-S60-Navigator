/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ROUTEDATUM_H
#define ROUTEDATUM_H
#include "arch.h"
#include "Penalties.h"

/** Returns the name of values from MetaActionType
 * @param num the MetaActionType value.
 * @return the symbolic name of the value, or UNKNOWN.
 */
#define routeMeta(num)                                                     \
((num == isab::RouteDatum::ORIGO) ? "ORIGO" :                              \
 ((num == isab::RouteDatum::SCALE) ? "SCALE" :                             \
  ((num == isab::RouteDatum::MINI_POINTS) ? "MINI_POINTS" :                \
   ((num == isab::RouteDatum::MICRO_DELTA_POINTS) ? "MICRO_DELTA_POINTS" : \
    ((num == isab::RouteDatum::META) ? "META" :                            \
     ((num == isab::RouteDatum::META_MAX) ? "META_MAX" :                   \
      ((num == isab::RouteDatum::META_MASK) ? "META_MASK" :                \
       "UNKNOWN")))))))

/** Returns the name of values from RouteActionType and MetaActionType.
 * @param num the value.
 * @return the symbolic name of the value, or UNKNOWN.
 */
#define actionName(num) \
   ((num & isab::RouteDatum::META_MASK) ? routeMeta(num) : routeAction(num))

/** Returns the name of values from CrossingType
 * @param num the CrossingTypeType value.
 * @return the symbolic name of the value, or UNKNOWN.
 */
#define  crossingString(num)                                      \
((num == isab::RouteDatum::NO_CROSSING) ? "NO_CROSSING" :         \
 ((num == isab::RouteDatum::THREE_WAY) ? "THREE_WAY" :            \
  ((num == isab::RouteDatum::FOUR_WAY) ? "FOUR_WAY" :             \
   ((num == isab::RouteDatum::MULTI_WAY) ? "MULTI_WAY" :          \
    "UNKNOWN"))))

namespace isab{
   class MiniPoints;
   class MicroDeltaPoints;
   class MetaPoint;
   class MetaPointOrigo;
   class MetaPointScale;
   class MetaPointLandmark;
   class MetaPointLaneInfo;
   class MetaPointLaneData;
   class MetaPointSignPost;
   class MetaPointTimeDistLeft;
   class RoutePoint;
   class Point;
   class Lane;
   /**
    *  The route datum structure contains two elements,
    *  the action element which describes the function
    *  of this datum, and the relevant structure for this
    *  type of datum.
    */
   class RouteDatum {
   public:
      /** Encodes what action should be taken at the NEXT waypoint. */
      enum RouteActionType {
         END_POINT        = 0x0000, //nav_route_point_end_point
         START_POINT      = 0x0001, //nav_route_point_start_point
         AHEAD            = 0x0002, //nav_route_point_ahead
         LEFT             = 0x0003, //nav_route_point_left
         RIGHT            = 0x0004, //nav_route_point_right
         UTURN            = 0x0005, //nav_route_point_uturn
         STARTAT          = 0x0006, //nav_route_point_startat
         FINALLY          = 0x0007, //nav_route_point_finally
         ENTER_RDBT       = 0x0008, //nav_route_point_enter_rdbt
         EXIT_RDBT        = 0x0009, //nav_route_point_exit_rdbt
         AHEAD_RDBT       = 0x000a, //nav_route_point_ahead_rdbt
         LEFT_RDBT        = 0x000b, //nav_route_point_left_rdbt
         RIGHT_RDBT       = 0x000c, //nav_route_point_right_rdbt
         EXITAT           = 0x000d, //nav_route_point_exitat
         ON               = 0x000e, //nav_route_point_on
         PARK_CAR         = 0x000f, //nav_route_point_park_car
         KEEP_LEFT        = 0x0010, //nav_route_point_keep_left
         KEEP_RIGHT       = 0x0011, //nav_route_point_keep_right

         START_WITH_UTURN = 0x0012, //nav_route_point_start_with_uturn
         UTURN_RDBT       = 0x0013, //nav_route_point_uturn_rdbt
         FOLLOW_ROAD      = 0x0014, //nav_route_point_follow_road
         ENTER_FERRY      = 0x0015, //nav_route_point_enter_ferry
         EXIT_FERRY       = 0x0016, //nav_route_point_exit_ferry
         CHANGE_FERRY     = 0x0017, //nav_route_point_change_ferry
         ROAD_END_LEFT    = 0x0018, //nav_route_point_endofroad_left
         ROAD_END_RIGHT   = 0x0019, //nav_route_point_endofroad_right
         OFF_RAMP_LEFT    = 0x001a, //nav_route_point_off_ramp_left
         OFF_RAMP_RIGHT   = 0x001b, //nav_route_point_off_ramp_right
         EXIT_RDBT_8      = 0x001c, //nav_route_point_exit_rdbt_8
         EXIT_RDBT_16     = 0x001d, //nav_route_point_exit_rdbt_8
         DELTA            = 0x03fe, //nav_route_point_delta
         TRACK_POINT      = 0x03fe, /** The new name, since it's not a delta 
                                        value. DELTA is kept for hysterical 
                                        reasons. */
         ACTION_MAX       = 0x03ff, //nav_route_point_max

         ACTION_MASK      = 0x03ff, /** Used to mask away crossing type and
                                        exit count values. */
         CROSSING_MASK    = 0x6000, /** Used to mask away everything but the
                                        crossing type bits.*/
         CROSSING_SHIFT   = 13,     /** Shift right with this amount to get a 
                                        CrossingType. */
         EXIT_MASK        = 0x1c00, /** Used to mask away everything but the
                                        exit count bits.*/
         EXIT_SHIFT       = 10,     /** Shift right with this amount to get a 
                                        integer holding the exit count. */
      };

      /** When not a RoutePoint, action holds one of these values. 
       * They all have values in the range 0x8000 to 0x807f inclusive.
       */ 
      enum RouteMetaType{
         ORIGO               = 0x8000, // nav_meta_point_origo
         SCALE               = 0x8001, // nav_meta_point_scale
         MINI_POINTS         = 0x8002, // nav_meta_point_mini_delta_points
         MICRO_DELTA_POINTS  = 0x8003, // nav_meta_point_micro_delta_points
         META                = 0x8005, // nav_meta_point_meta
         TDL                 = 0x8006, // nav_meta_point_tdl
         LANDMARK            = 0x8007, // nav_meta_point_landmark
         LANE_INFO           = 0x8008, // nav_meta_point_lane_info
         LANE_DATA           = 0x8009, // nav_meta_point_lane_data
         SIGNPOST            = 0x800a, // nav_meta_point_signpost
         META_MAX            = 0x807f, // nav_meta_point_max
         META_MASK           = 0x8000  /** Useful when determining if a 
                                           value is included in this enum. */
      };
      
      /** encodes what kind of crossing the NEXT waypoint holds. */
      enum CrossingType{
         NO_CROSSING = 0, /** No crossing. Something else happens.*/
         THREE_WAY = 1,   /** Three way crossing. */
         FOUR_WAY = 2,    /** Four way crossing. */
         MULTI_WAY = 3,   /** Crossing with more than 4 exits. */
      };

      /** Tests if this RouteDatum's action is set to the specified action.
       * @param action the action to test for.
       * @return true if this RouteDatum's action field is equal to the 
       *         action parameter.
       */
      inline bool actionIs(uint16 action);

      /** Tests if this RouteDatum object's action is set to 
       * RouteActionType::START_POINT. 
       * @return true if the action field equals START_POINT. */
      inline bool actionIsStart();

      /** Tests if this RouteDatum object's action is set to 
       * RouteActionType::END_POINT. 
       * @return true if the action field equals END_POINT. */
      inline bool actionIsEnd(bool allowTruncated); 

      /** Tests if this RouteDatum object can be considered a waypoint.
       * All RouteDatums that are not MetaPoints of some sort and not 
       * FullDeltaPoints are Waypoints.
       * @return true if this RouteDatum can be considered a Waypoint. */
      inline bool actionIsWaypoint();

      /** Tests if this RouteDatum object's action is set to 
       * RouteActionType::DELTA.
       * @return true if the action field equals DELTA. */
      inline bool actionIsFullDeltaPoint();
      
      /** Replaces actionIsFullDeltaPoint. Since the data in a full delta 
       * point wasn't delta coded we now try to use TRACK_POINT instead. 
       * actionIsFullDeltaPoint will kept for hysterical reasons.
       * @return true if the action field equals TRACK_POINT or DELTA.
       */
      inline bool actionIsTrackpoint();

      /** Tests if this RouteDatum object's action is set to 
       * RouteMetaType::ORIGO. 
       * @return true if the action field equals ORIGO. */
      inline bool actionIsOrigo();

      /** Tests if this RouteDatum object's action is set to 
       * RouteMetaType::SCALE.
       * @return true if the action field equals SCALE. */
      inline bool actionIsScale();

      /** Tests if this RouteDatum object is a MiniPoints object.
       * @return true if the action field equals MINI_POINTS. */
      inline bool actionIsMiniPoint();

      /** Tests if this RouteDatum object is a MicroDeltaPoints object,
       * a MiniPoints object, or a RoutePoint marked DELTA.
       * @return true if this object matches the above criteria. */
      inline bool actionIsDeltaPoint();

      /** Tests if this RouteDatum object is a TimeDistLeft packet.
       * @return true if this is a TDL point. */
      inline bool actionIsTimeDistLeft();

      /** Tests if this RouteDatum object is a Landmark packet.
       * @return true if this is a Landmark point. */
      inline bool actionIsLandmark();

      /** Tests if this RouteDatum object is a Lane info packet.
       * @return true if this is a Lane info point. */
      inline bool actionIsLaneInfo();

      /** Tests if this RouteDatum object is a Lane info packet.
       * @return true if this is a Lane data point. */
      inline bool actionIsLaneData();

      /** Tests if this RouteDatum object is a Sign post packet.
       * @return true if this is a Sign post point. */
      inline bool actionIsSignPost();

      /** Tests if this RouteDatum object is of one of the Meta classes.
       * @return true if this is a Meta point. */
      inline bool actionIsMeta();

      /** Tests if this RouteDatum object is a MicroDeltaPoints object.
       * @return true if the action field equals MICRO_DELTA_POINTS. */
      inline bool actionIsMicroDeltaPoint();


      /** Copies this waypoint.
       * @param dest a pointer to where to write the copy.
       * @return 0 if dest was a NULL-pointer, 1 otherwise.
       */
      int16 copyWaypoint(RouteDatum* dest);
      
      /** Calculates this RouteDatum's ID. 
       * @param start should point to the first RouteDatum of the route.
       * @return a uint16 RouteDatum ID.
       */
      uint16 getPointId(RouteDatum* start);

      /** Find the closest preceeding MetaPointOrigo in the route.
       * @return a pointer to the preceeding MetaPointOrigo closest this point.
       */
      MetaPointOrigo* getOrigo();

      /** Converts this object to a MiniPoints object.
       * @return a reference to this object as a MiniPoints object. */
      inline MiniPoints& asMiniPoints();

      /** Converts this object to a MiniPoints object.
       * @return a pointer to this object as a MiniPoints object. */
      inline MiniPoints* asMiniPointsP();

      /** Converts this object to a MetaPointOrigo object.
       * @return a reference to this object as a MetaPointsOrigo object. */
      inline MetaPointOrigo& asMetaPointOrigo();

      /** Converts this object to a MetaPointOrigo object.
       * @return a pointer to this object as a MetaPointOrigo object. */
      inline MetaPointOrigo* asMetaPointOrigoP();

      /** Converts this object to a MetaPointTimeDistLeft object.
       * @return a reference to this object as a MetaPointsTimeDistLeft object. */
      inline MetaPointTimeDistLeft& asMetaPointTimeDistLeft();

      /** Converts this object to a MetaPointTimeDistLeft object.
       * @return a pointer to this object as a MetaPointTimeDistLeft object. */
      inline MetaPointTimeDistLeft* asMetaPointTimeDistLeftP();

      /** Converts this object to a MicroDeltaPoints object.
       * @return a reference to this object as a MicroDeltaPoints object. */
      inline MicroDeltaPoints& asMicroDeltaPoints();

      /** Converts this object to a MicroDeltaPoints object.
       * @return a pointer to this object as a MicroDeltaPoints object. */
      inline MicroDeltaPoints* asMicroDeltaPointsP();

      /** Converts this object to a MetaPointScale object.
       * @return a reference to this object as a MetaPointScale object. */
      inline MetaPointScale& asMetaPointScale();

      /** Converts this object to a MetaPointScale object.
       * @return a pointer to this object as a MetaPointScale object. */
      inline MetaPointScale* asMetaPointScaleP();

      /** Converts this object to a MetaPointLandmark object.
       * @return a reference to this object as a MetaPointLandmark object. */
      inline MetaPointLandmark& asMetaPointLandmark();

      /** Converts this object to a MetaPointLandmark object.
       * @return a pointer to this object as a MetaPointLandmark object. */
      inline MetaPointLandmark* asMetaPointLandmarkP();

      /** Converts this object to a MetaPointLaneInfo object.
       * @return a reference to this object as a MetaPointLaneInfo object. */
      inline MetaPointLaneInfo& asMetaPointLaneInfo();

      /** Converts this object to a MetaPointLaneInfo object.
       * @return a pointer to this object as a MetaPointLaneInfo object. */
      inline MetaPointLaneInfo* asMetaPointLaneInfoP();

      /** Converts this object to a MetaPointLaneData object.
       * @return a reference to this object as a MetaPointLaneData object. */
      inline MetaPointLaneData& asMetaPointLaneData();

      /** Converts this object to a MetaPointLaneData object.
       * @return a pointer to this object as a MetaPointLaneData object. */
      inline MetaPointLaneData* asMetaPointLaneDataP();

      /** Converts this object to a MetaPointSignPost object.
       * @return a reference to this object as a MetaPointSignPost object. */
      inline MetaPointSignPost& asMetaPointSignPost();

      /** Converts this object to a MetaPointSignPost object.
       * @return a pointer to this object as a MetaPointSignPost object. */
      inline MetaPointSignPost* asMetaPointSignPostP();

      /** Converts this object to a MetaPoint object.
       * @return a reference to this object as a MetaPoint object. */
      inline MetaPoint& asMetaPoint();

      /** Converts this object to a MetaPoint object.
       * @return a pointer to this object as a MetaPoint object. */
      inline MetaPoint* asMetaPointP();

      /** Converts this object to a RoutePoint object.
       * @return a reference to this object as a RoutePoint object. */
      inline RoutePoint& asRoutePoint();

      /** Converts this object to a RoutePoint object.
       * @return a pointer to this object as a RoutePoint object. */
      inline RoutePoint* asRoutePointP();

      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 12 bytes of valid data.
       */
      void fill(const uint8* rawData);

      /** Provides access to the action field. This operation can be used 
       * both for reads and writes.
       * @return a reference to the action field. */
      inline uint16& action();
   protected:
      /** One of RouteActionType or RouteMetaType. */
      uint16 m_action;
      /** The data area. Interpreted differently depending on what subclass
       * is doing the interpreting. */
      uint8 m_data[10]; 

      /** Translates two unsigned bytes in network byte order to a 
       * uint16 in host byte order.
       * @param raw a pointer to at least two bytes of valid data. Note that 
       *            raw is passed by reference, and is updated to point two
       *            bytes later in memory.
       * @return the data as a uint16.
       */
      inline uint16 u16ntoh(const uint8*& raw);
      /** Translates four unsigned bytes in network byte order to a 
       * uint32 in host byte order.
       * @param raw a pointer to at least four bytes of valid data. Note that 
       *            raw is passed by reference, and is updated to point four
       *            bytes later in memory.
       * @return the data as a uint32.
       */
      inline uint32 u32ntoh(const uint8*& raw);
      /** Translates two unsigned bytes in network byte order to a 
       * int16 in host byte order.
       * @param raw a pointer to at least two bytes of valid data. Note that 
       *            raw is passed by reference, and is updated to point two
       *            bytes later in memory.
       * @return the data as a int16.
       */
      inline int16 s16ntoh(const uint8*& raw);
      /** Translates four unsigned bytes in network byte order to a 
       * int32 in host byte order.
       * @param raw a pointer to at least four bytes of valid data. Note that 
       *            raw is passed by reference, and is updated to point four
       *            bytes later in memory.
       * @return the data as a int32.
       */
      inline int32 s32ntoh(const uint8*& raw);
   
   private:
   };

   namespace{
      const char*  routeAction(RouteDatum::RouteActionType datum);
   }

   /** The RoutePoint class describes a point of some kind to be used 
    * as either an active point on the route or a meta point used only
    * for route detection, ie waypoint or trackpoint.
    * The difference between the two is only in the action field.
    */
   class RoutePoint : public RouteDatum{
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);

      /** Provides access to the flags field. Usable both for reads and 
       * writes. 
       * @return a reference to the flags field. */   
      inline uint8& rpFlags();
      
      /** Provides access to the speedLimit field. Usable both for reads 
       * and writes. 
       * @return a reference to the speedLimit field. */   
      inline uint8& speedLimit();
      
      /** Provides access to the x field. Usable both for reads and writes. 
       * @return a reference to the x field. */   
      inline int16& x();
      
      /** Provides access to the y field. Usable both for reads and writes. 
       * @return a reference to the y field. */   
      inline int16& y();
      
      /** Provides access to the meters field. Usable both for reads and 
       * writes. 
       * @return a reference to the meters field. */   
      inline uint16& meters();
      
      /** Provides access to the streetNameIndex field. Usable both for 
       * reads and writes. 
       * @return a reference to the streetNameIndex field. */   
      inline uint16& streetNameIndex();

      /** Gets the crossing type of this RoutePoint.
       * @return one of the values in the enum CrossingType.*/
      inline CrossingType crossingType();

      /** Gets the exit count of this RoutePoint.
       * @return the exit count.
       */
      inline uint8 exitCount();
   protected:
   private:
   };


   /**
    * The time left along the route at this point as well as the 
    * distance left.
    */
   class MetaPointTimeDistLeft : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the reserved field. Usable both for reads 
       * and writes.  
       * @return a reference to the reserved field. */ 
      inline uint16& reserved();
      /** Provides access to the timeLeft field. Usable both for reads 
       * and writes.  
       * @return a reference to the timeLeft field. */ 
      inline int32& timeLeft();
      /** Provides access to the distLeft field. Usable both for reads 
       * and writes.  
       * @return a reference to the distLeft field. */ 
      inline int32& distLeft();
   protected:
   private:
   };



   /**
    *  The route origo point describes a point which is to be
    *  used as origo when calculating the local coordinates
    *  from the GPS-data (WGS-84)
    */
   class MetaPointOrigo : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the nextOrigio field. Usable both for reads 
       * and writes.  
       * @return a reference to the nextOrigio field. */      
      inline uint16& nextOrigo();
      /** Provides access to the origoX field. Usable both for reads 
       * and writes.  
       * @return a reference to the origoX field. */ 
      inline int32& origoX();
      /** Provides access to the origoY field. Usable both for reads 
       * and writes.  
       * @return a reference to the origoY field. */ 
      inline int32& origoY();
   protected:
   private:
   };
   /**
    *  The route scale datum contains the scale factor to
    *  be used in the conversion of the GPS-data (WGS-84)
    *  to local coordinates.
    *
    *  Observe that the Latitude scale factor is constant
    *  where ever on the globe we are.
    *
    *  Fixed point value, scale_x1 is integer part and
    *  scale_x2 describes the fraction in 1/65536 parts.
    *  That is, scale_x2 is calculated by
    *   scale_x2 = (scale - scale_x1)*65536
    *  Since (scale - scale_x1) is between 1 and zero,
    *  scale_x2 will be between 65536 and zero.
    */
   class MetaPointScale : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the refX field. Usable both for reads and writes.
       * @return a reference to the refX field. */      
      inline int16& refX();
      /** Provides access to the refY field. Usable both for reads and writes.
       * @return a reference to the refY field. */      
      inline int16& refY();
      /** Provides access to the scaleX field. Usable both for reads 
       * and writes.
       * @return a reference to the scaleX field. */      
      inline uint16& scaleX2();
      /** Provides access to the scaleX field. Usable both for reads 
       * and writes.
       * @return a reference to the scaleX field. */      
      inline uint32& scaleX1();
   private:
   };
   /**
    * This datum represents one of the following 
    * conditions:
    *    a point like landmark
    *    the beginning of an extended landmark
    *    the end of an extended landmark
    *    a reminder of an active landmark.
    *
    *  A pointlike landmark is identified by the
    *  flags start=stop=1.
    *
    *  For extended landmarks the start and stop bits
    *  are used one at a time. If the landmark both
    *  begins and ends between the same two waypoints
    *  two MetaPointLandmark are output.
    *
    *  Reminders of active extended landmarks are placed 
    *  after the MetaPointScale of each new minimap. This
    *  allows a quick scan of the minimaps to not miss 
    *  the starts/stops of active landmarks.
    *
    *  The flags field is bit coded.
    *  The start/stop bits did not fit into the flags filed
    *  and were placed at the top of the id word.
    */
   class MetaPointLandmark : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the distance field. Usable both for 
       * reads and writes. 
       * @return a reference to the distance field. */   
      inline int32& distance();
      /** Provides access to the streetNameIndex field. Usable both for 
       * reads and writes. 
       * @return a reference to the streetNameIndex field. */   
      inline uint16& streetNameIndex();
      /** Provides access to the start flag. Usable ONLY for 
       * reads. 
       * @return true if the start flag is set. */   
      inline bool isStart();
      /** Provides access to the stop flag. Usable ONLY for 
       * reads. 
       * @return true if the stop flag is set. */   
      inline bool isStop();
      /** Provides access to the disturbed route flag. Usable ONLY for 
       * reads. 
       * @return true if the disturbed route flag is set. */   
      inline bool isDisturbedRoute();
      /** Provides access to the road side. Usable ONLY for 
       * reads. 
       * @return a copy of the road side. */   
      inline uint16 roadSide();
      /** Provides access to the landmark type. Usable ONLY for 
       * reads. 
       * @return a copy of the landmark type. */   
      inline uint16 landmarkType();
      /** Provides access to the landmark location. Usable ONLY for 
       * reads. 
       * @return a copy of the landmark location. */   
      inline uint16 landmarkLocation();
      /** Provides access to the landmark id. Usable ONLY for 
       * reads. 
       * @return a copy of the landmark id. */   
      inline uint16 id();
   protected:
      /** Provides access to the raw flags field. Usable both for 
       * reads and writes. 
       * @return a reference to the raw flags field. */   
      inline uint16& flags();
      /** Provides access to the raw id_and_startstop field. Usable both for 
       * reads and writes. 
       * @return a reference to the raw id_and_startstop field. */   
      inline uint16& idAndStartstop();
   private:
   };
   /**
    *  The meta point for lane info.
    */
   class MetaPointLaneInfo : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      void fill(const uint8* rawData);
      /** Provides access to the flags field. Usable both for 
       * reads and writes. 
       * @return a reference to the flags field. */   
      uint8& flags();
      /** Provides access to the nbrLanes field. Usable both for 
       * reads and writes. 
       * @return a reference to the nbrLanes field. */   
      uint8& nbrLanes();
      /** Provides access to the lane at index idx. Only for 
       * reads.
       * @param idx the index of the lane.
       * @return a reference to a lane field. */   
      Lane& getlane(int idx);
      /** Provides access to the first lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the first lane field. */   
      Lane& lane1();
      /** Provides access to the second lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the second lane field. */   
      Lane& lane2();
      /** Provides access to the third lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the third lane field. */   
      Lane& lane3();
      /** Provides access to the fourth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the fourth lane field. */   
      Lane& lane4();
      /** Provides access to the distance field. Usable both for 
       * reads and writes. 
       * @return a reference to the distance field. */   
      int32& distance();
      /** Provides access to the stop flag. Usable ONLY for 
       * reads. 
       * @return true if the stop flag is set. */   
      bool stopOfLanes();
      /** Provides access to the reminder of lanes flag. Usable ONLY for 
       * reads. 
       * @return true if the reminder of lanes flag is set. */   
      bool remindOfLanes();
   };
   /**
    *  The meta point for lane data.
    */
   class MetaPointLaneData : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      void fill(const uint8* rawData);
      /** Provides access to the lane at index idx. Only for 
       * reads.
       * @param idx the index of the lane.
       * @return a reference to a lane field. */   
      Lane& getlane(int idx);
      /** Provides access to the first lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the first lane field. */   
      Lane& lane1();
      /** Provides access to the second lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the second lane field. */   
      Lane& lane2();
      /** Provides access to the third lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the third lane field. */   
      Lane& lane3();
      /** Provides access to the fourth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the fourth lane field. */   
      Lane& lane4();
      /** Provides access to the fifth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the fifth lane field. */   
      Lane& lane5();
      /** Provides access to the sixth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the sixth lane field. */   
      Lane& lane6();
      /** Provides access to the seventh lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the seventh lane field. */   
      Lane& lane7();
      /** Provides access to the eigth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the eigth lane field. */   
      Lane& lane8();
      /** Provides access to the ninth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the ninth lane field. */   
      Lane& lane9();
      /** Provides access to the tenth lane field. Usable both for 
       * reads and writes. 
       * @return a reference to the tenth lane field. */   
      Lane& lane10();
   };
   /**
    *  The meta point for lane data.
    */
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

      bool isPreferred();

      bool isAllowed();

      Direction direction();

      bool isHalfLeft();

      bool isStraight();

      bool isHalfRight();

      bool isFullRight();

      uint8 m_lane;
   };
   /**
    *  The meta point for sign posts.
    */
   class MetaPointSignPost : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      void fill(const uint8* rawData);
      /** Provides access to the text index field. Usable both for reads and writes.
       * @return a reference to the text index field. */ 
      uint16& textIndex();
      /** Provides access to the text color field. Usable both for 
       * reads and writes. 
       * @return a reference to the text color field. */
      uint8& textColor();
      /** Provides access to the background color field. Usable both for 
       * reads and writes. 
       * @return a reference to the background color field. */
      uint8& backgroundColor();
      /** Provides access to the front color field. Usable both for 
       * reads and writes. 
       * @return a reference to the front color field. */
      uint8& frontColor();
      /** Provides access to the distance field. Usable both for 
       * reads and writes. 
       * @return a reference to the distance field. */
      int32& distance();
   };


   /**
    *  The meta point can be used for any kind of values
    *  which need data in 16-bit units.
    */
   class MetaPoint : public RouteDatum {
   public:
      /** The following values of the type are defined.*/
      enum MetaPointType{
         TEXT           = 1,
         SOUND          = 2,
         WAYPOINT       = 3,
         SEGMENT_END    = 4,
         REPORT_POINT   = 5,
      };
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the type field. Usable both for reads and writes.
       * @return a reference to the type field. */      
      inline uint16& type();
      /** Provides access to the a field. Usable both for reads and writes.
       * @return a reference to the a field. */      
      inline uint16& a();
      /** Provides access to the b field. Usable both for reads and writes.
       * @return a reference to the b field. */      
      inline uint16& b();
      /** Provides access to the c field. Usable both for reads and writes.
       * @return a reference to the c field. */      
      inline uint16& c();
      /** Provides access to the d field. Usable both for reads and writes.
       * @return a reference to the d field. */      
      inline uint16& d();
   };

   /**
    *  The nav_mini_delta_point can be used to pack trackpoints
    *  into a more compressed structure. The x and y offsets
    *  are the same as those found in the RoutePoint class.
    *  The "meters" and "flags" fields are missing.
    *
    *  The missing "meters" causes us to have to do a 
    *  more complicated distance calculation if we need the 
    *  distance to/from a MiniPoint. We cannot just
    *  sum the distances as we do for the full trackpoints
    *  (and thus for the waypoints as well).
    */
   class MiniPoint {
   public:
      int16 x;
      int16 y;
   };
 
   /**
    * This can be used to pack trackpoints
    * into a more compressed structure. The x and y offsets
    * are in units from last point (route or track).
    *
    * Using 1 meters as the unit, we can describe points
    * that lie within a square of 127 meters from the
    * last point.
    */
   class MicroDeltaPoint {
   public:
      int8 x;
      int8 y;
   };

   /**
    *  The MicroDeltaPoints are used to pack
    *  a number of MicroDeltaPoints into one structure.
    *  Using all available space gives us 5 MicroDeltaPoint
    *  per MicroDeltaPopints.
    *
    *  We can convey some minor data using the action field,
    *  but if we need anything more than the coordinates we
    *  could reuse one of the fields as data carrier.
    */
   class MicroDeltaPoints : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);

      /** Accesses the MicorDeltaPoints contained in this class. 
       * Note that no boundary checking. Don't use a idx parameter that 
       * is bigger than 4 or less than 0, or you will be sorry. 
       * @param idx index from 0 to 4.
       * @return a refernce to a MicroDeltaPoint.
       */
      inline MicroDeltaPoint& operator[](int idx);
   };

   /**
    *  The MiniPoints are used to pack
    *  a number of MiniPoint into one structure.
    *  Using all available space gives us 2 MiniPoint
    *  per MiniPoints.
    *
    *  The order of the fields may seem strange, but has hysterical reasons.
    */
   class MiniPoints : public RouteDatum {
   public:
      /** Takes raw binary data in network byte order and sorts it to
       * the correct byte order. 
       * @param rawData a pointer to at least 10 bytes of valid data.
       */
      inline void fill(const uint8* rawData);
      /** Provides access to the p field. Usable both for reads and writes.
       * @return a reference to the p field. */ 
      inline MiniPoint& p1();
      /** Provides access to the p field. Usable both for reads and writes.
       * @return a reference to the p field. */      
      inline MiniPoint& p2();
      /** Provides access to the speedLimit field. Usable both for reads 
       * and writes.
       * @return a reference to the speedLimit field. */      
      inline uint8& speedLimit1();
      /** Provides access to the speedLimit field. Usable both for reads 
       * and writes.
       * @return a reference to the speedLimit field. */      
      inline uint8& speedLimit2();
   };

   //==========================================================
   //==== inlines for RouteDatum ==============================
   inline bool RouteDatum::actionIs(uint16 action)
   {
      /* masking both values should allow to compare Meta values as well.*/
      return (m_action & ACTION_MASK) == (action & ACTION_MASK);
   }

   inline bool RouteDatum::actionIsStart()
   {
      return m_action == START_POINT;
   }
   inline bool RouteDatum::actionIsEnd(bool allowTruncated)
   {
      if (allowTruncated) {
         return ((m_action == END_POINT) && (uint8(asRoutePoint().rpFlags()) != uint8(0xff)) );
      } else {
         return m_action == END_POINT;
      }
   }
   inline bool RouteDatum::actionIsWaypoint()
   {
      return (!actionIsMeta() && !actionIsFullDeltaPoint());
   }
   inline bool RouteDatum::actionIsFullDeltaPoint()
   {
      return m_action == DELTA;
   }
   inline bool RouteDatum::actionIsTrackpoint()
   {
      return m_action == TRACK_POINT;
   }
   inline bool RouteDatum::actionIsOrigo()
   {
      return m_action == ORIGO;
   }
   inline bool RouteDatum::actionIsScale()
   {
      return m_action == SCALE;
   }
   inline bool RouteDatum::actionIsTimeDistLeft()
   {
      return m_action == TDL;
   }
   inline bool RouteDatum::actionIsLandmark()
   {
      return m_action == LANDMARK;
   }
   inline bool RouteDatum::actionIsLaneInfo()
   {
      return m_action == LANE_INFO;
   }
   inline bool RouteDatum::actionIsLaneData()
   {
      return m_action == LANE_DATA;
   }
   inline bool RouteDatum::actionIsSignPost()
   {
      return m_action == SIGNPOST;
   }
   inline bool RouteDatum::actionIsMeta()
   {
      return (m_action & META_MASK) == META_MASK;
   }
   inline bool RouteDatum::actionIsMiniPoint()
   {
      return m_action == MINI_POINTS;
   }
   inline bool RouteDatum::actionIsMicroDeltaPoint()
   {
      return m_action == MICRO_DELTA_POINTS;
   }
   inline bool RouteDatum::actionIsDeltaPoint()
   {
      return actionIsFullDeltaPoint() || 
         actionIsMiniPoint() || 
         actionIsMicroDeltaPoint();
   }
   inline MiniPoints& RouteDatum::asMiniPoints()
   {
      return static_cast<MiniPoints&>(*this);
   }
   inline MiniPoints* RouteDatum::asMiniPointsP()
   {
      return static_cast<MiniPoints*>(this);
   }
   inline MetaPointOrigo& RouteDatum::asMetaPointOrigo()
   {
      return static_cast<MetaPointOrigo&>(*this);
   }
   inline MetaPointOrigo* RouteDatum::asMetaPointOrigoP() 
   {
      return static_cast<MetaPointOrigo*>(this);
   }
   inline MetaPointTimeDistLeft& RouteDatum::asMetaPointTimeDistLeft()
   {
      return static_cast<MetaPointTimeDistLeft&>(*this);
   }
   inline MetaPointTimeDistLeft* RouteDatum::asMetaPointTimeDistLeftP() 
   {
      return static_cast<MetaPointTimeDistLeft*>(this);
   }
   inline MicroDeltaPoints& RouteDatum::asMicroDeltaPoints()
   {
      return static_cast<MicroDeltaPoints&>(*this);
   }
   inline MicroDeltaPoints* RouteDatum::asMicroDeltaPointsP()
   {
      return static_cast<MicroDeltaPoints*>(this);
   }
   inline MetaPointScale& RouteDatum::asMetaPointScale()
   {
      return static_cast<MetaPointScale&>(*this);
   }
   inline MetaPointScale* RouteDatum::asMetaPointScaleP()
   {
      return static_cast<MetaPointScale*>(this);
   }
   inline MetaPointLandmark& RouteDatum::asMetaPointLandmark()
   {
      return static_cast<MetaPointLandmark&>(*this);
   }
   inline MetaPointLandmark* RouteDatum::asMetaPointLandmarkP()
   {
      return static_cast<MetaPointLandmark*>(this);
   }
   inline MetaPointLaneInfo& RouteDatum::asMetaPointLaneInfo()
   {
      return static_cast<MetaPointLaneInfo&>(*this);
   }
   inline MetaPointLaneInfo* RouteDatum::asMetaPointLaneInfoP()
   {
      return static_cast<MetaPointLaneInfo*>(this);
   }
   inline MetaPointLaneData& RouteDatum::asMetaPointLaneData()
   {
      return static_cast<MetaPointLaneData&>(*this);
   }
   inline MetaPointLaneData* RouteDatum::asMetaPointLaneDataP()
   {
      return static_cast<MetaPointLaneData*>(this);
   }
   inline MetaPointSignPost& RouteDatum::asMetaPointSignPost()
   {
      return static_cast<MetaPointSignPost&>(*this);
   }
   inline MetaPointSignPost* RouteDatum::asMetaPointSignPostP()
   {
      return static_cast<MetaPointSignPost*>(this);
   }
   inline MetaPoint& RouteDatum::asMetaPoint()
   {
      return static_cast<MetaPoint&>(*this);
   }
   inline MetaPoint* RouteDatum::asMetaPointP()
   {
      return static_cast<MetaPoint*>(this);
   }
   inline RoutePoint& RouteDatum::asRoutePoint()
   {
      return static_cast<RoutePoint&>(*this);
   }
   inline RoutePoint* RouteDatum::asRoutePointP()
   {
      return static_cast<RoutePoint*>(this);
   }
   
   inline uint16& RouteDatum::action()
   {
      return m_action;
   }

   //==========================================================
   //====== inline helpers for the fill methods ===============
   inline uint16 RouteDatum::u16ntoh(const uint8*& raw)
   {
      uint16 data;
      data = uint16(raw[0]) << 8 | raw[1] ;
      raw += 2;
      return data;
   }
   inline uint32 RouteDatum::u32ntoh(const uint8*& raw)
   {
      uint32 data;
      data = uint32(raw[0]) << 24 | 
             uint32(raw[1]) << 16 |
             uint32(raw[2]) <<  8 |
             raw[3] ;
      raw += 4;
      return data;
   }
   inline int16 RouteDatum::s16ntoh(const uint8*& raw)
   {
      return u16ntoh(raw);
   }
   inline int32 RouteDatum::s32ntoh(const uint8*& raw)
   {
      return u32ntoh(raw);
   }

   

   //==========================================================
   //====== inlines for RoutePoint ============================
   inline void RoutePoint::fill(const uint8* rawData)
   {
      rpFlags() = *rawData++;
      speedLimit() = *rawData++;
      x() = s16ntoh(rawData);
      y() = s16ntoh(rawData);
      meters() = u16ntoh(rawData);
      streetNameIndex() = u16ntoh(rawData);
   }
   inline uint8& RoutePoint::rpFlags()
   {
      return m_data[0];
   }
   inline uint8& RoutePoint::speedLimit()
   {
      return m_data[1];
   }
   inline int16& RoutePoint::x()
   { 
      return reinterpret_cast<int16*>(m_data)[1];
   }
   inline int16& RoutePoint::y()
   {
      return reinterpret_cast<int16*>(m_data)[2];
   }
   inline uint16& RoutePoint::meters()
   {
      return reinterpret_cast<uint16*>(m_data)[3];
   }
   inline uint16& RoutePoint::streetNameIndex()
   {
      return reinterpret_cast<uint16*>(m_data)[4];
   }
   inline RouteDatum::CrossingType RoutePoint::crossingType()
   {
      CrossingType ret = CrossingType(m_action >> CROSSING_SHIFT);
      return ret;
   }
   inline uint8 RoutePoint::exitCount()
   {
      uint8 ret = (m_action & EXIT_MASK) >> EXIT_SHIFT;
      return ret;
   }
   //==================================================
   //====== inlines for MetaPointTimeDistLeft ================
   inline void MetaPointTimeDistLeft::fill(const uint8* rawData)
   {
      reserved() = s16ntoh(rawData);  // Reads the reserved data....
      timeLeft() = s32ntoh(rawData);
      distLeft() = s32ntoh(rawData);
   }
   inline uint16& MetaPointTimeDistLeft::reserved()
   {
      return reinterpret_cast<uint16*>(m_data)[0];
   }
   inline int32& MetaPointTimeDistLeft::timeLeft()
   {
      return reinterpret_cast<int32*>(m_data + 2)[0];
   }
   inline int32& MetaPointTimeDistLeft::distLeft()
   {
      return reinterpret_cast<int32*>(m_data + 2)[1];
   }
   //==================================================
   //====== inlines for MetaPointOrigo ================
   inline void MetaPointOrigo::fill(const uint8* rawData)
   {
      nextOrigo() = u16ntoh(rawData);
      origoX() = s32ntoh(rawData);
      origoY() = s32ntoh(rawData);
   }
   inline uint16& MetaPointOrigo::nextOrigo()
   {
      return reinterpret_cast<uint16*>(m_data)[0];
   }
   inline int32& MetaPointOrigo::origoX()
   {
      return reinterpret_cast<int32*>(m_data + 2)[0];
   }
   inline int32& MetaPointOrigo::origoY()
   {
      return reinterpret_cast<int32*>(m_data + 2)[1];
   }
   //==================================================
   //======= inlines for MetaPointScale ===============
   inline void MetaPointScale::fill(const uint8* rawData)
   {
      refX() = s16ntoh(rawData);
      refY() = s16ntoh(rawData);
      scaleX2() = u16ntoh(rawData);
      scaleX1() = u32ntoh(rawData);
   }
   inline int16& MetaPointScale::refX()
   {
      return reinterpret_cast<int16*>(m_data)[0];
   }
   inline int16& MetaPointScale::refY()
   {
      return reinterpret_cast<int16*>(m_data)[1];
   }
   inline uint16& MetaPointScale::scaleX2()
   {
      return reinterpret_cast<uint16*>(m_data)[2];
   }
   inline uint32& MetaPointScale::scaleX1()
   {
      return reinterpret_cast<uint32*>(m_data + 2)[1];
   }
   //=================================================
   //======= inlines for MetaPointLandmark ===============
   inline void MetaPointLandmark::fill(const uint8* rawData)
   {
      flags() = u16ntoh(rawData);
      streetNameIndex() = u16ntoh(rawData);
      idAndStartstop() = u16ntoh(rawData);
      distance() = s32ntoh(rawData);
   }
   inline uint16& MetaPointLandmark::flags()
   {
      return reinterpret_cast<uint16*>(m_data)[0];
   }
   inline uint16& MetaPointLandmark::streetNameIndex()
   {
      return reinterpret_cast<uint16*>(m_data)[1];
   }
   inline uint16& MetaPointLandmark::idAndStartstop()
   {
      return reinterpret_cast<uint16*>(m_data)[2];
   }
   inline int32& MetaPointLandmark::distance()
   {
      return reinterpret_cast<int32*>(m_data + 2)[1];
   }
   inline bool MetaPointLandmark::isStart()
   {
      return (idAndStartstop() & 0x8000) != 0;
   }
   inline bool MetaPointLandmark::isStop()
   {
      return (idAndStartstop() & 0x4000) != 0;
   }
   inline bool MetaPointLandmark::isDisturbedRoute()
   {
      return (flags() & 0xe000) == 0;
   }
   inline uint16 MetaPointLandmark::roadSide()
   {
      return (flags() & 0x1c00) >> 10;
   }
   inline uint16 MetaPointLandmark::landmarkType()
   {
      return (flags() & 0x03e0) >> 5;
   }
   inline uint16 MetaPointLandmark::landmarkLocation()
   {
      return (flags() & 0x001f) ;
   }
   inline uint16 MetaPointLandmark::id()
   {
      return (idAndStartstop() & 0x3fff);
   }

   //=================================================
   //========= inlines for MetaPoint =================
   inline void MetaPoint::fill(const uint8* rawData)
   {
      type() = u16ntoh(rawData);
      a() = u16ntoh(rawData);
      b() = u16ntoh(rawData);
      c() = u16ntoh(rawData);
      d() = u16ntoh(rawData);
   }
   inline uint16& MetaPoint::type()
   {
      return reinterpret_cast<uint16*>(m_data)[0];
   }
   inline uint16& MetaPoint::a()
   {
      return reinterpret_cast<uint16*>(m_data)[1];
   }
   inline uint16& MetaPoint::b()
   {
      return reinterpret_cast<uint16*>(m_data)[2];
   }
   inline uint16& MetaPoint::c()
   {
      return reinterpret_cast<uint16*>(m_data)[3];
   }
   inline uint16& MetaPoint::d()
   {
      return reinterpret_cast<uint16*>(m_data)[4];
   }
   //=========================================================
   //========== Inlines for MicroDeltaPoints =================
   inline void MicroDeltaPoints::fill(const uint8* rawData)
   {
      memcpy(m_data, rawData, 5 * 2);
   }
   inline MicroDeltaPoint& MicroDeltaPoints::operator[](int idx)
   {
      return *reinterpret_cast<MicroDeltaPoint*>(m_data+idx*2);
   }
   //=========================================================
   //========== Inlines for MiniPoints =======================
   inline void MiniPoints::fill(const uint8* rawData)
   {
      p1().x = u16ntoh(rawData);
      p1().y = u16ntoh(rawData);
      p2().x = u16ntoh(rawData);
      p2().y = u16ntoh(rawData);
      speedLimit1() = *(rawData++);
      speedLimit2() = *(rawData++);
   }
   inline MiniPoint& MiniPoints::p1()
   {
      return reinterpret_cast<MiniPoint*>(m_data)[0];
   }
   inline MiniPoint& MiniPoints::p2()
   {
      return reinterpret_cast<MiniPoint*>(m_data)[1];
   }
   inline uint8& MiniPoints::speedLimit1()
   {
      return m_data[8];
   }
   inline uint8& MiniPoints::speedLimit2()
   {
      return m_data[9];
   }

}

#endif
