/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TILE_MAP_PARAMTYPES_H
#define TILE_MAP_PARAMTYPES_H

#include "config.h"

class TileMapParamTypes {
public:
   enum param_t {
      TILE,
      BITMAP,
      FORMAT_DESC,
      FORMAT_DESC_CRC,
      UNKNOWN,
   };

   static inline param_t getParamType( const char* str );
   
   static inline param_t getParamType( const MC2SimpleString& str );

   static inline bool isMapFormatDesc( const char* paramsStr );
   
   static inline bool isMap( const char* paramsStr );
   
   static inline bool isBitmap( const char* paramsStr );

};

inline TileMapParamTypes::param_t
TileMapParamTypes::getParamType( const char* str )
{
   switch ( str[0] ) {
      case 'G': // Features
      case 'T': // Strings
         return TILE;
      case 'B':
      case 'b':
         return BITMAP;
      case 'D':
         return FORMAT_DESC;
      case 'C':
         return FORMAT_DESC_CRC;
   }
   return UNKNOWN;
}

inline TileMapParamTypes::param_t
TileMapParamTypes::getParamType( const MC2SimpleString& str )
{
   return getParamType( str.c_str() );
}

inline bool 
TileMapParamTypes::isMapFormatDesc( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::FORMAT_DESC ||
      TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::FORMAT_DESC_CRC;
}

inline bool 
TileMapParamTypes::isMap( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::TILE;
}

inline bool 
TileMapParamTypes::isBitmap( const char* paramsStr )
{
   return TileMapParamTypes::getParamType( paramsStr ) ==
      TileMapParamTypes::BITMAP;
}

#endif
