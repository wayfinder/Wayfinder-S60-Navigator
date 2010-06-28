/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbClock.h"

#include "gbBitmap.h"
#include "gbPoly.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

const char* const GB_CLOCK_BACK_FILENAME = "clock.tga";

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBClock_s
{
	char* dataPath;

	GBuint32 time;
	GBint32 offset;

	GBBitmap* back;

	GBuint8* renderBuffer;

	GBuint32 size;
	GBuint32 left;
	GBuint32 top;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool		GBClock_init	(GBClock* clock);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBClock* GBClock_create (const char* dataPath)
{
	GBClock* clock;
	GB_ASSERT(dataPath);

	clock = (GBClock*)malloc(sizeof(GBClock));
	if (!clock)
		return NULL;

	clock->dataPath = gbStrDuplicate(dataPath);
	if (!clock->dataPath)
	{
		free(clock);
		return NULL;
	}

	clock->time = 0;
	clock->offset = 0;

	clock->back = NULL;

	clock->renderBuffer = NULL;

	clock->size = 0;
	clock->left = 0;
	clock->top = 0;

	if (!GBClock_init(clock))
	{
		GBClock_destroy(clock);
		return NULL;
	}

	return clock;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBClock_init (GBClock* clock)
{
	GB_ASSERT(clock);

	clock->back = GBBitmap_createFromTga(clock->dataPath, GB_CLOCK_BACK_FILENAME, NULL);
	if (!clock->back)
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBClock_destroy (GBClock* clock)
{
	GB_ASSERT(clock);

	free(clock->renderBuffer);

	if (clock->back)
		GBBitmap_destroy(clock->back, NULL);

	free(clock->dataPath);
	free(clock);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBClock_setSize (GBClock* clock, GBuint32 size)
{
	GB_ASSERT(clock);
	if (size == clock->size)
		return GB_TRUE;

	clock->size = size;

	free(clock->renderBuffer);
	clock->renderBuffer = (GBuint8*)malloc(sizeof(GBuint8) * clock->size * clock->size * 4);
	if (!clock->renderBuffer)
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBClock_setCorner (GBClock* clock, GBuint32 left, GBuint32 top)
{
	GB_ASSERT(clock);
	clock->left = left;
	clock->top = top;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBClock_setTime (GBClock* clock, GBuint32 time)
{
	GB_ASSERT(clock);
	clock->time = time;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBClock_setOffset (GBClock* clock, GBint32 offset)
{
	GB_ASSERT(clock);
	clock->offset = offset;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static void GBClock_drawArrow (GBClock* clock, float angle, float length)
{
	float length2 = length * 6.0f + 2.0f;
	float c1;
	float c2;
	GBint32 vertices[4 * 2];

	GB_ASSERT(clock && clock->renderBuffer);

	c1 = (float)clock->size * 0.6f;
	c2 = (float)clock->size * 0.8f;
	
	vertices[0] = ((clock->size << 4) - 1) + (GBint32)(sin(angle + GB_PI / 2.0f) * c1);
	vertices[1] = ((clock->size << 4) - 1) + (GBint32)(-cos(angle + GB_PI / 2.0f) * c1);
	vertices[2] = ((clock->size << 4) - 1) + (GBint32)(sin(angle + GB_PI / length2) * c2 * length);
	vertices[3] = ((clock->size << 4) - 1) + (GBint32)(-cos(angle + GB_PI / length2) * c2 * length);
	vertices[4] = ((clock->size << 4) - 1) + (GBint32)(sin(angle - GB_PI / length2) * c2 * length);
	vertices[5] = ((clock->size << 4) - 1) + (GBint32)(-cos(angle - GB_PI / length2) * c2 * length);
	vertices[6] = ((clock->size << 4) - 1) + (GBint32)(sin(angle - GB_PI / 2.0f) * c1);
	vertices[7] = ((clock->size << 4) - 1) + (GBint32)(-cos(angle - GB_PI / 2.0f) * c1);

	gbDrawPolygon(4, vertices, clock->size * 2, clock->size * 2, clock->renderBuffer);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBClock_draw (GBClock* clock, const GBDisplayParams* displayParams)
{
	float localtime;
	GB_ASSERT(clock && displayParams);

	localtime = (float)clock->time + (float)clock->offset;

	GBBitmap_draw(clock->back, clock->left, clock->top, 256, displayParams);

	memset(clock->renderBuffer, 0, clock->size * clock->size * 4);

	GBClock_drawArrow(clock, localtime * GB_PI * 2.0f / (60.0f * 60.0f), 15.0f);
	GBClock_drawArrow(clock, localtime * GB_PI * 2.0f / (60.0f * 60.0f * 12.0f), 8.0f);

	{
		GBuint32 x;
		GBuint32 y;
		GBOutputFormat* tgt = (GBOutputFormat*)displayParams->buffer;
		GBuint8* src = clock->renderBuffer;

		tgt += (displayParams->x + clock->left) + (displayParams->y + clock->top) * displayParams->bufferWidth;

		for (y = 0; y < clock->size; y++)
		{
			for (x = 0; x < clock->size; x++)
			{
				GBuint32 v = src[0] + src[1] + src[clock->size * 2] + src[clock->size * 2 + 1];
				if (v)
				{
#if defined(GB_PIXELFORMAT_888)
					GBuint32 c = *tgt;
					v = 256 - (v << 6);
					*tgt = ((((c & 0xff00ff) * v) >> 8) & 0xff00ff) | ((((c & 0xff00) * v) >> 8) & 0xff00);
#elif defined(GB_PIXELFORMAT_444)
					GBuint16 c = *tgt;
					v = 16 - (v << 2);
					*tgt = ((((c & 0xf0f) * v) >> 4) & 0xf0f) | ((((c & 0xf0) * v) >> 4) & 0xf0);
#elif defined(GB_PIXELFORMAT_565)
					GBuint16 c = *tgt;
					v = 32 - (v << 3);
					*tgt = ((((c & 0xf81f) * (v >> 1)) >> 4) & 0xf81f) | ((((c & 0x7e0) * v) >> 5) & 0x7e0);
#endif
				}
				src += 2;
				tgt++;
			}
			src += clock->size * 2;
			tgt += (GBint32)displayParams->bufferWidth - clock->size;
		}
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
