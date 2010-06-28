/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbDefs.h"

#include <math.h>
#include <string.h>
#include <stdlib.h>

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 gbHighestBit (GBuint32 v)
{
	GBuint32 h = 0;
	v >>= 1;
	while (v)
	{
		h++;
		v >>= 1;
	}
	return h;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 gbHighestBit64 (GBuint64 v)
{
	GBuint32 h = 0;
	v >>= 1;
	while (v != 0)
	{
		h++;
		v >>= 1;
	}
	return h;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint64 gbSqrt64 (GBint64 v)
{
	GB_ASSERT(v > 0);
	{
		GBuint64 vu = (GBuint64)v;
		GBuint32 t = 1 << (gbHighestBit64(vu) >> 1);
		GBuint64 r = 0;

		while (t > 0)
		{
			GBuint64 rt = r + t;
			if (rt * rt <= vu)
				r += t;
			t >>= 1;
		}

		return (GBint64)(r << 8);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32 gbCeil4 (GBint32 v)
{
	return (v + 15) >> 4;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32 gbCeilA (GBint32 v, GBint32 bits)
{
	return (v + (1 << bits) - 1) >> bits;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32 gbFloor4 (GBint32 v)
{
	return v >> 4;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBfloat gbFAbs (GBfloat v)
{
	if (v < 0.0f)
		return -v;
	else
		return v;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBint32 gbAbs (GBint32 v)
{
	if (v < 0)
		return -v;
	else
		return v;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbMatrixIdentity (GBint32* result)
{
	GBuint32 i;
	GB_ASSERT(result);

	for (i = 0; i < 9; i++)
		result[i] = 0;

	result[0] = 65535;
	result[4] = 65535;
	result[8] = 65535;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#define sinf(v)			((GBfloat)sin(v))
#define cosf(v)			((GBfloat)cos(v))

void gbMatrixRotationX (GBfloat angle, GBint32* result)
{
	GB_ASSERT(result);
	gbMatrixIdentity(result);
	angle = (angle * GB_PI) / 180.0f;
	result[4] = (GBint32)(cosf(angle) * 65535.0f);
	result[5] = (GBint32)(-sinf(angle) * 65535.0f);
	result[7] = (GBint32)(sinf(angle) * 65535.0f);
	result[8] = (GBint32)(cosf(angle) * 65535.0f);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbMatrixRotationY (GBfloat angle, GBint32* result)
{
	GB_ASSERT(result);
	gbMatrixIdentity(result);
	angle = (angle * GB_PI) / 180.0f;
	result[0] = (GBint32)(cosf(angle) * 65535.0f);
	result[2] = (GBint32)(sinf(angle) * 65535.0f);
	result[6] = (GBint32)(-sinf(angle) * 65535.0f);
	result[8] = (GBint32)(cosf(angle) * 65535.0f);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbVec3Transform (const GBint32* vec, const GBint32* matrix, GBint32* result)
{
	GBint32 r[3];
	GB_ASSERT(vec && matrix && result);

	r[0] = ((vec[0] * matrix[0]) >> 16) + ((vec[1] * matrix[3]) >> 16) + ((vec[2] * matrix[6]) >> 16);
	r[1] = ((vec[0] * matrix[1]) >> 16) + ((vec[1] * matrix[4]) >> 16) + ((vec[2] * matrix[7]) >> 16);
	r[2] = ((vec[0] * matrix[2]) >> 16) + ((vec[1] * matrix[5]) >> 16) + ((vec[2] * matrix[8]) >> 16);

	result[0] = r[0];
	result[1] = r[1];
	result[2] = r[2];
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbVec3TransformHalfQ (const GBint32* vec, const GBint32* matrix, GBint32* result)
{
	GBint32 r[3];
	GB_ASSERT(vec && matrix && result);

	/* "optimization" */
	r[0] = (((vec[0] >> 1) * matrix[0]) >> 15) /*+ (((vec[1] >> 1) * matrix[3]) >> 15)*/ + (((vec[2] >> 1) * matrix[6]) >> 15);
	r[1] = (((vec[0] >> 1) * matrix[1]) >> 15) + (((vec[1] >> 1) * matrix[4]) >> 15) + (((vec[2] >> 1) * matrix[7]) >> 15);
	r[2] = (((vec[0] >> 1) * matrix[2]) >> 15) + (((vec[1] >> 1) * matrix[5]) >> 15) + (((vec[2] >> 1) * matrix[8]) >> 15);

	result[0] = r[0];
	result[1] = r[1];
	result[2] = r[2];
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbMatrixMultiply (const GBint32* left, const GBint32* right, GBint32* result)
{
	GBint32 i;
	GBint32 tmp[9];

	GB_ASSERT(left && right && result);

	for (i = 0; i < 3; i++)
	{
		GBint32 a = left[i + 0] >> 1;
		GBint32 b = left[i + 3] >> 1;
		GBint32 c = left[i + 6] >> 1;
		tmp[i + 0] = ((a * right[0]) >> 15) + ((b * right[1]) >> 15) + ((c * right[2]) >> 15);
		tmp[i + 3] = ((a * right[3]) >> 15) + ((b * right[4]) >> 15) + ((c * right[5]) >> 15);
		tmp[i + 6] = ((a * right[6]) >> 15) + ((b * right[7]) >> 15) + ((c * right[8]) >> 15);
	}

	memcpy(result, tmp, 9 * sizeof(GBint32));
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFMatrixIdentity (GBfloat* result)
{
	GBuint32 i;
	GB_ASSERT(result);

	for (i = 0; i < 9; i++)
		result[i] = 0.0f;

	result[0] = 1.0f;
	result[4] = 1.0f;
	result[8] = 1.0f;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFMatrixRotationX (GBfloat angle, GBfloat* result)
{
	GB_ASSERT(result);
	gbFMatrixIdentity(result);
	angle = (angle * GB_PI) / 180.0f;
	result[4] = cosf(angle);
	result[5] = -sinf(angle);
	result[7] = sinf(angle);
	result[8] = cosf(angle);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFMatrixRotationY (GBfloat angle, GBfloat* result)
{
	GB_ASSERT(result);
	gbFMatrixIdentity(result);
	angle = (angle * GB_PI) / 180.0f;
	result[0] = cosf(angle);
	result[2] = sinf(angle);
	result[6] = -sinf(angle);
	result[8] = cosf(angle);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFMatrixMultiply (const GBfloat* left, const GBfloat* right, GBfloat* result)
{
	GBint32 i;
	GBfloat tmp[9];

	GB_ASSERT(left && right && result);

	for (i = 0; i < 3; i++)
	{
		GBfloat a = left[i + 0];
		GBfloat b = left[i + 3];
		GBfloat c = left[i + 6];
		tmp[i + 0] = a * right[0] + b * right[1] + c * right[2];
		tmp[i + 3] = a * right[3] + b * right[4] + c * right[5];
		tmp[i + 6] = a * right[6] + b * right[7] + c * right[8];
	}

	memcpy(result, tmp, 9 * sizeof(GBfloat));
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFVec3Transform (const GBfloat* vec, const GBfloat* matrix, GBfloat* result)
{
	GBfloat r[3];
	GB_ASSERT(vec && matrix && result);

	/* Assumption that matrix[1] is always zero. */

	r[0] = vec[0] * matrix[0] + vec[1] * matrix[3] + vec[2] * matrix[6];
	r[1] = /*vec[0] * matrix[1] + */vec[1] * matrix[4] + vec[2] * matrix[7];
	r[2] = vec[0] * matrix[2] + vec[1] * matrix[5] + vec[2] * matrix[8];

	memcpy(result, r, 3 * sizeof(GBfloat));
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbFVec3Normalize (GBfloat* vec)
{
	GBfloat len;
	GB_ASSERT(vec);

	len = (GBfloat)sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
	vec[0] /= len;
	vec[1] /= len;
	vec[2] /= len;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

FILE* gbOpenFileFromPath (const char* path, const char* filename, const char* mode)
{
	FILE* f;
	GB_ASSERT(filename);

	if (path)
	{
		char full[256];
		gbStrCopy(full, path);
		strcat(full, filename);
		f = fopen(full, mode);
	}
	else
	{
		f = fopen(filename, mode);
	}

	return f;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBDisplayParams_init (GBDisplayParams* displayParams)
{
	GB_ASSERT(displayParams);

	displayParams->buffer = NULL;
	displayParams->bufferWidth = 0;

	displayParams->x = 0;
	displayParams->y = 0;

	displayParams->width = 0;
	displayParams->height = 0;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 gbStrLength (const char* str)
{
	GBuint32 len = 0;
	GB_ASSERT(str);

	while (str[len] != 0)
		len++;

	return len;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbStrCopy (char* tgt, const char* src)
{
	GBuint32 i = 0;
	GB_ASSERT(tgt && src);

	while (src[i] != 0)
	{
		*tgt++ = src[i];
		i++;
	}

	*tgt = 0;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

char* gbStrDuplicate (const char* str)
{
	char* dup;
	GBuint32 len;
	GB_ASSERT(str);

	len = gbStrLength(str);
	dup = (char*)malloc(len + 1);
	gbStrCopy(dup, str);

	return dup;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbReadWord (FILE* f, char* out, const char* stopChars)
{
	GBuint32 count;
	GBbool stop = GB_FALSE;
	GB_ASSERT(f && out && stopChars);

	count = gbStrLength(stopChars);

	*out = 0;

	while (!stop && fread(out, 1, 1, f) == 1)
	{
		GBuint32 i;
		for (i = 0; i < count; i++)
		{
			if (*out == stopChars[i])
			{
				stop = GB_TRUE;
				break;
			}
		}
		if (!stop)
			out++;
	}
	*out = 0;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void gbSkipChars (FILE* f, const char* chars)
{
	GBuint32 count;
	GBbool stop = GB_FALSE;
	GB_ASSERT(f && chars);

	count = gbStrLength(chars);

	while (!stop)
	{
		char c;
		GBuint32 pos = (GBuint32)ftell(f);
		GBuint32 i;

		if (fread(&c, 1, 1, f) == 0)
			return;

		stop = GB_TRUE;
		for (i = 0; i < count; i++)
		{
			if (c == chars[i])
			{
				stop = GB_FALSE;
				break;
			}
		}

		if (stop)
			fseek(f, pos, SEEK_SET);
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBfloat gbStringToFloat (const char* str)
{
	int neg = 0;
	float v = 0.0f;
	int i = 0;

	const int ASCII_NUMBERS_OFFSET = 48;
	const char DECIMAL_SEPARATOR = '.';
	const char MINUS_SIGN = '-';

	if (str[0] == MINUS_SIGN)
	{
		neg = 1;
		i++;
	}

	while (str[i] != 0 && str[i] != DECIMAL_SEPARATOR)
	{
		v *= 10.0f;
		v += (float)((int)str[i] - ASCII_NUMBERS_OFFSET);
		i++;
	}

	if (str[i] == DECIMAL_SEPARATOR)
	{
		int d = 10;
		i++;
		while (str[i] != 0)
		{
			v += (float)((int)str[i] - ASCII_NUMBERS_OFFSET) / (float)d;
			d *= 10;
			i++;
		}
	}

	if (neg)
		v = -v;

	return v;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#ifdef __VC32__

#define __NAKED__ __declspec( naked )

//extern "C"
//	{
__NAKED__  _allmul()
 {
  _asm mov         eax,dword ptr [esp+8]
  _asm mov         ecx,dword ptr [esp+10h]
  _asm or          ecx,eax
  _asm mov         ecx,dword ptr [esp+0Ch]
  _asm jne         label1
  _asm mov         eax,dword ptr [esp+4]
  _asm mul         ecx
  _asm ret 16
    label1:
 _asm push        ebx
  _asm mul         ecx
  _asm mov         ebx , eax
  _asm mov         eax,dword ptr [esp+8]
  _asm mul         dword ptr [esp+14h]
  _asm add         ebx,eax
  _asm mov         eax,dword ptr [esp+8]
  _asm mul         ecx
  _asm add         edx,ebx
  _asm pop         ebx
  _asm ret         10h
  }

__NAKED__  _allshl()
    {
        _asm cmp         cl,40h
        _asm jae         label3
        _asm cmp         cl,20h
        _asm jae         label2
        _asm  shld        edx,eax,cl
        _asm shl         eax,cl
        _asm ret
        label2:
        _asm mov         edx,eax
        _asm xor         eax,eax
        _asm and         cl,1Fh
        _asm shl         edx,cl
        _asm ret
        label3:
        _asm xor         eax,eax
        _asm xor         edx,edx
        _asm ret
    }

__NAKED__  _allshr()
    {
        _asm cmp         cl,40h
        _asm jae         label3
        _asm cmp         cl,20h
        _asm jae         label2
        _asm shrd		 eax,edx,cl
        _asm sar		 edx,cl
        _asm ret
		label2:
		_asm mov		 eax,edx
        _asm sar		 edx,31
        _asm and		 cl,31
        _asm sar		 eax,cl
        _asm ret
        label3:
        _asm sar		 edx,31
		_asm mov		 eax,edx
        _asm ret
    }

__NAKED__  _alldiv()
    {
        _asm push        edi
        _asm push        esi
        _asm push        ebx
        _asm xor         edi,edi
        _asm mov         eax,dword ptr [esp+14h]
        _asm or          eax,eax
        _asm jge         label4
        _asm inc         edi
        _asm mov         edx,dword ptr [esp+10h]
        _asm neg         eax
        _asm neg         edx
        _asm sbb         eax,0
        _asm mov         dword ptr [esp+14h],eax
        _asm mov         dword ptr [esp+10h],edx
        label4:
        _asm mov         eax,dword ptr [esp+1Ch]
        _asm or          eax,eax
        _asm jge         label5
        _asm inc         edi
        _asm mov         edx,dword ptr [esp+18h]
        _asm neg         eax
        _asm neg         edx
        _asm sbb         eax,0
        _asm mov         dword ptr [esp+1Ch],eax
        _asm mov         dword ptr [esp+18h],edx
        label5:
        _asm or          eax,eax
        _asm jne         label6
        _asm mov         ecx,dword ptr [esp+18h]
        _asm mov         eax,dword ptr [esp+14h]
        _asm xor         edx,edx
        _asm div         ecx
        _asm mov         ebx,eax
        _asm mov         eax,dword ptr [esp+10h]
        _asm  div        ecx
        _asm  mov         edx,ebx
        _asm  jmp         label8
        label6:
        _asm  mov         ebx,eax
        _asm mov         ecx,dword ptr [esp+18h]
        _asm mov         edx,dword ptr [esp+14h]
        _asm  mov         eax,dword ptr [esp+10h]
        label7:
        _asm shr         ebx,1
        _asm  rcr         ecx,1
        _asm  shr         edx,1
        _asm  rcr         eax,1
        _asm  or          ebx,ebx
        _asm  jne         label7
        _asm  div         ecx
        _asm mov         esi,eax
        _asm  mul         dword ptr [esp+1Ch]
        _asm mov         ecx,eax
        _asm mov         eax,dword ptr [esp+18h]
        _asm  mul         esi
        _asm  add         edx,ecx
        _asm  jb          label9
        _asm  cmp         edx,dword ptr [esp+14h]
        _asm ja           label9
        _asm  jb          label10
        _asm  cmp         eax,dword ptr [esp+10h]
        _asm  jbe         label10
label9:
        _asm dec         esi
label10:
        _asm  xor         edx,edx
        _asm  mov         eax,esi
label8:
        _asm dec         edi
        _asm  jne        label11
        _asm neg         edx
        _asm  neg         eax
        _asm sbb         edx,0
label11:
        _asm  pop         ebx
        _asm  pop         esi
        _asm  pop         edi
        _asm  ret         10h
    }
//}

#endif //__VC32__
