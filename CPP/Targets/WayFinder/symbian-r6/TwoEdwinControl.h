/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TWOEDWINCONTROL_H
#define TWOEDWINCONTROL_H

// INCLUDES
#include <coecntrl.h>  // for coe control
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for edwin text boxes

#include "WayFinderConstants.h"

// CLASS DECLARATION

/**
 *  CTwoEdwinControl  container control class.
 *  
 */
class CTwoEdwinControl : public CCoeControl,
						       public MCoeControlObserver
{
public: // Constructors and destructor

   /**
    * Constructor.
    */
   CTwoEdwinControl( TInt aLabelOneUid, TInt aLabelTwoUid,
                     TInt aEdwinOneUid, TInt aEdwinTwoUid );

   /**
    * Destructor.
    */
   ~CTwoEdwinControl();
        
   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructFromResourceL(TResourceReader& aReader);

public: // New functions

   void SetFirstEdwinL( TDes* aText );

   void SetSecondEdwinL( TDes* aText );

   void GetFirstEdwin( TDes* aText );

   void GetSecondEdwin( TDes* aText );

   TBool IsFirstFocused();

   TBool IsSecondFocused();

   void SetSecondFocused( TBool aFocused, TDrawNow aDrawNow );

   void SetFirstDimmed( TBool aDimmed );

   TBool IsFirstDimmed();

   void SetSecondDimmed( TBool aDimmed );

   TBool IsSecondDimmed();

public: // Functions from base classes

	void PrepareForFocusLossL();

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private: // New functions

   /**
    * Calculates the positions for this controls
    * components. If v3 the positions is calculated
    * depending on the positions on v2. The positions
    * will then differentiate depending on the screen
    * size.
    */
   TPoint CalculatePositions(TInt aX, TInt aY);

   /** 
    * Same as the function above but here we
    * calculate the size for the component 
    * instead of the position.
    */
   TSize CalculateSize(TInt aWidth, TInt aHeight);

   TSize GetSize(TSize size, const class CFont *aFont, TInt aNbrChars);
   TInt GetMaxEdwinHeight();

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

   TInt         iLabelOneUid;

   TInt         iLabelTwoUid;

   TInt         iEdwinOneUid;

   TInt         iEdwinTwoUid;
   
   /// Top left of the control rectangle;
   TPoint       iTopLeft;
   
   /// Bottom right of the control rectangle;
   TPoint       iBottomRight;

   /// Position of the lat label
   TPoint       iFirstLabelPos;

   /// Position of the lat label
   TPoint       iSecondLabelPos;

   /// Position of the lat minutes edwin
   TPoint       iFirstEdwinPos;

   /// Position of the lat seconds edwin
   TPoint       iSecondEdwinPos;

   /// Latitude label
   CEikLabel*   iLabelOne;

   /// Latitude label
   CEikLabel*   iLabelTwo;

   /// Latitude sign textbox
   CEikEdwin*   iEdwinOne;

   /// Latitude degrees textbox
   CEikEdwin*   iEdwinTwo;

};

#endif

// End of File
