/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBGLOBE_H
#define GBGLOBE_H

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#include "gbDefs.h"
#include "gbFontInterface.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBCityOut_s
{
	char		name[256];
	GBvec2		coords;
} GBCityOut;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBGlobe_s GBGlobe;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBGlobe*		GBGlobe_create				(const char* dataPath, 
                                     const char* countryListFileName,
                                     const char* cityListFileName);
void			GBGlobe_destroy				(GBGlobe* globe);

GBbool			GBGlobe_setDisplay			(GBGlobe* globe, GBDisplayParams* displayParams);

void			GBGlobe_setFont				(GBGlobe* globe, GBFontInterface* fontInterface, void* fontContext);

void			GBGlobe_update				(GBGlobe* globe, const GBvec2 coords,
											 GBfloat distance, const GBivec2 pointer);
GBbool			GBGlobe_render				(GBGlobe* globe);
GBbool			GBGlobe_renderTexts			(GBGlobe* globe);

const char*		GBGlobe_lookupCountry		(GBGlobe* globe, const GBvec2 coords);
const char*		GBGlobe_lookupTimezone		(GBGlobe* globe, const GBvec2 coords);
GBbool			GBGlobe_lookupCity			(GBGlobe* globe, const GBvec2 coords, GBCityOut* out);

void			GBGlobe_worldToScreen		(GBGlobe* globe, GBfloat lat, GBfloat lon, GBfloat* out);
void			GBGlobe_screenToNormal		(GBGlobe* globe, GBfloat x, GBfloat y, GBfloat* out);
void			GBGlobe_worldToScreenNormal	(GBGlobe* globe, GBfloat lat, GBfloat lon, GBfloat* out);
void			GBGlobe_worldToWorldNormal	(GBGlobe* globe, GBfloat lat, GBfloat lon, GBfloat* out);
void			GBGlobe_normalToWorld		(GBGlobe* globe, const GBfloat* normal, GBfloat* out);
void			GBGlobe_screenToWorld		(GBGlobe* globe, GBfloat x, GBfloat y, GBfloat* out);

GBbool			GBGlobe_isPointInside		(GBGlobe* globe, GBfloat x, GBfloat y);

GB_EXTERN_C_BLOCK_END

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#endif
