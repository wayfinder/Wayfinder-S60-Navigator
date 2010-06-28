/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Log.h"
#include "RouteDatum.h"
#include "Point.h"
#include <stdlib.h>
#include "Route.h"
#include "FastTrig.h"


#define SCALEY 6378137
#define MAX_ORIGO_DIST 32000

namespace isab{
   uint16 Point::getPointId()
   {  
      return m_currPoint->getPointId(m_route->getFirstDatum());
   }

   uint16 Point::getSegmentId()
   {  
      return m_trackpoint->getPointId(m_route->getFirstDatum());
   }

   const char* Point::getText()
   {
      return m_route->lookupString(m_trackpoint->streetNameIndex());
   }

   void Point::getLandmarks(RouteInfo::landmark_container &list)
   {
      list.clear();
      LandmarkMap::iterator i = m_landmarks.begin();
      while (i != m_landmarks.end()) {
         // It is a start if it is in m_newLandmarks
         LandmarkStartDistMap::iterator startLmark = m_newLandmarks.find((*i).first);
         bool start = (startLmark != m_newLandmarks.end());
         bool stop = (*i).second->isStop();

         // Assumes a 1:1 mapping here. Maybe dangeous?
         enum RouteInfoParts::Landmark::LandmarkSide side = (enum RouteInfoParts::Landmark::LandmarkSide)( (*i).second->roadSide() );
         enum RouteInfoParts::Landmark::LandmarkType type = (enum RouteInfoParts::Landmark::LandmarkType)( (*i).second->landmarkType() );

         RouteInfoParts::Landmark *l = new RouteInfoParts::Landmark(
               m_route->lookupString( (*i).second->streetNameIndex()),
               start,
               stop,
               (*i).second->isDisturbedRoute(),
               side,
               type,
               start ? ( (*startLmark).second ) : -1,
               stop  ? ( (*i).second->distance() ) : -1
               );
         list.push_back(l);
         ++i;
      }
   }

   void Point::getLanes(RouteInfoParts::Lanes &lanes, int32 distance)
   {
      //Search for the active lane info. 
      int i = m_laneVector.size();
      MetaPointLaneInfo* laneInfo = NULL;
      //If there are no lanes i is zero.
      // otherwise we start with the lane info closest to the way point.
      while( i > 0 ){
         --i;
         if( distance <= m_laneVector[i]->distance() ){
            //Current distance is smaler than the lane infos distance to the way point.
            //Preavious lane infos are further away.
            laneInfo = m_laneVector[i];
            i = 0;
         }
      }
      if( laneInfo ){
         lanes.distance = laneInfo->distance();
         lanes.noLanes = laneInfo->stopOfLanes();
         //Add the lane data for the lanes in lane info (maximum 4).
         uint32 j;
         //The number lanes in lane info is either four or the total number of lanes.
         uint8 infoLanes = std::min(laneInfo->nbrLanes(),uint8(4));
         uint8 lane;
         for( j=0; j < infoLanes; ++j){
            lane = laneInfo->getlane(j).m_lane;
            RouteInfoParts::Lane* ripLane = new RouteInfoParts::Lane( lane );
            lanes.m_laneVector.push_back(ripLane);
         }
         //Add following lane data
         for( j=4; j < laneInfo->nbrLanes(); ++j){
            int laneidx = (j-4)/10;
            MetaPointLaneData* lanedata = (MetaPointLaneData*)laneInfo+1+laneidx;
            lane = lanedata->getlane(j-4-laneidx*10).m_lane;
            RouteInfoParts::Lane* ripLane = new RouteInfoParts::Lane( lane );
            lanes.m_laneVector.push_back(ripLane);
         }
      }
      else{
         lanes.distance = 0;
         lanes.noLanes = true;
         lanes.m_laneVector.clear();
      }
   }

   void Point::getSignpost(RouteInfoParts::SignPost &signpost, int32 distance)
   {
      //Search for the signpost.
      MetaPointSignPost* mpSignpost = NULL;
      uint16 i=0;
      while( i < m_signpostVector.size() ){
         if( distance >= m_signpostVector[i]->distance() ){
            //The signpost hasn't been passed yet.
            mpSignpost = m_signpostVector[i];
            i = m_signpostVector.size();
         }
         else{
            i++;
         }
      }
      if( mpSignpost ){
         Route::RGBColor* txtColor = m_route->lookupColor( mpSignpost->textColor() );
         Route::RGBColor* bgColor = m_route->lookupColor( mpSignpost->backgroundColor() );
         Route::RGBColor* fgColor = m_route->lookupColor( mpSignpost->frontColor() );

         signpost = RouteInfoParts::SignPost(
               m_route->lookupString( mpSignpost->textIndex() ),
               mpSignpost->distance(),
               txtColor->red,
               txtColor->green,
               txtColor->blue,
               bgColor->red,
               bgColor->green,
               bgColor->blue,
               fgColor->red,
               fgColor->green,
               fgColor->blue
               );
      }
      else{
         signpost = RouteInfoParts::SignPost("",0,0xff,0xff,0xff,0,0,0xff,0xff,0xff,0xff); 
      }
   }

   void Point::getCoordWGS84(float &lat, float &lon)
   {
      int32 origoY = m_origo->origoY(); /* Cached value */
      int32 origoX = m_origo->origoX(); /* Cached value */
      float scaleX = getScaleX();       /* Cached value */

      lat = float(getY() + origoY) / float(SCALEY);
      lon = float(getX() + origoX) / scaleX;
   }

   void Point::getCoord(int32 &lat, int32 &lon)
   {
      int32 origoY = m_origo->origoY(); /* Cached value */
      int32 origoX = m_origo->origoX(); /* Cached value */
      float scaleX = getScaleX();       /* Cached value */
      
      lat = int32(((getY() + origoY) * 100000000.0) / SCALEY);
      lon = int32(((getX() + origoX) * 100000000.0) / scaleX);
   }

   void Point::updateLandmarkStatus()
   {
      // Empty the list of landmarks that started at
      // the last waypoint
      m_newLandmarks.clear();

      // Remove landmarks that have ended
      LandmarkMap::iterator i = m_landmarks.begin();
      LandmarkMap::iterator ni;
      while ( i != m_landmarks.end() ) {
         ni = i;
         ++ni;
         if ( (*i).second->isStop() ) {
            m_landmarks.erase(i);
         }
         i=ni;
      }

      RouteDatum *rp = m_currPoint + 1;

      while (rp->actionIsTimeDistLeft() ) { 
         ++rp;
      }
      while (rp->actionIsLandmark() ) {
         MetaPointLandmark &l = rp->asMetaPointLandmark();

         LandmarkMap::iterator oldLm;
         oldLm = m_landmarks.find( l.id() ) ;
         if (oldLm != m_landmarks.end()) {
            // Update an existing landmark
            (*oldLm).second = &l;
         } else {
            // New landmark id
            m_landmarks.insert(LandmarkMap::value_type (l.id(), &l) );
            if (l.isStart()) {
               // May just be a reminder landmark
               m_newLandmarks.insert(LandmarkStartDistMap::value_type( l.id(), l.distance() ) );
            }
         }

         ++rp;
      }
   }

   void Point::updateLanesStatus()
   {
      m_laneVector.clear();

      RouteDatum *rp = m_currPoint + 1;

      while (rp->actionIsTimeDistLeft() ) { 
         ++rp;
      }
      while (rp->actionIsLandmark() ) {
         ++rp;
      }
      while ( rp->actionIsLaneInfo() ) {
         MetaPointLaneInfo &li = rp->asMetaPointLaneInfo();
         m_laneVector.push_back(&li);
         ++rp;
         while ( rp->actionIsLaneData() ){
            ++rp;
         }
      }
   }

   void Point::updateSignpostStatus()
   {
      m_signpostVector.clear();

      RouteDatum *rp = m_currPoint + 1;

      while (rp->actionIsTimeDistLeft() ) { 
         ++rp;
      }
      while (rp->actionIsLandmark() ) {
         ++rp;
      }
      while ( rp->actionIsLaneInfo() ) {
         ++rp;
         while ( rp->actionIsLaneData() ){
            ++rp;
         }
      }
      while ( rp->actionIsSignPost() ) {
         MetaPointSignPost &sp = rp->asMetaPointSignPost();
         m_signpostVector.push_back(&sp);
         ++rp;
      }
   }

   int16  Point::getNextPointReal(Point* next_point, PointType type, bool allowTruncated)
   {
      RouteDatum* rp;
      int changed_origo_scale=0; /* Flag set if we pass a minimap border */
      
      /* Initialize to the current point... */
      rp = this->m_currPoint;
      if (rp->actionIsEnd(allowTruncated)) {
         /* Oups, they sent in the endpoint. Restart. */
         return 0;
      }
      /* Oki, initialize the origo and scale members. */
      next_point->m_origo = m_origo;
      next_point->m_scale = m_scale;

      /* Copy the parent route */
      next_point->m_route = m_route;

      /* Copy the cached wpt/tpt pointers and crossing id */
      next_point->m_waypoint = m_waypoint;
      next_point->m_trackpoint = m_trackpoint;
      next_point->m_crossingNo = m_crossingNo;

      /* Copy the cached landmark pointers */
      next_point->m_landmarks = m_landmarks;
      next_point->m_newLandmarks = m_newLandmarks;

      /* Check special case that this is a delta point. */
      if (type != TYPE_WAYPOINT &&
          this->m_currPoint->actionIsMiniPoint() &&
          this->m_currPoint->actionIsMiniPoint() &&
          this->m_deltaIndex == 1) {
         /* This is the first of two delta points. */
         /* We need to return the second one. */
         next_point->m_currPoint = rp;
         next_point->m_point.action() = RouteDatum::DELTA; 
         next_point->m_point.x() = rp->asMiniPoints().p2().x;
         next_point->m_point.y() = rp->asMiniPoints().p2().y;
         next_point->m_point.speedLimit() = rp->asMiniPoints().speedLimit2();
         /* next_point->m_point.flags() is already correct - leave it */
         next_point->m_point.meters() = MAX_UINT16;
         next_point->m_point.streetNameIndex() = 0;
         next_point->m_deltaIndex = 2;
         return 1;
      }
      if(type != TYPE_WAYPOINT &&
         this->m_currPoint->actionIsMicroDeltaPoint() &&
         this->m_deltaIndex < 4){ // XXX zero based index?
         /* not the last of 5 delta points (zero based index). */
         /* We need to return the next one. */
         MicroDeltaPoint& nextMicro = (*(rp->asMicroDeltaPointsP()))[m_deltaIndex + 1];
         if ( nextMicro.x == 0 && nextMicro.y == 0 ) {
            // if x,y = 0,0 then end of MicroPoint ignore and move on
            this->m_deltaIndex = 4;
         } else {
            next_point->m_currPoint = rp; // already the case.
            next_point->m_point.action() = RouteDatum::DELTA; // XXX: This ok?
            next_point->m_point.x() = m_point.x() + nextMicro.x;
            next_point->m_point.y() = m_point.y() + nextMicro.y;
            next_point->m_point.speedLimit() = 0; /* FIXME */
            /* Flags are alredy correct ? */
            next_point->m_point.meters() = MAX_UINT16;
            next_point->m_point.streetNameIndex() = 0;
            next_point->m_deltaIndex = m_deltaIndex + 1;
            return 1;
         }
      }
      /* ... and take one step forward. */
      rp++;

      /* Loop forwards until the next waypoint. */
      while (! rp->actionIsEnd(allowTruncated)) {
         // Skip the endmarker if it is the truncation marker and we are allowed
         // past it.
         if (allowTruncated && rp->actionIsEnd(false)) {
            ++rp;
            continue;
         }
         // Update the pointers to the currently active trackpoint/waypoint if one
         // is passed.
         if(rp->actionIsTrackpoint()) {
            next_point->m_trackpoint = rp->asRoutePointP();
         }
         if(rp->actionIsWaypoint()) {
            next_point->m_trackpoint = next_point->m_waypoint = rp->asRoutePointP();
            next_point->m_crossingNo += 1;
         }
         if (type == TYPE_WAYPOINT) {
            if(rp->actionIsWaypoint()) {
               /* This is the one we're looking for. */
               next_point->m_currPoint = rp;
               rp->copyWaypoint(&(next_point->m_point));
               next_point->updateLandmarkStatus();
               next_point->updateLanesStatus();
               next_point->updateSignpostStatus();
               return changed_origo_scale ? 2 : 1;
            }
         } else {
            if (rp->actionIsWaypoint() || rp->actionIsFullDeltaPoint()) {
               next_point->m_currPoint = rp;
               rp->copyWaypoint(&(next_point->m_point));
               if(rp->actionIsWaypoint()){
                  next_point->updateLandmarkStatus();
                  next_point->updateLanesStatus();
                  next_point->updateSignpostStatus();
               }
               return changed_origo_scale ? 2 : 1;
            } else if(rp->actionIsMiniPoint()) {
               next_point->m_currPoint = rp;
               next_point->m_point.action() = RouteDatum::DELTA;
               next_point->m_point.x() = rp->asMiniPoints().p1().x;
               next_point->m_point.y() = rp->asMiniPoints().p1().y;
               /* next_point->point.u.rp.flags is already correct - leave it */
               /* how come the flags are correct? are they the same as before*/
               next_point->m_point.speedLimit() = 
                  rp->asMiniPoints().speedLimit1();
               /* This is not a real waypoint, so meters is set to the 
                  error value. */
               next_point->m_point.meters() = MAX_UINT16;
               next_point->m_point.streetNameIndex() = 0;
               next_point->m_deltaIndex = 1;
               return changed_origo_scale ? 2 : 1;
            } else if ( rp->actionIsMicroDeltaPoint() ) {
               next_point->m_currPoint = rp;
               next_point->m_point.action() = RouteDatum::DELTA;
               MicroDeltaPoint& nextMicro = 
                  (*(rp->asMicroDeltaPointsP()))[ 0 ];
               next_point->m_point.x() = m_point.x() + nextMicro.x;
               next_point->m_point.y() = m_point.y() + nextMicro.y;
               /* next_point->point.u.rp.flags is already correct - leave it */
               /* how come the flags are correct? are they the same as before*/
               // Speed unchanged
               next_point->m_point.speedLimit() = m_point.speedLimit();
               /* This is not a real waypoint, so meters is set to the 
                  error value. */
               next_point->m_point.meters() = MAX_UINT16;
               // Don't think this is used, otherwise unchanged use m_point
               next_point->m_point.streetNameIndex() = 0;
               next_point->m_deltaIndex = 0;
               return changed_origo_scale ? 2 : 1;
            }
         }
         if (rp->actionIsOrigo()) {
            next_point->m_origo = rp->asMetaPointOrigoP();
            changed_origo_scale = 1;
         }
         if (rp->actionIsScale()) {
            next_point->m_scale = rp->asMetaPointScaleP();
            changed_origo_scale = 1;
         }
         /* Take one step ahead. */
         rp++;
      }
      if(rp->actionIsEnd(allowTruncated)) {
         /* Last waypoint beyond the one passed to us. */
         next_point->m_currPoint = rp;
         rp->copyWaypoint(&next_point->m_point);
         return changed_origo_scale ? 2 : 1;
      }
      /* Hmm... Should never get here... */
//       err("Fok! Niemals hier gewerden!\n");
      return 0;
   }

   int Point::getFirstPoint(RouteDatum* rp, Route *r)
   {
      int8 found_origo=0;
      int8 found_scale=0;
      
      if (!rp->actionIsStart()) {
//          dbg("RouteDatum::getFirstPoint: not at start of route\n");
         return 0;
      }
      
      m_landmarks.clear();
      m_newLandmarks.clear();
      m_route = r;
      while (1) {
         ++rp;
         if(rp->actionIsOrigo()) {
            found_origo = 1;
            this->m_origo = rp->asMetaPointOrigoP();
            continue;
         }
         if(rp->actionIsScale()) {
            found_scale = 1;
            this->m_scale = rp->asMetaPointScaleP();
            continue;
         }
         if(rp->actionIsEnd(false)) {
//             dbg("RouteDatum::getFirstPoint : no points in the route\n");
            return 0;
         }
         if(rp->actionIsWaypoint()) {
            if (!found_scale || !found_origo) {
//                dbg("RouteDatum::getFirstPoint : wpt before origo or scale\n");
               return 0;
            }
            this->m_point = rp->asRoutePoint();
            this->m_waypoint = this->m_trackpoint = rp->asRoutePointP();
            this->m_crossingNo = 1;
            this->m_currPoint = rp;
            this->m_deltaIndex = 0;
            this->updateLandmarkStatus();
            this->updateLanesStatus();
            this->updateSignpostStatus();
            return 1;
         }
         if(rp->actionIsDeltaPoint() || rp->actionIsMiniPoint()) {
//             dbg("RouteDatum::getFirstPoint : "
//                 "Trackpoint before first waypoint");
            return 0;
         }
      }
   }
   

   bool Point::isFirstSegAndStartWithUturn()
   {
      RouteDatum *rd = m_currPoint;

      if ( !rd->actionIsWaypoint() ) {
         return false;
      }

      /* Find the previous waypoint or startpoint */
      while ( 1 ) {
         --rd;
         if (rd->actionIsStart()) {
            // Start found, is it a start-with-uturn?
            if (rd->asRoutePoint().rpFlags() & 0x01) {
               return true;
            }
            return false;
         }
         if (rd->actionIsWaypoint()) {
            return false;
         }
      }
   }


   bool Point::isWptExitRamp()
   {
      RouteDatum *rd = m_waypoint;
      if ( ( rd->actionIsWaypoint()               ) && 
           rd->actionIs(RouteDatum::EXITAT)) {
         return true;
      }
      return false;
   }


   MetaPointTimeDistLeft * Point::getAssociatedTimeDistLeft()
   {
      RouteDatum *rd = m_currPoint;

      if ( !rd->actionIsWaypoint() &&
           !rd->actionIsTrackpoint() )
      {
         return NULL;
      }
      if ( rd->actionIsEnd(true) ) {
         return NULL;
      }
      while (1) {
         ++rd;
         if ( rd->actionIsEnd(true) ||
              rd->actionIsWaypoint() ||
              rd->actionIsTrackpoint() ||
              rd->actionIsMiniPoint() ||
              rd->actionIsMicroDeltaPoint() ||
              rd->actionIsOrigo() ||
              rd->actionIsScale()
            ) 
         {
            return NULL;
         }
         if (rd->actionIsTimeDistLeft()) {
            return rd->asMetaPointTimeDistLeftP();
         }
      }
   }


   int32 Point::distanceToEnd(int32 *etgResult)
   {
      Point last_point, next_point;
      uint32 distsum=0;
      uint32 tmpdist;
      int32 etg=0;
      
      last_point = *this;
      while (1){
         MetaPointTimeDistLeft * tdl;
         tdl = last_point.getAssociatedTimeDistLeft();
         // No need to sum beyond the tdl packet
         if (tdl) {
            etg += tdl->timeLeft();
            distsum += tdl->distLeft();
            if (etgResult) {
               *etgResult = etg;
            }
            return distsum;
         }
         switch (last_point.getNextPoint(&next_point)) {
         case 0:
            /* No waypoint (end of route probably) */
            if (etgResult) {
               *etgResult = etg;
            }
            return distsum;
         case 1:
            /* No minimap passed */
            tmpdist = DistanceCalc::distance(last_point.getX(), 
                                              last_point.getY(),
                                              next_point.getX(), 
                                              next_point.getY() );
            distsum += tmpdist;
            etg += int32(tmpdist * 3.6 / last_point.getSpeedLimit());
            break;
         case 2:
            /* next_point is in a new minimap */
            tmpdist = DistanceCalc::distance(next_point.m_scale->refX(),
                                              next_point.m_scale->refY(),
                                              next_point.getX(),
                                              next_point.getY() );
            distsum += tmpdist;
            etg += int32(tmpdist * 3.6 / last_point.getSpeedLimit());
            break;
         default:
//             ERR("Point::distanceToEnd : unknown "
//                 "return value from Point::getNextPoint");
            if (etgResult) {
               *etgResult=0;
            }
            return -1;
         }
         last_point = next_point;
      }
   }

   int32 Point::distanceToPoint(Point* endp, int32 *etgResult)
   {
      int32 distLeft1, timeLeft1;
      int32 distLeft2, timeLeft2;

      distLeft1 = distanceToEnd(&timeLeft1);
      if (endp) {
         distLeft2 = endp->distanceToEnd(&timeLeft2);
         distLeft1 -= distLeft2;
         timeLeft1 -= timeLeft2;
      }
      if (etgResult) {
         *etgResult = timeLeft1;
      }
      return distLeft1;
   }

   int32 Point::getClosestSegment(float gpsLat, 
                                  float gpsLon,
                                  uint8 car_angle,
                                  int32 maxlength,
                                  const Penalties* params,
                                  Point &a_point,
                                  Point &b_point,
                                  ClosestSegmentResult &result)
   {
      Point first_point, second_point;
      uint16  current_dist, best_dist=0;
      int8    none_yet=1;      /* Flag - cleared after the first segment */
      int8    valid_coord_in_this_minimap;
      int32   tmppx, tmppy;
      int16   px=0, py=0;      /* Silence the compiler */
      float   scaleX;           /* Cached value */
      int32   origoX,origoY;    /* Cached value */
      DistanceCalc::DistanceResult tmpSegmentData = {0};

      if (maxlength<0) {
//          err("Point::getClosestSegment - negative "
//              "maxlength - dazed and confused");
         return -1;
      }
      first_point = *this;
      if (! first_point.getNextPoint(&second_point)){
         /* Not even one segment available */
         a_point = first_point;
         b_point = first_point;
         return -1;
      }
      scaleX = first_point.getScaleX();//nav_route_get_scale_x(&first_point);
      origoX = first_point.m_origo->origoX();//first_point.origo->u.ro.origo_x;
      origoY = first_point.m_origo->origoY();//first_point.origo->u.ro.origo_y;
      tmppx = (int32)(gpsLon * scaleX) - origoX;
      tmppy = (int32)(gpsLat * SCALEY) - origoY;
      //dbg("sc: %f oX: %d oY: %d tmppx: %d tmppy: %d", scaleX, origoX, 
      //              origoY, tmppx, tmppy);
      if (labs(tmppx) < MAX_ORIGO_DIST && 
          labs(tmppy) < MAX_ORIGO_DIST) {
         px = int16(tmppx);
         py = int16(tmppy);
         valid_coord_in_this_minimap=1;
      } else {
         valid_coord_in_this_minimap=0;
      }
      //dbg("*****px=%d", px);
      //dbg("*****py=%d", py);

      /* Loop over the segments following startp */
      while (1){
         switch (first_point.getNextPoint(&second_point)){
         case 0:                    /* End of the route */
            a_point = first_point;
            b_point = first_point;
            if (none_yet) {
               return -2;
            }
            return best_dist;
         case 2:                    /* Passed into a new minimap */
            scaleX = second_point.getScaleX();
            origoX = second_point.m_origo->origoX();
            origoY = second_point.m_origo->origoY();
            tmppx=(int32)(gpsLon * scaleX) - origoX;
            tmppy=(int32)(gpsLat * SCALEY) - origoY;
            if (labs(tmppx) < MAX_ORIGO_DIST && 
                labs(tmppy) < MAX_ORIGO_DIST) {
               px=int16(tmppx);
               py=int16(tmppy);
               valid_coord_in_this_minimap=1;
               current_dist = 
                  DistanceCalc::
                  distancePointToSegment(
                                         params,
                                         car_angle,
                                         second_point.m_scale->refX(),
                                         second_point.m_scale->refY(),
                                         int16(second_point.m_point.x()),
                                         int16(second_point.m_point.y()),
                                         px, 
                                         py,
                                         tmpSegmentData);
               //dbg("*****current_dist_new_minimap=%u",current_dist);
               if (none_yet || current_dist < best_dist) {
                  /* Return the original trackpoint, not the 
                   * one used above (the one 
                   * taken from the scale */
                  a_point = first_point;
                  b_point = second_point;
                  best_dist = current_dist;
                  result.seg = first_point;
                  result.segData = tmpSegmentData;
               }
            } else {
               valid_coord_in_this_minimap=0;
            }
            break;
         case 1:
            if (valid_coord_in_this_minimap) {
               current_dist = 
                  DistanceCalc::
                  distancePointToSegment(
                                         params,
                                         car_angle,
                                         first_point.m_point.x(),  
                                         first_point.m_point.y(),
                                         second_point.m_point.x(), 
                                         second_point.m_point.y(),
                                         px,            
                                         py,
                                         tmpSegmentData);
               //dbg("*****current_dist = %u",current_dist);
               if (none_yet || current_dist < best_dist) {
                  a_point = first_point;
                  b_point = second_point;
                  best_dist = current_dist;
                  result.seg = first_point;
                  result.segData = tmpSegmentData;
               }
            }
            break;
         default:
//             err("Point::getClosestSegment : unknown "
//                 "return value from Point::getNextPoint.");
            return -1;
         }
         
         if (valid_coord_in_this_minimap) {
            /* Are we to look at a limited number of segments? We don't count
             * the first segment to guarantee that we always look at the next 
             * segment as well (unless this is the last segment in which case 
             * the switch statement above terminates the search). */
            if (!none_yet && maxlength && (maxlength -= tmpSegmentData.segmentLength) <= 0){
               return best_dist;
            }
            none_yet=0;
         }
         //cerr << "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG first=" << 
         //        first_point.m_crossingNo << " best=" << result.seg.m_crossingNo << 
         //        " name=" << first_point.getText() << endl;
         first_point = second_point;
      }
      /* Not reached */
   }

   int32 Point::getAngle(uint8 & segmentAngle)
   {
      Point first_point, second_point;
      int dx, dy;

      first_point = *this;
      if (! first_point.getNextPoint(&second_point)){
         /* Not even one segment available */
         return -1;
      }

      /* Loop over the segments following startp */
         switch (first_point.getNextPoint(&second_point)){
      case 0:                    /* End of the route */
         return -1;
      case 2:                    /* Passed into a new minimap */
         dx = int16(second_point.m_point.x()) - second_point.m_scale->refX();
         dy = int16(second_point.m_point.y()) - second_point.m_scale->refY();
         if ( (dx != 0) || (dy != 0) ) {
            segmentAngle = FastTrig::angleTheta2int(dx, dy);
         } else {
            segmentAngle = 0;   /* Degenerate segment */
         }
         return 0;
         break;
      case 1:
         dx = int16(second_point.m_point.x()) - first_point.m_point.x();
         dy = int16(second_point.m_point.y()) - first_point.m_point.y();
         if ( (dx != 0) || (dy != 0) ) {
            segmentAngle = FastTrig::angleTheta2int(dx, dy);
         } else {
            segmentAngle = 0;   /* Degenerate segment */
         }  
         return 0;
         break;
      default:
//          err("Point::getClosestSegment : unknown "
//              "return value from Point::getNextPoint.");
         return -1;
      }
      
  
      /* Not reached */
   }

}  /* namespace isab */
