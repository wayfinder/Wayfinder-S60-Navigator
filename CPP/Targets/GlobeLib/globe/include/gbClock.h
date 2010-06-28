/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GBCLOCK_H
#define GBCLOCK_H

#include "gbDefs.h"

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

typedef struct GBClock_s GBClock;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GB_EXTERN_C_BLOCK_BEGIN

GBClock*			GBClock_create			(const char* dataPath);
void				GBClock_destroy			(GBClock* clock);

GBbool				GBClock_setSize			(GBClock* clock, GBuint32 size);
void				GBClock_setCorner		(GBClock* clock, GBuint32 left, GBuint32 top);

void				GBClock_setTime			(GBClock* clock, GBuint32 time);
void				GBClock_setOffset		(GBClock* clock, GBint32 offset);

void				GBClock_draw			(GBClock* clock, const GBDisplayParams* displayParams);

GB_EXTERN_C_BLOCK_END

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#endif
