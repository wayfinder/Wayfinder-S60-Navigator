/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UICTRL_INTERNAL_H
#define UICTRL_INTERNAL_H

#include "HintNode.h"

namespace isab {

   namespace UiCtrlInternal {
      /**
       * Struct for storing GPS data.
       */
      struct GpsData{ 
         uint8 posQuality;
         uint8 speedQuality;
         uint8 headingQuality;
         int32 lat, lon;
         int32 routelat, routelon;
         uint8 heading;
         uint8 routeheading;
         int   speed;
         int32 alt;
         uint32 timeStampMillis;
         std::vector<HintNode> positionHints;
         
         GpsData() : 
            posQuality(QualityMissing), speedQuality(QualityMissing),
         headingQuality(QualityMissing), lat(MAX_INT32), lon(MAX_INT32),
         routelat(MAX_INT32), routelon(MAX_INT32),
         heading(0), routeheading(0), speed(0),
            alt(0), timeStampMillis(0)
         {}
         void setMissing()
         {
            *this = GpsData();
            //          posQuality = speedQuality = headingQuality = QualityMissing;
            //          lat = lon = heading = speed = 0;
         }
      }; 

      /**
       * Struct used for elements in the vector that is 
       * used when asking the parameter module for parameters
       * and joining its multicast groups.
       */
      struct UiCtrlParams {
         /**
          * The parameter id of a parameter we are 
          * interested in.
          */
         enum ParameterEnums::ParamIds paramId;

         /**
          * Initially false. Is set to true once we have
          * received a valid value for the paremter we've
          * asked for.
          */
         bool received;   
      };

      class RouteIdData: public RequestData {
         public:
            RouteIdData(int64 id) : routeid(id) {}
            virtual ~RouteIdData() {}
            int64 routeid;
      };

      class ImageData : public RequestData{
         public:
            ImageData(uint16 Width, uint16 Height) : width(Width), height(Height) {}
            virtual ~ImageData(){}
            uint16 width;
            uint16 height;
      };

   }

}

#endif
