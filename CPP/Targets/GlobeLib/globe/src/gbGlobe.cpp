/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "gbGlobe.h"
#include "gbMemory.h"
#include "gbRle.h"
#include "gbBitmap.h"
#include "gbRect.h"
#include "gbRayMath.h"

#define POLYGON_GLOBE

#if defined(POLYGON_GLOBE) || defined(USE_GLES)
#	include "gbPolySphere.h"
#	define SPHERE_CLASS			GBPolySphere
#	define SPHERE_CREATE		GBPolySphere_create
#	define SPHERE_DESTROY		GBPolySphere_destroy
#	define SPHERE_SETDISPLAY	GBPolySphere_setDisplay
#	define SPHERE_UPDATE		GBPolySphere_update
#	define SPHERE_DRAW			GBPolySphere_draw
#else
#	include "gbRaySphere.h"
#	define SPHERE_CLASS			GBRaySphere
#	define SPHERE_CREATE		GBRaySphere_create
#	define SPHERE_DESTROY		GBRaySphere_destroy
#	define SPHERE_SETDISPLAY	GBRaySphere_setDisplay
#	define SPHERE_UPDATE		GBRaySphere_update
#	define SPHERE_DRAW			GBRaySphere_draw
#endif

#include <stdlib.h>
#include <math.h>
#include <string.h>

//--------------------------------------------------------------------------//
// Filenames
//--------------------------------------------------------------------------//

const char* const GB_COUNTRY_DATA_FILENAME		= "country.dat";
const char* const GB_TIMEZONE_DATA_FILENAME		= "timezone.dat";

const char* const GB_COUNTRY_LIST_FILENAME		= "country.txt";
const char* const GB_TIMEZONE_LIST_FILENAME		= "timezone.txt";

#ifdef POLYGON_GLOBE
//const char* const GB_TEXTURE_FILENAME			= "earth1024poly.gtx";
const char* const GB_TEXTURE_FILENAME			= "earth2048poly.gtx";
#else
//const char* const GB_TEXTURE_FILENAME			= "earth1024.gtx";
const char* const GB_TEXTURE_FILENAME			= "earth2048.gtx";
#endif

const char* const GB_CITIES_FILENAME			= "cities.txt";
const char* const GB_CITYDOT_FILENAME			= "citydot.tga";

const char* const GB_CROSS_FILENAME				= "cross.tga";

//--------------------------------------------------------------------------//
// Some static data sizes
//--------------------------------------------------------------------------//

#define GB_STRING_LENGTH				256

#define	GB_MAX_COUNTRIES				300
#define	GB_MAX_TIMEZONES				50

//--------------------------------------------------------------------------//
// Limit for the internal memory pool
//--------------------------------------------------------------------------//

//const GBuint32	GB_MEMORY_LIMIT			= 360 * 1024;
const GBuint32	GB_MEMORY_LIMIT			= 2 * 1024 * 1024;

//--------------------------------------------------------------------------//
// City struct
//--------------------------------------------------------------------------//

typedef struct GBCity_s
{
	float		x;
	float		y;
	char*		name;
	GBint32		sx;
	GBint32		sy;
} GBCity;

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

struct GBGlobe_s
{
	char*					dataPath;
	char*				   countryListFileName;
	char*				   cityListFileName;
	GBDisplayParams			displayParams;

	GBMemory*				mem;

	GBint32					pointer[2];
	GBuint32				currentTime;

	GBfloat					coords[2];
	GBfloat					distance;
	GBint32					cameraZ;

	GBfloat					finalMatrixF[9];

	GBuint16*				countryData;
	GBuint16				countryDataHeight;
	GBuint16*				timezoneData;
	GBuint16				timezoneDataHeight;

	char					countryStr[GB_STRING_LENGTH];
	char					timezoneStr[GB_STRING_LENGTH];

	char*					countryList[GB_MAX_COUNTRIES];
	GBuint32				countryCount;
	char*					timezoneList[GB_MAX_TIMEZONES];
	GBuint32				timezoneCount;

	GBCity*					cities;
	GBuint32				cityCount;

	GBBitmap*				cityDotBitmap;

	GBBitmap*				crossBitmap;

	GBFontInterface*		fontInterface;
	void*					fontContext;

	GBRectList*				rectList;

	SPHERE_CLASS*			sphere;
};

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

static GBbool		GBGlobe_init				(GBGlobe* globe);

static GBbool		GBGlobe_renderCities		(GBGlobe* globe);

static GBuint32		GBGlobe_loadStrings			(GBGlobe* globe, const char* filename, GBuint32 max, char** out);

static GBbool		GBGlobe_loadCities			(GBGlobe* globe);

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBGlobe* GBGlobe_create (const char* dataPath,
                         const char* countryListFileName,
                         const char* cityListFileName)
{
	GBGlobe* globe;

	GB_ASSERT(dataPath);

	globe = (GBGlobe*)malloc(sizeof(GBGlobe));
	if (!globe)
		return NULL;

	globe->dataPath = gbStrDuplicate(dataPath);
	if (!globe->dataPath)
	{
		free(globe);
		return NULL;
	}

   if ( countryListFileName ) {
      globe->countryListFileName = gbStrDuplicate( countryListFileName );
   } else {
      globe->countryListFileName = gbStrDuplicate( GB_COUNTRY_LIST_FILENAME );
   }

   if ( !globe->countryListFileName ) {
      free(globe);
      return NULL;
   }
  
   if ( cityListFileName ) {
      globe->cityListFileName = gbStrDuplicate( cityListFileName );
   } else {
      globe->cityListFileName = gbStrDuplicate( GB_CITIES_FILENAME );
   }
   
   if ( !globe->cityListFileName ) {
      free(globe);
      return NULL;
   }

	GBDisplayParams_init(&globe->displayParams);

	globe->mem = NULL;

	globe->pointer[0] = 0;
	globe->pointer[1] = 0;

	globe->currentTime = 0;

	globe->coords[0] = 0.0f;
	globe->coords[1] = 0.0f;
	globe->distance = 0.0f;
	globe->cameraZ = 0;

	globe->countryData = NULL;
	globe->countryDataHeight = 0;
	globe->timezoneData = NULL;
	globe->timezoneDataHeight = 0;

	memset(globe->countryStr, 0, GB_STRING_LENGTH);
	memset(globe->timezoneStr, 0, GB_STRING_LENGTH);

	memset(globe->countryList, 0, sizeof(char*) * GB_MAX_COUNTRIES);
	globe->countryCount = 0;
	memset(globe->timezoneList, 0, sizeof(char*) * GB_MAX_TIMEZONES);
	globe->timezoneCount = 0;

	globe->cities = NULL;
	globe->cityCount = 0;

	globe->cityDotBitmap = NULL;

	globe->crossBitmap = NULL;

	globe->fontInterface = NULL;
	globe->fontContext = NULL;

	globe->rectList = NULL;

	globe->sphere = NULL;

	if (!GBGlobe_init(globe))
	{
		GBGlobe_destroy(globe);
		return NULL;
	}

	return globe;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_init (GBGlobe* globe)
{
	GB_ASSERT(globe);

	if (!gbLoadRLE(globe->dataPath, GB_COUNTRY_DATA_FILENAME, &globe->countryDataHeight, &globe->countryData))
		return GB_FALSE;
	if (!gbLoadRLE(globe->dataPath, GB_TIMEZONE_DATA_FILENAME, &globe->timezoneDataHeight, &globe->timezoneData))
		return GB_FALSE;

	globe->countryCount = GBGlobe_loadStrings(globe, globe->countryListFileName, GB_MAX_COUNTRIES, globe->countryList);
	globe->timezoneCount = GBGlobe_loadStrings(globe, GB_TIMEZONE_LIST_FILENAME, GB_MAX_TIMEZONES, globe->timezoneList);

	globe->mem = GBMemory_create(GB_MEMORY_LIMIT);
	if (!globe->mem)
		return GB_FALSE;

	{
		char full[256];
		gbStrCopy(full, globe->dataPath);
		strcat(full, GB_TEXTURE_FILENAME);
		globe->sphere = SPHERE_CREATE(full, globe->mem);
		if (!globe->sphere)
			return GB_FALSE;
	}

	GBGlobe_loadCities(globe);

	globe->cityDotBitmap = GBBitmap_createFromTga(globe->dataPath, GB_CITYDOT_FILENAME, globe->mem);
	if (!globe->cityDotBitmap)
		return GB_FALSE;

	globe->crossBitmap = GBBitmap_createFromTga(globe->dataPath, GB_CROSS_FILENAME, globe->mem);
	if (!globe->crossBitmap)
		return GB_FALSE;

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_destroy (GBGlobe* globe)
{
	GBuint32 i;
	GB_ASSERT(globe);

	gbClearRectList(&globe->rectList);

	if (globe->crossBitmap)
		GBBitmap_destroy(globe->crossBitmap, globe->mem);

	if (globe->cityDotBitmap)
		GBBitmap_destroy(globe->cityDotBitmap, globe->mem);

	for (i = 0; i < globe->cityCount; i++)
		free(globe->cities[i].name);

	if (globe->mem && globe->cities)
		GBMemory_free(globe->mem, globe->cities);

	SPHERE_DESTROY(globe->sphere);

	if (globe->mem)
		GBMemory_destroy(globe->mem);

	for (i = 0; i < globe->timezoneCount; i++)
		free(globe->timezoneList[i]);

	for (i = 0; i < globe->countryCount; i++)
		free(globe->countryList[i]);

	free(globe->timezoneData);
	free(globe->countryData);

	free(globe->dataPath);
	free(globe->countryListFileName);
	free(globe->cityListFileName);

	free(globe);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_setDisplay (GBGlobe* globe, GBDisplayParams* displayParams)
{
	GB_ASSERT(globe && displayParams);

	globe->displayParams = *displayParams;

	return SPHERE_SETDISPLAY(globe->sphere, displayParams);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_setFont (GBGlobe* globe, GBFontInterface* fontInterface, void* fontContext)
{
	GB_ASSERT(globe && fontInterface);
	globe->fontInterface = fontInterface;
	globe->fontContext = fontContext;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_update (GBGlobe* globe, const GBvec2 coords,
					 GBfloat distance, const GBivec2 pointer)
{
	GBfloat rotationMatrixF[9];
	GB_ASSERT(globe);

	globe->coords[0] = coords[0];
	globe->coords[1] = coords[1];

	gbFMatrixRotationX(-globe->coords[1], rotationMatrixF);
	gbFMatrixRotationY(-globe->coords[0], globe->finalMatrixF);
	gbFMatrixMultiply(globe->finalMatrixF, rotationMatrixF, globe->finalMatrixF);

	globe->distance = distance;
	globe->cameraZ = gbDistanceToCameraZ(globe->distance);

	globe->pointer[0] = pointer[0];
	globe->pointer[1] = pointer[1];

	SPHERE_UPDATE(globe->sphere, coords, distance);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_render (GBGlobe* globe)
{
	GB_ASSERT(globe);

	if (!SPHERE_DRAW(globe->sphere))
		return GB_FALSE;

	if (globe->distance <= 0.75f)
		if (!GBGlobe_renderCities(globe))
			return GB_FALSE;

	GBBitmap_draw(globe->crossBitmap, globe->pointer[0] - (16 >> 1),
		globe->pointer[1] - (16 >> 1), 256, &globe->displayParams);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

#define sinf(v)			((GBfloat)sin(v))
#define cosf(v)			((GBfloat)cos(v))

GBbool GBGlobe_renderCities (GBGlobe* globe)
{
	GBuint32 i;
	GB_ASSERT(globe);

	for (i = 0; i < globe->cityCount; i++)
	{
		GBCity* city = &globe->cities[i];
		GBfloat pos[3];
		GBfloat xa = -((city->x - globe->coords[0]) * GB_PI) / 180.0f;
		GBfloat ya = (city->y * GB_PI) / 180.0f;
		GBfloat xrot = (-globe->coords[1] * GB_PI) / 180.0f;
		GBfloat t[2];
		const GBfloat zclip = 0.25f;

		pos[0] = sinf(xa) * cosf(ya);
		pos[1] = sinf(ya);
		pos[2] = cosf(xa) * cosf(ya);

		t[0] = cosf(xrot) * pos[1] + sinf(xrot) * pos[2];
		t[1] = cosf(xrot) * pos[2] - sinf(xrot) * pos[1];
		pos[1] = t[0];
		pos[2] = t[1];

		if (pos[2] > zclip)
		{
#ifdef POLYGON_GLOBE
			GBint32 mul = 256;
#else
			GBint32 mul = (GBint32)(((pos[2] - zclip) / (1.0f - zclip)) * 255.0f);
#endif

			pos[2] = pos[2] - (GBfloat)globe->cameraZ / 256.0f;

			city->sx = (GBint32)((pos[0] * (GBfloat)globe->displayParams.height * GB_FOV_CONSTANT) / pos[2] + (GBfloat)globe->displayParams.width / 2.0f);
			city->sy = (GBint32)((pos[1] * (GBfloat)globe->displayParams.height * GB_FOV_CONSTANT) / pos[2] + (GBfloat)globe->displayParams.height / 2.0f);

			GBBitmap_draw(globe->cityDotBitmap, city->sx - (10 >> 1), city->sy - (10 >> 1), mul, &globe->displayParams);
		}
		else
		{
			city->sx = GB_INT32_MAX;
			city->sy = GB_INT32_MAX;
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_renderTexts (GBGlobe* globe)
{
	GBuint32 i;
	GB_ASSERT(globe);

	if (globe->distance > 0.75f)
		return GB_TRUE;

	if (!globe->fontInterface)
		return GB_FALSE;

	gbClearRectList(&globe->rectList);

	for (i = 0; i < globe->cityCount; i++)
	{
		GBbool draw = GB_FALSE;
		GBint32 width;
		GBint32 height;
		GBint32 x1;
		GBint32 y1;
		GBint32 x2;
		GBint32 y2;
		GBCity* city = &globe->cities[i];

		if (city->sx == GB_INT32_MAX && city->sy == GB_INT32_MAX)
			continue;

		width = (GBint32)globe->fontInterface->getWidth(globe->fontContext, city->name);
		height = (GBint32)globe->fontInterface->getHeight(globe->fontContext) - 4;

		x1 = city->sx;
		y1 = city->sy;
		x2 = city->sx + width;
		y2 = city->sy + height;

		if (!gbCheckOverlap(globe->rectList, x1, y1, x2, y2))
			if (gbAddRect(&globe->rectList, x1, y1, x2, y2))
				draw = GB_TRUE;

		if (!draw)
		{
			y1 -= height;
			y2 -= height;

			if (!gbCheckOverlap(globe->rectList, x1, y1, x2, y2))
				if (gbAddRect(&globe->rectList, x1, y1, x2, y2))
					draw = GB_TRUE;
		}

		if (draw)
		{
			GBFontParams params;
			GBTextRect pos;
			pos.x = x1;
			pos.y = y1 - 2;
			pos.left = globe->displayParams.x;
			pos.top = globe->displayParams.y;
			pos.right = globe->displayParams.x + globe->displayParams.width;
			pos.bottom = globe->displayParams.y + globe->displayParams.height;

			GBFontParams_init(&params);
			params.color = 0x00ffffff;
			globe->fontInterface->setParams(globe->fontContext, &params);

			globe->fontInterface->render(globe->fontContext, city->name, &pos);
		}
	}

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_worldToTexCoord (GBGlobe* globe, const GBfloat* coords, GBuint32 width, GBuint32 height, GBuint16* tex)
{
	GB_ASSERT(globe && coords && tex);

	tex[0] = (GBuint16)((GBint32)(coords[0] * width / 360.0f + width / 2.0f) & (width - 1));
	tex[1] = (GBuint16)((GBint32)((90.0f + coords[1]) * height / 180.0f) & (height - 1));
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

const char* GBGlobe_lookupCountry (GBGlobe* globe, const GBvec2 coords)
{
	GBuint16 tex[2];
	GBbool countryFocused = GB_FALSE;
	GB_ASSERT(globe);

	GBGlobe_worldToTexCoord(globe, coords, 1024, 512, tex);

	tex[0] = (tex[0] + 1) & 1023;

	{
		GBuint16 v = gbReadFromRLE(tex[0], tex[1], globe->countryData);
		if (v < globe->countryCount)
		{
			countryFocused = GB_TRUE;
			gbStrCopy(globe->countryStr, globe->countryList[v]);
		}
	}

	if (!countryFocused)
		gbStrCopy(globe->countryStr, "");

	return globe->countryStr;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

const char* GBGlobe_lookupTimezone (GBGlobe* globe, const GBvec2 coords)
{
	GBuint16 tex[2];
	GBbool timezoneFocused = GB_FALSE;
	GB_ASSERT(globe);

	GBGlobe_worldToTexCoord(globe, coords, 1024, 512, tex);

	{
		GBuint16 v = gbReadFromRLE(511 - tex[1], tex[0], globe->timezoneData);
		if (v < globe->timezoneCount)
		{
			timezoneFocused = GB_TRUE;
			gbStrCopy(globe->timezoneStr, globe->timezoneList[v]);
		}
	}

	if (!timezoneFocused)
		gbStrCopy(globe->timezoneStr, "");

	return globe->timezoneStr;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_lookupCity (GBGlobe* globe, const GBvec2 coords, GBCityOut* out)
{
	GBuint32 i;
	GB_ASSERT(globe && coords);

	for (i = 0; i < globe->cityCount; i++)
	{
		GBCity* city = &globe->cities[i];

		if (gbFAbs(coords[0] - city->x) < 1.5f && gbFAbs(coords[1] - city->y) < 1.5f)
		{
			gbStrCopy(out->name, city->name);
			out->coords[0] = city->x;
			out->coords[1] = city->y;
			return GB_TRUE;
		}
	}

	return GB_FALSE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBuint32 GBGlobe_loadStrings (GBGlobe* globe, const char* filename, GBuint32 max, char** out)
{
	FILE* f;
	GBuint32 count = 0;
	GBbool exit = 0;
	GBuint32 fileEnd;
	GB_ASSERT(globe && filename && out);

	f = gbOpenFileFromPath(globe->dataPath, filename, "rb");
	if (!f)
		return 0;

	fseek(f, 0, SEEK_END);
	fileEnd = ftell(f);
	fseek(f, 0, SEEK_SET);

	while (!exit)
	{
		char buf[256];
		GBuint32 i = 0;

		buf[0] = 0;

		for (;;)
		{
			fread(buf + i, 1, 1, f);
			if (buf[i] == '\n' || buf[i] == '\r' || (GBuint32)ftell(f) == fileEnd)
				break;
			i++;
		}

		buf[i] = 0;

		out[count] = gbStrDuplicate(buf);
		if (!out[count])
			exit = 1;
		else
			count++;

		if (count == max || (GBuint32)ftell(f) == fileEnd)
			exit = 1;
	}

	fclose(f);

	return count;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_loadCities (GBGlobe* globe)
{
	FILE* f;
	GBuint32 i;
	GBbool stop = GB_FALSE;
	GB_ASSERT(globe);
	GB_ASSERT(globe->cityCount == 0 && !globe->cities);

	f = gbOpenFileFromPath(globe->dataPath, globe->cityListFileName, "rb");
	if (!f)
		return GB_FALSE;

	while (!stop)
	{
		char buf[256];
		gbSkipChars(f, "\n\r");
		gbReadWord(f, buf, "\n\r");
		if (buf[0] == 0)
			break;
		globe->cityCount++;
	}

	fseek(f, 0, SEEK_SET);

	globe->cities = (GBCity*)GBMemory_alloc(globe->mem, sizeof(GBCity) * globe->cityCount);
	if (!globe->cities)
	{
		fclose(f);
		return GB_FALSE;
	}

	for (i = 0; i < globe->cityCount; i++)
	{
		char buf[256];
		gbSkipChars(f, "\n\r");

		gbReadWord(f, buf, "\t\n\r");
		if (buf[0] == 0)
			GB_ASSERT(0);
		globe->cities[i].y = (GBfloat)atof(buf);

		gbReadWord(f, buf, "\t\n\r");
		if (buf[0] == 0)
			GB_ASSERT(0);
		globe->cities[i].x = (GBfloat)atof(buf);

		gbReadWord(f, buf, "\t\n\r");
		if (buf[0] == 0)
			GB_ASSERT(0);
		globe->cities[i].name = gbStrDuplicate(buf);

		globe->cities[i].sx = 0;
		globe->cities[i].sy = 0;
	}

	fclose(f);

	return GB_TRUE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_worldToScreen (GBGlobe* globe, GBfloat lat, GBfloat lon, GBfloat* out)
{
	GBfloat pos[3];
	GBfloat xa;
	GBfloat ya;
	GBfloat xrot;
	GBfloat t[2];
	const GBfloat zclip = 0.1f;

	GB_ASSERT(globe && out);

	xa = (globe->coords[0] - lat) * GB_PI / 180.0f;
	ya = lon * GB_PI / 180.0f;
	xrot = -globe->coords[1] * GB_PI / 180.0f;

	pos[0] = sinf(xa) * cosf(ya);
	pos[1] = sinf(ya);
	pos[2] = cosf(xa) * cosf(ya);

	t[0] = cosf(xrot) * pos[1] + sinf(xrot) * pos[2];
	t[1] = cosf(xrot) * pos[2] - sinf(xrot) * pos[1];
	pos[1] = t[0];
	pos[2] = t[1];

	if (pos[2] < zclip)
	{
		GBfloat len;
		pos[2] = zclip;
		len = (GBfloat)sqrt(pos[0] * pos[0] + pos[1] * pos[1]);
		pos[0] /= len;
		pos[1] /= len;
	}

	pos[2] = pos[2] - (GBfloat)globe->cameraZ / 256.0f;

	out[0] = (pos[0] * (GBfloat)globe->displayParams.height * GB_FOV_CONSTANT) / pos[2] + (GBfloat)globe->displayParams.width / 2.0f;
	out[1] = (pos[1] * (GBfloat)globe->displayParams.height * GB_FOV_CONSTANT) / pos[2] + (GBfloat)globe->displayParams.height / 2.0f;

/*	if (out[0] >= 0.0f && out[0] < globe->displayParams.width &&
		out[1] >= 0.0f && out[1] < globe->displayParams.height)
	{
		GBuint32 x = (GBuint32)out[0];
		GBuint32 y = (GBuint32)out[1];
		((GBOutputFormat*)globe->displayParams.buffer)[(x + globe->displayParams.x) + (y + globe->displayParams.y) * globe->displayParams.bufferWidth] = 0xffffff;
	}*/
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_screenToNormal (GBGlobe* globe, GBfloat x, GBfloat y, GBfloat* out)
{
	GB_ASSERT(globe && out);

	if (x < 0.0f)
		x = 0.0f;
	else if (x >= (float)globe->displayParams.width)
		x = (float)globe->displayParams.width - 1.0f;

	if (y < 0.0f)
		y = 0.0f;
	else if (y >= (float)globe->displayParams.height)
		y = (float)globe->displayParams.height - 1;

	if (!gbCastRayF(x, y, globe->displayParams.width, globe->displayParams.height, globe->cameraZ, out))
	{
		out[0] = x * 2.0f - (GBfloat)globe->displayParams.width;
		out[1] = y * 2.0f - (GBfloat)globe->displayParams.height;
		out[2] = 1.5f / (((GBfloat)globe->cameraZ) / 256.0f) * ((GBfloat)globe->displayParams.height);
	}

	{
		GBfloat len = (GBfloat)sqrt(out[0] * out[0] + out[1] * out[1] + out[2] * out[2]);
		out[0] /= len;
		out[1] /= len;
		out[2] /= len;
	}

	gbFVec3Transform(out, globe->finalMatrixF, out);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_worldToScreenNormal (GBGlobe* globe, GBfloat lat, GBfloat lon, GBfloat* out)
{
	GBfloat xa;
	GBfloat ya;
	GBfloat xrot;
	GBfloat t[2];
	const GBfloat zclip = 0.1f;

	GB_ASSERT(globe && out);

	xa = (globe->coords[0] - lat) * GB_PI / 180.0f;
	ya = lon * GB_PI / 180.0f;
	xrot = -globe->coords[1] * GB_PI / 180.0f;

	out[0] = sinf(xa) * cosf(ya);
	out[1] = sinf(ya);
	out[2] = cosf(xa) * cosf(ya);

	t[0] = cosf(xrot) * out[1] + sinf(xrot) * out[2];
	t[1] = cosf(xrot) * out[2] - sinf(xrot) * out[1];
	out[1] = t[0];
	out[2] = t[1];

	if (out[2] < zclip)
	{
		GBfloat len;
		out[2] = zclip;
		len = (GBfloat)sqrt(out[0] * out[0] + out[1] * out[1]);
		out[0] /= len;
		out[1] /= len;
	}
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_worldToWorldNormal (GBGlobe* globe, GBfloat lon, GBfloat lat, GBfloat* out)
{
	GBfloat xa;
	GBfloat ya;

//	GBfloat w[2];

	GB_ASSERT(globe && out);

	xa = lon * GB_PI / 180.0f;
	ya = -lat * GB_PI / 180.0f;

	out[0] = sinf(xa) * cosf(ya);
	out[1] = sinf(ya);
	out[2] = cosf(xa) * cosf(ya);

//	printf("normal %f %f %f\n", out[0], out[1], out[2]);

//	GBGlobe_normalToWorld(globe, out, w);

//	printf("world %f %f\n", w[0], w[1]);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_normalToWorld (GBGlobe* globe, const GBfloat* normal, GBfloat* out)
{
	GBfloat v;
	GB_ASSERT(globe && normal && out);

	out[0] = (GBfloat)atan2(normal[0], normal[2]) * 180.0f / GB_PI;
	v = (GBfloat)acos(normal[1]) * 180.0f / GB_PI;
	if (v < 0.0f)
		v = -v;
	out[1] = -(90.0f - v);

	// some rounding
	out[0] = ((GBfloat)((GBint32)(out[0] * 10.0f))) / 10.0f;
	out[1] = ((GBfloat)((GBint32)(out[1] * 10.0f))) / 10.0f;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

void GBGlobe_screenToWorld	(GBGlobe* globe, GBfloat x, GBfloat y, GBfloat* out)
{
	GBfloat n[3];
	GB_ASSERT(globe && out);

	GBGlobe_screenToNormal(globe, x, y, n);
	GBGlobe_normalToWorld(globe, n, out);

//	GBGlobe_worldToWorldNormal(globe, out[0], out[1], n);
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//

GBbool GBGlobe_isPointInside (GBGlobe* globe, GBfloat x, GBfloat y)
{
	GB_ASSERT(globe);

	if (x < 0.0f)
		x = 0.0f;
	else if (x >= (float)globe->displayParams.width)
		x = (float)globe->displayParams.width - 1.0f;

	if (y < 0.0f)
		y = 0.0f;
	else if (y >= (float)globe->displayParams.height)
		y = (float)globe->displayParams.height - 1;

	if (gbCastRayF(x, y, globe->displayParams.width, globe->displayParams.height, globe->cameraZ, NULL))
		return GB_TRUE;
	else
		return GB_FALSE;
}

//--------------------------------------------------------------------------//
//
//--------------------------------------------------------------------------//
