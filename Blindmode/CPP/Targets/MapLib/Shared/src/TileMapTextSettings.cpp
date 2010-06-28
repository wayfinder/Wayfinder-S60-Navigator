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

#include "TileMapTextSettings.h"
#include "TileMapUtil.h"

#ifdef __unix__
   #define HARDCODED_FONT "-*-times-*-r-*-*-12-*-*-*-*-*-*-*"
#elif defined(ARCH_OS_WINDOWS)
   #define HARDCODED_FONT "Lucida Console"
#else
   #define HARDCODED_FONT "LatinPlain12"
#endif

using namespace std;

TileMapTextSettings::font_pair_t::font_pair_t( const char* name, int size )
{
   if ( name ) {
      first  = TileMapUtil::newStrDup( name );
   } else {
      first = NULL;
   }
   second = size;
}

#ifdef __SYMBIAN__
TileMapTextSettings::font_pair_t::font_pair_t( const TDesC& name, int size )
{
   first = new char[ name.Length() + 1 ];
   int i = 0;
   for ( i = 0; i < name.Length(); ++i ) {
      first[i] = name[i];      
   }
   first[i] = 0;
   second = size;
}
#endif

TileMapTextSettings::font_pair_t::~font_pair_t()
{
   delete [] first;
}

TileMapTextSettings::font_pair_t&
TileMapTextSettings::font_pair_t::operator=( const font_pair_t& other )
{
   if ( this != &other ) {
      delete [] first;
      first = NULL;
      if ( other.first ) {
         first  = TileMapUtil::newStrDup( other.first );
      }
      second = other.second;
   }
   return *this;
}

TileMapTextSettings::font_pair_t::font_pair_t( const font_pair_t& other )
{
   first = NULL;
   *this = other;
}

TileMapTextSettings::TileMapTextSettings()
{
   m_lineFonts.insert( make_pair( 12, font_pair_t( HARDCODED_FONT, 12 ) ) );
   m_horizFont = m_roundRectFont = 
      m_insidePolygonFont = m_progressIndicatorFont = 
      m_copyrightFont = font_pair_t( HARDCODED_FONT, 12 );
}

TileMapTextSettings::
TileMapTextSettings( const map<int, font_pair_t>& lineFonts,
                     const font_pair_t& horizontalFont,
                     const font_pair_t& roundRectFont,
                     const font_pair_t& insidePolygonFont,
                     const font_pair_t& progressIndicatorFont,
                     const font_pair_t& copyrightFont ) :
      m_lineFonts( lineFonts ),
      m_horizFont( horizontalFont ),
      m_roundRectFont( roundRectFont ),
      m_insidePolygonFont( insidePolygonFont ),
      m_progressIndicatorFont( progressIndicatorFont ),
      m_copyrightFont( copyrightFont )
{   
}

const TileMapTextSettings::font_pair_t&
TileMapTextSettings::getLineFont( int width ) const
{
   map<int, font_pair_t>::const_iterator it = m_lineFonts.lower_bound( width );
   if ( it != m_lineFonts.begin() ) {
      --it;
   }
   return it->second;
}
      
      
