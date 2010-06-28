/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "ImageBlender.h"

void ImageBlender::blendImage( uint32 originColor,
                               uint32 blendColor,
                               int32 height,
                               std::vector<uint32>& result,
                               uint8 minAlpha,
                               uint8 maxAlpha)
{
   if ( height == 0 ) {
      // No meaning.
      return;
   }
   // Calculate the start and the end color, blended.
   uint32 startColor = calcBlendedColor( originColor, blendColor, minAlpha );
   uint32 endColor   = calcBlendedColor( originColor, blendColor, maxAlpha );

   // Calculate the diff between the endColor and the startColor. 
   // R1 - R2, G1 - G2 and B1 - B2. By dividing the calculation into
   // three steps with each diff value seperated it is much easier to
   // handle a negative diff.
   int16 rDiff = ( ( 0xff & ( endColor >> 16 ) ) - 
                   ( 0xff & ( startColor >> 16 ) ) );
   int16 gDiff = ( ( 0xff & ( endColor >> 8 ) ) - 
                   ( 0xff & ( startColor >> 8 ) ) );
   int16 bDiff = ( ( 0xff & ( endColor >> 0 ) ) - 
                   ( 0xff & ( startColor >> 0 ) ) );

   result.clear();
   result.reserve( height );
   result.push_back( startColor );
   for( int32 x = 0; x < height; ++x ) {
      // For each line (givin from the height) calculate the new color.
      // This calculation is done by adding the startColor R value with
      // the diff R value divided with number of lines.
      uint32 color = ( ( ( ( 0xff & ( startColor >> 16 ) ) + rDiff * ( x + 1 ) / height ) << 16 ) |
                       ( ( ( 0xff & ( startColor >> 8 ) ) + gDiff * ( x + 1 ) / height ) << 8 ) |
                       ( ( ( 0xff & ( startColor >> 0 ) ) + bDiff * ( x + 1 ) / height ) << 0 ) );

      result.push_back( color );
   }
}

uint32 ImageBlender::calcBlendedColor( uint32 orgColor,
                                       uint32 blendColor,
                                       uint8  alpha )
{
   // Blend the colors, the formel looks like this:
   // ( ( R1 x ( 256 x 1 ) -  alpha ) / 256 ) + ( ( R2 x alpha ) / 256 ) where
   // R1 is the R value in RGB for the orgColor and R2 is the R value in RGB for 
   // the blendColor and alpha is the alpha value expressed as a byte instead
   // of a float. The formel is used not just on the R value but of course 
   // on the G and B value as well.
   return ( ( ( ( ( ( 0xff & ( orgColor >> 16 ) ) * ( ( 256 * 1 ) - alpha ) ) / 256 ) + 
                ( ( ( 0xff & ( blendColor >> 16 ) ) * alpha ) / 256 ) ) << 16 ) |
            ( ( ( ( ( 0xff & ( orgColor >> 8 ) ) * ( ( 256 * 1 ) - alpha ) ) / 256 ) + 
                ( ( ( 0xff & ( blendColor >> 8 ) ) * alpha ) / 256 ) ) << 8 ) |
            ( ( ( ( ( 0xff & ( orgColor >> 0 ) ) * ( ( 256 * 1 ) -  alpha ) ) / 256 ) + 
                ( ( ( 0xff & ( blendColor >> 0 ) ) * alpha ) / 256 ) ) << 0 ) );
}
