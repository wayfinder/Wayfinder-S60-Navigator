/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_PARSER_H
#define SERVER_PARSER_H

class NavRequestPacket;

#define NAV_ROUTE_REPLY_HEADER_SIZE 35
#define NAV_SERVER_COM_MAX_PACKET_SIZE (1024*1024)
#define NAV_SERVER_COM_HEADER_SIZE_WHOLE 14
#define NAV_SERVER_COM_HEADER_SIZE 14
#define NAV_SERVER_COM_REQUEST_HEADER_SIZE 13

#define XORSTRING                                     \
   "\x37\x77\x89\x05\x28\x72\x5b\x2a\xce\xe4\x44\x1a" \
   "\x28\x72\x5b\x2a\xce\xe4\x44\x1a\x79\x8a\xdb\x90" \
   "\xce\xe4\x44\x1a\x79\x8a\xdb\x90\x2f\x5b\xcc\xd1" \
   "\x79\x8a\xdb\x90\x2f\x5b\xcc\xd1\x6e\x72\xb4\x9a" \
   "\x2f\x5b\xcc\xd1\x6e\x72\xb4\x9a\x79\x9b\xa5\x2b" \
   "\x6e\x72\xb4\x9a\x79\x9b\xa5\x2b\x45\x6a\xc1\x7c" \
   "\x79\x9b\xa5\x2b\x45\x6a\xc1\x7c\xe0\x4a\x81\x08" \
   "\x45\x6a\xc1\x7c\xe0\x4a\x81\x08\xbc\xdb\x31\x8a" \
   "\xe0\x4a\x81\x08\xbc\xdb\x31\x8a\xbe\x75\xa3\x36" \
   "\xbc\xdb\x31\x8a\xbe\x75\xa3\x36\xfe\x7d\xc6\x2c" \
   "\xbe\x75\xa3\x36\xfe\x7d\xc6\x2c\xd8\x91\xfc\x45" \
   "\xfe\x7d\xc6\x2c\xd8\x91\xfc\x45\x03\xb0\xde\x7b" \
   "\xd8\x91\xfc\x45\x03\xb0\xde\x7b\x4b\x82\xa6\x90" \
   "\x03\xb0\xde\x7b\x4b\x82\xa6\x90\xeb\x67\x0c\xcb" \
   "\x4b\x82\xa6\x90\xeb\x67\x0c\xcb\xb0\x8c\xd2\x6c" \
   "\xeb\x67\x0c\xcb\xb0\x8c\xd2\x6c\x67\x03\xf5\x24" \
   "\xb0\x8c\xd2\x6c\x67\x03\xf5\x24\x77\x98\x5a\x74" \
   "\x67\x03\xf5\x24\x77\x98\x5a\x74\x14\x1f\x9f\xec" \
   "\x77\x98\x5a\x74\x14\x1f\x9f\xec\xb0\x9b\x30\xb2" \
   "\x14\x1f\x9f\xec\xb0\x9b\x30\xb2\x4b\x0e\x2d\x95" \
   "\xb0\x9b\x30\xb2\x4b\x0e\x2d\x95\x90\xd3\x25\x7a" \
   "\x4b\x0e\x2d\x95"
#define XORLEN 256

namespace isab{
   class NavServerCom;
   class Tracking;
   class SerialProviderPublic;
   

   class ServerParser {
   public:
      /** Constuctor. 
       * @param spp  pointer to the SerialProviderPublic used to send 
       *             Messages to the server.
       * @param nsc  pointer to the NavServerCom module.
       * @param freq the frequency of progressMessages. For every 
       *             <b>freq</b> byte uploaded or downloaded a progressMessage
       *             is sent. Default value 100.
       */
      ServerParser(SerialProviderPublic* spp, NavServerCom* nsc, 
                   unsigned freq = 500);
      /** Destructor. */
      virtual ~ServerParser();
      
      /** Sends a server poll message to the server.
       * @param reqID       the request id of the poll message.
       * @param navigatorID the id of this navigator unit.
       * @param userName    the username used in this session.
       * @param offset      the offset into the total poll block.
       */
      void pollServer(uint8 reqID, 
                      //uint32 navigatorID, const char* userName,
                      uint32 offset);

      /** Deletes the input buffer and resets the state too. */
      virtual void flush();

      /** Sends a parameter packet to the server.
       * @param reqID        the id of this request.
       * @param navigatorID  the id of the navigator.
       * @param userName     the username used in this session.
       * @param reflash      true if we want to send a reflash parameter.
       * @param clientParams a Buffer containing the 
       *                     NavClient-to-NavigatorServer parameters received
       *                     from the NavClient. The Buffer is aaumed to end 
       *                     with an end parameter.
       * @param needReply If a parameterreply is needed.
       * @param secondaryServer If using a secondary server.
       * @param sendUnaccompanied If about to send request that needs to be
       *                          sent alose, no server info before it.
       * @return the number of bytes sent.
       */
      uint32 sendParameters(uint8 reqID, 
                            // uint32 navigatorID, 
//                             const char* userName, bool reflash, 
                            const Buffer* clientParams,
                            bool needReply,
                            bool secondaryServer,
                            bool sendUnaccompanied );

      /** Sends the content buffer in a packet to the server.
       * @param reqID       the request id of this packet.
       * @param navigatorID the id of this navigator unit.
       * @param userName    the username used in this session.
       * @param reqType     the request type.
       * @param content     a Buffer containing the data of a server request.
       * @return the number of bytes sent.
       */
      uint32 sendContents(uint8 reqID, 
                          // uint32 navigatorID, 
//                           const char* userName,
                          uint16 reqType, const Buffer* content);
      

      virtual int sendPacket( const NavRequestPacket* pack );

      /** Decodes the data as a header and writes the header fields into the
       * different argument.
       * @param data     a Buffer containg the data to be interpreted.
       * @param length   returns the length field of the header
       * @param protover returns the protocol version  field of the header
       * @param type     returns the type field of the header
       * @param req_id   returns the request id field of the header
       * @param crc      returns the CRC field of the header
       * @param status   returns the status field of the header
       */
      void decodeHeader(Buffer& data, uint32& length, uint8& protover, 
                        uint16& type, uint8& req_id, uint32& crc ,
                        uint8& status
                        );


      void decodeHeaderA(Buffer& data, uint32& length, uint8& protover, 
                         uint16& type, uint8& req_id, uint8& req_ver,
                         uint8& status);

      /**
       * The method that decodes incomming bytes from server.
       */
      virtual void decodeServerPacket( const uint8* data, size_t len );

      unsigned setReportFrequency(unsigned newFreq = 100);
      unsigned getReportFrequency();


      /**
       * Set proxy mode and the real server for the proxty to connect to.
       *
       * @param realServererAndPort The server to ask the proxy to use.
       */
      virtual void setProxy( const char* realServererAndPort );

      /**
       * Set URL part for proxty to connect to.
       *
       * @param realUrl The server to ask the proxy to use.
       */
      virtual void setUrl( const char* realUrl );

      /**
       * Provide the host if the parser wants to use it. 
       */
      virtual void setHost(const char* host);

      /**
       * If last reply indicated that close of socket was due to happen.
       */
      virtual bool getCloseExpected() const;


      /**
       * Sets the tracking.
       */
      void setTracking( Tracking* tracking );

      /**
       * Get the ProtoVer used in this client.
       */
      static uint8 getCurrentProtoVer();

      /**
       * Set the license key type.
       * IMEI if NULL or not set, otherwise "imei" or "imsi".
       */
      void setLicenseKeyType( const char* licenseKeyType );
      
      void AddHWID( HWID* hwid);

      /**
       *
       */
      void setWayfinderIdStartUp( bool wayfinderIdStartUp );

   protected:
      enum ParameterTags{
         endTag    = 0x00000000,
         reflashMe = 0x03000000,
      };

      Buffer& writeCRC(Buffer& buf);
      Buffer& writeHeader(Buffer& buf, uint16 type, uint8 reqID);
      Buffer& writeLength(Buffer& buf);
      Buffer& xorCode(Buffer& buf, int offset);

      /**
       * Adds the packet as bytes to buff.
       *
       * @param buff The buffer to fill in the packet bytes into.
       * @param pack The packet to use.
       */
      void addPacketToBuff( std::vector< uint8 >& buff, 
                            const NavRequestPacket* pack );

      void handleServerUpload( NavReplyPacket* reply );

      SerialProviderPublic* m_serialProvider;
      NavServerCom* m_navServerCom;
      NavHandler* m_navHandler;
      /** Possible states for the parser of incoming messages. */
      enum ParseState{
         find_stx = 0,   /** Find start byte.*/
         length,         /** Find length field. */
         save_data,      /** Save data until end of packet. */
         header,         /** Read header. */
         send_chunks,    /** Incoming route. Send it as chunks */
      };
      /** Current parse state of the incoming packet. */
      ParseState m_parseState;
      /** Our very own log object. */
      Log* m_log;

      /// The tracking if any.
      Tracking* m_tracking;

   private:

      /** Bytes left to read in the current packet. */
      uint32 totLen;
      /** The request id of the current packet. */
      uint8 req_id;
      /** The buffer were the packet is assembled. */
      Buffer* m_assembleBuf;

      /** The frequency to send progress messages.*/
      unsigned m_freq;

      uint16 m_type;
      uint8 m_status;
      uint32 m_packetLength;

      const char* m_xorString;
      const uint8* m_xorPos;

      bool m_parametersSent;
      
      uint8 m_protoVer;
   };
}
#endif
