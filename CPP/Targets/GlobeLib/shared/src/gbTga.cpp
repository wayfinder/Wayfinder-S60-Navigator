/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbTga.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBTga* GBTga_load (const char* filename, GBMemory* mem)
{
	GBTga* tga;
	FILE* f;
	GBuint32 size;

	GB_ASSERT(filename);

	f = fopen(filename, "rb");
	if (!f)
	{
#ifdef GB_DEVEL
		printf("Error: Unable to open tga file \"%s\"\n", filename);
#endif
		return NULL;
	}

	if (mem)
        tga = (GBTga*)GBMemory_alloc(mem, sizeof(GBTga));
	else
		tga = (GBTga*)malloc(sizeof(GBTga));

	if (!tga)
	{
		fclose(f);
		return NULL;
	}

	fread(tga, 1, 3, f);
	fread((GBuint8*)(tga) + 4, 1, 5, f);
	fread((GBuint8*)(tga) + 12, 1, 10, f);

	if (tga->bits != 24)
	{
		if (mem)
			GBMemory_free(mem, tga);
		else
			free(tga);

		fclose(f);
		return NULL;
	}

	size = tga->width * tga->height * 3;

	if (mem)
		tga->data = (GBuint8*)GBMemory_alloc(mem, size);
	else
		tga->data = (GBuint8*)malloc(size);

	if (!tga->data)
	{
		GBTga_destroy(tga, GB_FALSE, mem);
		return NULL;
	}

	fread(tga->data, 1, size, f);
	fclose(f);

	return tga;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBTga* GBTga_loadArea (const char* filename, GBint32 x, GBint32 y, GBint32 width, GBint32 height, GBMemory* mem)
{
	GBTga* tga;
	FILE* f;
	GBuint32 size;

	GB_ASSERT(filename);

	f = fopen(filename, "rb");
	if (!f)
	{
#ifdef GB_DEVEL
		printf("Error: Unable to open tga file \"%s\"\n", filename);
#endif
		return NULL;
	}

	if (mem)
		tga = (GBTga*)GBMemory_alloc(mem, sizeof(GBTga));
	else
		tga = (GBTga*)malloc(sizeof(GBTga));

	if (!tga)
	{
		fclose(f);
		return NULL;
	}

	fread(tga, 1, 3, f);
	fread((GBuint8*)(tga) + 4, 1, 5, f);
	fread((GBuint8*)(tga) + 12, 1, 10, f);

	if (tga->bits != 24 ||
		(GBint32)tga->width - x < width ||
		(GBint32)tga->height - y < height)
	{
		if (mem)
			GBMemory_free(mem, tga);
		else
			free(tga);

		fclose(f);
		return NULL;
	}

	size = width * height * 3;

	if (mem)
		tga->data = (GBuint8*)GBMemory_alloc(mem, size);
	else
		tga->data = (GBuint8*)malloc(size);

	if (!tga->data)
	{
		GBTga_destroy(tga, GB_FALSE, mem);
		return NULL;
	}

	fseek(f, (x + y * tga->width) * 3, SEEK_CUR);

	{
		GBint32 cy = 0;
		while (cy < height)
		{
			fread(tga->data + (cy * width) * 3, 1, width * 3, f);
			fseek(f, (tga->width - width) * 3, SEEK_CUR);
			cy++;
		}
	}

	fclose(f);

	tga->width = (GBuint16)width;
	tga->height = (GBuint16)height;

	return tga;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBTga* GBTga_loadHeader (const char* filename, GBMemory* mem)
{
	GBTga* tga;
	FILE* f;

	GB_ASSERT(filename);

	f = fopen(filename, "rb");
	if (!f)
	{
#ifdef GB_DEVEL
		printf("Error: Unable to open tga file \"%s\"\n", filename);
#endif
		return NULL;
	}

	if (mem)
		tga = (GBTga*)GBMemory_alloc(mem, sizeof(GBTga));
	else
		tga = (GBTga*)malloc(sizeof(GBTga));

	if (!tga)
	{
		fclose(f);
		return NULL;
	}

	fread(tga, 1, 3, f);
	fread((GBuint8*)(tga) + 4, 1, 5, f);
	fread((GBuint8*)(tga) + 12, 1, 10, f);

	fclose(f);

	tga->data = NULL;

	return tga;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBTga_destroy (GBTga* tga, GBbool keepData, GBMemory* mem)
{
	GB_ASSERT(tga);

	if (mem)
	{
		if (!keepData)
			GBMemory_free(mem, tga->data);

		GBMemory_free(mem, tga);
	}
	else
	{
		if (!keepData)
			free(tga->data);

		free(tga);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBTga_save (const char* filename, GBuint32 width, GBuint32 height, void* data)
{
	GBTga tga;
	FILE* f;
	GB_ASSERT(data && filename);

	tga.identsize = 0;
	tga.colourmaptype = 0;
	tga.imagetype = 2;
	tga.colourmapstart = 0;
	tga.colourmaplength = 0;
	tga.colourmapbits = 0;
	tga.xstart = 0;
	tga.ystart = 0;
	tga.width = (GBuint16)width;
	tga.height = (GBuint16)height;
	tga.bits = 24;
	tga.descriptor = 0;

	f = fopen(filename, "wb");
	if (!f)
		return GB_FALSE;

	fwrite(&tga, 1, 3, f);
	fwrite((GBuint8*)(&tga) + 4, 1, 5, f);
	fwrite((GBuint8*)(&tga) + 12, 1, 10, f);
	fwrite(data, width * height * 3, 1, f);

	fclose(f);

	return GB_TRUE;
}
