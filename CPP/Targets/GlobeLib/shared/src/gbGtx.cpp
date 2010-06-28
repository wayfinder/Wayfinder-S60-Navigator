/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbGtx.h"

#include <stdlib.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGtxBlock_init (GBGtxBlock* block)
{
	GB_ASSERT(block);
	
	block->flags = 0;
	block->dataIndex = 0xffffffff;
	block->color = 0xff;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGtx_save (GBuint32 width, GBuint32 height,
				 GBuint32 blockWidth, GBuint32 blockHeight,
				 GBuint8* palette, GBuint8* data, const char* filename)
{
	FILE* f;
	GBuint32 hBlocks;
	GBuint32 vBlocks;
	GBuint32 bx;
	GBuint32 by;
	GBGtxBlock* infoBlocks;
	GBuint32 index = 0;
	GB_ASSERT(palette && data);

	hBlocks = width / blockWidth;
	vBlocks = height / blockHeight;

	infoBlocks = (GBGtxBlock*)malloc(sizeof(GBGtxBlock) * hBlocks * vBlocks);
	if (!infoBlocks)
		return GB_FALSE;

	f = fopen(filename, "wb");
	if (!f)
	{
		free(infoBlocks);
		return GB_FALSE;
	}

	fwrite(&width, sizeof(GBuint32), 1, f);
	fwrite(&height, sizeof(GBuint32), 1, f);
	fwrite(&blockWidth, sizeof(GBuint32), 1, f);
	fwrite(&blockHeight, sizeof(GBuint32), 1, f);

	fwrite(palette, sizeof(GBuint8), 256 * 3, f);

	for (by = 0; by < vBlocks; by++)
	{
		for (bx = 0; bx < hBlocks; bx++)
		{
			GBGtxBlock* block = &infoBlocks[bx + by * hBlocks];
			GBbool plain = GB_TRUE;
			GBuint32 y;
			GBuint8 firstColor = data[bx * blockWidth + by * blockHeight * width];
			for (y = 0; y < blockHeight; y++)
			{
				GBuint32 x;
				for (x = 0; x < blockWidth; x++)
				{
					if (data[bx * blockWidth + x + (y + by * blockHeight) * width] != firstColor)
					{
						plain = GB_FALSE;
						break;
					}
				}
				if (!plain)
					break;
			}

			block->color = firstColor;

			if (plain)
			{
				block->flags = GB_GTX_BLOCK_FLAG_PLAIN;
				block->dataIndex = 0xffffffff;
			}
			else
			{
				block->flags = 0;
				block->dataIndex = index;
				index++;
			}
		}
	}

	for (by = 0; by < vBlocks; by++)
	{
		for (bx = 0; bx < hBlocks; bx++)
		{
			GBGtxBlock* block = &infoBlocks[bx + by * hBlocks];
			fwrite(&block->flags, sizeof(GBuint32), 1, f);
			fwrite(&block->dataIndex, sizeof(GBuint32), 1, f);
			fwrite(&block->color, sizeof(GBuint8), 1, f);
		}
	}

	for (by = 0; by < vBlocks; by++)
	{
		for (bx = 0; bx < hBlocks; bx++)
		{
			if (!(infoBlocks[bx + by * hBlocks].flags & GB_GTX_BLOCK_FLAG_PLAIN))
			{
				GBuint32 y;
				for (y = 0; y < blockHeight; y++)
					fwrite(data + bx * blockWidth + (y + by * blockHeight) * width, sizeof(GBuint8), blockWidth, f);
			}
		}
	}

	free(infoBlocks);
	fclose(f);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBGtx* GBGtx_load (const char* filename, GBMemory* mem)
{
	GBuint32 i;
	FILE* f;
	GBGtx* gtx;
	GBuint32 hBlocks;
	GBuint32 vBlocks;
	GB_ASSERT(filename);

	f = fopen(filename, "rb");
	if (!f)
		return NULL;

	if (mem)
		gtx = (GBGtx*)GBMemory_alloc(mem, sizeof(GBGtx));
	else
		gtx = (GBGtx*)malloc(sizeof(GBGtx));

	if (!gtx)
	{
		fclose(f);
		return NULL;
	}

	fread(gtx, sizeof(GBuint32), 4, f);

	hBlocks = gtx->width / gtx->blockWidth;
	vBlocks = gtx->height / gtx->blockHeight;

	if (mem)
		gtx->palette = (GBuint8*)GBMemory_alloc(mem, sizeof(GBuint8) * 256 * 3);
	else
		gtx->palette = (GBuint8*)malloc(sizeof(GBuint8) * 256 * 3);

	if (!gtx->palette)
	{
		free(gtx);
		fclose(f);
		return NULL;
	}

	if (mem)
		gtx->infoMap = (GBGtxBlock*)GBMemory_alloc(mem, sizeof(GBGtxBlock) * hBlocks * vBlocks);
	else
		gtx->infoMap = (GBGtxBlock*)malloc(sizeof(GBGtxBlock) * hBlocks * vBlocks);

	if (!gtx->infoMap)
	{
		free(gtx->palette);
		free(gtx);
		fclose(f);
		return NULL;
	}

	fread(gtx->palette, sizeof(GBuint8), 256 * 3, f);

	for (i = 0; i < hBlocks * vBlocks; i++)
	{
		fread(&gtx->infoMap[i].flags, sizeof(GBuint32), 1, f);
		fread(&gtx->infoMap[i].dataIndex, sizeof(GBuint32), 1, f);
		fread(&gtx->infoMap[i].color, sizeof(GBuint8), 1, f);
	}

	fclose(f);

	return gtx;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGtx_destroy (GBGtx* gtx, GBbool keepPalette, GBMemory* mem)
{
	GB_ASSERT(gtx);

	if (mem)
	{
		GBMemory_free(mem, gtx->infoMap);
		if (!keepPalette)
			GBMemory_free(mem, gtx->palette);
		GBMemory_free(mem, gtx);
	}
	else
	{
		free(gtx->infoMap);
		if (!keepPalette)
			free(gtx->palette);
		free(gtx);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint8* GBGtx_loadBlock (FILE* f,
						 GBuint32 blockX, GBuint32 blockY,
						 GBMemory* mem)
{
	GBuint8* block;
	GBuint32 width;
	GBuint32 height;
	GBuint32 blockWidth;
	GBuint32 blockHeight;
	GBuint32 hBlocks;
	GBuint32 vBlocks;
	GBuint32 dataIndex;

	GB_ASSERT(f);

	fseek(f, 0, SEEK_SET);

	fread(&width, sizeof(GBuint32), 1, f);
	fread(&height, sizeof(GBuint32), 1, f);
	fread(&blockWidth, sizeof(GBuint32), 1, f);
	fread(&blockHeight, sizeof(GBuint32), 1, f);

	hBlocks = width / blockWidth;
	vBlocks = height / blockHeight;

	fseek(f, 256 * 3 + GB_GTX_BLOCK_INFO_STRIDE * (blockX + blockY * hBlocks) + 4, SEEK_CUR);
	fread(&dataIndex, sizeof(GBuint32), 1, f);

	if (dataIndex == 0xffffffff)
		return NULL;

	if (mem)
		block = (GBuint8*)GBMemory_alloc(mem, sizeof(GBuint8) * blockWidth * blockHeight);
	else
		block = (GBuint8*)malloc(sizeof(GBuint8) * blockWidth * blockHeight);

	if (!block)
	{
		fclose(f);
		return NULL;
	}

	fseek(f, 4 * 4 + 256 * 3 + hBlocks * vBlocks * GB_GTX_BLOCK_INFO_STRIDE + blockWidth * blockHeight * dataIndex, SEEK_SET);
	fread(block, sizeof(GBuint8), blockWidth * blockHeight, f);

	return block;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool
GBGtx_loadBlockToMem (FILE* f,
					 GBuint32 blockX, GBuint32 blockY,
					 GBuint8* block)
{
	GBuint32 width;
	GBuint32 height;
	GBuint32 blockWidth;
	GBuint32 blockHeight;
	GBuint32 hBlocks;
	GBuint32 vBlocks;
	GBuint32 dataIndex;

	GB_ASSERT(f && block);

	fseek(f, 0, SEEK_SET);

	fread(&width, sizeof(GBuint32), 1, f);
	fread(&height, sizeof(GBuint32), 1, f);
	fread(&blockWidth, sizeof(GBuint32), 1, f);
	fread(&blockHeight, sizeof(GBuint32), 1, f);

	hBlocks = width / blockWidth;
	vBlocks = height / blockHeight;

	fseek(f, 256 * 3 + GB_GTX_BLOCK_INFO_STRIDE * (blockX + blockY * hBlocks) + 4, SEEK_CUR);
	fread(&dataIndex, sizeof(GBuint32), 1, f);

	if (dataIndex == 0xffffffff)
		return GB_FALSE;

	fseek(f, 4 * 4 + 256 * 3 + hBlocks * vBlocks * GB_GTX_BLOCK_INFO_STRIDE + blockWidth * blockHeight * dataIndex, SEEK_SET);
	fread(block, sizeof(GBuint8), blockWidth * blockHeight, f);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
