/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NParam.h"

namespace isab{

/*inline*/ NParam::NParam( uint16 paramID, const uint8* buff, uint16 len ) 
      : m_paramID( paramID ), m_buff( buff, buff + len )
{
}


/*inline*/ NParam::NParam( const NParam& other ) {
   m_paramID = other.m_paramID;
   m_buff = other.m_buff;
}


/*inline*/ void
NParam::addByte( uint8 data ) {
   m_buff.push_back( data );
}


/*inline*/ void 
NParam::addUint16( uint16 data ) {
   m_buff.push_back( ( data ) >> 8 );
   m_buff.push_back( ( data ) & 0xff );
}

void
NParam::addUint32ToByteVector( std::vector<byte>& v, uint32 d ) {
   v.push_back( (( d ) >> 24 ) & 0xff );
   v.push_back( (( d ) >> 16 ) & 0xff );
   v.push_back( (( d ) >> 8  ) & 0xff );
   v.push_back( (( d ) )       & 0xff );
}

/*inline*/ void 
NParam::addUint32( uint32 data ) {
   addUint32ToByteVector( m_buff, data );
}


/*inline*/ void 
NParam::addInt32( int32 data ) {
   addUint32( data );
}


/*inline*/ void 
NParam::addByteArray( const uint8* buff, uint16 len ) {
   m_buff.insert( m_buff.end(), buff, buff + len );
}

void
NParam::addByteArray( const char* buff, uint16 len ) {
   addByteArray( reinterpret_cast<const uint8*>(buff), len );
}


/*inline*/ void
NParam::addString( const char* str ) {
   m_buff.insert( m_buff.end(), str, str + strlen( str ) + 1 );
}


/*inline*/ NParam::NParam( uint16 paramID, bool data ) 
      : m_paramID( paramID )
{
   if ( !data ) {
      m_buff.push_back( data );
   }
}


/*inline*/ NParam::NParam( uint16 paramID, uint8 data ) 
      : m_paramID( paramID )
{
   m_buff.push_back( data );
}


/*inline*/ NParam::NParam( uint16 paramID, uint16 data ) 
      : m_paramID( paramID )
{
   addUint16( data );
}


/*inline*/ NParam::NParam( uint16 paramID, uint32 data ) 
      : m_paramID( paramID )
{
   addUint32( data );
}


/*inline*/ NParam::NParam( uint16 paramID, int32 data ) 
      : m_paramID( paramID )
{
   addInt32( data );
}


/*inline*/ NParam::NParam( uint16 paramID, const char* data )
      : m_paramID( paramID ), m_buff( data, data + strlen( data ) + 1 )
{
}


/*inline*/ NParam::NParam( uint16 paramID, const uint16* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*2 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addUint16( data[ i ] );
   }
}


/*inline*/ NParam::NParam( uint16 paramID, const uint32* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*4 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addUint32( data[ i ] );
   }
}


/*inline*/ NParam::NParam( uint16 paramID, const int32* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*4 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addInt32( data[ i ] );
   }
}


/*inline*/ NParam::NParam( uint16 paramID, int32 lat, int32 lon )
      : m_paramID( paramID )
{
   addInt32( lat );
   addInt32( lon );
}


/*inline*/ NParam::NParam( uint16 paramID )
      : m_paramID( paramID )
{
}


/*inline*/ uint16
NParam::getParamID() const {
   return m_paramID;
}


/*inline*/ void
NParam::setParamID( uint16 paramID ) {
   m_paramID = paramID;
}


/*inline*/ uint16 
NParam::getLength() const {
   return m_buff.size();
}


/*inline*/ const uint8* 
NParam::getBuff() const {
   return &m_buff.front();
}


/*inline*/ bool 
NParam::getBool() const {
   if ( getLength() == 0 ) {
      return true;
   } else {
      return m_buff[ 0 ] != 0;
   }
}


/*inline*/ uint8 
NParam::getByte( uint16 index ) const {
   return m_buff[ index ];
}


/*inline*/ uint16 
NParam::getUint16( uint16 index ) const {
//   return ntohs(*((uint16 *)(&(m_buff.front())+index)));
   return ( (uint16( m_buff[ index + 0 ]) << 8) |
            (uint16( m_buff[ index + 1 ])    ) );
}

uint32
NParam::getUint32FromByteArray( const byte* b, uint16 index ) {
   return ( (uint32(b[ index + 0 ]) << 24) |
            (uint32(b[ index + 1 ]) << 16) |
            (uint32(b[ index + 2 ]) <<  8) |
            (uint32(b[ index + 3 ])      ) );
}

/*inline*/ uint32 
NParam::getUint32( uint16 index ) const {
//   return ntohl(*((uint32 *)(&(m_buff.front())+index)));
   return getUint32FromByteArray( &m_buff.front(), index );
}


/*inline*/ int32 
NParam::getInt32( uint16 index ) const {
//   return ntohl(*((int32 *)(&(m_buff.front())+index)));
   return ( (int32(m_buff[ index + 0 ]) << 24) |
            (int32(m_buff[ index + 1 ]) << 16) |
            (int32(m_buff[ index + 2 ]) <<  8) |
            (int32(m_buff[ index + 3 ])      ) );
}


/*inline*/ const char* 
NParam::getString( uint16 index ) const {
   return reinterpret_cast<const char*> (&m_buff.front()+index);
}


/*inline*/ const uint8* 
NParam::getByteArray() const {
   return &m_buff.front();
}


/*inline*/ uint16
NParam::getUint16Array( uint16 index ) const {
   return ntohs(*(((uint16 *)&(m_buff.front()))+index));
}


/*inline*/ uint32
NParam::getUint32Array( uint16 index ) const {
   return ntohl(*(((uint32 *)&(m_buff.front()))+index));
}


/*inline*/ int32
NParam::getInt32Array( uint16 index ) const {
   return ntohl(*(((int32 *)&(m_buff.front()))+index));
}


/*inline*/ void
NParam::writeTo( std::vector< uint8 >& buff ) const {
   buff.push_back( ( m_paramID ) >> 8 );
   buff.push_back( ( m_paramID ) & 0xff );
   buff.push_back( ( m_buff.size() ) >> 8 );
   buff.push_back( ( m_buff.size() ) & 0xff );
   buff.insert( buff.end(), m_buff.begin(), m_buff.end() );
}


/*inline*/ bool 
NParam::operator < ( const NParam& b ) const {
   return m_paramID < b.m_paramID;
}


/*inline*/ bool 
NParam::operator > ( const NParam& b ) const {
   return m_paramID > b.m_paramID;
}


/*inline*/ bool 
NParam::operator != ( const NParam& b ) const {
   return m_paramID != b.m_paramID;
}


/*inline*/ bool 
NParam::operator == ( const NParam& b ) const {
   return m_paramID == b.m_paramID;
}


/*inline*/ void
NParam::setBuff( const std::vector< uint8 > buff ) {
   m_buff = buff;
}


/*inline*/ const std::vector< uint8 >&
NParam::getVector() const {
   return m_buff;
}


/*inline*/ void
NParam::clear() {
   m_buff.clear();
}


/*inline*/ void
NParam::dump( Log* out, bool dumpValue ) const {
   char txt[ 512 ];
   sprintf( txt, "   ParamID %u len %u\n", getParamID(), getLength() );
   if ( dumpValue ) {
      out->debugDump( txt, getBuff(), getLength() );
   }
}


} // End namespace isab

