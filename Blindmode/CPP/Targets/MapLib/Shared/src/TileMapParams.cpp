/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"

#include "TileMapParams.h"
#include "MC2SimpleString.h"

#include <stdio.h>
#include "BitBuffer.h"
#include <algorithm>

#include "RouteID.h"

#include "MathUtility.h"

#define USE_TABLE
#undef USE_TABLE

using namespace std;

const char * const TileMapParams::c_sortedCodeChars = TILEMAP_CODE_CHARS;

#ifndef USE_TABLE
// Eh. Nothing.
#else

/// 128 bytes of translation buffer.
static const uint8 c_table[] = { 
   0xff, /* 0x00 */ 0xff, /* 0x01 */ 0xff, /* 0x02 */ 0xff, /* 0x03 */ 
   0xff, /* 0x04 */ 0xff, /* 0x05 */ 0xff, /* 0x06 */ 0xff, /* 0x07 */ 
   0xff, /* 0x08 */ 0xff, /* 0x09 */ 0xff, /* 0x0a */ 0xff, /* 0x0b */ 
   0xff, /* 0x0c */ 0xff, /* 0x0d */ 0xff, /* 0x0e */ 0xff, /* 0x0f */ 
   0xff, /* 0x10 */ 0xff, /* 0x11 */ 0xff, /* 0x12 */ 0xff, /* 0x13 */ 
   0xff, /* 0x14 */ 0xff, /* 0x15 */ 0xff, /* 0x16 */ 0xff, /* 0x17 */ 
   0xff, /* 0x18 */ 0xff, /* 0x19 */ 0xff, /* 0x1a */ 0xff, /* 0x1b */ 
   0xff, /* 0x1c */ 0xff, /* 0x1d */ 0xff, /* 0x1e */ 0xff, /* 0x1f */ 
   0xff, /* 0x20 */ 0xff, /* 0x21 */ 0xff, /* 0x22 */ 0xff, /* 0x23 */ 
   0xff, /* 0x24 */ 0xff, /* 0x25 */ 0xff, /* 0x26 */ 0xff, /* 0x27 */ 
   0x01, /* 0x28 */ 0x02, /* 0x29 */ 0x03, /* 0x2a */ 0x04, /* 0x2b */ 
   0xff, /* 0x2c */ 0x05, /* 0x2d */ 0xff, /* 0x2e */ 0xff, /* 0x2f */ 
   0xff, /* 0x30 */ 0xff, /* 0x31 */ 0xff, /* 0x32 */ 0xff, /* 0x33 */ 
   0xff, /* 0x34 */ 0xff, /* 0x35 */ 0xff, /* 0x36 */ 0xff, /* 0x37 */ 
   0xff, /* 0x38 */ 0xff, /* 0x39 */ 0xff, /* 0x3a */ 0xff, /* 0x3b */ 
   0x06, /* 0x3c */ 0xff, /* 0x3d */ 0x07, /* 0x3e */ 0xff, /* 0x3f */ 
   0x08, /* 0x40 */ 0x09, /* 0x41 */ 0x0a, /* 0x42 */ 0x0b, /* 0x43 */ 
   0x0c, /* 0x44 */ 0x0d, /* 0x45 */ 0x0e, /* 0x46 */ 0x0f, /* 0x47 */ 
   0x10, /* 0x48 */ 0x11, /* 0x49 */ 0x12, /* 0x4a */ 0x13, /* 0x4b */ 
   0x14, /* 0x4c */ 0x15, /* 0x4d */ 0x16, /* 0x4e */ 0x17, /* 0x4f */ 
   0x18, /* 0x50 */ 0x19, /* 0x51 */ 0x1a, /* 0x52 */ 0x1b, /* 0x53 */ 
   0x1c, /* 0x54 */ 0x1d, /* 0x55 */ 0x1e, /* 0x56 */ 0x1f, /* 0x57 */ 
   0x20, /* 0x58 */ 0x21, /* 0x59 */ 0x22, /* 0x5a */ 0x23, /* 0x5b */ 
   0xff, /* 0x5c */ 0x24, /* 0x5d */ 0x25, /* 0x5e */ 0xff, /* 0x5f */ 
   0xff, /* 0x60 */ 0x26, /* 0x61 */ 0x27, /* 0x62 */ 0x28, /* 0x63 */ 
   0x29, /* 0x64 */ 0x2a, /* 0x65 */ 0x2b, /* 0x66 */ 0x2c, /* 0x67 */ 
   0x2d, /* 0x68 */ 0x2e, /* 0x69 */ 0x2f, /* 0x6a */ 0x30, /* 0x6b */ 
   0x31, /* 0x6c */ 0x32, /* 0x6d */ 0x33, /* 0x6e */ 0x34, /* 0x6f */ 
   0x35, /* 0x70 */ 0x36, /* 0x71 */ 0x37, /* 0x72 */ 0x38, /* 0x73 */ 
   0x39, /* 0x74 */ 0x3a, /* 0x75 */ 0x3b, /* 0x76 */ 0x3c, /* 0x77 */ 
   0x3d, /* 0x78 */ 0x3e, /* 0x79 */ 0x3f, /* 0x7a */ 0xff, /* 0x7b */ 
   0xff, /* 0x7c */ 0xff, /* 0x7d */ 0xff, /* 0x7e */ 0xff, /* 0x7f */ 
};

static inline uint8 tableLookup( uint8 inchar )
{
   // Table ends with 0x7f -> return now if 0x80-bit set.
   if ( inchar & 0x80 ) {
      return 0xff;
   } else {
      return c_table[inchar];
   }
}

#endif

TileMapParams::TileMapParams(const MC2SimpleString& paramString)
{
   m_routeID = NULL;
   // Since we use the ref-counting now, copy the string
   m_paramString = paramString;
   parseParamString( paramString );
}

void
TileMapParams::updateParamString()
{
   // Alloc on stack is probably faster than with new
   byte innerBuf[64];
   BitBuffer buf(innerBuf, 64);
   // Zero the buffer.
   memset(buf.getBufferAddress(), 0 , buf.getBufferSize() );
   // Low bits first.
   buf.writeNextBits(m_importanceNbr & 0xf, 4);
   buf.writeNextBits(m_detailLevel, 4);
   
   int nbrBits = 15;
   if ( m_detailLevel > 0 ) {
      nbrBits = MathUtility::getNbrBitsSignedGeneric( m_tileIndexLat );
      int tmpNbrBits = 
         MathUtility::getNbrBitsSignedGeneric( m_tileIndexLon );
      nbrBits = MAX( nbrBits, tmpNbrBits );
      buf.writeNextBits( nbrBits, 4 );
   }
   
   if ( nbrBits > 8 ) {
      // Write lower bits first since they seem to differ more often.
      buf.writeNextBits(m_tileIndexLat & 0xff, 8);
      buf.writeNextBits(m_tileIndexLon & 0xff, 8);
      // Then some higher bits.
      buf.writeNextBits(m_tileIndexLat >> 8, nbrBits - 8);
      buf.writeNextBits(m_tileIndexLon >> 8, nbrBits - 8);
   } else {
      // Write all at once.
      buf.writeNextBits(m_tileIndexLat, nbrBits);
      buf.writeNextBits(m_tileIndexLon, nbrBits);
   }
  
   buf.writeNextBits(m_layer, 4);
   
   // Lowest bits of server prefix.
   buf.writeNextBits( m_serverPrefix & 0x1f, 5 ); 
   
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      // The features do not have any language.
   } else {
      // For strings, the language is important.
      // Lowest bits first.
      buf.writeNextBits(m_langType & 0x7, 3);
      // Then highest bits.
      buf.writeNextBits(m_langType >> 3, 3 );
   }   
   
   // High bit of server prefix. 
   buf.writeNextBits(m_serverPrefix >> 5, 1); 
   // Highest bits of importance nbr.
   buf.writeNextBits(m_importanceNbr >> 4, 1);
   // Inverted gzip
   buf.writeNextBits(!m_gzip, 1);
   
   if ( m_layer == TileMapTypes::c_routeLayer ) {
      if ( m_routeID == NULL ) {
         mc2log << error << "[TMH]: No routeID in route layer params" << endl;
         RouteID inval;
         inval.save(&buf);
      } else {
         m_routeID->save(&buf);
      }
   }
   
   // Char-encode. 
   int nbrChars = (buf.getCurrentBitOffset() + 5) / 6;
   // The length of the buffer cannot be larger than 64. I know it.
   // Add space for K and \0
   char* tmpString = new char[nbrChars+1+1];
   
   int pos = 0;
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      tmpString[pos++] = 'G'; // We use G for features now. G - Geometry :)
   } else {
      tmpString[pos++] = 'T'; // T as in sTring map. errm.. No no, T - Text
   }
   buf.reset();
   for( int i = 0; i < nbrChars; ++i ) {
      tmpString[pos++] = c_sortedCodeChars[buf.readNextBits(6)];
   }

   // Skip trailing zero '+'.
   while ( (pos > 0 ) && tmpString[pos - 1] == '+' ) {
      tmpString[ --pos ] = '\0';
   }
   tmpString[pos] = '\0';
   // Put the tmpString in the paramstring.
   m_paramString = tmpString;
   delete [] tmpString;
   
#if 0
   mc2log << "m_paramString = " << m_paramString << endl;
   mc2log << "m_importanceNbr = " << m_importanceNbr << endl;
   mc2log << "m_detailLevel = " << m_detailLevel << endl;
   mc2log << "m_tileIndexLat = " << m_tileIndexLat << endl;
   mc2log << "m_tileIndexLon = " << m_tileIndexLon << endl;
   mc2log << "m_layer = " << m_layer << endl;
   mc2log << "m_gzip = " << m_gzip << endl;
   mc2log << "m_mapOrStrings = " << m_mapOrStrings << endl;
   mc2log << "m_langType = " << m_langType << endl;
   mc2log << "m_serverPrefix = " << m_serverPrefix << endl;
#endif   
}

void
TileMapParams::parseParamString( const MC2SimpleString& paramString )
{
   char firstChar = paramString[0];

   switch ( firstChar ) {
      case ( 'T' ) :
      case ( 'G' ) :
         break;
      default:
         // Other not supported first chars
         return;
         break;
   }

   // Skip the 'T' or 'G'
   if ( firstChar == 'T' ) {
      m_mapOrStrings = TileMapTypes::tileMapStrings;
   } else {
      MC2_ASSERT( firstChar == 'G' );
      m_mapOrStrings = TileMapTypes::tileMapData;
   }

           
   const char* paramChars = paramString.c_str() + 1;
   const int len = paramString.length();
   
   // Alloc on stack is probably faster than with new
   byte innerBuf[64];
   BitBuffer buf(innerBuf, 64);
   // Zero the buffer.
   memset(buf.getBufferAddress(), 0 , buf.getBufferSize() ); 

#ifndef USE_TABLE
   const int sortLen = strlen(c_sortedCodeChars);
   const char* sortedEnd = c_sortedCodeChars + sortLen;
   for ( int i = 0; i < len; ++i ) {
      const char* foundChar = lower_bound(c_sortedCodeChars,
                                          sortedEnd,
                                          paramChars[i]);
      if ( foundChar == sortedEnd ) {
         mc2dbg << "[TMP]: ERROR: Invalid character 0x"
                << hex << int(paramChars[i]) << dec << " in params" << endl;
         m_valid = false;
         return;
      }      
      buf.writeNextBits(int(foundChar-c_sortedCodeChars), 6);
   }
#else
   // Don't include the \0 - char.
   for ( int i = 0; i < len - 1; ++i ) {
      uint8 foundChar = tableLookup( paramChars[i] );
      if ( foundChar == 0xff ) {
         mc2dbg << "[TMP]: ERROR: Invalid character 0x"
                << hex << int(paramChars[i]) << dec << " in params" << endl;
         m_valid = false;
         return;
      }
      buf.writeNextBits( foundChar, 6 );
   }
#endif
   buf.reset();
   // Low bits first.
   int lowImportance;
   buf.readNextBits(lowImportance, 4);
   buf.readNextBits(m_detailLevel, 4);            
   int nbrBits = 15;
   if ( m_detailLevel > 0 ) {
      // Read the number of bits used for the lat/lon index.
      nbrBits = buf.readNextBits( 4 );
   }

   if ( nbrBits > 8 ) {
      // Read the lower bits first.
      int lowLat = buf.readNextBits(8);
      int lowLon = buf.readNextBits(8);
      
      // And then the higher.
      buf.readNextSignedBits(m_tileIndexLat, nbrBits - 8 );
      buf.readNextSignedBits(m_tileIndexLon, nbrBits - 8 );

      m_tileIndexLat = (m_tileIndexLat << 8 ) | lowLat;
      m_tileIndexLon = (m_tileIndexLon << 8 ) | lowLon;
   } else {
      // Read all at once.
      buf.readNextSignedBits(m_tileIndexLat, nbrBits );
      buf.readNextSignedBits(m_tileIndexLon, nbrBits );
   } 
   
   buf.readNextBits(m_layer, 4);
   
   // Lower bits of serverprefix first.
   uint32 lowServerPrefix;
   buf.readNextBits( lowServerPrefix, 5);
   
   if ( m_mapOrStrings == TileMapTypes::tileMapData ) {
      // The features do not have any language. Set to swedish.
      m_langType = LangTypes::swedish;
   } else {
      // Lowest bits first.
      uint32 lowLangType;
      buf.readNextBits(lowLangType, 3);
      // Then highest bits.
      buf.readNextBits(m_langType, 3);
      m_langType = 
         LangTypes::language_t((m_langType << 3 ) | lowLangType);
   }

   // Then the high.
   buf.readNextBits( m_serverPrefix, 1 );
   m_serverPrefix = (m_serverPrefix << 5 ) | lowServerPrefix;
   
   // High bits of importance nbr.
   buf.readNextBits(m_importanceNbr, 1);
   m_importanceNbr = (m_importanceNbr << 4 ) | lowImportance;
   
   // Read inverted gzip.
   buf.readNextBits(m_gzip, 1);
   m_gzip = !m_gzip;
   
   // Then also load the route id.
   if ( m_layer == TileMapTypes::c_routeLayer ) {
      m_routeID = new RouteID();
      m_routeID->load(&buf);
   }

   m_valid = true;

#if 0   
   mc2log << "paramString = " << paramString << endl;
   mc2log << "m_importanceNbr = " << m_importanceNbr << endl;
   mc2log << "m_detailLevel = " << m_detailLevel << endl;
   mc2log << "m_tileIndexLat = " << m_tileIndexLat << endl;
   mc2log << "m_tileIndexLon = " << m_tileIndexLon << endl;
   mc2log << "m_layer = " << m_layer << endl;
   mc2log << "m_gzip = " << m_gzip << endl;
   mc2log << "m_mapOrStrings = " << m_mapOrStrings << endl;
   mc2log << "m_langType = " << m_langType << endl;
   mc2log << "m_serverPrefix = " << m_serverPrefix << endl;
#endif   
}

