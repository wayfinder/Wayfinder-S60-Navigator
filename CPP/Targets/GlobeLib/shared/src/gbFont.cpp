/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbFont.h"

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#define		GB_FONT_CHAR_BEGIN		32
#define		GB_FONT_CHAR_END		128
#define		GB_FONT_CHAR_COUNT		(GB_FONT_CHAR_END - GB_FONT_CHAR_BEGIN)

struct GBFont_s
{
	GBBitmap*	bitmap;
	GBuint16*	charStart;
	GBuint16*	charWidths;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBbool		GBFont_init			(GBFont* font);
static void			GBFont_drawChar		(GBFont* font, GBuint8 ind, GBint32* pos, const GBDisplayParams* displayParams);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBFont* GBFont_create (GBBitmap* bitmap)
{
	GBFont* font;
	GB_ASSERT(bitmap);

	font = (GBFont*)malloc(sizeof(GBFont));
	if (!font)
		return NULL;

	font->bitmap = bitmap;

	font->charStart = NULL;
	font->charWidths = NULL;

	if (!GBFont_init(font))
	{
		GBFont_destroy(font);
		return NULL;
	}

	return font;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBFont_init (GBFont* font)
{
	GB_ASSERT(font);

	font->charStart = (GBuint16*)malloc(sizeof(GBuint16) * GB_FONT_CHAR_COUNT);
	if (!font->charStart)
		return GB_FALSE;

	font->charWidths = (GBuint16*)malloc(sizeof(GBuint16) * GB_FONT_CHAR_COUNT);
	if (!font->charWidths)
		return GB_FALSE;

	{
		GBuint16 i = 0;
		GBuint16 x = 0;
		GBuint16 width = 0;
		GBuint16 start = 0;
		GBuint16 count = GB_FONT_CHAR_COUNT;
		while (count--)
		{
			GBuint8 c = 0;
			do
			{
				x++;
				width++;
				c = font->bitmap->data[x * 3];
			}
			while (c == 0);

			font->charStart[i] = start;
			font->charWidths[i] = width;

			start = x;
			width = 0;
			i++;
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBFont_destroy (GBFont* font)
{
	GB_ASSERT(font);
	free(font->charWidths);
	free(font->charStart);
	free(font);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBFont_drawChar (GBFont* font, GBuint8 ind, GBint32* pos, const GBDisplayParams* displayParams)
{
	GBuint32 y;
	GBuint32 height;
	GB_ASSERT(font && pos && displayParams);

	if (ind == 0)
		return;

	height = font->bitmap->height - 1;

	for (y = 0; y < height; y++)
	{
		if (y + pos[1] >= 0 && y + pos[1] < displayParams->height)
		{
			GBuint8* src;
			GBOutputFormat* tgt = (GBOutputFormat*)displayParams->buffer + (pos[0] + displayParams->x + (y + pos[1] + displayParams->y) * displayParams->bufferWidth);
			GBuint16 x;
			GBuint16 width;

			width = font->charWidths[ind];

			src = font->bitmap->data + ((y + 1) * font->bitmap->width + font->charStart[ind]) * 3;

			for (x = 0; x < width; x++)
			{
				if (x + pos[0] >= (GBint32)displayParams->width)
					break;

				if (x + pos[0] >= 0)
				{
					GBuint8 l = src[0];
					if (l != 0)
					{
#if defined(GB_PIXELFORMAT_888)
						*tgt = (l << 16) | (l << 8) | l;
#elif defined(GB_PIXELFORMAT_444)
						l >>= 4;
						*tgt = (l << 8) | (l << 4) | l;
#elif defined(GB_PIXELFORMAT_565)
                        *tgt = ((l << 8) & 0xf800) | ((l << 3) & 0x7e0) | (l >> 3);
#endif
					}
				}

				src += 3;
				tgt++;
			}
		}
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBFont_drawText (GBFont* font, const char* text, GBint32 x, GBint32 y, const GBDisplayParams* displayParams)
{
	GBuint32 i;
	GBuint32 len;
	GBint32 p[2];
	GB_ASSERT(font && text && displayParams);

	len = gbStrLength(text);
	p[0] = x;
	p[1] = y;

	for (i = 0; i < len; i++)
	{
		GBuint8 c = text[i];
		if (c >= GB_FONT_CHAR_BEGIN && c < GB_FONT_CHAR_END)
		{
			GBFont_drawChar(font, c - GB_FONT_CHAR_BEGIN, p, displayParams);
			p[0] += font->charWidths[c - GB_FONT_CHAR_BEGIN];
		}
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBFont_drawClippedText (GBFont* font, const char* text, GBint32 x, GBint32 y, const GBDisplayParams* displayParams)
{
	GBint32 maxWidth;
	GBint32 dotWidth;
	GBuint32 i;
	GBuint32 len;
	GBint32 p[2];
	GB_ASSERT(font && text && displayParams);

	len = gbStrLength(text);
	p[0] = x;
	p[1] = y;

	maxWidth = displayParams->width - x;

	dotWidth = font->charWidths['.' - GB_FONT_CHAR_BEGIN];
	maxWidth -= dotWidth * 3;

	for (i = 0; i < len; i++)
	{
		GBuint8 c = text[i];
		if (c >= GB_FONT_CHAR_BEGIN && c < GB_FONT_CHAR_END)
		{
			GBint32 w = font->charWidths[c - GB_FONT_CHAR_BEGIN];
			maxWidth -= w;
			if (maxWidth <= 0)
				break;

			GBFont_drawChar(font, c - GB_FONT_CHAR_BEGIN, p, displayParams);
			p[0] += w;
		}
	}

	if (maxWidth <= 0)
	{
        GBFont_drawChar(font, '.' - GB_FONT_CHAR_BEGIN, p, displayParams);
		p[0] += dotWidth;
        GBFont_drawChar(font, '.' - GB_FONT_CHAR_BEGIN, p, displayParams);
		p[0] += dotWidth;
        GBFont_drawChar(font, '.' - GB_FONT_CHAR_BEGIN, p, displayParams);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 GBFont_getTextWidth (GBFont* font, const char* text)
{
	GBuint32 i;
	GBuint32 len;
	GBuint32 width = 0;
	GB_ASSERT(font && text);

	len = gbStrLength(text);

	for (i = 0; i < len; i++)
	{
		GBuint8 c = text[i];
		if (c >= GB_FONT_CHAR_BEGIN && c < GB_FONT_CHAR_END)
			width += font->charWidths[c - GB_FONT_CHAR_BEGIN];
	}

	return width;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 GBFont_getHeight (GBFont* font)
{
	GB_ASSERT(font);
	return font->bitmap->height;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
