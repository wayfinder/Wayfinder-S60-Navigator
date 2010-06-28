/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbPolySphere.h"
#include "gbRayMath.h"
#include "gbGtx.h"

#ifndef USE_GLES
#	include "gbPoly.h"
#endif

#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define USE_QUADS
#define USE_SHADING

#ifdef USE_SHADING
#define LIGHT_TABLE_SIZE_SHIFT 6
#else
#define LIGHT_TABLE_SIZE_SHIFT 0
#endif
const GBuint32 LIGHT_TABLE_SIZE = 1 << LIGHT_TABLE_SIZE_SHIFT;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBbool	GBPolySphere_init				(GBPolySphere* sphere, const char* textureFilename);
static GBbool	GBPolySphere_generateMesh		(GBPolySphere* sphere);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBPolySphere_s
{
	GBDisplayParams			displayParams;
	GBMemory*				mem;

	GBuint16				detailLevel;

	GBuint32				vertexCount;
	GBuint32				vertexStride;
	GBint32*				vertices;
	GBint32*				vertices2;

	GBuint32				faceCount;
	GBuint32				indexCount;
	GBuint16*				indices;

	GBuint8**				faceTexPtr;
	GBOutputFormat*			faceColor;

	GBvec2					coords;
	GBfloat					distance;

	GBint32					finalMatrix[9];

	GBuint8*				texture;
	GBOutputFormat*			palette;

	GBuint32				xBlocks;
	GBuint32				yBlocks;
	GBuint32				blockWidth;
	GBuint32				blockHeight;
	GBuint32				uShift;
	GBuint32				vShift;

	GBOutputFormat*			blockColors;
	GBuint32*				blockIndices;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBPolySphere*
GBPolySphere_create (const char* textureFilename, GBMemory* mem)
{
	GBPolySphere* sphere;

	sphere = (GBPolySphere*)malloc(sizeof(GBPolySphere));
	if (!sphere)
		return NULL;

	GBDisplayParams_init(&sphere->displayParams);

	GB_ASSERT(mem);
	sphere->mem = mem;

	sphere->vertexCount = 0;
	sphere->vertexStride = 5 * sizeof(GBint32);
	sphere->vertices = NULL;
	sphere->vertices2 = NULL;

	sphere->faceCount = 0;
	sphere->indexCount = 0;
	sphere->indices = NULL;

	sphere->faceTexPtr = NULL;
	sphere->faceColor = NULL;

	sphere->coords[0] = 0.0f;
	sphere->coords[1] = 0.0f;
	sphere->distance = 0.0f;

	memset(sphere->finalMatrix, 0, sizeof(sphere->finalMatrix));

	sphere->texture = NULL;
	sphere->palette = NULL;

	sphere->xBlocks = 0;
	sphere->yBlocks = 0;
	sphere->blockWidth = 0;
	sphere->blockHeight = 0;
	sphere->uShift = 0;
	sphere->vShift = 0;

	sphere->blockColors = NULL;
	sphere->blockIndices = NULL;

	if (!GBPolySphere_init(sphere, textureFilename))
	{
		GBPolySphere_destroy(sphere);
		return NULL;
	}

	return sphere;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
GBPolySphere_destroy (GBPolySphere* sphere)
{
	SAFE_FREE(sphere->texture);
	SAFE_FREE(sphere->blockIndices);
	SAFE_FREE(sphere->blockColors);
	SAFE_FREE(sphere->palette);
	SAFE_FREE(sphere->faceColor);
	SAFE_FREE(sphere->faceTexPtr);
	SAFE_FREE(sphere->indices);
	SAFE_FREE(sphere->vertices2);
	SAFE_FREE(sphere->vertices);
	SAFE_FREE(sphere);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBPolySphere_setDisplay (GBPolySphere* sphere, GBDisplayParams* displayParams)
{
	GB_ASSERT(sphere && displayParams);

	sphere->displayParams = *displayParams;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
GBPolySphere_update (GBPolySphere* sphere, const GBvec2 coords, GBfloat distance)
{
	GBint32 rotationMatrix[9];
	GB_ASSERT(sphere);

	sphere->coords[0] = coords[0];

	sphere->coords[1] = coords[1];
	sphere->distance = distance;

	gbMatrixRotationY(-sphere->coords[0], rotationMatrix);
	gbMatrixRotationX(sphere->coords[1], sphere->finalMatrix);
	gbMatrixMultiply(sphere->finalMatrix, rotationMatrix, sphere->finalMatrix);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#ifdef ENABLE_SHOW_LINES
extern int g_showLines;
#endif

GBbool
GBPolySphere_draw (GBPolySphere* sphere)
{
	GB_ASSERT(sphere);

#ifdef USE_GLES
	/* This code is under contruction and is not tested. */

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(coords[0], 0.0f, 1.0f, 0.0f);
	glRotatef(coords[1], 1.0f, 0.0f, 0.0f);
	glTranslatef(0.0f, 0.0f, distance);

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FIXED, sphere->vertexStride, sphere->vertices);
	glTexCoordPointer(2, GL_FIXED, sphere->vertexStride, &sphere->vertices[3]);

	glDrawElements(GL_TRIANGLES, sphere->faceCount, GL_UNSIGNED_SHORT, sphere->indices);
#else
	gbDrawGradientBack(&sphere->displayParams);

	{
		GBint32 width = sphere->displayParams.width << 4;
		GBint32 height = sphere->displayParams.height << 4;
		GBint32 centerX = width >> 1;
		GBint32 centerY = height >> 1;

		GBint32 fov = (GBint32)(65536.0f * 16.0f * GB_FOV_CONSTANT * (float)sphere->displayParams.height);

		GBint32* src = sphere->vertices;
		GBint32* srcEnd = sphere->vertices + sphere->vertexCount * 5;
		GBint32* tgt = sphere->vertices2;

		GBint32 cameraZ = (GBint32)(65536.0f * 2.0f + 65536.0f * 4.0f * sphere->distance);
		if (cameraZ <= 65536)
			return GB_TRUE;

#define POLYGON_Z_CLIP 4096

		while (src < srcEnd)
		{
			GBint32 r[3];
			gbVec3TransformHalfQ(src, sphere->finalMatrix, r);

			if (r[2] < POLYGON_Z_CLIP)
			{
				tgt[0] = 0x7fffffff;
			}
			else
			{
#ifdef USE_SHADING
				GBint32 l = r[2] >> 8;
				l = (l * l) >> 9;
				if (l < 0)
					l = 0;
				else if (l > 63)
					l = 63;
#endif
				r[2] = r[2] - cameraZ;
				r[2] = fov / r[2];

//				tgt[0] = centerX + ((r[0] * fov) / r[2]);
//				tgt[1] = centerY - ((r[1] * fov) / r[2]);
				tgt[0] = centerX + ((r[0] * r[2]) >> 16);
#ifdef USE_SHADING
				tgt[1] = ((0x7fff + centerY - ((r[1] * r[2]) >> 16)) & 0xffff) | ((l & 0xffff) << 16);
#else
				tgt[1] = ((0x7fff + centerY - ((r[1] * r[2]) >> 16)) & 0xffff);
#endif
			}

			src += 5;
			tgt += 2;
		}
	}
	{
		GBint32 width = sphere->displayParams.width << 4;
		GBint32 height = sphere->displayParams.height << 4;

		GBOutputFormat* buffer = (GBOutputFormat*)sphere->displayParams.buffer + sphere->displayParams.bufferWidth * sphere->displayParams.y;

		GBuint16* ind = sphere->indices;
		GBuint16* indEnd = sphere->indices + sphere->indexCount;

		GBuint8** texPtr = sphere->faceTexPtr;
		GBOutputFormat* color = sphere->faceColor;
#ifndef USE_QUADS
		GBbool odd = GB_FALSE;
#endif
#ifdef USE_QUADS
#define VERTICES_PER_POLYGON 4
#else
#define VERTICES_PER_POLYGON 3
#endif

#ifdef USE_SHADING
		GBint32 vertices[5 * VERTICES_PER_POLYGON];
#else
		GBint32 vertices[4 * VERTICES_PER_POLYGON];
#endif

		while (ind < indEnd)
		{
			GBint32 leftCount = 0;
			GBint32 rightCount = 0;
			GBint32 topCount = 0;
			GBint32 bottomCount = 0;

			GBint32* out = vertices;
			GBuint32 i;
			for (i = 0; i < VERTICES_PER_POLYGON; i++)
			{
				GBuint32 off = ind[i];
				GBint32* in = &sphere->vertices2[off << 1];
				GBint32* inTex;

				if (in[0] == 0x7fffffff)
					break;

				inTex = &sphere->vertices[off + (off << 2)];

				out[0] = in[0];
				out[1] = (in[1] & 0xffff) - 0x7fff;
				out[2] = inTex[3];
				out[3] = inTex[4];
#ifdef USE_SHADING
				out[4] = in[1] >> 16;
#endif

				if (out[0] < 0)
					leftCount++;
				else if (out[0] >= width)
					rightCount++;
				if (out[1] < 0)
					topCount++;
				else if (out[1] >= height)
					bottomCount++;

#ifdef USE_SHADING
				out += 5;
#else
				out += 4;
#endif
			}

			if (i == VERTICES_PER_POLYGON && leftCount < VERTICES_PER_POLYGON && rightCount < VERTICES_PER_POLYGON && topCount < VERTICES_PER_POLYGON && bottomCount < VERTICES_PER_POLYGON)
			{
#ifdef USE_SHADING
#ifdef ENABLE_SHOW_LINES
#else
#ifdef USE_QUADS
#else
				if (*texPtr)
					gbDrawShadedTexTriangle(vertices, *texPtr, sphere->palette, sphere->uShift, sphere->vShift, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
				else
					gbDrawShadedTriangle(vertices, color, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
#endif
#endif
#else
#ifdef ENABLE_SHOW_LINES
				if (*texPtr)
					gbDrawTexQuad(vertices, *texPtr, sphere->palette, sphere->uShift, sphere->vShift, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer, g_showLines);
				else
					gbDrawQuad(vertices, *color, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer, g_showLines);
#else
#ifdef USE_QUADS
				if (*texPtr)
					gbDrawTexQuad(vertices, *texPtr, sphere->palette, sphere->uShift, sphere->vShift, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
				else
					gbDrawQuad(vertices, *color, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
#else
				if (*texPtr)
					gbDrawTexTriangle(vertices, *texPtr, sphere->palette, sphere->uShift, sphere->vShift, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
				else
					gbDrawTriangle(vertices, *color, sphere->displayParams.bufferWidth, sphere->displayParams.height, buffer);
#endif
#endif
#endif
			}

			ind += VERTICES_PER_POLYGON;

#ifdef USE_QUADS
			texPtr++;
			color += LIGHT_TABLE_SIZE;
#else
			if (odd)
			{
				texPtr++;
				color += LIGHT_TABLE_SIZE;
			}
			odd = !odd;
#endif
		}
	}
#endif // USE_GLES

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBPolySphere_init (GBPolySphere* sphere, const char* textureFilename)
{
	GBGtx* gtx;
	GB_ASSERT(sphere && textureFilename);

	gtx = GBGtx_load(textureFilename, NULL);
	if (!gtx)
		return GB_FALSE;

	{
		GBuint32 i;
		GBuint8* p = gtx->palette;
		sphere->palette = (GBOutputFormat*)malloc(sizeof(GBOutputFormat) * 256 * LIGHT_TABLE_SIZE);
		for (i = 0; i < 256; i++)
		{
			GBuint32 c = i * 3;
			GBuint32 l;
			for (l = 0; l < LIGHT_TABLE_SIZE; l++)
			{
				GBuint32 r = (p[c + 2] * (l + 1)) >> LIGHT_TABLE_SIZE_SHIFT;
				GBuint32 g = (p[c + 1] * (l + 1)) >> LIGHT_TABLE_SIZE_SHIFT;
				GBuint32 b = (p[c] * (l + 1)) >> LIGHT_TABLE_SIZE_SHIFT;
				sphere->palette[i * LIGHT_TABLE_SIZE + l] = 
#if defined(GB_PIXELFORMAT_888)
					(GBOutputFormat)((r << 16) | (g << 8) | b);
#elif defined(GB_PIXELFORMAT_444)
					(GBOutputFormat)(((r << 4) & 0xf00) | (g & 0xf0) | (b >> 4));
#elif defined(GB_PIXELFORMAT_565)
					(GBOutputFormat)(((r << 8) & 0xf800) | ((g << 3) & 0x7e0) | (b >> 3));
#endif
			}
		}
	}

	{
		GBuint32 x;
		GBuint32 y;
		GBuint32 blockCount = 0;

		sphere->xBlocks = gtx->width / gtx->blockWidth;
		sphere->yBlocks = gtx->height / gtx->blockHeight;
		sphere->blockWidth = gtx->blockWidth;
		sphere->blockHeight = gtx->blockHeight;

		sphere->uShift = gbHighestBit(sphere->blockWidth);
		sphere->vShift = gbHighestBit(sphere->blockHeight);

		for (y = 0; y < sphere->yBlocks; y++)
		{
			for (x = 0; x < sphere->xBlocks; x++)
			{
				GBuint32 off = x + y * sphere->xBlocks;
				if (!(gtx->infoMap[off].flags & GB_GTX_BLOCK_FLAG_PLAIN))
					blockCount++;
			}
		}

		sphere->blockColors = (GBOutputFormat*)malloc(sizeof(GBOutputFormat) * LIGHT_TABLE_SIZE * sphere->xBlocks * sphere->yBlocks);
		sphere->blockIndices = (GBuint32*)malloc(sizeof(GBuint32) * sphere->xBlocks * sphere->yBlocks);
		if (!sphere->blockColors || !sphere->blockIndices)
		{
			GBGtx_destroy(gtx, GB_FALSE, NULL);
			return GB_FALSE;
		}

		for (y = 0; y < sphere->yBlocks; y++)
		{
			for (x = 0; x < sphere->xBlocks; x++)
			{
				GBuint32 i;
				GBuint32 off = x + y * sphere->xBlocks;
				GBGtxBlock* block = &gtx->infoMap[off];

				for (i = 0; i < LIGHT_TABLE_SIZE; i++)
                    sphere->blockColors[off * LIGHT_TABLE_SIZE + i] = sphere->palette[block->color * LIGHT_TABLE_SIZE + i];
/*
#if defined(GB_PIXELFORMAT_888)
				sphere->blockColors[off] = (sphere->palette[block->color * 3 + 2] << 16) |
					(sphere->palette[block->color * 3 + 1] << 8) |
					sphere->palette[block->color * 3];
#elif defined(GB_PIXELFORMAT_444)
				sphere->blockColors[off] = (GBOutputFormat)((8sphere->palette[block->color * 3 + 2] & 0xf0) << 4) |
					(sphere->palette[block->color * 3 + 1] & 0xf0) |
					(sphere->palette[block->color * 3] >> 4));
#elif defined(GB_PIXELFORMAT_565)
				sphere->blockColors[off] = (GBOutputFormat)(((((sphere->palette[block->color * 3 + 2] << 16) & 0xf80000) >> 8) | (((sphere->palette[block->color * 3 + 1] << 8) >> 5) & 0x7e0) | ((sphere->palette[block->color * 3] & 0xff) >> 3)));
#endif
*/
				if (block->flags & GB_GTX_BLOCK_FLAG_PLAIN)
					sphere->blockIndices[off] = 0xffffffff;
				else
					sphere->blockIndices[off] = block->dataIndex;
			}
		}

		GBGtx_destroy(gtx, GB_FALSE, NULL);

		sphere->texture = (GBuint8*)malloc(sizeof(GBuint8) * sphere->blockWidth * sphere->blockHeight * blockCount);
		if (!sphere->texture)
			return GB_FALSE;

		{
			FILE* f = fopen(textureFilename, "rb");
			if (!f)
				return GB_FALSE;

			for (y = 0; y < sphere->yBlocks; y++)
			{
				for (x = 0; x < sphere->xBlocks; x++)
				{
					GBuint32 off = x + y * sphere->xBlocks;
					if (sphere->blockIndices[off] != 0xffffffff)
					{
						if (!GBGtx_loadBlockToMem(f, x, y, sphere->texture + sphere->blockIndices[off] * sphere->blockWidth * sphere->blockHeight))
						{
							fclose(f);
							return GB_FALSE;
						}
					}
				}
			}

			fclose(f);
		}
	}

	if (!GBPolySphere_generateMesh(sphere))
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBPolySphere_generateMesh (GBPolySphere* sphere)
{
	GBuint16 iter = 1;
	GBuint16 sides = (GBuint16)((sphere->xBlocks) * iter) + 1;
	GBuint16 segments = (GBuint16)(sphere->yBlocks * iter) + 1;

	sphere->vertexCount = sides * segments;

#ifdef USE_QUADS
	sphere->faceCount = (sides - 1) * (segments - 1);
	sphere->indexCount = sphere->faceCount * 4;
#else
	sphere->faceCount = (sides - 1) * (segments - 1) * 2;
	sphere->indexCount = sphere->faceCount * 3;
#endif

	SAFE_FREE(sphere->vertices2);
	SAFE_FREE(sphere->vertices);
	SAFE_FREE(sphere->indices);

	sphere->vertices = (GBint32*)malloc(sphere->vertexCount * sphere->vertexStride);
	if (!sphere->vertices)
		return GB_FALSE;

	sphere->vertices2 = (GBint32*)malloc(sphere->vertexCount * 2 * sizeof(GBuint32));
	if (!sphere->vertices2)
		return GB_FALSE;

	sphere->indices = (GBuint16*)malloc(sphere->indexCount * sizeof(GBuint16));
	if (!sphere->indices)
		return GB_FALSE;

#ifdef USE_QUADS
	sphere->faceTexPtr = (GBuint8**)malloc(sphere->faceCount * sizeof(GBuint8*));
	if (!sphere->faceTexPtr)
		return GB_FALSE;

	sphere->faceColor = (GBOutputFormat*)malloc(sphere->faceCount * sizeof(GBOutputFormat) * LIGHT_TABLE_SIZE);
	if (!sphere->faceColor)
		return GB_FALSE;
#else
	sphere->faceTexPtr = (GBuint8**)malloc(sphere->faceCount / 2 * sizeof(GBuint8*));
	if (!sphere->faceTexPtr)
		return GB_FALSE;

	sphere->faceColor = (GBOutputFormat*)malloc(sphere->faceCount / 2 * sizeof(GBOutputFormat) * LIGHT_TABLE_SIZE);
	if (!sphere->faceColor)
		return GB_FALSE;
#endif

	{
		GBuint16 i;
		GBint32* vert = sphere->vertices;

		for (i = 0; i < segments; i++)
		{
			float y = (float)i / (float)(segments - 1);
			float ry = y * 3.14159265f;
			float ysin = (float)sin(ry);
			float ycos = (float)cos(ry);
			float v = (float)i / (float)iter;
			GBuint16 j;
			for (j = 0; j < sides; j++)
			{
				float t = (float)j / (float)(sides - 1);
				float r = t * 3.14159265f * 2.0f;

				float x = (float)cos(r) * ysin;
				float z = (float)sin(r) * ysin;
				float u = (float)j / (float)iter;

				*vert++ = (GBint32)(x * 65536.0f);
				*vert++ = (GBint32)(ycos * 65536.0f);
				*vert++ = (GBint32)(z * 65536.0f);
				*vert++ = (GBint32)(u * 65536.0f);
				*vert++ = (GBint32)(v * 65536.0f);
			}
		}
	}

	{
		GBuint16 i;
		GBuint16* ind = sphere->indices;
		GBuint16 c = 0;

		GBuint8** texPtr = sphere->faceTexPtr;
		GBOutputFormat* color = sphere->faceColor;

		for (i = 0; i < segments - 1; i++)
		{
			GBuint16 j;
			for (j = 0; j < sides - 1; j++)
			{
				GBuint32 l;
				GBuint8* ptr = NULL;
				GBuint32 off = (((j + (sides - 1) * 1 / 4) % (sides - 1)) / iter) + (i / iter) * sphere->xBlocks;
				if (sphere->blockIndices[off] != 0xffffffff)
					ptr = sphere->texture + sphere->blockIndices[off] * sphere->blockWidth * sphere->blockHeight;

#ifdef USE_QUADS
				*ind++ = c;
				*ind++ = c + 1;
				*ind++ = c + sides + 1;
				*ind++ = c + sides;
#else
				*ind++ = c;
				*ind++ = c + sides + 1;
				*ind++ = c + sides;

				*ind++ = c;
				*ind++ = c + 1;
				*ind++ = c + sides + 1;
#endif

				*texPtr++ = ptr;

				for (l = 0; l < LIGHT_TABLE_SIZE; l++)
					*color++ = sphere->blockColors[off * LIGHT_TABLE_SIZE + l];

				c++;
			}
			c++;
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
