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

#include "SFDBufReader.h"

#include <algorithm>

#include "FileHandler.h"
#include "SFDHeader.h"
#include "SFDBufReaderListener.h"

SFDBufReader::SFDBufReader( const SFDHeader& header,
                            FileHandler& fh,
                            SFDBufReaderListener& listener )
      : m_header( header ),
        m_bufOffsetBuf(8)
{
   m_listener = &listener;
   m_fileHandler = &fh;
   m_state = IDLE;
   m_readBuffer = NULL;
}

SFDBufReader::~SFDBufReader()
{
   delete m_readBuffer;
}

uint32
SFDBufReader::strNbrToBufIdxOffset( uint32 strNbr ) const
{
   return m_header.getBufIdxStartOffset() + strNbr * 4;
}

void
SFDBufReader::start( uint32 strNbr )
{
   startAbsolute( strNbrToBufIdxOffset( strNbr ) );
}

void
SFDBufReader::startAbsolute( uint32 offset )
{
   if ( m_state == PERMANENT_ERROR ) {
      // Cannot recover from this.
      m_listener->bufferRead( NULL );
      return;
   }
   
   MC2_ASSERT( m_state == IDLE );

   m_state = READING_BUF_OFFSET;
   m_fileHandler->setPos( offset );
   // Read it into the pos and length buffer
   m_fileHandler->read( m_bufOffsetBuf.getBufferAddress(),
                        m_bufOffsetBuf.getBufferSize(),
                        this );
   // Started
}


void
SFDBufReader::readDone( int nbrRead )
{
   MC2_ASSERT( m_state == READING_BUF_OFFSET ||
               m_state == READING_BUF );

   if ( m_state == READING_BUF_OFFSET ) {
      // Will read buf.
      m_state = READING_BUF;
      
      // Read from the start of our buf
      m_bufOffsetBuf.reset();
      // Read offset of the buffer
      uint32 bufOffset = m_bufOffsetBuf.readNextBALong();
      // Find out the offset of the next buffer and calculate length
      int bufLength = m_bufOffsetBuf.readNextBALong() - bufOffset;
      mc2dbg8 << "[SDFBR]: READING_BUF_OFFSET : offset = "
              << bufOffset << ", len = " << bufLength << endl;

      // This should really not happen, but you never know
      if ( bufLength == 0 ) {
         m_state = IDLE;
         m_listener->bufferRead( NULL );
         return;
      }
      
      // Create buffer
      delete m_readBuffer;
      m_readBuffer = new BitBuffer( bufLength );
      // Seek to the position
      m_fileHandler->setPos( bufOffset );
      // And read
      m_fileHandler->read( m_readBuffer->getBufferAddress(),
                           m_readBuffer->getBufferSize(),
                           this );
      
   } else if ( m_state == READING_BUF ) {
      // Done and done
      m_state = IDLE;
      // Cannot use the member variable anymore since we might
      // get a callback from m_listener
      BitBuffer* tmpBuf = NULL;
      std::swap( tmpBuf, m_readBuffer );
      // All is set - call listener.
      m_listener->bufferRead( tmpBuf );      
   }
}


