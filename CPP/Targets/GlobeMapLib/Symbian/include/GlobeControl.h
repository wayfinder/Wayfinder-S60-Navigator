/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GLOBECONTROL_H
#define GLOBECONTROL_H

#include "config.h"
#include "GlobeMapMovingInterface.h"
#include "MapDrawingInterface.h"
#include "MC2Coordinate.h"
#include "MC2Point.h"
#include "MapComponentWrapper.h"
#include <eiklabel.h>

#include "ClockInterface.h"
#include "TimeZoneInterface.h"
#include "InfoBoxInterface.h"
#include "MapRectInterface.h"

#include "SymbianBitmapPainter.h"


class CFbsBitmap;
class GlobeClickInfo;
class CPeriodic;
struct GBGlobe_s;
typedef struct GBGlobe_s GBGlobe;
struct GBTopBar_s;
typedef struct GBTopBar_s GBTopBar;
struct GBClock_s;
typedef struct GBClock_s GBClock;
struct GBFont_s;
typedef struct GBFont_s GBFont;
struct GBBitmap_s;
typedef struct GBBitmap_s GBBitmap;
struct GBFontInterface_s;
typedef struct GBFontInterface_s GBFontInterface;
struct GBFontParams_s;
typedef struct GBFontParams_s GBFontParams;
struct GBTextRect_s;
typedef struct GBTextRect_s GBTextRect;

typedef unsigned int GBuint32;

struct GBDisplayParams_s;
typedef struct GBDisplayParams_s GBDisplayParams;

/**
 *    Globe control.
 */
class CGlobeControlX :
	public CCoeControl,
	public MapDrawingInterface,
	public ClockInterface,
	public TimeZoneInterface,
	public InfoBoxInterface,
	public SymbianBitmapPainter,
   public MapRectInterface
{
public:

	/*
	*    Constructor.
	*/
	CGlobeControlX();

	/*
	*    Destructor.
	*/
	~CGlobeControlX();

   /**
    *    Second phase constructor.
    */
   void ConstructL( const CCoeControl& container );

   /**
    *    Second phase constructor.
    *
    *    @param   container         The parent control.
    *    @param   aPath             The path to the resources.
    *    @param   aRect             The rectangle.
    *    @param   aLocationString   The location string e.g. 
    *                               "I'm currently in".
    *    @param   aLanguageCode     The two character language code.
    */
   void ConstructL( const CCoeControl& container, 
                    const char* aPath, 
                    const TRect& aRect,
                    const char* aLocationString,
                    const char* aLanguageCode );

   /**
    *    Requests that the display should be repainted now.
    */
   void repaintNow();

   /**
    *    Requests that the display should be repainted when
    *    convinient.
    */
   void requestRepaint();

   /**
    *    Get the MapMovingInterface.
    */
   MapMovingInterface& getMapMovingInterface() { return m_handler; }
   
   /**
    *    Set the offset to the local time (of the device) in seconds. 
    */
   void setOffsetToLocalTime( int offset ) const;

   /**
    *    Get the offset to the local time (of the device) in seconds 
    *    for the specified coordinate.
    */
   int getOffsetToLocalTime( const MC2Coordinate& coord ) const;

   /**
    *    Get the time as seconds since 00:00:00 1 January 1970 in 
    *    local time.
    */
   unsigned int getLocalTimeNow() const;

   /**
    *    Set the text for the info box in utf8.  
    */
   void setText( const char* utf8Text ) const;

   /**
    *    Get the actual map rectangle, relative to the control.
    *    I.e. the rect for the top bar is not included.
    */
   PixelBox getMapRect() const; 

protected:

   /**
    * Respond to size changed events.
    */
   virtual void SizeChanged();

private:   

   void BaseConstructL();
   void ConstructDrawingDevices();

   /**
    * Draw everything within the specified rectangle.
    */
   virtual void Draw( const TRect& rect ) const;
   
   /**
    *    Prepares what to draw.
    */
   void prepareDraw();
   
   /**
    *    Updates globe's transformation and other variables.
    */
   void updateGlobe();
   
   float getTimeZoneAtCoordinate( float coords[2] );

private:
   /// The globe handler.
   GlobeMapMovingInterface m_handler;
   
   CFbsBitmap* m_bitmap;
   
   GBDisplayParams* m_displayParams;
   GBDisplayParams* m_topDisplayParams;
   
   GBGlobe* m_globe;
   
   //GBBitmap* m_fontBitmap;
   //GBFont* m_font;

   //GBBitmap* m_fontBitmapBlack;
   //GBFont* m_fontBlack;
   
   GBTopBar* m_topbar;
   GBClock* m_clock;
   
   int m_localtime;
   int m_offset;
   
   int m_time;
   
   char* m_globePath;
   char* m_locationString;
   char* m_countryListFileName;
   char* m_cityListFileName;
   
   TInt m_globeWidth;
   TInt m_globeHeight;
   TInt m_topBarHeight;   

   float m_currentLocationCoords[2];
   float m_currentLocationTimezone;
   
   CPeriodic* m_scrollTimer;
   bool m_scrolling;
   bool m_timerRunning;
   bool m_refreshGlobe;
   
   static TInt timerCallBack( TAny* ptr );

   class CFbsBitmapDevice* m_device;
   class CFbsBitGc* m_gc;

   GBFontInterface* m_fontInterface;
   
   struct FontContext
   {
      //GBFont* m_font;
      //GBDisplayParams* m_displayParams;
      CGraphicsContext* m_gc;
      const CFont* m_font;
      bool m_black;
   };
   
   FontContext m_fontContext;
   FontContext m_blackFontContext;
   
   static void			fontSetParamsWrapper	(void* fontContext, const GBFontParams* params);
   
   static GBuint32		fontGetWidthWrapper		(void* fontContext, const char* text);
   
   static GBuint32		fontGetHeightWrapper	(void* fontContext);
   
   static void			fontRenderWrapper		(void* fontContext, const char* text, const GBTextRect* pos);
   
};

/**
 *    Wrapper class for a map component.
 */
class GlobeMapComponentWrapper : public MapComponentWrapper {

   public:
      /**
       *    Constructor.
       */
	  GlobeMapComponentWrapper( const CCoeControl& container);
     
      /**
       *    Constructor with additional parameters.
       * 
       *    @param   container         The parent control.
       *    @param   aPath             The path to the resources.
       *    @param   aRect             The rectangle.
       *    @param   aLocationString   The location string e.g. 
       *                               "I'm currently in".
       *    @param   aLanguageCode     The two character language code.
       */
     GlobeMapComponentWrapper( const CCoeControl& container,
                                const char* aPath, 
                                const TRect& aRect,
                                const char* aLocationString,
                                const char* aLanguageCode );
      
      /**
       *    Destroys the contained objects.
       */
      ~GlobeMapComponentWrapper();

      /**
       *    Get the CCoeControl.
       */
      CCoeControl& getControl() const; 
     
      /**
       *    Get the map rect interface.
       */
      const MapRectInterface& getMapRectInterface() const; 

      /**
       *    Get the MapMovingInterface.
       */
      MapMovingInterface& getMapMovingInterface() const;

      /**
       *    Get the MapDrawingInterface.
       */
      MapDrawingInterface& getMapDrawingInterface() const;
     
      /**
       *    Get the BitmapDrawingInterface.
       */
      SymbianBitmapDrawingInterface& getBitmapDrawingInterface() const;

      /**
       *    Get the minimum scale when this map component should be shown.
       */
      int getMinScale() const;

   private:

      /**
       *    The control.
       */
      CGlobeControlX* m_control;
};

#endif

