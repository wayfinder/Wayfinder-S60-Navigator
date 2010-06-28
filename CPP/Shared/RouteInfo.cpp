/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "RouteInfo.h"
#include "Buffer.h"
#include "nav2util.h"
#include "Log.h"

namespace isab{

/******************************************************/
/************  RouteInfoParts::Segment ****************/
/******************************************************/

   void RouteInfoParts::Segment::writeToBuf(Buffer *buf) const
   {
      buf->writeNextCharString(streetName);
      buf->writeNextUnaligned16bit(speedLimit);
      buf->writeNext8bit(flags);
      buf->writeNext8bit( (valid?0x01:0) | (changed?0x02:0) );
   }

   void RouteInfoParts::Segment::readFromBuf(Buffer *buf)
   {
      if (ownStreetName) {
         delete[] (char *)(streetName);
      }
      streetName = buf->getNextCharStringAlloc();
      ownStreetName = true;
      speedLimit = buf->readNextUnaligned16bit();
      flags = buf->readNext8bit();
      uint8 tmp = buf->readNext8bit();
      valid = tmp & 0x01;
      changed = (tmp & 0x02) != 0;
   }

   RouteInfoParts::Segment::~Segment()
   {
      // Sort of hackish. If the name is created from readFromBuf()
      // then we may delete the string pointer. Hack!
      if (ownStreetName) {
         delete[] (char *)(streetName);
      }
   }
   RouteInfoParts::Segment::Segment() : 
      streetName(NULL), ownStreetName(false)
   { }

   RouteInfoParts::Segment::Segment(const Segment &old) :
      streetName(strdup_new(old.streetName)),
      speedLimit(old.speedLimit),
      flags(old.flags),
      valid(old.valid),
      changed(old.changed),
      ownStreetName(true)
   {
   }

   RouteInfoParts::Segment::Segment(Buffer *buf) : 
      streetName(NULL), ownStreetName(false)
   {
      ownStreetName = false;
      readFromBuf(buf);
   }

   RouteInfoParts::Segment &
   RouteInfoParts::Segment::operator= (const RouteInfoParts::Segment & old)
   {
      if (ownStreetName) {
         delete[] const_cast<char *>(streetName);
      }
      streetName = NULL;
      streetName = strdup_new(old.streetName);
      speedLimit = old.speedLimit;
      flags = old.flags;
      valid = old.valid;
      changed = old.changed;
      ownStreetName = true;
      return *this;
   }

   int RouteInfoParts::Segment::log(Log* log, const char* name) const
   {
#ifndef NO_LOG_OUTPUT
      log->debug("%s : streetName : %s", name, streetName);
      log->debugDump("streetName", (uint8*)streetName, strlen(streetName) + 1);
      log->debug("%s : speedLimit    : %"PRId16, name, speedLimit);
      log->debug("%s : flags         : %"PRIu8,  name, flags);
      log->debug("%s : valid         : %s", name, valid ? "yes" : "no");
      log->debug("%s : changed       : %s", name, changed ? "yes" : "no");
      log->debug("%s : ownStreetName : %s", name, ownStreetName ? "yes" :"no");
#endif
      return 0;
   }

/******************************************************/
/************  RouteInfoParts::Landmark ***************/
/******************************************************/

   RouteInfoParts::Landmark::Landmark(const RouteInfoParts::Landmark& other )
   {
      // Do not delete infostring
      ownInfo = false;
      info = NULL;
      *this = other;
   }
   
   RouteInfoParts::Landmark&
   RouteInfoParts::Landmark::operator=( const RouteInfoParts::Landmark& other)
   {
      if ( this != &other ) {
         Buffer buf(256);
         other.writeToBuf( &buf );
         this->readFromBuf( &buf );
      }
      return *this;
   }
   
   void RouteInfoParts::Landmark::writeToBuf(Buffer *buf) const
   {
      buf->writeNextCharString(info);
      buf->writeNextUnaligned16bit(type);
      buf->writeNext8bit(side);
      buf->writeNext8bit( (start?0x01:0) | 
                          (stop?0x02:0)  | 
                          (disturbedRoute?0x04:0) 
                        );
      buf->writeNextUnaligned32bit(startDistance);
      buf->writeNextUnaligned32bit(stopDistance);
   }

   void RouteInfoParts::Landmark::readFromBuf(Buffer *buf)
   {
      if (ownInfo) {
         delete[] (char *)(info);
      }
      info = buf->getNextCharStringAlloc();
      ownInfo = true;
      type = LandmarkType(buf->readNextUnaligned16bit());
      side = LandmarkSide(buf->readNext8bit());
      uint8 tmp = buf->readNext8bit();
      start = tmp & 0x01;
      stop = (tmp & 0x02) != 0;
      disturbedRoute = (tmp & 0x04) != 0;
      startDistance = buf->readNextUnaligned32bit();
      stopDistance = buf->readNextUnaligned32bit();
   }

   int RouteInfoParts::Landmark::log(class Log* log, int order) const
   {
#ifndef NO_LOG_OUTPUT
      log->debug("%d : info : %s", order, info);
      log->debugDump("info : ", (uint8*)info, strlen(info) + 1);
      log->debug("%d : side           : %u", order, side);
      log->debug("%d : type           : %u", order, type);
      log->debug("%d : disturbedRoute : %s", order, 
                 disturbedRoute ? "yes" : "no");
      log->debug("%d : start          : %s", order, start ? "yes" : "no");
      log->debug("%d : stop           : %s", order, stop ? "yes" : "no");
      log->debug("%d : startDistance  : %"PRId32, order, startDistance);
      log->debug("%d : stopDistance   : %"PRId32, order, stopDistance);
      log->debug("%d : ownInfo        : %s", order, ownInfo ? "yes" : "no");
#endif
      return 0;
   }

   RouteInfoParts::Landmark::Landmark(const char *ainfo,
                                      bool astart, bool astop, bool adetour,
                                      enum LandmarkSide aside,
                                      enum LandmarkType atype,
                                      int32 astartDistance,
                                      int32 astopDistance)
      : info(ainfo), side(aside), type(atype), disturbedRoute(adetour),
        start(astart), stop(astop), startDistance(astartDistance), 
        stopDistance(astopDistance), ownInfo(false)
   {
   }

   RouteInfoParts::Landmark::Landmark(Buffer *buf)
   {
      ownInfo = false;
      readFromBuf(buf);
   }

   RouteInfoParts::Landmark::~Landmark()
   {
      if (ownInfo) {
         delete[] (char *)(info);
      }
   }

   bool RouteInfoParts::Landmark::matchesStartStop(enum Landmark::StartStopQuery q,
                                                   int32 currdist) const
   {
      switch (q) {
         case ActiveNow:
            if (start && (startDistance>=0) && (currdist > startDistance) ) {
               return false;
            }
            if (stop && (stopDistance>=0) && (currdist < stopDistance) ) {
               return false;
            }
            return true;    // Neither before the start nor after the end.
         case ActiveOnWpt:
            return true;
         case Begins:
            return (start);
         case Ends:
            return (stop);
      }
      return false;
   }

   bool RouteInfoParts::Landmark::isDetour(enum Landmark::StartStopQuery q, int32 currDist) const
   {
      if (! matchesStartStop(q, currDist)) {
         return false;
      }
      return disturbedRoute;
   }

   bool RouteInfoParts::Landmark::isSpeedCamera(enum Landmark::StartStopQuery q, int32 currDist) const
   {
      if (! matchesStartStop(q, currDist)) {
         return false;
      }
      return (type == cameraLM);
   }

   const char * RouteInfoParts::Landmark::getDescription() const
   {
      return info;
   }

/******************************************************/
/************  RouteInfoParts::Lane *******************/
/******************************************************/

   RouteInfoParts::Lane::Lane( uint8 lane )
   {
      m_lane = lane;
   }

   RouteInfoParts::Lane::Lane( Buffer *buf )
   {
      readFromBuf(buf);
   }

   RouteInfoParts::Lane::Lane( const Lane& other )
   {
      m_lane = other.m_lane;
   }

   bool RouteInfoParts::Lane::isPreferred()
   {
      return (m_lane >> 7 & 0x1) != 0;
   }

   bool RouteInfoParts::Lane::isAllowed()
   {
      return (m_lane >> 6 & 0x1) == 0;
   }

   RouteInfoParts::Lane::Direction RouteInfoParts::Lane::direction()
   {
      return (Direction)(m_lane & 0x1F);
   }
   
   void RouteInfoParts::Lane::writeToBuf(Buffer *buf) const
   {
      buf->writeNext8bit(m_lane);
   }

   void RouteInfoParts::Lane::readFromBuf(Buffer *buf)
   {
      m_lane = buf->readNext8bit();
   }

   int RouteInfoParts::Lane::log(class Log* log, int order) const
   {
#ifndef NO_LOG_OUTPUT
#endif
      return 0;
   }

/******************************************************/
/************  RouteInfoParts::SignPost ***************/
/******************************************************/

   RouteInfoParts::SignPost::SignPost()
      : m_text(""), m_distance(0), ownInfo(false)
   {
      m_textColor.red = 0xff;
      m_textColor.green = 0xff;
      m_textColor.blue = 0xff;

      m_backColor.red = 0;
      m_backColor.green = 0;
      m_backColor.blue = 0xff;

      m_frontColor.red = 0;
      m_frontColor.green = 0xff;
      m_frontColor.blue = 0xff;
   }

   RouteInfoParts::SignPost::SignPost( const char *atext, int32 adist, 
                                       uint8 atextRed, uint8 atextGreen, uint8 atextBlue,
                                       uint8 abgRed, uint8 abgGreen, uint8 abgBlue,
                                       uint8 afgRed, uint8 afgGreen, uint8 afgBlue)
      : m_text(atext), m_distance(adist), ownInfo(false)
   {
      m_textColor.red = atextRed;
      m_textColor.green = atextGreen;
      m_textColor.blue = atextBlue;

      m_backColor.red = abgRed;
      m_backColor.green = abgGreen;
      m_backColor.blue = abgBlue;

      m_frontColor.red = afgRed;
      m_frontColor.green = afgGreen;
      m_frontColor.blue = afgBlue;
   }

   RouteInfoParts::SignPost::SignPost(Buffer *buf)
   {
      ownInfo = false;
      readFromBuf(buf);
   }

   RouteInfoParts::SignPost::~SignPost()
   {
      if (ownInfo) {
         delete[] (char *)(m_text);
      }
   }

   RouteInfoParts::SignPost& RouteInfoParts::SignPost::operator=( const RouteInfoParts::SignPost& other )
   {
      if (ownInfo) {
         delete[] (char *)(m_text);
      }
      m_text = NULL;
      m_text = strdup_new(other.m_text);
      ownInfo = true;
      m_textColor = other.m_textColor;
      m_backColor = other.m_backColor;
      m_frontColor = other.m_frontColor;
      return *this;
   }
   
   void RouteInfoParts::SignPost::writeToBuf(Buffer *buf) const
   {
      buf->writeNextCharString(m_text);
      buf->writeNext8bit( m_textColor.red );
      buf->writeNext8bit( m_textColor.green );
      buf->writeNext8bit( m_textColor.blue );

      buf->writeNext8bit( m_backColor.red );
      buf->writeNext8bit( m_backColor.green );
      buf->writeNext8bit( m_backColor.blue );

      buf->writeNext8bit( m_frontColor.red );
      buf->writeNext8bit( m_frontColor.green );
      buf->writeNext8bit( m_frontColor.blue );

      buf->writeNextUnaligned32bit(m_distance);
   }

   void RouteInfoParts::SignPost::readFromBuf(Buffer *buf)
   {
      if (ownInfo) {
         delete[] (char *)(m_text);
      }
      m_text = buf->getNextCharStringAlloc();
      ownInfo = true;
      m_textColor.red = buf->readNext8bit();
      m_textColor.green = buf->readNext8bit();
      m_textColor.blue = buf->readNext8bit();

      m_backColor.red = buf->readNext8bit();
      m_backColor.green = buf->readNext8bit();
      m_backColor.blue = buf->readNext8bit();

      m_frontColor.red = buf->readNext8bit();
      m_frontColor.green = buf->readNext8bit();
      m_frontColor.blue = buf->readNext8bit();

      m_distance = buf->readNextUnaligned32bit();
   }

   int RouteInfoParts::SignPost::log(class Log* log, int order) const
   {
#ifndef NO_LOG_OUTPUT
#endif
      return 0;
   }

   const char* RouteInfoParts::SignPost::getText() const
   {
      return m_text;
   }

/******************************************************/
/************  RouteInfoParts::Crossing ***************/
/******************************************************/

   void RouteInfoParts::Crossing::writeToBuf(Buffer *buf) const
   {
      buf->writeNextUnaligned32bit(action);
      buf->writeNextUnaligned32bit(distToNextCrossing);
      buf->writeNextUnaligned32bit(lat);
      buf->writeNextUnaligned32bit(lon);
      buf->writeNext8bit(crossingType);
      buf->writeNext8bit(exitCount);
      buf->writeNext8bit( (valid?0x01:0) | (changed?0x02:0) );
   }

   void RouteInfoParts::Crossing::readFromBuf(Buffer *buf)
   {
      action = buf->readNextUnaligned32bit();
      distToNextCrossing = buf->readNextUnaligned32bit();
      lat=buf->readNextUnaligned32bit();
      lon=buf->readNextUnaligned32bit();
      crossingType = buf->readNext8bit();
      exitCount = buf->readNext8bit();
      uint8 tmp = buf->readNext8bit();
      valid = tmp & 0x01;
      changed = (tmp & 0x02) != 0;
   }

   int RouteInfoParts::Crossing::log(class Log* log, const char* name) const
   {
#ifndef NO_LOG_OUTPUT
      log->debug("%s : action             : %"PRIu32, name, action);
      log->debug("%s : distToNextCrossing : %"PRIu32, name, distToNextCrossing);
      log->debug("%s : crossingType       : %"PRIu8,  name, crossingType);
      log->debug("%s : exitCount          : %"PRIu8,  name, exitCount);
      log->debug("%s : lat                : %"PRId32, name, lat);
      log->debug("%s : lon                : %"PRId32, name, lon);
      log->debug("%s : valid              : %s", name, valid   ? "yes" : "no");
      log->debug("%s : changed            : %s", name, changed ? "yes" : "no");
#endif
      return 0;
   }

   void RouteInfoParts::RouteListCrossing::writeToBuf(Buffer *buf) const
   {
      buf->writeNextUnaligned16bit(wptNo);
      buf->writeNext8bit(toTarget);
      buf->writeNextUnaligned32bit(distToGoal);
      buf->writeNextUnaligned32bit(timeToGoal);
      crossing.writeToBuf(buf);
      buf->writeNext8bit(segments.size());   // One Segment before the next crossing
      buf->writeNext8bit(rlc_landmarks.size());   // No landmarks

      segment_container::const_iterator i;
      for (i=segments.begin(); i!=segments.end(); ++i) {
         (*i)->writeToBuf(buf);
      }

      landmark_container::const_iterator j;
      for (j=rlc_landmarks.begin(); j!=rlc_landmarks.end(); ++j) {
         (*j)->writeToBuf(buf);
      }

      buf->writeNext8bit(rlc_lanes.noLanes);
      buf->writeNextUnaligned32bit(rlc_lanes.distance);
      buf->writeNext8bit(rlc_lanes.m_laneVector.size());
      RouteInfoParts::Lanes::rip_lane_vector::const_iterator k;
      for( k=rlc_lanes.m_laneVector.begin();
           k!=rlc_lanes.m_laneVector.end(); ++k ) {
         (*k)->writeToBuf(buf);
      }
   }

   void RouteInfoParts::RouteListCrossing::readFromBuf(Buffer *buf)
   {
      wptNo=buf->readNextUnaligned16bit();
      toTarget=buf->readNext8bit();
      distToGoal=buf->readNextUnaligned32bit();
      timeToGoal=buf->readNextUnaligned32bit();
      crossing.readFromBuf(buf);
      int numSegments = buf->readNext8bit();
      int numLandmarks = buf->readNext8bit();
      int i;

      segments.resize(numSegments);
      for (i=0; i<numSegments; ++i) {
         segments[i]=new Segment(buf);
      }

      rlc_landmarks.resize(numLandmarks);
      for (i=0; i<numLandmarks; ++i) {
         rlc_landmarks[i]=new Landmark(buf);
      }

      rlc_lanes.noLanes = buf->readNext8bit()==1;
      rlc_lanes.distance = buf->readNextUnaligned32bit();
      int numLanes = buf->readNext8bit();
      rlc_lanes.m_laneVector.resize(numLanes);
      for( i=0; i<numLanes; ++i ){
         RouteInfoParts::Lane* lane = new RouteInfoParts::Lane(buf);
         rlc_lanes.m_laneVector[i] = lane;
      }
   }

   RouteInfoParts::RouteListCrossing::~RouteListCrossing()
   {
      std::vector<Segment*>::iterator i;
      for (i=segments.begin(); i!=segments.end(); ++i) {
         delete *i;
      }
      segments.clear();
      std::vector<Landmark*>::iterator j;
      for (j=rlc_landmarks.begin(); j!=rlc_landmarks.end(); ++j) {
         delete *j;
      }
      rlc_landmarks.clear();

      RouteInfoParts::Lanes::rip_lane_vector::iterator k;
      for( k = rlc_lanes.m_laneVector.begin();
           k != rlc_lanes.m_laneVector.end();
           ++k ) {
         delete *k;
      }
      rlc_lanes.m_laneVector.clear();
   }

   const char * RouteInfoParts::RouteListCrossing::hasDetourLandmark(enum Landmark::StartStopQuery q) const
   {
      landmark_container::const_iterator i = rlc_landmarks.begin();
      while (i != rlc_landmarks.end()) {
         if ((*i)->isDetour(q, -1)) {
            return (*i)->getDescription();
         }
         ++i;
      }
      return NULL;
   }

   const char * RouteInfoParts::RouteListCrossing::hasSpeedCameraLandmark(enum Landmark::StartStopQuery q) const
   {
      landmark_container::const_iterator i = rlc_landmarks.begin();
      while (i != rlc_landmarks.end()) {
         if ((*i)->isSpeedCamera(q, -1)) {
            return (*i)->getDescription();;
         }
         ++i;
      }
      return NULL;
   }

/******************************************************/
/****************  RouteInfo **************************/
/******************************************************/

   void RouteInfo::writeToBuf(Buffer *buf) const
   {
      buf->writeNext8bit (onTrackStatus);
      buf->writeNextUnaligned16bit(simInfoStatus);
      buf->writeNextUnaligned32bit(timeToGoal);
      buf->writeNextUnaligned32bit(distToWpt);
      buf->writeNextUnaligned32bit(distToGoal);
      buf->writeNextUnaligned32bit(distToTrack);
      buf->writeNextUnaligned32bit(lat);
      buf->writeNextUnaligned32bit(lon);
//      buf->writeNextUnaligned32bit(origLat);
//      buf->writeNextUnaligned32bit(origLon);
//      buf->writeNextUnaligned32bit(destLat);
//      buf->writeNextUnaligned32bit(destLon);
      buf->writeNext8bit (toOnTrackTurn);
      buf->writeNext8bit (toTrackTurn);
      buf->writeNextUnaligned16bit(timeToWpt);
      buf->writeNextUnaligned16bit(latency);
      buf->writeNextUnaligned16bit(speed);
      buf->writeNext8bit(overSpeed);
      buf->writeNext8bit(toTarget);
      buf->writeNextUnaligned16bit(crossingNo);
      currSeg.writeToBuf(buf);
      buf->writeNextUnaligned32bit(distToAltAttribSegment);
      altAttribSegment.writeToBuf(buf);
      currCrossing.writeToBuf(buf);
      nextSeg1.writeToBuf(buf);
      nextCrossing1.writeToBuf(buf);
      nextSeg2.writeToBuf(buf);
      buf->writeNext8bit(rip_landmarks.size());
      landmark_container::const_iterator j;
      for (j=rip_landmarks.begin(); j!=rip_landmarks.end(); ++j) {
         (*j)->writeToBuf(buf);
      }
      buf->writeNext8bit(rip_lanes.noLanes);
      buf->writeNextUnaligned32bit(rip_lanes.distance);
      buf->writeNext8bit(rip_lanes.m_laneVector.size());
      RouteInfoParts::Lanes::rip_lane_vector::const_iterator k;
      for( k=rip_lanes.m_laneVector.begin();
           k!=rip_lanes.m_laneVector.end(); ++k ) {
         (*k)->writeToBuf(buf);
      }
      rip_signpost.writeToBuf(buf);
   }

   void RouteInfo::readFromBuf(Buffer *buf)
   {
      onTrackStatus = OnTrackEnum(buf->readNext8bit());
      simInfoStatus = buf->readNextUnaligned16bit();
      timeToGoal    = buf->readNextUnaligned32bit();
      distToWpt     = buf->readNextUnaligned32bit();
      distToGoal    = buf->readNextUnaligned32bit();
      distToTrack   = buf->readNextUnaligned32bit();
      lat           = buf->readNextUnaligned32bit();
      lon           = buf->readNextUnaligned32bit();
//      origLat       = buf->readNextUnaligned32bit();
//      origLon       = buf->readNextUnaligned32bit();
//      destLat       = buf->readNextUnaligned32bit();
//      destLon       = buf->readNextUnaligned32bit();
      toOnTrackTurn = buf->readNext8bit();
      toTrackTurn   = buf->readNext8bit();
      timeToWpt     = buf->readNextUnaligned16bit();
      latency       = buf->readNextUnaligned16bit();
      speed         = buf->readNextUnaligned16bit();
      overSpeed     = buf->readNext8bit() != 0;
      toTarget      = buf->readNext8bit();
      crossingNo    = buf->readNextUnaligned16bit();
      currSeg.readFromBuf(buf);
      distToAltAttribSegment = buf->readNextUnaligned32bit();
      altAttribSegment.readFromBuf(buf);
      currCrossing.readFromBuf(buf);
      nextSeg1.readFromBuf(buf);
      nextCrossing1.readFromBuf(buf);
      nextSeg2.readFromBuf(buf);
      int numLandmarks = buf->readNext8bit();
      int i;
      landmark_container::iterator lmIt;
      for( lmIt = rip_landmarks.begin();
           lmIt != rip_landmarks.end();
           ++lmIt ) {
         delete *lmIt;
      } 
      rip_landmarks.resize(numLandmarks);
      for (i=0; i<numLandmarks; ++i) {
         rip_landmarks[i]=new RouteInfoParts::Landmark(buf);
      }
      rip_lanes.noLanes = buf->readNext8bit()==1;
      rip_lanes.distance = buf->readNextUnaligned32bit();
      int numLanes = buf->readNext8bit();
      RouteInfoParts::Lanes::rip_lane_vector::iterator lvIt;
      for( lvIt = rip_lanes.m_laneVector.begin();
           lvIt != rip_lanes.m_laneVector.end();
           ++lvIt ) {
         delete *lvIt;
      }
      rip_lanes.m_laneVector.resize(numLanes);
      for( i=0; i<numLanes; ++i ){
         RouteInfoParts::Lane* lane = new RouteInfoParts::Lane(buf);
         rip_lanes.m_laneVector[i] = lane;
      }
      rip_signpost = RouteInfoParts::SignPost(buf);
   }

   RouteInfo& RouteInfo::operator=( const RouteInfo& other )
   {
      // FIXME: Better way to do this, please.
      if ( this != &other ) {
         Buffer buf(100);
         other.writeToBuf( &buf );
         this->readFromBuf( &buf );
      }
      return *this;
   }
   
   RouteInfo::RouteInfo( const RouteInfo& other ) 
   {
      *this = other;
   }
   
   RouteInfo::~RouteInfo()
   {
      landmark_container::iterator lmIt;
      for( lmIt = rip_landmarks.begin();
           lmIt != rip_landmarks.end();
           ++lmIt ) {         
         delete *lmIt;
      }
      RouteInfoParts::Lanes::rip_lane_vector::iterator lnIt;
      for( lnIt = rip_lanes.m_laneVector.begin();
           lnIt != rip_lanes.m_laneVector.end();
           ++lnIt ) {
         delete *lnIt;
      }
      rip_lanes.m_laneVector.clear();
   }

   int RouteInfo::log(class Log* log) const
   {
#ifndef NO_LOG_OUTPUT
      log->debug("--- Start of Routeinfo %p ---", (void*)this);
      log->debug(" onTrackStatus: %u", onTrackStatus);
      log->debug(" simInfoStatus: %"PRIu16, simInfoStatus);
      log->debug(" timeToGoal   : %"PRId32, timeToGoal);
      log->debug(" distToWpt    : %"PRId32, distToWpt);
      log->debug(" distToGoal   : %"PRId32, distToGoal);
      log->debug(" distToTrack  : %"PRId32, distToTrack);
      log->debug(" lat          : %"PRId32, lat);
      log->debug(" lon          : %"PRId32, lon);       
      log->debug(" toOnTrackTurn: %"PRIu8,  toOnTrackTurn);
      log->debug(" toTrackTurn  : %"PRIu8,  toTrackTurn);
      log->debug(" timeToWpt    : %"PRIu16, timeToWpt);
      log->debug(" latency      : %"PRIu16, latency);
      log->debug(" speed        : %"PRIu16, speed);       
      log->debug(" overSpeed    : %s",      overSpeed ? "yes" : "no");
      log->debug(" toTarget     : %"PRIu8,  toTarget);
      log->debug(" crossingNo   : %"PRId16, crossingNo);
      currSeg.log(log, "currSeg");
      log->debug(" distToAltAttribSegment: %"PRId32, distToAltAttribSegment);
      altAttribSegment.log(log, "altAttribSegment");
      currCrossing.log(log, "currCrossing");
      nextSeg1.log(log, "nextSeg1");
      nextCrossing1.log(log, "nextCrossing1");
      nextSeg2.log(log, "nextSeg2");
      log->debug(" rip_landmarks (%u):", rip_landmarks.size());
      for(landmark_container::const_iterator q = rip_landmarks.begin();
          q != rip_landmarks.end(); ++q){
         (*q)->log(log, std::distance(rip_landmarks.begin(), q));
      }
      log->debug("--- End   of Routeinfo %p ---", (void*)this);
#endif
      return 0;
   }


   const char * RouteInfo::hasDetourLandmark(enum RouteInfoParts::Landmark::StartStopQuery q) const
   {
      landmark_container::const_iterator i = rip_landmarks.begin();
      while (i != rip_landmarks.end()) {
         if ((*i)->isDetour(q, distToWpt)) {
            return (*i)->getDescription();
         }
         ++i;
      }
      return NULL;
   }

   const char * RouteInfo::hasSpeedCameraLandmark(
      enum RouteInfoParts::Landmark::StartStopQuery q) const
   {
      landmark_container::const_iterator i = rip_landmarks.begin();
      while (i != rip_landmarks.end()) {
         if ((*i)->isSpeedCamera(q, distToWpt)) {
            return (*i)->getDescription();;
         }
         ++i;
      }
      return NULL;
   }

/******************************************************/
/****************  RouteList **************************/
/******************************************************/

   RouteList::RouteList(Buffer *buf)
   {
      const int numCrossings = buf->readNextUnaligned16bit();

      crossings.resize(numCrossings);

      for (int i=0; i<numCrossings; ++i) {
         RouteInfoParts::RouteListCrossing *tmpcrossing =
            new RouteInfoParts::RouteListCrossing;
         tmpcrossing->readFromBuf(buf);
         crossings[i]=tmpcrossing;
      }
   }

   void RouteList::writeToBuf(Buffer *buf) const
   {
      buf->writeNextUnaligned16bit(crossings.size());
      std::vector<RouteInfoParts::RouteListCrossing *>::const_iterator i;

      for (i=crossings.begin(); i!=crossings.end(); ++i) {
         (*i)->writeToBuf(buf);
      }
   }

   RouteList::~RouteList()
   {
       std::vector<RouteInfoParts::RouteListCrossing *>::iterator i;

      for (i=crossings.begin(); i!=crossings.end(); ++i) {
         delete *i;
      }
   }

}
