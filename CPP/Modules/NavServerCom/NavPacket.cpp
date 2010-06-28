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
#include "NavHandler.h"
#include "ServerParser.h"
#include "NavRequestType.h"

namespace isab{


NavPacket::NavPacket( uint8 protoVer, uint16 type, uint8 reqID, uint8 reqVer )
      : m_protoVer( protoVer ), m_type( type ), m_reqID( reqID ), 
        m_reqVer( reqVer )
{
}


NavPacket::NavPacket( uint8 protoVer, uint16 type, uint8 reqID, uint8 reqVer,
                      const uint8* buff, uint32 buffLen )
      : m_protoVer( protoVer ), m_type( type ), m_reqID( reqID ), 
        m_reqVer( reqVer ), m_params( buff, buffLen, protoVer )
{
}

uint8
NavPacket::getCurrentProtoVer() {
   return ServerParser::getCurrentProtoVer();
}

NavRequestPacket::NavRequestPacket( uint8 protoVer, uint16 type, uint8 reqID,
                                    uint8 reqVer )
      : NavPacket( protoVer, type, reqID, reqVer )
{
}


void 
NavRequestPacket::writeTo( std::vector< uint8 >& buff, 
                           const NavHandler* nh ) const 
{
   // Header

   // STX 1
   buff.push_back( 0x02 );
   // Length 4 (Real value written last)
   buff.push_back( 0 );
   buff.push_back( 0 );
   buff.push_back( 0 );
   buff.push_back( 0 );
   // protover 1 
   buff.push_back( m_protoVer );
   // type 2
   buff.push_back( ( m_type ) >> 8 );
   buff.push_back( ( m_type ) & 0xff );
   // req_id 1
   buff.push_back( m_reqID );
   // req_ver 1
   buff.push_back( m_reqVer );

   // ParamBlock
   m_params.writeParams( buff, m_protoVer );

   // Write length
   // Length 1-4
   buff[ 1 ] = (( buff.size() + 4 ) >> 24 ) & 0xff;
   buff[ 2 ] = (( buff.size() + 4 ) >> 16 ) & 0xff;
   buff[ 3 ] = (( buff.size() + 4 ) >> 8  ) & 0xff;
   buff[ 4 ] = (( buff.size() + 4 ) )       & 0xff;

   // CRC 4
   uint32 crc = nh->crc32( &buff.front(), buff.size() );
   buff.push_back( (( crc ) >> 24 ) & 0xff );
   buff.push_back( (( crc ) >> 16 ) & 0xff );
   buff.push_back( (( crc ) >> 8  ) & 0xff );
   buff.push_back( (( crc ) )       & 0xff );

}


NavReplyPacket::NavReplyPacket( uint8 protoVer, uint16 type, uint8 reqID, 
                                uint8 reqVer, uint8 statusCode, 
                                const char* statusMessage, 
                                const uint8* buff, uint32 buffLen )
      : NavPacket( protoVer, type, reqID, reqVer, buff, buffLen ), 
        m_statusCode( statusCode )
{
   setStatusMessage( statusMessage );
}


NavReplyPacket::NavReplyPacket( const byte* data, uint32 size ) 
      : NavPacket( 0, 0, 0, 0 )
{
   NonOwningBuffer buff( data, size );

   setProtoVer(      buff.readNext8bit() );
   setType(          buff.readNextUnaligned32bit() );
   setRequestVer(    buff.readNext8bit() );
   setStatusCode(    buff.readNext8bit() );
   setStatusMessage( buff.getNextCharString() );
   m_params.assignToThis( 
      NParamBlock( buff.accessRawData(), buff.remaining(), m_protoVer ) );
}


navRequestType::RequestType
NavReplyPacket::getRequestType( const uint8* data, int size ) {
   NonOwningBuffer buff( data, size );
   buff.readNext8bit(); // ProtoVer
   return navRequestType::RequestType( buff.readNextUnaligned32bit() );
}


} // End namespace isab
