/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NParamBlock.h"
#include "Log.h"
#include "LogMacros.h"
#include "NavGzipUtil.h"
#include "NavGunzipUtil.h"
#include <vector>


namespace isab{

NParamBlock::NParamBlock()
      : m_find( 0 ), m_valid( true ), m_flags( 0 )
{
}


NParamBlock::NParamBlock( const uint8* buff, uint32 len, byte protVer ) 
      : m_find( 0 ), m_valid( true )
{
//   Log* m_log = new Log( "NParamBlock( bye* )" );
//   DBG( "NParamBlock::NParamBlock len %u", len );
   // Read and add NParams
   uint32 pos = 0;
   m_flags = 0;
   std::vector< byte > gzBuff;

   if ( protVer >= 0xc && pos + 4 <= len ) {
      // Read flags
      m_flags = NParam::getUint32FromByteArray( buff, pos ); pos += 4;
      uint32 tflag = m_flags;
      while ( (tflag & 0x80000000) && pos + 4 <= len ) {
         tflag = NParam::getUint32FromByteArray( buff, pos ); pos += 4;
      }
      if ( m_flags & 0x1 ) {
         // CRC and length last in buffer
         if ( NavGunzipUtil::check_header( buff + pos, len - pos ) ) {
            int realLength = NavGunzipUtil::origLength( buff + pos, 
                                                        len - pos );
            gzBuff.reserve( realLength );
            int gzRes = NavGunzipUtil::gunzip( &gzBuff.front(), realLength,
                                               buff + pos, len - pos );
            if ( gzRes >= 0 ) {
               buff = &gzBuff.front();
               len = realLength;
               pos = 0;
            } else {
               Log* m_log = new Log( "NParamBlock" );
               ERR( "NParamBlock::NParamBlock %"PRIu32"  Not ok gzip %d",
                    (len - pos), gzRes );
               pos = len;
               m_valid = false;
               delete m_log;
            }
         } else {
            Log* m_log = new Log( "NParamBlock" );
            ERR( "NParamBlock::NParamBlock %"PRIu32" Not ok gzip header.",
                 (len - pos) );
            pos = len;
            m_valid = false;
            delete m_log;
         }
      }
   }

   while ( pos + 4 <= len ) {
//      DBG( "NParamBlock::NParamBlock pos %u", pos );
      uint16 paramID = ((uint16(buff[ pos + 0 ]) << 8) |
                        (uint16(buff[ pos + 1 ])     ) );

      //ntohs( *((uint16*)(buff + pos)) );
//      DBG( "NParamBlock::NParamBlock id %hu", paramID );
      pos += 2;
      uint16 length = ((uint16(buff[ pos + 0 ]) << 8) |
                       (uint16(buff[ pos + 1 ])     ) );
      //ntohs( *((uint16*)(buff + pos)) );
//      DBG( "NParamBlock::NParamBlock len %hu", length );
      pos += 2;

      if ( pos + length <= len ) {
         NParam& p = addParam( NParam( paramID ) );
//         DBG( "NParamBlock::NParamBlock param created pos %u", pos);
         p.addByteArray( buff + pos, length );
         pos += length;
//         DBG( "NParamBlock::NParamBlock param data added pos %u", pos );
      } else {
         Log* m_log = new Log( "NParamBlock" );
         ERR( "NParamBlock::NParamBlock param %"PRIu16" len %"PRIu16
              " too large %"PRIu32" >= %"PRIu32,
              paramID, length, (pos + length), len );
         pos = len;
         m_valid = false;
         delete m_log;
      }
   }
   if ( pos != len ) {
      Log* m_log = new Log( "NParamBlock" );
      ERR( "NParamBlock::NParamBlock %"PRIu32" trailing bytes. Buffer: ",
           (len - pos) );
      m_log->debugDump( "", buff, len );
      
      m_valid = false;
      delete m_log;
   }
//   DBG( "NParamBlock::NParamBlock done" );
}


const NParam* 
NParamBlock::getParam( uint16 paramID ) const {
   m_find.setParamID( paramID );
   NParams::const_iterator it = m_params.find( m_find );
   if ( it != m_params.end() ) {
      return &*it;
   } else {
      return NULL;
   }
}


void 
NParamBlock::getAllParams( uint16 paramID, 
                           std::vector< const NParam* >& params ) const
{
   m_find.setParamID( paramID );
   std::pair<NParams::const_iterator, NParams::const_iterator> res =
      m_params.equal_range( m_find );
   for ( NParams::const_iterator it = res.first ; it != res.second ; ++it )
   {
      params.push_back( &*it );
   }
}


void
NParamBlock::writeParams( std::vector< uint8 >& buff, byte protVer,
                          bool mayUseGzip ) const
{
   uint32 flags = 0x0;
   uint32 startSize = buff.size();
   if ( protVer >= 0xc ) {
      // Supports gzip
      if ( NavGunzipUtil::implemented() ) {
         flags |= 0x2;  
      }
      NParam::addUint32ToByteVector( buff, flags );
   }
   for ( NParams::const_iterator it = m_params.begin() ; 
         it != m_params.end() ; ++it )
   {
      (*it).writeTo( buff );
   }
   if ( flags & 0x2 && mayUseGzip ) {
      // Try gzip
      std::vector<byte> gzBuff( buff.begin(), buff.begin() + startSize );
      NParam::addUint32ToByteVector( gzBuff, flags | 0x1 ); /* gziped */
      
      gzBuff.resize( buff.size() );
      int gres = NavGzipUtil::gzip( &gzBuff.front() + startSize + 4, 
                                    gzBuff.size()   - startSize - 4,
                                    &buff.front()   + startSize + 4, 
                                    buff.size()     - startSize - 4 );
      if ( gres > 0 ) {
         gzBuff.erase( gzBuff.begin() + gres + startSize + 4, 
                       gzBuff.end() );
         buff.swap( gzBuff );
      }
   }
}


NParam& 
NParamBlock::addParam( const NParam& param ) {
   return const_cast< NParam& >( *m_params.insert( param ) );
}


void 
NParamBlock::dump( Log* out, bool dumpValues ) const {
   out->debug( "NParamBlock %u params", m_params.size() );
   for ( NParams::const_iterator it = m_params.begin() ; 
         it != m_params.end() ; ++it )
   {
      (*it).dump( out, dumpValues );
   }
}


const NParamBlock& 
NParamBlock::assignToThis( const NParamBlock& p ) {
//   m_params = p.m_params;
   m_params.clear();
   for ( NParams::const_iterator it = p.m_params.begin() ; 
         it != p.m_params.end() ; ++it )
   {
      m_params.insert( *it );
   }
   m_find = p.m_find;
   m_valid = p.m_valid;
   return *this;
}

bool
NParamBlock::mayUseGzip() const {
   return m_flags & 0x2;
}


} // End namespace isab

