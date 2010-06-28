/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TRIALCONTAINER_H
#define TRIALCONTAINER_H

#include <coecntrl.h>
#include <eiklbo.h>
#include <badesca.h>

namespace isab{
   class Log;
}

/**
 *  CTrialContainer  container control class. 
 */
class CTrialContainer : public CCoeControl,
                        public MEikListBoxObserver
{

public: // Constructors and destructor

   CTrialContainer(class isab::Log* aLog);

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect, 
                    class CTrialView* aView );
private:
   class CEikLabel* CreateLabelL(TInt aTextResource);
public:
   /**
    * Destructor.
    */
   virtual ~CTrialContainer();

public: // New functions

   void ResetItems(TBool showTrial);
   /**
    * Change to the selected page.
    */
   void GoToSelection();

   void SetTrialMessageL(const TDesC& aMessage);

   void ReportProgress(TInt aVal, TInt aMax);
   void AskForIAPDone();

   void SetTimerTick(class TTimeIntervalMicroSeconds32  aTick);
   void StartTimer();
   void StopTimer();
   static TInt Period(TAny* aPtr);
   void DoPeriod();

public: // Functions from base classes
   
   // From MEikListBoxObserver

   /**
    * Handles listbox event.
    * @param aListBox Pointer to ListBox object is not used.
    * @param aEventType Type of listbox event.
    */
   void HandleListBoxEventL(class CEikListBox* aListBox,
                            enum TListBoxEvent aEventType );


private: // Functions from base classes
   void SetLabelRects();

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
   class CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From ?base_class ?member_description
    */

   /**
    * From CCoeControl : Handling key event.
    * @param aKeyEvent : Information of inputted key. 
    * @param aType : EEventKeyDown | EEventKey | EEventKeyUp
    * @return EKeyWasConsumed if keyevent is used.
    *         Otherwise, return EKeyWasNotConsumed. 
    */
   enum TKeyResponse OfferKeyEventL(const struct TKeyEvent& aKeyEvent, 
                                    enum TEventCode aType);

private: //data
   void AppendItemAndUpdateBoxL(TInt aResource, CDesCArray& aArray);


   /// The parent view
   class CTrialView* iView;

   /// The links list.
   class CAknDoubleStyleListBox* iListBox;

   class CPeriodic* iPeriodicTimer;
   class TTimeIntervalMicroSeconds32  iTick;
   TPoint iNormalTopLeft;

   class isab::Log* iLog;
/*    TInt iProgress; */
/*    TInt iMaxProgress; */
/*    TRgb iCurColor; */
   TBool iShowTrial;
};

#endif

// End of File
