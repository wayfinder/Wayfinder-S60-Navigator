/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// INCLUDE FILES
#include <eikcapc.h>
#include <eikmenup.h>
#include <eikedwin.h>
#include <avkon.rsg>
#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "NewDestDialog.h"
#include "NewDestView.h"
#include "WayFinderConstants.h"
#include "GuiDataStore.h"
#include "wficons.mbg"

#define ILOG_POINTER aLog
#include "memlog.h"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------------------------
// CNewDestDialog::NewL()
// Two-phased constructor.
// ----------------------------------------------------------------------------
//
CNewDestDialog* CNewDestDialog::NewL( CNewDestView* aView, isab::Log* aLog )
{
   CNewDestDialog* self = new (ELeave) CNewDestDialog( aLog );
   CleanupStack::PushL(self);
   LOGNEW(self, CNewDestDialog);
   self->ConstructL( aView );
   CleanupStack::Pop();
   return self;
}

// Destructor
CNewDestDialog::~CNewDestDialog()
{
}

// ----------------------------------------------------------------------------
// CNewDestDialog::ExecuteLD( TInt aResourceId )
// 
// ----------------------------------------------------------------------------
//
TInt CNewDestDialog::ExecuteLD( TInt aResourceId )
{
   return CAknForm::ExecuteLD( aResourceId );
}

// ----------------------------------------------------------------------------
// CNewDestDialog::PrepareLC( TInt aResourceId )
// 
// ----------------------------------------------------------------------------
//
void CNewDestDialog::PrepareLC( TInt aResourceId )
{
   CAknForm::PrepareLC( aResourceId );
   SetLineNonFocusing( EWayFinderCtrlNewDestCountryEdwin );
   CEikEdwin* control = static_cast<CEikEdwin*>( ControlOrNull( EWayFinderCtrlNewDestStringEdwin ) );
   if( control ){
      control->AddEdwinObserverL( this );
   }
}

// ---------------------------------------------------------
// CNewDestDialog::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CNewDestDialog::ConstructL( CNewDestView* aView )
{
   CAknForm::ConstructL( R_NEWDEST_QUERY_MENUBAR );
   iView = aView;
   iHasFocus = ETrue;
   iProcessExit = ETrue;
   iShowHelp = EFalse;
}

void CNewDestDialog::Close(TBool aProcessExit)
{
   iProcessExit = aProcessExit;
   ButtonCommandObserver()->ProcessCommandL( EAknSoftkeyBack );
}

void CNewDestDialog::RemoveInputFocus()
{
   CCoeControl* control = ControlOrNull( EWayFinderCtrlNewDestCityEdwin );
   if( control ) {
      control->SetFocus( EFalse );
   }
   control = ControlOrNull( EWayFinderCtrlNewDestHouseNumEdwin);
   if( control ) {
      control->SetFocus( EFalse );
   }
   control = ControlOrNull( EWayFinderCtrlNewDestStringEdwin );
   if( control ) {
      control->SetFocus( EFalse );
   }
   control = ControlOrNull( EWayFinderCtrlNewDestCountryEdwin );
   if( control ) {
      control->SetFocus( EFalse );
   }
   iHasFocus = EFalse;
}

void CNewDestDialog::SetCountry( TDesC &aCountry )
{
   if( ControlOrNull( EWayFinderCtrlNewDestCountryEdwin ) ){
      SetEdwinTextL( EWayFinderCtrlNewDestCountryEdwin, &aCountry );
   }
}

void CNewDestDialog::SetHouseNum( TDesC &aCity )
{
   if( ControlOrNull( EWayFinderCtrlNewDestHouseNumEdwin ) ){
      SetEdwinTextL( EWayFinderCtrlNewDestHouseNumEdwin, &aCity );
   }
}
void CNewDestDialog::SetCity( TDesC &aCity )
{
   if( ControlOrNull( EWayFinderCtrlNewDestCityEdwin ) ){
      SetEdwinTextL( EWayFinderCtrlNewDestCityEdwin, &aCity );
   }
}

void CNewDestDialog::SetString( TDesC &aSearchString )
{
   if( ControlOrNull( EWayFinderCtrlNewDestStringEdwin ) ){
      SetEdwinTextL( EWayFinderCtrlNewDestStringEdwin, &aSearchString );
   }
}

void CNewDestDialog::HandleEdwinEventL( CEikEdwin* aEdwin,TEdwinEvent aEventType )
{
   CEikEdwin* control = static_cast<CEikEdwin*>( ControlOrNull( EWayFinderCtrlNewDestStringEdwin ) );
   if( control == aEdwin ){
      if( aEventType != EEventFormatChanged && aEventType != EEventNavigation ){
         iView->SetCategorySelected( EFalse );
      }
   }
}

TKeyResponse CNewDestDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
{
   if (iView->iSelectingCountry) {
      return CCoeControl::OfferKeyEventL(aKeyEvent, aType);
   }
   if( aType == EEventKeyDown ){
      if( aKeyEvent.iScanCode == EStdKeyDevice1 && 
          !(aKeyEvent.iModifiers & EModifierShift) ){
         iEditKeyModifier = EFalse;
         //ProcessCommandL( EAknSoftkeyBack );
      }
   } 
   if( aKeyEvent.iScanCode == EStdKeyDevice1 && 
       (aKeyEvent.iModifiers & EModifierShift) ) {
      iEditKeyModifier = ETrue;
      return EKeyWasConsumed;
   } else {
      iEditKeyModifier = EFalse;
   }
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
   if (!iShowHelp && aKeyEvent.iCode == EKeyEscape) {
      return EKeyWasNotConsumed;
   }
   // Use the arrow keys to move focus between the controls.
   switch (aKeyEvent.iScanCode)
      {
      case EStdKeyDevice3:
         if( iHasFocus && IdOfFocusControl() == EWayFinderCtrlNewDestCountryEdwin){
            iView->HandleCommandL( EWayFinderCmdNewDestCountry );
            CAknForm::OfferKeyEventL( aKeyEvent, aType );
            return EKeyWasConsumed;
         } else {
            ProcessCommandL( EWayFinderCmdNewDestSearch );
            return EKeyWasConsumed;
         }
         break;
      case EStdKeyUpArrow:
         if( IdOfFocusControl() == EWayFinderCtrlNewDestStringEdwin ){
            RemoveInputFocus();
            return EKeyWasConsumed;
         } else {
            iHasFocus = ETrue;
            return CAknForm::OfferKeyEventL( aKeyEvent, aType );
         }
         break;
      case EStdKeyDownArrow:
         if( IdOfFocusControl() == EWayFinderCtrlNewDestCountryEdwin ){
            RemoveInputFocus();
            return EKeyWasConsumed;
         } else {
            iHasFocus = ETrue;
            return CAknForm::OfferKeyEventL( aKeyEvent, aType );
         }
         break;
      case EStdKeyRightArrow:
         if( !iHasFocus ){
            if( iView->HasSearchData() ){
               iView->HandleCommandL( EWayFinderCmdNewDest2 );
            }
            return EKeyWasConsumed;
         }
         else{
            return CAknForm::OfferKeyEventL( aKeyEvent, aType );
         }
         break;
      default:
         if( IdOfFocusControl() == EWayFinderCtrlNewDestStringEdwin ){
            iView->SetCategorySelected( EFalse );
         }
         return CAknForm::OfferKeyEventL( aKeyEvent, aType );
         break;
      }
   return EKeyWasNotConsumed;
}

// ----------------------------------------------------------------------------
// CNewDestDialog::PostLayoutDynInitL()
// Set default field value to member data.
// ----------------------------------------------------------------------------
//
void CNewDestDialog::PostLayoutDynInitL()
{
   CAknForm::PostLayoutDynInitL();

   // To Following line change the edit mode
   //SetEditableL( ETrue );

   // The following line set to change status
   // This avoid back set form View status in the beginning 
   // Usually without change back do that, like when we start an empty Form.
   // SetChangesPending(ETrue);
}

// ----------------------------------------------------------------------------
// CNewDestDialog::PreLayoutDynInitL()
// Load the correct bitmap.
// ----------------------------------------------------------------------------
void CNewDestDialog::PreLayoutDynInitL()
{
/*    Line(EWayFinderCtrlNewDestStringEdwin)-> */
/*       SetBitmapFromFileL( iView->GetGuiDataStore()->iWayfinderMBMFilename->Des(), */
/*                           EMbmWficonsRed_star, EMbmWficonsRed_star_m); */
/*    Line(EWayFinderCtrlNewDestCityEdwin)-> */
/*       SetBitmapFromFileL( iView->GetGuiDataStore()->iWayfinderMBMFilename->Des(), */
/*                           EMbmWficonsRed_star, EMbmWficonsRed_star_m); */
#ifdef NAV2_CLIENT_SERIES60_V3
   Line(EWayFinderCtrlNewDestCountryEdwin)->
      SetBitmapFromFileL( iView->GetGuiDataStore()->iWayfinderMBMFilename->Des(),
                          EMbmWficonsRed_star, EMbmWficonsRed_star_mask);
#else
   Line(EWayFinderCtrlNewDestCountryEdwin)->
      SetBitmapFromFileL( iView->GetGuiDataStore()->iWayfinderMBMFilename->Des(),
                          EMbmWficonsRed_star, EMbmWficonsRed_star_m);
#endif
}

// ----------------------------------------------------------------------------
// CNewDestDialog::QuerySaveChangesL()
// Show save query. If user answers "No" to this query.
// return field value to the value which is before editing.
// This is called Back-softkey when edit mode.
// ----------------------------------------------------------------------------
//
TBool CNewDestDialog::QuerySaveChangesL()
{
   /*
   TBool isAnsYes(CAknForm::QuerySaveChangesL());

   if (isAnsYes){
      SaveFormDataL();
   }
   else{
      // Case that answer "No" to query.
      DoNotSaveFormDataL();
   }

   return isAnsYes;
   */
   SaveFormDataL();
   return ETrue;
}

// ----------------------------------------------------------------------------
// CNewDestDialog::SaveFormDataL()
// Save form data.
// ----------------------------------------------------------------------------
//
TBool CNewDestDialog::SaveFormDataL()
{
   if( ControlOrNull( EWayFinderCtrlNewDestCountryEdwin ) ){
      GetEdwinText( iCountry, EWayFinderCtrlNewDestCountryEdwin );
   }
   if( ControlOrNull( EWayFinderCtrlNewDestHouseNumEdwin ) ){
      GetEdwinText( iHouseNum, EWayFinderCtrlNewDestHouseNumEdwin );
   }
   if( ControlOrNull( EWayFinderCtrlNewDestCityEdwin ) ){
      GetEdwinText( iCity, EWayFinderCtrlNewDestCityEdwin );
   }
   if( ControlOrNull( EWayFinderCtrlNewDestStringEdwin ) ){
      GetEdwinText( iSearchString, EWayFinderCtrlNewDestStringEdwin );
   }
   iView->SetSearchStrings( iCountry, iCity, iHouseNum, iSearchString );
   return ETrue;
}

void CNewDestDialog::DynInitMenuPaneL( TInt aMenuId, CEikMenuPane* aMenuPane )
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
      if( iView->IsPreviousSearchSet() ) {
         aMenuPane->SetItemDimmed( EWayFinderCmdNewDestString, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdNewDestString, ETrue );
      }
      if( iView->HasCities() || iView->IsGpsAllowed()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdNewDestCity, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdNewDestCity, ETrue );
      }
   }
   iView->DynInitMenuPaneL( aMenuId, aMenuPane );
}

void CNewDestDialog::ProcessCommandL( TInt aCommandId )
{
   switch(aCommandId)
   {
   case EWayFinderCmdNewDestSearch:
      SaveFormDataL();
   case EWayFinderCmdNewDestCountry:
   case EWayFinderCmdNewDestCity:
   case EWayFinderCmdNewDestCategory:
   case EWayFinderCmdNewDestString:
   case EWayFinderCmdAbout:
   case EWayFinderCmdGPSConnect:
   case EWayFinderCmdGPSDisConnect:
   case EWayFinderCmdDebugAction:
      CAknForm::ProcessCommandL( aCommandId );
      iView->HandleCommandL( aCommandId );
      break;
   case EWayFinderCmdInfo:
      CAknForm::ProcessCommandL( aCommandId );
      iView->HandleCommandL( EWayFinderCmdNewDestInfo );
      break;
   case EWayFinderCmdHelp:
      iShowHelp = ETrue;
      iView->ShowNewSearchHelpL();
      CAknForm::ProcessCommandL( aCommandId );
      break;
   case EAknSoftkeyBack :
      CAknForm::ProcessCommandL( aCommandId );
      iView->HandleCommandL( EWayFinderCmdNewDestQueryBack );
      break;
   case EAknCmdExit:
   case EAknSoftkeyExit:
   case EEikCmdExit:
   case EWayFinderExitAppl:
   default :
      CAknForm::ProcessCommandL( aCommandId );
      iView->HandleCommandL( aCommandId );
      break;
   }
}

void CNewDestDialog::PrepareForFocusTransitionL()
{
   CAknForm::PrepareForFocusTransitionL();
}

TBool CNewDestDialog::OkToExitL(TInt aButtonId)
{
//    if (aButtonId == EAknSoftkeyBack && iEditKeyModifier) {
//       return EFalse;
//    } else {
//    return CAknForm::OkToExitL(aButtonId);
//    }

   if (iProcessExit && aButtonId == EAknSoftkeyBack) {
      TBool ret = CAknForm::OkToExitL(aButtonId);
      ProcessCommandL(aButtonId);
      return ret;
   }
   iProcessExit = ETrue;
   return CAknForm::OkToExitL(aButtonId);
}


// End of File  
