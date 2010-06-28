/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVPACKET_H
#define NAVPACKET_H

#include "arch.h"
#include "NParamBlock.h"
#include "Buffer.h"
#include "NavRequestType.h"

namespace isab {


/**
 * XXX: Copied from MC2 20040622 10:00.
 * Class that represents a Navigator packet.
 * 
 */
class NavPacket {
   public:

      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       */
      NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer );


      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       * @param buff The parameter block buffer.
       * @param buffLen The length of the parameter block buffer.
       */
      NavPacket( byte protoVer, uint16 type, byte reqID, byte reqVer,
                 const byte* buff, uint32 buffLen );


      /**
       * Get the protoVer.
       */
      byte getProtoVer() const;


      /**
       * Get the type.
       */
      uint16 getType() const;


      /**
       * Get the reqID.
       */
      byte getReqID() const;


      /**
       * Get the reqVer.
       */
      byte getReqVer() const;


      /**
       * Get the parameter block.
       */
      NParamBlock& getParamBlock();


      /**
       * Get the parameter block.
       */
      const NParamBlock& getParamBlock() const;

      /**
       * Set the protoVer.
       */
      void setProtoVer( byte protoVer );

      /**
       * Sets the type.
       */
      void setType( uint16 type );


      /**
       * Sets the reqVer.
       */
      void setRequestVer( byte reqVer );

      /**
       *
       */
      void setReqID(byte reqId);

      /**
       * Get the ProtoVer used in this client.
       */
      static uint8 getCurrentProtoVer();

   protected:
       /// The protocol version.
       byte m_protoVer;


       /// The type of packet.
       uint16 m_type;


       /// The ID of the packet.
       byte m_reqID;


       /// The version of type.
       byte m_reqVer;


       /// The parameter block.
       NParamBlock m_params;
};


/**
 * XXX: Copied from MC2 20040622 10:00.
 * Class that represents a Navigator request packet.
 * 
 */
class NavRequestPacket : public NavPacket {
   public:
      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of reply.
       * @param statusCode The status of the reply.
       * @param statusMessage The status code as text.
       */
      NavRequestPacket( byte protoVer, uint16 type, byte reqID, 
                        byte reqVer );


      /**
       * Writes the packet to a buffer.
       *
       * @param buff The buffer to write to.
       */
      void writeTo( std::vector< byte >& buff, const class NavHandler* nh ) const;
};



/**
 * XXX: Copied from MC2 20040622 10:00.
 * Class that represents a Navigator reply packet.
 * 
 */
class NavReplyPacket : public NavPacket {
   public:
      enum ReplyStatus {
         NAV_STATUS_OK                           = 0x00,
         NAV_STATUS_NOT_OK                       = 0x01,
         NAV_STATUS_REQUEST_TIMEOUT              = 0x02,
         NAV_STATUS_PARAM_REQ_NOT_FIRST          = 0x03,
         NAV_STATUS_OUTSIDE_MAP                  = 0x04,
         NAV_STATUS_PROTOVER_NOT_SUPPORTED       = 0x05,
         NAV_STATUS_OUTSIDE_ALLOWED_AREA         = 0x06,
         NAV_STATUS_NO_TRANSACTIONS_LEFT         = 0x07,
         NAV_STATUS_GENERIC_PERMANENT_ERROR      = 0x08,
         NAV_STATUS_GENERIC_TEMPORARY_ERROR      = 0x09,
         NAV_STATUS_SERVER_UPGRADE_IN_PROGRESS   = 0x0a,
         NAV_STATUS_SERVER_OVERLOADED            = 0x0b,
         NAV_STATUS_EXPIRED_USER                 = 0x0c,
         NAV_STATUS_UNAUTHORIZED_USER            = 0x0d,
         NAV_STATUS_REDIRECT                     = 0x0e,
         NAV_STATUS_UPDATE_NEEDED                = 0x0f,
         NAV_STATUS_WF_TYPE_TOO_HIGH_LOW         = 0x10,
         NAV_STATUS_REQ_VER_NOT_SUPPORTED        = 0x11,
         NAV_STATUS_CRC_ERROR                    = 0x12,
         NAV_STATUS_PARAMBLOCK_INVALID           = 0x13,
         NAV_STATUS_PARAMETER_INVALID            = 0x15,
         NAV_STATUS_MISSING_PARAMETER            = 0x16,
         NAV_STATUS_UNAUTH_OTHER_HAS_LICENSE     = 0x17,
         NAV_STATUS_EXTENDED_ERROR               = 0x18,
   
         NAV_STATUS_REQUEST_SPECIFIC_MASK        = 0x80
      };

      /**
       * Some upgrade specific status codes.
       */
      enum UpgradeStatus {
         UPGRADE_MUST_CHOOSE_REGION      = 0xF1,
      };

      /**
       * Constructor.
       *
       * @param protoVer The protocol version.
       * @param type The type of packet.
       * @param reqID The ID of the packet.
       * @param reqVer The version of request.
       * @param buff The parameter block buffer.
       * @param buffLen The length of the parameter block buffer.
       */
      NavReplyPacket( byte protoVer, uint16 type, byte reqID, 
                      byte reqVer, byte statusCode, 
                      const char* statusMessage, 
                      const byte* buff, uint32 buffLen );


      /**
       * Constructs from a Nav2 buffer with type and all in it.
       */
      NavReplyPacket( const byte* buff, uint32 buffLen );


      /**
       * Get the statusCode.
       */
      byte getStatusCode() const;


      /**
       * Set the statusCode.
       */
      void setStatusCode( byte statusCode );


      /**
       * Get the statusMessage.
       */
      const char* getStatusMessage() const;


      /**
       * Set the statusMessage.
       */
      void setStatusMessage( const char* statusMessage );


      /**
       * Get the type from a Nav2 buffer with type and all in it.
       */
      static navRequestType::RequestType getRequestType( const uint8* buf, int size );


   private:
      /// The status of the reply.
      byte m_statusCode;


      /// The status code as text.
      Buffer m_statusMessage;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline byte
NavPacket::getProtoVer() const {
   return m_protoVer;
}


inline uint16 
NavPacket::getType() const {
   return m_type;
}

inline void
NavPacket::setProtoVer( byte protoVer ) {
   m_protoVer = protoVer;
}

inline void
NavPacket::setType( uint16 type ) {
   m_type = type;
}


inline void
NavPacket::setRequestVer( byte reqVer ) {
   m_reqVer = reqVer;
}


inline void 
NavPacket::setReqID(byte reqId) {
   m_reqID = reqId;
}


inline byte 
NavPacket::getReqID() const {
   return m_reqID;
}


inline byte 
NavPacket::getReqVer() const {
   return m_reqVer;
}


inline NParamBlock& 
NavPacket::getParamBlock() {
   return m_params;
}


inline const NParamBlock& 
NavPacket::getParamBlock() const {
   return m_params;
}


inline byte 
NavReplyPacket::getStatusCode() const {
   return m_statusCode;
}


inline void 
NavReplyPacket::setStatusCode( byte statusCode ) {
   m_statusCode = statusCode;
}


inline const char* 
NavReplyPacket::getStatusMessage() const {
   return reinterpret_cast< const char* > (
      m_statusMessage.accessRawData( 0 ) );
}


inline void 
NavReplyPacket::setStatusMessage( const char* statusMessage ) {
   m_statusMessage.reserve( strlen( statusMessage ) + 1 );
   m_statusMessage.clear();
   m_statusMessage.writeNextCharString( statusMessage );
}


} /* namespace isab */

#endif // NAVPACKET_H

