/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CALLBACKDIALOG_H
#define CALLBACKDIALOG_H
#include <e32def.h>
#include <e32std.h>
#include <coemain.h>
#include <aknmessagequerydialog.h>
#include <aknselectionlist.h>
#include <aknsfld.h>
#include <aknutils.h>
#include <coecntrl.h>

struct no_op { void operator()(TInt){} };

///Template callback structor for use with CEventGenerator.
///This callback functor doesn't require it's EventHandler to actually
///use CEventGenerator, but was written for that purpose.
///A call to operator()(TInt) on an object of this class will, under
///certain circumstances, result in a call to
///EventGenerator::GenerateEvent. The argument to this call will be
///whatever the TCallBackEvent object received in its constructor.
///@param EventHandler the type for the callback. Must have a member
///                    function <code>GenerateEvent</code>. The
///                    function may have any return type
///                    (TCallBackEvent wont use it anyway), but must
///                    be callable with one argument that is
///                    convertible from EventType.
///@param EventType the argument type used in calls to 
///                 EventHandler::GenerateEvent
template<class EventHandler, typename EventType>
class TCallBackEvent{
   ///Flags. ORed values from the flag enums of this class.
   TInt iFlags;
   ///Pointer to the callback class. 
   EventHandler* iHandler;
   ///Event reported if operator() is called with EEikBidOk
   EventType iSuccess;
   ///Event reported if operator() is called with EEikBidCancel
   EventType iUnsuccess;
public:
   ///@name Flag enums.
   //@{
   ///Flags for deciding what buttons that should generate events. 
   enum TReportMode{
      ///Generate events when operator() is called with EEikBidCancel
      EReportCancel = 1
   };
   //@}
   ///@name Constructors
   //@{
   ///Constructor. TCallBackEvents constructed with this constructor
   ///will not report EEikBidCancel.
   ///@param aHandler pointer to the EventHandler class. 
   ///@param aSuccessfullEvent the value that will be used to report EEikBidOk.
   TCallBackEvent(EventHandler* aHandler, EventType aSuccessfullEvent) : 
      iFlags(0), iHandler(aHandler), iSuccess(aSuccessfullEvent)
   {}
   ///Constructor. TCallBackEvents constructed with this constructor
   ///will report EEikBidCancel.
   ///@param aHandler pointer to the EventHandler class. 
   ///@param aSuccessfullEvent the value that will be used to report EEikBidOk.
   ///@param aUnuccessfullEvent the value that will be used to report 
   ///                          EEikBidCancel.
   TCallBackEvent(EventHandler* aHandler, 
                  EventType aSuccessfullEvent, EventType aUnsuccessfullEvent) :
      iFlags(EReportCancel), iHandler(aHandler), iSuccess(aSuccessfullEvent), 
      iUnsuccess(aUnsuccessfullEvent)
   {}
   //@}

   ///Operator(). When called will, depending on iFlags and aButton,
   ///call EventHandler::GenerateEvent.
   ///@param aButton the button pressed in the CCallBackDialog.
   void operator()(TInt aButton)
   {
      if(aButton == EEikBidOk){
         iHandler->GenerateEvent(iSuccess);
      } else if(aButton == EEikBidCancel){
         iHandler->GenerateEvent(iUnsuccess);
      }
   }
};

///This is a tricky extension to the normal CLongTextDialog.  The
///trick lies in the templated callback type.  The callback type can
///be anything that is callable with a TInt as argument, including
///those which have an implicit conversion from TInt to the actual
///argument type.
///
///An important thing to notice is the fact that the CallBack function
///call may leave, but that will cause the framework to consider
///OkToExitL as failed and not close the dialog.
///
///The dialog takes owenership of the strings sent to it.
template<class CallBack>
class CCallBackDialog : public CAknMessageQueryDialog
{
   CallBack iCallBack;

   CCallBackDialog(TDesC* aTitle, TDesC* aText, CallBack aCallBack) : 
      CAknMessageQueryDialog(aText, aTitle), iCallBack(aCallBack)
   {
   }


   static CCallBackDialog<CallBack>* NewL(CallBack aCallBack, 
                                          HBufC* aTitle, 
                                          HBufC* aText)
   {
      CCallBackDialog<CallBack>* self = 
         new (ELeave) CCallBackDialog(aTitle, aText, aCallBack);
      return self;
   }

   static CCallBackDialog<CallBack>* NewL(CallBack aCallBack, 
                                          HBufC* aTitle, 
                                          TInt aText)
   {
      HBufC* text = CCoeEnv::Static()->AllocReadResourceLC( aText );
      CCallBackDialog<CallBack>* self = 
         new (ELeave) CCallBackDialog(aTitle, text, aCallBack);
      CleanupStack::Pop(text);
      return self;
   }

   static CCallBackDialog<CallBack>* NewL(CallBack aCallBack, 
                                          TInt aTitle, 
                                          const TDesC& aText)
   {
      HBufC* title = CCoeEnv::Static()->AllocReadResourceLC( aTitle );
      CCallBackDialog<CallBack>* self = 
         new (ELeave) CCallBackDialog(title, const_cast<TDesC*>(&aText), 
                                      aCallBack);
      CleanupStack::Pop(title);
      return self;
   }

   static CCallBackDialog<CallBack>* NewL(CallBack aCallBack, 
                                          TInt aTitle, 
                                          HBufC* aText)
   {
      HBufC* title = CCoeEnv::Static()->AllocReadResourceLC( aTitle );
      CCallBackDialog<CallBack>* self = 
         new (ELeave) CCallBackDialog(title, aText, aCallBack);
      CleanupStack::Pop(title);
      return self;
   }

   static CCallBackDialog<CallBack>* NewL(CallBack aCallBack,
         TInt aTitle, 
         TInt aText)
   {
      HBufC* text = CCoeEnv::Static()->AllocReadResourceLC( aText );
      CCallBackDialog<CallBack>* self = CCallBackDialog::NewL(aCallBack, 
                                                              aTitle, *text);
      CleanupStack::Pop(text);
      return self;
   }

public:

   static TInt RunDlgLD(CallBack aCallBack, TInt aTitle, TInt aText, 
                        TInt aResourceId)
   {
      CCallBackDialog<CallBack>* self = 
         CCallBackDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   static TInt RunDlgLD(CallBack aCallBack, TInt aTitle, const TDesC& aText, 
                        TInt aResourceId)
   {
      CCallBackDialog<CallBack>* self = 
         CCallBackDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   static TInt RunDlgLD(CallBack aCallBack, TInt aTitle, HBufC* aText, 
                        TInt aResourceId)
   {
      CCallBackDialog<CallBack>* self = 
         CCallBackDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   static TInt RunDlgLD(CallBack aCallBack, HBufC* aTitle, HBufC* aText, 
                        TInt aResourceId)
   {
      CCallBackDialog<CallBack>* self = 
         CCallBackDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   static TInt RunDlgLD(CallBack aCallBack, HBufC* aTitle, TInt aText, 
                        TInt aResourceId)
   {
      CCallBackDialog<CallBack>* self = 
         CCallBackDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   virtual TBool OkToExitL(TInt aButton)
   {
      if(aButton != EWFEmpty && aButton != 0){
         iCallBack(aButton);
         return ETrue;
      }
      return EFalse;
   }

};

/**
 * It is NOT possible to use a HBufC with this callback if 
 * the resource does not have EEikDialogFlagWait set.
 * To use a non-blocking resource you have to use TBuf 
 * to be able to get the value from the dialog.
 */
template<class CallBack>
class CCallBackEditDialog : public CAknTextQueryDialog
{
   CallBack iCallBack;

   CCallBackEditDialog(TDes& aText, CallBack aCallBack) : 
      CAknTextQueryDialog(aText, ENoTone),
      iCallBack(aCallBack)
   {
   }

public:

   static CCallBackEditDialog<CallBack>* NewL(CallBack aCallBack, 
         TInt aTitle, 
         TDes& aText)
   {
      HBufC* title = CCoeEnv::Static()->AllocReadResourceLC( aTitle );
      CCallBackEditDialog<CallBack>* self = 
         new (ELeave) CCallBackEditDialog(aText, aCallBack);

      self->SetPromptL(*title);
      CleanupStack::PopAndDestroy(title);
      return self;
   }

   static TInt RunDlgLD(CallBack aCallBack, TInt aTitle, TDes& aText, 
                        TInt aResourceId)
   {
      CCallBackEditDialog<CallBack>* self = 
         CCallBackEditDialog<CallBack>::NewL(aCallBack, aTitle, aText);
      return self->CEikDialog::ExecuteLD(aResourceId);
   }

   virtual TBool OkToExitL(TInt aButton)
   {
      if(aButton != EWFEmpty && aButton != 0){
         CAknTextQueryDialog::OkToExitL(aButton);
         iCallBack(aButton);
         return ETrue;
      }
      return EFalse;
   }

};

/**
 * List query dialog. There are four existing resources that 
 * could be used with this class:
 * r_wayfinder_callback_single_list_query, single line.
 * r_wayfinder_callback_double_list_query, double line.
 * r_wayfinder_callback_single_icon_list_query, single line with icons.
 * r_wayfinder_callback_double_icon_list_query, double line with icons.
 */
template<class CallBack>
class CCallBackListDialog : public CAknListQueryDialog
{
   CallBack iCallBack;

   CCallBackListDialog(TInt& aIndex, CallBack aCallBack) : 
      CAknListQueryDialog(&aIndex),
      iCallBack(aCallBack)
   {
   }

   static CCallBackListDialog<CallBack>* NewPrepareLC(CallBack aCallBack, 
                                                      TInt& aIndex,
                                                      TInt aTitle,
                                                      TInt aResourceId)
   {
      TBuf<256> title;
      if (aTitle > 0) {
         CCoeEnv::Static()->ReadResource(title,  aTitle);
      } else {
         _LIT(KEmpty, "");
         title.Copy(KEmpty);
      }
      return NewPrepareLC(aCallBack, aIndex, title, aResourceId);
   }

   static CCallBackListDialog<CallBack>* NewPrepareLC(CallBack aCallBack, 
                                                      TInt& aIndex,
                                                      HBufC* aTitle,
                                                      TInt aResourceId)
   {
      if (!aTitle) {
         _LIT(KEmpty, "");
         aTitle = KEmpty().AllocL();
      }
      CCallBackListDialog* self = NewPrepareLC(aCallBack, aIndex, *aTitle, aResourceId);
      delete aTitle;
      return self;
   }

   static CCallBackListDialog<CallBack>* NewPrepareLC(CallBack aCallBack, 
                                                      TInt& aIndex,
                                                      const TDesC& aTitle,
                                                      TInt aResourceId)
   {
      CCallBackListDialog<CallBack>* self = 
         new (ELeave) CCallBackListDialog(aIndex, aCallBack);
      self->PrepareLC(aResourceId);
      self->QueryHeading()->SetTextL(aTitle);
      return self;
   }

   void InitiateArray(const MDesCArray* aItemTextArray, 
                      TBool aExternalOwnership,
                      TInt aPreSelectedIndex,
                      CArrayPtr<CGulIcon>* aIconList)
   {
      SetItemTextArray(const_cast<MDesCArray*>(aItemTextArray));
      if (aIconList) {
         SetIconArrayL(aIconList);
      }
      SetOwnershipType(aExternalOwnership ? ELbmDoesNotOwnItemArray : ELbmOwnsItemArray);
      if (aPreSelectedIndex < aItemTextArray->MdcaCount()) {
         ListBox()->SetCurrentItemIndex(aPreSelectedIndex);
      }
//       ListBox()->ItemDrawer()->ColumnData()->SetSkinEnabledL(ETrue)
   }


public:

   static CCallBackListDialog<CallBack>* NewL(CallBack aCallBack, 
                                              TInt& aIndex,
                                              TInt aTitle, 
                                              const MDesCArray* aItemTextArray,
                                              TInt aResourceId,
                                              TInt aPreSelectedIndex)
   {
      CCallBackListDialog<CallBack>* self = 
         NewPrepareLC(aCallBack, aIndex, aTitle, aResourceId);
      self->InitiateArray(aItemTextArray, ETrue, aPreSelectedIndex);
      return self;
   }


   static CCallBackListDialog<CallBack>* NewL(CallBack aCallBack, 
                                              TInt& aIndex,
                                              TInt aTitle, 
                                              MDesCArray* aItemTextArray,
                                              TInt aResourceId,
                                              TBool aExternalOwnership,
                                              TInt aPreSelectedIndex,
                                              CArrayPtr<CGulIcon>* aIconList)
   {
      CCallBackListDialog<CallBack>* self = 
         NewPrepareLC(aCallBack, aIndex, aTitle, aResourceId);
      self->InitiateArray(aItemTextArray, aExternalOwnership, 
                          aPreSelectedIndex, aIconList);
      return self;
   }
   
   static CCallBackListDialog<CallBack>* NewL(CallBack aCallBack, 
                                              TInt& aIndex,
                                              const TDesC& aTitle, 
                                              MDesCArray* aItemTextArray,
                                              TInt aResourceId,
                                              TBool aExternalOwnership,
                                              TInt aPreSelectedIndex,
                                              CArrayPtr<CGulIcon>* aIconList)
   {
      CCallBackListDialog<CallBack>* self = 
         NewPrepareLC(aCallBack, aIndex, aTitle, aResourceId);
      self->InitiateArray(aItemTextArray, aExternalOwnership, 
                          aPreSelectedIndex, aIconList);
      return self;
   }
   
   static CCallBackListDialog<CallBack>* NewL(CallBack aCallBack, 
                                              TInt& aIndex,
                                              HBufC* aTitle, 
                                              MDesCArray* aItemTextArray,
                                              TInt aResourceId,
                                              TBool aExternalOwnership,
                                              TInt aPreSelectedIndex,
                                              CArrayPtr<CGulIcon>* aIconList)
   {
      CCallBackListDialog<CallBack>* self = 
         NewPrepareLC(aCallBack, aIndex, aTitle, aResourceId);
      self->InitiateArray(aItemTextArray, aExternalOwnership, 
                          aPreSelectedIndex, aIconList);
      return self;
   }

   template<typename TitleType>
   static TInt RunDlgLD(CallBack aCallBack, 
                        TInt& aIndex,
                        TitleType const &aTitle, 
                        MDesCArray* aItemTextArray,
                        TInt aResourceId,
                        TBool aExternalOwnership = ETrue,
                        TInt aPreSelectedIndex = 0,
                        CArrayPtr<CGulIcon>* aIconList = NULL)
   {
      CCallBackListDialog<CallBack>* self = 
         CCallBackListDialog<CallBack>::NewL(aCallBack, 
                                             aIndex, 
                                             aTitle, 
                                             aItemTextArray, 
                                             aResourceId, 
                                             aExternalOwnership,
                                             aPreSelectedIndex,
                                             aIconList);
      return self->RunLD();
   }

   static TInt RunDlgLD(CallBack aCallBack, 
                        TInt& aIndex,
                        TInt aTitle, 
                        const MDesCArray* aItemTextArray, 
                        TInt aResourceId, 
                        TInt aPreSelectedIndex = 0)
   {
      CCallBackListDialog<CallBack>* self = 
         CCallBackListDialog<CallBack>::NewL(aCallBack, 
                                             aIndex, 
                                             aTitle, 
                                             aItemTextArray, 
                                             aResourceId, 
                                             aPreSelectedIndex);
      return self->RunLD();
   }


   virtual TBool OkToExitL(TInt aButton)
   {
      if(aButton != EWFEmpty && aButton != 0){
         CAknListQueryDialog::OkToExitL(aButton);
         iCallBack(aButton);
         return ETrue;
      }
      return EFalse;
   }
};


/**
 * Selection List query dialog (with edwin in bottom). 
 * There one existing genereal resource.
 * This dialog never takes ownership of array.
 * could be used with this class:
 * r_wayfinder_callback_select_list_query.
 */
template<class CallBack>
class CCallBackSelectionListDialog : public CAknSelectionListDialog
{
   CallBack iCallBack;
   TInt iPreSelectedIndex;
   TBool iExitKeyPressed;
   TBool iEditKeyActive;
   CCallBackSelectionListDialog(CallBack aCallBack, 
                                TInt& aIndex, 
                                MDesCArray* aItemTextArray,
                                TInt aPreSelectedIndex) :
      CAknSelectionListDialog(aIndex, aItemTextArray, NULL),
      iCallBack(aCallBack),
      iPreSelectedIndex(aPreSelectedIndex),
      iExitKeyPressed(EFalse),
      iEditKeyActive(EFalse)
   {
   }

public:

   static CCallBackSelectionListDialog<CallBack>* NewL(CallBack aCallBack, 
                                                       TInt& aIndex,
                                                       MDesCArray* aItemTextArray,
                                                       TInt aPreSelectedIndex)
   {
      CCallBackSelectionListDialog<CallBack>* self = 
         new (ELeave) CCallBackSelectionListDialog(aCallBack, 
                                                   aIndex, 
                                                   aItemTextArray,
                                                   aPreSelectedIndex);
      return self;
   }

   static TInt RunDlgLD(CallBack aCallBack, 
                        TInt& aIndex,
                        MDesCArray* aItemTextArray, 
                        TInt aResourceId,
                        TInt aPreSelectedIndex = 0)
   {
      CCallBackSelectionListDialog<CallBack>* self = 
         CCallBackSelectionListDialog<CallBack>::NewL(aCallBack, 
                                                      aIndex,
                                                      aItemTextArray, 
                                                      aPreSelectedIndex);
      return self->ExecuteLD(aResourceId);
   }

   virtual void PostLayoutDynInitL()
   {
      CAknSearchField* findBox = this->FindBox();
      CEikListBox* listBox = this->ListBox();
      if (listBox) {
         if (iPreSelectedIndex < listBox->Model()->NumberOfItems()) {
            listBox->SetCurrentItemIndex(iPreSelectedIndex);
         }
      }
      if (findBox) {
         findBox->MakeVisible(ETrue);
         findBox->SetFocus(ETrue);
      }
   }

   //This is needed to always keep the edwin visible in the dialog 
   //and to be able to exit the dialog with one back key press.
   virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType) 
   {
      if( (aKeyEvent.iScanCode == EStdKeyLeftShift || 
           aKeyEvent.iScanCode == EStdKeyRightShift) ){
         iEditKeyActive = ETrue;
         TKeyResponse ret = CCoeControl::OfferKeyEventL(aKeyEvent, aType);
         return ret;
      } else if ((aKeyEvent.iModifiers & EModifierShift) || iEditKeyActive) {
         this->ListBox()->SetFocus(EFalse);
         if ((aKeyEvent.iScanCode == EStdKeyDevice0) || 
             (aKeyEvent.iScanCode == EStdKeyDevice1)) {
            iEditKeyActive = EFalse;
            SetFocus(ETrue);
         } else if (aKeyEvent.iScanCode == EStdKeyDevice3) {
            if (aType == EEventKeyUp) {
               iEditKeyActive = EFalse;
            }
            SetFocus(ETrue);
            TKeyResponse ret = CCoeControl::OfferKeyEventL(aKeyEvent, aType);
            return ret;
         }
         return CCoeControl::OfferKeyEventL(aKeyEvent, aType);
      } else {
         if(aKeyEvent.iScanCode == EStdKeyDevice3 || 
            aKeyEvent.iScanCode == EStdKeyDevice0) {
            iEditKeyActive = EFalse;
         }
         if ((aType == EEventKeyDown) && (aKeyEvent.iScanCode == EStdKeyDevice1)) {
            iExitKeyPressed = ETrue;
            return EKeyWasConsumed;
         }
         if ((aType == EEventKeyUp) && 
             (aKeyEvent.iScanCode == EStdKeyDevice1) && iExitKeyPressed) {
            iExitKeyPressed = EFalse;
            TryExitL(EEikBidCancel);
            return EKeyWasConsumed;
         }
         if (this->ListBox()->IsFocused() && aType != EEventKey) {
            return CAknSelectionListDialog::OfferKeyEventL(aKeyEvent, aType);
         }
         if (this->IsFocused()) { 
            TBool needRefresh = ETrue;
            if (AknFind::HandleFindOfferKeyEventL(aKeyEvent, aType, this,
                                                  this->ListBox(), this->FindBox(),
                                                  EFalse, //Make edwin permanent.
                                                  needRefresh) == EKeyWasConsumed) {
               return EKeyWasConsumed;
            }
         }
         if(this->ListBox()->IsFocused()) {
            return CAknSelectionListDialog::OfferKeyEventL(aKeyEvent, aType);
         }
         return EKeyWasNotConsumed;
      }
   }

   virtual TBool OkToExitL(TInt aButton) 
   {
      if (iEditKeyActive) {
         iEditKeyActive = EFalse;
         this->ListBox()->SetFocus(ETrue);
      } else if (!iEditKeyActive && aButton != EWFEmpty && aButton != 0) {
         CAknSelectionListDialog::OkToExitL(aButton);
         iCallBack(aButton);
         return ETrue;
      }
      return EFalse;
   }
};


///Convenience function for a callback dialog which generates an event
///only when completed successfully.
///@param aHandler pointer to the object whos HandleEventL function
///                will be called when the dialog is closed.
///@param aEvent   the event value that will be sent as an argument to
///                HandleEventL when the dialog is closed with EEikBidOk.
///@param aHeader  the resource id that identifies the text used as a
///                header for the dialog.
///@param aText    the resource id that identifies the text used as
///                dialog content.
///@param aDialog  the resource id that identifies a specific dialog
///                configuration.
template<class H, typename E>
void RunCallbackDialogL(H* aHandler, const E& aEvent, TInt aHeader, TInt aText,
                        TInt aDialog)
{
   typedef TCallBackEvent<H, E> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(aHandler, aEvent), aHeader, aText, aDialog);
}

///Convenience function for a callback dialog which generates an event
///when completed.
///@param aHandler pointer to the object whos HandleEventL function
///                will be called when the dialog is closed.
///@param aEventOk the event value that will be sent as an argument to
///                HandleEventL when the dialog is closed with EEikBidOk.
///@param aEventCancel the event value that will be sent as an argument to
///                    HandleEventL when the dialog is closed with
///                    EEikBidCancel.
///@param aHeader  the resource id that identifies the text used as a
///                header for the dialog.
///@param aText    the resource id that identifies the text used as
///                dialog content.
///@param aDialog  the resource id that identifies a specific dialog
///                configuration.
template<class H, typename E>
void RunCallbackDialogL(H* aHandler, const E& aEventOk, const E& aEventCancel, 
                        TInt aHeader, TInt aText, TInt aDialog)
{
   typedef TCallBackEvent<H, E> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(aHandler, aEventOk, aEventCancel), 
                   aHeader, aText, aDialog);
}


#endif
