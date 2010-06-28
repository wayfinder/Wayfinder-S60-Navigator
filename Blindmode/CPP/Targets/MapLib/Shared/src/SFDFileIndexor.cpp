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

#include "SFDFileIndexor.h"
#include "SFDFileIndexorListener.h"
#include "FileHandler.h"
#include "SFDHeader.h"

#include "BitBuffer.h"

SFDFileIndexor::SFDFileIndexor( FileHandler* fh,                               
                                SFDFileIndexorListener* listener,
                                const SFDHeader& header )
      : m_strOffsetBuf( 2 * header.getStrIdxEntrySizeBits() / 8 )
{   
   m_fileHandler   = fh;
   allocStr( header.maxStringSize() + 1 );
   m_minBufSize = header.maxStringSize() + 1;
   m_listener = listener;
   m_header = &header;
   m_state = IDLE;
}

SFDFileIndexor::~SFDFileIndexor()
{   
}

uint32
SFDFileIndexor::strNbrToOffset( uint32 idx ) const
{
   return (m_header->getStrIdxEntrySizeBits() / 8) * idx +
          m_header->getStrIdxStartOffset();
}

bool
SFDFileIndexor::start()
{
   if ( m_stringRead ) {
      m_listener->indexorDone( this, 0 );
      return false;
   }

   if ( m_state == PERMANENT_ERROR ) {
      mc2dbg << "[SFDFI]: In PERMANENT_ERROR state" << endl;
      m_listener->indexorDone( this, -1 );
   }

   // Realloc the string buffer if needed
   if ( m_strAllocSize < m_minBufSize ) {
      allocStr( m_minBufSize + 1 );
   }
   
   m_state = READING_STR_OFFSET;
   m_fileHandler->setPos( strNbrToOffset( m_strNbr ) );
   // Read it into the buffer.
   m_fileHandler->read( m_strOffsetBuf.getBufferAddress(),
                        m_strOffsetBuf.getBufferSize(),
                        this );
   
   return true;
}

void
SFDFileIndexor::readDone( int nbrRead )
{
   MC2_ASSERT( m_state == READING_STR_OFFSET ||
               m_state == READING_STR );

   if ( nbrRead <= 0 ) {
      m_state = PERMANENT_ERROR;
      m_listener->indexorDone( this, -1 );
      return;
   }
   
   if ( m_state == READING_STR_OFFSET ) {
      // Will read string
      m_state = READING_STR;      
      // Start at beginning.
      m_strOffsetBuf.reset();
      // Read offset of our string
      m_strOffset = m_strOffsetBuf.readNextBALong();
      // The length of the string is the difference between
      // this and the next one.
      m_curStrLength = m_strOffsetBuf.readNextBALong() - m_strOffset;
      // Seek to that pos
      m_fileHandler->setPos( m_header->getStrDataStartOffset() + m_strOffset );
      // Read a string      
      m_fileHandler->read( reinterpret_cast<uint8*>(m_str),
                           m_curStrLength,
                           this );

   } else if ( m_state == READING_STR ) {
      mc2dbg << "[SFDFileIndexor]: String read = "
             << m_str << endl;
      m_stringRead = true;
      // It is done.
      m_state = IDLE;
      // Inform the listener
      m_listener->indexorDone( this, 0 );
   }
      
}
