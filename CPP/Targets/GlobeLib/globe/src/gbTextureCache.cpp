/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbTextureCache.h"

#include "gbGtx.h"

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBSplitTexture*	GBSplitTexture_create		(GBMemory* mem);
static void				GBSplitTexture_destroy		(GBSplitTexture* tex, GBMemory* mem);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBTextureCache_s
{
	char*				filename;

	GBMemory*			mem;

	GBSplitTexture*		levels[GB_TEXTURE_CACHE_LEVEL_COUNT];
	GBSplitTexture*		cachedLevels[GB_TEXTURE_CACHE_LEVEL_COUNT];

	GBbool				freeingMemory;

	GBuint32			width;
	GBuint32			height;
	GBuint32			blockWidth;
	GBuint32			blockHeight;
	GBuint8*			palette;

	GBGtxBlock			infoBlocks[GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT];

	FILE*				texFile;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBbool		GBTextureCache_init				(GBTextureCache* texCache, const char* filename);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBTextureCache* GBTextureCache_create (const char* filename, GBMemory* mem)
{
	GBTextureCache* texCache;
	GB_ASSERT(filename);

	texCache = (GBTextureCache*)malloc(sizeof(GBTextureCache));
	if (!texCache)
		return NULL;

	texCache->filename = NULL;

	texCache->mem = mem;

	{
		GBuint32 i;
		for (i = 0; i < GB_TEXTURE_CACHE_LEVEL_COUNT; i++)
		{
			texCache->levels[i] = NULL;
			texCache->cachedLevels[i] = NULL;
		}
	}

	texCache->freeingMemory = GB_FALSE;

	texCache->width = 0;
	texCache->height = 0;
	texCache->blockWidth = 0;
	texCache->blockHeight = 0;
	texCache->palette = NULL;

	{
		GBuint32 i;
		for (i = 0; i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT; i++)
			GBGtxBlock_init(&texCache->infoBlocks[i]);
	}

	texCache->texFile = NULL;

	if (!GBTextureCache_init(texCache, filename))
	{
		GBTextureCache_destroy(texCache);
		return NULL;
	}

	return texCache;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTextureCache_init (GBTextureCache* texCache, const char* filename)
{
	GB_ASSERT(texCache && filename);

	texCache->filename = gbStrDuplicate(filename);
	if (!texCache->filename)
		return GB_FALSE;

	{
		GBGtx* gtx = GBGtx_load(filename, texCache->mem);
		if (!gtx)
			return GB_FALSE;

		texCache->width = gtx->width;
		texCache->height = gtx->height;
		texCache->blockWidth = gtx->blockWidth;
		texCache->blockHeight = gtx->blockHeight;

		texCache->palette = gtx->palette;

		{
			GBuint32 i;
			for (i = 0; i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT; i++)
			{
				GBGtxBlock* block = &texCache->infoBlocks[i];
				*block = gtx->infoMap[i];
				block->finalColor =
					(texCache->palette[block->color * 3 + 2] << 16) |
					(texCache->palette[block->color * 3 + 1] << 8) |
					texCache->palette[block->color * 3];
			}
		}

		GBGtx_destroy(gtx, GB_TRUE, texCache->mem);

		texCache->texFile = fopen(filename, "rb");
		if (!texCache->texFile)
			return GB_FALSE;

#ifdef GB_LOAD_WHOLE_TEXTURE_AT_STARTUP
		GBTextureCache_getLevel(texCache, 0);

		{
			GBuint32 i;
			for (i = 0; i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT; i++)
			{
				if (!(texCache->infoBlocks[i].flags & GB_GTX_BLOCK_FLAG_PLAIN))
				{
					if (!GBTextureCache_load(texCache, 0,
						i & GB_TEXTURE_CACHE_BLOCK_DIVISION_H_MASK,
						i >> GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H))
						return GB_FALSE;
				}
			}
		}
#endif
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTextureCache_destroy (GBTextureCache* texCache)
{
	GBuint32 i;
	GB_ASSERT(texCache);

	if (texCache->texFile)
		fclose(texCache->texFile);

	for (i = 0; i < GB_TEXTURE_CACHE_LEVEL_COUNT; i++)
	{
		if (texCache->levels[i])
			GBSplitTexture_destroy(texCache->levels[i], texCache->mem);
		if (texCache->cachedLevels[i])
			GBSplitTexture_destroy(texCache->cachedLevels[i], texCache->mem);
	}

	if (texCache->mem)
		GBMemory_free(texCache->mem, texCache->palette);
	else
		free(texCache->palette);

	free(texCache->filename);
	free(texCache);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint8* GBTextureCache_getPalette (GBTextureCache* texCache)
{
	GB_ASSERT(texCache);
	return texCache->palette;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBGtxBlock* GBTextureCache_getInfoBlocks (GBTextureCache* texCache)
{
	GB_ASSERT(texCache);
	return texCache->infoBlocks;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBSplitTexture* GBTextureCache_getLevel (GBTextureCache* texCache, GBuint32 level)
{
	GB_ASSERT(texCache);
	GB_ASSERT(level < GB_TEXTURE_CACHE_LEVEL_COUNT);

	if (!texCache->levels[level])
	{
		GBSplitTexture* levelTex = GBSplitTexture_create(texCache->mem);
		if (!levelTex)
			return NULL; /* Out of memory, failure */

		levelTex->width = texCache->width >> level;
		levelTex->height = texCache->height >> level;
		levelTex->hMask = levelTex->width - 1;
		levelTex->vMask = levelTex->height - 1;
		levelTex->hBlockMask = (texCache->blockWidth >> level) - 1;
		levelTex->vBlockMask = (texCache->blockHeight >> level) - 1;
		levelTex->hBlockShift = gbHighestBit(texCache->blockWidth >> level);
		levelTex->vBlockShift = gbHighestBit(texCache->blockHeight >> level);

		texCache->levels[level] = levelTex;
	}

	return texCache->levels[level];
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTextureCache_load (GBTextureCache* texCache, GBuint32 level, GBuint32 x, GBuint32 y)
{
	GBuint32 i;
	GB_ASSERT(texCache);
	GB_ASSERT(level < GB_TEXTURE_CACHE_LEVEL_COUNT);
	GB_ASSERT(x < GB_TEXTURE_CACHE_BLOCK_DIVISION_H);
	GB_ASSERT(y < GB_TEXTURE_CACHE_BLOCK_DIVISION_V);
	GB_ASSERT(texCache->levels[level]);

	i = x + y * GB_TEXTURE_CACHE_BLOCK_DIVISION_H;

	GB_ASSERT(i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT);

	if (texCache->levels[level]->blocks[i])
		return GB_TRUE; /* Unnecessary load, return success */

	texCache->levels[level]->blocks[i] = GBGtx_loadBlock(texCache->texFile, x, y, texCache->mem);
	if (!texCache->levels[level]->blocks[i])
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTextureCache_restore (GBTextureCache* texCache, GBuint32 level, GBuint32 x, GBuint32 y)
{
	GBuint32 i;
	GB_ASSERT(texCache);
	GB_ASSERT(level < GB_TEXTURE_CACHE_LEVEL_COUNT);
	GB_ASSERT(x < GB_TEXTURE_CACHE_BLOCK_DIVISION_H);
	GB_ASSERT(y < GB_TEXTURE_CACHE_BLOCK_DIVISION_V);

	i = x + y * GB_TEXTURE_CACHE_BLOCK_DIVISION_H;

	if (texCache->freeingMemory &&
		texCache->cachedLevels[level] &&
		texCache->cachedLevels[level]->blocks[i])
	{
		texCache->levels[level]->blocks[i] = texCache->cachedLevels[level]->blocks[i];
		texCache->cachedLevels[level]->blocks[i] = NULL;
		return GB_TRUE; /* Restored from cache, success */
	}

	return  GB_FALSE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTextureCache_free (GBTextureCache* texCache)
{
	GBuint32 i;
	GB_ASSERT(texCache);

	if (texCache->freeingMemory)
		return;

	for (i = 0; i < GB_TEXTURE_CACHE_LEVEL_COUNT; i++)
	{
		texCache->cachedLevels[i] = texCache->levels[i];
		texCache->levels[i] = NULL;
	}

	texCache->freeingMemory = GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTextureCache_isFreeing (GBTextureCache* texCache)
{
	GB_ASSERT(texCache);
	return texCache->freeingMemory;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTextureCache_stopFreeing (GBTextureCache* texCache)
{
	GBuint32 i;
	GB_ASSERT(texCache);

	for (i = 0; i < GB_TEXTURE_CACHE_LEVEL_COUNT; i++)
	{
		if (texCache->cachedLevels[i])
		{
			GBSplitTexture_destroy(texCache->cachedLevels[i], texCache->mem);
			texCache->cachedLevels[i] = NULL;
		}
	}

	texCache->freeingMemory = GB_FALSE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBSplitTexture* GBSplitTexture_create (GBMemory* mem)
{
	GBSplitTexture* tex = (GBSplitTexture*)GBMemory_alloc(mem, sizeof(GBSplitTexture));
	if (!tex)
		return NULL;

	{
		GBuint32 i;
		for (i = 0; i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT; i++)
			tex->blocks[i] = NULL;
	}

	tex->width = 0;
	tex->height = 0;
	tex->hMask = 0;
	tex->vMask = 0;
	tex->hBlockMask = 0;
	tex->vBlockMask = 0;
	tex->hBlockShift = 0;
	tex->vBlockShift = 0;

	return tex;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBSplitTexture_destroy (GBSplitTexture* tex, GBMemory* mem)
{
	GBuint32 i;
	GB_ASSERT(tex);

	for (i = 0; i < GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT; i++)
		GBMemory_free(mem, tex->blocks[i]);

	GBMemory_free(mem, tex);
}
