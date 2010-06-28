/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbRaySphere.h"
#include "gbTextureCache.h"
#include "gbRayMath.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

//--------------------------------------------------------------------------//
// Interpolation block definitions
//--------------------------------------------------------------------------//

#define GB_GLOBE_BLOCK_SIZE_SHIFT_W		3
#define GB_GLOBE_BLOCK_SIZE_SHIFT_H		3
#define GB_GLOBE_BLOCK_SIZE_W			(1 << GB_GLOBE_BLOCK_SIZE_SHIFT_W)
#define GB_GLOBE_BLOCK_SIZE_H			(1 << GB_GLOBE_BLOCK_SIZE_SHIFT_H)
#define GB_GLOBE_BLOCK_SIZE_MASK_W		(GB_GLOBE_BLOCK_SIZE_W - 1)
#define GB_GLOBE_BLOCK_SIZE_MASK_H		(GB_GLOBE_BLOCK_SIZE_H - 1)

#define GB_GLOBE_NORMAL_Z_TOLERANCE		24000

//--------------------------------------------------------------------------//
// Interpolation block struct
//--------------------------------------------------------------------------//

typedef struct GBNormalBlock_s
{
	GBbool accurate;
	GBbool empty;
	GBint16* data;
} GBNormalBlock;

//--------------------------------------------------------------------------//
// Arctan table parameters
//--------------------------------------------------------------------------//

#define GB_ATAN_TABLE_LENGTH_SHIFT	10
#define GB_ATAN_TABLE_LENGTH		(1 << GB_ATAN_TABLE_LENGTH_SHIFT)
#define GB_ATAN_TABLE_MASK			(GB_ATAN_TABLE_LENGTH - 1)
#define GB_ATAN_TABLE_SHIFT_IN		9
#define GB_ATAN_TABLE_SHIFT_OUT		14
#define GB_ATAN_TABLE_LERP_BITS		8
#define GB_ATAN_TABLE_LERP_MASK		((1 << GB_ATAN_TABLE_LERP_BITS) - 1)

//--------------------------------------------------------------------------//
// Bilinear parameters
//--------------------------------------------------------------------------//

//#define GB_ENABLE_BILINEAR

#define	GB_BILINEAR_BITS			3
#define GB_BILINEAR_MASK			((1 << GB_BILINEAR_BITS) - 1)

//--------------------------------------------------------------------------//
// Mapping parameters
//--------------------------------------------------------------------------//

#define GB_RENDER_WRAP_TOLERANCE_SHIFT	2
#define GB_RENDER_LERP_BITS				(GB_GLOBE_BLOCK_SIZE_SHIFT_W + GB_GLOBE_BLOCK_SIZE_SHIFT_H + GB_BILINEAR_BITS + 8)
#define GB_RENDER_EXTRA_BITS_IN_UV		12
#define GB_RENDER_POLE_TOLERANCE		32500

//--------------------------------------------------------------------------//
// Ray direction table parameters
//--------------------------------------------------------------------------//

#define GB_DIRTABLE_SIZE_DOWN_SHIFT		3
#define GB_DIRTABLE_SIZE_LENGTH			(1 << GB_DIRTABLE_SIZE_DOWN_SHIFT)
#define GB_DIRTABLE_SIZE_MASK			(GB_DIRTABLE_SIZE_LENGTH - 1)

//--------------------------------------------------------------------------//
// Ray direction interpolation parameters
//--------------------------------------------------------------------------//

#define GB_RAY_BLOCK_H_SPAN_SHIFT		3
#define GB_RAY_BLOCK_H_SPAN_LENGTH		(1 << GB_RAY_BLOCK_H_SPAN_SHIFT)
#define GB_RAY_BLOCK_H_SPAN_MASK		(GB_RAY_BLOCK_H_SPAN_LENGTH - 1)

//--------------------------------------------------------------------------//
// Interpolation data struct for rendering
//--------------------------------------------------------------------------//

typedef struct GBLerpData_s
{
	GBint32 normal[3];
	GBint32 u;
	GBint32 v;
	GBint32 l;
} GBLerpData;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBbool	GBRaySphere_init				(GBRaySphere* sphere, const char* textureFilename);

static GBbool	GBRaySphere_renderInit			(GBRaySphere* sphere);

static GBbool	GBRaySphere_castRay				(GBRaySphere* sphere,
												GBuint32 sx, GBuint32 sy,
												GBint16* out);

static GBbool	GBRaySphere_calculateSphere		(GBRaySphere* sphere);
static void		GBRaySphere_calculateDirTable	(GBRaySphere* sphere);

static GBint32	GBRaySphere_atan				(GBRaySphere* sphere, GBint32 p);
static void		GBRaySphere_initLerpData		(GBRaySphere* sphere, GBLerpData* data,
												 const GBint16* normalSrc,
												 const GBSplitTexture* tex);
static GBbool	GBRaySphere_renderSub			(GBRaySphere* sphere);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBRaySphere_s
{
	GBMemory*				mem;

	GBDisplayParams			displayParams;

	GBTextureCache*			texCache;

	GBuint32				bWidth;
	GBuint32				bHeight;
	GBNormalBlock*			blocks;

	GBbool					reallocateDirTable;
	GBbool					updateDirTable;
	GBbool					updateSphere;

	GBint32*				dirTable;
	GBuint32				dirTableWidth;
	GBuint32				dirTableHeight;
	GBint32*				atanTable;

	GBuint32				prepBlockX;
	GBuint32				prepBlockY;
	GBuint32				prepareBlock;

	GBbool					freeingProcess;
	GBuint32				numRestored;

	GBvec2					coords;
	GBfloat					distance;
	GBint32					cameraZ;
	GBint32					cameraZc;

	GBint32					finalMatrix[9];
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBRaySphere*
GBRaySphere_create (const char* textureFilename, GBMemory* mem)
{
	GBRaySphere* sphere = (GBRaySphere*)malloc(sizeof(GBRaySphere));
	if (!sphere)
		return NULL;

	GB_ASSERT(mem);
	sphere->mem = mem;

	GBDisplayParams_init(&sphere->displayParams);

	sphere->texCache = NULL;

	sphere->bWidth = 0;
	sphere->bHeight = 0;
	sphere->blocks = NULL;

	sphere->reallocateDirTable = GB_FALSE;
	sphere->updateDirTable = GB_FALSE;
	sphere->updateSphere = GB_FALSE;

	sphere->dirTable = NULL;
	sphere->dirTableWidth = 0;
	sphere->dirTableHeight = 0;
	sphere->atanTable = NULL;

	sphere->prepBlockX = 0;
	sphere->prepBlockY = 0;
	sphere->prepareBlock = GB_FALSE;

	sphere->freeingProcess = GB_FALSE;
	sphere->numRestored = 0;

	sphere->coords[0] = 0.0f;
	sphere->coords[1] = 0.0f;
	sphere->distance = 0.0f;
	sphere->cameraZ = 0;
	sphere->cameraZc = 0;

	memset(sphere->finalMatrix, 0, sizeof(sphere->finalMatrix));

	if (!GBRaySphere_init(sphere, textureFilename))
	{
		GBRaySphere_destroy(sphere);
		return NULL;
	}

	return sphere;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
GBRaySphere_destroy (GBRaySphere* sphere)
{
	if (sphere->blocks)
	{
		GBuint32 i;
		for (i = 0; i < sphere->bWidth * sphere->bHeight; i++)
			free(sphere->blocks[i].data);
	}
	free(sphere->blocks);

	if (sphere->mem)
	{
		GBMemory_free(sphere->mem, sphere->atanTable);
		GBMemory_free(sphere->mem, sphere->dirTable);
	}

	if (sphere->texCache)
		GBTextureCache_destroy(sphere->texCache);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_setDisplay (GBRaySphere* sphere, GBDisplayParams* displayParams)
{
	GB_ASSERT(sphere && displayParams);

	if (displayParams->width != sphere->displayParams.width ||
		displayParams->height != sphere->displayParams.height)
	{
		sphere->reallocateDirTable = GB_TRUE;
	}

	sphere->displayParams = *displayParams;

	return GBRaySphere_renderInit(sphere);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
GBRaySphere_update (GBRaySphere* sphere, const GBvec2 coords, GBfloat distance)
{
	GBint32 rotationMatrix[9];
	GB_ASSERT(sphere);

	sphere->coords[0] = coords[0];
	sphere->coords[1] = coords[1];

	gbMatrixRotationX(-sphere->coords[1], rotationMatrix);
	gbMatrixRotationY(-sphere->coords[0], sphere->finalMatrix);
	gbMatrixMultiply(sphere->finalMatrix, rotationMatrix, sphere->finalMatrix);

	if (sphere->distance != distance)
	{
		sphere->distance = distance;
		sphere->updateSphere = GB_TRUE;
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_draw (GBRaySphere* sphere)
{
	GB_ASSERT(sphere);

	if (!GBRaySphere_renderInit(sphere))
		return GB_FALSE;

	if (GBTextureCache_isFreeing(sphere->texCache))
		sphere->freeingProcess = GB_TRUE;
	else
		sphere->freeingProcess = GB_FALSE;

	sphere->numRestored = 0;

	if (!sphere->freeingProcess)
	{
		gbDrawGradientBack(&sphere->displayParams);
	}

	if (!GBRaySphere_renderSub(sphere))
		return GB_FALSE;

	if (sphere->prepareBlock)
	{
		sphere->prepareBlock = GB_FALSE;
		if (!GBTextureCache_load(sphere->texCache, 0, sphere->prepBlockX, sphere->prepBlockY))
			GBTextureCache_free(sphere->texCache);
	}

	if (sphere->freeingProcess && !sphere->numRestored)
		GBTextureCache_stopFreeing(sphere->texCache);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_init (GBRaySphere* sphere, const char* textureFilename)
{
	sphere->texCache = GBTextureCache_create(textureFilename, sphere->mem);
	if (!sphere->texCache)
		return GB_FALSE;

	sphere->atanTable = (GBint32*)GBMemory_alloc(sphere->mem, sizeof(GBint32) * GB_ATAN_TABLE_LENGTH);
	if (!sphere->atanTable)
		return GB_FALSE;

	{
		GBuint32 i;
		for (i = 0; i < GB_ATAN_TABLE_LENGTH; i++)
		{
			float a = (float)i / (1 << (GB_ATAN_TABLE_SHIFT_IN - GB_ATAN_TABLE_LERP_BITS));
			sphere->atanTable[i] = (GBint32)(atan(a) * (1 << (GB_ATAN_TABLE_SHIFT_OUT - 1)) / GB_PI);
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_renderInit (GBRaySphere* sphere)
{
	GB_ASSERT(sphere);

	if (sphere->reallocateDirTable)
	{
		GBuint32 i;
		if (sphere->blocks)
		{
			for (i = 0; i < sphere->bWidth * sphere->bHeight; i++)
				free(sphere->blocks[i].data);
		}
		free(sphere->blocks);
		sphere->blocks = NULL;

		sphere->bWidth = ((sphere->displayParams.width + GB_GLOBE_BLOCK_SIZE_MASK_W) >> GB_GLOBE_BLOCK_SIZE_SHIFT_W) + 1;
		sphere->bHeight = ((sphere->displayParams.height + GB_GLOBE_BLOCK_SIZE_MASK_H) >> GB_GLOBE_BLOCK_SIZE_SHIFT_H) + 1;

		sphere->blocks = (GBNormalBlock*)malloc(sizeof(GBNormalBlock) * sphere->bWidth * sphere->bHeight);
		if (!sphere->blocks)
			return GB_FALSE;

		for (i = 0; i < sphere->bWidth * sphere->bHeight; i++)
			sphere->blocks[i].data = NULL;

		GBMemory_free(sphere->mem, sphere->dirTable);
		sphere->dirTable = NULL;

		sphere->dirTableWidth = ((sphere->bWidth << GB_GLOBE_BLOCK_SIZE_SHIFT_W) >> GB_DIRTABLE_SIZE_DOWN_SHIFT) + 1;
		sphere->dirTableHeight = ((sphere->bHeight << GB_GLOBE_BLOCK_SIZE_SHIFT_H) >> GB_DIRTABLE_SIZE_DOWN_SHIFT) + 1;

		sphere->dirTable = (GBint32*)GBMemory_alloc(sphere->mem, sizeof(GBint32) * 3 * sphere->dirTableWidth * sphere->dirTableHeight);
		if (!sphere->dirTable)
			return GB_FALSE;

		sphere->updateDirTable = GB_TRUE;
		sphere->reallocateDirTable = GB_FALSE;
	}

	if (sphere->updateDirTable)
	{
		GBRaySphere_calculateDirTable(sphere);
		sphere->updateSphere = GB_TRUE;
		sphere->updateDirTable = GB_FALSE;
	}

	if (sphere->updateSphere)
	{
		if (!GBRaySphere_calculateSphere(sphere))
			return GB_FALSE;

		sphere->updateSphere = GB_FALSE;
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_castRay (GBRaySphere* sphere,
					GBuint32 sx, GBuint32 sy,
					GBint16* out)
{
	GBint64 dx;
	GBint64 dy;
	GBint64 dz;

	GBint64 b;
	GBint64 d;
	GBint64 t;

	GB_ASSERT(sphere);

	sx = sx >= (sphere->dirTableWidth << GB_DIRTABLE_SIZE_DOWN_SHIFT) ? (sphere->dirTableWidth << GB_DIRTABLE_SIZE_DOWN_SHIFT) - 1 : sx;
	sy = sy >= (sphere->dirTableHeight << GB_DIRTABLE_SIZE_DOWN_SHIFT) ? (sphere->dirTableHeight << GB_DIRTABLE_SIZE_DOWN_SHIFT) - 1 : sy;

	{
		GBint32 dq[12];
		GBint32 tx = sx & GB_DIRTABLE_SIZE_MASK;
		GBint32 ty = sy & GB_DIRTABLE_SIZE_MASK;
		GBint32 itx = GB_DIRTABLE_SIZE_LENGTH - tx;
		GBint32 ity = GB_DIRTABLE_SIZE_LENGTH - ty;
		GBuint32 pos = (sx >> GB_DIRTABLE_SIZE_DOWN_SHIFT) + (sy >> GB_DIRTABLE_SIZE_DOWN_SHIFT) * sphere->dirTableWidth;
		pos += pos + pos;

		if (tx || ty)
		{
			dq[0] = sphere->dirTable[pos];
			dq[1] = sphere->dirTable[pos + 1];
			dq[2] = sphere->dirTable[pos + 2];

			pos += 3;
			if (tx)
			{
				dq[3] = sphere->dirTable[pos];
				dq[4] = sphere->dirTable[pos + 1];
				dq[5] = sphere->dirTable[pos + 2];
			}

			pos += (sphere->dirTableWidth - 1) * 3;
			if (tx || ty)
			{
				dq[6] = sphere->dirTable[pos];
				dq[7] = sphere->dirTable[pos + 1];
				dq[8] = sphere->dirTable[pos + 2];
			}

			pos += 3;
			if (tx)
			{
				dq[9] = sphere->dirTable[pos];
				dq[10] = sphere->dirTable[pos + 1];
				dq[11] = sphere->dirTable[pos + 2];
			}

			if (tx)
			{
				dq[0] = (dq[0] * itx + dq[3] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
				dq[1] = (dq[1] * itx + dq[4] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
				dq[2] = (dq[2] * itx + dq[5] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;

				if (ty)
				{
					dq[6] = (dq[6] * itx + dq[9] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
					dq[7] = (dq[7] * itx + dq[10] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
					dq[8] = (dq[8] * itx + dq[11] * tx) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
				}
			}
			if (ty)
			{
				dx = (dq[0] * ity + dq[6] * ty) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
				dy = (dq[1] * ity + dq[7] * ty) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
				dz = (dq[2] * ity + dq[8] * ty) >> GB_DIRTABLE_SIZE_DOWN_SHIFT;
			}
			else
			{
				dx = dq[0];
				dy = dq[1];
				dz = dq[2];
			}
		}
		else
		{
			dx = sphere->dirTable[pos];
			dy = sphere->dirTable[pos + 1];
			dz = sphere->dirTable[pos + 2];
		}
	}

	b = (dz * (GBint64)sphere->cameraZ) >> 7;

	d = ((b * b) >> 16) - (GBint64)sphere->cameraZc;
	if (d < 0)
		return GB_FALSE;

	if (out)
	{
		GBint64 x;
		GBint64 y;

		if (d != 0)
			t = -b - (GBint64)gbSqrt64(d);
		else
			t = -b;

		x = (dx * t) >> 18;
		y = (dy * t) >> 18;
		out[2] = (GBint16)(((GBint64)sphere->cameraZ << 7) + ((dz * t) >> 18));

		out[0] = (GBint16)(x);
		out[1] = (GBint16)(y);
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_blockCast (GBRaySphere* sphere, GBNormalBlock* block, GBint16* tgt, GBuint32 x, GBuint32 y)
{
	if (GBRaySphere_castRay(sphere, x, y, tgt))
	{
		block->empty = GB_FALSE;
		if (tgt[2] < GB_GLOBE_NORMAL_Z_TOLERANCE)
			block->accurate = GB_TRUE;
		return GB_TRUE;
	}
	else
	{
		tgt[0] = 0;
		tgt[1] = 0;
		tgt[2] = 0;
		block->accurate = GB_TRUE;
		return GB_FALSE;
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBRaySphere_calculateSphere (GBRaySphere* sphere)
{
	GBuint32 x;
	GBuint32 y;
	GBuint32 bx;
	GBuint32 by;
	GB_ASSERT(sphere);
	GB_ASSERT(sphere->blocks);
	GB_ASSERT(sphere->dirTable);

	sphere->cameraZ = gbDistanceToCameraZ(sphere->distance);
	sphere->cameraZc = ((sphere->cameraZ * sphere->cameraZ) - 65536) << 2;

	{
		GBNormalBlock* block = sphere->blocks;
		for (by = 0; by < sphere->bHeight; by++)
		{
			for (bx = 0; bx < sphere->bWidth; bx++)
			{
				GBint16* tgt;
				GBbool topLeftHit;
				GBbool topRightHit;
				GBbool bottomLeftHit;
				GBbool bottomRightHit;
				GBint16 topLeftNormal[3];
				GBint16 topRightNormal[3];
				GBint16 bottomLeftNormal[3];
				GBint16 bottomRightNormal[3];
				GBbool checkTopRight = (bx + 1 >= (sphere->bWidth >> 1)) || (by <= (sphere->bWidth >> 1));
				GBbool checkBottomLeft = (bx <= (sphere->bWidth >> 1)) || (by + 1 >= (sphere->bWidth >> 1));
				GBbool checkBottomRight = (bx + 1 >= (sphere->bWidth >> 1)) || (by + 1 >= (sphere->bWidth >> 1));

				free(block->data);
				block->data = NULL;

				block->empty = GB_TRUE;
				block->accurate = GB_FALSE;

				topLeftHit = GBRaySphere_castRay(sphere, bx << GB_GLOBE_BLOCK_SIZE_SHIFT_W, by << GB_GLOBE_BLOCK_SIZE_SHIFT_H, topLeftNormal);
				topRightHit = GBRaySphere_castRay(sphere, (bx + 1) << GB_GLOBE_BLOCK_SIZE_SHIFT_W, by << GB_GLOBE_BLOCK_SIZE_SHIFT_H, checkTopRight ? topRightNormal : NULL);
				bottomLeftHit = GBRaySphere_castRay(sphere, bx << GB_GLOBE_BLOCK_SIZE_SHIFT_W, (by + 1) << GB_GLOBE_BLOCK_SIZE_SHIFT_H, checkBottomLeft ? bottomLeftNormal : NULL);
				bottomRightHit = GBRaySphere_castRay(sphere, (bx + 1) << GB_GLOBE_BLOCK_SIZE_SHIFT_W, (by + 1) << GB_GLOBE_BLOCK_SIZE_SHIFT_H, checkBottomRight ? bottomRightNormal : NULL);

				if (!topLeftHit &&
					!topRightHit &&
					!bottomLeftHit &&
					!bottomRightHit)
				{
					block++;
					continue;
				}

				if (topLeftHit &&
					topRightHit &&
					bottomLeftHit &&
					bottomRightHit &&
					!(topLeftNormal[2] <= GB_GLOBE_NORMAL_Z_TOLERANCE &&
					(!checkTopRight || topRightNormal[2] <= GB_GLOBE_NORMAL_Z_TOLERANCE) &&
					(!checkBottomLeft || bottomLeftNormal[2] <= GB_GLOBE_NORMAL_Z_TOLERANCE) &&
					(!checkBottomRight || bottomRightNormal[2] <= GB_GLOBE_NORMAL_Z_TOLERANCE)))
				{
					block->data = (GBint16*)malloc(sizeof(GBint16) * 3);
					if (!block->data)
						return GB_FALSE;

					block->data[0] = topLeftNormal[0];
					block->data[1] = topLeftNormal[1];
					block->data[2] = topLeftNormal[2];

					block->empty = GB_FALSE;

					block++;
					continue;
				}

				block->data = (GBint16*)malloc(sizeof(GBint16) * 3 * GB_GLOBE_BLOCK_SIZE_W * GB_GLOBE_BLOCK_SIZE_H);
				if (!block->data)
					return GB_FALSE;

				tgt = block->data;

				y = 0;
				while (y < GB_GLOBE_BLOCK_SIZE_H)
				{
					GBbool doLerp;
					x = 0;
					doLerp = GBRaySphere_blockCast(sphere, block, tgt, bx << GB_GLOBE_BLOCK_SIZE_SHIFT_W, y + (by << GB_GLOBE_BLOCK_SIZE_SHIFT_H));
					while (x < GB_GLOBE_BLOCK_SIZE_W)
					{
						GBint16* lerpStart = NULL;
						if ((x & GB_RAY_BLOCK_H_SPAN_MASK) == 0)
						{
							GBbool hit;

							if (x + GB_RAY_BLOCK_H_SPAN_LENGTH < GB_GLOBE_BLOCK_SIZE_W)
								hit = GBRaySphere_blockCast(sphere, block, tgt + GB_RAY_BLOCK_H_SPAN_LENGTH * 3, x + GB_RAY_BLOCK_H_SPAN_LENGTH + (bx << GB_GLOBE_BLOCK_SIZE_SHIFT_W), y + (by << GB_GLOBE_BLOCK_SIZE_SHIFT_H));
							else
								hit = GB_FALSE;

							if (doLerp && hit)
								lerpStart = tgt;
							else
								doLerp = GB_FALSE;
						}
						else if (doLerp)
						{
							GBint32 t = x & GB_RAY_BLOCK_H_SPAN_MASK;
							GBint32 it = GB_RAY_BLOCK_H_SPAN_LENGTH - t;
							tgt[0] = (GBint16)((lerpStart[0] * it + lerpStart[0 + 3 * GB_RAY_BLOCK_H_SPAN_LENGTH] * t) >> GB_RAY_BLOCK_H_SPAN_SHIFT);
							tgt[1] = (GBint16)((lerpStart[1] * it + lerpStart[1 + 3 * GB_RAY_BLOCK_H_SPAN_LENGTH] * t) >> GB_RAY_BLOCK_H_SPAN_SHIFT);
							tgt[2] = (GBint16)((lerpStart[2] * it + lerpStart[2 + 3 * GB_RAY_BLOCK_H_SPAN_LENGTH] * t) >> GB_RAY_BLOCK_H_SPAN_SHIFT);
						}
						else
						{
							GBRaySphere_blockCast(sphere, block, tgt, x + (bx << GB_GLOBE_BLOCK_SIZE_SHIFT_W), y + (by << GB_GLOBE_BLOCK_SIZE_SHIFT_H));
						}
						tgt += 3;
						x++;
					}
					y++;
				}

				if (block->empty)
				{
					free(block->data);
					block->data = NULL;
				}
				else if (!block->accurate)
				{
					GBint16 x = block->data[0];
					GBint16 y = block->data[1];
					GBint16 z = block->data[2];

					free(block->data);

					block->data = (GBint16*)malloc(sizeof(GBint16) * 3);
					if (!block->data)
						return GB_FALSE;

					block->data[0] = x;
					block->data[1] = y;
					block->data[2] = z;
				}

				block++;
			}
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBRaySphere_calculateDirTable (GBRaySphere* sphere)
{
	GBint32* tgt = sphere->dirTable;
	GBuint32 y;

	GB_ASSERT(sphere && sphere->dirTable);

	for (y = 0; y < sphere->dirTableHeight; y++)
	{
		GBuint32 x;
		for (x = 0; x < sphere->dirTableWidth; x++)
		{
			GBint64 dx = ((GBint64)(x << GB_DIRTABLE_SIZE_DOWN_SHIFT) * 2 - sphere->displayParams.width) * 4;
			GBint64 dy = ((GBint64)(y << GB_DIRTABLE_SIZE_DOWN_SHIFT) * 2 - sphere->displayParams.height) * 4;
			GBint64 dz;

			GBint64 dlen;

			dz = (GBint64)(GBint32)((GBfloat)sphere->displayParams.height * -8.0f * GB_FOV_CONSTANT);

			dlen = gbSqrt64(dx * dx + dy * dy + dz * dz) + 1;

			*tgt++ = (GBint32)((dx << 24) / dlen);
			*tgt++ = (GBint32)((dy << 24) / dlen);
			*tgt++ = (GBint32)((dz << 24) / dlen);
		}
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32
GBRaySphere_atan (GBRaySphere* sphere, GBint32 p)
{
	GBint32 r = 0;
	GBint32 sub = p & GB_ATAN_TABLE_LERP_MASK;
	GBint32 isub = (1 << GB_ATAN_TABLE_LERP_BITS) - sub;
	p >>= GB_ATAN_TABLE_LERP_BITS;

	if (p < GB_ATAN_TABLE_MASK)
		r = ((sphere->atanTable[p] * isub) + (sphere->atanTable[p + 1] * sub)) >> GB_ATAN_TABLE_LERP_BITS;
	else if (p >= GB_ATAN_TABLE_MASK)
		r = sphere->atanTable[GB_ATAN_TABLE_MASK];

	return r;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void
GBRaySphere_initLerpData (GBRaySphere* sphere, GBLerpData* data, const GBint16* normalSrc, const GBSplitTexture* tex)
{
	GB_ASSERT(data);
	GB_ASSERT(normalSrc);
	GB_ASSERT(tex);

	data->normal[0] = normalSrc[0];
	data->normal[1] = normalSrc[1];
	data->normal[2] = normalSrc[2];

	data->l = (data->normal[2] * data->normal[2]) >> 21;

	gbVec3Transform(data->normal, sphere->finalMatrix, data->normal);

	{
		GBint32 uoff = 0;
		GBint32 xn = data->normal[0];
		GBint32 zn = data->normal[2];
		GBbool inv = GB_FALSE;
		GBint32 a;
		GBint32 av;

		if (xn < 0 && zn < 0)
		{
			xn = -xn;
			zn = -zn;
		}
		else if (xn >= 0 && zn < 0)
		{
			zn = -zn;
			inv = GB_TRUE;
		}
		else if (xn >= 0 && zn >= 0)
		{
			uoff = tex->width << (GB_RENDER_EXTRA_BITS_IN_UV - 1);
		}
		else if (xn < 0 && zn >= 0)
		{
			uoff = tex->width << (GB_RENDER_EXTRA_BITS_IN_UV - 1);
			xn = -xn;
			inv = GB_TRUE;
		}

		if (zn)
            a = (xn << GB_ATAN_TABLE_SHIFT_IN) / zn;
		else
			a = GB_INT32_MAX;

		av = GBRaySphere_atan(sphere, a);

		if (inv)
			data->u = (GBuint32)(((-av * tex->width) >> (GB_ATAN_TABLE_SHIFT_OUT - GB_RENDER_EXTRA_BITS_IN_UV)) + uoff) & ((tex->width << GB_RENDER_EXTRA_BITS_IN_UV) - 1);
		else
			data->u = (GBuint32)(((av * tex->width) >> (GB_ATAN_TABLE_SHIFT_OUT - GB_RENDER_EXTRA_BITS_IN_UV)) + uoff) & ((tex->width << GB_RENDER_EXTRA_BITS_IN_UV) - 1);
	}

	data->v = ((0x8000 - data->normal[1]) * (GBint32)tex->height) >> (16 - GB_RENDER_EXTRA_BITS_IN_UV);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBRaySphere_renderSub (GBRaySphere* sphere)
{
	GBuint32 bx;
	GBuint32 by;
	GBSplitTexture* tex;
	GBGtxBlock* infoBlocks;
	GBuint8* palette;
	GBNormalBlock* blocks[4];
	GBint32 poleToleranceTexSpace;
	GBOutputFormat* lastLineEnd;

	GB_ASSERT(sphere);

	tex = GBTextureCache_getLevel(sphere->texCache, 0);
	infoBlocks = GBTextureCache_getInfoBlocks(sphere->texCache);
	palette = GBTextureCache_getPalette(sphere->texCache);

	if (!tex || !infoBlocks || !palette)
		return GB_FALSE;

	poleToleranceTexSpace = (GBint32)(tex->height * (33000 - GB_RENDER_POLE_TOLERANCE)) >> 16;

	lastLineEnd = (GBOutputFormat*)sphere->displayParams.buffer + (sphere->displayParams.width - 1 + (sphere->displayParams.y + (sphere->displayParams.height - 1)) * sphere->displayParams.bufferWidth + sphere->displayParams.x);

	for (by = 0; by < sphere->bHeight - 1; by++)
	{
		GBbool lastBlockLerp = GB_FALSE;
		GBLerpData corners[4];
		GBOutputFormat* btgt = (GBOutputFormat*)sphere->displayParams.buffer +
			(((by << GB_GLOBE_BLOCK_SIZE_SHIFT_H) + sphere->displayParams.y) * sphere->displayParams.bufferWidth + sphere->displayParams.x);
		GBOutputFormat* scanlineEnd = btgt + sphere->displayParams.width;

		blocks[0] = NULL;
		blocks[1] = &sphere->blocks[by * sphere->bWidth];
		blocks[2] = NULL;
		blocks[3] = &sphere->blocks[(by + 1) * sphere->bWidth];

		for (bx = 0; bx < sphere->bWidth - 1; bx++)
		{
			blocks[0] = blocks[1];
			blocks[2] = blocks[3];
			blocks[1]++;
			blocks[3]++;

			if (blocks[0]->empty)
			{
				lastBlockLerp = GB_FALSE;
				btgt += GB_GLOBE_BLOCK_SIZE_W;
				continue;
			}

			if (blocks[0]->accurate)
			{
				GBuint32 x;
				GBuint32 y;
				GBOutputFormat* tgt = btgt;
				GBOutputFormat* scanlineEndInner = scanlineEnd;
				GBint16* normalSrc = blocks[0]->data;

				lastBlockLerp = GB_FALSE;

				y = 0;
				while (tgt < lastLineEnd && y < GB_GLOBE_BLOCK_SIZE_H)
				{
					GBOutputFormat* tgtx = tgt;
					GBint16* normalSrcX = normalSrc;
					x = 0;

					while (tgtx < scanlineEndInner && x < GB_GLOBE_BLOCK_SIZE_W)
					{
						GBint32 normal[3];
						normal[0] = *normalSrcX;
						normal[1] = *(normalSrcX + 1);
						if (!(normal[0] == 0 && normal[1] == 0))
						{
							GBuint8* tblock = NULL;
							GBint32 tbx;
							GBint32 tby;

							GBint32 u;
							GBint32 v;

							GBint32 l;

							normal[2] = normalSrcX[2];

							l = (normal[2] * normal[2]) >> 21;
							if (l < 0)
								l = 0;
							else if (l > 255)
								l = 255;

							gbVec3Transform(normal, sphere->finalMatrix, normal);

							if (normal[1] < -GB_RENDER_POLE_TOLERANCE || normal[1] > GB_RENDER_POLE_TOLERANCE)
							{
#if defined(GB_PIXELFORMAT_888)
								*tgtx = (l << 16) | (l << 8) | l;
#elif defined(GB_PIXELFORMAT_444)
								*tgtx = (((l) & 0xf0) << 4) | (l & 0xf0) | (l >> 4);
#elif defined(GB_PIXELFORMAT_565)
								*tgtx = (GBOutputFormat)(((l << 8) & 0xf800) | ((l << 3) & 0x7e0) | ((l & 0xff) >> 3));
#endif
							}
							else
							{
								GBint32 uoff = 0;
								GBint32 xn = normal[0];
								GBint32 zn = normal[2];
								GBbool inv = GB_FALSE;
								GBint32 a;
								GBint32 av;
								GBGtxBlock* info;
								GBbool plain = GB_FALSE;

								if (xn < 0 && zn < 0)
								{
									xn = -xn;
									zn = -zn;
								}
								else if (xn >= 0 && zn < 0)
								{
									zn = -zn;
									inv = GB_TRUE;
								}
								else if (xn >= 0 && zn >= 0)
								{
									uoff = tex->width >> 1;
								}
								else if (xn < 0 && zn >= 0)
								{
									uoff = tex->width >> 1;
									xn = -xn;
									inv = GB_TRUE;
								}

								if (zn)
									a = (xn << GB_ATAN_TABLE_SHIFT_IN) / zn;
								else
									a = GB_INT32_MAX;

								av = GBRaySphere_atan(sphere, a);

								if (inv)
									u = (GBuint32)(((-av * tex->width) >> GB_ATAN_TABLE_SHIFT_OUT) + uoff) & tex->hMask;
								else
									u = (GBuint32)(((av * tex->width) >> GB_ATAN_TABLE_SHIFT_OUT) + uoff) & tex->hMask;

								v = ((0x8000 - normal[1]) * (GBint32)tex->height) >> 16;

								tbx = (u & tex->hMask) >> tex->hBlockShift;
								tby = (v & tex->vMask) >> tex->vBlockShift;

								info = &infoBlocks[tbx + (tby << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];

								if (info->flags & GB_GTX_BLOCK_FLAG_PLAIN)
									plain = GB_TRUE;

								if (!plain)
									tblock = (GBuint8*)tex->blocks[tbx + (tby << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];

								if (!sphere->freeingProcess && (plain || !tblock))
								{
#if defined(GB_PIXELFORMAT_888)
									if (l < 255)
									{
										GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
										GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
										*tgtx = g | rb;
									}
									else
									{
										*tgtx = info->finalColor;
									}
#elif defined(GB_PIXELFORMAT_444)
									if (l < 255)
									{
										GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
										GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
										*tgtx = ((rb & 0xf00000) >> 12) | ((g & 0xf000) >> 8) | ((rb & 0xf0) >> 4);
									}
									else
									{
										*tgtx = ((info->finalColor & 0xf00000) >> 12) | ((info->finalColor & 0xf000) >> 8) | ((info->finalColor & 0xf0) >> 4);
									}
#elif defined(GB_PIXELFORMAT_565)
									if (l < 255)
									{
										GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
										GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
										*tgtx = (GBOutputFormat)(((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3));
									}
									else
									{
										*tgtx = (GBOutputFormat)(((info->finalColor & 0xf80000) >> 8) | ((info->finalColor >> 5) & 0x7e0) | ((info->finalColor & 0xff) >> 3));
									}
#endif
								}

								if (!plain)
								{
									if (!tblock)
									{
#ifndef GB_LOAD_WHOLE_TEXTURE_AT_STARTUP
										if (sphere->freeingProcess)
										{
											if (GBTextureCache_restore(sphere->texCache, 0, tbx, tby))
												sphere->numRestored++;
										}
										else if (!sphere->prepareBlock)
										{
											sphere->prepBlockX = tbx;
											sphere->prepBlockY = tby;
											sphere->prepareBlock = GB_TRUE;
										}
#endif
									}
									else if (!sphere->freeingProcess)
									{
										GBuint32 rb;
										GBuint32 g;
										GBuint32 off = (u & tex->hBlockMask) + ((v & tex->vBlockMask) << tex->hBlockShift);
										GBuint8* src;
										off = tblock[off];
										src = palette + (off << 1) + off;

										if (l == 255)
										{
											rb = (src[2] << 16) | (*src);
											g = (src[1] << 8) & 0xff00;
										}
										else
										{
											rb = (src[2] << 16) | (*src);
											rb = ((rb * l) >> 8) & 0xff00ff;
											g = (src[1] * l) & 0xff00;
										}

#if defined(GB_PIXELFORMAT_888)
										*tgtx = rb | g;
#elif defined(GB_PIXELFORMAT_444)
										{
											GBOutputFormat r2 = (GBOutputFormat)((rb >> 12) & 0xf00);
											GBOutputFormat g2 = (GBOutputFormat)((g >> 8) & 0xf0);
											GBOutputFormat b2 = (GBOutputFormat)((rb >> 4) & 0xf);
											*tgtx = r2 | g2 | b2;
										}
#elif defined(GB_PIXELFORMAT_565)
				                        *tgtx = (GBOutputFormat)(((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3));
#endif
									}
								}
							}
						}

						normalSrcX += 3; 
						tgtx++;
						x++;
					}

					scanlineEndInner += sphere->displayParams.bufferWidth;
					tgt += sphere->displayParams.bufferWidth;
					y++;
					normalSrc += 3 * GB_GLOBE_BLOCK_SIZE_W;
				}
			}
			else
			{
				GBuint32 x;
				GBuint32 y;
				GBOutputFormat* tgt = btgt;
				GBOutputFormat* scanlineEndInner = scanlineEnd;

				GBint32 currLeftU;
				GBint32 currLeftV;
				GBint32 currLeftL;

				GBint32 currRightU;
				GBint32 currRightV;
				GBint32 currRightL;

				GBint32 leftDeltaU;
				GBint32 leftDeltaV;
				GBint32 leftDeltaL;

				GBint32 rightDeltaU;
				GBint32 rightDeltaV;
				GBint32 rightDeltaL;

				if (lastBlockLerp)
				{
					corners[0] = corners[1];
					corners[2] = corners[3];
				}
				else
				{
					GBRaySphere_initLerpData(sphere, &corners[0], blocks[0]->data, tex);
					GBRaySphere_initLerpData(sphere, &corners[2], blocks[2]->data, tex);
				}

				GBRaySphere_initLerpData(sphere, &corners[1], blocks[1]->data, tex);
				GBRaySphere_initLerpData(sphere, &corners[3], blocks[3]->data, tex);

				lastBlockLerp = GB_TRUE;

				if ((corners[0].u >> GB_RENDER_EXTRA_BITS_IN_UV) > (GBint32)(tex->width - (tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT)) ||
				(corners[1].u >> GB_RENDER_EXTRA_BITS_IN_UV) > (GBint32)(tex->width - (tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT)) ||
				(corners[2].u >> GB_RENDER_EXTRA_BITS_IN_UV) > (GBint32)(tex->width - (tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT)) ||
				(corners[3].u >> GB_RENDER_EXTRA_BITS_IN_UV) > (GBint32)(tex->width - (tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT)))
				{
					if ((corners[0].u >> GB_RENDER_EXTRA_BITS_IN_UV) < (GBint32)(tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT))
					{
						corners[0].u += tex->width << GB_RENDER_EXTRA_BITS_IN_UV;
						lastBlockLerp = GB_FALSE;
					}
					if ((corners[1].u >> GB_RENDER_EXTRA_BITS_IN_UV) < (GBint32)(tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT))
					{
						corners[1].u += tex->width << GB_RENDER_EXTRA_BITS_IN_UV;
						lastBlockLerp = GB_FALSE;
					}
					if ((corners[2].u >> GB_RENDER_EXTRA_BITS_IN_UV) < (GBint32)(tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT))
					{
						corners[2].u += tex->width << GB_RENDER_EXTRA_BITS_IN_UV;
						lastBlockLerp = GB_FALSE;
					}
					if ((corners[3].u >> GB_RENDER_EXTRA_BITS_IN_UV) < (GBint32)(tex->width >> GB_RENDER_WRAP_TOLERANCE_SHIFT))
					{
						corners[3].u += tex->width << GB_RENDER_EXTRA_BITS_IN_UV;
						lastBlockLerp = GB_FALSE;
					}
				}

				currLeftU = corners[0].u << (GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV);
				currLeftV = corners[0].v << (GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV);
				currLeftL = corners[0].l << GB_RENDER_LERP_BITS;

				currRightU = corners[1].u << (GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV);
				currRightV = corners[1].v << (GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV);
				currRightL = corners[1].l << GB_RENDER_LERP_BITS;

				leftDeltaU = (corners[2].u - corners[0].u) << ((GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV) - GB_GLOBE_BLOCK_SIZE_SHIFT_H);
				leftDeltaV = (corners[2].v - corners[0].v) << ((GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV) - GB_GLOBE_BLOCK_SIZE_SHIFT_H);
				leftDeltaL = (corners[2].l - corners[0].l) << (GB_RENDER_LERP_BITS - GB_GLOBE_BLOCK_SIZE_SHIFT_H);

				rightDeltaU = (corners[3].u - corners[1].u) << ((GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV) - GB_GLOBE_BLOCK_SIZE_SHIFT_H);
				rightDeltaV = (corners[3].v - corners[1].v) << ((GB_RENDER_LERP_BITS - GB_RENDER_EXTRA_BITS_IN_UV) - GB_GLOBE_BLOCK_SIZE_SHIFT_H);
				rightDeltaL = (corners[3].l - corners[1].l) << (GB_RENDER_LERP_BITS - GB_GLOBE_BLOCK_SIZE_SHIFT_H);

				y = 0;
				while (tgt < lastLineEnd && y < GB_GLOBE_BLOCK_SIZE_H)
				{
					GBOutputFormat* tgtx = tgt;

					GBint32 currHorU = currLeftU;
					GBint32 currHorV = currLeftV;
					GBint32 currHorL = currLeftL;

					GBint32 horDeltaU;
					GBint32 horDeltaV;
					GBint32 horDeltaL;

					horDeltaU = (currRightU - currLeftU) >> GB_GLOBE_BLOCK_SIZE_SHIFT_W;
					horDeltaV = (currRightV - currLeftV) >> GB_GLOBE_BLOCK_SIZE_SHIFT_W;
					horDeltaL = (currRightL - currLeftL) >> GB_GLOBE_BLOCK_SIZE_SHIFT_W;

					x = 0;
					while (tgtx < scanlineEndInner && x < GB_GLOBE_BLOCK_SIZE_W)
					{
						GBint32 u = currHorU >> GB_RENDER_LERP_BITS;
						GBint32 v = currHorV >> GB_RENDER_LERP_BITS;
						GBint32 tbx = (u & tex->hMask) >> tex->hBlockShift;
						GBint32 tby = (v & tex->vMask) >> tex->vBlockShift;
						GBint32 l = currHorL >> GB_RENDER_LERP_BITS;

						l = l > 255 ? 255 : l < 0 ? 0 : l;

						if (v < poleToleranceTexSpace || v > (GBint32)tex->height - poleToleranceTexSpace)
						{
#if defined(GB_PIXELFORMAT_888)
							*tgtx = (l << 16) | (l << 8) | l;
#elif defined(GB_PIXELFORMAT_444)
							*tgtx = (((l) & 0xf0) << 4) | (l & 0xf0) | (l >> 4);
#elif defined(GB_PIXELFORMAT_565)
	                        *tgtx = (GBOutputFormat)(((l << 8) & 0xf800) | ((l << 3) & 0x7e0) | ((l & 0xff) >> 3));
#endif
						}
						else
						{
							GBuint8* tblock = NULL;
							GBbool plain = GB_FALSE;
							GBGtxBlock* info;

							info = &infoBlocks[tbx + (tby << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];

							if (info->flags & GB_GTX_BLOCK_FLAG_PLAIN)
								plain = GB_TRUE;

							if (!plain)
								tblock = (GBuint8*)tex->blocks[tbx + (tby << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];

							if (!sphere->freeingProcess && (plain || !tblock))
							{
#if defined(GB_PIXELFORMAT_888)
								if (l < 255)
								{
									GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
									GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
									*tgtx = g | rb;
								}
								else
								{
									*tgtx = info->finalColor;
								}
#elif defined(GB_PIXELFORMAT_444)
								if (l < 255)
								{
									GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
									GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
									*tgtx = ((rb & 0xf00000) >> 12) | ((g & 0xf000) >> 8) | ((rb & 0xf0) >> 4);
								}
								else
								{
									*tgtx = ((info->finalColor & 0xf00000) >> 12) | ((info->finalColor & 0xf000) >> 8) | ((info->finalColor & 0xf0) >> 4);
								}
#elif defined(GB_PIXELFORMAT_565)
								if (l < 255)
								{
									GBuint32 rb = (((info->finalColor & 0xff00ff) * l) >> 8) & 0xff00ff;
									GBuint32 g = (((info->finalColor & 0xff00) * l) >> 8) & 0xff00;
			                        *tgtx = (GBOutputFormat)(((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3));
								}
								else
								{
			                        *tgtx = (GBOutputFormat)((((info->finalColor & 0xf80000) >> 8) | ((info->finalColor >> 5) & 0x7e0) | ((info->finalColor & 0xff) >> 3)));
								}
#endif
							}

							if (!plain)
							{
								if (!tblock)
								{
									if (sphere->freeingProcess)
									{
										if (GBTextureCache_restore(sphere->texCache, 0, tbx, tby))
											sphere->numRestored++;
									}
									else if (!sphere->prepareBlock)
									{
										sphere->prepBlockX = tbx;
										sphere->prepBlockY = tby;
										sphere->prepareBlock = GB_TRUE;
									}
								}
								else if (!sphere->freeingProcess)
								{
#ifdef GB_ENABLE_BILINEAR
									GBint32 u2 = u + 1;
									GBint32 v2 = v + 1;
									GBint32 tbx2 = (u2 & tex->hMask) >> tex->hBlockShift;
									GBint32 tby2 = (v2 & tex->vMask) >> tex->vBlockShift;
									GBuint32 rb[4];
									GBuint32 g[4];
									GBuint8* src[4];
									GBuint32 c;

									src[0] = tblock;

									if (tbx2 != tbx)
										src[1] = (GBuint8*)tex->blocks[tbx2 + (tby << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];
									else
										src[1] = src[0];

									if (tby2 != tby)
										src[2] = (GBuint8*)tex->blocks[tbx + (tby2 << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];
									else
										src[2] = src[0];

									if (tbx2 != tbx || tby2 != tby)
										src[3] = (GBuint8*)tex->blocks[tbx2 + (tby2 << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)];
									else
										src[3] = src[0];

									if (src[1] && src[2] && src[3])
									{
										GBint32 biu = (currHorU >> (GB_RENDER_LERP_BITS - GB_BILINEAR_BITS)) & GB_BILINEAR_MASK;
										GBint32 biv = (currHorV >> (GB_RENDER_LERP_BITS - GB_BILINEAR_BITS)) & GB_BILINEAR_MASK;
										GBint32 ibiu = (GB_BILINEAR_MASK + 1) - biu;
										GBint32 ibiv = (GB_BILINEAR_MASK + 1) - biv;
										GBuint32 off[4];

										off[0] = (u & tex->hBlockMask) + ((v & tex->vBlockMask) << tex->hBlockShift);
										off[1] = (u2 & tex->hBlockMask) + ((v & tex->vBlockMask) << tex->hBlockShift);
										off[2] = (u & tex->hBlockMask) + ((v2 & tex->vBlockMask) << tex->hBlockShift);
										off[3] = (u2 & tex->hBlockMask) + ((v2 & tex->vBlockMask) << tex->hBlockShift);

										for (c = 0; c < 4; c++)
										{
											GBuint32 ind = src[c][off[c]];
											ind = (ind << 1) + ind;
											if (l < 255)
											{
												rb[c] = ((((palette[ind + 2] << 16) | (palette[ind])) * l) >> 8) & 0xff00ff;
												g[c] = (((palette[ind + 1] << 8) * l) >> 8) & 0xff00;
											}
											else
											{
												rb[c] = (palette[ind + 2] << 16) | (palette[ind]);
												g[c] = palette[ind + 1] << 8;
											}
										}

										if (biu)
										{
											rb[0] = (((rb[0] * ibiu) >> GB_BILINEAR_BITS) & 0xff00ff) + (((rb[1] * biu) >> GB_BILINEAR_BITS) & 0xff00ff);
											g[0] = (((g[0] * ibiu) >> GB_BILINEAR_BITS) & 0xff00) + (((g[1] * biu) >> GB_BILINEAR_BITS) & 0xff00);
											if (biv)
											{
												rb[2] = (((rb[2] * ibiu) >> GB_BILINEAR_BITS) & 0xff00ff) + (((rb[3] * biu) >> GB_BILINEAR_BITS) & 0xff00ff);
												g[2] = (((g[2] * ibiu) >> GB_BILINEAR_BITS) & 0xff00) + (((g[3] * biu) >> GB_BILINEAR_BITS) & 0xff00);
											}
										}
										if (biv)
										{
											rb[0] = (((rb[0] * ibiv) >> GB_BILINEAR_BITS) & 0xff00ff) + (((rb[2] * biv) >> GB_BILINEAR_BITS) & 0xff00ff);
											g[0] = (((g[0] * ibiv) >> GB_BILINEAR_BITS) & 0xff00) + (((g[2] * biv) >> GB_BILINEAR_BITS) & 0xff00);
										}

#if defined(GB_PIXELFORMAT_888)
										*tgtx = rb[0] | g[0];
#elif defined(GB_PIXELFORMAT_444)
										{
											GBOutputFormat r2 = (GBOutputFormat)((rb[0] >> 12) & 0xf00);
											GBOutputFormat g2 = (GBOutputFormat)((g[0] >> 8) & 0xf0);
											GBOutputFormat b2 = (GBOutputFormat)((rb[0] >> 4) & 0xf);
											*tgtx = r2 | g2 | b2;
										}
#elif defined(GB_PIXELFORMAT_565)
				                        *tgtx = (GBOutputFormat)(((rb[0] & 0xf80000) >> 8) | ((g[0] >> 5) & 0x7e0) | ((rb[0] & 0xff) >> 3));
#endif
									}
									else
									{
										GBuint32 off = (u & tex->hBlockMask) + ((v & tex->vBlockMask) << tex->hBlockShift);
										GBuint32 rb;
										GBuint32 g;
										GBuint32 ind;

										ind = src[0][off];
										ind = (ind << 1) + ind;

										rb = (palette[ind + 2] << 16) | palette[ind];
										g = (palette[ind + 1] * l) & 0xff00;
										rb = ((rb * l) >> 8) & 0xff00ff;

#if defined(GB_PIXELFORMAT_888)
										*tgtx = rb | g;
#elif defined(GB_PIXELFORMAT_444)
										{
											GBOutputFormat r2 = (GBOutputFormat)((rb >> 12) & 0xf00);
											GBOutputFormat g2 = (GBOutputFormat)((g >> 8) & 0xf0);
											GBOutputFormat b2 = (GBOutputFormat)((rb >> 4) & 0xf);
											*tgtx = r2 | g2 | b2;
										}
#elif defined(GB_PIXELFORMAT_565)
				                        *tgtx = (GBOutputFormat)((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3);
#endif
#ifndef GB_LOAD_WHOLE_TEXTURE_AT_STARTUP
										if (!src[1])
										{
											if (sphere->freeingProcess)
											{
												if (GBTextureCache_restore(sphere->texCache, 0, tbx2, tby))
													sphere->numRestored++;
											}
											else if (!sphere->prepareBlock)
											{
												sphere->prepBlockX = tbx2;
												sphere->prepBlockY = tby;
												sphere->prepareBlock = GB_TRUE;
											}
										}
										if (!src[2])
										{
											if (sphere->freeingProcess)
											{
												if (GBTextureCache_restore(sphere->texCache, 0, tbx, tby2))
													sphere->numRestored++;
											}
											else if (!sphere->prepareBlock)
											{
												sphere->prepBlockX = tbx;
												sphere->prepBlockY = tby2;
												sphere->prepareBlock = GB_TRUE;
											}
										}
										if (!src[3])
										{
											if (sphere->freeingProcess)
											{
												if (GBTextureCache_restore(sphere->texCache, 0, tbx2, tby2))
													sphere->numRestored++;
											}
											else if (!sphere->prepareBlock)
											{
												sphere->prepBlockX = tbx2;
												sphere->prepBlockY = tby2;
												sphere->prepareBlock = GB_TRUE;
											}
										}
#endif
									}

#else
									GBuint32 off = (u & tex->hBlockMask) + ((v & tex->vBlockMask) << tex->hBlockShift);
									GBuint8* src;
									off = tblock[off];
									src = palette + (off << 1) + off;

									if (l < 255)
									{
										GBuint32 rb = ((src[2]) << 16) | src[0];
										GBuint32 g = ((src[1]) * l) & 0xff00;
										rb = ((rb * l) >> 8) & 0xff00ff;

#if defined(GB_PIXELFORMAT_888)
										*tgtx = rb | g;
#elif defined(GB_PIXELFORMAT_444)
										*tgtx = ((rb & 0xf00000) >> 12) | ((g & 0xf000) >> 8) | ((rb & 0xf0) >> 4);
#elif defined(GB_PIXELFORMAT_565)
				                        *tgtx = (GBOutputFormat)((rb & 0xf80000) >> 8) | ((g >> 5) & 0x7e0) | ((rb & 0xff) >> 3);
#endif
									}
									else
									{
#if defined(GB_PIXELFORMAT_888)
										*tgtx = (src[2] << 16) | (src[1] << 8) | src[0];
#elif defined(GB_PIXELFORMAT_444)
										*tgtx = ((src[2] << 4) & 0xf00) | (src[1] & 0xf0) | (src[0] >> 4);
#elif defined(GB_PIXELFORMAT_565)
				                        *tgtx = ((src[2] << 8) & 0xf800) | ((src[1] << 3) & 0x7e0) | (src[0] >> 3);
#endif
									}
#endif
								}
							}
						}

						currHorU += horDeltaU;
						currHorV += horDeltaV;
						currHorL += horDeltaL;

						tgtx++;
						x++;
					}

					currLeftU += leftDeltaU;
					currLeftV += leftDeltaV;
					currLeftL += leftDeltaL;
					currRightU += rightDeltaU;
					currRightV += rightDeltaV;
					currRightL += rightDeltaL;

					scanlineEndInner += sphere->displayParams.bufferWidth;
					tgt += sphere->displayParams.bufferWidth;
					y++;
				}
			}

			btgt += GB_GLOBE_BLOCK_SIZE_W;
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
