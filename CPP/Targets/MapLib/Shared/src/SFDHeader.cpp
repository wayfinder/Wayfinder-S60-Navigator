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

#include "SFDHeader.h"
#include "RouteID.h"

#include "TileMapParams.h"
#include "TileMapParamTypes.h"
#include "TileCollectionNotice.h"

SFDHeader::SFDHeader() : m_maxStringSize( 0 )
{
   
}

SFDHeader::~SFDHeader() 
{

}

uint32 
SFDHeader::getStrIdxStartOffset() const
{
   return m_strIdxStartOffset;
}
   
uint32 
SFDHeader::getStrIdxEntrySizeBits() const 
{
   return m_strIdxEntrySizeBits;
}

uint32 
SFDHeader::getNbrStrings() const 
{
   return m_nbrStrings;
}

uint32 
SFDHeader::getStrDataStartOffset() const
{
   return m_strDataStartOffset;
}

bool
SFDHeader::maybeInBinaryCache( const MC2SimpleString& desc ) const
{
   for ( int i = 0, n = m_initialCharacters.size(); i < n; ++i ) {
      if ( desc[0] == m_initialCharacters[i] ) {
         // Could be
         return true;
      }
   }
   return false;
}

bool 
SFDHeader::maybeInCache( const MC2SimpleString& desc ) const
{
   if ( TileMapParamTypes::isMap( desc.c_str() ) ) {
      if ( getMultiBufferOffsetOffset( desc ) >= 0 ) {
         return true;
      }
   }
   return maybeInBinaryCache( desc );
}

uint32 
SFDHeader::maxStringSize() const
{
   return m_maxStringSize;
}

bool 
SFDHeader::stringsAreNullTerminated() const
{
   return m_stringsAreNullTerminated;
}

int 
SFDHeader::getMultiBufferOffsetOffset( const TileMapParams& param ) const
{
   for ( uint32 i = 0; i < m_tileCollection.size(); ++i ) {
      int offset = m_tileCollection[ i ].getOffset( param );
      if ( offset >= 0 ) {
         return offset;
      }
   }
   return -1;
}
   
const MC2SimpleString& 
SFDHeader::getName() const
{
   return m_name;
}

const impRange_t* 
SFDHeader::getImportanceRange( const TileMapParams& param ) const
{
   for ( uint32 i = 0; i < m_tileCollection.size(); ++i ) {
      const impRange_t* range = m_tileCollection[ i ].getImpRange( param );
      if ( range != NULL ) {
         return range;
      }
   }
   return NULL;
}

bool 
SFDHeader::readDebugParams() const
{
   return m_readDebugParams;
}
  
uint32 
SFDHeader::getCreationTime() const
{
   return m_creationTime;
}

