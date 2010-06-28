/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrackPIN.h"
#include "Buffer.h"
#include "TraceMacros.h"

namespace isab{


TrackPINList::TrackPINList( const uint8* data, int size ) 
      : m_crc( 0 )
{
   NonOwningBuffer buff( data, size );
   m_crc = buff.readNextUnaligned32bit();
   while ( buff.remaining() ) {
      uint32 id =           buff.readNextUnaligned32bit();
      const char* pin =     buff.getNextCharString();
      const char* comment = buff.getNextCharString();
      push_back( new TrackPIN( id, pin, comment ) );
   }
}


TrackPINList::TrackPINList()
      : m_crc( 0 )
{

}


TrackPINList::~TrackPINList() {
   for ( iterator it = begin() ; it != end() ; ++it ) {
      delete *it;
   }
}


void
TrackPINList::addPIN( const char* pin, const char* comment, uint32 id ) {
   m_crc = 0;
   push_front( new TrackPIN( id, pin, comment ) );
}


void
TrackPINList::deletePIN( uint32 id ) {
   m_crc = 0;
   for ( iterator it = begin() ; it != end() ; ++it ) {
      if ( (*it)->getID() == id ) {
         (*it)->setID( SET_UINT32_MSB( (*it)->getID() ) );
         break;
      }
   }
}


void
TrackPINList::deletePIN( const TrackPIN* pin ) {
   m_crc = 0;
   for ( iterator it = begin() ; it != end() ; ++it ) {
      if ( *it == pin ) {
         (*it)->setID( SET_UINT32_MSB( (*it)->getID() ) );
         break;
      }
   }
}


void
TrackPINList::packInto( Buffer* buff ) const {
   buff->writeNextUnaligned32bit( m_crc );
   for ( const_iterator it = begin() ; it != end() ; ++it ) {
      buff->writeNextUnaligned32bit( (*it)->getID() );
      buff->writeNextCharString( (*it)->getPIN() );
      buff->writeNextCharString( (*it)->getComment() );
   }
}


void
TrackPINList::removePIN( uint32 id ) {
   for ( iterator it = begin() ; it != end() ; ++it ) {
      if ( REMOVE_UINT32_MSB( (*it)->getID() ) == id ) {
         erase( it );
         break;
      }
   }
}


void
TrackPINList::removePIN( const TrackPIN* pin ) {
   for ( iterator it = begin() ; it != end() ; ++it ) {
      if ( *it == pin ) {
         erase( it );
         break;
      }
   }
}


void
TrackPINList::TRACE() {
   for ( const_iterator it = begin() ; it != end() ; ++it ) {
      TRACE_DBG( " PIN %s ID %"PRId32" Comment %s", 
                 (*it)->getPIN(), (*it)->getID(), (*it)->getComment() );
   }
}

} // End namespace isab
