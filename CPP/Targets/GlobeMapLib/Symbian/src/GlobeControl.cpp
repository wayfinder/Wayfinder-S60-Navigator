/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "GlobeControl.h"

#include <e32base.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <bitdev.h>
#include <bitstd.h>

#include <eikenv.h>

#ifdef NAV2_CLIENT_SERIES60_V3
#include <aknutils.h>
#endif

#ifdef NAV2_CLIENT_UIQ3
#include <gulfont.h>
#include <tz.h>
#include <tzconverter.h>
#endif

#include "gbDefs.h"
#include "gbGlobe.h"
#include "gbTopBar.h"
#include "gbClock.h"
#include "gbFont.h"
#include "gbFontInterface.h"

#include <math.h>

#include "MapUtility.h"
#include "GlobeMapMovingInterface.h"

//const double GB_MIN_SCALE = 12000;
//#define GB_TOP_LAT 912909609
//#define GB_BOTTOM_LAT -912909609

const int TOP_BAR_HEIGHT = 40;

CGlobeControlX::CGlobeControlX()
{
}

CGlobeControlX::~CGlobeControlX()
{
	m_scrollTimer->Cancel();
	delete m_scrollTimer;

	if (m_clock)
		GBClock_destroy(m_clock);
	
	if (m_topbar)
		GBTopBar_destroy(m_topbar);
	
	if (m_fontInterface)
		delete m_fontInterface;

//	if (m_fontBlack)
//		GBFont_destroy(m_fontBlack);
	
//	if (m_fontBitmapBlack)
//		GBBitmap_destroy(m_fontBitmapBlack, NULL);

//	if (m_font)
//		GBFont_destroy(m_font);

//	if (m_fontBitmap)
//		GBBitmap_destroy(m_fontBitmap, NULL);

	if (m_globe)
		GBGlobe_destroy(m_globe);
	
	if (m_displayParams)
		delete m_displayParams;
	
	if (m_topDisplayParams)
		delete m_topDisplayParams;

   delete m_bitmap;
   delete[] m_globePath;
   delete[] m_locationString;
   delete[] m_countryListFileName;
   delete[] m_cityListFileName;

   delete m_device;
   delete m_gc;
}

#if defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2
  const int GLOBE_WIDTH = 176;
  const int GLOBE_HEIGHT = 208;
#elif defined NAV2_CLIENT_UIQ
  const int GLOBE_WIDTH = 208;
  const int GLOBE_HEIGHT = 216;
#elif defined NAV2_CLIENT_SERIES80
  const int GLOBE_WIDTH = 463;
  const int GLOBE_HEIGHT = 200;
#else
  const int GLOBE_WIDTH = 176;
  const int GLOBE_HEIGHT = 128 + 16;
#endif

const char* const GLOBE_DATA_PATH = "c:/system/data/wayfinder6/globe/";

const float GB_LOCAL_OFFSET = 2.0f;

void
CGlobeControlX::ConstructL( const CCoeControl& container )
{
   SetContainerWindowL( container );
   SetExtent( TPoint( 0, 0 ), TSize( GLOBE_WIDTH, GLOBE_HEIGHT )  );
   TRect rect = Rect();

   m_globeWidth = rect.Width();
   m_globeHeight = rect.Height();

   m_globePath = strdup(GLOBE_DATA_PATH);
   m_locationString = NULL;
   m_countryListFileName = NULL; 

   BaseConstructL();

   ActivateL();
}

void
CGlobeControlX::ConstructL( const CCoeControl& container, 
                            const char* aPath, 
                            const TRect& aRect,
                            const char* aLocationString,
                            const char* aLanguageCode )
{
   SetContainerWindowL( container );
   SetRect( aRect );

   m_globeWidth = aRect.Width();
   m_globeHeight = aRect.Height();

   m_globePath = new char[256];
   sprintf( m_globePath, "%sglobe\\", aPath );
   
   m_locationString = new char[256];
   sprintf( m_locationString, "%s", aLocationString );

   m_countryListFileName = new char[256];
   sprintf( m_countryListFileName, "country_%s.txt", aLanguageCode );

   m_cityListFileName = new char[256];
   sprintf( m_cityListFileName, "cities_%s.txt", aLanguageCode );
   
   BaseConstructL();

   ActivateL();
}

TInt CGlobeControlX::timerCallBack( TAny* ptr )
{
	CGlobeControlX* ctr = (CGlobeControlX*)ptr;

	if (!ctr->IsVisible())
	{
		ctr->m_scrollTimer->Cancel();
		ctr->m_timerRunning = false;
		ctr->m_refreshGlobe = true;
	}
	else
	{
		ctr->repaintNow();
	}

	return 1;
}

void
CGlobeControlX::BaseConstructL()
{
   ConstructDrawingDevices();

   m_displayParams = new(ELeave) GBDisplayParams();
   m_topDisplayParams = new(ELeave) GBDisplayParams();
   
   //ScanLineLength returns the number of bytes in a scanline.
   TInt scanLineLength = m_bitmap->ScanLineLength( m_globeWidth, EColor64K );
   //Divide by 2 since EColor4K uses 12 bits which requires 2 bytes per pixel.
   //Divide by 2 since EColor64K uses 12 bits which requires 2 bytes per pixel.
   scanLineLength = scanLineLength / 2; 

#ifdef NAV2_CLIENT_SERIES60_V3
   TRect rect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, rect);
   m_topBarHeight = TInt(float(rect.Height()) * 
                         (float(TOP_BAR_HEIGHT) / 208));
#else
   m_topBarHeight = TOP_BAR_HEIGHT;
#endif

   GBDisplayParams_init( m_displayParams );
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->LockHeap();
#endif
   m_displayParams->buffer = m_bitmap->DataAddress();
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->UnlockHeap();
#endif
   m_displayParams->bufferWidth = scanLineLength; //m_globeWidth;
   m_displayParams->y = m_topBarHeight;
   m_displayParams->width = m_globeWidth;
   m_displayParams->height = m_globeHeight - m_topBarHeight;
  
   m_handler.setTopLeftGlobePos( MC2Point( 0, m_topBarHeight ) );
   
   GBDisplayParams_init( m_topDisplayParams );
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->LockHeap();
#endif
   m_topDisplayParams->buffer = m_bitmap->DataAddress();
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->UnlockHeap();
#endif
   m_topDisplayParams->bufferWidth = scanLineLength; //m_globeWidth;
   m_topDisplayParams->width = m_globeWidth;
   m_topDisplayParams->height = m_topBarHeight;
   
//   m_globe = GBGlobe_create(m_globePath);
   m_globe = GBGlobe_create(m_globePath, m_countryListFileName, 
                            m_cityListFileName );
   if (m_globe) {
      GBGlobe_setDisplay( m_globe, m_displayParams );
      m_handler.setMoved(true);
   }

/*   m_fontBitmap = GBBitmap_createFromTga( m_globePath, "font.tga", NULL );
   if (m_fontBitmap)
      m_font = GBFont_create( m_fontBitmap );

   m_fontBitmapBlack = GBBitmap_createFromTga( m_globePath, "font_black.tga", NULL );
   if (m_fontBitmapBlack)
      m_fontBlack = GBFont_create( m_fontBitmapBlack );
*/   
   m_fontInterface = new GBFontInterface();
   if (m_fontInterface)
   {
      GBFontInterface_init(m_fontInterface);
      m_fontInterface->setParams = fontSetParamsWrapper;
      m_fontInterface->getWidth = fontGetWidthWrapper;
      m_fontInterface->getHeight = fontGetHeightWrapper;
	  m_fontInterface->render = fontRenderWrapper;
   }

//   m_fontContext.m_font = m_font;
//   m_fontContext.m_displayParams = m_displayParams;
   m_fontContext.m_gc = m_gc;
#ifdef NAV2_CLIENT_UIQ3
   m_fontContext.m_font = CEikonEnv::Static()->AnnotationFont();
#else
   m_fontContext.m_font = CEikonEnv::Static()->LegendFont();
#endif
   m_fontContext.m_black = false;

   if (m_fontInterface && m_globe)
      GBGlobe_setFont( m_globe, m_fontInterface, &m_fontContext );
   
   m_topbar = GBTopBar_create( m_globePath, m_locationString );

   m_blackFontContext.m_gc = m_gc;
#ifdef NAV2_CLIENT_UIQ3
   TLogicalFont logicalFont( TLogicalFont::EAnnotation, TLogicalFont::ELight, TZoomFactor() );
   m_blackFontContext.m_font = CEikonEnv::Static()->Font( logicalFont );
#else
   m_blackFontContext.m_font = CEikonEnv::Static()->LegendFont();
#endif
   m_blackFontContext.m_black = true;

   if (m_topbar && m_fontInterface)
      GBTopBar_setFont( m_topbar, m_fontInterface, &m_blackFontContext );

   m_clock = GBClock_create( m_globePath );

   TInt m_clockSize = 36;
   int clockBorder = (m_topBarHeight - m_clockSize) / 2;
   GBClock_setSize(m_clock, m_clockSize);
   GBClock_setCorner(m_clock, 6, clockBorder);
#ifdef NAV2_CLIENT_UIQ3
   GBTopBar_setLayout(m_topbar, m_clockSize + 6, 0, -3, 0, -2);
#else
   GBTopBar_setLayout(m_topbar, m_clockSize + 6, 0, 1, 0, -2);
#endif

   m_handler.setGlobe( m_globe );

	// Malm�'s coordinates
//	m_currentLocationCoords[0] = 13.00f; // longitude
//	m_currentLocationCoords[1] = 55.36f; // latitude
//	m_currentLocationTimezone = getTimeZoneAtCoordinate( m_currentLocationCoords );

	float offset;

//#ifdef SYMBIAN_9
#ifdef NAV2_CLIENT_UIQ3
	TTimeIntervalSeconds timezoneOffset( User::UTCOffset() );
	offset = (float)timezoneOffset.Int();
	RTz timezoneServer;
	if( timezoneServer.Connect() == KErrNone ){
   	CTzConverter* timezoneConverter = CTzConverter::NewL( timezoneServer );
   	CleanupStack::PushL( timezoneConverter );
   	CTzId* timezoneId = CTzId::NewL( timezoneConverter->CurrentTzId() );
   	CleanupStack::PushL( timezoneId );
      if( timezoneServer.IsDaylightSavingOnL( *timezoneId ) ){
         offset -= 3600.0f;
      }
   	CleanupStack::PopAndDestroy( timezoneId );
   	CleanupStack::PopAndDestroy( timezoneConverter );
	}
#else
	TLocale locale;
	TTimeIntervalSeconds timezoneOffset( locale.UniversalTimeOffset() );
	offset = (float)timezoneOffset.Int();
#endif

	m_currentLocationTimezone = offset / 3600.0f;

	m_scrollTimer = CPeriodic::NewL( CTimer::EPriorityLow );
	m_timerRunning = false;
	m_refreshGlobe = true;
}

void
CGlobeControlX::ConstructDrawingDevices()
{
   delete m_bitmap;
   m_bitmap = NULL;
   delete m_device;
   m_device = NULL;
   delete m_gc;
   m_gc = NULL;

   //Create the bitmap
   m_bitmap = new(ELeave) CFbsBitmap();
   m_bitmap->Create(TSize(m_globeWidth, m_globeHeight), EColor64K);
   //Create the bitmap device
   m_device = CFbsBitmapDevice::NewL(m_bitmap);
   //Create the graphics context for the device
   m_device->CreateContext(m_gc);

   m_fontContext.m_gc = m_gc;
   m_blackFontContext.m_gc = m_gc; 

   //Clear bitmap
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->LockHeap();
#endif
   Mem::FillZ(m_bitmap->DataAddress(), m_globeWidth * m_globeHeight * 2);
#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->UnlockHeap();
#endif
}

void
CGlobeControlX::SizeChanged()
{
   if (m_globe) {
      if ((m_globeWidth == Rect().Width()) && 
          (m_globeHeight == Rect().Height())) {
         return;
      }
      m_globeWidth = Rect().Width();
      m_globeHeight = Rect().Height();

	  ConstructDrawingDevices();

      TInt scanLineLength = m_bitmap->ScanLineLength(m_globeWidth, 
                                                     EColor64K);
      scanLineLength = scanLineLength / 2; 

#ifdef NAV2_CLIENT_SERIES60_V3
      TRect rect;
      AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EApplicationWindow, 
                                        rect);
      m_topBarHeight = TInt(float(rect.Height()) * 
                            (float(TOP_BAR_HEIGHT) / 208));
#else
      m_topBarHeight = TOP_BAR_HEIGHT;
#endif

      GBDisplayParams_init(m_displayParams);

#ifndef NAV2_CLIENT_SERIES60_V1
      m_bitmap->LockHeap();
#endif
      m_displayParams->buffer = m_bitmap->DataAddress();
#ifndef NAV2_CLIENT_SERIES60_V1
      m_bitmap->UnlockHeap();
#endif
      m_displayParams->bufferWidth = scanLineLength; //m_globeWidth;
      m_displayParams->y = m_topBarHeight ;
      m_displayParams->width = m_globeWidth;
      m_displayParams->height = m_globeHeight - m_topBarHeight;

      GBDisplayParams_init( m_topDisplayParams );

#ifndef NAV2_CLIENT_SERIES60_V1
      m_bitmap->LockHeap();
#endif
      m_topDisplayParams->buffer = m_bitmap->DataAddress();
#ifndef NAV2_CLIENT_SERIES60_V1
      m_bitmap->UnlockHeap();
#endif
      m_topDisplayParams->bufferWidth = scanLineLength; //m_globeWidth;
      m_topDisplayParams->width = m_globeWidth;
      m_topDisplayParams->height = m_topBarHeight;

      TInt m_clockSize = 36;
#ifdef NAV2_CLIENT_UIQ3
      GBTopBar_setLayout(m_topbar, m_clockSize + 6, 0, -3, 0, -2);
#else
      GBTopBar_setLayout(m_topbar, m_clockSize + 6, 0, 1, 0, -2);
#endif

      GBGlobe_setDisplay(m_globe, m_displayParams);
      m_handler.setMoved(true);

      requestRepaint();
   }
}
/*
void CGlobeControlX::MakeVisible( TBool aVisible )
{
	m_scrollTimer->Cancel();
	if (aVisible)
	{
		m_scrollTimer->Start( 1000 * 4000, 1000 * 4000, TCallBack( timerCallBack, this ) );
		m_scrolling = false;
	}
}
*/
/*TKeyResponse
CGlobeControlX::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
{
	switch (aKeyEvent.iScanCode)
	{
	case '1':
		if (aType == EEventKeyDown)
			m_zoomTweak += 0.005;
		return EKeyWasConsumed;
	case '3':
		if (aType == EEventKeyDown)
			m_zoomTweak -= 0.005;
		return EKeyWasConsumed;
	default:
		return EKeyWasNotConsumed;
	}
}*/

float CGlobeControlX::getTimeZoneAtCoordinate( float coords[2] )
{
   if (!m_globe)
      return 0.0f;

	const char* timezone = GBGlobe_lookupTimezone( m_globe, coords );
	return gbStringToFloat( timezone );
}

void
CGlobeControlX::updateGlobe()
{
//	test: setWorldBox( Madrid, Helsinki )
//	m_handler.setWorldBox( MC2Coordinate( WGS84Coordinate( 40.26, 3.42 ) ), MC2Coordinate( WGS84Coordinate( 64.0, 30.0 ) ) );

//	m_handler.setCenter(MC2Coordinate( WGS84Coordinate( 48.51, 2.2 ) ));

   GBvec2 coords;
   coords[0] = (float)WGS84Coordinate( m_handler.getCenter() ).lonDeg;
   coords[1] = (float)WGS84Coordinate( m_handler.getCenter() ).latDeg;

   if (coords[1] < -89.0f)
      coords[1] = -89.0f;
   else if (coords[1] > 89.0f)
      coords[1] = 89.0f;

   GBfloat distance = (float)(m_handler.getScale() - (float)GB_MIN_SCALE) / (float) GB_MIN_SCALE / 6.0f - m_handler.m_zoomTweak;

   GBivec2 pointer;
   pointer[0] = m_displayParams->width / 2; 
   pointer[1] = m_displayParams->height / 2;

	if (m_globe)
	{
		GBGlobe_update( m_globe, coords, distance, pointer );

		char infotext[512];
		GBbool cityFound = GB_FALSE;
		const char* country = NULL;
		GBCityOut city;

		infotext[0] = 0;

		cityFound = GBGlobe_lookupCity(m_globe, coords, &city);

		if (cityFound)
			country = GBGlobe_lookupCountry(m_globe, city.coords);

		if (!country || country[0] == 0)
			country = GBGlobe_lookupCountry(m_globe, coords);

		if (cityFound && country[0] != 0)
			sprintf(infotext, "%s, %s", city.name, country);
		else if (cityFound)
			sprintf(infotext, "%s", city.name);
//			gbStrCopy(infotext, city.name);
		else if (country[0] != 0)
			sprintf(infotext, "%s", country);
//			gbStrCopy(infotext, country);

//		sprintf(infotext, "%f %f", m_handler.y, m_handler.x);

		setText(infotext);

/*		const char* country = GBGlobe_lookupCountry(m_globe, coords);
		if (country)
			setText(country);
*/
		setOffsetToLocalTime( (int)((getTimeZoneAtCoordinate( coords ) - m_currentLocationTimezone) * 3600.0f) );
	}

	if (!m_timerRunning)
	{
		m_refreshGlobe = true;
	}

	if (m_topbar && GBTopBar_isScrolling( m_topbar, m_topDisplayParams ))
	{
		if (!m_scrolling || !m_timerRunning)
		{
			m_timerRunning = true;
			m_scrolling = true;
			m_scrollTimer->Cancel();
			m_scrollTimer->Start( 1000 * 100, 1000 * 100, TCallBack( timerCallBack, this ) );
			// 1000 * 100 = 10 fps
		}
	}
	else if (m_scrolling || !m_timerRunning)
	{
		m_timerRunning = true;
		m_scrolling = false;
		m_scrollTimer->Cancel();
		m_scrollTimer->Start( 1000 * 4000, 1000 * 4000, TCallBack( timerCallBack, this ) );
	}

	if (m_clock)
	{
	   time_t rawtime;
		struct tm* timeinfo;
		time( &rawtime );
		timeinfo = localtime( &rawtime );

      m_localtime = timeinfo->tm_hour * 60 * 60 + timeinfo->tm_min * 60 + timeinfo->tm_sec;

		GBClock_setTime( m_clock, m_localtime );
	}
}

void
CGlobeControlX::prepareDraw()
{
   if (m_globe && (m_handler.isMoved() || m_refreshGlobe))
   {
	   m_refreshGlobe = false;
       m_handler.clearMoved();
#ifndef NAV2_CLIENT_SERIES60_V1
	   m_bitmap->LockHeap();
#endif
	   m_displayParams->buffer = m_bitmap->DataAddress();
	   GBGlobe_setDisplay( m_globe, m_displayParams );
	   GBGlobe_render( m_globe );
#ifndef NAV2_CLIENT_SERIES60_V1
	   m_bitmap->UnlockHeap();
#endif
   }

#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->LockHeap();
#endif
   m_topDisplayParams->buffer = m_bitmap->DataAddress();

   TTime time;
   time.HomeTime();
   TInt64 time64( time.Int64() / 1000 );

   if (m_topbar){
#ifdef SYMBIAN_9
      GBTopBar_draw( m_topbar,  I64LOW(time64), m_topBarHeight, m_topDisplayParams );
#else
      GBTopBar_draw( m_topbar,  time64.Low(), m_topBarHeight, m_topDisplayParams );
#endif
   }

   if (m_clock)
		GBClock_draw( m_clock, m_topDisplayParams );

#ifndef NAV2_CLIENT_SERIES60_V1
   m_bitmap->UnlockHeap();
#endif
}

void 
CGlobeControlX::repaintNow()
{
	updateGlobe();
	DrawNow();
}

void 
CGlobeControlX::requestRepaint()
{
	updateGlobe();
	DrawDeferred();
}

void 
CGlobeControlX::Draw( const TRect& rect ) const
{
   ((CGlobeControlX*)this)->prepareDraw();

   if (m_globe)
      GBGlobe_renderTexts( m_globe );
   if (m_topbar)
      GBTopBar_drawTexts( m_topbar, m_topDisplayParams );
   DrawBitmaps( *m_gc );

   SystemGc().BitBlt( TPoint( 0, 0 ), m_bitmap );

   // Use this for some draw debugging.
   //SystemGc().SetPenColor(TRgb(255, 0, 0));
   //SystemGc().SetPenSize(TSize(2,2));
   //SystemGc().SetPenStyle(CGraphicsContext::ESolidPen);
   //SystemGc().DrawRect(rect);

}

/**
*    Set the offset to the local time (of the device) in seconds. 
*/

void CGlobeControlX::setOffsetToLocalTime( int offset ) const
{
   if (m_clock)
      GBClock_setOffset( m_clock, offset );
}

/**
*    Get the offset to the local time (of the device) in seconds 
*    for the specified coordinate.
*/

int CGlobeControlX::getOffsetToLocalTime( const MC2Coordinate& coord ) const
{
	if (!m_globe)
		return 0;

   GBvec2 coords;
   coords[0] = (float)WGS84Coordinate( m_handler.getCenter() ).lonDeg;
   coords[1] = (float)WGS84Coordinate( m_handler.getCenter() ).latDeg;
   
   const char* timezone = GBGlobe_lookupTimezone(m_globe, coords);
   if (!timezone)
      return 0;

   return (int)((atof(timezone) - GB_LOCAL_OFFSET) * 60.0f * 60.0f);
}

/**
*    Get the time as seconds since 00:00:00 1 January 1970 in 
*    local time.
*/

unsigned int CGlobeControlX::getLocalTimeNow() const
{
	return m_localtime;
}

/**
*    Set the text for the info box in utf8.  
*/

void CGlobeControlX::setText( const char* utf8Text ) const
{
	if (m_topbar)
		GBTopBar_setText( m_topbar, utf8Text );
}

PixelBox CGlobeControlX::getMapRect() const 
{
   TRect rect = Rect();
   return PixelBox( MC2Point( rect.iTl.iX, rect.iTl.iY + m_topBarHeight), 
         MC2Point( rect.iBr.iX, rect.iBr.iY ) );
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/

void CGlobeControlX::fontSetParamsWrapper (void* fontContext, const GBFontParams* params)
{
	GB_ASSERT(params);
	/* Not used at the moment */
}

/*--------------------------------------------------------------------------*/
/* Return the width of the text string in pixels.                           */
/*--------------------------------------------------------------------------*/

GBuint32 CGlobeControlX::fontGetWidthWrapper (void* fontContext, const char* text)
{
	GB_ASSERT(fontContext && text);
//	return GBFont_getTextWidth(((FontContext*)fontContext)->m_font, text);

	FontContext* ctx = (FontContext*)fontContext;

   TBuf<256> tt;
   const int length = strlen( text );
   MapUtility::ConvertUTF8ToUnicode( (const unsigned char*) text,
                                     tt, length );

//	TBuf8<256> t((TUint8*)text);
//	TBuf16<256> tt;
//	tt.Copy(t);

	return ctx->m_font->TextWidthInPixels(tt);
}

/*--------------------------------------------------------------------------*/
/* Return the maximum character height in pixels.                           */
/*--------------------------------------------------------------------------*/

GBuint32 CGlobeControlX::fontGetHeightWrapper (void* fontContext)
{
	GB_ASSERT(fontContext);
//	return GBFont_getHeight(((FontContext*)fontContext)->m_font);
	FontContext* ctx = (FontContext*)fontContext;
	return ctx->m_font->HeightInPixels();
}

/*--------------------------------------------------------------------------*/
/* Draw text to screen                                                      */
/*--------------------------------------------------------------------------*/

void CGlobeControlX::fontRenderWrapper (void* fontContext, const char* text, const GBTextRect* pos)
{
//	GBDisplayParams displayParams;
	GB_ASSERT(fontContext && text && pos);

	// GBTextRect structure contains the clipping rectangle and text's upper-left corner coordinate

/*	GBDisplayParams_init(&displayParams);
	displayParams.buffer = ((FontContext*)fontContext)->m_displayParams->buffer;
	displayParams.bufferWidth = ((FontContext*)fontContext)->m_displayParams->bufferWidth;
	displayParams.x = pos->left;
	displayParams.y = pos->top;
	displayParams.width = pos->right - pos->left;
	displayParams.height = pos->bottom - pos->top;

	GBFont_drawText(((FontContext*)fontContext)->m_font, text, pos->x, pos->y, &displayParams);
*/

	FontContext* ctx = (FontContext*)fontContext;
	CGraphicsContext* gc = ctx->m_gc;

	gc->UseFont(ctx->m_font);

	TRect box( pos->x + pos->left, pos->y + pos->top, pos->right, pos->bottom );
	TInt baseline = ctx->m_font->AscentInPixels() + 1;

	TRect clip( pos->left, pos->top, pos->right, pos->bottom );
	gc->SetClippingRect( clip );

	TInt margin = 1;

	gc->SetBrushStyle( CGraphicsContext::ENullBrush );

	if (ctx->m_black)
		gc->SetPenColor( KRgbBlack );
	else
		gc->SetPenColor( KRgbWhite );

   // Convert the text from utf8 to unicode.
   TBuf<256> tt;
   const int length = strlen( text );
   MapUtility::ConvertUTF8ToUnicode( (const unsigned char*) text,
                                     tt, length );

   
//	TBuf8<256> t((TUint8*)text);
//	TBuf16<256> tt;
//	tt.Copy(t);

	gc->DrawText(tt,
	box,
	baseline,
	CGraphicsContext::ELeft,
	margin);

	gc->CancelClippingRect();
}

// --- GlobeMapComponentWrapper

GlobeMapComponentWrapper::GlobeMapComponentWrapper( 
                                 const CCoeControl& container ) 
{
   m_control = new CGlobeControlX();
   m_control->ConstructL( container );
}

GlobeMapComponentWrapper::GlobeMapComponentWrapper( const CCoeControl& container,
                                                    const char* aPath, 
                                                    const TRect& aRect,
                                const char* aLocationString,
                                const char* aLanguageCode )
{
   m_control = new CGlobeControlX();
   m_control->ConstructL( container, aPath, aRect, 
                          aLocationString, aLanguageCode );
}

GlobeMapComponentWrapper::~GlobeMapComponentWrapper() 
{
   delete m_control;
}

CCoeControl& GlobeMapComponentWrapper::getControl() const 
{
   return *m_control;
}

const MapRectInterface& 
GlobeMapComponentWrapper::getMapRectInterface() const
{
   return *m_control;
}

MapMovingInterface& GlobeMapComponentWrapper::getMapMovingInterface() const
{
   return m_control->getMapMovingInterface();
}

MapDrawingInterface& GlobeMapComponentWrapper::getMapDrawingInterface() const
{
   return *m_control;
}

SymbianBitmapDrawingInterface& 
GlobeMapComponentWrapper::getBitmapDrawingInterface() const
{
   return *m_control;
}

int GlobeMapComponentWrapper::getMinScale() const
{
   return (int) GB_MIN_SCALE;
}

// --- GlobeCreator

MapComponentWrapper*
GlobeCreator::createGlobe( const CCoeControl& container )
{
   return new GlobeMapComponentWrapper( container );
}

MapComponentWrapper*
GlobeCreator::createGlobe( const CCoeControl& container, 
                           const char* aPath, 
                           const TRect& aRect,
                           const char* aLocationString,
                           const char* aLanguageCode )
{
   return new GlobeMapComponentWrapper( container, aPath, aRect,
                                        aLocationString,
                                        aLanguageCode );
}
