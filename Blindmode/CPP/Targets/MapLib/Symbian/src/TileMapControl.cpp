/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "TileMapControl.h"
#include "TileMapHandler.h"
#include "MapRenderer.h"
#include "SymbianTilemapToolkit.h"
#include "SharedHttpDBufRequester.h"
#include "HttpClientConnection.h"
#include "SymbianTCPConnectionHandler.h"
#include "SharedDBufRequester.h"
#include "Surface.h"
#include <eikenv.h>
#include <bautils.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikappui.h>
#include <coemain.h>

#include "SymbianMapLib.h"
#include "TileMapKeyHandler.h"

//#include <stdlib.h>

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

using namespace std;
using namespace isab;

namespace {
   class MyMapLib : public SymbianMapLib {
   public:
      MyMapLib( TileMapHandler* tileMapHandler,
                RFs& fileServer ) :
            SymbianMapLib( tileMapHandler, fileServer ) {
      }   
   };
}

/* error messages */
_LIT(KMsgGenericMsgTitle, "TMap : ");
_LIT(KMsgGenericErrorTitle, "TMap : Error :");

/* macros to display above messages */
#define SHOWMSGWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericMsgTitle, x)
#define SHOWMSG(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 1000000)
#define SHOWERRWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericErrorTitle, x)
#define SHOWERR(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 2000000)
#define SHOWBUSY(x)     CEikonEnv::Static()->BusyMsgL(x)
#define CANCELBUSY()    CEikonEnv::Static()->BusyMsgCancel()


#define INFO_FONT        _L("LatinPlain12")
#define INFO_FONT_SIZE   12
#define BUTTON_FONT      _L("LatinPlain12")
#define BUTTON_FONT_SIZE 12
// The following is approx the fonts used in the Nokia 60 menues.
//  #define BUTTON_FONT      _L("Alb17b")
//  #define BUTTON_FONT_SIZE 17

/* private constructor */
CTileMapControl::CTileMapControl() 
   : iToolkit(NULL),
     iPlotter(NULL),
     iRequester(NULL),
     iMapHandler(NULL),
     iNumDraws(0),
     iDrawCrosshair(false),
     iDrawFeatureName(false),
     m_hiddenCursorPoint( 0, 0 )
{
   m_infoDisplayLeftButton  = NULL;
   m_infoDisplayRightButton = NULL;
   m_keyHandlerToDelete = NULL;
}

void
CTileMapControl::deleteInfoStrings()
{
   int size = m_infoStrings.size();
   for( int i = size - 1; i >=0; --i ) {
      iPlotter->deleteString( m_infoStrings[i] );
   }
   m_infoStrings.clear();
   // Also delete the button texts
   iPlotter->deleteString( m_infoDisplayLeftButton );
   m_infoDisplayLeftButton = NULL;
   iPlotter->deleteString( m_infoDisplayRightButton );
   m_infoDisplayRightButton = NULL;
}

/* destructor */
CTileMapControl::~CTileMapControl()
{
   deleteInfoStrings();
   //delete iMapHandler; // Deleted by maplib now, I hope.
   delete m_keyHandlerToDelete; // Must be deleted before TMH.
   delete m_mapLib;
   delete iPlotter;
   delete iToolkit;
}

void
CTileMapControl::receiveExtraMaps( const uint8* serverMessage,
                                   int length )
{
   if ( serverMessage != NULL ) {         
      DBufRequester tmp;
      // Tell the maplib that the map comes from an external source.
      tmp.setType( DBufRequester::EXTERNAL );
      SharedDBufRequester::
         sendToListener( serverMessage,
                         length,
                         iMapHandler->getDBufRequestListener(),
                         tmp );
   }
}

/* allocation methods */
CTileMapControl* CTileMapControl::NewLC( DBufRequester* aRequester,
                                         const TRect& aRect,
                                         TileMapEventListener* eventListener )
{

   CTileMapControl* self = new (ELeave) CTileMapControl();
   CleanupStack::PushL(self);
   self->ConstructL(aRequester, aRect, eventListener );
   return(self);
}

CTileMapControl* CTileMapControl::NewLC( DBufConnection* aRequester,
                                         const TRect& aRect,
                                         TileMapEventListener* eventListener )
{

   CTileMapControl* self = new (ELeave) CTileMapControl();
   CleanupStack::PushL(self);
   self->ConstructL(aRequester, aRect, eventListener );
   return(self);
}

CTileMapControl* CTileMapControl::NewL( DBufRequester* aRequester,
                                        const TRect& aRect,
                                        TileMapEventListener* eventListener )
{

   CTileMapControl* self = CTileMapControl::NewLC(aRequester, aRect,
                                                  eventListener );
   CleanupStack::Pop(self);
   return(self);
}

CTileMapControl* CTileMapControl::NewL( DBufConnection* aRequester,
                                        const TRect& aRect,
                                        TileMapEventListener* eventListener )
{

   CTileMapControl* self = CTileMapControl::NewLC(aRequester, aRect,
                                                  eventListener );
   CleanupStack::Pop(self);
   return(self);
}


void CTileMapControl::ConstructL( DBufConnection* connection,
                                  const TRect& aRect,
                                  TileMapEventListener* listener )
{
   SharedDBufRequester* dbufReq = new SharedDBufRequester( connection );
   ConstructL( dbufReq,
               aRect, listener );
   dbufReq->setToolkit( iToolkit );
   
}

/* second-phase constructor */
void CTileMapControl::ConstructL( DBufRequester* aRequester,
                                  const TRect aRect,
                                  TileMapEventListener* eventListener ) {

   /* get the devices color deopth */
   int colorDepth;
   TDisplayMode nativeColorDepth = 
      CCoeEnv::Static()->ScreenDevice()->DisplayMode();
   switch(nativeColorDepth)
   {
      case(EColor16): colorDepth = 4; break;
      case(EColor256): colorDepth = 8; break;
      case(EColor4K): colorDepth = 12; break;
      case(EColor64K): colorDepth = 16; break;
      default: colorDepth = 12; break;
   }

   iToolkit = new SymbianTileMapToolkit(*this);
   User::LeaveIfNull(iToolkit);
   
   iRequester = aRequester;

   iPlotter = MapRenderer::NewL(*this, 
                                aRect.Width(), aRect.Height(), 
                                colorDepth);

   // Create the TileMapHandler
   // When the TileMapHandler is deleted, it also deletes all the requesters.
   iMapHandler = new TileMapHandler(iPlotter, iRequester,
                                    iToolkit,
                                    eventListener );
   User::LeaveIfNull(iMapHandler);

   // Create the MapLib interface
   m_mapLib = new MyMapLib( iMapHandler,
                            CCoeEnv::Static()->FsSession() );

   /* set the size and position of the control */
   TRect controlRect(TPoint(0,0), TSize(aRect.Width(),aRect.Height()));
   SetRect(controlRect);

   return;
}

/* draws the map ... from CCoeControl */
void CTileMapControl::Draw(const TRect& aRect) const {
#ifdef USE_TRACE
   TRACE_FUNC();
#endif

   CWindowGc& gc = SystemGc();

   /* increment the draw counter */
   iNumDraws++;
#if 0
   TBuf<32> str;
   _LIT(KFMT, "Draw Count : %u");
   str.Format(KFMT, iNumDraws);
   iPlotter->setFont(_L("LatinPlain12"), 12);
   iPlotter->setPenColor(220,8,16);
   iPlotter->drawText(str, MC2Point(5, 15));
#endif

   MC2Point center( Size().iWidth >> 1,
                    Size().iHeight >> 1 );
   
   if( (iDrawCrosshair) && (iNumDraws > 5) ) { // otherwise it gets drawn on help page too
      /* draw the cross-hair */
      const int32 x = center.getX();
      const int32 y = center.getY();
      iPlotter->setLineWidth(2);
      iPlotter->setPenColor(64,64,255);
      iPlotter->drawLine(x, y-5, x, y+5);
      iPlotter->drawLine(x-5, y, x+5, y);
   }

   /* Draw the feature name if required */
   if(iDrawFeatureName) {
      // Use the center point.
      drawInfoText();
   }

   drawUserDefinedBitmap();
//   iPlotter->getBuffer()->Gc().BitBltMasked(iZoomOutPoint,iZoomOutBitmap,TRect(0,0,24,24),iZoomOutBitmapMask,EFalse);
//   iPlotter->getBuffer()->Gc().BitBltMasked(iZoomInPoint,iZoomInBitmap,TRect(0,0,24,24),iZoomInBitmapMask,EFalse);

   // Move temporary rectangle to top-left corner.
   TRect tmprect( aRect );
   tmprect.Move( MC2Point(0,0)-MC2Point(iPosition) );
   iPlotter->RenderTo(gc, MC2Point(aRect.iTl), tmprect );

#if 0
   gc.SetPenSize( TSize(5,5) );
   // Also draw the rect.
   gc.SetPenColor( TRgb( int(rand() / float(RAND_MAX) * 255.0),
                         int(rand() / float(RAND_MAX) * 255.0),
                         int(rand() / float(RAND_MAX) * 255.0) ) );
   gc.DrawRect( aRect );
   gc.SetPenSize( TSize(1,1) );
#endif   
}

inline PixelBox
CTileMapControl::getButtonPosition( bool left, STRING* text ) const
{
   if ( text != NULL ) {
      // Ok - do the work
      PixelBox buttonBox = iPlotter->getStringAsRectangle(*text,
                                                          MC2Point(100,100));
      int xMovement = 1;
      int bottomY = Size().iHeight - 1;
      if ( left ) {
         return buttonBox.moveBottomLeftTo( MC2Point( xMovement, bottomY ) );
      } else {
         return buttonBox.moveBottomRightTo(
            MC2Point( Size().iWidth- xMovement, bottomY ) );
      }
   }
   return PixelBox();
}

void
CTileMapControl::drawInfoText() const
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   iPlotter->setPenColor(0,0,0);
   iPlotter->setLineWidth(1);
   // Set the correct font for the size calculations
   iPlotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
   
   PixelBox leftButtonBox(
      getButtonPosition( true, m_infoDisplayLeftButton) );
   
   PixelBox rightButtonBox(
      getButtonPosition( false, m_infoDisplayRightButton ) );

   // Make a thin box covering the width of the screen
   static const int spaceFromBottom = 3;
   PixelBox allStringsBox( MC2Point( 0, Size().iHeight - spaceFromBottom ),
                           MC2Point( Size().iWidth, Size().iHeight )  );

   if ( m_infoDisplayLeftButton ) {
      allStringsBox.update( leftButtonBox );
      // Add distance from bottom
      allStringsBox.update( allStringsBox.getTopLeft() + MC2Point( 0, -5 ) );
   }
   if ( m_infoDisplayRightButton ) {
      allStringsBox.update( rightButtonBox );
      // Add distance from bottom
      allStringsBox.update( allStringsBox.getTopLeft() + MC2Point( 0, -5 ) );
   }

   iPlotter->setFont( INFO_FONT, INFO_FONT_SIZE );
   // Now add the other strings.
   vector<PixelBox> stringBoxes;
   {
      stringBoxes.resize( m_infoStrings.size() );
      // Add a distance between each string
      // Also add a distance from the edge
      MC2Point distance( 5, -2 );
      // Add the strings from the bottom and up.
      for ( int i = m_infoStrings.size() - 1; i >= 0; --i ) {
         STRING* str = m_infoStrings[i];
         PixelBox curBox( iPlotter->getStringAsRectangle( *str,
                                                          MC2Point(0,0) ) );
         // Put the new box on top of the old one.
         curBox.moveBottomLeftTo( allStringsBox.getTopLeft() + distance );
         allStringsBox.update( curBox );
         stringBoxes[ i ] = curBox;
      }
   }

   // Start drawing

   // Draw the rectangle
   iPlotter->setPenColor( 180,180,255 ); 
   iPlotter->setFillColor(180,180,255 );
   iPlotter->drawRect(true, allStringsBox);
   
#if defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3
   vector<MC2Point> triangle;
   triangle.push_back( MC2Point( Size().iWidth-24, Size().iHeight-2 ));
   triangle.push_back( MC2Point( Size().iWidth-14, Size().iHeight-12 ));
   triangle.push_back( MC2Point( Size().iWidth-4, Size().iHeight-2 ));
   iPlotter->drawPolygonWithColor(triangle.begin(), triangle.end(), 0);
#endif

   // Draw the "buttons"
   iPlotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
   iPlotter->setPenColor(0,0,0);
   
   if ( m_infoDisplayLeftButton ) {
      iPlotter->drawText( *m_infoDisplayLeftButton,
                          leftButtonBox.getCenterPoint() );
      //iPlotter->drawRect( false, leftButtonBox );      
   }
   if ( m_infoDisplayRightButton ) {
      iPlotter->drawText( *m_infoDisplayRightButton,
                          rightButtonBox.getCenterPoint() );
      //iPlotter->drawRect( false, rightButtonBox );      
   }

   // Draw the info text
   iPlotter->setFont( INFO_FONT, INFO_FONT_SIZE );
   for ( uint32 i = 0; i < m_infoStrings.size(); ++i ) {
      STRING* str = m_infoStrings[i];
      iPlotter->drawText( *str, stringBoxes[i].getCenterPoint() );
   }   
   
}

/* handles a size change .. resizes the Plotter */
void CTileMapControl::SizeChanged()
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   iPlotter->resizeBuffer(Size().iWidth, Size().iHeight);
//   iMapHandler->requestRepaint();
   iMapHandler->repaint( 0 );
   return;
}

/* connects to the TMap Server and starts reuqesting\displaying maps */
void CTileMapControl::Connect() {
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   /* request a repaint */
   iMapHandler->repaint(20);
   return;
}

/* moves the map by the specified pixels */
void CTileMapControl::MoveMap(TInt dX, TInt dY) {
   
   /* tell the map Handler to move the map */
   iMapHandler->move(dX, dY);

   /* set the feature name to not be drawn */
   iDrawFeatureName = false;

   /* request a repaint */
   iMapHandler->repaint(10);

   return;
}

/* rotates the map to the left by the specified degrees */
void CTileMapControl::RotateMapLeft(TInt angle) {
 
   /* tell the map Handler to rotate the map to the left */
   iMapHandler->rotateLeftDeg(angle);

   /* request a repaint */
   iMapHandler->repaint(10);

   return;
}


/* rotates the map to the right by the specified degrees */
void CTileMapControl::RotateMapRight(TInt angle) {

   /* tell the map Handler to rotate the map to the right */
   iMapHandler->rotateLeftDeg(-angle);

   /* request a repaint */
   iMapHandler->repaint(10);

   return;
}


/* zooms the map */
void CTileMapControl::ZoomMap(float32 zoomFactor) {

   /* tell the map Handler to zoom the map */
   iMapHandler->zoom(zoomFactor);

   /* set the feature name to not be drawn */
   iDrawFeatureName = false;

   /* request a repaint */
   iMapHandler->repaint(10);

   return;
}

double
CTileMapControl::getScale() const
{
   return iMapHandler->getScale();
}

void
CTileMapControl::setTextSettings( const TileMapTextSettings& settings )
{
   iMapHandler->setTextSettings( settings );
}

void CTileMapControl::setInfoText( const vector<const char*>& rows )
{
   if ( rows.empty() && m_infoStrings.empty() ) {
      // Nothing to do.
      return;
   }
   deleteInfoStrings();
   int inSize = rows.size();
   for ( int i = 0; i < inSize; ++i ) {
      m_infoStrings.push_back( iPlotter->createString( rows[i] ) );
   }
   iMapHandler->requestRepaint();
}

void CTileMapControl::setInfoText( const char* utf8name,
                                   const char* leftButtonText,
                                   const char* rightButtonText )
{
   if ( utf8name == NULL && m_infoStrings.empty() ) {
      // Nothing to do.
      return;
   }
   
//#define TEST_LINES
#ifdef TEST_LINES
   char* xtmpStr = new char[1024];
   sprintf(xtmpStr, "%s\n%s\n%s\n", utf8name, "Annat namn",
           "lllllllllllllll----------");
   utf8name = xtmpStr;
#endif
   /* delete old string, if present */
   deleteInfoStrings();

   if ( leftButtonText ) {
      m_infoDisplayLeftButton  = iPlotter->createString( leftButtonText );
   }
   if ( rightButtonText ) {
      m_infoDisplayRightButton = iPlotter->createString( rightButtonText );
   }
   
   // Set the new one.
   if( utf8name ) {
      /* convert each line of the char string to Native string format */
      /* Use a stupid method, perhaps */
      int inLength = strlen(utf8name) + 1;
      char* tmpStr = new char[ inLength ];
      int writePos = 0;
      for ( int readPos = 0; readPos < inLength; ++readPos ) {
         char curChar = utf8name[readPos];
         // Add a new string to the vector if we reach newline or
         // the zero termination.
         if ( ( curChar == '\n' ) || ( curChar == '\0' ) ) {
            tmpStr[writePos] = '\0';
            // Do not add empty strings.
            if ( writePos != 0 ) {
               m_infoStrings.push_back( iPlotter->createString( tmpStr ) );
            }
            // Reset the string
            writePos = 0;
         } else {
            if ( curChar != '\r' ) {
               tmpStr[writePos++] = curChar;
            }
         }
      }
      delete [] tmpStr;
      /* set map to draw feature name */
      iDrawFeatureName = true;
   } else {
      /* don't draw the feature name */
      iDrawFeatureName = false;
   }
#ifdef TEST_LINES
   delete [] xtmpStr;
#endif
   iMapHandler->requestRepaint();
}

/* prints the name of the feature at the specified coordinate */
void CTileMapControl::PrintFeatureName(const MC2Point& coord)
{      
   /* get feature name from the TileMapHandler */
   setInfoText( iMapHandler->getNameForFeatureAt(coord) );
   return;
}

/* gets the name of the feature at the specified coordinate */
const char*
CTileMapControl::getFeatureName(const MC2Point& point) {

   /* get feature name from the TileMapHandler */
   return iMapHandler->getNameForFeatureAt( point );
}

void CTileMapControl::addBitmapToArray(TUserBitmap& aUserBitmap)
{
   iBitmapArray.Append(&aUserBitmap);
}

/* removes a user defined bitmap from bitmap array*/
void CTileMapControl::removeBitmapFromArray(TUserBitmap& aUserBitmap)
{
   TInt pos = iBitmapArray.Find(&aUserBitmap);
      if(pos != KErrNotFound){
         iBitmapArray.Remove(pos);
      }
}


void CTileMapControl::drawUserDefinedBitmap() const
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   if(iBitmapArray.Count() > 0){
      CFbsBitGc& gc = iPlotter->getBuffer()->Gc();
      gc.SetBrushStyle(CGraphicsContext::ENullBrush);
      for(TInt i=0;i<iBitmapArray.Count();i++){
         TUserBitmap* userbitmap = iBitmapArray[i];
         if(userbitmap->iShow){
            if(userbitmap->iBitmap != NULL){
               if(userbitmap->iBitmapMask != NULL){
                  gc.BitBltMasked( userbitmap->iBlitPoint,
                                   userbitmap->iBitmap,
                                   userbitmap->iRect,
                                   userbitmap->iBitmapMask,ETrue );
               }
               else
               {
                  gc.BitBlt( userbitmap->iBlitPoint,
                             userbitmap->iBitmap,
                             userbitmap->iRect );
               }
            }
         }
      }
      gc.SetBrushStyle(CGraphicsContext::ESolidBrush);                     
   }
}


void 
CTileMapControl::setHighlight( bool /*highlight*/ )
{
   // The crosshair looks the same, highlighted or not highlighted.
}

void
CTileMapControl::setCursorPos( const MC2Point& pos )
{
   m_hiddenCursorPoint = pos;
}

MC2Point 
CTileMapControl::getCursorPos() const
{
   if ( iDrawCrosshair ) {
      // Cursor is always in the center if visible..
      return MC2Point( Size().iWidth >> 1,
                       Size().iHeight >> 1 );
   } else {
      // Or else at the specified position.
      return m_hiddenCursorPoint;
   }
}

void 
CTileMapControl::setCursorVisible( bool visible )
{
   iDrawCrosshair = visible;
}

void 
CTileMapControl::getCursorBox( PixelBox& box ) const
{
   box = PixelBox( MC2Point( 0, 0), 
                   MC2Point( Size().iWidth, Size().iHeight ) );

}

void
CTileMapControl::setKeyHandlerAndOwnership( TileMapKeyHandler* keyHandler )
{
   m_keyHandlerToDelete = keyHandler;
}
