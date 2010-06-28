/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBGTX_H
#define GBGTX_H

#include "gbDefs.h"
#include "gbMemory.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBGtxBlock_s
{
	GBuint32 flags;
	GBuint32 dataIndex;
	GBuint8 color;
	GBuint32 finalColor;
} GBGtxBlock;

#define GB_GTX_BLOCK_INFO_STRIDE	9

GB_EXTERN_C_BLOCK_BEGIN

void	GBGtxBlock_init		(GBGtxBlock* block);

GB_EXTERN_C_BLOCK_END

enum
{
	GB_GTX_BLOCK_FLAG_PLAIN = 1,
	GB_GTX_BLOCK_FLAG_MAX
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBGtx_s
{
	GBuint32 width;
	GBuint32 height;

	GBuint32 blockWidth;
	GBuint32 blockHeight;

	GBuint8* palette;

	GBGtxBlock* infoMap;
} GBGtx;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBbool				GBGtx_save				(GBuint32 width, GBuint32 height,
											GBuint32 blockWidth, GBuint32 blockHeight,
											GBuint8* palette, GBuint8* data,
											 const char* filename);

GBGtx*				GBGtx_load				(const char* filename, GBMemory* mem);
void				GBGtx_destroy			(GBGtx* gtx, GBbool keepPalette, GBMemory* mem);

GBuint8*			GBGtx_loadBlock			(FILE* f,
											GBuint32 blockX, GBuint32 blockY,
											GBMemory* mem);
GBbool				GBGtx_loadBlockToMem	(FILE* f,
											GBuint32 blockX, GBuint32 blockY,
											GBuint8* block);

GB_EXTERN_C_BLOCK_END

#endif
