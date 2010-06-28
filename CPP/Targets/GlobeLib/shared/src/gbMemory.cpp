/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbMemory.h"

#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBAllocBlock_s GBAllocBlock;

struct GBAllocBlock_s
{
	void*			ptr;
	GBuint32		bytes;
	GBAllocBlock*	next;
};

static GBAllocBlock*	GBAllocBlock_create		(void* ptr, GBuint32 bytes);
static void				GBAllocBlock_destroy	(GBAllocBlock* block);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBMemory_s
{
	GBuint32		used;
	GBuint32		limit;
	GBAllocBlock*	firstBlock;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBMemory* GBMemory_create (GBuint32 limit)
{
	GBMemory* mem = (GBMemory*)malloc(sizeof(GBMemory));
	if (!mem)
		return NULL;

	mem->used = 0;
	mem->limit = limit;
	mem->firstBlock = NULL;

	return mem;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBMemory_destroy (GBMemory* mem)
{
	GB_ASSERT(mem);

#ifdef GB_DEVEL
	if (mem->firstBlock)
		printf("Error: Detected memory leaks\n");
#endif

	free(mem);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void* GBMemory_alloc (GBMemory* mem, GBuint32 bytes)
{
	GBAllocBlock* block;
	void* ptr;
	GB_ASSERT(mem);

	if (mem->used + bytes > mem->limit)
		return NULL;

	ptr = malloc(bytes);
	if (!ptr)
		return NULL;

	block = GBAllocBlock_create(ptr, bytes);
	if (!block)
	{
		free(ptr);
		return NULL;
	}

	mem->used += bytes;

	block->next = mem->firstBlock;
	mem->firstBlock = block;

	return ptr;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBMemory_free (GBMemory* mem, void* ptr)
{
	GBAllocBlock* last = NULL;
	GBAllocBlock* block;
	GB_ASSERT(mem);

	if (!ptr)
		return;

	block = mem->firstBlock;
	while (block)
	{
		if (block->ptr == ptr)
		{
			if (last)
				last->next = block->next;
			else
				mem->firstBlock = block->next;

			mem->used -= block->bytes;
			GBAllocBlock_destroy(block);
			return;
		}
		last = block;
		block = block->next;
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBAllocBlock* GBAllocBlock_create (void* ptr, GBuint32 bytes)
{
	GBAllocBlock* block;
	GB_ASSERT(ptr);

	block = (GBAllocBlock*)malloc(sizeof(GBAllocBlock));
	if (!block)
		return NULL;

	block->ptr = ptr;
	block->bytes = bytes;
	block->next = NULL;

	return block;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBAllocBlock_destroy (GBAllocBlock* block)
{
	GB_ASSERT(block);
	free(block->ptr);
	free(block);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
