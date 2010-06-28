/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBTEXTURECACHE_H
#define GBTEXTURECACHE_H

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#include "gbDefs.h"
#include "gbMemory.h"
#include "gbGtx.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#define GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H		5
#define GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_V		4
#define GB_TEXTURE_CACHE_BLOCK_DIVISION_H			(1 << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_H)
#define GB_TEXTURE_CACHE_BLOCK_DIVISION_V			(1 << GB_TEXTURE_CACHE_BLOCK_DIVISION_SHIFT_V)
#define GB_TEXTURE_CACHE_BLOCK_DIVISION_H_MASK		(GB_TEXTURE_CACHE_BLOCK_DIVISION_H - 1)
#define GB_TEXTURE_CACHE_BLOCK_DIVISION_V_MASK		(GB_TEXTURE_CACHE_BLOCK_DIVISION_V - 1)
#define GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT			(GB_TEXTURE_CACHE_BLOCK_DIVISION_H * GB_TEXTURE_CACHE_BLOCK_DIVISION_V)

#define GB_TEXTURE_CACHE_LEVEL_COUNT				1

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBSplitTexture_s
{
	void* blocks[GB_TEXTURE_CACHE_TOTAL_BLOCK_COUNT];
	GBuint32 width;
	GBuint32 height;
	GBuint32 hMask;
	GBuint32 vMask;
	GBuint32 hBlockMask;
	GBuint32 vBlockMask;
	GBuint32 hBlockShift;
	GBuint32 vBlockShift;
} GBSplitTexture;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBTextureCache_s GBTextureCache;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBTextureCache*			GBTextureCache_create			(const char* filename, GBMemory* mem);
void					GBTextureCache_destroy			(GBTextureCache* texCache);

GBuint8*				GBTextureCache_getPalette		(GBTextureCache* texCache);
GBGtxBlock*				GBTextureCache_getInfoBlocks	(GBTextureCache* texCache);
GBSplitTexture*			GBTextureCache_getLevel			(GBTextureCache* texCache, GBuint32 level);

GBbool					GBTextureCache_load				(GBTextureCache* texCache, GBuint32 level, GBuint32 x, GBuint32 y);
GBbool					GBTextureCache_restore			(GBTextureCache* texCache, GBuint32 level, GBuint32 x, GBuint32 y);

void					GBTextureCache_free				(GBTextureCache* texCache);
GBbool					GBTextureCache_isFreeing		(GBTextureCache* texCache);
void					GBTextureCache_stopFreeing		(GBTextureCache* texCache);

GB_EXTERN_C_BLOCK_END

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#endif
