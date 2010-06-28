/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbRayMath.h"

#include <math.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32
gbDistanceToCameraZ (GBfloat distance)
{
	return (GBint32)(512.0f + distance * 1024.0f);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
gbCastRayF (GBfloat sx, GBfloat sy,
			GBuint32 width, GBuint32 height,
			GBuint32 cameraZ, GBfloat* out)
{
	GBfloat dx;
	GBfloat dy;
	GBfloat dz;

	GBfloat b;
	GBfloat d;
	GBfloat t;

	GBfloat dlen;

	GBfloat viewDist;

	dx = (sx * 2.0f - (GBfloat)width) * 4.0f;
	dy = (sy * 2.0f - (GBfloat)height) * 4.0f;

	dz = (GBfloat)height * -8.0f * GB_FOV_CONSTANT;

	dlen = (GBfloat)sqrt(dx * dx + dy * dy + dz * dz);

	dx /= dlen;
	dy /= dlen;
	dz /= dlen;

	viewDist = ((GBfloat)cameraZ) / 256.0f;

	b = 2.0f * dz * viewDist;

	d = b * b - ((viewDist * viewDist) - 1.0f) * 4.0f;
	if (d < 0)
		return GB_FALSE;

	if (out)
	{
		GBfloat x;
		GBfloat y;

		if (d)
			t = -b - (GBfloat)sqrt(d);
		else
			t = -b;

		t /= 2.0f;

		x = dx * t;
		y = dy * t;
		out[2] = viewDist + dz * t;

		out[0] = x;
		out[1] = y;
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
gbDrawGradientBack (GBDisplayParams* displayParams)
{
	GBuint32 y;
	for (y = 0; y < displayParams->height; y++)
	{
		GBOutputFormat c;
		GBOutputFormat* tgt = (GBOutputFormat*)displayParams->buffer +
			(y + displayParams->y) * displayParams->bufferWidth + displayParams->x;
		GBOutputFormat* tgtEnd = tgt + displayParams->width;
		{
#if defined(GB_PIXELFORMAT_888)
			GBuint32 t = (y << 8) / displayParams->height;
			GBOutputFormat rb = ((((GB_BACKGROUND_TOP_COLOR & 0xff00ff) * (256 - t)) >> 8) & 0xff00ff) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xff00ff) * t) >> 8) & 0xff00ff);
			GBOutputFormat g = ((((GB_BACKGROUND_TOP_COLOR & 0xff00) * (256 - t)) >> 8) & 0xff00) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xff00) * t) >> 8) & 0xff00);
			c = rb | g;
#elif defined(GB_PIXELFORMAT_444)
			GBuint32 t = (y << 4) / displayParams->height;
			GBOutputFormat rb = (GBOutputFormat)(((((GB_BACKGROUND_TOP_COLOR & 0xf0f) * (16 - t)) >> 4) & 0xf0f) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xf0f) * t) >> 4) & 0xf0f));
			GBOutputFormat g = (GBOutputFormat)(((((GB_BACKGROUND_TOP_COLOR & 0xf0) * (16 - t)) >> 4) & 0xf0) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xf0) * t) >> 4) & 0xf0));
			c = rb | g;
#elif defined(GB_PIXELFORMAT_565)
			GBuint32 t = (y << 8) / displayParams->height;
			GBuint32 rb = ((((GB_BACKGROUND_TOP_COLOR & 0xff00ff) * (256 - t)) >> 8) & 0xff00ff) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xff00ff) * t) >> 8) & 0xff00ff);
			GBuint32 g = ((((GB_BACKGROUND_TOP_COLOR & 0xff00) * (256 - t)) >> 8) & 0xff00) + ((((GB_BACKGROUND_BOTTOM_COLOR & 0xff00) * t) >> 8) & 0xff00);
			c = (GBOutputFormat)((((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3)));
#endif
		}
		while (tgt < tgtEnd)
			*tgt++ = c;
	}
}

