/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* This module is intended for various experiments. 
 * NOT FOR USE IN A PRODUCTION TARGET !!!
 */

#include "arch.h"

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "CtrlHub.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "NavPacket.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerComEnums.h"
#include "ErrorModule.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "NavServerCom.h"
#include "Destinations.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "NavTask.h"
#include "TestShell.h"

#include "Buffer.h"
#include "nav2util.h"

/* #include "RouteInfo.h" */

/* #include "../NavTask/Route.h" */
#include "../NavTask/Point.h"

#include "Nav2Error.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/MapClasses.h"
#include <functional>
#include <algorithm>

#include "GetIMEI.h"

// #define USE_UICTRL
//#ifdef USE_UICTRL
//#  include "../UiCtrl/ThinClient/include/NavCtrlGuiProt.h"
//#  include "../UiCtrl/ThinClient/include/StringObject.h"
//#  include "../UiCtrl/ThinClient/include/DataBuffer.h"
//#endif

#ifdef __SYMBIAN32__
#  include <ctype.h>
#endif

#include "LogMacros.h"
#include <assert.h>

#define TOLAT 97223144
#define TOLON 23016896

using namespace std;

namespace isab {

   ///Special subclass of buffer that doesn't zero terminate strings
   ///written with nprintf.
   class OutputBuffer : public Buffer {
   public:
      OutputBuffer(int len) : Buffer(len) {}
      OutputBuffer() : Buffer() {}
      int nprintf(size_t size, const char* format, ...);
   };

   inline int OutputBuffer::nprintf(size_t size, const char* format, ...)
   {
      va_list args;
      va_start(args, format);
      int ret = Buffer::vnprintf(size, format, args);
      va_end(args);
      m_validBytes -= 1;
      m_writePos -= 1;
      return ret;
   }

   TestShell::TestShell(SerialProviderPublic *p, Nav2Error::Nav2ErrorTable * errorTable,
         bool regressionTest)
      :  Module("TestShell"),
         m_serialProvider(p),
/*          m_route(m_log), */
         m_syncrequestID(0),
         m_regressionTest(regressionTest),
         m_shortRouteOutput(0),
         m_errorTable(errorTable)
   {
      SerialConsumerPublic *consumer;

      DBG("TestShell::TestShell\n");

      p->setOwnerModule(this);
      consumer = new SerialConsumerPublic(m_queue);
      p->connect(consumer);
      newLowerModule(p);

      m_parameterProvider = new ParameterProviderPublic(this);
      m_nscProvider = new NavServerComProviderPublic(this);
      m_dstProvider = new DestinationsProviderPublic(this);
      m_navTaskProvider = new NavTaskProviderPublic(this);

      m_mapnum = 0;
   }

   void TestShell::decodedConnectionNotify(enum ConnectionNotify state, 
                                           enum ConnectionNotifyReason /*reason*/,
                                           uint32 /*src*/)
   {
      switch(state){
      case CONNECTED:
         DBG("Connected");
         m_serialProvider->
            sendData(7, reinterpret_cast<const uint8*>("READY.\n"));
         break;
      case CONNECTING:
      case DISCONNECTING:
      case CLEAR:
      case WAITING_FOR_USER:
         break;
      }
   }

   void TestShell::decodedSolicitedError( const ErrorObj& err, 
                                          uint32 src, uint32 dst )
   {
      OutputBuffer error(128);
      if(m_regressionTest){
         error.nprintf(128,
                       "dst: %0#10x received from src: %0#10x:\n",
                       dst, src);
         error.nprintf(256,
                       "SolicitedError: %s\n",
                       m_errorTable->getErrorString( err.getErr() ) );
         error.nprintf(32, "<end of reply>\n");
      } else {
         error.nprintf(512, "dst: %0#10x received from src: %0#10x: "
                       "SolicitedError: %s\n", dst, src, 
                       m_errorTable->getErrorString( err.getErr() ) );
      }
      outputToTcp(error);
   }

   void TestShell::decodedUnsolicitedError( const ErrorObj& err, 
                                            uint32 src )
   {
      OutputBuffer error(128);
      if(m_regressionTest){
         error.nprintf(128,
                       "Received from \n");
         error.nprintf(128,
                       "src: %0#10x:\n", src);
         error.nprintf(128,
                       "UnsolicitedError %0#x: %s\n", err.getErr(),
                       m_errorTable->getErrorString( err.getErr() ) );
         error.nprintf(200, "<end of reply>\n");
      } else {
         error.nprintf(256, "Recieved from src: %0#10x: "
                       "UnsolicitedError %0#x: %s\n", src, err.getErr(), 
                       m_errorTable->getErrorString( err.getErr() ) );
      }
      outputToTcp(error);
   }


   void TestShell::decodedStartupComplete()
   {
      Module::decodedStartupComplete();
      m_rawRootPublic->manageMulticast(JOIN, Module::NSCProgressMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::NavTaskMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::ErrorMessages);
      m_rawRootPublic->manageMulticast(JOIN, Module::LatestNews);
      m_rawRootPublic->manageMulticast(JOIN, ParameterProviderPublic::paramIdToMulticastGroup(ParameterEnums::NT_UserTrafficUpdatePeriod));
   }

   void TestShell::treeIsShutdown()
   {
      delete m_parameterProvider;
      m_parameterProvider = NULL;

      delete m_nscProvider;
      m_nscProvider = NULL;

      delete m_dstProvider;
      m_dstProvider = NULL;
      
      delete m_navTaskProvider;
      m_navTaskProvider = NULL;

      Module::treeIsShutdown();
   }

   CtrlHubPublic * TestShell::rootPublic()
   {
      //   // FIXME - use dynamic_cast here? Check null result then.
      //   // FIXME - inline this!
      //   return dynamic_cast<SerialConsumerPublic *>m_rawRootPublic;

      // This is actually safe since the connect message is only sent 
      // by a directly attached public class to the module. Since we
      // created the public interface (of type SerialProviderPublic)
      // we know that that is the class that created the connect message.
      // SerialProviderPublic in turn only accepts a 
      // SerialConsumerPublic as parameter.
      return reinterpret_cast<CtrlHubPublic *>(m_rawRootPublic);
   }


   MsgBuffer * TestShell::dispatch(MsgBuffer *buf)
   {
      //DBG("TestShell::dispatch : %i\n", buf->getMsgType());
      buf = m_serialConsumerDecoder.dispatch(buf, this);
      if (buf) buf = m_ctrlHubAttachedDecoder.dispatch(buf, this);
      if (buf) buf = m_parameterConsumerDecoder.dispatch(buf, this);
      if (buf) buf = m_nscDecoder.dispatch(buf, this);
      if (buf) buf = m_serialProviderDecoder.dispatch(buf, this);
      if (buf) buf = m_dstDecoder.dispatch(buf, this);
      if (buf) buf = m_navTaskDecoder.dispatch(buf, this);
      if (buf) buf = m_errorDecoder.dispatch(buf, this);
      if (buf) buf=Module::dispatch(buf);
      return buf;
   }


   CtrlHubAttachedPublic * TestShell::newPublicCtrlHub()
   {
      DBG("newPublicCtrlHub()\n");
      return new CtrlHubAttachedPublic(m_queue);

   }


#ifndef M_PI
#define M_PI      3.14159265358979323846
#endif

static const double degreeFactor = 11930464.7111;
static const double radianFactor = degreeFactor * 180 / M_PI;
static const double invRadianFactor = 1.0 / radianFactor;


int32 mc2Lat2Nav2(int32 mc2Lat)
{
   return int32(mc2Lat * invRadianFactor * 100000000);
}

int32 mc2Lon2Nav2(int32 mc2Lon)
{
   return int32(mc2Lon * invRadianFactor * 100000000);
}


   void TestShell::decodedReceiveData(int length, const uint8 *inData, 
                                      uint32 /*src*/)
   {
      while (length) {
         --length;
         int letter = *inData++;
         switch (toupper(letter)) {
         case 'A':
            DBG("Got A");
            {
               int32 data[] = {1000, 1001, 1002, 1003, 1004};
               m_parameterProvider->setParam(ParameterEnums::TS_Param1, data, 5);
            }
            break;
         case 'B':
            DBG("Got B");
            {
               m_parameterProvider->getParam(ParameterEnums::TS_Param1);
               m_parameterProvider->getParam(ParameterEnums::TS_Param2);
               m_parameterProvider->getParam(ParameterEnums::TS_Param3);
               m_parameterProvider->getParam(ParameterEnums::TS_Param4);
            }
            break;
         case 'C':
            DBG("Got C");
            {
               float data[] = {5.31F,  6.1F,  2.41F,  83.3F};
               m_parameterProvider->setParam(ParameterEnums::TS_Param2, data, 4);
            }
            break;
         case 'D':
            DBG("Got D");
            {
               DBG("Will join 0x%08x", 
                   ParameterProviderPublic::paramIdToMulticastGroup(ParameterEnums::TS_Param2));
               // FIXME - create a paramToMulticastGroup.
               m_rawRootPublic->manageMulticast(Module::JOIN, 
                     ParameterProviderPublic::paramIdToMulticastGroup(ParameterEnums::TS_Param2));
            }
            break;
         case 'E':
            DBG("Got E -- set string ");
            {
               const char *data[] = { "Adam", "Bertil", "Caesar" };
               m_parameterProvider->setParam(ParameterEnums::TS_Param3, data, 3);
            }
            break;
         case 'F':
            DBG("Got F -- displaying favorites");
            m_dstProvider->getFavorites();
            break;
#ifdef USE_UICTRL
         case 'G':
            {
               DBG("Got G");

               // Where am I packet.
               // 
               // Serial protocol:
               // 
               // uint16 id       = 0xaa; // something easy to spot.
               // Packet* data = new Packet(WHERE_AM_I, id);
               //
               // Gui protocol:
               //int32 length = 6;
               //uint8  version = 0; // Must be 0 for the test UiCtrl 
               // to recognize this message as 
               // a Gui message.
               //uint16 packetType = (0xA0 | 0x04); // where am i.
               //Buffer* data = new Buffer(length);
               //data->writeNext8bit( version );
               //data->writeNext8bit( (length >> 16) & 0xff);
               //data->writeNext8bit( (length >> 8)  & 0xff);
               //data->writeNext8bit( (length)       & 0xff);
               //data->writeNext16bit( packetType );

               //StringObject city("Lund");
               //StringObject street("Trolle");
               wchar_t* city = new wchar_t[1024];
               wcscpy(city, L"Lund");
               wchar_t* street = new wchar_t[1024];
               wcscpy(street, L"Trolle");
               uint32 countryCode = 1; // Sweden.
               int32 citySelection = -1; // Do not use selection.
               SearchRequestMessage message( countryCode, city, 
                                             citySelection, street);
               DataBuffer buf(1024);
               message.serialize(&buf);
               Buffer* data = new Buffer(buf.getCurrentOffset());
               data->writeNextByteArray(buf.getBufferAddress(), buf.getCurrentOffset());



               //byte formatType = 0;    // only type
               //byte packetType = 0x25; // where am i

               //data->writeNext8bit(formatType);
               //data->writeNext8bit(packetType);
               //data->writeNext16bit(id);

               static_cast<SerialConsumerPublic*>(m_rawRootPublic)->receiveData(data);
               
            }
            break;
#else
         case 'G':
            {
               DBG("Got G Vector map request.");
               const char *reqstr = reinterpret_cast<const char*>(inData);
               char *end = strchr(reqstr, '\n');
               DBG("end: %#x", *end);
               inData += end - reqstr + 1;
               length -= end - reqstr + 1;
               DBG("assert(*(inData-1) == '\\n': %d", *(inData-1) == '\n');
               assert(*(inData-1) == '\n');
               *end = 0;
               DBG("vectorMapRequest");
               m_nscProvider->vectorMapRequest(reqstr);
               break;
            }
            case 'J' : {
               DBG("Got J Multi Vector map request.");
               // J0,1000,TUD3GB-UU,T+D3GB-WU
               const char* reqstr = reinterpret_cast<const char*>(inData);
               char *end = strchr( reqstr, '\n' );
               inData += end - reqstr + 1;
               length -= end - reqstr + 1;
               char* tmpPtr = NULL;
               uint32 startOffset = strtoul( reqstr, &tmpPtr, 0 );
               reqstr = tmpPtr + 1;
               uint32 maxSize = strtoul( reqstr, &tmpPtr, 0 );
               reqstr = tmpPtr + 1;
               char* data = const_cast<char*>( reqstr );
               DBG("vectorMapRequest %"PRIu32" %"PRIu32, startOffset, maxSize);
               *end = '\0';
               while ( reqstr < end ) {
                  char* tEnd = strchr( reqstr, ',' );
                  if ( tEnd == NULL ) {
                     DBG("vectorMapRequest End %s", reqstr );
                     reqstr = end;
                  } else {
                     *tEnd = '\0';
                     DBG("vectorMapRequest %s", reqstr );
                     reqstr = tEnd + 1;
                  }
               }
               uint32 dataSize = end - data + 1;
               m_nscProvider->multiVectorMapRequest( 
                  startOffset, maxSize, reinterpret_cast<const uint8*>(data), 
                  dataSize );
            } break;
#endif
         case 'H':
            DBG("Got H -- Parameter Sync");
            m_nscProvider->paramSync();
            break;
         case 'I':
            {
               DBG("Got I -- Adding 5 random favs.");
               for(int i = 0; i < 5; ++i){
                  char name[128] = {0};
                  snprintf(name, sizeof(name), "Fav%d-%#x", i, rand());
                  Favorite fav(rand(), rand(), "Fav1", "","","","");
                  m_dstProvider->addFavorite(&fav);
               }
            }
            break;
         case 'K':
            m_nscProvider->verifyLicenseKey("0CBHSDFHDKDHASFLSDHF",
                                            "+46462882236", NULL, NULL, NULL,
                                            MAX_INT32);
            break;
         case 'L':
            {
               /* Dumpa loggar här! FIXME - XXX */
            }
            break;

               
#if defined(__SYMBIAN32__) && !defined(_WIN32)
         case 'M':
            {
               OutputBuffer out;
               char* tmp = GetIMEI();
               out.nprintf(200, "Got IMEI result: \"%s\".", 
                           tmp ? tmp : "FAILED");
               outputToTcp(out);
                  
            }
            break;
#endif
         case 'N':
            {
               const char* one = "calling *(uint8*)(NULL)=0; ";
               const char* two = "Prepare to exit!";
               const char* three = "Did not exit";
               DBG("Got N -- %s", one);
               DBG("Got N -- %s", two);
               OutputBuffer out;
               out.nprintf(200, "%s\n%s\n", one, two);
               outputToTcp(out);
#if defined(__SYMBIAN32__) && defined(_MSC_VER)
               User::Leave(KErrNoMemory);
#endif
               *(volatile uint8*)(NULL)=0;
               *(volatile uint8*)(NULL)=1;
               DBG("Got N -- %s", three);
               out.clear();
               out.nprintf(200, three);
               outputToTcp(out);
            }
            break;
         case 'O':
            {
               DBG("Got O -- set traffic update period to 1 minute.");
               int32 data[] = {1};
               m_parameterProvider->setParam(ParameterEnums::NT_UserTrafficUpdatePeriod, data, 1);
               m_parameterProvider->setParam(ParameterEnums::NT_ServerTrafficUpdatePeriod, data, 1);
            }
            break;
         case 'P':
            {
               DBG("Got P -- Pda search for 'Bangat'");
               m_nscProvider->originalSearch("", "Bangat", 0, MAX_UINT32,
                                             97225505, 23016529);
            }
            break;
         case 'Q':
            DBG("Got Q (Quit - start shutdown)");
            {
               rootPublic()->startShutdownProcedure();
            }
            break;
         case 'R':
            {
               DBG("Got R -- Route Request");
               //barav. till bangatan
               uint32 lat1 = TOLAT;
               uint32 lon1 = TOLON;
               uint32 lat2 = 97244585;
               uint32 lon2 = 23022070;
               m_navTaskProvider->ntRouteToGps(lat1,lon1,lat2,lon2);
               break;
            }
         case 'U':
            {
               DBG("Got U -- German Route Request");
               //tyska coords.
               uint32 lat1 = 91363287;
               uint32 lon1 = 23737542;
               uint32 lat2 = 91240692;
               uint32 lon2 = 23844572;
               uint8 heading = 180;
               m_nscProvider->routeToGps(lat1, lon1, heading, 
                                         lat2, lon2);
            }
            break;
         case 'V':
            { 
               const char* passwd = "STORKAFINGER";
               DBG("Got W - set web password: %s", passwd);
               m_parameterProvider->setParam(ParameterEnums::UC_WebPasswd, &passwd, 1);
            }
            break;
         case '_': // input with parameters.
            {
               // search or route or whatever, with additional parameters.
               // assume all of the command and parameters are available..
               --length;
               uint8 letter = toupper(*inData++);
               char* idCopy = new char[length+1];// allocates a little too much.
               char* idCopyStart = idCopy;
               strncpy(idCopy, reinterpret_cast<const char*>(inData), length); // may copy a little too much.
               idCopy[length] = '\0';
               switch (letter) {
               case 'A':
                  {
                     DBG("Got A -- delete destination.");
                     uint32 id = strtoul(idCopy, NULL, 0);
                     m_dstProvider->removeFavorite(id);
                  }
                  break;
               case 'O':
                  {
                     char* server_ptui = strsep(&idCopy, "_%\n");
                     int32 server = strtoul(server_ptui, NULL, 0);
                     if(server != 0){
                        m_parameterProvider->setParam(ParameterEnums::NT_ServerTrafficUpdatePeriod, &server, 1);
                     }
                     int32 user = strtoul(idCopy, NULL, 0);
                     if(user != 0){
                        m_parameterProvider->setParam(ParameterEnums::NT_UserTrafficUpdatePeriod, &user, 1);
                     }
                     DBG("Got O -- set user_ptui to %"PRId32
                         " and server_ptui to %"PRId32, user, server);
                  }
                  break;
               case 'Q':
                  {
                     // exit immediately
                     exit(0);
                  } break;
               case 'S':
                  {
                     char* areaQuery = strsep(&idCopy, "½%\n");
                     char* itemQuery = strsep(&idCopy, "½%\n");
                     char* areaQueryCopy = new char[strlen(areaQuery)+1];
                     char* itemQueryCopy = new char[strlen(itemQuery)+1];
                     strcpy(areaQueryCopy, areaQuery);
                     strcpy(itemQueryCopy, itemQuery);
                     // maybe copy first!?
                     m_nscProvider->originalSearch(areaQueryCopy, 
                                                   itemQueryCopy, 0, 
                                                   MAX_UINT32, 
                                                   97225505, 23016529);
                     DBG("areaQueryCopy=%s, itemQueryCopy=%s", 
                         areaQueryCopy, itemQueryCopy);
                     delete[] areaQueryCopy;
                     delete[] itemQueryCopy;
                  } break;
               case 'R':
                  {
                     int32 latOrig = atol(strsep(&idCopy, "½%\n"));
                     int32 lonOrig = atol(strsep(&idCopy, "½%\n"));
                     int32 latDest = atol(strsep(&idCopy, "½%\n"));
                     int32 lonDest = atol(strsep(&idCopy, "½%\n"));
                     m_shortRouteOutput = atol(strsep(&idCopy, "½%\n"));

                     DBG("latOrig=%"PRIu32", lonOrig=%"PRIu32", "
                         "latDest=%"PRIu32", lonDest=%"PRIu32,
                         latOrig, lonOrig, latDest, lonDest);
                     
                     m_navTaskProvider->ntRouteToGps(latDest, lonDest,
                                                     latOrig, lonOrig);
                  } break;
               case 'M':
                  {
                     char* oLat = strsep(&idCopy, "½%\n");
                     const int32 latOrig = atol(oLat);
                     char* oLon = strsep(&idCopy, "½%\n");
                     const int32 lonOrig = atol(oLon);
                     const int32 latDest = atol(strsep(&idCopy, "½%\n"));
                     const int32 lonDest = atol(strsep(&idCopy, "½%\n"));
                     m_shortRouteOutput = atol(strsep(&idCopy, "½%\n"));

                     DBG("oLat=%s, oLon=%s", oLat, oLon);
                     DBG("latOrig=%"PRId32", lonOrig=%"PRId32", "
                         "latDest=%"PRId32", lonDest=%"PRId32,
                         latOrig, lonOrig, latDest, lonDest);
                     DBG("latOrig=%"PRId32", lonOrig=%"PRId32", "
                         "latDest=%"PRId32", lonDest=%"PRId32,
                         mc2Lat2Nav2(latOrig), mc2Lon2Nav2(lonOrig),
                         mc2Lat2Nav2(latDest), mc2Lon2Nav2(lonDest));
                     m_navTaskProvider->ntRouteToGps(mc2Lat2Nav2(latDest),
                                                     mc2Lon2Nav2(lonDest),
                                                     mc2Lat2Nav2(latOrig), 
                                                     mc2Lon2Nav2(lonOrig));

                  } break;                  
               case 'W':
                  {
                     uint32 lat = atol(strsep(&idCopy, "½\n"));
                     uint32 lon = atol(strsep(&idCopy, "½\n"));

                     DBG("lat=%"PRIu32", lon=%"PRIu32, lat, lon);
                     m_nscProvider->whereAmI(lat, lon);
                  } break;
               default:
                  letter = ((letter & 0x7f) < ' ') ? '.' : letter;
                  DBG("Unknown command %c", letter);
               };
               // calc remaining length
               inData += idCopy - idCopyStart;
               delete[] idCopyStart;
               length = 0; // xxX;
               //               length -= inData - inDataStart + 1;
            } break;
         case ' ':
            // Wait.
            // Don't do anything but reply.
            if(m_regressionTest){
               OutputBuffer txt(100);
               txt.nprintf(200, "Space command, doing nothing.\n");
               txt.nprintf(200, "<end of reply>\n");
               outputToTcp(txt);
            }
            break;
         case 'S':
            {
               DBG("Got S -- originalSearch(, Lund, Traktor, ...)");
               m_nscProvider->originalSearch("Lund", "Traktor", 0,
                                             MAX_UINT32, 
                                             97225505, 23016529);
            }
            break;
         case 'T':
            {
               DBG("Got T -- Sync destinations");
               m_syncrequestID = m_dstProvider->syncFavorites();
            }
            break;
         case 'W':
            {
               using namespace MapEnums;
               int32 lat = 97225505;
               int32 lon = 23016529;
               DBG("Got W - sending whereAmI(lat: %"PRId32", lon: %"PRId32")",
                   lat, lon);
               m_nscProvider->whereAmI(lat, lon);
               m_nscProvider->whereAmI();
               DBG("Requesting map.");
               int64 data;
               SET(data, lon, lat);
               PositionItem user(MapEnums::MapUserPositionItem, 
                                 lat, lon);
               m_nscProvider->mapRequest(BoxBox(lat + 40000, lon - 40000, 
                                                lat - 40000, lon + 40000),
                                         200, 200, 150, 150, MapEnums::GIF,
                                         1, &user);
               m_nscProvider->mapRequest(DiameterBox(lat, lon, 1200*2), 
                                         288, 190, 288, 190, MapEnums::GIF, 1, &user);
               m_nscProvider->mapRequest(DiameterBox(lat,lon, 5000), 458, 190, 458, 
                                         190, MapEnums::GIF, 0, NULL);
               m_nscProvider->mapRequest(DiameterBox(MAX_INT32, MAX_INT32, 
                                                     2*3000), 200, 200, 150, 
                                         150, MapEnums::GIF, 1, &user);

            }
            break;
         case 'X':
            m_nscProvider->cancelRequest();
            break;
         case 'Y':
            m_nscProvider->mapRequest(BoxBox(97225505, 23016529, 97211645, 
                                             23034280), 200, 200, 100, 100);
            break;
         case 'Z':
            {
               const char* server = "oss-nav.services.wayfinder.com:80";
               const char* user[] = {"iola","passwd_not_used"};
               const int32 strategy[3] = {0x03a, 60*1000, 20*1000};
               int32 navigatorid = 0x0beef;
               DBG("Got Z -- setting NSC_serverHostName to %s",server);
               m_parameterProvider->setParam(ParameterEnums::NSC_ServerHostname, 
                                             &server);
               m_parameterProvider->setParam(ParameterEnums::NSC_UserAndPasswd, user, 2);
               m_parameterProvider->setParam(ParameterEnums::NSC_NavigatorID, 
                                             &navigatorid);
               m_parameterProvider->setParam(ParameterEnums::NSC_CachedConnectionStrategy,
                                             strategy, 3);
               break;
            }
         case '\n':
         case '\r':
            break;
         default:
            {
               char c = *(inData - 1);
               c = ((c & 0x7f) < ' ') ? '.' : c;
               DBG("Unknown command %c", c);
            }
            break;
         }
      }
   }

   void TestShell::decodedParamNoValue(uint32 paramId,
                                       uint32 src,
                                       uint32 dst)
   {
      DBG("decodedParamNoValue(0x%08"PRIx32", 0x%08"PRIx32", 0x%08"PRIx32")", 
          paramId, src, dst);
   }

   void TestShell::decodedParamValue(uint32 paramId,
                                     const int32 * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst)
   {
      DBG("decodedParamValue(int32)(0x%08"PRIx32", (%"PRId32"), %"PRId32", "
          "0x%08"PRIx32", 0x%08"PRIx32")", 
          paramId, *data, numEntries, src, dst);
      for (int i=0; i<numEntries; i++) {
         DBG("    %2i:(%"PRId32"):%#"PRIx32, i, data[i], uint32(data[i]));
      }
   }

   void TestShell::decodedParamValue(uint32 paramId,
                                     const float * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst)
   {
      DBG("decodedParamValue(float)(0x%08"PRIx32", (%f), %"PRId32", "
          "0x%08"PRIx32", 0x%08"PRIx32")", 
          paramId, *data, numEntries, src, dst);
      int i;
      for (i=0; i<numEntries; i++) {
         DBG("    %2i:(%f)", i, data[i]);
      }
   }

   void TestShell::decodedParamValue(uint32 paramId,
                                     const char * const * data,
                                     int32 numEntries,
                                     uint32 src,
                                     uint32 dst)
   {
      DBG("decodedParamValue(string)(0x%08"PRIx32", (%s), %"PRId32", "
          "0x%08"PRIx32", 0x%08"PRIx32")", 
          paramId, *data, numEntries, src, dst);
      int i;
      for (i=0; i<numEntries; i++) {
         DBG("    %2i:(%s)", i, data[i]);
      }
   }

   void TestShell::decodedParamValue(uint32 paramId, const uint8* /*data*/, 
                                     int /*size*/, uint32 /*src*/, uint32 /*dst*/)
   {
      switch(paramId){
#ifdef __VC32__
      case 0: // fall through. avoids compiler warning.
#endif
      default:
         DBG("Received unknown %s parameter 0x%"PRIx32, "binary block", 
             paramId);
      }
   }

   void TestShell::decodedWhereAmIReply(const char* country, 
                                        const char* municipal, 
                                        const char* city, const char* district,
                                        const char* streetname,
                                        int32 lat, int32 lon, uint32 src, 
                                        uint32 dst)
   {
      INFO("Where Am I Reply -- results on tcp port");
      OutputBuffer txt(100);
      txt.nprintf(200, "Where Am I Reply\n"); 
      txt.nprintf(200, "country: %s\n",country); 
      txt.nprintf(200, "municipal: %s\n", municipal); 
      txt.nprintf(200, "city: %s\n", city); 
      txt.nprintf(200, "district: %s\n", district); 
      txt.nprintf(200, "streetname: %s\n", streetname); 
      txt.nprintf(200, "lat: %u\n", lat); 
      txt.nprintf(200, "lon: %u\n", lon); 
      txt.nprintf(200, "src: %#x\n", src); 
      txt.nprintf(200, "dst: %#x\n", dst); 
      if(m_regressionTest){
         txt.nprintf(200, "<end of reply>\n");
      }
      outputToTcp(txt);
   }

   void TestShell::decodedSyncDestinationsReply(std::vector<Favorite*>& /*favs*/, 
                                                std::vector<uint32>& /*removed*/,
                                                uint32 /*src*/, uint32 /*dst*/)
   {
   }

   void TestShell::decodedDestRequestOk( uint32 /*src*/, uint32 dst )
   {
      if(dst == m_syncrequestID){
         m_dstProvider->getFavorites();
         m_syncrequestID = 0;
      }
      // Remove any outstanding requests here.
   }

   namespace {
      /// A functor that outputs a textual representation of SearchAreas.
      class OutputSearchArea{
         Buffer& b;
      public:
         OutputSearchArea(Buffer& buf):b(buf){}
         typedef void result_type;
         typedef const SearchArea* argument_type;
         result_type operator()(argument_type arg)
         {
            b.nprintf(200, "SearchArea :: id %12s, name: %s\n", 
                      arg->getID(), arg->getName());
         }
      };

      /// A functor that outputs a textual represntation of a
      /// SearchRegion. The usage is a bit arcane.
      class OutputSearchRegion{
         Buffer& b;
         const SearchItem& i;
         const char* prefix;
      public:
         OutputSearchRegion(Buffer& buf, const SearchItem& si, 
                            const char* p):
            b(buf), i(si), prefix(p)
         {}
         typedef void result_type;
         typedef unsigned argument_type;
         result_type operator()(argument_type arg)
         {
            const SearchRegion& r = *(i.getRegion(arg));
            b.nprintf(256, "%sSearchRegion (%u) id: %s\n"
                      "%s                name: %s\n"
                      "%s                type: %#x\n", prefix, arg,
                      r.getId(), prefix, r.getName(), prefix, (unsigned int)(r.getType()));
         }
      };

      /// A functor that outputs a textual represntation of a
      /// AdditionalInfo. The usage is a bit arcane.
      class OutputAdditionalInfo{
         Buffer& b;
         const FullSearchItem& i;
         const char* prefix;
      public:
         OutputAdditionalInfo(Buffer& buf, const FullSearchItem& si, 
                              const char* p) :
            b(buf),i(si), prefix(p){}
         typedef void result_type;
         typedef unsigned argument_type;
         result_type operator()(argument_type arg)
         {
            const AdditionalInfo& a = *(i.getInfo(arg));
            b.nprintf(256, "%sAdditionalInfo (%u) key: %s\n"
                      "%s                  value: %s\n"
                      "%s                   type: %#x\n", prefix, arg, 
                      a.getKey(), prefix, a.getValue(), prefix, (unsigned int)(a.getType()));
         }
      };

      ///Iterator that lets POD objects be used in algorithms
      /// probably not completely implemented.
      template<class T>
      class TypeIterator{
         T q;
      public:
         TypeIterator(T w) : q(w) {}
         T operator*() { return q; };
         TypeIterator<T> operator++() 
         { 
            TypeIterator<T> ret(q); ++q; return ret;
         }
         TypeIterator<T>& operator++(int a) { ++q; return *this;}
         bool operator!=(const TypeIterator<T>& ii){return q != ii.q;}
         bool operator==(const TypeIterator<T>& ii){return q == ii.q;}
      };

      /// A functor that outputs FullSearchItems.
      class OutputFullSearchItem{
         Buffer& b;
      public:
         OutputFullSearchItem(Buffer& buf):b(buf){}
         typedef void result_type;
         typedef const FullSearchItem* argument_type;
         result_type operator()(argument_type arg)
         {
            b.nprintf(256, "FullSearchItem::\n  name:       %s\n"
                      "  id:     %s\n  distance: %u\n  type:     %#x\n"
                      "  subtype:  %#x\n", arg->getName(), arg->getID(), 
                      (unsigned int)(arg->getDistance()), arg->getType(), arg->getSubType());
            std::for_each(TypeIterator<unsigned>(0), 
                          TypeIterator<unsigned>(arg->noRegions()),
                          OutputSearchRegion(b,*arg,"  "));
            b.nprintf(256, "  lat:      %i\n  lon:      %i\n  alt:      %i\n"
                      "  flags:    %#x\n", (int)(arg->getLat()), (int)(arg->getLon()),
                      (int)(arg->getAlt()), (unsigned int)(arg->getSearchModuleStatusFlags()));
            std::for_each(TypeIterator<unsigned>(0), 
                          TypeIterator<unsigned>(arg->noAdditionalInfo()),
                          OutputAdditionalInfo(b, *arg, "  "));
         }
      };

      class ExtractId : 
         public unary_function<const FullSearchItem*, const char*> {
      public:
         result_type operator()(argument_type arg)
         {
            return arg->getID();
         }
      };

   }

   void TestShell::decodedSearchReply(unsigned nAreas, 
                                      const SearchArea* const *sa,
                                      unsigned nItems, 
                                      const FullSearchItem* const *fsi,
                                      uint16 /*begindex*/, uint16 /*total*/,
                                      uint32 /*src*/, uint32 /*dst*/)
   {
      typedef const SearchArea*const* iterator;

      OutputBuffer txt(512);
      for_each(sa, sa + nAreas, OutputSearchArea(txt));
      for_each(fsi, fsi + nItems, OutputFullSearchItem(txt));
      outputToTcp(txt);

      if(nAreas > 1){
         MatchString<SearchArea> ms("Lund", &SearchArea::getName); 
         iterator res = find_if(sa, sa + nAreas, ms);
         if(res != (sa + nAreas)){
            m_nscProvider->refinedSearch("Bara",
                                         (*res)->getID(), 0, MAX_UINT32,
                                         97225505, 23016529, 0);
         }
      }
      if(nItems > 0){
         std::vector<const char*> id(nItems, NULL);
         std::transform(fsi, fsi + nItems, id.begin(), ExtractId());
         std::transform(fsi, fsi + nItems, id.begin(), 
                        std::const_mem_fun_t<const char*, 
                        FullSearchItem>(&FullSearchItem::getID));
         m_nscProvider->requestSearchInfo(nItems, &(id.front()));
      }
   }

   void TestShell::decodedSearchInfoReply(uint16 num, const char* const* id,
                                          const unsigned* index,
                                          unsigned numInfo,
                                          const AdditionalInfo*const* info,
                                          uint32 /*src*/, uint32 /*dst*/)
   {
      OutputBuffer txt(1024);
      for(int i = 0; i < num; ++i){
         txt.nprintf(64,"%d: SearchItem id: %s\n", i, id[i]);
         int end = (i+1) < num ? index[i+1] : numInfo;
         for(int j = index[i]; j < end; ++j){
            txt.nprintf(128, "%d: %d: AI key  : '%s'\n%d: %d: AI value: '%s'\n"
                        "%d: %d: AI type : %#x\n", i, j, info[j]->getKey(), i, 
                        j, info[j]->getValue(), i, j, info[j]->getType());
         }
      }
      outputToTcp(txt);
   }


   void TestShell::decodedRouteReply(uint32 /*src*/, uint32 /*dst*/){}
   void TestShell::decodedNTRouteReply(int64 routeid,uint32 /*src*/, uint32 /*dst*/)
   {
      using namespace MapEnums;
      OutputBuffer txt(256);
      txt.nprintf(256,"Received route with id: %#010x%08x %X_%X\n", HIGH(routeid), 
                  LOW(routeid), HIGH(routeid), LOW(routeid));
      outputToTcp(txt);
		char ROUTE[64] = {0};
      sprintf(ROUTE, "%#010x%08x", HIGH(routeid), LOW(routeid));
//       m_nscProvider->messageRequest(GuiProtEnums::non_HTML_email, 
//                                     GuiProtEnums::RouteMessage,
//                                     "ola@wayfinder.biz", "ola@wayfinder.biz",
//                                     "-- \nOla", ROUTE);
      RouteItem routeItem(routeid);
      m_nscProvider->mapRequest(RouteBox(routeid), 300, 300, 200, 200, MapEnums::GIF, 1,
                                &routeItem);
      m_nscProvider->mapRequest(RouteBox(routeid), 288, 190, 188, 190, MapEnums::GIF, 1, 
                                &routeItem);
      m_nscProvider->mapRequest(RouteBox(routeid), 458, 190, 458, 190, MapEnums::GIF, 1, 
                                &routeItem);
      m_nscProvider->mapRequest(RouteBox(routeid), 176, 144, 176, 144, MapEnums::GIF, 1, 
                                &routeItem);
      m_nscProvider->mapRequest(RouteBox(routeid), 176, 208, 176, 208, MapEnums::GIF, 1,
                                &routeItem);
   }

   void TestShell::decodedBinaryDownload(const uint8* /*data*/, size_t /*length*/, 
                                         uint32 src, uint32 dst)
   {
      INFO("Binary Download ");
      INFO("Src: 0x%"PRIx32, src);
      INFO("Dst: 0x%"PRIx32, dst);
      //      INFODUMP("data: ", data, length);
   }

   void TestShell::decodedBinaryUploadReply(uint32 /*src*/, uint32 /*dst*/){}


   const char* comStatus(ComStatus status)
   {
      switch(status){
      case invalid:            return "invalid";
      case connecting:         return "connecting";
      case connected:          return "connected";
      case sendingData:        return "sendingData";
      case downloadingData:    return "downloadingData";
      case done:               return "done";
      case disconnectionError: return "disconnectionError";
      case unknownError:       return "unknownError";
      case connectionTimedOut: return "connectionTimedOut";
      case numberOfStatuses:   return "numberOfStatuses";
      }
      return "???";
   }

   void TestShell::decodedProgressMessage(ComStatus status, 
                                          GuiProtEnums::ServerActionType type, 
                                          uint32 done, uint32 of,
                                          uint32 /*dst*/, uint32 /*src*/)
   {
      //DBG("Received progressMessage");
      int maxlen = strlen(" disconnectionError"); // spaces wanted and added by tomas for regressiontest.
      OutputBuffer output(70);
      output.nprintf(70, "%*s : %0#4x : %u/%u\n", maxlen, comStatus(status), 
                     type, done, of);
      outputToTcp(output);
   }

   void TestShell::outputToTcp(OutputBuffer& txt)
   {
      m_serialProvider->sendData(txt.getLength(), txt.accessRawData(0));
   }

   
   void TestShell::decodedSendData(int length, const uint8 *data, uint32 src)
   {
      DBG("TestShell::decodedSendData(%i, %p)\n", length, data);
      //      if(m_tcpThread){
      //         m_tcpThread->write(data, ssize_t(length));
      //      int bufferSize = 1024;
      //      if (length > bufferSize){
      //         length = bufferSize;
      //      }
      //      OutputBuffer output(bufferSize);
      //      output.writeNextByteArray(data, length);
      OutputBuffer output;
      output.nprintf(200,"decodedSendData(%d, %p, %#0x)\n", length, data, src);
      outputToTcp(output);
      //      } else {
      //         WARN("Cannot send data while disconnected!");
      //      }
   }

   SerialProviderPublic * 
   TestShell::newPublicSerial()
   {
      DBG("newPublicSerial()\n");
      SerialProviderPublic* spp = new SerialProviderPublic(m_queue);
      return spp;
   }

   void TestShell::decodedGetFavoritesReply(vector<GuiFavorite*>& guiFavorites, 
                                            uint32 /*src*/, uint32 /*dst*/ )
   {
      OutputBuffer txt(guiFavorites.size() * 60);
      std::vector<GuiFavorite*>::iterator q;
      for(q = guiFavorites.begin(); q != guiFavorites.end(); ++q){
         GuiFavorite* gfav = *q;
         //       Favorite* fav = gfav->getFavorite();
         txt.nprintf(300,">>>>\nID: %0#10x\nName: %s\n",
                     gfav->getID(), gfav->getName());
      }
      outputToTcp(txt);
      m_dstProvider->getFavoritesAllData();
      OutputBuffer txt2(100);
      txt2.nprintf(200, "<end of reply>\n");
      outputToTcp(txt2);
   }

   void TestShell::decodedGetFavoritesAllDataReply(vector<Favorite*>& favorites,
                                                   uint32 /*src*/, uint32 /*dst*/  )
   {
      OutputBuffer txt(favorites.size() * 60);
      std::vector<Favorite*>::iterator q;
      for(q = favorites.begin(); q != favorites.end(); ++q){
         Favorite* fav = *q;
         GuiFavorite* gfav = fav->getGuiFavorite();
         txt.nprintf(300,">>>>\nID: %0#10x\nName: %s\nID: %0#10x\n"
                     "lat: %d\nlon: %d\nname: %s\nshortname: %s\n"
                     "description: %s\ncategory: %s\niconname: %s\n"
                     "synced: %s\n", gfav->getID(), gfav->getName(),
                     fav->getID(), fav->getLat(), fav->getLon(), 
                     fav->getName(), fav->getShortName(),
                     fav->getDescription(), fav->getCategory(), 
                     fav->getMapIconName(), fav->isSynced() ? "yes": "no");
         delete gfav;
      }
      outputToTcp(txt);
   }


   void TestShell::decodedGetFavoriteInfoReply( Favorite* /*favorite*/, 
                                                uint32 /*src*/, uint32 /*dst*/  )
   {
   }

   void TestShell::decodedFavoritesChanged(  uint32 /*src*/, uint32 /*dst*/  )
   {
   }

   void TestShell::Point__printAsString(Point& self, OutputBuffer &txt)
   {
      txt.nprintf(300, "<point>\n");
      txt.nprintf(300, "isTrackPoint: %"PRIu8",\n",
                  self.isTrackPoint());
      txt.nprintf(300, "isWayPoint: %"PRIu8", \n",
                  self.isWaypoint());
      txt.nprintf(300, "action: %"PRIu16", \n",
                  self.getWptAction());
      txt.nprintf(300, "speedLimit: %"PRIu8", \n",
                  self.getSpeedLimit());
      txt.nprintf(300, "pointId: %"PRIu16", \n",
                  self.getPointId());
      txt.nprintf(300, "getX: %"PRIu16", \n",
                  self.getX());
      txt.nprintf(300, "getY: %"PRIu16", \n",
                  self.getY());
      txt.nprintf(300, "getScaleX: %f, \n",
                  self.getScaleX());
      txt.nprintf(300, "isMiniPoint: %"PRIu8", \n",
                  self.isMiniPoint());
      txt.nprintf(300, "isMicroPoint: %"PRIu8", \n",
                  self.isMicroPoint());
      //if (self.getMeters() != MAX_UINT16) {
      //   txt.nprintf(300,
      //               "meters: %"PRIu16", \n",
      //               self.getMeters());
      //}
      if (self.isWaypoint()) {
         txt.nprintf(300,
                     "text: %s, \n",
                     self.getText());
      }
      txt.nprintf(300, "</point>\n");
   }

   void TestShell::decodedInvalidateRoute(bool newRouteAvailable,
         int64 routeid,
                                          int32 tLat, int32 lLon, 
                                          int32 bLat, int32 rLon,
         int32 oLat, int32 oLon, int32 dLat, int32 dLon,
         uint32 /*src*/)
   {
      OutputBuffer txt(512);
      if(newRouteAvailable){
         txt.nprintf(500, 
                     "New Route: %#x%x\n"
                     "Box:       (%d,%d),(%d,%d)\n"
                     "Origin:    (%d,%d)\n"
                     "Dest:      (%d,%d)\n", HIGH(routeid), LOW(routeid), 
                     tLat, lLon, bLat, rLon, oLat, oLon, dLat, dLon);
      } else {
         txt.nprintf(500,"Route invalidated, no new route\n");
      }
      outputToTcp(txt);
   }

   void TestShell::decodedMapReply(const BoxBox& bb, uint32 realWorldWidth,
                                   uint32 realWorldHeight, uint16 imageWidth,
                                   uint16 imageHeight,
                                   enum MapEnums::ImageFormat imageType, 
                                   uint32 imageBufferSize, 
                                   const uint8* imageBuffer, uint32 /*dst*/)
   {
      OutputBuffer txt(600);
      txt.nprintf(400, "[%d, %d] [%d, %d]\nimagetype: %d\n"
                  "bufsize: %u\nbuf: %p\n", 
                  bb.getTopLat(), bb.getLeftLon(), bb.getBottomLat(), 
                  bb.getRightLon(), imageType, imageBufferSize, imageBuffer);
      outputToTcp(txt);
#ifndef __SYMBIAN32__
      //                 0123456789
      char filename[] = "mapimageXX.gif";
      char datafile[] = "mapdataXX.txt";
      datafile[7] = filename[8] = '0' + (m_mapnum / 10);
      datafile[8] = filename[9] = '0' + m_mapnum - ((m_mapnum / 10) * 10);
      m_mapnum = (m_mapnum + 1) % 100;
      FILE* file = fopen(filename, "wb");
      if(file){
         fwrite(imageBuffer, 1, imageBufferSize, file);
         fclose(file);
      } else {
         txt.clear();
         txt.nprintf(100, "Failed to create file %s\n", filename);
         outputToTcp(txt);
      }
      if((file = fopen(datafile, "w"))){
         fprintf(file, "TopLat: %d\nLeftLon: %d\nBottomLat: %d\nRightLon: %d\n"
                 "RealWorldWidth: %u m\nRealWorldHeight %u m\n"
                 "ImageWidth: %u pixels\nImagwHeight: %u pixels\n", 
                 bb.getTopLat(), bb.getLeftLon(), bb.getBottomLat(), 
                 bb.getRightLon(), realWorldWidth, realWorldHeight, 
                 imageWidth, imageHeight);
         fclose(file);
      } else {
         txt.clear();
         txt.nprintf(100, "Failed to create file %s\n", filename);
         outputToTcp(txt);
      }
#else
      realWorldWidth = realWorldWidth; // To remove warnings.
      realWorldHeight = realWorldHeight;
      realWorldWidth = imageWidth;
      realWorldHeight = imageHeight;
#endif
   }

   void TestShell::decodedVectorMapReply(const char* request, uint32 size,
                                         const uint8* data, uint32 /*dst*/)
   {
      DBG("decodedVectorMapReply, request: '%s', size: %"PRIu32"(%#"PRIx32")", 
          request, size, size);
      DBGDUMP("vectormapdata", data, size);
      const char* tag = "VMD";
      OutputBuffer txt(size + strlen(tag) + strlen(request) + 15);
      txt.nprintf(64,"%s:%s:%d:",tag,request,size);;
      for(unsigned i = 0; i < size; ++i){
         txt.nprintf(3,"%02x",data[i]);
      }
      txt.nprintf(2,"\n");
      outputToTcp(txt);
   }

   void TestShell::decodedMulitVectorMapReply(uint32 size,
                                              const uint8* data, uint32 /*dst*/)
   {
      DBG("decodedMultiVectorMapReply, size: %"PRIu32"(%#"PRIX32")",size,size);
      const char* tag = "MVMD";
      OutputBuffer txt(size + strlen(tag) + 15);
      txt.nprintf(64,"%s:%d:",tag,size);;
      for(unsigned i = 0; i < size; ++i){
         txt.nprintf(3,"%02x",data[i]);
      }
      txt.nprintf(2,"\n");
      outputToTcp(txt);      
   }


   void TestShell::decodedForceFeedMuliVectorMapReply(
      uint32 size, const uint8* data, uint32 dst )
   {
      DBG( "decodedForceFeedMultiVectorMapReply, size: %"PRIu32"(%#"PRIx32")", 
           size, size );
      decodedMulitVectorMapReply( size, data, dst );
   }

   void TestShell::decodedLatestNews(uint32 checksum, const uint8* data, 
                                     unsigned length, uint32 /*src*/, 
                                     uint32 /*dst*/)
   {
      DBG("decodedLatestNews");
#ifdef __linux
      char filename[] = "latestnews.gif";
#endif
      OutputBuffer txt(600);
      txt.nprintf(400, "New LatestNews image.\n  Checksum: %#x\n  Size    : "
                  "%u bytes\n"
#ifdef __linux
                  "  Image saved to %s\n"
#endif
                  "", checksum, data
#ifdef __linux
                  , filename
#endif
                  );
      outputToTcp(txt);
#ifdef __linux
      FILE* fd = fopen(filename, "wb");
      if(fd){
         fwrite(data, 1, length, fd);
         fclose(fd);
      }
#else 
      length = length; // To remove warnings.
#endif
   }

#define ISOK(boolean) (boolean?"Ok":"not Ok")

   void TestShell::decodedLicenseReply(bool keyok, bool phoneok, bool regionok,
                                       bool nameok, bool emailok, 
                                       GuiProtEnums::WayfinderType type,
                                       uint32 /*src*/, uint32 /*dst*/)
   {
      DBG("decodedLicenseReply");
      OutputBuffer txt(400);
      txt.nprintf(400, "LicenseReply:\n Key: %s\n Phone: %s\n Region: %s\n"
                  " Name: %s\n Email: %s\n"
                  " WayfinderType: %d\n", ISOK(keyok), ISOK(phoneok), 
                  ISOK(regionok), ISOK(nameok), ISOK(emailok), int(type));
      outputToTcp(txt);
   }


} /* namespace isab */

