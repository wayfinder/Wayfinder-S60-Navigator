/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef IMAGE_BLENDER_H
#define IMAGE_BLENDER_H

#include "config.h"

#include <vector>

/**
 * Class that can blend two colors together, creates
 * a gradient.
 */
class ImageBlender 
{
public:
   /**
    * Function that creates a gradient with a given 
    * number of color steps.
    * @param originColor The orginal color, the color that should
    *                    be the base color of the gradient.
    * @param blendColor  The color ot use for blending the org color
    *                    with.
    * @param height      Number of color steps, a high amount of
    *                    color steps will result in a smoother gradient
    *                    than with a small amount of color steps.
    * @param minAlpha    Defines the startColor, the originColor and the
    *                    blendColor will be blended toghether with this alpha
    *                    value defining the first color of the gradient.
    *                    The alpha value is expressed as a byte (0 - 255).
    * @param maxAlpha    Defines the endColor, the originColor and the
    *                    blendColor will be blended toghether with this alpha
    *                    value defining the last color of the gradient.
    *                    The alpha value is expressed as a byte (0 - 255).
    * @param result      Vector holding all the colors in the gradient when
    *                    the blending is done.
    */
   static void blendImage( uint32 originColor,
                           uint32 blendColor,
                           int32 height,
                           uint8 minAlpha,
                           uint8 maxAlpha,
                           std::vector<uint32>& result );
   
private:
   /**
    * Blends two colors together using a alpha value.
    * @param orgColor   The original color that should be the
    *                   base of the blended color.
    * @param blendColor The blendColor, defines wich color that
    *                   the orgColor should be blended with.
    * @param alpha      The alpha value expressed in a byte (0 -255)
    *                   With a high alpha value, the blended color
    *                   will look a lot like the blendColor and with
    *                   a small alpha value, the blended color will look
    *                   a lot like the orgColor.
    */
   static uint32 calcBlendedColor( uint32 orgColor,
                                   uint32 blendColor,
                                   uint8  alpha );
   
};

#endif
