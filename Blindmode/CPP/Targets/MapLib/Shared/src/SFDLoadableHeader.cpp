/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SFDLoadableHeader.h"
#include "SharedBuffer.h"
#include "XorFileHandler.h"
#include "RouteID.h"
#include "SFDLoadableHeaderListener.h"
#include "TileCollectionNotice.h"

#define INITIAL_HEADER_SIZE 19


SFDLoadableHeader::SFDLoadableHeader( const SharedBuffer* uidXorbuffer,
                                      const SharedBuffer* warezXorbuffer )
      : SFDHeader(),
        m_state( nothing_loaded ),
        m_nbrBytesToRead( INITIAL_HEADER_SIZE ),
        m_xorBuffer( NULL ),
        m_uidXorBuffer( uidXorbuffer ),
        m_warezXorBuffer( warezXorbuffer ),
        m_listener( NULL ),
        m_readBuffer( NULL ),
        m_fileHandler( NULL )
{

}

SFDLoadableHeader::~SFDLoadableHeader() 
{
   delete m_fileHandler;
   delete[] m_readBuffer;
}

void
SFDLoadableHeader::innerLoad() 
{
   if ( m_nbrBytesToRead > 0 ) {
      
      delete [] m_readBuffer;
      m_readBuffer = new uint8[ m_nbrBytesToRead ];
      
      m_fileHandler->read( m_readBuffer, m_nbrBytesToRead, this ); 
   } else {
      delete [] m_readBuffer;
      m_readBuffer = NULL;
      m_readOK = ( m_state == loaded_all_header );
      m_listener->loadDone( this, m_fileHandler );
   }
}

void
SFDLoadableHeader::load( FileHandler* file, SFDLoadableHeaderListener* listener )
{
   delete m_fileHandler;
   m_fileHandler = new XorFileHandler( file );
   m_listener = listener;
   innerLoad();
}

void
SFDLoadableHeader::loadInitialHeader( SharedBuffer& buf )
{
   // Load initial header.
   const char* str = buf.readNextString();
   if ( strcmp( str, "storkafinger" ) != 0 ) {
      m_state = failed_to_load;
      m_nbrBytesToRead = 0;
      innerLoad();
      return;
   }

   // Version
   m_version = buf.readNextBAByte();
  
   // Encryption type.
   m_encryptionType = encryption_t( buf.readNextBAByte() );

   // Set the right xorbuffer depending on the encryption type.
   switch ( m_encryptionType ) {
      case ( no_encryption ) :
         m_xorBuffer = NULL;
         break;
      case ( uid_encryption ) :
         MC2_ASSERT( m_uidXorBuffer != NULL );
         m_xorBuffer = m_uidXorBuffer;
         break;
      case ( warez_encryption ) :
         MC2_ASSERT( m_warezXorBuffer != NULL );
         m_xorBuffer = m_warezXorBuffer;
         break;
   }

   // Header size.
   m_headerSize = buf.readNextBALong();

   // Rest of buffer is encrypted.
   if ( m_xorBuffer != NULL ) {
      m_fileHandler->setXorHelper( XorHelper( m_xorBuffer->getBufferAddress(),
                                             m_xorBuffer->getBufferSize(),
                                             buf.getCurrentOffset() ) );
   }

   m_nbrBytesToRead = m_headerSize;
   
   // Load the rest.
   m_state = loaded_initial_header;
   innerLoad();
}

void
SFDLoadableHeader::loadRemainingHeader( SharedBuffer& buf )
{
   // File size.
   m_fileSize = buf.readNextBALong();

   // The name.
   m_name = buf.readNextString();

   mc2dbg << "[SFDLH] m_name = " << m_name << endl;

   // Check the file size.
   if ( m_fileHandler->getFileSize() != m_fileSize ) {
      m_state = failed_to_load;
      m_nbrBytesToRead = 0;
      innerLoad();
      return;
   }

   // Creation time.
   m_creationTime = buf.readNextBALong();

   // Null terminated strings?
   m_stringsAreNullTerminated = buf.readNextBAByte();

   // Longest length of string.
   m_maxStringSize = buf.readNextBAByte();

   // Nbr initial chars.
   byte nbrInitialChars = buf.readNextBAByte();
   m_initialCharacters.resize( nbrInitialChars );
   // Initial chars.
   {for ( byte b = 0; b < nbrInitialChars; ++b ) {
      m_initialCharacters[ b ] = buf.readNextBAByte();
   }}
   
   // Nbr route ids.
   byte nbrRouteIDs = buf.readNextBAByte();
   m_routeIDs.reserve( nbrRouteIDs );
   {for ( byte b = 0; b < nbrRouteIDs; ++b ) {
      uint32 id = buf.readNextBALong();
      uint32 creationTime = buf.readNextBALong(); 
      m_routeIDs.push_back( RouteID( id, creationTime ) );
   }}

   // Number of bits for the string index.
   m_strIdxEntrySizeBits = buf.readNextBALong();

   // Position for the start of strings index.
   m_strIdxStartOffset = buf.readNextBALong();

   // Number strings.
   m_nbrStrings = buf.readNextBALong();
   
   // Position for the start of string data.
   m_strDataStartOffset = buf.readNextBALong();

   // Position for the start of the buffers index.
   m_bufferIdxStartOffset = buf.readNextBALong();

   // Position for the start of the buffer data.
   m_bufferDataStartOffset = buf.readNextBALong();
  
   // If to read debug param strings for the multi buffers.
   m_readDebugParams = buf.readNextBAByte();
   
   // The tile collections.
   uint32 nbrCollections = buf.readNextBAShort();

   m_tileCollection.resize( nbrCollections );
   {for ( uint32 i = 0; i < nbrCollections; ++i ) {
      m_tileCollection[ i ].load( buf );
   }}
  
   // All is now loaded.
   m_state = loaded_all_header;
   m_nbrBytesToRead = 0;
   innerLoad();
}

void 
SFDLoadableHeader::readDone( int nbrRead ) 
{
   if ( nbrRead == -1 ) {
      m_readOK = false;
      m_listener->loadDone( this, m_fileHandler );      
      return;
   }
   SharedBuffer buf( m_readBuffer, nbrRead );
   switch ( m_state ) {
      case ( nothing_loaded ) :
         loadInitialHeader( buf );
         return;
      case ( loaded_initial_header ) :
         loadRemainingHeader( buf );
         return;
      default:
         // This shouldn't happen.
         MC2_ASSERT( false );
   }
}


