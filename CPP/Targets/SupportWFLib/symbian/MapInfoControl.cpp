/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <coemain.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <aknutils.h>
#endif
#include "MapInfoControl.h"

#include "PixelBox.h"
#include "Surface.h"

using namespace std;

#if defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3
   #ifdef MOTOROLA
      #define INFO_FONT _L("Symbian Viking")
      #define BUTTON_FONT _L("Symbian Viking")
   #else
      #define INFO_FONT _L("SwissA")
      #define BUTTON_FONT _L("SwissA")
   #endif
   #define INFO_FONT_SIZE   17
   #define BUTTON_FONT_SIZE   17
#else
   #ifdef NAV2_CLIENT_SERIES60_V3
      #define INFO_FONT        _L("LatinBold17")
      #define INFO_FONT_SIZE   17
      #define BUTTON_FONT      _L("LatinBold17")
      #define BUTTON_FONT_SIZE 17
   #else   
      #define INFO_FONT        _L("LatinPlain12")
      #define INFO_FONT_SIZE   12
      #define BUTTON_FONT      _L("LatinPlain12")
      #define BUTTON_FONT_SIZE 12
   #endif
#endif

static const int spaceFromBottom = 3;

using namespace isab;

MMapInfoControl::~MMapInfoControl()
{
   // Nothing to be done
}

void 
CMapInfoControl::ConstructL( const TRect& rect ) {

   // 256 colors should be enough.
   int colorDepth = 8;
   
   m_plotter = MapRenderer::NewL( *this, 
                                  rect.Width(), rect.Height(),
                                  colorDepth );
   m_maxSize = rect.Size();  
   SetFocusing( false );

   PixelBox pixelRect( rect );
   
   SetRect( TRect( pixelRect.getBottomLeft(), TSize(0,0 ) ) );
   m_infoBox = Rect();
   m_rect = rect;
}

void 
CMapInfoControl::ConstructL( CCoeControl& parent,
                             const TRect& rect )
{
   m_parentControl = &parent;
   m_windowOwning = true;
   CreateWindowL( &parent );
   ConstructL( rect );
}


CMapInfoControl::~CMapInfoControl() {
   deleteInfoStrings();
   delete m_plotter;
}

void 
CMapInfoControl::SetMaxSize( const TSize& size )
{
   m_maxSize = size;
   m_plotter->resizeBuffer(maxSize().iWidth, maxSize().iHeight);
   setInfoText( NULL );
}

void
CMapInfoControl::SizeChanged()
{
   TSize size( Rect().Size() );
   if ( size.iHeight > m_maxSize.iHeight ||
        size.iWidth  > m_maxSize.iWidth ) {
#ifndef NAV2_CLIENT_SERIES60_V3
      size.iWidth = m_rect.iBr.iX;
#endif
      SetMaxSize( size );
   }
}

PixelBox 
CMapInfoControl::getButtonPosition( bool left, STRING* text ) const
{
   if ( text != NULL ) {
      // Ok - do the work
      PixelBox buttonBox = m_plotter->getStringAsRectangle(*text,
                                                          MC2Point(100,100));
      int xMovement = 1;
      int bottomY = maxSize().iHeight - 1;
      if ( left ) {
         return buttonBox.moveBottomLeftTo( MC2Point( xMovement, bottomY ) );
      } else {
         return buttonBox.moveBottomRightTo(
            MC2Point( maxSize().iWidth- xMovement, bottomY ) );
      }
   }
   return PixelBox();
}

PixelBox 
CMapInfoControl::drawInfoText() const
{
   if ( m_infoStrings.empty() ) {
      return PixelBox();
   }
   m_plotter->setPenColor(0, 0, 0);
   m_plotter->setLineWidth(1);
#if defined NAV2_CLIENT_SERIES60_V3
   const CFbsFont* font = dynamic_cast<const CFbsFont *>(AknLayoutUtils::
      FontFromId(EAknLogicalFontPrimaryFont));
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      m_plotter->setFont(fontAttrib.ShortFullName(), font->HeightInPixels());

   } else {
      m_plotter->setFont( INFO_FONT, INFO_FONT_SIZE );
   }
#else   
   // Set the correct font for the size calculations
   m_plotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
#endif
   PixelBox leftButtonBox(
      getButtonPosition( true, m_infoDisplayLeftButton) );
   
   PixelBox rightButtonBox(
      getButtonPosition( false, m_infoDisplayRightButton ) );

   // Make a thin box covering the width of the screen
   PixelBox allStringsBox( MC2Point( 0, maxSize().iHeight - spaceFromBottom ),
                           MC2Point( maxSize().iWidth, maxSize().iHeight )  );

   if ( m_infoDisplayLeftButton ) {
      allStringsBox.update( leftButtonBox );
      // Add distance from bottom
      allStringsBox.update( allStringsBox.getTopLeft() + MC2Point( 0, -5 ) );
   }
   if ( m_infoDisplayRightButton ) {
#if !defined NAV2_CLIENT_UIQ && !defined NAV2_CLIENT_UIQ3
      allStringsBox.update( rightButtonBox );
      // Add distance from bottom
      allStringsBox.update( allStringsBox.getTopLeft() + MC2Point( 0, -5 ) );
#else 
      bool moveString = false;
      STRING* str = m_infoStrings[ m_infoStrings.size() - 1 ];
      PixelBox lastBox( m_plotter->getStringAsRectangle( *str,
                                                         MC2Point(0,0) ) );
      // Move the strings if the last string
      // overlaps with the "More" text
      int rightMargin = allStringsBox.getWidth() - 
                          (rightButtonBox.getWidth() + 3);
      if( lastBox.getWidth() > rightMargin ){
         moveString = true;
      }
      if( moveString ){
         // Add distance from bottom
         allStringsBox.update( rightButtonBox );
      }
#endif
   }

   // Now add the other strings.
   vector<PixelBox> stringBoxes;
   {
      stringBoxes.resize( m_infoStrings.size() );
      // Add a distance between each string
      // Also add a distance from the edge
      MC2Point distance( c_sideMargin, -2 );
      // Add the strings from the bottom and up.
      for ( int i = m_infoStrings.size() - 1; i >= 0; --i ) {
         STRING* str = m_infoStrings[i];
         PixelBox curBox( m_plotter->getStringAsRectangle( *str,
                                                          MC2Point(0,0) ) );
         // Put the new box on top of the old one.
         curBox.moveBottomLeftTo( allStringsBox.getTopLeft() + distance );
         
         allStringsBox.update( curBox );
         stringBoxes[ i ] = curBox;
      }
   }
   // Special case: center the text if we only have one line of text.
   if(stringBoxes.size() == 1) {
      MC2Point newCenter(allStringsBox.getWidth() / 2, stringBoxes[0].getCenterPoint().getY());
      stringBoxes[0].moveCenterTo(newCenter);
   }

   // Start drawing

   // Draw the rectangle
   if( m_nightMode ){
      m_plotter->setPenColor(66, 69, 66);
      m_plotter->setFillColor(66, 69, 66);
   }
   else{
      m_plotter->setPenColor(255, 255, 255);
      m_plotter->setFillColor(255, 255, 255);
   }
   m_plotter->drawRect(true, allStringsBox);

   m_plotter->setPenColor(0, 0, 0);
   m_plotter->setFillColor(0, 0, 0);
   m_plotter->drawLine(allStringsBox.getTopLeft().getX(), allStringsBox.getTopLeft().getY(),
                       allStringsBox.getTopRight().getX(), allStringsBox.getTopRight().getY());


#if defined NAV2_CLIENT_SERIES60_V3
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      m_plotter->setFont(fontAttrib.ShortFullName(), font->HeightInPixels());

   } else {
      m_plotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
   }
#else   
   // Draw the "buttons"
   m_plotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
#endif
   if( m_nightMode ){
      m_plotter->setPenColor(221,221,221);
   }
   else{
      m_plotter->setPenColor(0,0,0);
   }
   if ( m_infoDisplayLeftButton ) {
      m_plotter->drawText( *m_infoDisplayLeftButton,
                          leftButtonBox.getCenterPoint() );
   }
   if ( m_infoDisplayRightButton ) {
      m_plotter->drawText( *m_infoDisplayRightButton,
                          rightButtonBox.getCenterPoint() );
   }

#if defined NAV2_CLIENT_SERIES60_V3
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      m_plotter->setFont(fontAttrib.ShortFullName(), font->HeightInPixels());

   } else {
      m_plotter->setFont( INFO_FONT, INFO_FONT_SIZE );
   }
#else
   // Draw the info text
   m_plotter->setFont( INFO_FONT, INFO_FONT_SIZE );
#endif
   for ( uint32 i = 0; i < m_infoStrings.size(); ++i ) {
      STRING* str = m_infoStrings[i];
      m_plotter->drawText( *str, stringBoxes[i].getCenterPoint() );
   }   
   return allStringsBox;
}

void 
CMapInfoControl::Draw( const TRect& rect ) const {
   

   CWindowGc& gc = SystemGc();   
   
   PixelBox infoBox(m_infoBox);
   
   // I think we will have to move the infobox
   // The bottom of the info box should be at the bottom of the
   // control...
   // Thank god for the PixelBox.
   PixelBox myBox( Rect() );
   infoBox.moveBottomLeftTo( myBox.getBottomLeft() );
   
   if ( m_windowOwning ) {
      // Try to render the bottom
      m_plotter->RenderTo( gc,                       
                           myBox.getTopLeft().getX(),
                           myBox.getTopLeft().getY(),
                           m_infoBox );
   } else {
      TRect rectToDraw = rect;
      
      rectToDraw.Intersection( infoBox );

      TRect rectToBlitFrom = m_infoBox;
      
      m_plotter->RenderTo( gc, 
                           rectToDraw.iTl.iX,
                           rectToDraw.iTl.iY, 
                           rectToBlitFrom );
      
      // Enable to be able to see what's redrawn
#if 0
      gc.SetPenSize( TSize(5,5) );
      // Also draw the rect.
      gc.SetPenColor( TRgb( 0,0,0 ) );
      gc.DrawRect( rectToDraw );
      gc.SetPenSize( TSize(1,1) );
#endif

   }
}

void 
CMapInfoControl::deleteInfoStrings()
{
   int size = m_infoStrings.size();
   for( int i = size - 1; i >=0; --i ) {
      m_plotter->deleteString( m_infoStrings[i] );
   }
   m_infoStrings.clear();
   // Also delete the button texts
   m_plotter->deleteString( m_infoDisplayLeftButton );
   m_infoDisplayLeftButton = NULL;
   m_plotter->deleteString( m_infoDisplayRightButton );
   m_infoDisplayRightButton = NULL;
}

void
CMapInfoControl::breakLines( vector<STRING*>& broken,
                             const vector<MC2SimpleString>& lines,
                             int nbrLinesToBreak )
{
   MC2Point origo(0,0);
   const uint32 screenWidth = maxSize().iWidth - ( c_sideMargin << 1 );
   broken.reserve( lines.size() * 2 );
   for( vector<MC2SimpleString>::const_iterator it = lines.begin();
        it != lines.end();
        ++it ) {
      // Will only work for max 2147483647 lines (MAX_INT32)
      if ( nbrLinesToBreak-- > 0 ) {
         // Blaargh! It became complicated.
         STRING* curLine = m_plotter->createString( (*it).c_str() );
         isab::Rectangle curRect = m_plotter->getStringAsRectangle( *curLine,
                                                                    origo );
         m_plotter->deleteString( curLine );
         if ( curRect.getWidth() < screenWidth ) {
            // All is ok
            broken.push_back( m_plotter->createString( (*it).c_str() ) );
         } else {
            // Now we must do tricks. Expensive tricks.
            const MC2SimpleString& curLine = *it;
            char* curWords  = new char[ curLine.length() + 2 ];
            char* lastWords = new char[ curLine.length() + 2 ];
            
            lastWords[0] = '\0';
            
            int curPos = 0;
            
            for ( uint32 i = 0; i < curLine.length() + 1; ++i ) {
               if ( curLine[i] == ' ' || curLine[i] == '\0' ) {
                  curWords[curPos++] = '\0';
                  // Now we must test it. Unfortunately with an alloc or two.
                  STRING* plotterString = m_plotter->createString( curWords );
                  if ( m_plotter->getStringAsRectangle( *plotterString,
                                                        origo ).getWidth() >=
                       screenWidth ) {
                     if ( lastWords[0] != '\0' ) {
                        broken.push_back(
                           m_plotter->createString( lastWords ) );
                        // Move last word of curWords to lastWords
                        const char* firstSpace = strrchr( curWords, ' ' );
                        if ( firstSpace == NULL ) {
                           firstSpace = curWords;
                        } else {
                           // Skip the space first in the string.
                           ++firstSpace;
                        }
                        strcpy( lastWords, firstSpace );
                        // Keep last word in curWords too.
                        strcpy( curWords, lastWords );
                        curPos = strlen( curWords );
                        // Put the space back again
                        curWords[ curPos++ ] = curLine[i];
                        // For debug purposes
                        curWords[curPos] = '\0';
                     } else {
                        // No last word
                        broken.push_back(
                           m_plotter->createString( curWords ) );
                        // Restart the copying.
                        curWords[0] = '\0';
                        curPos = 0;
                     }
                     
                  } else {
                     // Add last word of curWords to lastWords
                     char* firstSpace = strrchr( curWords, ' ' );
                     if ( firstSpace != NULL ) {
                        strcat( lastWords, firstSpace );
                     } else {
                        strcpy( lastWords, curWords );
                     }
                     // Put back the space again
                     --curPos;
                     curWords[ curPos++ ] = curLine[i];
                  }
                  m_plotter->deleteString( plotterString );
               } else {
                  curWords[curPos++] = curLine[i];
                  // For debug purposes
                  curWords[curPos] = '\0';
               }
            }
            
            if ( lastWords[0] != '\0' ) {
               broken.push_back( m_plotter->createString( lastWords ) );
            }
            
            delete [] curWords;
            delete [] lastWords;
         }
      } else {
         broken.push_back( m_plotter->createString( (*it).c_str() ) );
      }
   }
}

void 
CMapInfoControl::setInfoText( const char* utf8name,
                              const char* leftButtonText,
                              const char* rightButtonText )
{
   if ( utf8name == NULL && m_infoStrings.empty() ) {
      // Nothing to do.
      return;
   }
   
   /* delete old string, if present */
   deleteInfoStrings();

   if ( leftButtonText ) {
      m_infoDisplayLeftButton  = m_plotter->createString( leftButtonText );
   }
   if ( rightButtonText ) {
      m_infoDisplayRightButton = m_plotter->createString( rightButtonText );
   }
   
   // Set the new one.   
   vector<MC2SimpleString> strings;
   if( utf8name ) {
      // Guess some size
      strings.reserve( 16 );
      /* convert each line of the char string to Native string format */
      /* Use a stupid method, perhaps */
      int inLength = strlen(utf8name) + 1;

      // Alloc on the stack if the string is small enough.
      char stackStr[64];      
      char* tmpStr       = stackStr;
      char* toDelete     = NULL;
      
      if ( inLength >= int(sizeof(stackStr)/sizeof(stackStr[0]) ) ) {
         toDelete = new char[ inLength ];
         tmpStr   = toDelete;
      }
      int writePos = 0;
      for ( int readPos = 0; readPos < inLength; ++readPos ) {
         char curChar = utf8name[readPos];
         // Add a new string to the vector if we reach newline or
         // the zero termination.
         if ( ( curChar == '\n' ) || ( curChar == '\0' ) ) {
            tmpStr[writePos] = '\0';
            // Do not add empty strings.
            if ( writePos != 0 ) {
               strings.push_back( tmpStr );
            }
            // Reset the string
            writePos = 0;
         } else {
            if ( curChar != '\r' ) {
               tmpStr[writePos++] = curChar;
            }
         }
      }
      delete [] toDelete;
   }

   //Set the font. We need to set it before the call to breakLines.
#if defined NAV2_CLIENT_SERIES60_V3
   const CFbsFont* font = dynamic_cast<const CFbsFont *>(AknLayoutUtils::
      FontFromId(EAknLogicalFontPrimaryFont));
   if (font && font->IsOpenFont()) {
      TOpenFontFaceAttrib fontAttrib;
      font->GetFaceAttrib(fontAttrib);
      m_plotter->setFont(fontAttrib.ShortFullName(), font->HeightInPixels());
   } else {
      m_plotter->setFont( INFO_FONT, INFO_FONT_SIZE );
   }
#else
   // Set the correct font for the size calculations
   m_plotter->setFont( BUTTON_FONT, BUTTON_FONT_SIZE );
#endif

   // Break the lines to fit on the screen.
   breakLines( m_infoStrings, strings, MAX_INT32 ); // Only break first line.
      
   // Calculate the rect that needs to be redrawn.
   TRect newInfoBox = drawInfoText();

   TRect invalidateBox = m_infoBox;
   // The invalidate rect should be the union of the 
   // previous and current rect.
   invalidateBox.BoundingRect( newInfoBox );

   m_infoBox = newInfoBox;

   if ( ! m_windowOwning ) {
      // Get a redraw. 
      Window().Invalidate(
         PixelBox(invalidateBox).moveBottomLeftTo(
            PixelBox(Rect()).getBottomLeft() ) );
      // Set size
      TPoint topLeft(m_rect.iTl.iX, 0);
      PixelBox screenRect = TRect( topLeft, Window().Size() );
      PixelBox infoPixBox( m_infoBox );
      infoPixBox.moveBottomLeftTo( screenRect.getBottomLeft() );
      SetRect( infoPixBox );
   } else {
      PixelBox infoPix( m_infoBox );
      infoPix.moveBottomLeftTo(
         PixelBox(m_parentControl->Rect()).getBottomLeft());
      Window().Invalidate( infoPix );
      SetRect( infoPix );
   }
}


void
CMapInfoControl::SetNightMode( TBool aNightMode )
{
   m_nightMode = aNightMode;
}


TRect
CMapInfoControl::getInfoBox()
{
   return m_infoBox;
}

TInt
CMapInfoControl::getDefaultHeight()
{
#if defined NAV2_CLIENT_SERIES60_V3
   TInt fontHeight;

   const CFbsFont* font = dynamic_cast<const CFbsFont *>
      (AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont));

   if (font && font->IsOpenFont()) {
      fontHeight = font->HeightInPixels();
   } else {
      fontHeight = BUTTON_FONT_SIZE;
   }
   return fontHeight + spaceFromBottom;
#else   
   return BUTTON_FONT_SIZE + spaceFromBottom;
#endif
}
