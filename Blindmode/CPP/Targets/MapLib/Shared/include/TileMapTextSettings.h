/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TILEMAPTEXTSETTINGS_H
#define TILEMAPTEXTSETTINGS_H

#include "config.h"

#include<map>

#ifdef __SYMBIAN32__
#include <e32std.h>
#endif

class TileMapTextSettings {
public:

   /// Copying replacement for the old font_pair_t
   class font_pair_t {
   public:
      font_pair_t() : first(0), second(0) {}
#ifdef __SYMBIAN__
      /// This one only supports ascii names.
      font_pair_t( const TDesC& name, int size );
#endif      
      font_pair_t( const char* name, int size );
      font_pair_t( const font_pair_t& other );
      ~font_pair_t();
      font_pair_t& operator=( const font_pair_t& other);
      
      char* first;
      int   second;
   };
   
   /**
    *   Empty constructor. Sets all to HARDCODED_FONT.
    */
   TileMapTextSettings();
   
   /**
    *   NB! The strings will not be copied!
    *   @param lineFonts         Map of fonts to use for lines with the
    *                            minimum line width in first and
    *                            font in second.
    *   @param horizontalFont    Font for horizontal placement e.g. under
    *                            citycentres.
    *   @param roundRectFont     Font for placement in road number signs.
    *   @param insidePolygonFont Font for placement inside polygons.
    *   @param progressIndicatorFont   Font for progress indicator.
    *   @param copyrightFont     Font for the copyright string.
    *
    */
   TileMapTextSettings( const std::map<int, font_pair_t>& lineFonts,
                        const font_pair_t& horizontalFont,
                        const font_pair_t& roundRectFont,
                        const font_pair_t& insidePolygonFont,
                        const font_pair_t& progressIndicatorFont,
                        const font_pair_t& copyrightFont );
   
   /// @return The font to be used for lines with the supplied width.
   const font_pair_t& getLineFont( int width ) const;

   /// @return The font to be used for horizontal placement (cc:s)
   const font_pair_t& getHorizontalFont() const {
      return m_horizFont;
   }

   /// @return The font to be used for road signs.
   const font_pair_t& getRoundRectFont() const {
      return m_roundRectFont;
   }

   /// @return The font to be used when placing text in a polygon.
   const font_pair_t& getInsidePolygonFont() const {
      return m_insidePolygonFont;
   }

   /// @return The font to be used for the progress indicator.
   const font_pair_t& getProgressIndicatorFont() const {
      return m_progressIndicatorFont;
   }

   /// @return The font to be used for the copyright string.
   const font_pair_t& getCopyrightFont() const {
      return m_copyrightFont;
   }

   /// Map of line fonts
   std::map<int, font_pair_t> m_lineFonts;
   /// Horizontal font
   font_pair_t m_horizFont;
   /// Roundrect font
   font_pair_t m_roundRectFont;
   /// Inside polygon font
   font_pair_t m_insidePolygonFont;
   /// Progress indicator font
   font_pair_t m_progressIndicatorFont;
   /// Copyright font
   font_pair_t m_copyrightFont;
};


#endif
