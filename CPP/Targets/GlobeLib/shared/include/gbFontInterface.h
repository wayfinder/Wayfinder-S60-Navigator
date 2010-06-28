/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBFONTINTERFACE_H
#define GBFONTINTERFACE_H

#include "gbDefs.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBFontParams_s
{
	GBbool		italic;
	GBbool		bold;
	GBint32		size;
	GBuint32	color;
	char*		font;
} GBFontParams;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef void		(*GBFontInterface_setParamsFunc)	(void* userPtr, const GBFontParams* params);
typedef GBuint32	(*GBFontInterface_getWidthFunc)		(void* userPtr, const char* text);
typedef GBuint32	(*GBFontInterface_getHeightFunc)	(void* userPtr);
typedef void		(*GBFontInterface_renderFunc)		(void* userPtr, const char* text, const GBTextRect* pos);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBFontInterface_s
{
	GBFontInterface_setParamsFunc		setParams;
	GBFontInterface_getWidthFunc		getWidth;
	GBFontInterface_getHeightFunc		getHeight;
	GBFontInterface_renderFunc			render;
} GBFontInterface;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//


GB_EXTERN_C_BLOCK_BEGIN

void	GBFontParams_init		(GBFontParams* fontParams);

void	GBFontInterface_init	(GBFontInterface* fontInterface);

GB_EXTERN_C_BLOCK_END

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#endif

