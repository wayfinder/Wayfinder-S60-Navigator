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

#ifndef _MAPINFOCONTROL_H_
#define _MAPINFOCONTROL_H_

// FORWARD DECLARATIONS
#include <coecntrl.h>
#include <fbs.h>

#include "config.h"
#include <vector>
#include "MapRenderer.h"

#include <utility>

#include "MC2SimpleString.h"
#include <vector>

class PixelBox;

/**
 *    Interface for Blue notes.
 */
class MMapInfoControl {
public:

   /**
    *    Set info text of the info-square.
    *    @param   utf8name        If NULL no text will be displayed.
    *    @param   leftButtonText  If NULL no left button will be there.
    *    @param   rightButtonText If NULL no right button will be there.
    */
   virtual void setInfoText( const char* utf8name,
                             const char* leftButtonText = NULL,
                             const char* rightButtonText = NULL ) = 0;

   /**
    *    Empty virtual destructor.
    */
   virtual ~MMapInfoControl();
   
};

/**
 *    Control containing map information.
 *    Also known as the blue note.
 */
class CMapInfoControl : public CCoeControl, MMapInfoControl {
public:

   /**
    *    Constructor.
    *    @param   rect  The rect of the parent window.
    */
   void ConstructL( const TRect& rect ); 

   /**
    *    Constructor for window owning version of the control.
    *    @param   rect  The rect of the parent window.
    */
   void ConstructL( CCoeControl& parent,
                    const TRect& rect ); 

   /**
    *    Destructor.
    */
   ~CMapInfoControl();

   /**
    *    Draw everything within the specified rectangle.
    */
   void Draw( const TRect& rect ) const;
   
   /**
    *    Size of the parent window has changed. Resize the plotter.
    */
   void SetMaxSize( const TSize& newMaxSize );

   /**
    *    Called by the framework when the size changes.
    */
   void SizeChanged();
   
   /**
    *    Set info text of the info-square.
    *    @param   utf8name        If NULL no text will be displayed.
    *    @param   leftButtonText  If NULL no left button will be there.
    *    @param   rightButtonText If NULL no right button will be there.
    */
   void setInfoText( const char* utf8name,
                     const char* leftButtonText = NULL,
                     const char* rightButtonText = NULL );


   /**
    *    Returns the info box rect.
    */
   TRect getInfoBox();

   /**
    *    Set the night mode flag to true or false.
    */
   void SetNightMode( TBool aNightMode );

   /**
    *    Returns the maximum size.
    */
   const TSize& maxSize() const { return m_maxSize; }

   /**
    * Getter for the default height, the height of one line of text.
    */
   TInt getDefaultHeight();

  
private:   

   enum {
      /// Margin from sides in pixels
      c_sideMargin = 5,
   };
   
   /**
    *    Get the button position.
    */
   PixelBox getButtonPosition( bool left, STRING* text ) const;

   /**
    *    Draw info text.
    */
   PixelBox drawInfoText() const;
   
   /**
    *    Delete the info strings.
    */
   void deleteInfoStrings();

   /**
    *    Break lines for strings that are too long
    */
   void breakLines( std::vector<STRING*>& broken,
                    const std::vector<MC2SimpleString>& lines,
                    int nbrLinesToBreak = MAX_INT32 );
   
   /* stores the current feature name and position */
   std::vector<STRING*> m_infoStrings;

   /**
    *   Stores the current text of the left button for the
    *   info stuff. (Somewhat series 60 specific)
    */
   STRING* m_infoDisplayLeftButton;

   /**
    *   Stores the current text of the right button for the
    *   info stuff. (Somewhat series 60 specific)
    */
   STRING* m_infoDisplayRightButton;
   
   /**
    *    The plotter.
    */
   isab::MapRenderer* m_plotter;

   /**
    *    The last rectangle of what was drawn in the control.
    */
   TRect m_infoBox;

   /**
    *    Maximum size of the control, i.e. the size of the screen.
    */
   TSize m_maxSize;

   /// True if the control owns a window
   bool m_windowOwning;

   /// Parent control
   class CCoeControl* m_parentControl;

   /// The original rect
   TRect m_rect;

   /// Night mode on or off
   TBool m_nightMode;
};

#endif


