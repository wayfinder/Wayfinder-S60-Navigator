/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef FAKER_H
#define FAKER_H

#include <map>

namespace isab{

   class fakerstring {
      char* m_str;
   public:
      fakerstring() : m_str(NULL) {}
      explicit fakerstring(const char* s) : m_str(NULL)
      {
         m_str = strdup_new(s);
      }
      fakerstring(const fakerstring& f) : m_str(NULL)
      {
         m_str = strdup_new(f.m_str);
      }
      explicit fakerstring(const std::vector<char>& v) : m_str(NULL)
      {
         if(!v.empty()){
            m_str = new char[v.size() + 1];
            std::copy(v.begin(), v.end(), m_str);
            m_str[v.size()] = 0;
         }
      }
      const fakerstring& operator=(const fakerstring& rhs)
      {
         if(*this != rhs){
            delete[] m_str;
            m_str = strdup_new(rhs.m_str);
         }
         return *this;
      }
      ~fakerstring()
      {
         delete[] m_str;
      }
      bool operator<(const fakerstring& rhs) const
      {
         return 0 > strcmp(m_str ? m_str : "", rhs.m_str ? rhs.m_str : "");
      }
      bool operator>=(const fakerstring& rhs) const
      {
         return !(*this < rhs);
      }
      bool operator>(const fakerstring& rhs) const
      {
         return 0 < strcmp(m_str ? m_str : "", rhs.m_str ? rhs.m_str : "");
      }
      bool operator<=(const fakerstring& rhs) const
      {
         return !(*this > rhs);
      }
      bool operator==(const fakerstring& rhs) const
      {
         return strequ(m_str ? m_str : "", rhs.m_str ? rhs.m_str : "");
      }
      bool operator!=(const fakerstring& rhs) const
      {
         return !(*this == rhs);
      }
      const char* c_str() const
      {
         return m_str;
      }
   };

   class Faker : public NavServerComProviderInterface, 
                 public NavServerComTimerCallBack {
      NavServerComReplyInterface* m_interface;
      Log* m_log;
      Faker(const Faker&);
      const Faker& operator=(const Faker&);
 
      // NOTE!!! Memory leak of AdditionalInfo's!!!
      // Need to delete when shutting down...
      std::multimap<fakerstring, class AdditionalInfo*> m_info;
      // NOTE!!! Memory leak of AdditionalInfo's!!!
      // Need to delete when shutting down...
      std::set<class SearchRegion* > m_regions;
      std::multimap<fakerstring, fakerstring> m_item2region;
      std::vector<class FullSearchItem*> m_items;

      void parseinfo(char** currentinfo, FILE* sf);
      void parseregion(FILE* sf);           
      void parseitem(FILE* sf);             
      void parsemap(FILE* sf);
      bool readSearchData(const char* filename);

      struct routeRequestData {
         routeRequestData() : 
            timerid(0xffff), toLat(MAX_INT32), toLon(MAX_INT32),
            fromLat(MAX_INT32), fromLon(MAX_INT32), heading(0), oldRouteId(0),
            src(0)
         {}
         uint16 timerid;
         int32 toLat; int32 toLon; 
         int32 fromLat; int32 fromLon; 
         uint16 heading; int64 oldRouteId;
         uint8 rerouteReason;
         uint32 src;
      } m_routeRequest;

   public:
      Faker() : 
         m_interface(NULL) 
      {
         m_log = new Log("Faker");
      }

      void timerExpired(uint16 id); 
      
      void setInterface(NavServerComReplyInterface* a_interface)
      { 
         m_interface = a_interface;
      }

      virtual ~Faker()
      {
         delete m_log;
      }
      /** This method is called from NavServerComDecoder::dispatch.
       * It clears out old parameter settings and set the new ones.
       * @param data   the parameter data. This data is never parsed but sent 
       *               as is to the server at the beginning of every session.
       *               NavServerCom depends on this block being terminated with
       *               the end parameter tag.
       * @param length the size of the data block.
       * @param src    the address of the sender of the parameter block.
       */
      virtual void decodedSetServerParams(const uint8* data, size_t length, 
                                          uint32 src);
      /** Called by NavServerComDecoder::dispatch when a syncdestinations 
       * request arrives from another module. 
       * XXX this method is not yet implemented.
       * @param length the size of the data block.
       * @param data   a byte array containing the syncDestinationsRequest
       *               already coded for transmission to the server.
       * @param src    the original requester.
       */
      virtual void decodedSyncDestinations(int length, const uint8* data, 
                                           uint32 src) ;

      /** Called by NavServerComDecoder::dispatch when a whereAmI request 
       * arrives from another module. 
       * @param lat the latitude of the postion to look up. Given in rad*1e8.
       * @param lon the longitude of the position to look up. Given in rad*1e8.
       * @param src the address of the original requester.
       */
      virtual void decodedWhereAmI(int32 lat, int32 lon, uint32 src);

      /** Called by NavServerComDecoder::dispatch when a search request 
       * arrives from another module. 
       * @param data   the request data already coded as a server request.
       * @param length the size of the data block.
       * @param lat the latitude to search from. 
       *            (Redundant, but used for error checking.)
       * @param lon the longitude to search from.
       *            (Redundant, but used for error checking.)
       * @param country the code for the country to search in. 
       *                (Redundant, but used for error checking.)
       * @param city the city string entered by the user.
       *             (Redundant, but use for error checking.)
       * @param src    the address of the original requester.
       * @param latOffset the offset into the data where the new latitude 
       *                  should be written. Defaults to -1, which 
       *                  means don't refresh.
       * @param lonOffset the offset into the data where the new longitude 
       *                  should be written. Defaults to -1, which 
       *                  means don't refresh.
       */   
      virtual void decodedSearchRequest(const uint8* data, int length,
                                        int32 lat, int32 lon, uint8 hdg,
                                        uint32 country, const char* city,
                                        uint32 src, 
                                        int latOffset = -1, 
                                        int lonOffset = -1,
                                        int hdgOffset = -1);

      /** Called by NavServerComDecoder::dispatch when a route request 
       * arrives from another module. All latitude and lonitude coordinates 
       * are specified in rad * 1e8.
       * @param toLat   latitude to route to.
       * @param toLon   longitude to route to.
       * @param fromLat latitude to route from.
       * @param fromLon longitude to route from.
       * @param heading current heading.
       * @param src  the address of the original requester.
       */   
      virtual void decodedRouteToGps(int32 toLat, int32 toLon, 
                                     int32 fromLat, int32 fromLon,
                                     uint16 heading, 
                                     uint32 routeRequestorId,
                                     int64 oldRouteId,
                                     uint8 rerouteReason,
                                     uint32 src);

      virtual void decodedMapRequest(int len, const uint8* data,  uint32 dst,
                                     int latOffset, int lonOffset, 
                                     int hdgOffset, int spdOffset);

      virtual void decodedVectorMapRequest(int /*len*/, const uint8* /*data*/,
                                           uint32 /*src*/)
      {
         //this fubction intentionally left blank.
      }

      virtual void decodedMultiVectorMapRequest(int /*len*/, const uint8* /*data*/, 
                                                uint32 /*src*/)
      {
         //This function intentionally left blank
      }

      virtual void decodedSearchInfoRequest(int len, const uint8* data, 
                                            uint32 src);

      virtual void decodedMessageRequest(const uint8* /*data*/, unsigned /*length*/,
                                         uint32 /*src*/)
      {}


      /** Called by NavServerComDecoder::dispatch when a binary upload request 
       * arrives from another module. 
       * @param len  the size of the data block.
       * @param data the bainry block to send to the server.
       * @param src  the address of the original requester.
       */   
      virtual void decodedBinaryUpload(int len, const uint8 * data, 
                                       uint32 src);
      virtual void decodedCancelRequest(uint32 originalSrc, uint32 netmask, 
                                        uint32 src);

      virtual void decodedRequestReflash(uint32 src);

      virtual void decodedVerifyLicenseKey(const char *key, const char *phone, 
                                           const char* name, const char* email,
                                           const char* optional,
                                           uint32 region, uint32 src);
      virtual void decodedParamSync(uint32 src);
      virtual void decodedCellReport(const uint8* /*data*/, uint32 /*size*/, 
                                     uint32 /*dst*/)
      { /*This function intentionally left blank. */}
      virtual void decodedSendRequest( const MsgBuffer* /*buf*/ )
      { /*This function intentionally left blank. */}

   };
}

inline void isab::Faker::decodedSyncDestinations(int /*length*/, 
                                                 const uint8* /*data*/, 
                                                 uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}
      
inline void isab::Faker::decodedSetServerParams(const uint8* /*data*/, 
                                                size_t /*length*/, 
                                                uint32 /*src*/)
{
}

inline void isab::Faker::decodedWhereAmI(int32 /*lat*/, int32 /*lon*/, 
                                         uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}


inline void isab::Faker::decodedCancelRequest(uint32 /*originalSrc*/, 
                                              uint32 /*netmask*/, uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}

inline void isab::Faker::decodedRequestReflash(uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}

inline void isab::Faker::decodedParamSync(uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}

#endif
