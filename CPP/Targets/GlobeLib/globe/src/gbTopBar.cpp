/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbTopBar.h"

#include <string.h>
#include <stdlib.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

const char* const GB_TOPBAR_LOCATION_STR	= "I'm currently in";

const GBint32 GB_TOPBAR_HORIZONTAL_SPACING = 6;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBTopBar_s
{
	char*				dataPath;
	char*				locationString;

	GBFontInterface*	fontInterface;
	void*				fontContext;

	char*				text;

	GBint32				currentTime;
	GBint32				textStart;

	GBbool				scrolling;
	GBuint32			textAreaWidth;

	GBuint32			clockSize;
	GBuint32			locXOff;
	GBuint32			locYOff;
	GBuint32			textXOff;
	GBuint32			textYOff;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool		GBTopBar_init	(GBTopBar* topBar);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBTopBar* GBTopBar_create (const char* dataPath,
                           const char* locationString)
{
	GBTopBar* topBar;
	GB_ASSERT(dataPath);

	topBar = (GBTopBar*)malloc(sizeof(GBTopBar));
	if (!topBar)
		return NULL;

	topBar->dataPath = gbStrDuplicate(dataPath);
	if (!topBar->dataPath)
	{
		free(topBar);
		return NULL;
	}

   if ( locationString ) {
      topBar->locationString = gbStrDuplicate( locationString );
   } else {
      topBar->locationString = gbStrDuplicate( GB_TOPBAR_LOCATION_STR );
   }
	if (!topBar->locationString )
	{
		free(topBar);
		return NULL;
	}

	topBar->fontInterface = NULL;
	topBar->fontContext = NULL;

	topBar->text = NULL;

	topBar->currentTime = 0;
	topBar->textStart = 0;

	topBar->scrolling = GB_FALSE;
	topBar->textAreaWidth = 0;
	
	topBar->clockSize = 0;
	topBar->locXOff = 0;
	topBar->locYOff = 0;
	topBar->textXOff = 0;
	topBar->textYOff = 0;

	if (!GBTopBar_init(topBar))
	{
		GBTopBar_destroy(topBar);
		return NULL;
	}

	return topBar;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTopBar_init (GBTopBar* topBar)
{
	GB_ASSERT(topBar);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTopBar_destroy (GBTopBar* topBar)
{
	GB_ASSERT(topBar);
	free(topBar->text);
	free(topBar->dataPath);
	free(topBar->locationString);
	free(topBar);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTopBar_setFont (GBTopBar* topBar, GBFontInterface* fontInterface, void* fontContext)
{
	GB_ASSERT(topBar && fontInterface);
	topBar->fontInterface = fontInterface;
	topBar->fontContext = fontContext;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTopBar_setText (GBTopBar* topBar, const char* text)
{
	GB_ASSERT(topBar);

	if (topBar->text && strcmp(text, topBar->text) == 0)
		return GB_TRUE;

	topBar->textStart = 0;

	free(topBar->text);
	topBar->text = gbStrDuplicate(text);
	if (!topBar->text)
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTopBar_setLayout (GBTopBar* topBar, GBint32 clockSize,
						GBint32 locXOff, GBint32 locYOff,
						GBint32 textXOff, GBint32 textYOff)
{
	GB_ASSERT(topBar);
	
	topBar->clockSize = clockSize;
	topBar->locXOff = locXOff;
	topBar->locYOff = locYOff;
	topBar->textXOff = textXOff;
	topBar->textYOff = textYOff;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTopBar_draw (GBTopBar* topBar, GBint32 time, GBint32 step, const GBDisplayParams* displayParams)
{
	GB_ASSERT(topBar && displayParams);

	if (step > 0)
	{
		if (time > topBar->currentTime + 2000 || time < topBar->currentTime - 2000)
			topBar->currentTime = time;

		while (time > topBar->currentTime)
		{
			topBar->currentTime += step;
			topBar->textStart++;
		}
	}

	{
		GBuint32 step = 128 / displayParams->height;
		GBOutputFormat* tgty = (GBOutputFormat*)displayParams->buffer + displayParams->y * displayParams->bufferWidth + displayParams->x;
		GBuint32 y;
		GBuint32 tx1 = topBar->clockSize + GB_TOPBAR_HORIZONTAL_SPACING;
		GBuint32 ty1 = displayParams->height / 2 - 3;
		GBuint32 tx2 = displayParams->width - GB_TOPBAR_HORIZONTAL_SPACING;
		GBuint32 ty2 = displayParams->height - 3;

		for (y = 0; y < displayParams->height; y++)
		{
			GBOutputFormat* tgt = tgty;
			GBuint32 x;

			for (x = 0; x < displayParams->width; x++)
			{
				GBuint8 l;

				if (x >= tx1 && x < tx2 && y >= ty1 && y < ty2)
				{
					if (x == tx1 || x == tx2 - 1 || y == ty1 || y == ty2 - 1)
					{
						if ((x == tx1 && (y == ty1 || y == ty2 - 1)) || (x == tx2 - 1 && (y == ty1 || y == ty2 - 1)))
							if (y < displayParams->height >> 1)
								l = (GBuint8)(255 - y * step * 2);
							else
								l = (GBuint8)(96 + y * step / 2);
						else
							l = 64;
					}
					else
					{
						l = (GBuint8)((displayParams->height - y) * step / 2 + 210);
					}
				}
				else
				{
					if (y == displayParams->height - 1 || y == 0)
						l = 31;
					else if (y < displayParams->height >> 1)
						l = (GBuint8)(255 - y * step * 2);
					else
						l = (GBuint8)(96 + y * step / 2);
				}

#if defined(GB_PIXELFORMAT_888)
				*tgt++ = (l << 16) | (l << 8) | l;
#elif defined(GB_PIXELFORMAT_444)
				l >>= 4;
				*tgt++ = (l << 8) | (l << 4) | l;
#elif defined(GB_PIXELFORMAT_565)
				*tgt++ = (GBOutputFormat)(((l << 8) & 0xf800) | ((l << 3) & 0x7e0) | ((l & 0xff) >> 3));
#endif
			}
			tgty += displayParams->bufferWidth;
		}

	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTopBar_drawTexts (GBTopBar* topBar, const GBDisplayParams* displayParams)
{
	GBint32 maxWidth;
	GBint32 textWidth = 0;
	GBint32 maxScroll;
	GBint32 pos = 0;
	GBFontParams params;

	GB_ASSERT(topBar && displayParams);

	GBFontParams_init(&params);
	params.color = 0;
	topBar->fontInterface->setParams(topBar->fontContext, &params);

	maxWidth = displayParams->width - (topBar->clockSize + GB_TOPBAR_HORIZONTAL_SPACING + 12);
	if (topBar->fontInterface && topBar->text)
		textWidth = topBar->fontInterface->getWidth(topBar->fontContext, topBar->text);
	maxScroll = textWidth;// - maxWidth;
	if (maxScroll < 0)
		maxScroll = 0;

	if (textWidth > maxWidth)
	{
		topBar->scrolling = GB_TRUE;

		maxScroll += 16;
//		maxScroll += 30;

		pos = topBar->textStart % maxScroll;
//		pos = topBar->textStart % (maxScroll * 2);
//		if (pos > maxScroll)
//			pos = maxScroll * 2 - pos;

//		if (pos > maxScroll - 10)
//			pos = maxScroll - 10;

//		pos -= 10;
//		if (pos < 0)
//			pos = 0;
	}
	else
	{
		topBar->scrolling = GB_FALSE;
	}

	{
		GBuint32 tx1 = topBar->clockSize + GB_TOPBAR_HORIZONTAL_SPACING;
		GBuint32 ty1 = displayParams->height / 2 - 3;
		GBuint32 ty2 = displayParams->height - 3;

		if (topBar->fontInterface)
		{
			{
				GBTextRect rect;
				rect.left = displayParams->x;
				rect.top = displayParams->y;
				rect.right = displayParams->x + displayParams->width;
				rect.bottom = displayParams->y + displayParams->height;
				rect.x = tx1 + 3 + topBar->locXOff;
				rect.y = 3 + topBar->locYOff;
				topBar->fontInterface->render(topBar->fontContext, topBar->locationString, &rect);
			}
			if (topBar->text)
			{
				if (topBar->scrolling)
				{
					GBTextRect rect;
					rect.left = tx1 + 3;
					rect.top = displayParams->y;
					rect.right = rect.left + maxWidth;
					rect.bottom = displayParams->y + displayParams->height;

					rect.x = -pos + topBar->textXOff;
					rect.y = (ty1 + ty2) / 2 - 6 + topBar->textYOff;

					topBar->fontInterface->render(topBar->fontContext, topBar->text, &rect);

					rect.x = -pos + textWidth + 16;

					topBar->fontInterface->render(topBar->fontContext, topBar->text, &rect);
				}
				else
				{
					GBTextRect rect;
					rect.x = topBar->textXOff;
					rect.y = (ty1 + ty2) / 2 - 6 + topBar->textYOff;
					rect.left = tx1 + 3;
					rect.top = displayParams->y;
					rect.right = rect.left + maxWidth;
					rect.bottom = displayParams->y + displayParams->height;
					topBar->fontInterface->render(topBar->fontContext, topBar->text, &rect);
				}
			}
		}
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTopBar_isScrolling (GBTopBar* topBar, const GBDisplayParams* displayParams)
{
	GBint32 maxWidth;
	GBint32 textWidth = 0;
	GBint32 maxScroll;

	GB_ASSERT(topBar && displayParams);

	maxWidth = displayParams->width - (topBar->clockSize + GB_TOPBAR_HORIZONTAL_SPACING + 12);

	if (topBar->fontInterface && topBar->text)
		textWidth = topBar->fontInterface->getWidth(topBar->fontContext, topBar->text);

	maxScroll = textWidth - maxWidth;
	if (maxScroll < 0)
		maxScroll = 0;

	if (maxScroll)
		return GB_TRUE;

	return GB_FALSE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
