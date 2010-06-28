/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBTGA_H
#define GBTGA_H

#include "gbDefs.h"
#include "gbMemory.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct
{
	GBuint8		identsize;			/* size of ID field that follows 18 int8 header (0 usually) */
	GBuint8		colourmaptype;		/* type of colour map 0=none, 1=has palette */
	GBuint8		imagetype;			/* type of image 0=none,1=indexed,2=rgb,3=grey,+8=rle packed */

	GBuint8		alignA;

	/* 4 */

	GBuint16	colourmapstart;		/* first colour map entry in palette */
	GBuint16	colourmaplength;	/* number of colours in palette */
	GBuint8		colourmapbits;		/* number of bits per palette entry 15,16,24,32 */

	GBuint8		alignB[3];

	/* 12 */

	GBuint16	xstart;				/* image x origin */
	GBuint16	ystart;				/* image y origin */
	GBuint16	width;				/* image width in pixels */
	GBuint16	height;				/* image height in pixels */
	GBuint8		bits;				/* image bits per pixel 8,16,24,32 */
	GBuint8		descriptor;			/* image descriptor bits (vh flip bits) */

	GBuint8		alignC[2];

	GBuint8*	data;
} GBTga;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBTga*				GBTga_load			(const char* filename, GBMemory* mem);
GBTga*				GBTga_loadArea		(const char* filename, GBint32 x, GBint32 y, GBint32 width, GBint32 height, GBMemory* mem);
GBTga*				GBTga_loadHeader	(const char* filename, GBMemory* mem);
void				GBTga_destroy		(GBTga* tga, GBbool keepData, GBMemory* mem);

GBbool				GBTga_save			(const char* filename, GBuint32 width, GBuint32 height, void* data);

GB_EXTERN_C_BLOCK_END

#endif /* BBTGA_H */

