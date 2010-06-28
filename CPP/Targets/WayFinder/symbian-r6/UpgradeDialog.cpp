/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "UpgradeDialog.h"

#include <eikcapc.h> 
#include <eikmenup.h>
#include <eikedwin.h> 
#include <avkon.rsg>

#include "RsgInclude.h"
#include "DataHolder.h"
#include "Dialogs.h"
#include "PopUpList.h"
#include "memlog.h"
#include "wficons.mbg"
#include "GuiDataStore.h"

// #include <aknwaitdialog.h>
// #include "wayfinder.hrh"
// #include "WayFinderAppUi.h"
// #include "WayFinderConstants.h"
// #include "WayFinderConstants.h"
// #include "Log.h"


// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------------------------
// CUpgradeDialog::NewL()
// Two-phased constructor.
// ----------------------------------------------------------------------------
//
class CUpgradeDialog* 
CUpgradeDialog::NewL(class CWayFinderAppUi* aAppUi,
                     class isab::Log* aLog,
                     enum CWayFinderAppUi::TUpgradeMode aMode )
{
   class CUpgradeDialog* self = new (ELeave) CUpgradeDialog( aLog );
   CleanupStack::PushL(self);
   self->ConstructL( aAppUi, aMode );
   CleanupStack::Pop();
   return self;
}
        

// Destructor
CUpgradeDialog::~CUpgradeDialog()
{
   DeleteBufs();
}

CUpgradeDialog::CUpgradeDialog(class isab::Log* aLog) : 
   iLog(aLog), iInProgress(EFalse)
{}


// ----------------------------------------------------------------------------
// CUpgradeDialog::ExecuteLD( TInt aResourceId )
// 
// ----------------------------------------------------------------------------
//
TInt CUpgradeDialog::ExecuteLD( TInt aResourceId )
{
   return CAknForm::ExecuteLD( aResourceId );
}

// ----------------------------------------------------------------------------
// CUpgradeDialog::PrepareLC( TInt aResourceId )
// 
// ----------------------------------------------------------------------------
//
void CUpgradeDialog::PrepareLC( TInt aResourceId )
{
   CAknForm::PrepareLC( aResourceId );

   iCountry = iCoeEnv->AllocReadResourceL(R_WAYFINDER_SELECT_COUNTRY_TEXT );
   SetCountry( *iCountry );
   iCountryId = MAX_INT32;
   delete iCountry;
   iCountry = NULL;

   class CEikEdwin* control = NULL;
   if(GetControl(control, EWayFinderCtrlUpgradeRegNumberEdwin)){
      control->SetTextLimit(24);
      control->AddEdwinObserverL( this );
      control->SetAknEditorCase(EAknEditorUpperCase);
   }

   if(GetControl(control, EWayFinderCtrlUpgradePhoneEdwin)){
      //init code for phone number edwin
      control->SetAknEditorInputMode(EAknEditorNumericInputMode);
      control->SetAknEditorSpecialCharacterTable(R_AVKON_SPECIAL_CHARACTER_TABLE_DIALOG);
   }
   
   if(GetControl(control, EWayFinderCtrlUpgradeCountryEdwin)){
      //init code for country edwin
      control->MakeVisible(EFalse);
      control->SetComponentsToInheritVisibility();
      SetLineNonFocusing( EWayFinderCtrlUpgradeCountryEdwin );
   }

   if(GetControl(control, EWayFinderCtrlUpgradeNameEdwin)){
      //init code for name control here
   }

   if(GetControl(control, EWayFinderCtrlUpgradeEmailEdwin)){
      //init code for email control here
      _LIT(KAt, "@");
      control->SetTextL(&KAt);
   }
}

// ---------------------------------------------------------
// CUpgradeDialog::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CUpgradeDialog::ConstructL(class CWayFinderAppUi* aAppUi, 
                                enum CWayFinderAppUi::TUpgradeMode aMode)
{
   iNewItemId = EWayFinderCtrlUpgradeRegNumberEdwin + 1;

   switch(aMode){
   case CWayFinderAppUi::EUpgrade:
      CAknForm::ConstructL( R_WAYFINDER_UPGRADE_MENUBAR );
      break;
   case CWayFinderAppUi::EActivate:
      CAknForm::ConstructL( R_WAYFINDER_REGISTER_MENUBAR );
      break;
   }
   iUpgradeMode = aMode;
   iAppUi = aAppUi;
   iHasFocus = ETrue;
}

void CUpgradeDialog::Close()
{
   iInProgress = EFalse;
   ButtonCommandObserver()->ProcessCommandL( EAknSoftkeyBack );
}

void CUpgradeDialog::SetEdwinTextIfPresentL(TInt aId, const TDesC& aText)
{
   if(class CEikEdwin* edwin = GetControl(edwin, aId)){
      edwin->SetTextL(&aText);
   }
}

HBufC* CUpgradeDialog::GetEdwinTextIfPresentL(TInt aId)
{
   HBufC* text = NULL;
   if(class CEikEdwin* edwin = GetControl(edwin, aId)){
      text = edwin->GetTextInHBufL();
   }
   return text;
}

void CUpgradeDialog::SetCountry(const TDesC &aCountry )
{
   SetEdwinTextIfPresentL(EWayFinderCtrlUpgradeCountryEdwin, aCountry);
}

void CUpgradeDialog::SetPhone(const TDesC &aPhone )
{
   SetEdwinTextIfPresentL(EWayFinderCtrlUpgradePhoneEdwin, aPhone);
}

void CUpgradeDialog::SetRegNumber(const TDesC &aRegNumber )
{
   SetEdwinTextIfPresentL( EWayFinderCtrlUpgradeRegNumberEdwin, aRegNumber );
}

void CUpgradeDialog::SetName(const TDesC &aName )
{
   SetEdwinTextIfPresentL( EWayFinderCtrlUpgradeNameEdwin, aName );
}

void CUpgradeDialog::SetEmail(const TDesC &aEmail )
{
   SetEdwinTextIfPresentL( EWayFinderCtrlUpgradeEmailEdwin, aEmail );
}

void CUpgradeDialog::SelectCountryL()
{
   class CDataHolder* dataHolder = iAppUi->GetDataHolder();

   CDesCArray* descArray = new (ELeave) CDesCArrayFlat(5);
   LOGNEW(descArray, CDesCArrayFlat(5));
   CleanupStack::PushL( descArray );

   if( dataHolder->GetNbrCountries() > 0 ){
      for( TInt i=0; i < dataHolder->GetNbrCountries(); i++ ){
         descArray->AppendL( *(dataHolder->GetCountry(i)) );
      }
   }

   TBool okChoosen = EFalse;
   TInt countryIdx = MAX_INT32;
   countryIdx = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL,
                                          *descArray, okChoosen, countryIdx );
   if( okChoosen ){
      SetCountry( *(dataHolder->GetCountry( countryIdx )) );
      iCountryId = dataHolder->GetCountryId( countryIdx );
   }
   CleanupStack::PopAndDestroy(descArray);
}

void CUpgradeDialog::HandleEdwinEventL(class CEikEdwin* /*aEdwin*/, 
                                       enum TEdwinEvent /*aEventType*/ )
{
}

enum TKeyResponse CUpgradeDialog::OfferKeyEventL(const struct TKeyEvent& aKeyEvent,
                                                 enum TEventCode aType)
{
   if ( aType != EEventKey ){ // Is key up or down
      return CAknForm::OfferKeyEventL(aKeyEvent, aType);
   }

   // Use the arrow keys to move focus between the controls.
   switch (aKeyEvent.iScanCode){
   case EStdKeyDevice3:
      if(iHasFocus && IdOfFocusControl() == EWayFinderCtrlUpgradeCountryEdwin){
         SelectCountryL();
         return EKeyWasConsumed;
      }
      return CAknForm::OfferKeyEventL( aKeyEvent, aType );
      break;
   case EStdKeyDownArrow:
      CAknForm::OfferKeyEventL( aKeyEvent, aType );
      if( IdOfFocusControl() == EWayFinderCtrlUpgradePhoneEdwin ){
         TBuf<KBuf64Length> tmp;
         GetEdwinText( tmp, EWayFinderCtrlUpgradePhoneEdwin );
         if( tmp.Length() <= 1 ){
            TBuf<8> plus( _L("+") );
            SetPhone( plus );
            TKeyEvent keyEvent = aKeyEvent;
            keyEvent.iScanCode = EStdKeyRightArrow;
            keyEvent.iCode = EKeyRightArrow;
            CAknForm::OfferKeyEventL( keyEvent, aType );
         }
      }
      return EKeyWasConsumed;
      break;
   case EStdKeyUpArrow:
   case EStdKeyLeftArrow:
   case EStdKeyRightArrow:
   case EStdKeyBackspace:
   case EStdKeyDevice1:
      return CAknForm::OfferKeyEventL( aKeyEvent, aType );
      break;
   default:
      if( IdOfFocusControl() == EWayFinderCtrlUpgradeRegNumberEdwin ){
         CAknForm::OfferKeyEventL( aKeyEvent, aType );
         return EKeyWasConsumed;
      } else {
         return CAknForm::OfferKeyEventL( aKeyEvent, aType );
      }
      break;
   }
   return EKeyWasNotConsumed;
}

// ----------------------------------------------------------------------------
// CUpgradeDialog::PreLayoutDynInitL()
// Load the correct bitmap.
// ----------------------------------------------------------------------------
void CUpgradeDialog::PreLayoutDynInitL()
{
   const TDesC* mbmfile = iAppUi->GetGuiDataStore()->iWayfinderMBMFilename;
   if(iUpgradeMode == CWayFinderAppUi::EUpgrade && 
      ControlOrNull(EWayFinderCtrlUpgradePhoneEdwin)){
         DeleteLine(EWayFinderCtrlUpgradePhoneEdwin);
   }
   if(mbmfile){
      const TInt bitmap = EMbmWficonsRed_star;
#ifdef NAV2_CLIENT_SERIES60_V3
      const TInt mask   = EMbmWficonsRed_star_mask;
#else
      const TInt mask   = EMbmWficonsRed_star_m;
#endif
      const TInt lines[] = { EWayFinderCtrlUpgradeRegNumberEdwin, 
                             EWayFinderCtrlUpgradePhoneEdwin, 
                             EWayFinderCtrlUpgradeEmailEdwin, 
                             EWayFinderCtrlUpgradeNameEdwin };
      for(size_t i = 0; i < sizeof(lines)/sizeof(*lines); ++i){
         if(ControlOrNull(lines[i])){
            class CEikCaptionedControl* line = Line(lines[i]);
            line->SetBitmapFromFileL(*mbmfile, bitmap, mask);
         }
      }
   }
}

// ----------------------------------------------------------------------------
// CUpgradeDialog::PostLayoutDynInitL()
// Set default field value to member data.
// ----------------------------------------------------------------------------
//
void CUpgradeDialog::PostLayoutDynInitL()
{
   CAknForm::PostLayoutDynInitL();
   if(class CEikEdwin* control = GetControl(control, 
                                            EWayFinderCtrlUpgradePhoneEdwin)){
      _LIT(KPlus, "+");
      SetPhone(KPlus);
   }
   
   // To Following line change the edit mode
   //SetEditableL( ETrue );

   // The following line set to change status
   // This avoid back set form View status in the beginning 
   // Usually without change back do that, like when we start an empty Form.
   // SetChangesPending(ETrue);
}

// ----------------------------------------------------------------------------
// CUpgradeDialog::QuerySaveChangesL()
// Show save query. If user answers "No" to this query.
// return field value to the value which is before editing.
// This is called Back-softkey when edit mode.
// ----------------------------------------------------------------------------
//
TBool CUpgradeDialog::QuerySaveChangesL()
{
   TBool isAnsYes = EFalse; //(CAknForm::QuerySaveChangesL());

   if (isAnsYes){
      SaveFormDataL();
   }else{
      // Case that answer "No" to query.
      DoNotSaveFormDataL();
   }
   return isAnsYes;
}

void CUpgradeDialog::DeleteBufs()
{
   delete iCountry;       
   iCountry = NULL;
   delete iPhone;
   iPhone = NULL;
   delete iRegNumber;
   iRegNumber = NULL;
   delete iName; 
   iName = NULL;
   delete iEmail;
   iEmail = NULL;
}

// ----------------------------------------------------------------------------
// CUpgradeDialog::SaveFormDataL()
// Save form data.
// ----------------------------------------------------------------------------
//
TBool CUpgradeDialog::SaveFormDataL()
{
   TBool saved = ETrue;
   DeleteBufs();
   iCountry   = GetEdwinTextIfPresentL(EWayFinderCtrlUpgradeCountryEdwin);
   iPhone     = GetEdwinTextIfPresentL(EWayFinderCtrlUpgradePhoneEdwin);
   iRegNumber = GetEdwinTextIfPresentL(EWayFinderCtrlUpgradeRegNumberEdwin);
   iName      = GetEdwinTextIfPresentL(EWayFinderCtrlUpgradeNameEdwin);
   iEmail     = GetEdwinTextIfPresentL(EWayFinderCtrlUpgradeEmailEdwin);

   saved = !iPhone || (iPhone->Length() > 7 || (*iPhone)[0] == '+');
   if(!saved){
      //phone number validation failed
      WFDialog::ShowErrorL( R_WAYFINDER_REGISTER_GOLD_FAIL_NUMBER_MSG );
   }

   saved = saved && 
      ((iPhone && iRegNumber) || (iName && iEmail) || 
       (iRegNumber && iUpgradeMode == CWayFinderAppUi::EUpgrade));

   if(!saved){
      //delete  stored texts
      DeleteBufs();
   }
   return saved;
}

void CUpgradeDialog::DynInitMenuPaneL( TInt aMenuId, CEikMenuPane* aMenuPane )
{
   CAknForm::DynInitMenuPaneL( aMenuId, aMenuPane ); 
   // this is for the SYSTEM FORM MENU options
   if (aMenuId == R_AVKON_FORM_MENUPANE ){
      // dim these unwanted menu options
      aMenuPane->SetItemDimmed(EAknFormCmdLabel, ETrue);
      aMenuPane->SetItemDimmed(EAknFormCmdAdd , ETrue);
      aMenuPane->SetItemDimmed(EAknFormCmdDelete , ETrue);
      aMenuPane->SetItemDimmed(EAknFormCmdEdit , ETrue);
      aMenuPane->SetItemDimmed(EAknFormCmdSave , ETrue);
      aMenuPane->SetItemDimmed(EAknSoftkeyExit , ETrue);

      TBool dimUpgradeCountry = 
         (NULL == ControlOrNull( EWayFinderCtrlUpgradeCountryEdwin ));
      aMenuPane->SetItemDimmed(EWayFinderCmdUpgradeCountry, dimUpgradeCountry);

      if(iUpgradeMode == CWayFinderAppUi::EUpgrade){
         aMenuPane->SetItemDimmed(EWayFinderCmdUpgradeUpgrade, ETrue);
      } else {
         aMenuPane->SetItemDimmed(EWayFinderCmdUpgradeUpgrade2, ETrue);
      }
   }
   iAppUi->DynInitMenuPaneL( aMenuId, aMenuPane );
}

void CUpgradeDialog::ProcessCommandL( TInt aCommandId )
{
   switch(aCommandId){
   case EWayFinderCmdUpgradeUpgrade:
   case EWayFinderCmdUpgradeUpgrade2:
      if (iInProgress) {
         WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
         return;
      }
      if( SaveFormDataL() ){
         if( iAppUi->AcceptLegalNoticeL() ){
            iInProgress = ETrue;
            //local variables to avoid Des16Align 18 panics
            TPtrC phone(KNullDesC);
            TPtrC key(KNullDesC);
            TPtrC name(KNullDesC);
            TPtrC email(KNullDesC);
            if(iPhone){   phone.Set(*iPhone); }
            if(iRegNumber){ key.Set(*iRegNumber); }
            if(iName){     name.Set(*iName); }
            if(iEmail){   email.Set(*iEmail); }
            iAppUi->SendUpgradeDetails( iCountryId, phone,
                                        key, name, email);
         }
      }
      break;
   case EWayFinderCmdUpgradeCountry:
      SelectCountryL();
      break;
   case EWayFinderSoftkeyStart:
      if (iInProgress) {
         WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
         return;
      }
      aCommandId = EAknSoftkeyBack;
      break;
   case EWayFinderCmdHelp:
/*       iAppUi->ShowUpgradeHelp(); */
      break;
   case EEikCmdExit:
      {
         class CWayFinderAppUi *appUi = iAppUi;
         Close();
         appUi->HandleCommandL(EWayFinderExitAppl);
      }
      break;
   case EWayFinderCmdAbout:
      {
      iAppUi->HandleCommandL(aCommandId);
      }
      break;
   default :
      break;
   }
   CAknForm::ProcessCommandL( aCommandId );
}

TBool CUpgradeDialog::OkToExitL(TInt aButtonId)
{
   TBool sysSaysOkToExit = CAknForm::OkToExitL(aButtonId);
   if (sysSaysOkToExit) {
      if (iInProgress) {
         WFDialog::ShowScrollingErrorDialogL( R_CONNECT_DIALOG_BUSY );
         return EFalse;
      }
      iAppUi->CloseUpgradeDialog( EFalse );
      return ETrue;
   }
   return EFalse;
}

TInt
CUpgradeDialog::AddFormItemL(const TDesC &promt, const TDesC &defText)
{
   TInt myId = iNewItemId;
   class CCoeControl* myEdControl = 
      CreateLineByTypeL(promt, ActivePageId(), myId, EEikCtEdwin, NULL);

   class CEikEdwin* myEdwin = static_cast<CEikEdwin*>(myEdControl);
   myEdwin->ConstructL(EEikEdwinNoHorizScrolling | EEikEdwinResizable, 10,
                       64, 10);
   myEdwin->CreateTextViewL();
   myEdwin->SetTextL(&defText);

   Line(myId)->ActivateL();

   SetEditableL(IsEditable()) ;
   DrawNow() ;
   iNewItemId++;
   return myId;
}



// End of File  
