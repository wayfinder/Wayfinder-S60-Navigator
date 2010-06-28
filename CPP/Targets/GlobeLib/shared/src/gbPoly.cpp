/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbPoly.h"

//_______________________________________________________________________________________________

const GBint32 KSubpixelMask = 15;
const GBint32 KSubpixelBits = 4;

const GBuint32 LIGHT_TABLE_SIZE_SHIFT = 6;

//_______________________________________________________________________________________________

void
gbDrawPolygon (GBuint32 primSize, GBint32* vertices,
			   GBuint32 width, GBuint32 height, GBuint8* out)
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBuint8* vStart = out;

	GB_ASSERT(vertices && primSize >= 3 && out);

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < primSize; i++)
	{
		if (vertices[i * 2 + 1] < topY)
		{
			topIndex = i;
			topY = vertices[i * 2 + 1];
		}
		else if (vertices[i * 2 + 1] > bottomY)
		{
			bottomY = vertices[i * 2 + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBuint8* hStart;
		GBuint8* hEnd;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= (GBint32)primSize)
					leftIndex -= (GBint32)primSize;

				leftEdgeCounter = gbCeil4(vertices[leftIndex * 2 + 1]) - top;
			}

			leftEdgeYDelta = vertices[leftIndex * 2 + 1] - vertices[lastLeftIndex * 2 + 1];

			if (vertices[leftIndex * 2] != vertices[lastLeftIndex * 2])
				leftEdgeXDelta = ((vertices[leftIndex * 2] - vertices[lastLeftIndex * 2]) << 16) / leftEdgeYDelta;
			else
				leftEdgeXDelta = 0;

			ySub = vertices[lastLeftIndex * 2 + 1] & KSubpixelMask;
			ySubMul = 0;
			if (ySub)
				ySubMul = KSubpixelMask - ySub;

			leftEdgeX = vertices[lastLeftIndex * 2] << 12;
			if (ySubMul)
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += (GBint32)primSize;

				rightEdgeCounter = gbCeil4(vertices[rightIndex * 2 + 1]) - top;
			}

			rightEdgeYDelta = vertices[rightIndex * 2 + 1] - vertices[lastRightIndex * 2 + 1];

			if (vertices[rightIndex * 2] != vertices[lastRightIndex * 2])
				rightEdgeXDelta = ((vertices[rightIndex * 2] - vertices[lastRightIndex * 2]) << 16) / rightEdgeYDelta;
			else
				rightEdgeXDelta = 0;

			ySub = vertices[lastRightIndex * 2 + 1] & KSubpixelMask;
			ySubMul = 0;
			if (ySub)
				ySubMul = KSubpixelMask - ySub;

			rightEdgeX = vertices[lastRightIndex * 2] << 12;
			if (ySubMul)
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		// Positive X clipping
		right = gbCeilA(rightEdgeX, 16);
		if (right >= (GBint32)width)
			right = width;

		hStart = vStart + gbCeilA(leftEdgeX, 16);
		hEnd = vStart + right;

		while (hStart < hEnd)
			*hStart++ = 1;

		vStart += width;
		top++;
	}
}


//_______________________________________________________________________________________________

#ifdef ENABLE_SHOW_LINES
void
gbDrawTexTriangle (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawTexTriangle (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;
	GBint32 leftEdgeU = 0;
	GBint32 leftEdgeV = 0;
	GBint32 leftEdgeUDelta = 0;
	GBint32 leftEdgeVDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 hUDelta;
	GBint32 hVDelta;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

	const GBuint32 uMask = (1 << uShift) - 1;
	const GBuint32 vMask = ((1 << vShift) - 1) << uShift;
	vShift += uShift;
	uShift = 18 - uShift;
	vShift = 18 - vShift;

	GB_ASSERT(vertices && tex && palette && out);

	{
		GBint32 areaYDelta1 = vertices[(0 << 2) + 1] - vertices[(2 << 2) + 1];
		GBint32 areaYDelta2 = vertices[(1 << 2) + 1] - vertices[(2 << 2) + 1];
		GBint32 area = ((vertices[1 << 2] - vertices[2 << 2]) * areaYDelta1 - (vertices[0 << 2] - vertices[2 << 2]) * areaYDelta2) >> 4;

		if (area <= 0)
			return;

		hUDelta = ((((vertices[(1 << 2) + 2] - vertices[(2 << 2) + 2]) * areaYDelta1 - (vertices[(0 << 2) + 2] - vertices[(2 << 2) + 2]) * areaYDelta2)) << 2) / area;
		hVDelta = ((((vertices[(1 << 2) + 3] - vertices[(2 << 2) + 3]) * areaYDelta1 - (vertices[(0 << 2) + 3] - vertices[(2 << 2) + 3]) * areaYDelta2)) << 2) / area;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 3; i++)
	{
		if (vertices[(i << 2) + 1] < topY)
		{
			topIndex = i;
			topY = vertices[(i << 2) + 1];
		}
		else if (vertices[(i << 2) + 1] > bottomY)
		{
			bottomY = vertices[(i << 2) + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		GBint32 hU;
		GBint32 hV;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;
//			GBint32 d;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 3)
					leftIndex -= 3;

				leftEdgeCounter = gbCeil4(vertices[(leftIndex << 2) + 1]) - top;
			}

			leftEdgeYDelta = vertices[(leftIndex << 2) + 1] - vertices[(lastLeftIndex << 2) + 1];

//			d = (1 << 20) / leftEdgeYDelta;

			leftEdgeXDelta = ((vertices[leftIndex << 2] - vertices[lastLeftIndex << 2]) << 16) / leftEdgeYDelta;
			leftEdgeUDelta = ((vertices[(leftIndex << 2) + 2] - vertices[(lastLeftIndex << 2) + 2]) << 6) / leftEdgeYDelta;
			leftEdgeVDelta = ((vertices[(leftIndex << 2) + 3] - vertices[(lastLeftIndex << 2) + 3]) << 6) / leftEdgeYDelta;
//			leftEdgeXDelta = ((vertices[leftIndex << 2] - vertices[lastLeftIndex << 2]) * d) >> 4;
//			leftEdgeUDelta = ((vertices[(leftIndex << 2) + 2] - vertices[(lastLeftIndex << 2) + 2]) * d) >> 14;
//			leftEdgeVDelta = ((vertices[(leftIndex << 2) + 3] - vertices[(lastLeftIndex << 2) + 3]) * d) >> 14;

			leftEdgeX = vertices[(lastLeftIndex << 2)] << 12;
			leftEdgeU = vertices[(lastLeftIndex << 2) + 2] << 2;
			leftEdgeV = vertices[(lastLeftIndex << 2) + 3] << 2;

			ySub = vertices[(lastLeftIndex << 2) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
				leftEdgeU += (leftEdgeUDelta * ySubMul) >> KSubpixelBits;
				leftEdgeV += (leftEdgeVDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeU += leftEdgeUDelta;
			leftEdgeV += leftEdgeVDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 3;

				rightEdgeCounter = gbCeil4(vertices[(rightIndex << 2) + 1]) - top;
			}

			rightEdgeYDelta = vertices[(rightIndex << 2) + 1] - vertices[(lastRightIndex << 2) + 1];

			rightEdgeXDelta = ((vertices[rightIndex << 2] - vertices[lastRightIndex << 2]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex << 2] << 12;

			ySub = vertices[(lastRightIndex << 2) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);

			GBint32 xSub = (leftEdgeX >> 12) & KSubpixelMask;
			if (xSub)
			{
				GBint32 xSubMul = KSubpixelMask - xSub;
				hU = leftEdgeU + ((hUDelta * xSubMul) >> KSubpixelBits);
				hV = leftEdgeV + ((hVDelta * xSubMul) >> KSubpixelBits);
			}
			else
			{
				hU = leftEdgeU;
				hV = leftEdgeV;
			}

			if (leftX < 0)
			{
				hU -= (hUDelta * (leftEdgeX >> 12)) >> 4;
				hV -= (hVDelta * (leftEdgeX >> 12)) >> 4;
				leftX = 0;
			}

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
			{
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
				hU += hUDelta;
				hV += hVDelta;
			}
#endif

			while (hStart < hEnd)
			{
//				*hStart++ = ((hU >> 12) & 0xff) | ((hV >> 4) & 0xff00);
//				GBuint32 c = tex[((hU >> (20 - uShift)) & uMask)];
				*hStart++ = palette[tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)]];
/*				GBuint32 c = tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)];
				c = c + (c << 1);

#if defined(GB_PIXELFORMAT_888)
				*hStart++ = (GBOutputFormat)((palette[c + 2] << 16) | (palette[c + 1] << 8) | palette[c]);
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 4) & 0xf00) | (palette[c + 1] & 0xf0) | (palette[c] >> 4));
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 8) & 0xf800) | ((palette[c + 1] << 3) & 0x7e0) | (palette[c] >> 3));
#endif
*/
				hU += hUDelta;
				hV += hVDelta;
			}
		}

		vStart += width;
		top++;
	}
}

#ifdef ENABLE_SHOW_LINES
void
gbDrawTriangle (GBint32* vertices, GBOutputFormat color, GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawTriangle (GBint32* vertices, GBOutputFormat color, GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

	GB_ASSERT(vertices && out);

	{
		GBint32 areaYDelta1 = vertices[(0 << 2) + 1] - vertices[(2 << 2) + 1];
		GBint32 areaYDelta2 = vertices[(1 << 2) + 1] - vertices[(2 << 2) + 1];
		GBint32 area = ((vertices[1 << 2] - vertices[2 << 2]) * areaYDelta1 - (vertices[0 << 2] - vertices[2 << 2]) * areaYDelta2);

		if (area <= 0)
			return;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 3; i++)
	{
		if (vertices[(i << 2) + 1] < topY)
		{
			topIndex = i;
			topY = vertices[(i << 2) + 1];
		}
		else if (vertices[(i << 2) + 1] > bottomY)
		{
			bottomY = vertices[(i << 2) + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 3)
					leftIndex -= 3;

				leftEdgeCounter = gbCeil4(vertices[(leftIndex << 2) + 1]) - top;
			}

			leftEdgeYDelta = vertices[(leftIndex << 2) + 1] - vertices[(lastLeftIndex << 2) + 1];

			leftEdgeXDelta = ((vertices[leftIndex << 2] - vertices[lastLeftIndex << 2]) << 16) / leftEdgeYDelta;

			leftEdgeX = vertices[lastLeftIndex << 2] << 12;

			ySub = vertices[(lastLeftIndex << 2) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 3;

				rightEdgeCounter = gbCeil4(vertices[(rightIndex << 2) + 1]) - top;
			}

			rightEdgeYDelta = vertices[(rightIndex << 2) + 1] - vertices[(lastRightIndex << 2) + 1];

			rightEdgeXDelta = ((vertices[rightIndex << 2] - vertices[lastRightIndex << 2]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex << 2] << 12;

			ySub = vertices[(lastRightIndex << 2) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);
			if (leftX < 0)
				leftX = 0;

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
#endif

			while (hStart < hEnd)
			{
				*hStart++ = color;
			}
		}

		vStart += width;
		top++;
	}
}

//_______________________________________________________________________________________________

#ifdef ENABLE_SHOW_LINES
void
gbDrawTexQuad (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawTexQuad (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;
	GBint32 leftEdgeU = 0;
	GBint32 leftEdgeV = 0;
	GBint32 leftEdgeUDelta = 0;
	GBint32 leftEdgeVDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 hUDelta;
	GBint32 hVDelta;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

#define AREA_SHIFT 2

	const GBuint32 uMask = (1 << uShift) - 1;
	const GBuint32 vMask = ((1 << vShift) - 1) << uShift;
	vShift += uShift;
	uShift = (16 + AREA_SHIFT) - uShift;
	vShift = (16 + AREA_SHIFT) - vShift;

	GB_ASSERT(vertices && tex && palette && out);

	{
		GBint32 area1YDelta1 = vertices[0 * 4 + 1] - vertices[2 * 4 + 1];
		GBint32 area1YDelta2 = vertices[1 * 4 + 1] - vertices[2 * 4 + 1];
		GBint32 area1 = ((vertices[1 * 4] - vertices[2 * 4]) * area1YDelta1 - (vertices[0 * 4] - vertices[2 * 4]) * area1YDelta2) >> 4;

		GBint32 area2YDelta1 = vertices[0 * 4 + 1] - vertices[3 * 4 + 1];
		GBint32 area2YDelta2 = vertices[2 * 4 + 1] - vertices[3 * 4 + 1];
		GBint32 area2 = ((vertices[2 * 4] - vertices[3 * 4]) * area2YDelta1 - (vertices[0 * 4] - vertices[3 * 4]) * area2YDelta2) >> 4;

		GBint32 areaR;
		GBint32 yDelta1R;
		GBint32 yDelta2R;
		GBint32 o;

		if (area1 < area2)
		{
			yDelta1R = area2YDelta1;
			yDelta2R = area2YDelta2;
			areaR = area2;
			o = 1;
		}
		else
		{
			yDelta1R = area1YDelta1;
			yDelta2R = area1YDelta2;
			areaR = area1;
			o = 0;
		}

		if (areaR <= 0)
			return;

		hUDelta = ((((vertices[(1 + o) * 4 + 2] - vertices[(2 + o) * 4 + 2]) * yDelta1R - (vertices[0 * 4 + 2] - vertices[(2 + o) * 4 + 2]) * yDelta2R)) << AREA_SHIFT) / areaR;
		hVDelta = ((((vertices[(1 + o) * 4 + 3] - vertices[(2 + o) * 4 + 3]) * yDelta1R - (vertices[0 * 4 + 3] - vertices[(2 + o) * 4 + 3]) * yDelta2R)) << AREA_SHIFT) / areaR;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 4; i++)
	{
		if (vertices[i * 4 + 1] < topY)
		{
			topIndex = i;
			topY = vertices[i * 4 + 1];
		}
		else if (vertices[i * 4 + 1] > bottomY)
		{
			bottomY = vertices[i * 4 + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		GBint32 hU;
		GBint32 hV;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 4)
					leftIndex -= 4;

				leftEdgeCounter = gbCeil4(vertices[leftIndex * 4 + 1]) - top;
			}

			leftEdgeYDelta = vertices[leftIndex * 4 + 1] - vertices[lastLeftIndex * 4 + 1];

			leftEdgeXDelta = ((vertices[leftIndex * 4] - vertices[lastLeftIndex * 4]) << 16) / leftEdgeYDelta;
			leftEdgeUDelta = ((vertices[leftIndex * 4 + 2] - vertices[lastLeftIndex * 4 + 2]) << (4 + AREA_SHIFT)) / leftEdgeYDelta;
			leftEdgeVDelta = ((vertices[leftIndex * 4 + 3] - vertices[lastLeftIndex * 4 + 3]) << (4 + AREA_SHIFT)) / leftEdgeYDelta;

			leftEdgeX = vertices[lastLeftIndex * 4] << 12;
			leftEdgeU = vertices[lastLeftIndex * 4 + 2] << AREA_SHIFT;
			leftEdgeV = vertices[lastLeftIndex * 4 + 3] << AREA_SHIFT;

			ySub = vertices[lastLeftIndex * 4 + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
				leftEdgeU += (leftEdgeUDelta * ySubMul) >> KSubpixelBits;
				leftEdgeV += (leftEdgeVDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeU += leftEdgeUDelta;
			leftEdgeV += leftEdgeVDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 4;

				rightEdgeCounter = gbCeil4(vertices[rightIndex * 4 + 1]) - top;
			}

			rightEdgeYDelta = vertices[rightIndex * 4 + 1] - vertices[lastRightIndex * 4 + 1];

			rightEdgeXDelta = ((vertices[rightIndex * 4] - vertices[lastRightIndex * 4]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex * 4] << 12;

			ySub = vertices[lastRightIndex * 4 + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);

			GBint32 xSub = (leftEdgeX >> 12) & KSubpixelMask;
			if (xSub)
			{
				GBint32 xSubMul = KSubpixelMask - xSub;
				hU = leftEdgeU + ((hUDelta * xSubMul) >> KSubpixelBits);
				hV = leftEdgeV + ((hVDelta * xSubMul) >> KSubpixelBits);
			}
			else
			{
				hU = leftEdgeU;
				hV = leftEdgeV;
			}

			if (leftX < 0)
			{
				hU -= (hUDelta * (leftEdgeX >> 12)) >> 4;
				hV -= (hVDelta * (leftEdgeX >> 12)) >> 4;
				leftX = 0;
			}

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
			{
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
				hU += hUDelta;
				hV += hVDelta;
			}
#endif

			while (hStart < hEnd)
			{
//				*hStart++ = ((hU >> 12) & 0xff) | ((hV >> 4) & 0xff00);
//				GBuint32 c = tex[((hU >> (20 - uShift)) & uMask)];
				*hStart++ = palette[tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)]];
/*				GBuint32 c = tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)];
				c = c + (c << 1);

#if defined(GB_PIXELFORMAT_888)
				*hStart++ = (GBOutputFormat)((palette[c + 2] << 16) | (palette[c + 1] << 8) | palette[c]);
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 4) & 0xf00) | (palette[c + 1] & 0xf0) | (palette[c] >> 4));
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 8) & 0xf800) | ((palette[c + 1] << 3) & 0x7e0) | (palette[c] >> 3));
#endif
*/
				hU += hUDelta;
				hV += hVDelta;
			}
		}

		vStart += width;
		top++;
	}
}

#ifdef ENABLE_SHOW_LINES
void
gbDrawQuad (GBint32* vertices, GBOutputFormat color, GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawQuad (GBint32* vertices, GBOutputFormat color, GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

	GB_ASSERT(vertices && out);

	{
		GBint32 areaYDelta1 = vertices[0 * 4 + 1] - vertices[2 * 4 + 1];
		GBint32 areaYDelta2 = vertices[1 * 4 + 1] - vertices[2 * 4 + 1];
		GBint32 area = ((vertices[1 * 4] - vertices[2 * 4]) * areaYDelta1 - (vertices[0 * 4] - vertices[2 * 4]) * areaYDelta2);

		if (area <= 0)
			return;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 4; i++)
	{
		if (vertices[i * 4 + 1] < topY)
		{
			topIndex = i;
			topY = vertices[i * 4 + 1];
		}
		else if (vertices[i * 4 + 1] > bottomY)
		{
			bottomY = vertices[i * 4 + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 4)
					leftIndex -= 4;

				leftEdgeCounter = gbCeil4(vertices[leftIndex * 4 + 1]) - top;
			}

			leftEdgeYDelta = vertices[leftIndex * 4 + 1] - vertices[lastLeftIndex * 4 + 1];

			leftEdgeXDelta = ((vertices[leftIndex * 4] - vertices[lastLeftIndex * 4]) << 16) / leftEdgeYDelta;

			leftEdgeX = vertices[lastLeftIndex * 4] << 12;

			ySub = vertices[lastLeftIndex * 4 + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 4;

				rightEdgeCounter = gbCeil4(vertices[rightIndex * 4 + 1]) - top;
			}

			rightEdgeYDelta = vertices[rightIndex * 4 + 1] - vertices[lastRightIndex * 4 + 1];

			rightEdgeXDelta = ((vertices[rightIndex * 4] - vertices[lastRightIndex * 4]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex * 4] << 12;

			ySub = vertices[lastRightIndex * 4 + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);
			if (leftX < 0)
				leftX = 0;

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
#endif

			while (hStart < hEnd)
			{
				*hStart++ = color;
			}
		}

		vStart += width;
		top++;
	}
}

#ifdef ENABLE_SHOW_LINES
void
gbDrawShadedTexTriangle (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawShadedTexTriangle (GBint32* vertices, GBuint8* tex, GBOutputFormat* palette,
					GBuint32 uShift, GBuint32 vShift,
					GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;
	GBint32 leftEdgeU = 0;
	GBint32 leftEdgeV = 0;
	GBint32 leftEdgeL = 0;
	GBint32 leftEdgeUDelta = 0;
	GBint32 leftEdgeVDelta = 0;
	GBint32 leftEdgeLDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 hUDelta;
	GBint32 hVDelta;
	GBint32 hLDelta;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

	const GBuint32 uMask = (1 << uShift) - 1;
	const GBuint32 vMask = ((1 << vShift) - 1) << uShift;
	vShift += uShift;
	uShift = 18 - uShift;
	vShift = 18 - vShift;

	GB_ASSERT(vertices && tex && palette && out);

	{
		GBint32 areaYDelta1 = vertices[(0 * 5) + 1] - vertices[(2 * 5) + 1];
		GBint32 areaYDelta2 = vertices[(1 * 5) + 1] - vertices[(2 * 5) + 1];
		GBint32 area = ((vertices[1 * 5] - vertices[2 * 5]) * areaYDelta1 - (vertices[0 * 5] - vertices[2 * 5]) * areaYDelta2) >> 4;

		if (area <= 0)
			return;

		hUDelta = ((((vertices[(1 * 5) + 2] - vertices[(2 * 5) + 2]) * areaYDelta1 - (vertices[(0 * 5) + 2] - vertices[(2 * 5) + 2]) * areaYDelta2)) << 2) / area;
		hVDelta = ((((vertices[(1 * 5) + 3] - vertices[(2 * 5) + 3]) * areaYDelta1 - (vertices[(0 * 5) + 3] - vertices[(2 * 5) + 3]) * areaYDelta2)) << 2) / area;
		hLDelta = ((((vertices[(1 * 5) + 4] - vertices[(2 * 5) + 4]) * areaYDelta1 - (vertices[(0 * 5) + 4] - vertices[(2 * 5) + 4]) * areaYDelta2)) << 16) / area;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 3; i++)
	{
		if (vertices[(i * 5) + 1] < topY)
		{
			topIndex = i;
			topY = vertices[(i * 5) + 1];
		}
		else if (vertices[(i * 5) + 1] > bottomY)
		{
			bottomY = vertices[(i * 5) + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		GBint32 hU;
		GBint32 hV;
		GBint32 hL;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;
//			GBint32 d;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 3)
					leftIndex -= 3;

				leftEdgeCounter = gbCeil4(vertices[(leftIndex * 5) + 1]) - top;
			}

			leftEdgeYDelta = vertices[(leftIndex * 5) + 1] - vertices[(lastLeftIndex * 5) + 1];

//			d = (1 << 20) / leftEdgeYDelta;

			leftEdgeXDelta = ((vertices[leftIndex * 5] - vertices[lastLeftIndex * 5]) << 16) / leftEdgeYDelta;
			leftEdgeUDelta = ((vertices[(leftIndex * 5) + 2] - vertices[(lastLeftIndex * 5) + 2]) << 6) / leftEdgeYDelta;
			leftEdgeVDelta = ((vertices[(leftIndex * 5) + 3] - vertices[(lastLeftIndex * 5) + 3]) << 6) / leftEdgeYDelta;
			leftEdgeLDelta = ((vertices[(leftIndex * 5) + 4] - vertices[(lastLeftIndex * 5) + 4]) << 20) / leftEdgeYDelta;
//			leftEdgeXDelta = ((vertices[leftIndex * 5] - vertices[lastLeftIndex * 5]) * d) >> 4;
//			leftEdgeUDelta = ((vertices[(leftIndex * 5) + 2] - vertices[(lastLeftIndex * 5) + 2]) * d) >> 14;
//			leftEdgeVDelta = ((vertices[(leftIndex * 5) + 3] - vertices[(lastLeftIndex * 5) + 3]) * d) >> 14;

			leftEdgeX = vertices[(lastLeftIndex * 5)] << 12;
			leftEdgeU = vertices[(lastLeftIndex * 5) + 2] << 2;
			leftEdgeV = vertices[(lastLeftIndex * 5) + 3] << 2;
			leftEdgeL = vertices[(lastLeftIndex * 5) + 4] << 16;

			ySub = vertices[(lastLeftIndex * 5) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
				leftEdgeU += (leftEdgeUDelta * ySubMul) >> KSubpixelBits;
				leftEdgeV += (leftEdgeVDelta * ySubMul) >> KSubpixelBits;
				leftEdgeL += (leftEdgeLDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeU += leftEdgeUDelta;
			leftEdgeV += leftEdgeVDelta;
			leftEdgeL += leftEdgeLDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 3;

				rightEdgeCounter = gbCeil4(vertices[(rightIndex * 5) + 1]) - top;
			}

			rightEdgeYDelta = vertices[(rightIndex * 5) + 1] - vertices[(lastRightIndex * 5) + 1];

			rightEdgeXDelta = ((vertices[rightIndex * 5] - vertices[lastRightIndex * 5]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex * 5] << 12;

			ySub = vertices[(lastRightIndex * 5) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);

			GBint32 xSub = (leftEdgeX >> 12) & KSubpixelMask;
			if (xSub)
			{
				GBint32 xSubMul = KSubpixelMask - xSub;
				hU = leftEdgeU + ((hUDelta * xSubMul) >> KSubpixelBits);
				hV = leftEdgeV + ((hVDelta * xSubMul) >> KSubpixelBits);
				hL = leftEdgeL + ((hLDelta * xSubMul) >> KSubpixelBits);
			}
			else
			{
				hU = leftEdgeU;
				hV = leftEdgeV;
				hL = leftEdgeL;
			}

			if (leftX < 0)
			{
				hU -= (hUDelta * (leftEdgeX >> 12)) >> 4;
				hV -= (hVDelta * (leftEdgeX >> 12)) >> 4;
				hL -= (hLDelta * (leftEdgeX >> 12)) >> 4;
				leftX = 0;
			}

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
			{
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
				hU += hUDelta;
				hV += hVDelta;
				hL += hLDelta;
			}
#endif

			while (hStart < hEnd)
			{
//				*hStart++ = ((hU >> 12) & 0xff) | ((hV >> 4) & 0xff00);
//				GBuint32 c = tex[((hU >> (20 - uShift)) & uMask)];
				*hStart++ = palette[(tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)] << LIGHT_TABLE_SIZE_SHIFT) + (hL >> 16)];
/*				GBuint32 c = tex[((hU >> uShift) & uMask) + ((hV >> vShift) & vMask)];
				c = c + (c << 1);

#if defined(GB_PIXELFORMAT_888)
				*hStart++ = (GBOutputFormat)((palette[c + 2] << 16) | (palette[c + 1] << 8) | palette[c]);
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 4) & 0xf00) | (palette[c + 1] & 0xf0) | (palette[c] >> 4));
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = (GBOutputFormat)(((palette[c + 2] << 8) & 0xf800) | ((palette[c + 1] << 3) & 0x7e0) | (palette[c] >> 3));
#endif
*/
				hU += hUDelta;
				hV += hVDelta;
				hL += hLDelta;
			}
		}

		vStart += width;
		top++;
	}
}

#ifdef ENABLE_SHOW_LINES
void
gbDrawShadedTriangle (GBint32* vertices, GBOutputFormat* color, GBuint32 width, GBuint32 height, GBOutputFormat* out, GBbool showLines)
#else
void
gbDrawShadedTriangle (GBint32* vertices, GBOutputFormat* color, GBuint32 width, GBuint32 height, GBOutputFormat* out)
#endif
{
	GBint32 leftIndex = 0;
	GBint32 leftEdgeCounter = 0;
	GBint32 leftEdgeX = 0;
	GBint32 leftEdgeXDelta = 0;
	GBint32 leftEdgeL = 0;
	GBint32 leftEdgeLDelta = 0;

	GBint32 rightIndex = 0;
	GBint32 rightEdgeCounter = 0;
	GBint32 rightEdgeX = 0;
	GBint32 rightEdgeXDelta = 0;

	GBint32 hLDelta;

	GBint32 topIndex = 0;
	GBint32 topY;
	GBint32 bottomY;
	GBint32 top;
	GBint32 bottom;

	GBuint32 i;

	GBOutputFormat* vStart = out;

	GB_ASSERT(vertices && out);

	{
		GBint32 areaYDelta1 = vertices[(0 * 5) + 1] - vertices[(2 * 5) + 1];
		GBint32 areaYDelta2 = vertices[(1 * 5) + 1] - vertices[(2 * 5) + 1];
		GBint32 area = ((vertices[1 * 5] - vertices[2 * 5]) * areaYDelta1 - (vertices[0 * 5] - vertices[2 * 5]) * areaYDelta2) >> 4;

		if (area <= 0)
			return;

		hLDelta = ((((vertices[(1 * 5) + 4] - vertices[(2 * 5) + 4]) * areaYDelta1 - (vertices[(0 * 5) + 4] - vertices[(2 * 5) + 4]) * areaYDelta2)) << 16) / area;
	}

	topY = vertices[1];
	bottomY = vertices[1];

	for (i = 1; i < 3; i++)
	{
		if (vertices[(i * 5) + 1] < topY)
		{
			topIndex = i;
			topY = vertices[(i * 5) + 1];
		}
		else if (vertices[(i * 5) + 1] > bottomY)
		{
			bottomY = vertices[(i * 5) + 1];
		}
	}

	leftIndex = topIndex;
	rightIndex = topIndex;

	top = gbCeil4(topY);
	bottom = gbCeil4(bottomY);
	vStart += top * width;

	if (bottom < 0)
		return;

	if (bottom >= (GBint32)height)
		bottom = (GBint32)height - 1;

	while (top < bottom)
	{
		GBint32 right;
		GBOutputFormat* hStart;
		GBOutputFormat* hEnd;

		GBint32 hL;

		if (leftEdgeCounter <= 0)
		{
			GBint32 lastLeftIndex = 0;
			GBint32 leftEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (leftEdgeCounter <= 0)
			{
				lastLeftIndex = leftIndex;

				leftIndex++;
				if (leftIndex >= 3)
					leftIndex -= 3;

				leftEdgeCounter = gbCeil4(vertices[(leftIndex * 5) + 1]) - top;
			}

			leftEdgeYDelta = vertices[(leftIndex * 5) + 1] - vertices[(lastLeftIndex * 5) + 1];

			leftEdgeXDelta = ((vertices[leftIndex * 5] - vertices[lastLeftIndex * 5]) << 16) / leftEdgeYDelta;
			leftEdgeLDelta = ((vertices[(leftIndex * 5) + 4] - vertices[(lastLeftIndex * 5) + 4]) << 20) / leftEdgeYDelta;

			leftEdgeX = vertices[lastLeftIndex * 5] << 12;
			leftEdgeL = vertices[(lastLeftIndex * 5) + 4] << 16;

			ySub = vertices[(lastLeftIndex * 5) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				leftEdgeX += (leftEdgeXDelta * ySubMul) >> KSubpixelBits;
				leftEdgeL += (leftEdgeLDelta * ySubMul) >> KSubpixelBits;
			}

			leftEdgeCounter--;
		}
		else
		{
			leftEdgeX += leftEdgeXDelta;
			leftEdgeL += leftEdgeLDelta;
			leftEdgeCounter--;
		}

		if (rightEdgeCounter <= 0)
		{
			GBint32 lastRightIndex = 0;
			GBint32 rightEdgeYDelta;
			GBint32 ySub;
			GBint32 ySubMul;

			while (rightEdgeCounter <= 0)
			{
				lastRightIndex = rightIndex;

				rightIndex--;
				if (rightIndex < 0)
					rightIndex += 3;

				rightEdgeCounter = gbCeil4(vertices[(rightIndex * 5) + 1]) - top;
			}

			rightEdgeYDelta = vertices[(rightIndex * 5) + 1] - vertices[(lastRightIndex * 5) + 1];

			rightEdgeXDelta = ((vertices[rightIndex * 5] - vertices[lastRightIndex * 5]) << 16) / rightEdgeYDelta;

			rightEdgeX = vertices[lastRightIndex * 5] << 12;

			ySub = vertices[(lastRightIndex * 5) + 1] & KSubpixelMask;
			if (ySub)
			{
				ySubMul = KSubpixelMask - ySub;
				rightEdgeX += (rightEdgeXDelta * ySubMul) >> KSubpixelBits;
			}

			rightEdgeCounter--;
		}
		else
		{
			rightEdgeX += rightEdgeXDelta;
			rightEdgeCounter--;
		}

		if (top >= 0)
		{
			GBint32 leftX = gbCeilA(leftEdgeX, 16);

			GBint32 xSub = (leftEdgeX >> 12) & KSubpixelMask;
			if (xSub)
			{
				GBint32 xSubMul = KSubpixelMask - xSub;
				hL = leftEdgeL + ((hLDelta * xSubMul) >> KSubpixelBits);
			}
			else
			{
				hL = leftEdgeL;
			}

			if (leftX < 0)
			{
				hL -= (hLDelta * (leftEdgeX >> 12)) >> 4;
				leftX = 0;
			}

			// Positive X clipping
			right = gbCeilA(rightEdgeX, 16);
			if (right >= (GBint32)width)
				right = width;

			hStart = vStart + leftX;
			hEnd = vStart + right;

#ifdef ENABLE_SHOW_LINES
			if (showLines && hStart < hEnd)
			{
#if defined(GB_PIXELFORMAT_888)
				*hStart++ = 0xffffffff;
#elif defined(GB_PIXELFORMAT_444)
				*hStart++ = 0xffff;
#elif defined(GB_PIXELFORMAT_565)
				*hStart++ = 0xffffff;
#endif
				hL += hLDelta;
			}
#endif

			while (hStart < hEnd)
			{
				*hStart++ = color[hL >> 16];
				hL += hLDelta;
			}
		}

		vStart += width;
		top++;
	}
}
