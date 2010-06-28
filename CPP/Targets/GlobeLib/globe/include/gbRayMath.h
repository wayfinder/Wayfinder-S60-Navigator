/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBRAYMATH_H
#define GBRAYMATH_H

#include "gbDefs.h"

//--------------------------------------------------------------------------//
// Field of view (vertical)
//--------------------------------------------------------------------------//

#define	GB_FOV_CONSTANT		2.95f

//--------------------------------------------------------------------------//
// Background gradient colors
//--------------------------------------------------------------------------//

#if defined(GB_PIXELFORMAT_888) || defined(GB_PIXELFORMAT_565)
#	define GB_BACKGROUND_TOP_COLOR			0x333366
#	define GB_BACKGROUND_BOTTOM_COLOR		0x6699cc
#elif defined(GB_PIXELFORMAT_444)
#	define GB_BACKGROUND_TOP_COLOR			0x336
#	define GB_BACKGROUND_BOTTOM_COLOR		0x6ad
#endif

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32		gbDistanceToCameraZ		(GBfloat distance);

GBbool		gbCastRayF				(GBfloat sx, GBfloat sy,
									 GBuint32 width, GBuint32 height,
									 GBuint32 cameraZ, GBfloat* out);

void		gbDrawGradientBack		(GBDisplayParams* displayParams);

#endif
