/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POINT_H
#define POINT_H
#include <arch.h>
#include "Penalties.h"
#include "RouteDatum.h"
#include "DistanceCalc.h"
#include "RouteInfo.h"
#include <map>
#include <set>


namespace isab{
   /** A form of iterator used to work with routes. */
   class Point { 
   public:
      class ClosestSegmentResult;
      friend class TestShell;

      /** Tells if this is a trackpoint rather than a waypoint.
       * Works on the real RouteDatum, not the calculated RoutePoint.
       * @return true if this is a RoutePoint but not a Waypoint.
       */
      bool isTrackPoint();

      /** Tells if this is an instance of RoutePoint and a real waypoint.
       * Works on the real RouteDatum, not the calculated RoutePoint.
       * @return true if this is a real waypoint.
       */
      bool isWaypoint();

      /** Tells if this is the end marker.
       * Works on the real RouteDatum, not the calculated RoutePoint.
       * @return true if this is the end marker
       */
      bool isEnd();

      /** Hack to find out if this is a good place to show 
       * "Start with u-turn"
       */
      bool isFirstSegAndStartWithUturn();

      /** Hack to move exit ramps to a more intiutive position
       * for the user.
       */
      bool isWptExitRamp();

      /** Gets the action of the current waypoint.
       * @return the value of the calculated RoutePoints action field.
       */
      uint16 getWptAction();

      /** Gets the flags for the segment.
       * @return the value of the calculated RoutePoints flags field.
       */
      uint8 getFlags();

      /** Gets the crossingType for the waypoint.
       * @return the value of the calculated RoutePoints crossingType field.
       */
      uint16 getWptCrossingType();

      /** Gets the exitCount for the waypoint.
       * @return the value of the calculated RoutePoints exitCount field.
       */
      uint16 getWptExitCount();

      /** Gets the speedLimit value.
       * @return the current speedlimit in km/h.
       */
      uint8 getSpeedLimit();

      /** Gets the crossing number of the active waypoint.
       * @return the current crossing number.
       */
      uint16 getCrossingNo();

      /** Returns an id of the current segment (i.e. trackpoint)
       * @return the id of the current trackpoint
       */
      uint16 getSegmentId();

      /** Gets the real RouteDatum's order number.
       * @return the order number of the currently underlying RouteDatum.
       */
      uint16 getPointId(); 

      /** Gets the text associated with a waypoint. 
       * @param table a pointer to a char array holding the 
       *        zero-terminated strings.
       * @return the text associated with this point. Note that if this method
       *         is called on anything other than Waypoints, the returned value
       *         may be NULL or any part of the strings in table.
       */
      const char *getText();

      /** Find the next waypoint.
       * @param next_point a pointer to the Point object where the next
       *                   waypoint will be written.
       * @return 0 if there are no more waypoints, non-zero otherwise.
       */
      int16 getNextWaypoint(Point* next_point);

      /** Find the next waypoint. This may also get waypoints that 
       * are in the truncated part of the route, i.e. ones that are not
       * in the 
       * @param next_point a pointer to the Point object where the next
       *                   waypoint will be written.
       * @return 0 if there are no more waypoints, non-zero otherwise.
       */
      int16 getNextWaypointEvenTruncated(Point* next_point);

      /** Find the next point, be it Waypoint, Trackpoint, MiniPoint, or
       * MicroPoint.
       * @param next_point a pointer to the Point object that will contain 
       *                   the next point. 
       * @return 0 if there are no more points, non-zero otherwise. 
       */      
      int16 getNextPoint(Point* next_point);

      /** Sets this Point to refer to the first RouteDatum on the route, 
       * skipping startpoint, scalepoint, and origopoint.
       * @param rp the start of the route.
       * @return 0 on failure and 1 on success.
       */
      int getFirstPoint(RouteDatum* rp, class Route *r);

      /** Calculates the distance from one point on the route to the end.
       * @param etgResult a pointer to the storage where the ETG estimate
       *                  will be stored or NULL if the ETG is not needed.
       * @return the calculated distance in meters, or -1 if the operation 
       *         failed. 
       */
      int32 distanceToEnd(int32 *etgResult=NULL);

      /** Calculates the distance from one point on the route to another.
       * @param endp the point to calculate the distance to. If NULL, the 
       *             distance to the end of the route is calculated.
       * @param etgResult a pointer to the storage where the ETG estimate
       *                  will be stored or NULL if the ETG is not needed.
       * @return the distance between this point and endp, in meters. -1 if 
       *         the operation failed.
       */
      int32 distanceToPoint(Point* endp, int32 *etgResult=NULL);

      /** Checks if this and another point are the same. The points are 
       * considered the same if they both refer to the same underlying 
       * RouteDatum and the same sub-point if the RouteDatum is a MinPoints or
       * a MicroDeltaPoints.
       * @param p the point to compare to.
       * @return true if the points are actually the same one.
       */
      bool isSamePoint(Point* p);

      /** Find the route segment closest to a specified coordinate. Look at 
       * all route segments from this point and forward a specified length.
       * @param gpsLat              the latitude of the coordinate, in radians.
       * @param gpsLon              the longitude of the coordinate, 
       *                            in radians.
       * @param maxlength           
       * @param params              a Penalties object holding parameters for
       *                            the calculation.
       * @param car_angle           the compass heading  the car is travelling.
       * @param result              an objet that is filled in with the details,
       * @return the distance to the closest segment, in meters, with 
       *         penalties applied.
       */
      int32 getClosestSegment(float gpsLat, float gpsLon,
                              uint8 car_angle,
                              int32 maxlength,
                              const Penalties* params,
                              Point &a_point,
                              Point &b_point,
                              ClosestSegmentResult &result);

      /** Return the angle of the segment in 256th of a full turn.
       * @param segmentAngle   Returned value of the angle 
       * @return the distance to the closest segment, in meters, with 
       *         penalties applied.
       */
      int32 getAngle(uint8 & segmentAngle);

      /** Return the RouteInfo style list of landmarks as a
       * vector. */
      void getLandmarks(RouteInfo::landmark_container &list);

      /** Gets the RouteInfoParts style lanes */
      void getLanes(RouteInfoParts::Lanes &lanes, int32 distance);

      /** Gets the RouteInfoParts style signpost */
      void getSignpost(RouteInfoParts::SignPost &signpost, int32 distance);

      /** Return the current coordiantes as wgs-84.
       * WARNING - do not use often as it is a floating point 
       * operation!!! */
      void getCoordWGS84(float &lat, float &lon);

      /** Return the current coordiantes as int32.*/
      void getCoord(int32 &lat, int32 &lon);

      /** The default constructor. Creates a Point with all fields zero.*/
      Point();

   protected:
      /** Values used when calling getNextPointReal and getPrevPointReal. */
      enum PointType { 
         TYPE_WAYPOINT = 1, /** Only stop at full waypoints. */
         /* type_trackpoint = 2,   
            This one was never used, nor handled - petersv */
         TYPE_ANY = 3, /** Stop at any way- , track-, mini-, or micropoint.*/
      };
      
      /** Finds the next point or Waypoint depending on the second argument.
       * @param next_point a pointer to a Point object were data for the next
       *                   point (or waypoint) will be written.
       * @param type       if set to TYPE_WAYPOINT, the next Waypoint will be 
       *                   found, if set to TYPE_ANY, the next point of any 
       *                   kind will be found.
       * @return 0 if there are no more points (or waypoints), non-zero 
       *         otherwise
       */
      int16 getNextPointReal(Point* next_point, PointType type, bool allowTruncated);

      /** Return the associated tdl datum, if one is associated with the given
       * route datum. 
       * @param rd the route datum for which an associated TDL is requested.
       * @return a pointer to the associated tdl datum or NULL if none is 
       *         available.*/
      MetaPointTimeDistLeft * getAssociatedTimeDistLeft();

   private:
      /** A RoutePoint representing the underlying RouteDatum. All fields in 
       * this object are calculated. */
      RoutePoint m_point;

      /** A pointer to the closest preceeding MetaPointOrigo. */
      MetaPointOrigo* m_origo;
      
      /** A pointer to the closest preceeding MetaPointScale. */
      MetaPointScale* m_scale;
      
      /** Pointers to currently active landmark RouteDatums */
      typedef std::map <uint16, MetaPointLandmark *> LandmarkMap;
      typedef std::map <uint16, int32> LandmarkStartDistMap;
      LandmarkMap m_landmarks;
      LandmarkStartDistMap m_newLandmarks;  // A list of landmars that start 
                                            // at this waypoint and their 
                                            // distance

      typedef std::vector<MetaPointLaneInfo*> LaneVector;
      LaneVector m_laneVector;

      typedef std::vector<MetaPointSignPost*> SignPostVector;
      SignPostVector m_signpostVector;

      /** A pointer to the waypoint for this Point (itself if this is a 
       * waypoint, otherwise the previous waypoint). */
      RoutePoint* m_waypoint;

      /** A pointer to the last full trackpoint for this Point (itself if this is a 
       * full trackpoint or waypoint, otherwise the previous full trackpoint or waypoint). */
      RoutePoint* m_trackpoint;

      /** Numeric id of which crossing is described by m_waypoint */
      int16 m_crossingNo;

      /** A pointer to the RouteDatum that is represented by this Point. */
      RouteDatum* m_currPoint;
      
      /** The index into MiniPoints and MicroDeltaPoints. 1-based. */
      int m_deltaIndex;

      /** Not used ? */
      int m_structType;

      /** A pointer to the owning route object. Used mainly to access the string 
       * table. */
      class Route * m_route;

      /** Gets the x (Lon) coordinate in this minimap.
       * @return the calculated x coordinate in this minimap.
       */
      int16 getX();

      /** Gets the y (Lat) coordinate in this minimap.
       * @return the calculated y coordinate in this minimap.
       */
      int16 getY();

      /** Gets the current X-scale.
       * @return the x-scale calculated from the closest preceeding
       *         MetaPointScale.
       */
      float getScaleX();

      /** Tells if this is an instance of MiniPoints.
       * Works on the real RouteDatum, not the calculated RoutePoint.
       * @return true if this is a MiniPoints.
       */
      bool isMiniPoint();

      /** Tells if this is an instance of MicroDeltaPoints.
       * Works on the real RouteDatum, not the calculated RoutePoint.
       * @return true if this is a MicroDeltaPoints.
       */
      bool isMicroPoint();

      /** Called after advancing past a Waypoint in getNextPointReal().
       * Updates the status of the m_landmarks map according to the 
       * MetaPointLandmark datums that may follow the waypoint.
       */
      void updateLandmarkStatus();

      /** Called after advancing past a Waypoint in getNextPointReal().
       * Updates the status of the m_lanesVector according to the 
       * MetaPointLanesInfo datums that may follow the waypoint.
       */
      void updateLanesStatus();

      /** Called after advancing past a Waypoint in getNextPointReal().
       * Updates the status of the m_signpostVector according to the 
       * MetaPointSignPost datums that may follow the waypoint.
       */
      void updateSignpostStatus();

   };

   class Point::ClosestSegmentResult {
      public:
         DistanceCalc::DistanceResult segData;
         Point seg;
   };

   //=================================================================
   //================== inlines for Point ============================

   inline uint16 Point::getWptAction()
   {
      return m_waypoint->action();
   }

   inline uint8 Point::getFlags()
   {
      return m_trackpoint->rpFlags();
   }

   inline uint16 Point::getWptExitCount()
   {
      return m_waypoint->exitCount();
   }

   inline uint16 Point::getWptCrossingType()
   {
      return m_waypoint->crossingType();
   }

   inline bool Point::isMiniPoint()
   {
      return m_currPoint->action() == RouteDatum::MINI_POINTS;
   }

   inline bool Point::isMicroPoint()
   {
      return m_currPoint->action() == RouteDatum::MICRO_DELTA_POINTS;
   }


   inline bool Point::isTrackPoint()
   {
      return (m_currPoint->action() & RouteDatum::ACTION_MASK) == 
         RouteDatum::TRACK_POINT;
   }   

   inline bool Point::isWaypoint() {
      return (! (m_currPoint->action() & RouteDatum::META_MASK)) &&
         ((m_currPoint->action() & RouteDatum::ACTION_MASK) 
          != RouteDatum::TRACK_POINT);
   }

   inline bool Point::isEnd() {
      return (m_currPoint->actionIsEnd(false));
   }

   inline Point::Point()
      :m_origo(NULL), m_scale(NULL), 
       m_waypoint(NULL), m_trackpoint(NULL), m_crossingNo(0),
       m_currPoint(NULL), 
       m_deltaIndex(0), m_structType(0), m_route(NULL)
   {
   }

   inline uint16 Point::getCrossingNo()
   {
      return m_crossingNo;
   }

   inline uint8 Point::getSpeedLimit()
   {
      return m_trackpoint->speedLimit();
   }

   inline int16 Point::getX()
   {  
      return m_point.x();
   }
   
   inline int16 Point::getY()
   {
      return m_point.y();
   }
   
   inline float Point::getScaleX()
   {
      float scaleX;
      scaleX = float(m_scale->scaleX1());
      scaleX += float(m_scale->scaleX2()) / 0x10000;
      return scaleX;
   }

   inline int16 Point::getNextWaypoint(Point* next_point)
   {
      return getNextPointReal(next_point, TYPE_WAYPOINT, false);
   }

   inline int16 Point::getNextWaypointEvenTruncated(Point* next_point)
   {
      return getNextPointReal(next_point, TYPE_WAYPOINT, true);
   }

   inline int16 Point::getNextPoint(Point* next_point)
   {
      return getNextPointReal(next_point, TYPE_ANY, false);
   }

   inline bool Point::isSamePoint(Point* p)
   {
      return (this->m_currPoint == p->m_currPoint) &&
         ( (this->m_deltaIndex == p->m_deltaIndex) ||
           (!this->isMicroPoint() && ! this->isMiniPoint()));
   }


}


#endif /* POINT_H */
