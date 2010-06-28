/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
*  Description:
*     Declares container control for application.
*/

#ifndef COORDINATESCONTROL_H
#define COORDINATESCONTROL_H

// INCLUDES
#include <coecntrl.h>  // for coe control
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for edwin text boxes
#include "WayFinderConstants.h"

#define MAX_LABELS 10
#define MAX_EDWINS 10

// CLASS DECLARATION

/**
 *  CCoordinatesControl  container control class.
 *  
 */
class CCoordinatesControl : public CCoeControl,
						          public MCoeControlObserver
{
public: // Constructors and destructor

   /**
    * Constructor.
    */
   CCoordinatesControl(TRgb aBackground,
                       TRgb aText,
                       TBool aBold );

   /**
    * Destructor.
    */
   ~CCoordinatesControl();
        
   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructFromResourceL(TResourceReader& aReader);

public: // New functions

   void SetCoordinateEditors( TChar& aSign, TInt* aDeg,
         TInt* aMin, TInt* aSec, TInt* aSecFr, TBool isLat);

   /**
    * Set the latitude value.
    */
   void SetLatitudeL( TInt32 &aLat );

   /**
    * Set the longitude value.
    */
   void SetLongitudeL( TInt32 &aLon );

   /**
    * Get the latitude value.
    */
   TBool GetLatitudeL( TInt32 &aLat );

   /**
    * Get the longitude value.
    */
   TBool GetLongitudeL( TInt32 &aLon );

   TBool IsLatFocused();

   TBool IsLonFocused();

   void SetLatFocus( TBool aFocused, TDrawNow aDrawNow );

   void SetLonFocus( TBool aFocused, TDrawNow aDrawNow );

private: // New Functions

   void SetCoordinate( TInt32 aCoord,
                       TChar &aSign,
                       TInt &aDeg,
                       TInt &aMin,
                       TInt &aSec,
                       TInt &aSecFr,
                       TBool aIsLat );

   void GetCoordinate( TInt32 &aCoord,
                       TChar &aSign,
                       TLex16 aDeg,
                       TLex16 aMin,
                       TLex16 aSec,
                       TLex16 aSecFr );

   /**
    * Calculates the positions for this controls
    * components. If v3 the positions is calculated
    * depending on the positions on v2. The positions
    * will then differentiate depending on the screen
    * size.
    */
//    TPoint CalculatePositions(TInt aX, TInt aY);

   /** 
    * Same as the function above but here we
    * calculate the size for the component 
    * instead of the position.
    */
//    TSize CalculateSize(TInt aWidth, TInt aHeight);

   TSize GetSize(TSize size, const class CFont *aFont, TInt aNbrChars);
   TInt GetMaxEdwinHeight();

   TBool WrapRight(CEikEdwin* edwin);
   TBool WrapLeft(CEikEdwin* edwin);

public: // Functions from base classes

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private: // Functions from base classes

	void FocusChanged(TDrawNow aDrawNow);

   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
   TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From ?base_class ?member_description
    */
   // event handling section
   // e.g Listbox events
   void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

private: //data
   
   /// Top left of the control rectangle;
   TPoint       iTopLeft;
   
   /// Bottom right of the control rectangle;
   TPoint       iBottomRight;

   /// Position of the lat label
   TPoint       iLatLabelPos[MAX_LABELS];

   /// Position of the lat sign edwin
   TPoint       iLatEdwinPos[MAX_EDWINS];

   /// Latitude label
   CEikLabel*   iLabelLat[MAX_LABELS];

   /// Latitude sign textbox
   CEikEdwin*   iEdwinLat0;

   /// Latitude degrees textbox
   CEikEdwin*   iEdwinLat1;

   /// Latitude minutes textbox
   CEikEdwin*   iEdwinLat2;

   /// Latitude seconds textbox
   CEikEdwin*   iEdwinLat3;

   /// Latitude seconds textbox
   CEikEdwin*   iEdwinLat4;

   /// Longitude sign textbox
   CEikEdwin*   iEdwinLon0;

   /// Longitude degrees textbox
   CEikEdwin*   iEdwinLon1;

   /// Longitude minutes textbox
   CEikEdwin*   iEdwinLon2;

   /// Longitude seconds textbox
   CEikEdwin*   iEdwinLon3;

   /// Longitude seconds textbox
   CEikEdwin*   iEdwinLon4;

   TRgb iBackground;

   TRgb iText;

   TBool iBold;
};

#endif

// End of File
