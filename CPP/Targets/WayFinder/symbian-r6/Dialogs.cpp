/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Dialogs.h"
#include <aknnotewrappers.h>
#include <aknstaticnotedialog.h>
#include <aknmessagequerydialog.h>

#include "RsgInclude.h"
#include "WayFinderConstants.h"
#include "WFTextUtil.h"

//forward declaration. Defined in WayFinderAppUi.cpp
TBool IsReleaseVersion();

namespace WFDialog{
   namespace {
      /**This is the refactored implementation function of all the
       * ShowXXDialogL functions.
       * @param aDialogTypeId the resource id of the dialog type. This
       *                      defines whether it will be a error-, 
       *                      warning-, or some othe kind of dialog.
       * @param aTone this enum value determines what kind of tone, if 
       *              any, should be played when the dialog opens.
       * @param aText the text to display in the dialog.
       */
      void ShowDialogL(TInt aDialogTypeId, enum CAknNoteDialog::TTone aTone,
                       const TDesC& aText, CAknNoteDialog::TTimeout aTimeout = CAknNoteDialog::ENoTimeout)
      {       
         
         CAknStaticNoteDialog* dlg = new (ELeave) CAknStaticNoteDialog;
         CleanupStack::PushL(dlg);
         dlg->PrepareLC(aDialogTypeId);
         dlg->SetNumberOfBorders(1);
         dlg->SetTone(aTone);
         dlg->SetTimeout(aTimeout);
         dlg->SetTextL(aText);
         CleanupStack::Pop(dlg);
         dlg->RunLD();
      }

     /* The diffeence between this method and the above is that this one is used for
      *  several options in the dialog and returns the ID of the button pressed
      */
      // TInt ShowMultipleOptionsDialogL(TInt aDialogTypeId, enum CAknNoteDialog::TTone aTone,
      //                                 const TDesC& aText, MCoeControlObserver* aObserver, CAknNoteDialog::TTimeout aTimeout = /*(CAknNoteDialog::TTimeout)7000000*/CAknNoteDialog::ENoTimeout)
      // {
      //   //TBuf<128> buf.Copy(aText);
      //   //        iRouteDlg = CRoamingQueryDialog::NewL(buf);
  
      //   // CAknStaticNoteDialog* dlg = new (ELeave) CAknStaticNoteDialog;
      //   // CleanupStack::PushL(dlg);
      //   // //dlg->SetObserver(aObserver);
      //   // dlg->PrepareLC(aDialogTypeId);
      //   // dlg->SetNumberOfBorders(1);
      //   // dlg->SetTone(aTone);
      //   // dlg->SetTimeout(aTimeout);
      //   // dlg->SetTextL(aText);
      //   // CleanupStack::Pop(dlg);
      //   // return dlg->RunLD();
      //   return 0
      // }
   }
   //****************ShowErrorDialogL******************
   void ShowErrorDialogL( const TDesC &aText )
   {
      ShowDialogL(R_WAYFINDER_ERROR_DIALOG, CAknNoteDialog::EErrorTone, aText);
   }

   void ShowErrorDialogL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowErrorDialogL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

   //*****************ShowWarningDialogL********************
   void ShowWarningDialogL( const TDesC &aText )
   {
      ShowDialogL(R_WAYFINDER_WARNING_DIALOG, CAknNoteDialog::EWarningTone,
                  aText);
   }

   void ShowWarningDialogL( TInt aResourceId, class CCoeEnv* coeEnv)
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowErrorDialogL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

  //****************ShowRoaming Dialog **********************
  // TInt ShowRoamingDialogL(TInt aResourceId, class CCoeEnv* coeEnv, MCoeControlObserver* aObserver)
  // {
  //   HBufC* buf;
  //   buf = coeEnv->AllocReadResourceLC(aResourceId);
  //   TInt res = ShowRoamingDialogL(*buf, aObserver);
  //   CleanupStack::PopAndDestroy(buf);
  //   return res;
  // }

  // TInt ShowRoamingDialogL(const TDesC& aText, MCoeControlObserver* aObserver)
  // {
  //   return ShowMultipleOptionsDialogL(R_WAYFINDER_ROAMING_DIALOG, CAknNoteDialog::ENoTone, aText, aObserver);
  // }
  
  //****************ShowInfoDialogL******************
   void ShowInfoDialogL( const TDesC &aText, TBool aAutoClose )
   {
      if(aAutoClose) {
         ShowDialogL(R_WAYFINDER_INFO_DIALOG, CAknNoteDialog::ENoTone, aText, (CAknNoteDialog::TTimeout)7000000);
      }
      else {
         ShowDialogL(R_WAYFINDER_INFO_DIALOG, CAknNoteDialog::ENoTone, aText);
      }
   }
   void ShowInfoDialogL( TInt aResourceId, class CCoeEnv* coeEnv, TBool aAutoClose)
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowInfoDialogL( *buf , aAutoClose);
      CleanupStack::PopAndDestroy(buf);
   }

   //***************ShowDebugDialogL**********************
   void ShowDebugDialogL( const char *str )
   {
      if(!IsReleaseVersion()){
         HBufC* tmp;
         tmp = HBufC::NewLC(strlen(str)+1);
         WFTextUtil::char2HBufC(tmp, str);
         ShowQueryL(*tmp);
         CleanupStack::PopAndDestroy(tmp);
      }
   }

   void
   ShowDebugDialogL( const TDesC& aText )
   {
      if (!IsReleaseVersion()) {
         ShowQueryL(aText);
      }
   }
   //***************ShowScrollingDialogL********************
   void ShowScrollingDialogL(class CCoeEnv* coeEnv, 
                             TInt aHeader, TInt aText, TBool aTone,
                             TInt32 aBitmap, TInt32 aMask)
   {
      HBufC* header = coeEnv->AllocReadResourceLC( aHeader );
      HBufC* text = coeEnv->AllocReadResourceLC( aText );
      ShowScrollingDialogL(*header, *text, aTone, aBitmap, aMask);
      CleanupStack::PopAndDestroy(2, header);
   }

   void ShowScrollingDialogL( const TDesC &aHeader, const TDesC &aText,
                              TBool aTone, TInt32 /*aBitmap*/, TInt32 /*aMask*/)
   {
      CAknMessageQueryDialog* dlg;
      CAknMessageQueryDialog::TTone tone = CAknQueryDialog::ENoTone;
      if(aTone){
         tone = CAknQueryDialog::EWarningTone;
      }
      dlg = CAknMessageQueryDialog::NewL(const_cast<TDesC&>(aText), tone);
      CleanupStack::PushL(dlg); //autodestroy below
      dlg->PrepareLC( R_MESSAGE_DIALOG );
      CAknPopupHeadingPane *header = dlg->QueryHeading();
      header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
      header->SetTextL( aHeader );

      CleanupStack::Pop(dlg);
      dlg->RunLD();
   }

   TBool ShowScrollingQueryL(class CCoeEnv* coeEnv, 
                             TInt aHeader, TInt aText, TBool aTone,
                             TInt32 aBitmap, TInt32 aMask)
   {
      HBufC* header = coeEnv->AllocReadResourceLC( aHeader );
      HBufC* text = coeEnv->AllocReadResourceLC( aText );
      TBool res = ShowScrollingQueryL(*header, *text, aTone, aBitmap, aMask);
      CleanupStack::PopAndDestroy(2, header);
      return res;
   }

   TBool ShowScrollingQueryL(TInt aResourceId,
                            const TDesC &aHeader, const TDesC &aText,
                            TBool aTone)
   {
      CAknMessageQueryDialog* dlg;
      CAknMessageQueryDialog::TTone tone = CAknQueryDialog::ENoTone;
      if(aTone){
         tone = CAknQueryDialog::EWarningTone;
      }
      dlg = CAknMessageQueryDialog::NewL(const_cast<TDesC&>(aText), tone);
      CleanupStack::PushL(dlg); //autodestroy below
      dlg->PrepareLC( aResourceId );
      CAknPopupHeadingPane *header = dlg->QueryHeading();
      header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
      header->SetTextL( aHeader );

      CleanupStack::Pop(dlg);
      return (dlg->RunLD() == EAknSoftkeyYes);
   }

   TBool ShowScrollingQueryL(const TDesC &aHeader, const TDesC &aText,
                             TBool aTone, TInt32 /*aBitmap*/, TInt32 /*aMask*/)
   {
      CAknMessageQueryDialog* dlg;
      CAknMessageQueryDialog::TTone tone = CAknQueryDialog::ENoTone;
      if(aTone){
         tone = CAknQueryDialog::EWarningTone;
      }
      dlg = CAknMessageQueryDialog::NewL(const_cast<TDesC&>(aText), tone);
      CleanupStack::PushL(dlg); //autodestroy below
      dlg->PrepareLC( R_MESSAGE_QUERY );
      CAknPopupHeadingPane *header = dlg->QueryHeading();
      header->SetLayout(CAknPopupHeadingPane::EQueryHeadingPane );
      header->SetTextL( aHeader );

      CleanupStack::Pop(dlg);
      return (dlg->RunLD() == EAknSoftkeyYes);
   }

   //********************ShowScrollingWarningDialogL************************

   void ShowScrollingWarningDialogL(const TDesC &aText, class CCoeEnv* coeEnv )
   {
      TBuf<32> wrn;
      coeEnv->ReadResource( wrn, R_WAYFINDER_WARNING_MSG );
      ShowScrollingDialogL( wrn, aText, ETrue);
   }

   void ShowScrollingWarningDialogL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowScrollingWarningDialogL(*buf, coeEnv);
      CleanupStack::PopAndDestroy(buf);
   }

   //**********************ShowScrollingErrorDialogL***********************

   void
   ShowScrollingErrorDialogL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* message;
      message = coeEnv->AllocReadResourceLC( aResourceId );
      ShowScrollingErrorDialogL(*message);
      CleanupStack::PopAndDestroy(message);
   }

   void ShowScrollingErrorDialogL( const TDesC &aText, 
                                   class CCoeEnv* coeEnv )
   {
      TBuf<32> wrn;
      coeEnv->ReadResource( wrn, R_WAYFINDER_ERROR_MSG );
      ShowScrollingDialogL( wrn, aText, ETrue );
   }

   //*****ShowScrollingInfoDialogL*****
   void ShowScrollingInfoDialogL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* message;
      message = coeEnv->AllocReadResourceLC( aResourceId );
      ShowScrollingInfoDialogL(*message);
      CleanupStack::PopAndDestroy(message);
   }

   void ShowScrollingInfoDialogL( const TDesC &aText, class CCoeEnv* coeEnv )
   {
      TBuf<32> wrn;
      coeEnv->ReadResource( wrn, R_MAP_INFO_TITLE );
      ShowScrollingDialogL( wrn, aText, EFalse );
   }

   //******************ShowInformationL*************************
   void ShowInformationL( const TDesC &aText )
   {
      CAknInformationNote* note = new (ELeave) CAknInformationNote;
      note->ExecuteLD( aText );
   }

   void ShowInformationL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowInformationL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

   //********ShowConfirmationL ******************

   void ShowConfirmationL( TDesC const &aText )
   {
      CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
      note->ExecuteLD( aText );
   }

   void ShowConfirmationL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      WFDialog::ShowConfirmationL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

   //*************ShowWarningL****************************'
   void ShowWarningL( const TDesC &aText )
   {
      TBool aWaitingDialog = true; // needed for timeout to work.
      CAknWarningNote* note = new(ELeave) CAknWarningNote(aWaitingDialog);
      note->ExecuteLD( aText );
   }

   void ShowWarningL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowWarningL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

   //***********ShowErrorL*****************************

   void ShowErrorL( const TDesC &aText )
   {
      TBool aWaitingDialog = true; // needed for timeout to work.
      CAknErrorNote* note = new(ELeave) CAknErrorNote(aWaitingDialog);
      note->ExecuteLD( aText );
   }

   void ShowErrorL( TInt aResourceId, class CCoeEnv* coeEnv )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      ShowErrorL( *buf );
      CleanupStack::PopAndDestroy(buf);
   }

   //***************ShowQuery***************************
   TBool ShowQueryL( const TDesC &aText )
   {
      CAknQueryDialog* dlg = CAknQueryDialog::NewL();
      return TBool(dlg->ExecuteLD( R_WAYFINDER_QUERY_DIALOG, aText ) );
   }

   TBool ShowQueryL( TInt aResourceId, class CCoeEnv* coeEnv  )
   {
      HBufC* buf;
      buf = coeEnv->AllocReadResourceLC( aResourceId );
      TBool res = ShowQueryL( *buf );
      CleanupStack::PopAndDestroy(buf);
      return res;
   }
}
