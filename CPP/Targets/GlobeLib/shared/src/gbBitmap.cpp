/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbBitmap.h"
#include "gbTga.h"

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBBitmap* GBBitmap_create (GBuint32 width, GBuint32 height, GBuint8* data, GBMemory* mem)
{
	GBBitmap* bitmap;
	GB_ASSERT(data);

	if (mem)
		bitmap = (GBBitmap*)GBMemory_alloc(mem, sizeof(GBBitmap));
	else
		bitmap = (GBBitmap*)malloc(sizeof(GBBitmap));

	if (!bitmap)
		return NULL;

	bitmap->width = width;
	bitmap->height = height;
	bitmap->data = data;

	return bitmap;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBBitmap* GBBitmap_createFromTga (const char* path, const char* filename, GBMemory* mem)
{
	GBBitmap* bitmap = NULL;
	GBTga* tga;
	GB_ASSERT(path && filename);

	{
		char full[256];
		gbStrCopy(full, path);
		strcat(full, filename);
		tga = GBTga_load(full, mem);
		if (!tga)
			return NULL;
	}

	{
		GBuint8* data;
		if (mem)
			data = (GBuint8*)GBMemory_alloc(mem, sizeof(GBuint8) * tga->width * tga->height * 3);
		else
			data = (GBuint8*)malloc(sizeof(GBuint8) * tga->width * tga->height * 3);

		if (data)
		{
			GBuint8* tgt = data;
			GBuint32 y;
			for (y = 0; y < tga->height; y++)
			{
				GBuint8* src = &((GBuint8*)tga->data)[(tga->height - 1 - y) * tga->width * 3];
				GBuint32 x;
				for (x = 0; x < tga->width; x++)
				{
					*tgt++ = *src++;
					*tgt++ = *src++;
					*tgt++ = *src++;
				}
			}
			bitmap = GBBitmap_create(tga->width, tga->height, data, mem);
			if (!bitmap)
				free(data);
		}
	}

	GBTga_destroy(tga, GB_FALSE, mem);

	return bitmap;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBBitmap_destroy (GBBitmap* bitmap, GBMemory* mem)
{
	GB_ASSERT(bitmap);
	if (mem)
	{
		GBMemory_free(mem, bitmap->data);
		GBMemory_free(mem, bitmap);
	}
	else
	{
		free(bitmap->data);
		free(bitmap);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBBitmap_draw (GBBitmap* bitmap, GBint32 x, GBint32 y, GBint32 mul, const GBDisplayParams* params)
{
	GBuint32 v;
	GBOutputFormat* ytgt;
	GB_ASSERT(bitmap && bitmap->data && params);

	if (x >= (GBint32)params->width ||
		x + (GBint32)(bitmap->width) < 0 ||
		y >= (GBint32)params->height ||
		y + (GBint32)(bitmap->height) < 0)
		return;

	ytgt = (GBOutputFormat*)params->buffer +
		params->bufferWidth * (params->y + y) +
		params->x + x;

	for (v = 0; v < bitmap->height; v++)
	{
		if (v + y >= 0 && v + y < params->height)
		{
			GBuint8* xdata = bitmap->data + 3 * bitmap->width * v;
			GBOutputFormat* tgt = ytgt;
			GBuint32 u;
			for (u = 0; u < bitmap->width; u++)
			{
				if (u + x >= 0 && u + x < params->width)
				{
					if (xdata[0] != 0 || xdata[1] != 0 || xdata[2] != 0)
					{
#if defined(GB_PIXELFORMAT_888)
						if (mul >= 255)
						{
                            *tgt = (xdata[2] << 16) | (xdata[1] << 8) | xdata[0];
						}
						else
						{
							GBuint32 rb = (((xdata[2] << 8) * mul) & 0xff0000) | (((xdata[0] * mul) >> 8) & 0xff);
							GBuint32 g = (xdata[1] * mul) & 0xff00;
							*tgt = rb | g;
						}
#elif defined(GB_PIXELFORMAT_444)
                        *tgt = ((xdata[2] << 4) & 0xf00) | (xdata[1] & 0xf0) | (xdata[0] >> 4);
#elif defined(GB_PIXELFORMAT_565)
                        *tgt = ((xdata[2] << 8) & 0xf800) | ((xdata[1] << 3) & 0x7e0) | (xdata[0] >> 3);
#endif
					}
				}
				xdata += 3;
				tgt++;
			}
		}
		ytgt += params->bufferWidth;
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
