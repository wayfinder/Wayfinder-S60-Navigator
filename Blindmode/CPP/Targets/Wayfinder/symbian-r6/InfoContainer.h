/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef INFOCONTAINER_H
#define INFOCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <aknutils.h>
#include <aknlists.h>
#include "Log.h" 
// FORWARD DECLARATIONS
class CInfoView;

#undef USE_BT_TIME_DEBUG

// CLASS DECLARATION

/**
 *  CInfoContainer  container control class.
 *  
 */
class CInfoContainer : public CCoeControl,
                       MCoeControlObserver
{
public: // Constructors and destructor
	static class CInfoContainer * NewL(
			class CInfoView * aParent,
			const TRect& aRect,
			const TDesC& aMbmFile,
			isab::Log * aLog
	);

   /**
    * Destructor.
    */
   ~CInfoContainer();

private:

   CInfoContainer(isab::Log* aLog) : iLog(aLog) {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL(
			CInfoView * aView, 
			const TRect& aRect, 
			const TDesC& aMbmFile
	);

public: // New functions
   HBufC * GetSpeedLabel() const;

   void SetPadding(HBufC * text, TInt num);
   
   void SetPosL(TInt32 aLatitude, TInt32 aLongitude);

   void SetAltitude(TInt altitude);

   void SetHeading(TInt aHeading);
   
   void SetSpeed(TInt aSpeed);

   void SetGpsState(TInt aState);

   void SetSpeedLimit(TInt aLimit);

   void PositionOfControlChanged(CCoeControl *control, TPoint pos);

   void SetListArray();

   void ReplaceListEntryL(TInt aNumber, const TDesC& aNewEntry);


   void UpdateReading();

private: // Functions from base classes

   /*
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

   void HandleResourceChange(TInt aType);

	/**
	 * From CCoeControl : Handling key event.
	 * @param aKeyEvent : Information of inputted key.
	 * @param aType : EEventKeyDown | EEventKey | EEventKeyUp
	 * @return EKeyWasConsumed if keyevent is used.
	 *         Otherwise, return EKeyWasNotConsumed.
	 */
	TKeyResponse OfferKeyEventL(
			const TKeyEvent& aKeyEvent,
			TEventCode aType
	);

private: //data
   void SetLatOrLon(TInt32 aLatOrLon, TInt32 index);
   
   CInfoView* iView;

   class CAknDoubleStyleListBox * iListBox;

   isab::Log* iLog;
};

#endif

// End of File
