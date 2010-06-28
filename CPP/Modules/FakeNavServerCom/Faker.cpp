/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"

#include "GuiProt/ServerEnums.h"
#include "Nav2Error.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"

#include "NavPacket.h"
#include "Buffer.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "NavServerCom.h"

#include "GuiProt/SearchRegion.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "nav2util.h"
#include "Faker.h"

#include "Quality.h"
#include "isabTime.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"

#include "GuiProt/MapClasses.h"
#include "GlobalData.h"
#include "LogMacros.h"

#include "ParameterEnums.h"
#include "Parameter.h"
//#include "../NavServerCom/DestinationSync.h"
#include "Constants.h"

#include <stdio.h>
#include <stddef.h>
#include <ctype.h>
#include <vector>
#include <math.h>

#define NAV_SERVER_COM_HEADER_SIZE 14

#define LINUX_BASE_PATH "Resources/DemoData/Stockholm2/"

using namespace std;

void isab::Faker::decodedMapRequest(int len, const uint8* data,  
                                    uint32 dst,
                                    int /*latOffset*/, int /*lonOffset*/, 
                                    int /*hdgOffset*/, int /*spdOffset*/)
{
   // decode data
   Buffer req(len);
   req.writeNextByteArray(data, len);
   BoundingBox* bb = BoundingBox::deserialize(&req);
   uint16 width = req.readNextUnaligned16bit();
   uint16 height = req.readNextUnaligned16bit();
   uint16 vbw = req.readNextUnaligned16bit();
   uint16 vbh = req.readNextUnaligned16bit();
   vbh = vbh;
   vbw = vbw;
   DBG("decodedMapRequest: width: %d, height: %d", width, height);
   //select map file
#ifdef __SYMBIAN32__
   const char* path = getGlobalData().m_commondata_base_path;
#else
   const char* path = LINUX_BASE_PATH;
#endif
#define SCNCRD(fmtspec) "(%"fmtspec",%"fmtspec")"
   const char* scfmt = SCNCRD(SCNd32)SCNCRD(SCNd32)SCNCRD("d")SCNCRD(SCNd32);

   int32 tlat = 0, tlon = 0, blat = 0, blon = 0, rww = 0, rwh = 0;
   int w = 0, h = 0;

   char filename[256] = {0};
   const char* type = NULL;
   switch(bb->getBoxType()){
   case BoundingBox::invalidBox: //error
      break;
   case BoundingBox::diameterBox: //zoom to turn, position, ...
      {
         DBG("DiameterBox request");
         DiameterBox* db = static_cast<DiameterBox*>(bb);
         const char * const types[] = {"orig", "dest"};
         char tmp[256] = {0};
         for(size_t a = 0; (!type) && (a < sizeof(types)/sizeof(*types)); ++a){
            snprintf(tmp, sizeof(tmp) - 1, "%s%dx%d%sdata.txt",
                     path, width, height, types[a]);
            DBG("Testing file '%s' for coords (%"PRId32",%"PRId32")", tmp, 
                db->getLat(), db->getLon());
            FILE* dataf = fopen(tmp, "r");
            if(dataf){
               DBG("'%s' opened OK", tmp);
               tlat = tlon = blat = blon = rww = rwh = w = h = 0;
               if(8 == fscanf(dataf, scfmt, &tlat, &tlon, &blat, &blon, 
                              &rww, &rwh, &w, &h)){
                  DBG("(%"PRId32",%"PRId32")(%"PRId32",%"PRId32")"
                      "(%"PRId32",%"PRId32")(%d,%d)",
                      tlat, tlon, blat, blon,
                      rww, rwh, w, h);
                  if(BoxBox(tlat, tlon, 
                            blat, blon).Contains(db->getLat(), db->getLon())){
                     DBG("Contains(%"PRId32",%"PRId32")", db->getLat(), db->getLon());
                     type = types[a];
                  }
               }
               fclose(dataf);
            }
         }
         if(type == NULL){
            type = "route";
         }
      }
      break;
   case BoundingBox::boxBox:  //zoom?
   case BoundingBox::vectorBox: //tracking
   case BoundingBox::routeBox: //zoom to route
      type = "route";
   }

   if(type != NULL){
      snprintf(filename, sizeof(filename) - 1, "%s%dx%d%s.gif", 
               path, width, height, type);
      DBG("opening file %s", filename);
      FILE* mapfile = fopen(filename, "rb");
      if(mapfile){
         snprintf(filename, sizeof(filename) - 1, "%s%dx%d%sdata.txt",
                  path, width, height, type);
         FILE* datafile = fopen(filename, "r");
         if(datafile && (8 == fscanf(datafile, scfmt, &tlat, &tlon, 
                                     &blat, &blon, &rww, &rwh, &w, &h))){
            Buffer reply(5*1024);
            reply.writeNextUnaligned32bit(tlat);
            reply.writeNextUnaligned32bit(tlon);
            reply.writeNextUnaligned32bit(blat);
            reply.writeNextUnaligned32bit(blon);
            reply.writeNextUnaligned16bit(w);
            reply.writeNextUnaligned16bit(h);
            reply.writeNextUnaligned32bit(rww);
            reply.writeNextUnaligned32bit(rwh);
            int lengthpos = reply.getWritePos();
            reply.writeNextUnaligned32bit(0); //buffer size
            reply.writeNextUnaligned16bit(GIF);
            int start = reply.getWritePos(); //buffer start
            
            const size_t BUFFER_LEN = 1024;
            uint8* buffer = new uint8[BUFFER_LEN];
            size_t len = 0;
            while(BUFFER_LEN == (len = fread(buffer, sizeof(*buffer), 
                                             BUFFER_LEN, mapfile))){
               reply.writeNextByteArray(buffer, len);
            }
            reply.writeNextByteArray(buffer, len);
            delete[] buffer;
            int end = reply.setWritePos(lengthpos);
            reply.writeNextUnaligned32bit(end - start);
            m_interface->
               NavServerComConsumerPublicRef().mapReply(reply.getLength(), 
                                                        reply.accessRawData(0),
                                                        dst);
            fclose(datafile);
         } else {  //data error
            m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED,dst);
         }
         fclose(mapfile);
      } else {
         m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, dst);
      }
   } else {
      m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, dst);
   }      
   delete bb;
}

namespace {
   ///Warning! Code duplication from GuiCtrl.cpp
   uint32 crowsFlight(int32 lat1, int32 lon1, int32 lat2, int32 lon2)
   {
      if ((lat1 == 0 && lon1 == 0) ||
          (lat1 == MAX_INT32 && lon1 == MAX_INT32) ||
          (lat2 == 0 && lon2 == 0) ||
          (lat2 == MAX_INT32 && lon2 == MAX_INT32)
         ) {
         /* No good coordinates. */
         return MAX_UINT32;
      }
      const double C = 1e8f; 
      double d1 = lat1 / C;
      double d2 = lat2 / C;
      double cd1 = cos(d1);
      double cd2 = cos(d2);
      double sd1 = sin(d1);
      double sd2 = sin(d2);
      double ldiff = (lon1 - lon2) / C;
      double cldiff = cos(ldiff);
      double ca = cd1 * cd2 * cldiff + sd1 * sd2;
      double distance = isab::Constants::earthRadius * acos(ca);
      return uint32(distance);
   }

   ///Warning! Almost code duplication from GuiCtrl.cpp
   class UpdateDistance{
      const int32 m_lat;
      const int32 m_lon;
   public:
      UpdateDistance(int32 lat, int32 lon) : m_lat(lat), m_lon(lon) {}
      typedef void result_type;
      typedef class isab::SearchItem* argument_type;
      result_type operator()(argument_type arg)
      {
         arg->setDistance(crowsFlight(m_lat, m_lon, 
                                     arg->getLat(), arg->getLon()));
      }
   };

   
}

void isab::Faker::decodedSearchRequest(const uint8* /*data*/, int /*length*/,
                                       int32 lat, int32 lon, uint8 /*hdg*/,
                                       uint32 /*country*/, const char* /*city*/,
                                       uint32 src, 
                                       int /*latOffset*/, int /*lonOffset*/,
                                       int /*hdgOffset*/)
{
   DBG("FakeSearchRequest");
   if(m_items.empty()){
#ifdef __SYMBIAN32__
      const char* path = getGlobalData().m_commondata_base_path;
#else
      const char* path = LINUX_BASE_PATH;
#endif
      char filename[256] = {0};
      snprintf(filename, sizeof(filename) - 1, "%ssearch.txt", path);
      DBG("Reading search data from '%s'", filename);
      readSearchData(filename);
      DBG("Search data read. %d items, %d regions, %d infoitems, "
          "and %d mappings", m_items.size(), m_regions.size(), m_info.size(), 
          m_item2region.size());
   }

   Buffer searchReply(256);
   searchReply.writeNext8bit(0); //search items
   searchReply.writeNextUnaligned16bit(m_items.size()); //num in this packet
   searchReply.writeNextUnaligned16bit(0); //start index of total
   searchReply.writeNextUnaligned16bit(m_items.size()); //total num
   int numRegionsPos = searchReply.getWritePos();
   searchReply.writeNextUnaligned16bit(0); //num of regions, fill in later

   std::set<class SearchRegion*>::iterator q;
   std::vector<fakerstring> regionids; //store ids of written regions here
   typedef std::multimap<fakerstring,fakerstring>::iterator idmapiterator;
   // for each region, see if it's id is mapped to any of the
   // searchitems.
   for(q = m_regions.begin(); q != m_regions.end(); ++q){
      regionids.push_back(fakerstring((*q)->getId()));//push id to vector

      //find if the id is present in the mapping
      idmapiterator n = m_item2region.begin();
      while(n != m_item2region.end() && n->second != regionids.back()){ 
         ++n; 
      }
      if(n != m_item2region.end()){
         //the region is mapped to a fullsearchitem Serialize the region
         DBG("Serializing SearchRegion '%s'", regionids.back().c_str());
         (*q)->serialize(&searchReply);
      } else {
         //the region is unused, remove from vector of used ids
         DBG("ID '%s' not found", regionids.back().c_str());
         regionids.pop_back();
      }      
   }

   int fsipos = searchReply.setWritePos(numRegionsPos);//remeber pos
   searchReply.writeNextUnaligned16bit(regionids.size()); //write num regions
   DBG("Wrote %u searchregions", regionids.size());
   searchReply.setWritePos(fsipos); //return to current buffer pos

   ///serialize searchitems. The server protocol uses another
   ///serialization of SearchItems than the GUI protocol so we cn't
   ///use the serialization member function.
   std::vector<class FullSearchItem*>::iterator p;
#ifndef _MSC_VER
   //wont compile in visual studio 6
   if(lat == MAX_INT32 || lon == MAX_INT32){
      m_interface->getCoords(lat,lon);
   }
   DBG("Updating Distance (lat: %"PRId32", lon: %"PRId32")", lat, lon);
   std::for_each(m_items.begin(), m_items.end(), UpdateDistance(lat, lon));
   std::sort(m_items.begin(), m_items.end(), DistanceLess());
#endif
   for(p = m_items.begin(); p != m_items.end(); ++p){
      //serialize the easy part
      searchReply.writeNextCharString((*p)->getID());
      searchReply.writeNextUnaligned16bit((*p)->getType());
      searchReply.writeNextUnaligned16bit((*p)->getSubType());
      searchReply.writeNextUnaligned32bit((*p)->getLat());
      searchReply.writeNextUnaligned32bit((*p)->getLon());
      searchReply.writeNextUnaligned32bit((*p)->getAlt());
      //multimap<fakerstring, fakerstring>::iterator
      typedef std::pair<idmapiterator, idmapiterator> iditeratorpair;
      //find all region ids that are mapped to this FSI
      iditeratorpair range = 
         m_item2region.equal_range(fakerstring((*p)->getID()));
      //write number of regions mapped to this FSI
      searchReply.writeNext8bit(std::distance(range.first, range.second));
      searchReply.writeNextCharString((*p)->getName()); //write FSI name
      for(idmapiterator w = range.first; w != range.second; ++w){
         //find the region id in the vector containing the written regions ids
         std::vector<fakerstring>::iterator reg = 
            std::find(regionids.begin(), regionids.end(), w->second);
         //write the regions index
         int index = std::distance(regionids.begin(), reg);
         DBG("SearchItem '%s' (dist %"PRId32") refers to regions %d", 
             (*p)->getName(), (*p)->getDistance(), index);
         searchReply.writeNextUnaligned16bit(index);
      }
   }
   //send reply
   m_interface->
      NavServerComConsumerPublicRef().searchReply(searchReply.accessRawData(0),
                                                  searchReply.getLength(), 
                                                  src);
}

void isab::Faker::timerExpired(uint16 id)
{
   if(id == m_routeRequest.timerid){
      DBG("route timer expired.");
      m_routeRequest.timerid = 0xffff;
      char file[256];
#ifdef __SYMBIAN32__
      const char* path = getGlobalData().m_commondata_base_path;
#else
      const char* path = LINUX_BASE_PATH;
#endif
      snprintf(file, sizeof(file), "%sroute.rte", path);
      DBG("opening file %s", file);
      FILE* route = fopen(file, "rb");
      if(route){
         DBG("file open alright!");
         Buffer packet(1024);
         const size_t BUFFER_LEN = 1024;
         uint8* buffer = new uint8[BUFFER_LEN];
         size_t len = 0;
         while(BUFFER_LEN == (len = fread(buffer, sizeof(*buffer), 
                                          BUFFER_LEN, route))){
            packet.writeNextByteArray(buffer, len);
         }
         packet.writeNextByteArray(buffer, len);
         delete[] buffer;
         fclose(route);
         
         snprintf(file, sizeof(file), "%sroutedata.txt", path);
         DBG("Opening file '%s'", file);
         FILE* routedatafile = fopen(file, "r");
         if(routedatafile){
            DBG("File '%s' opened alright", file);
            int32 olat, olon, dlat, dlon;
            if(4 == fscanf(routedatafile, 
                           "_R%"SCNd32"%%%"SCNd32"%%%"SCNd32"%%%"SCNd32, 
                           &olat, &olon, &dlat, &dlon)){
               m_routeRequest.fromLat = olat;
               m_routeRequest.fromLon = olon;
               m_routeRequest.toLat   = dlat;
               m_routeRequest.toLon   = dlon;
            }
            fclose(routedatafile);
         }

         packet.setReadPos(1);
         int packlen = packet.readNextUnaligned32bit() - 
            NAV_SERVER_COM_HEADER_SIZE;
         
         m_interface->
            NavTaskProviderPublicRef().newRoute(true,
                  m_routeRequest.fromLat, 
                  m_routeRequest.fromLon, 
                  m_routeRequest.toLat, 
                  m_routeRequest.toLon,
                  packlen);
         packet.setReadPos(14);
         m_interface->
            NavTaskProviderPublicRef().routeChunk(false, packet.remaining(),
                                                  packet.accessRawData());

         DBG("sent the fake route.");
         m_interface->NavServerComConsumerPublicRef().
            progressMessage( NavServerComEnums::done, GuiProtEnums::CreatingRoute);
      } else {
         ERR("Unable to open route file %s", file);
      }   
   }
}



void isab::Faker::decodedRouteToGps(int32 toLat, int32 toLon, 
                                    int32 fromLat, int32 fromLon, 
                                    uint16 heading, 
                                    uint32 /*routeRequestorId*/,
                                    int64 oldRouteId,
                                    uint8 rerouteReason,
                                    uint32 src)
{
   if(m_routeRequest.timerid == 0xffff){
      m_interface->NavServerComConsumerPublicRef().
         progressMessage( NavServerComEnums::connecting, GuiProtEnums::CreatingRoute);

      DBG("decodedRouteToGps, set 2 sec timer.");
      m_routeRequest.toLat = toLat; 
      m_routeRequest.toLon = toLon; 
      m_routeRequest.fromLat = fromLat; 
      m_routeRequest.fromLon = fromLon; 
      m_routeRequest.heading = heading; 
      m_routeRequest.oldRouteId = oldRouteId;
      m_routeRequest.rerouteReason = rerouteReason;
      m_routeRequest.src = src;
      m_routeRequest.timerid = m_interface->setTimer(2000, this);
   } else {
      WARN("Already 'waiting' for route");
      m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
   }
}

void isab::Faker::decodedVerifyLicenseKey(const char *key, 
                                          const char* /*phone*/, 
                                          const char* /*name */,
                                          const char* /*email*/,
                                          const char* /*optional*/,
                                          uint32 /*region*/, 
                                          uint32 src)
{
   if(*key == '0'){
      int32 gold = GuiProtEnums::Gold;
      ParameterEnums::ParamIds paramid = 
         ParameterEnums::WayfinderType;
      m_interface->ParameterProviderPublicRef().setParam(paramid, &gold);
   }
   m_interface->NavServerComConsumerPublicRef().licenseKeyReply(*key == '0', 
                                                                true, true,
                                                                true, true,
                                                                GuiProtEnums::Gold,
                                                                src);      
}

void isab::Faker::decodedBinaryUpload(int len, const uint8 * data, 
                                      uint32 src)
{
   m_interface->sendError(Nav2Error::NSC_OPERATION_NOT_SUPPORTED, src);
}

void isab::Faker::decodedSearchInfoRequest(int len, const uint8* data, 
                                           uint32 src)
{
   Buffer req(len + 16);
   req.writeNextByteArray(data, len);
   Buffer rep(len*2);
   uint16 num = req.readNextUnaligned16bit(); 
   rep.writeNextUnaligned16bit(num); 
   for(; num > 0; --num){
      const char* id = req.getNextCharString();
      req.getNextCharString();
      typedef std::multimap<fakerstring, class AdditionalInfo *> infocontainer;
      typedef infocontainer::iterator infoiterator;
      typedef std::pair<infoiterator, infoiterator> inforange;
      inforange range = m_info.equal_range(fakerstring(id));
      rep.writeNextCharString(id);
      rep.writeNext8bit(std::distance(range.first, range.second));
      for(infoiterator ii = range.first; ii != range.second; ++ii){
         rep.writeNextUnaligned32bit(ii->second->getType());
         rep.writeNextCharString(ii->second->getKey());
         rep.writeNextCharString(ii->second->getValue());
      }
   }
   m_interface->
      NavServerComConsumerPublicRef().searchInfoReply(rep.accessRawData(0), 
                                                      rep.getLength(), src);
}

//This part of the code deals with parsing the Searchdata
//file. There's room for improvement....

// For parser debugging
#ifdef DEBUG_PARSER
# define DBGP DBG
#else
# define DBGP if(0)DBG
#endif

namespace {
   void clearline(FILE* f)
   {
      int c;
      while((EOF != (c = fgetc(f))) && (c != '\n')) { continue; }
   }
}

void isab::Faker::parseinfo(char** id, FILE* f)
{
   std::vector<char> name;
   std::vector<char> content;
   int type = -1;
   int c;
   std::vector<char>* current = &name;
   DBGP("parseinfo \"%s\"",*id);
   while(EOF != (c = fgetc(f)) && c != '\n'){
      if(c == '\''){ //signal break in input
         DBGP("Found ''', input break");
         c = fgetc(f);
         if(c == EOF){
            ERR("''' followed by unexpected EOF.");
            break; //while
         }
         if(c == '\n'){ 
            if(!current->empty()){
               current->push_back(0);
               replaceString(*id, &current->front());
               DBG("New id for additionalinfos: '%s'", *id);
               return;
            } else {
               ERR("''' followed by unexpected EOL.");
            }
            break; //while
         }
         if(c == ','){
            DBGP("',' found. May be new field");
            if(EOF == (c = fgetc(f)) || c == '\n'){
               ERR("',' followed by unexpected EOL or EOF");
               return; //error eof or eol
            } else if(c == '\''){ 
               DBGP("',' followed by '''. New field.");
               //comma followed by '\'', new field, switch inserter
               current = &content;
            } else if(c == '0'){
               DBGP("''' followed by '0'. Unget character, scan integer");
               ungetc(c, f); //unget char
               if(1 != fscanf(f, "0x%x", &type)){
                  ERR("Unable to parse hexadecimal integer");
                  clearline(f);
                  return;
               }
               clearline(f);
               break;
            } else {
               //add to string
               current->push_back(c);
            }
         } else {
            current->push_back(c);
         }
      } else {
         //ordinary char, insert in string
         current->push_back(c);
      }
   } 
   if(type > -1){
      name.push_back(0);
      content.push_back(0);

      //insert newlines every 80 character or so. This helps the GUI
      //scrolling dialogs display all the text.
      std::vector<char>::iterator foo = content.begin();
      const int linelength = 80;
      while(std::distance(foo, content.end()) > linelength){
         std::advance(foo, linelength - 1);
         if(content.end() != (foo = std::find(foo, content.end(), ' '))){
            *foo++ = '\n';
         }
      }

      DBG("AdditionalInfo(%s,%s,%#x)", &name.front(), &content.front(), type);
      m_info.insert(make_pair(fakerstring(*id), 
                              new AdditionalInfo(&(name.front()), 
                                             &(content.front()), type)));
   } else {
      ERR("Unable to read full info string.");
   }
}

void isab::Faker::parseregion(FILE* f)
{
   char text[256] = {0};
   if(fgets(text, sizeof(text), f) && text[strlen(text) - 1] == '\n'){
      char* name = text;
      DBGP("read line \"%s\" from file", text);
      if(*name++ != '\''){
         ERR("Found '%c', was expecting '''", *(name - 1));
         return;
      }
      char* delim = strstr(name, "','");
      if(!delim){
         ERR("No \"','\" found in line \"%s\"", name);
         return;
      }
      *delim = 0;
      delim += 3;
      char* id = delim;
      delim = strstr(id, "',");
      if(!delim){
         ERR("No \"',\" found in line \"%s\"", id);
         return;
      }
      *delim = 0;
      delim += 2;
      char* end;
      unsigned long type = strtoul(delim, &end, 0);
      if(end == delim){
         ERR("No integer read from line \"%s\"", delim);
         return;
      }
      DBG("Region(%#lx, %s, %s) read from file", type, id, name);
      m_regions.insert(new SearchRegion(type, id, name));
   } else {
      ERR("Unable to read line from file.");
   }
}

void isab::Faker::parseitem(FILE* f)
{
   int lookahead[3] = {0};
   std::vector<char> name;
   for(int i = 0; i < 3; ++i){
      if(EOF == (lookahead[i] = fgetc(f)) || lookahead[i] == '\n'){
         ERR("Unexpected end of line or file");
         return;
      }
   }
   if(lookahead[0] != '\''){
      clearline(f);
      ERR("Character '%c' found. Was expecting '''", lookahead[0]);
      return;
   }
   memmove(lookahead, lookahead + 1, 2*sizeof(*lookahead));
   if(EOF == (lookahead[2] = fgetc(f)) || lookahead[2] == '\n'){
      ERR("Unexpected end of line or file");
      return;
   }
   const int delim[] = {'\'',',','\''};
   while(0 != memcmp(lookahead, delim, sizeof(delim))){
      name.push_back(*lookahead);
      memmove(lookahead, lookahead + 1, 2*sizeof(*lookahead));
      if(EOF == (lookahead[2] = fgetc(f)) || lookahead[2] == '\n'){
         ERR("Unexpected en of line or file.");
         return;
      }
   }
   char line[256] = {0};
   if(fgets(line, sizeof(line), f) && line[strlen(line) - 1] == '\n'){
      char id[40] = {0};
      unsigned type, subtype;
      int32 lat, lon;
      if(5 == sscanf(line, "%[a-zA-Z:0-9]',0x%x,0x%x,%"SCNd32",%"SCNd32,
                     id, &type, &subtype, &lat, &lon)){
         name.push_back(0);
         DBG("FSI(%s, %s, %#x, %#x, %"PRId32", %"PRId32") read from file",
             &(name.front()), id, type, subtype, lat, lon);
         m_items.push_back(new FullSearchItem(&(name.front()),
                  id, type, subtype, lat, lon));
      } else {
         ERR("line \"%s\" is badly formatted", line);
      }
   } else {
      ERR("Failed to read line from file");
   }
}

void isab::Faker::parsemap(FILE* f)
{
   char line[128] = {0};
   std::vector<char> first;
   std::vector<char> second;
   first.reserve(40);
   second.reserve(40);
   if(fgets(line, sizeof(line), f) && line[strlen(line) - 1] == '\n'){
      DBGP("Read line \"%s\"", line);
      char *c = line;
      if(*c++ != '\''){
         ERR("first character was '%c', expected '''", *(c-1));
         return;
      }
      for(; *c != '\'' && *c != 0; ++c){
         first.push_back(*c);
      }
      if((c > line + strlen(line) - 2) || 
         *c++ != '\'' || *c++ != ',' || *c++ != '\''){
         ERR("*c == '%c', position %u in \"%s\"", *c, c - line, line);
         return;
      }
      for(; *c != '\'' && *c != 0; ++c){
         second.push_back(*c);
      }
      first.push_back(0);
      second.push_back(0);
      DBG("Parsed item-to-regionsmapping: \"%s\"<->\"%s\"",
          &first.front(), &second.front());
      m_item2region.insert(make_pair(fakerstring(first), fakerstring(second)));
   } else {
      ERR("Failed to read line from file");
   }
}

bool isab::Faker::readSearchData(const char* filename)
{
   enum ParseMode { INVALID = 0, ADDINFO = 'A' , REGIONS = 'R', 
                    ITEMS = 'I', MAP = 'M'} parsing = INVALID;
   char* currentinfo = NULL;
   FILE* sf = fopen(filename, "r");
   if(sf){
      DBG("Reading search data from file.");
      int firstinline;
      while((firstinline = fgetc(sf)) != EOF){
         DBGP("firstinline: '%c'", firstinline);
         if(strchr("# \t\n", firstinline)){ //comment or empty
            DBGP("COMMENT OR EMPTY");
            ungetc(firstinline, sf); //put back in case of \n
            clearline(sf);
         } else if(firstinline == '\'') {
            ungetc(firstinline, sf);
            DBGP("decode data: %c", parsing);
            switch(parsing){
            case INVALID: clearline(sf);               break;
            case ADDINFO: parseinfo(&currentinfo, sf); break;
            case REGIONS: parseregion(sf);             break;
            case ITEMS:   parseitem(sf);               break;
            case MAP:     parsemap(sf);                break;
            }
         } else if(strchr("ARIM", firstinline)){
            DBG("Switch to parse mode '%c'", firstinline);
            parsing = ParseMode(firstinline);
            clearline(sf);
         } else {
            ERR("firstinline == '%c', unknown character, parser fails", 
                firstinline);
            fclose(sf);
            return false;
         }         
      }
      fclose(sf);
      delete[] currentinfo;
   }else {
      ERR("unable to open file '%s'", filename);
      return false;
   }
   return true;
}


