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

#ifndef LINEOBSERVABLEDIALOG_H
#define LINEOBSERVABLEDIALOG_H
#include <eikdialg.h> //CEikDialog

///Mixin class tha observes events on certain lines of a dialog. In
///effect this is the same as observing the control on that line, but
///we get the entire line to play with in the observer.
class MDialogLineObserver{
public:
   ///Called whenever a control in a CLineObservableDialog triggers a
   ///controlevent that is reported to a MCoeControlObserver.
   ///@param aLine the line containing the control that triggered the event.
   ///@param aEvent the event as reported to 
   ///              MCoeControlObserver::HandleControlEventL.
   virtual void HandleLineEventL(class CEikCaptionedControl* aLine,
                                 enum MCoeControlObserver::TCoeEvent aEvent)=0;
                                    
};

///A CEikDialog subclass that allows any line of the dialog to be
///observed individually. It's possible for a line to be observed from
///any number of observers and for any observer to observe any number
///of lines. It's even possible for a single observer to observe the
///same line several times and receive several calls to
///HandleLineEventL.
///Also notable is the fact that if the dialog already has an
///MCoeControlObserver set, any events will still cascade to that
///observer unaffected. This only hold if the obeserver has been set
///before the first lineobserver is set.
class CLineObservableDialog : public CEikDialog {
   ///Class used to keep track of which observer that observes which
   ///line.
   class  TLineObserver {
      ///The id of the observed line, i.e. the id of the control on
      ///that line.
      TInt iId;
      ///Pointer to the observer. 
      class MDialogLineObserver* iObserver;
   public:
      ///Constructor.
      ///@param aId the id of the line to observe.
      ///@param aObserver pointer to the observer. 
      TLineObserver(TInt aId, class MDialogLineObserver* aObserver) : 
         iId(aId), iObserver(aObserver)
      {}
      ///Returns the observer.
      ///@return the observer. 
      class MDialogLineObserver* operator()()
      {
         return iObserver;
      }
      ///Returns the id.
      ///@return the id of the observed line. 
      TInt Id() const
      {
         return iId;
      }
   };

   ///Holds the line observers. 
   RArray<TLineObserver> iObservers;
   ///Pointer to previously set observer.
   class MCoeControlObserver* iCascadeObserver;

   ///Sees if a control id and a control match. 
   ///@param aId an Id that may or may not identify a control in this dialog.
   ///@param aControl pointer to a dialog that may or may not reside
   ///                in this dialog.
   ///@return ETrue if the id identifies a control in this dialog and
   ///        that control is the same as aControl. EFalse otherwise
   ///        excpet if the id is not in this dialog and aControl is
   ///        NULL.
   TBool MatchControl(TInt aId, class CCoeControl* aControl);
public:
   ///Virtual destructor. 
   virtual ~CLineObservableDialog();
   ///@name From  MCoeControlObserver
   //@{
   ///Dispatches HandleLineEventL calls to all line observers, then on
   ///to any previously set observer.
   virtual void HandleControlEventL(class CCoeControl* aControl,
                                    enum TCoeEvent aEventType);
   //@}

   ///@name From MEikDialogPageObserver
   //@{
   virtual void PageChangedL(TInt aPageId);
   //@}

   ///Registers a line observer. If any MCoeControlObserver was set
   ///before the first call to this function, that observer is kept
   ///around, and all events will be passed along to it.
   ///@param aControl id for the control on the line that is to be observed. 
   ///@param aObserver pointer to the observer. 
   void RegisterControlObserver(TInt aControl, 
                                class MDialogLineObserver* aObserver);
};
#endif
