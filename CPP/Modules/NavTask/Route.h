/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* An object of the class Route stores a whole or partitial route. It
 * is responsible for assembling the route from chunks of data, 
 * parse it and ansers queries concerning it. 
 *
 * It is also responsible for managing the storage required.
 */


#ifndef MODULE_NavTask_Route_H
#define MODULE_NavTask_Route_H

#include <vector>

#include "Buffer.h"
#include "Log.h"
#include "Nav2Error.h"
#include "RouteDatum.h"


namespace isab {
   class Point;

   /** 
    * Class responsible for the routes. It receives chunks of the route
    * and sets header variables, the string table with street names and 
    * fills a dynamic vector of route datums to form the route. 
    * 
    */
   class Route {
      public:
         Route(Log *log);
         virtual ~Route();

         /** @param length is the length of the complete route, not counting 
          * the packet header which should already be stripped */
         Nav2Error::ErrorNbr prepareNewRoute(int length, 
                                             bool routeOrigFromGPS,
                                             int32 origLat, int32 origLon, 
                                             int32 destLat, int32 destLon);

         /** @return 1 when the route is complete, -1 when the route
          * was broken and aborted and 0 when adding data normally. */
         int routeChunk(bool failedRoute, int chunkLength, const uint8 *chunkData,
                        Nav2Error::ErrorNbr &errval);
         
      /** 
       * Method for saving the current route to stable media.
       * Uses classic c-style I/O requests.
       * 
       * @param aFileName   The path and filename of the file to save.
       * @return            zero on success.
       */
      int writeRoute(const char* aFileName);
      
      /** 
       * Method for loading a route from stable media.
       * Uses classic c-style I/O requests.
       * 
       * @param aFileName   The path and filename of the file to load.
       * @return            zero on success.
       */
      int readRoute(const char* aFileName);

      /** 
       * Method for deleting a route from stable media.
       * Uses classic c-style I/O requests.
       * 
       * @param aFileName   The path and filename of the file to delete.
       * @return            zero on success.
       */
      int deleteRoute(const char* aFileName);

         RouteDatum* getDatum(int index);

         int findFirstPoint(Point &p);

         /** This is only to be used by Point. */
         inline RouteDatum* getFirstDatum() {
            //return m_route;
            return &m_routevector.front();
         }

         /** This is only to be used by Point. */
         inline const char* lookupString(int index) {
            return (char*)(m_stringIndex[index]);
         }


         struct RGBColor {
            uint8 red;
            uint8 green;
            uint8 blue;
         };

         /** This is only to be used by Point. */
         inline RGBColor* lookupColor(uint16 index) {
            if( index < m_colorvector.size() ){
               return (RGBColor*)(&m_colorvector[index]);
            }
            else{
               return NULL;
            }
         }


      private:
         enum ChunkDecoderState {
            Idle = 0,
            IgnoringRestOfRoute,
            ReadingHeader,
            ReadingStringTable,
            ReadingRouteDatum,
            ReadingRouteIncompleteDatum,
            ReadingColorTable,
         };

      /** 
       * Struct used to contain the header variables of the route. 
       * 
       */
      struct RouteHeader {
         int64 routeId;
         int32 topLat;
         int32 leftLon;
         int32 bottomLat;
         int32 rightLon;
         uint32 truncatedDistance;
         uint32 phoneHomeDistance;
         int32 origLat;
         int32 origLon;
         int32 destLat;
         int32 destLon;
         bool routeOrigFromGPS;
         int stringLength;
         int colorTableSize;
      };

      /** 
       * Struct used to contain the header data for the file. 
       * 
       */
      struct FileHeader {
         float version;
         char fileName[100];
         int routeVectorSize;
      };

#ifndef _MSC_VER
         static const int ExpectedHeaderLength = 40;
         static const int RouteDatumSize = 12;
         static const int MaxRouteData = 1500;
         static const int InitialRouteDataSize = 1500;
         static const int tmptest = 0;
#else
         enum { ExpectedHeaderLength = 40,
                RouteDatumSize = 12,
                MaxRouteData = 1500,
                InitialRouteDataSize = 1500 };
#endif

         Buffer m_mergeBuf;
         // Comes from NavTaskProviderDecoder::dispatch - packetLength 
         // is this the total amount of data for the route?
         int m_expectedDataSize;
         int m_dataReceivedSoFar;
         int m_stringBytes;
         ChunkDecoderState m_decoderState;
         //RouteDatum m_route[MaxRouteData];
         // Store the route in a vector instead to make it dynamic.
         RouteHeader m_routeHeader;
         FileHeader m_fileHeader;
         std::vector<RouteDatum> m_routevector;
      //int m_numData; //for the route datum array, can be removed later.
         uint8* m_stringTable;
         std::vector<uint8*> m_stringIndex;

         uint8* m_colordata;
         std::vector<RGBColor> m_colorvector;

         Log *m_log;

         /** @return 1 when the route is valid, -1 if the route
          * is damaged and unusable. */
         int checkFinishedRoute();

         /** Build an index into the string table. */
         void buildStringIndex();

         /** Build the color table for sign posts. */
         void buildColorTable();

      public:
         int64 getRouteId() const { return m_routeHeader.routeId; }
         int getPhoneHomeDistance() const { return m_routeHeader.phoneHomeDistance; }
         int getTruncatedDistance() const { return m_routeHeader.truncatedDistance; }
         int32 getTopLat() const { return m_routeHeader.topLat; }
         int32 getLeftLon() const { return m_routeHeader.leftLon; }
         int32 getBottomLat() const { return m_routeHeader.bottomLat; }
         int32 getRightLon() const { return m_routeHeader.rightLon; }
         int32 getOrigLat() const { return m_routeHeader.origLat; }
         int32 getOrigLon() const { return m_routeHeader.origLon; }
         int32 getDestLat() const { return m_routeHeader.destLat; }
         int32 getDestLon() const { return m_routeHeader.destLon; }
         bool wasRouteOrigGPS() const { return m_routeHeader.routeOrigFromGPS; }

   };


} /* namespace isab */

#endif /* MODULE_NavTask_Route_H */

