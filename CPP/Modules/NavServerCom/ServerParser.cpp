/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "ErrorModule.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "Serial.h"
#include "CtrlHub.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"
#include "RequestList.h"
#include "NavServerComInternal.h"

#include "NavHandler.h"
#include "ServerParser.h"

#include "NParamBlock.h"
#include "GlobalData.h"
#include <ctype.h>

#include "LogMacros.h"
#include "TraceMacros.h"
#include "nav2util.h"

#include "NavServerComProtoVer.h"

#define NAV_SERVER_INFO_REQ_VER 2

using namespace std;

namespace isab{
   ServerParser::ServerParser(SerialProviderPublic* spp, 
                              NavServerCom* nsc, unsigned freq) : 
      m_serialProvider(spp), m_navServerCom(nsc),
      m_parseState(find_stx), totLen(0),
      req_id(0), m_assembleBuf(NULL), m_freq(freq), m_xorString(XORSTRING), 
      m_xorPos(NULL), m_protoVer(NSC_PROTO_VER)
   {
      m_log = new Log("ServerParser");
      m_tracking = NULL;
      m_navHandler = new NavHandler( NSC_PROTO_VER );
   }


   ServerParser::~ServerParser()
   {
      delete m_log;
      delete m_navHandler;
      delete m_assembleBuf;
   }

   using namespace NavServerComEnums;
   void ServerParser::pollServer(uint8 reqID, 
                                 uint32 sequenceNo)
   {
      DBG("Polling server with sequenceNo %"PRIu32" and request id 0x%"PRIx8, 
          sequenceNo, reqID);
      // Reply with pollserverfinal buffer
      Buffer poll( 52 );
      writeHeader( poll, NAV_SERVER_POLL_SERVER_REPLY, reqID );
      // Status 0x82 POLL_SERVER_FINAL
      poll.writeNext8bit( 0x82 );
      writeLength( poll );
      writeCRC( poll );
      xorCode( poll, NAV_SERVER_COM_REQUEST_HEADER_SIZE + 1 );
      uint8 protover;
      uint32 crc;
      uint32 length;
      decodeHeader( poll, length, protover, m_type, req_id, 
                    crc, m_status );
      DBG( " pollServer len %"PRIu32" pos %u", 
           poll.getLength(), poll.getReadPos() );
      ErrorObj err( Nav2Error::DUMMY_ERROR, "", "" );
      m_navServerCom->decodeBuffer(
         &poll, reqID, NAV_SERVER_POLL_SERVER_REPLY, 0x82, err, 0 ); 
   }

   uint32  ServerParser::sendParameters(uint8 reqID, 
                                        const Buffer* clientParams,
                                        bool needReply,
                                        bool secondaryServer,
                                        bool sendUnaccompanied )
   {
      DBG("Parameter Packet sent with request id 0x%"PRIx8, reqID);
      m_navHandler->handleParameters( const_cast<Buffer&>( *clientParams ),
                                      &secondaryServer );
      int res = 0;

      //      Set m_parametersSent. Is unset when a reply is received
      //      and a session-key can be used to verify the session (needs
      //      to send less info about client, saves bytes)
      m_parametersSent = true;
      

      // If we have changed license send a ChangedLicense-request
      // Else if we don't have user[ID|Name|login] send a whoAmI-request
      bool haveLicense = false;
      bool haveAuthParams = m_navHandler->haveAuthParams( haveLicense );
      if ( sendUnaccompanied || m_navHandler->isWayfinderIdStartUp() ) {
         // Don't send might be expired before upgrade, sends server-info
         // on upgrade reply
      } else if ( !haveAuthParams ) {
         if ( haveLicense ) {
            // Send a whoAmI-request
            NavRequestPacket* pack = new NavRequestPacket( 
               m_protoVer, navRequestType::NAV_WHOAMI_REQ, reqID, 1 );
            // Add the ones we have (License + clientType at least)
            m_navHandler->addAuthParams( pack->getParamBlock() );
            res = sendPacket( pack );
            delete pack;
         } else {
            ERR( "ServerParser no userID and no License key, no way to"
                 "authenticate with server." );
         }
      } else if ( m_navHandler->haveChangedLicense() ) {
         // Send changed license req with as much data as possible.
         if ( haveLicense ) {
            // Send a ChangedLicense-request
            NavRequestPacket* pack = new NavRequestPacket( 
               m_protoVer, navRequestType::NAV_CHANGED_LICENSE_REQ, reqID, 1 );
            // Add the ones we have (License + clientType at least)
            m_navHandler->addAuthParams( pack->getParamBlock() );
            // And the changed license
            m_navHandler->addChangedLicense( pack->getParamBlock() );
            res = sendPacket( pack );
            delete pack;
         } else {
            ERR( "No  License key to authenticate with and changed "
                 "license?" );
         }
      }

      DBG("Check if we need to sync server info. "
          "haveAuthParams: %d, upgrade: %d", 
          int(haveAuthParams), int(sendUnaccompanied));
      if ( ( haveAuthParams || 
             (haveLicense && m_navHandler->isWayfinderIdStartUp()) ) 
           && !sendUnaccompanied ) {
         if ( needReply || m_navHandler->needParameterSync() ) { 
            DBG( "Serverinfo sent with request id 0x%02"PRIx8, reqID );
            // send ServerInfo. Paramsync needs a param-reply.
            NavRequestPacket pack( m_protoVer, 
                                   navRequestType::NAV_SERVER_INFO_REQ,
                                   reqID, NAV_SERVER_INFO_REQ_VER );
            // Needs client type etc.
            m_navHandler->addAuthParams( pack.getParamBlock() );
            res = sendPacket( &pack );
         } else {
            DBG("We will not send a NAV_SERVER_INFO_REQ message");
         }
      } // End if haveAuthParams

      return res;
   }

   uint32 ServerParser::sendContents(uint8 reqID, 
                                     uint16 reqType,
                                   const Buffer* content)
   {
      DBG("Packet sent with request id 0x%"PRIx8, reqID);
      NavRequestPacket* pack = m_navHandler->handleRequest(
         m_protoVer, reqType, reqID, const_cast< Buffer* > ( content ) );
      if ( m_parametersSent || m_navHandler->getSessionKey() == NULL ||
           reqType == NAV_SERVER_UPGRADE_REQ ) 
      {
         m_navHandler->addAuthParams( pack->getParamBlock() );
      } else {
         // Else just add sessionKey (m_navHandler->addSessionKey( ))
         m_navHandler->addSessionParams( pack->getParamBlock() );
      }
      int res = sendPacket( pack );
      delete pack;
      return res;
   }

   void ServerParser::decodeHeader(Buffer& data, uint32& length, 
                                   uint8& protover, uint16& type, 
                                   uint8& req_id, uint32& crc, uint8& status
                                   )
   {
      data.readNext8bit(); // skip STX byte
      length     = data.readNextUnaligned32bit();
      protover   = data.readNext8bit();
      type       = data.readNextUnaligned16bit();
      req_id     = data.readNext8bit();
      crc        = data.readNextUnaligned32bit();
      // Packet data (included in all packets, decoded here)
      status    = data.readNext8bit();
      DBG("Length:%"PRIu32", ProtoVer:%"PRIu8", Type:0x%"PRIx16", "
          "ReqID:0x%"PRIx8", CRC:%"PRIu32",  Status:%"PRIu8, 
          length, protover, type, req_id, crc, status);
   }
      
   void ServerParser::decodeHeaderA(Buffer& data, uint32& length, 
                                    uint8& protover, uint16& type, 
                                    uint8& req_id, uint8& req_ver, 
                                    uint8& status )
   {
      data.readNext8bit(); // skip STX byte
      length     = data.readNextUnaligned32bit();
      protover   = data.readNext8bit();
      type       = data.readNextUnaligned16bit();
      req_id     = data.readNext8bit();
      req_ver    = data.readNext8bit();
      status     = data.readNext8bit();
      DBG("Length:%"PRIu32", ProtoVer:%"PRIu8", Type:0x%"PRIx16", "
          "ReqID:0x%"PRIx8", ReqV:%"PRIu8",  Status:%"PRIu8,
          length, protover, type, req_id, req_ver, status);
   }

   void ServerParser::flush()
   {
      delete m_assembleBuf;
      m_assembleBuf = NULL;
      m_parseState = find_stx;
   }

   void ServerParser::decodeServerPacket( const uint8* data, size_t len ) {
      //TRACE_DBG( "Got %d", len );
      int inlen = len;
      DBG("%sBuffer Exists.", m_assembleBuf ? "" : "No ");
      if(!data){
         ERR("No input data to decodeServerPacket");
         return;
      }

      //DBGDUMP("decodeServerPacket recieved this:", data, len);
      while (inlen--) {
         switch (m_parseState) {
         case find_stx:
            DBG("find_stx");
            /* This is the start of a new route/search packet. */
            if (*data == 0x02) {
               DBG("NSC received buf found STX!\n");
               
               /* Try to allocate a new buffer for reply. */
               // XXX: What if we already have a buffer? delete it?
               //      We never has a buffer in find_stx state. NULL it is.
               m_assembleBuf = new Buffer(32);
               DBG("NSC: got buffer %p\n", m_assembleBuf);
               totLen = 0;
               m_parseState = length;
               
               /* We will ignore the STX later. */
               m_assembleBuf->writeNext8bit(*data);
            } else {
               /* We will ignore any data between the end of */
               /* previous packet and a STX. */
               DBG("Ignore CH 0x%02x\n", *data);
            }
            break;
         case length:
            /* Write each byte to the buffer. */
            m_assembleBuf->writeNext8bit(*data);
            if(m_assembleBuf->getLength() == 5){
               m_assembleBuf->setReadPos(1);
               totLen = m_assembleBuf->readNextUnaligned32bit() - 5;//5 read
               m_packetLength = totLen;
               DBG("Packet Length %"PRIu32, totLen);
               m_assembleBuf->setReadPos(0);
               /* Check header if we should allocate larger buffer. */
               if ( m_packetLength > (NAV_SERVER_COM_MAX_PACKET_SIZE - 5) )
               {
                  /* Too much! */
                  // This is a really large size, 35000 bytes or
                  // so. If a packet reports a larger length than
                  // that something is seriously wrong, probably we
                  // are out of sync with the server.
                  ERR( "Reply packet size too large (%"PRId32")! - discarding",
                       m_packetLength );
                  m_parseState = find_stx;
                  delete m_assembleBuf;
                  m_assembleBuf = NULL;
                  break;
               }
               m_assembleBuf->reserve(totLen + 10);
               m_parseState = header;
            }
            break;
            case header: {
            m_assembleBuf->writeNext8bit(*data);
            totLen--;
            const uint32 constHeaderSize = 10;
            if ( m_assembleBuf->getLength() == constHeaderSize ) {
               uint8 protover;
               uint8 req_ver;
               uint32 length;
               DBG("NSC: Got header:\n");
               /* Extract header information. */
               decodeHeaderA(*m_assembleBuf, length, protover, m_type, req_id,
                             req_ver, m_status);
               m_xorPos = reinterpret_cast<const uint8*>(m_xorString);
               m_parseState = save_data;
               if (totLen == 0) {
                  /* Packet was only header and m_status. */
                  goto correct_packet_gotten;
               }
            } else {
               if (totLen == 0) {
                  // BAD. We've not even read the whole header and the
                  // data is all used up.
                  //XXX Why? Just wait for more data!
                  DBG("NSC: all bytes used and not all of header,"
                      " should wait for more but skipping packet.");
                  m_parseState = find_stx;
                  delete m_assembleBuf;
                  m_assembleBuf = NULL;
               }
            }
            }break;
         case send_chunks:
            m_assembleBuf->writeNext8bit(*data ^ *m_xorPos++);
            if(--totLen == 0){
               m_navServerCom->sendChunk(req_id, m_assembleBuf, true);
               delete m_assembleBuf;
               m_assembleBuf = NULL;
               m_parseState = find_stx;
            }
            break;
         case save_data:
            m_assembleBuf->writeNext8bit(*data ^ *m_xorPos++);
            if (--totLen == 0) {
            correct_packet_gotten:
               DBG("correct_packet_gotten, totlen == 0");
               m_parseState = find_stx;
               uint8 protoVer =   m_assembleBuf->accessRawData( 6 ) [ 0 ];
               uint8 requestVersion = m_assembleBuf->accessRawData( 9 ) [ 0 ];
               uint8 statusCode = m_assembleBuf->accessRawData( 10 ) [ 0 ];
               const char* statusMessage = reinterpret_cast<const char*> (
                  m_assembleBuf->accessRawData( 11 ) );
               uint32 statusMessageLen = strlen( statusMessage );
               uint32 headerLen = 11 + statusMessageLen + 1;
               // The last 4 are crc
               m_assembleBuf->setReadPos( m_assembleBuf->getLength() - 4 );
               uint32 crc = m_assembleBuf->readNextUnaligned32bit();
#ifdef CHECK_NAV_CRC
               uint32 realCrc = m_navHandler->crc32( 
                  m_assembleBuf->accessRawData( 0 ),
                  m_assembleBuf->getLength() -4 );
               if ( crc != realCrc ) {
                  ERR( "Reply crc error! Got %X Expected %X", 
                       crc, realCrc );
                  m_parseState = find_stx;
                  delete m_assembleBuf;
                  m_assembleBuf = NULL;
                  break;
               }
#endif
               DBG( "Reply " );
               class NavReplyPacket* reply = m_navHandler->handleReply( 
                  m_protoVer, m_type, req_id, requestVersion, statusCode,
                  statusMessage, m_assembleBuf->accessRawData( headerLen ),
                  m_assembleBuf->getLength() - headerLen - 4 );

               //reply->getParamBlock().dump( m_log, false );

               
               m_navServerCom->reportProgress(downloadingData,
                                              MessageType(m_type), 
                                              m_packetLength, m_packetLength);
               //TRACE_DBG( "Got reply %d size %"PRIu32, m_type, m_packetLength );

               // We have communication with server
               m_parametersSent = false;
               // Check if sessionKey is present then set it
               if ( statusCode == NavReplyPacket::NAV_STATUS_OK &&
                    reply->getParamBlock().getParam( 14 ) != NULL ) 
               {
                  m_navHandler->setSessionKey( 
                     reply->getParamBlock().getParam( 14 )->getString() );
               }
               // Check here for 15 - Upload file here?
               if ( reply->getParamBlock().getParam( 15 ) != NULL ) {
                  handleServerUpload( reply );
                  // Get the ',' separated list files and upload them
                  // fopen(  "rb" ); fread( );
               }

               uint32 extendedError = 0;
               const char* extendedErrorString = "";
               if ( reply->getParamBlock().getParam( 24 ) != NULL ) {
                  extendedError = reply->getParamBlock().getParam( 24 )->
                     getUint32();
               } 
               if ( reply->getParamBlock().getParam( 26 ) != NULL ){
                  extendedErrorString = reply->getParamBlock().
                     getParam( 26 )->getString();
               }
               //TRACE_DBG( "Got %d extendederror %d and %s status %s", statusCode, extendedError, extendedErrorString, statusMessage );
               ErrorObj err( Nav2Error::DUMMY_ERROR, extendedErrorString,
                             statusMessage );


               // Make 0x8 buffer
               if ( statusCode == NavReplyPacket::NAV_STATUS_EXPIRED_USER
                    || statusCode == 
                    NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER ||
                    statusCode == 
                    NavReplyPacket::NAV_STATUS_REDIRECT )
               {
                  // If problem report it
                  // XXX: Only unsolicited if server_info?
                  req_id = MAX_UINT8; // Make it unsolicited
                  m_type = NAV_SERVER_PARAMETER_REPLY;
                  if ( statusCode == 
                       NavReplyPacket::NAV_STATUS_EXPIRED_USER ) 
                  {
                     statusCode = PARAM_REPLY_EXPIRED_USER;
                  } else if ( statusCode == 
                              NavReplyPacket::NAV_STATUS_UNAUTHORIZED_USER)
                  {
                     statusCode = PARAM_REPLY_UNAUTHORIZED_USER;
                  } else if ( statusCode == 
                              NavReplyPacket::NAV_STATUS_REDIRECT )
                  {
                     statusCode = PARAM_REPLY_REDIRECT;
                  }
               }
               if ( m_type == NAV_SERVER_UPGRADE_REPLY && statusCode == 
                    NavReplyPacket::UPGRADE_MUST_CHOOSE_REGION ) {
                   statusCode = UPGRADE_REPLY_MUST_CHOOSE_REGION;
               }

               Buffer* eightB = new Buffer( m_packetLength*2 );
               writeHeader( *eightB, m_type/*Yiekes!*/, req_id );
               eightB->writeNext8bit( statusCode );
               m_navHandler->makeReplyBuffer( 
                  *eightB, reply, requestVersion, statusCode, req_id, m_type,
                  protoVer );
               uint8 protoverToDump;
               uint32 length;
               decodeHeader( *eightB, length, protoverToDump, m_type,
                             req_id, crc, m_status );

               if ( m_type == NAV_SERVER_ROUTE_REPLY ) {
                  if ( statusCode == NAV_STATUS_OK ) {
                     // No packet header in route chunk
                     uint32 size = eightB->getLength() - 
                        NAV_SERVER_COM_HEADER_SIZE;
                     Buffer chunk( const_cast<uint8*> (
                                      eightB->accessRawData(
                                         NAV_SERVER_COM_HEADER_SIZE ) ),
                                   size, size );
                     uint32 ptui = MAX_UINT32;
                     if(const class NParam* ptuiParam = 
                        reply->getParamBlock().getParam( 1107 )){
                        ptui = ptuiParam->getUint32();
                        DBG("PTUI param: %"PRIu32, ptui);
                     } else {
                        DBG("No PTUI parameter");
                     }
                     m_navServerCom->newRoute( req_id, size, ptui );
                     m_navServerCom->sendChunk( req_id, &chunk, true );
                     chunk.releaseData();
                     // Check if TileMap data after last route chunk so
                     // route has a request to match
                     // This mapdata goes as unsolicited
                     if ( reply->getParamBlock().getParam( 4700 ) != NULL )
                     {
                        Buffer* tB = new Buffer( m_packetLength*2 );
                        writeHeader( 
                           *tB, NAV_SERVER_MULTI_VECTOR_MAP_REPLY, req_id);
                        tB->writeNext8bit( statusCode );
                        m_navHandler->makeReplyBuffer( 
                           *tB, reply, requestVersion, statusCode, req_id,
                           NAV_SERVER_MULTI_VECTOR_MAP_REPLY, protoVer );
                        uint8 protover;
                        uint32 length;
                        decodeHeader( *tB, length, protover, m_type, 
                                      req_id, crc, m_status );
                        m_navServerCom->decodeBuffer( 
                           tB, req_id, NAV_SERVER_MULTI_VECTOR_MAP_REPLY,
                           statusCode, err, extendedError );
                     } // End if MultiVectorMap data in route reply
                  } else if (statusCode == ROUTE_REPLY_NO_ROUTE_CHANGE){
                     DBG("ROUTE_REPLY_NO_ROUTE_CHANGE");
                     //keep the current route, special case. 
                     int64 routeId64 = 0;
                     uint32 ptui = MAX_UINT32;
                     if(const class NParam* rid = 
                        reply->getParamBlock().getParam( 1100 ) ) {
                        uint32 routeID = 0;
                        uint32 routeCTime = 0;
                        sscanf( rid->getString(), "%"SCNx32"_%"SCNx32, 
                                &routeID, &routeCTime );
                        DBG("Keep route %#"PRIx32"%08"PRIx32, 
                            routeID, routeCTime);
                        routeId64 = MakeInt64( routeID, routeCTime );
                     }
                     if(const class NParam* ptuiparam = 
                        reply->getParamBlock().getParam( 1107 ) ) {
                        ptui = ptuiparam->getUint32();
                        DBG("Server PTUI %"PRIu32, ptui);
                     } else {
                        DBG("No PTUI parameter");
                     }
                     m_navServerCom->keepRoute(req_id, routeId64, ptui);
                  } else {
                  // If not ok send in decodeBuffer
                     m_navServerCom->decodeBuffer( 
                        eightB, req_id, m_type, statusCode, 
                        err, extendedError );
                  }



               } else if ( m_type == navRequestType::NAV_WHOAMI_REPLY ) {
                  // Reply with info about user
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_SERVER_PARAMETER_REPLY, 
                     statusCode, err, extendedError );
               } else if ( m_type == navRequestType::NAV_SERVER_INFO_REPLY ) {
                  // Send the needed "poll" packets here
                  vector<NavRequestPacket*> polls;
                  m_navHandler->makePollPackets( reply, polls );
                  for ( uint32 i = 0 ; i < polls.size() ; ++i ) {
                     DBG("Added POLL packet");
                     m_navServerCom->addNrpRequest(polls[ i ]);
                  }
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_SERVER_PARAMETER_REPLY, 
                     statusCode, err, extendedError );
               } else if ( m_type == navRequestType::NAV_CALLCENTER_LIST_REPLY||
                           m_type == navRequestType::NAV_SERVER_LIST_REPLY ||
                           m_type == navRequestType::NAV_NEW_PASSWORD_REPLY ||
                           m_type == navRequestType::NAV_SERVER_AUTH_BOB_REPLY )
               { // Send as param-reply
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_SERVER_PARAMETER_REPLY,
                     statusCode, err, extendedError );
               } else if ( m_type == navRequestType::NAV_TOP_REGION_REPLY  ||
                           m_type == navRequestType::NAV_LATEST_NEWS_REPLY  ||
                           m_type == navRequestType::NAV_CATEGORIES_REPLY )
               { // Send as poll-data
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_SERVER_POLL_SERVER_REPLY,
                     statusCode, err, extendedError );
               } else if ( m_type == navRequestType::NAV_CHANGED_LICENSE_REPLY )
               {
                  // Send it as paramreply
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_SERVER_PARAMETER_REPLY, 
                     statusCode, err, extendedError );
               } else if ( m_type == NAV_SERVER_PARAMETER_REPLY ||
                           m_type == NAV_SERVER_POLL_SERVER_REPLY ||
                           m_type == NAV_SERVER_SEARCH_REPLY ||
                           m_type == NAV_SERVER_GPS_ADDRESS_REPLY ||
                           m_type == NAV_SERVER_GPS_POS_REPLY ||
                           m_type == NAV_SERVER_PICK_ME_UP_REPLY ||
                           m_type == NAV_SERVER_PICK_UP_REPLY ||
                           m_type == NAV_SERVER_DEST_REPLY ||
                           m_type == NAV_SERVER_MAP_REPLY ||
                           m_type == NAV_SERVER_INFO_REPLY ||
                           m_type == NAV_SERVER_MESSAGE_REPLY ||
                           m_type == NAV_SERVER_UPGRADE_REPLY ||
                           m_type == NAV_SERVER_VECTOR_MAP_REPLY ||
                           m_type == NAV_SERVER_MULTI_VECTOR_MAP_REPLY ||
                           m_type == NAV_SERVER_CELL_CONFIRM ||
                           m_type == NAV_SERVER_GPS_INIT_REPLY ||
                           m_type == NAV_SERVER_BINARY_UPLOAD_REPLY ||
                           m_type == NAV_SERVER_ALARM_REPLY ) 
               {
                  // Old v8 requests
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, m_type/*Yikes*/, statusCode, 
                     err, extendedError );
               } else {
                  // NGP packet
                  m_navServerCom->decodeBuffer( 
                     eightB, req_id, NAV_REPLY, statusCode, 
                     err, extendedError );
               }
               if ( m_type == navRequestType::NAV_UPGRADE_REPLY ) {
                  // Parameter with new centerpoint and other params
                  Buffer* wfB = new Buffer( m_packetLength*2 );
                  writeHeader( *wfB, m_type, req_id );
                  wfB->writeNext8bit( 0 );

                  if ( m_navHandler->upgradeParams( reply, wfB ) ) {
                     uint8 protover;
                     uint32 length;
                     decodeHeader( *wfB, length, protover, m_type,
                                   req_id, crc, m_status );
                     m_navServerCom->decodeBuffer( 
                        wfB, req_id, NAV_SERVER_PARAMETER_REPLY, 
                        0/*statusCode*/, err, 0 );
                  }
                  delete wfB;
               }
               if ( (m_type == navRequestType::NAV_UPGRADE_REPLY ||
                     m_type == navRequestType::NAV_WHOAMI_REPLY ) &&
                    /*m_navHandler->needParameterSync() &&*/ // We always want parameter sync after whoami.
                    statusCode == NavReplyPacket::NAV_STATUS_OK )
               {
                  DBG("Request new server info after upgrade/whoami");
                  // Get new upgraded server info
                  NavRequestPacket pack( m_protoVer, 
                                         navRequestType::NAV_SERVER_INFO_REQ,
                                         req_id, NAV_SERVER_INFO_REQ_VER );
                  // Needs client type etc.
                  m_navHandler->addAuthParams( pack.getParamBlock() );
                  sendPacket( &pack );
               }

               // Check here for changed WFST
               if ( reply->getParamBlock().getParam( 7 ) != NULL &&
                    m_type != navRequestType::NAV_UPGRADE_REPLY &&
                    m_type != navRequestType::NAV_SERVER_INFO_REPLY &&
                    m_type != navRequestType::NAV_WHOAMI_REPLY )
               {
                  if ( m_navHandler->changedWFST( 
                          reply->getParamBlock().getParam( 7 )->getByte() ) )
                  {
                     Buffer* wfB = new Buffer( m_packetLength*2 );
                     writeHeader( *wfB, m_type, req_id );
                     wfB->writeNext8bit( 0 );
                     m_navHandler->makeChangedWFSTBuffer( 
                        *wfB, 
                        reply->getParamBlock().getParam( 7 )->getByte() );
                     uint8 protover;
                     uint32 length;
                     decodeHeader( *wfB, length, protover, m_type, req_id, 
                                   crc, m_status );
                     m_navServerCom->decodeBuffer( 
                        wfB, req_id, NAV_SERVER_PARAMETER_REPLY, 
                        0/*statusCode*/, err, 0 );
                     delete wfB;
                  }
               }

               delete eightB;
               delete m_assembleBuf;
               m_assembleBuf = NULL; // Reset buffer pointer.
               delete reply;
            }
            break;
         }
         data++;
         if(reinterpret_cast<const char*>(m_xorPos) >= m_xorString + XORLEN){
            m_xorPos = reinterpret_cast<const uint8*>(m_xorString);
         }
#ifndef NO_LOG_OUTPUT
         if(m_parseState != find_stx && m_parseState != length &&
            totLen % m_freq == 0){
            m_navServerCom->reportProgress(downloadingData, 
                                           MessageType(m_type), 
                                           m_packetLength - totLen , 
                                           m_packetLength);
            DBG("Progress: sending message %"PRIu32" of %"PRIu32, 
                m_packetLength - totLen, m_packetLength);
         }
#endif
      } // End while more in indata
      DBG("Data block parsed");
      if(m_parseState == send_chunks && m_assembleBuf != NULL){
         DBG("Sending chunk");
         m_navServerCom->sendChunk(req_id, m_assembleBuf);
         delete m_assembleBuf;
         m_assembleBuf = NULL;
         m_assembleBuf = new Buffer(100);
      }
   }

   unsigned ServerParser::setReportFrequency(unsigned newFreq)
   {
      unsigned tmp = m_freq;
      m_freq = newFreq;
      return tmp;
   }

   unsigned ServerParser::getReportFrequency()
   {
      return m_freq;
   }

   void
   ServerParser::setUrl( const char* /*realUrl*/ ) 
   {
      // The are no Url for NavProt.
   }
   void
   ServerParser::setProxy( const char* /*realServrerAndPort*/ ) 
   {
      // The are no proxies for NavProt.
   }

   void ServerParser::setHost(const char* /*host*/)
   {
      // NavProt doesn't need the host.
   }

   
   bool
   ServerParser::getCloseExpected() const {
      // Never gets close flag in reply in NavProt.
      return false;
   }


   void
   ServerParser::setTracking( Tracking* tracking ) {
      m_tracking = tracking;
      m_navHandler->setTracking( m_tracking );
   }

   uint8
   ServerParser::getCurrentProtoVer() {
      return NSC_PROTO_VER;
   }


   Buffer& ServerParser::writeHeader(Buffer& buf, uint16 type, uint8 reqID)
   {
      const uint8 STX = 0x02;
      buf.writeNext8bit(STX);
      buf.writeNextUnaligned32bit(0); // length, fill in later.
      buf.writeNext8bit(0x08);
      buf.writeNextUnaligned16bit(type);
      buf.writeNext8bit(reqID);
      buf.writeNextUnaligned32bit(0); //crc, fill in later.
      return buf;
   }

   inline Buffer& ServerParser::writeLength(Buffer& buf)
   {
      uint32 pos = buf.setWritePos(1);
      buf.writeNextUnaligned32bit(pos);
      buf.setWritePos(pos);
      return buf;
   }

   Buffer& ServerParser::writeCRC(Buffer& buf)
   {
      uint32 pos = buf.setWritePos(9);
      buf.writeNextUnaligned32bit(0);//write crc here XXX
      buf.setWritePos(pos);
      return buf;
   }
   
   Buffer& ServerParser::xorCode(Buffer& buf, int offset)
   {
      const uint8* code = reinterpret_cast<const uint8*>(m_xorString);
      uint8* data = const_cast<uint8*>(buf.accessRawData(offset));
      int dataLen = buf.getLength() - offset;
      //DBGDUMP("preXOR", data, dataLen);
         
      for(int i = 0; i < dataLen;){
         for(int c = 0; c < XORLEN && i < dataLen; ++c, ++i){
            data[i] ^= code[c];
         }
      }
      //DBGDUMP("postXOR", data, dataLen);
      return buf;
   }

   int 
   ServerParser::sendPacket( const NavRequestPacket* pack ) {
      vector< uint8 > buff;
      addPacketToBuff( buff, pack );
      m_serialProvider->sendData( buff.size(), &buff.front() );
      return buff.size();
   }


   void 
   ServerParser::addPacketToBuff( vector< uint8 >& buff, 
                                  const NavRequestPacket* pack )
   {
      pack->writeTo( buff, m_navHandler );
      //xorCode(params, NAV_SERVER_COM_REQUEST_HEADER_SIZE);
      uint32 magicPos = 0;
      const uint8* MAGICBYTES = reinterpret_cast<const uint8*> (
         m_xorString );
      for ( uint32 i = 10 ; i < buff.size() ; i++ ) {
         buff[ i ] = buff[ i ] ^ MAGICBYTES[ magicPos ];
         magicPos++;
         if ( magicPos >= XORLEN ) {
            magicPos = 0;
         }
      }
   }


   void 
   ServerParser::handleServerUpload( NavReplyPacket* reply ) {
      if ( reply->getParamBlock().getParam( 15 ) != NULL ) {
         char* fileStr = strdup_new( 
            reply->getParamBlock().getParam( 15 )->getString() );
         char* endFileStr = fileStr + 
            reply->getParamBlock().getParam( 15 )->getLength() - 1;
         NavRequestPacket* pack = new NavRequestPacket( 
            m_protoVer, navRequestType::NAV_NOP_REQ, reply->getReqID(), 1 );
         m_navHandler->addAuthParams( pack->getParamBlock() );
         NParam p( 16 );
         
         char* pos = fileStr;
         do {
            // Add NULL byte after ','
            char* end = strchr( pos, ',' );
            if ( end != NULL ) {
               end[ 0 ] = '\0';
            }

            vector< uint8 > data;

            // Get file
            FILE* f = NULL;
#ifdef __SYMBIAN32__
            // Is it c:\navlog.txt?
            if ( strlen( pos ) > 2 && pos[ 1 ] == ':' && 
                 isalpha( pos[ 0 ] ) )
            {
               f = fopen( pos, "rb" );
            } else {
               char fileName[256];
               strcpy( fileName, getGlobalData().m_commondata_base_path );
               strcat( fileName, pos );
               f = fopen( fileName, "rb" );
            }
#else
            f = fopen( pos, "rb" );
#endif
            if ( f != NULL ) {
               const size_t buffSize = 256;
               uint8 buff[ buffSize ];
               while ( !feof( f ) ) {
                  size_t readBytes = fread( buff,1, buffSize, f );
                  data.insert( data.end(), buff, buff + readBytes );
               }
               fclose( f );
            } else {
               DBG( "Failed to open file \"%s\"", pos );
               // And send empty file
            }

            size_t size = data.size();
            if ( p.getLength() + strlen( pos ) + 1 + 11 > MAX_UINT16 ) {
               pack->getParamBlock().addParam( p );
               p.clear();
            }
            p.addByteArray( (uint8*)pos, strlen( pos ) );
            p.addByteArray( (uint8*)":", 1 );
            char tmpStr[ 20 ];
            sprintf( tmpStr, "%u", int(size) );
            p.addByteArray( (uint8*)tmpStr, strlen( tmpStr ) );
            p.addByteArray( (uint8*)":", 1 );
            uint32 fpos = 0;
            while ( fpos < size ) {
               // Add in chunks if needed
               uint32 addSize = size - fpos;
               if ( p.getLength() + addSize > MAX_UINT16 ) {
                  addSize = MAX_UINT16 - p.getLength();
               }
               p.addByteArray( &data.front() + fpos, addSize );
               fpos += addSize;
               if ( p.getLength() >= MAX_UINT16 ) {
                  pack->getParamBlock().addParam( p );
                  p.clear();
               }
            }

            if ( end != NULL ) {
               pos = end + 1;
            } else {
               pos = endFileStr;
            }
         } while ( *pos != '\0' );
         pack->getParamBlock().addParam( p );

         sendPacket( pack );
         delete pack;

         delete [] fileStr;
      } // End if has param 15
   } // End handleserverupload
      
   void 
   ServerParser::setLicenseKeyType( const char* licenseKeyType )
   {
      m_navHandler->setLicenseKeyType( licenseKeyType );
   }

   void ServerParser::AddHWID( HWID* hwid)
   {
      m_navHandler->addHWID(hwid);
   }

   void ServerParser::setWayfinderIdStartUp( bool wayfinderIdStartUp )
   {
      m_navHandler->setWayfinderIdStartUp( wayfinderIdStartUp );
   }

} // End namespace isab
