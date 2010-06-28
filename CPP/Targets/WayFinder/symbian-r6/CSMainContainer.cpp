/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <avkon.hrh>
#include <avkon.rsg>
#include <eikmenup.h>
#include <aknappui.h>
#include <eikcmobs.h>
#include <barsread.h>
#include <stringloader.h>
#include <gdi.h>
#include <eikedwin.h>
#include <eikenv.h>
#include <w32std.h>
#include <eiklabel.h> 

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WFLayoutUtils.h"
#include "CSMainContainer.h"

CCSMainContainer::CCSMainContainer( MEikCommandObserver* aCommandObserver,
                                    CCSMainView& aView, 
                                    CCSMainView::TFormData& aSavedData ) : 
   iView(aView), iSavedData(aSavedData)
{
   iCommandObserver = aCommandObserver;
   
}

CCSMainContainer* CCSMainContainer::NewL( MEikCommandObserver* aCommandObserver,
                                          CCSMainView& aView, 
                                          CCSMainView::TFormData& aSavedData )
{
   CCSMainContainer* self = CCSMainContainer::NewLC( aCommandObserver, aView, aSavedData );
   CleanupStack::Pop( self );
   return self;
}

CCSMainContainer* CCSMainContainer::NewLC( MEikCommandObserver* aCommandObserver,
                                           CCSMainView& aView, 
                                           CCSMainView::TFormData& aSavedData )
{
   CCSMainContainer* self = new ( ELeave ) CCSMainContainer( aCommandObserver, aView, aSavedData );
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}

void CCSMainContainer::ConstructL( )
{
   CAknForm::ConstructL( R_WAYFINDER_MENUBAR_CS_MAIN_VIEW_FORM );
}


CCSMainContainer::~CCSMainContainer()
{
}

TKeyResponse CCSMainContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                               TEventCode aType )
{
   TUint lower = ENonCharacterKeyBase;
   TUint higher = ENonCharacterKeyBase + ENonCharacterKeyCount;

   if (aType == EEventKey) {
      if (IdOfFocusControl() == EWayFinderCSCountrySelect) {
         if ( aKeyEvent.iCode == EKeyDevice3 || aKeyEvent.iCode == EKeyEnter ) {
            if (iView.HasSetCountry()) {
               iView.HandleCommandL(EWayFinderCmdCSNewSearch);
               return EKeyWasConsumed;
            } else {
               SwitchViewL(EWayFinderSearchableListBoxView);
               return EKeyWasConsumed;
            }
         }
         if ( aKeyEvent.iCode < lower || aKeyEvent.iCode > higher ) {
            TChar pressedKey(aKeyEvent.iCode);
            TBuf8<1> key;
            key.Append(pressedKey);
            SwitchViewL(EWayFinderSearchableListBoxView, ENoMessage, key);
            return EKeyWasConsumed;
         }
      } else if ( aKeyEvent.iCode == EKeyDevice3 ||
                  aKeyEvent.iCode == EKeyEnter ) {
         iView.HandleCommandL(EWayFinderCmdCSNewSearch);
         return EKeyWasConsumed;
      }
   }

   return CAknForm::OfferKeyEventL( aKeyEvent, aType );
}

void CCSMainContainer::HandleControlEventL(CCoeControl *aControl, 
                                           TCoeEvent aEventType)
{
   CAknForm::HandleControlEventL(aControl, aEventType);
   if ((iESearchWhat == aControl) && (aEventType == EEventStateChanged)) {
      iSavedData.iCategoryId.Copy(KEmpty); //reset category id
   }
   if ((iESearchWhere == aControl) && (aEventType == EEventStateChanged)) {
      iSavedData.iUseFromGps = 0;
   	  InitSearchFields(ETrue);
      iView.UpdateGpsButton();
   }
}

TBool CCSMainContainer::QuerySaveChangesL()
{
   SaveFormDataL();
   return ETrue;
}

TBool CCSMainContainer::SaveFormDataL()
{
   //XXX it could be nice not to store where and country
   //if they are set to from-gps cause then we could always
   //remember the previous where and country that the user gave.
   //BUT this might also be helped if we have a correct history!?.
   GetEdwinText(iSavedData.iWhat, EWayFinderCSSearchText);
   if (!iSavedData.iUseFromGps) {
      GetEdwinText(iSavedData.iWhere, EWayFinderCSCitySelect);
      GetEdwinText(iSavedData.iCountry, EWayFinderCSCountrySelect);
   }

   iSavedData.iActiveIndex = IdOfFocusControl();
   return ETrue;
}

void CCSMainContainer::DoNotSaveFormDataL()
{
   LoadFromDataL();
}

void CCSMainContainer::LoadFromDataL()
{
	SetEdwinTextL(EWayFinderCSSearchText, &iSavedData.iWhat);
   	SetEdwinTextL(EWayFinderCSCitySelect, &iSavedData.iWhere);
   	SetEdwinTextL(EWayFinderCSCountrySelect, &iSavedData.iCountry);
   	iUpdate = ETrue;
   	iView.UpdateGpsButton();
	InitSearchFields();
}

void CCSMainContainer::SetInitialCurrentLine()
{
   CEikDialog::SetInitialCurrentLine();
   if (iSavedData.iActiveIndex > 0) {
      TryChangeFocusToL( iSavedData.iActiveIndex);//Id of focused control 	
   }
}

void CCSMainContainer::PreLayoutDynInitL()
{
   iESearchWhat = static_cast< CEikEdwin* >
      ( ControlOrNull( EWayFinderCSSearchText ) );
//    {
//       HBufC* text = StringLoader::LoadLC( R_STEST_FORM_ESEARCH_TEXT );
//       iESearchText->SetTextL( text );
//       CleanupStack::PopAndDestroy( text );
//    }

   iESearchWhere = static_cast< CEikEdwin* >
      ( ControlOrNull( EWayFinderCSCitySelect ) );
//    {
//       HBufC* text = StringLoader::LoadLC( R_STEST_FORM_ECITY_SELECT );
//       iECitySelect->SetTextL( text );
//       CleanupStack::PopAndDestroy( text );
//    }

   iESearchCountry = static_cast< CEikEdwin* >
      ( ControlOrNull( EWayFinderCSCountrySelect ) );
//    {
//       HBufC* text = StringLoader::LoadLC( R_STEST_FORM_ESEARCH_COUNTRY );
//       iESearchCountry->SetTextL( text );
//       CleanupStack::PopAndDestroy( text );
//    }

   LoadFromDataL();
}

void CCSMainContainer::HandleResourceChange( TInt aType )
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
}

void CCSMainContainer::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   CAknForm::DynInitMenuPaneL( aResourceId, aMenuPane );
	
   if ( aResourceId == R_AVKON_FORM_MENUPANE ) {
      aMenuPane->SetItemDimmed( EAknFormCmdAdd, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdLabel, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdDelete, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdEdit, ETrue );
      aMenuPane->SetItemDimmed( EAknFormCmdSave, ETrue );

      if (iView.IsPreviousSearchSet()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdCSSearchHistory, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdCSSearchHistory, ETrue );
      }
      if (iView.IsGpsConnectedOrCellidAvailable()) {
         aMenuPane->SetItemDimmed( EWayFinderCmdCSSetFromGps, EFalse );
      } else {
         aMenuPane->SetItemDimmed( EWayFinderCmdCSSetFromGps, ETrue );
      }
   }
   
   iView.DynInitMenuPaneL( aResourceId, aMenuPane );
}

void CCSMainContainer::ProcessCommandL( TInt aCommandId )
{
   switch(aCommandId)
      {
      case EWayFinderCmdCSSwitchToCountrySelect:
         SaveFormDataL();
         CAknForm::ProcessCommandL( aCommandId );
         iView.HandleCommandL( aCommandId );
         break;
      case EWayFinderCmdCSSwitchToCategorySelect:
         SaveFormDataL();
         CAknForm::ProcessCommandL( aCommandId );
         iView.HandleCommandL( aCommandId );
         break;
      default:
         CAknForm::ProcessCommandL( aCommandId );
         iView.HandleCommandL( aCommandId );
         break;
      }
}

TBool CCSMainContainer::OkToExitL( TInt aButtonId )
{
   if ( aButtonId == EAknSoftkeyBack ) {
      SaveFormDataL();
      if ( CAknForm::OkToExitL( aButtonId ) ) {
         iView.HandleCommandL( aButtonId );
         // The dialog (form) may never die by itself since it will 
         // give a cone 44 panic if it is deleted without beeing 
         // removed from view stack (e.g. dodeactivate).
         return EFalse; 
      } else {
         return EFalse;
      }
   }

   return CAknForm::OkToExitL( aButtonId );
}

void CCSMainContainer::SetWhat( const TDesC& aValue )
{
   if (iESearchWhat /*&& aValue.Length() > 0*/) {
      _LIT(KTab, "\t");
      TInt pos = aValue.Find(KTab);
      HBufC* tmp;
      if (pos == KErrNotFound) {
         tmp = aValue.Mid(0).AllocLC();
         iESearchWhat->SetTextL(tmp);
      } else {
         tmp = aValue.Mid(pos+1).AllocLC();
         iESearchWhat->SetTextL(tmp);
      }
      CleanupStack::PopAndDestroy(tmp);
      DrawNow();
   }
}

void CCSMainContainer::SetWhere( const TDesC& aValue )
{
   if (iESearchWhere /*&& aValue.Length() > 0*/) {
      _LIT(KTab, "\t");
      TInt pos = aValue.Find(KTab);
      HBufC* tmp;
      if (pos == KErrNotFound) {
         tmp = aValue.Mid(0).AllocLC();
         iESearchWhere->SetTextL(tmp);
      } else {
         tmp = aValue.Mid(pos+1).AllocLC();
         iESearchWhere->SetTextL(tmp);
      }
      CleanupStack::PopAndDestroy(tmp);
      DrawNow();
   }
}

void CCSMainContainer::SetCountry( const TDesC& aValue )
{
   if (iESearchCountry /*&& aValue.Length() >=0*/) {
      _LIT(KTab, "\t");
      TInt pos = aValue.Find(KTab);
      HBufC* tmp;
      if (pos == KErrNotFound) {
         tmp = aValue.Mid(0).AllocLC();
         iESearchCountry->SetTextL(tmp);
      } else {
         tmp = aValue.Mid(pos+1).AllocLC();
         iESearchCountry->SetTextL(tmp);
      }
      CleanupStack::PopAndDestroy(tmp);
      DrawNow();
   }
}

void CCSMainContainer::InitSearchFields(TBool aIgnoreWhereField)
{
   iView.UpdateGpsButton();
   if (iSavedData.iUseFromGps && iUpdate) {
      //gps connected fill in where and country field with gps-string
      HBufC* fromGps = 
         CCoeEnv::Static()->AllocReadResourceLC( R_CONNECT_PERSONAL_CURRENT_POSITION );
       	//iView.UpdateGpsButton();
       	iSavedData.iSavedCountryId = iSavedData.iCountryId;
       	iSavedData.iCountryId = -1;
        SetWhere( *fromGps );
      	SetCountry( *fromGps );
      	iSavedData.iWhere = _L("");

      CleanupStack::PopAndDestroy( fromGps );
   } else {
   iSavedData.iCountryId = iSavedData.iSavedCountryId;
      if(!aIgnoreWhereField) {
         //gps not connected
         if (CompareToResource(iESearchWhere->Text()->Read(0), 
                               R_CONNECT_PERSONAL_CURRENT_POSITION)) {
            //where field is set with gps-string
            if (CompareToResource(iSavedData.iWhere, 
                                  R_CONNECT_PERSONAL_CURRENT_POSITION)) {
               //empty where field if previous entry was gps-string
               SetWhere(KEmpty);
            } else {
               //else previous entry was set by user so set it back
               SetWhere(iSavedData.iWhere);
            }
         }
      }
      if (CompareToResource(iESearchCountry->Text()->Read(0), 
                            R_CONNECT_PERSONAL_CURRENT_POSITION)) {
         //country field is set with gps-string
         if (CompareToResource(iSavedData.iCountry, 
                               R_CONNECT_PERSONAL_CURRENT_POSITION)) {
            //empty country field if previous entry was gps-string
            if (iSavedData.iCountryId > 0 || iView.GetCellCountry() >= 0)
            {
            	SetCountry(iSavedData.iCountry);
            }
            else
            {
            	SetCountry(KEmpty);
            }
            //SetCountry(KEmpty);
         } else {
            //else previous entry was a country name so set it back
            	SetCountry(iSavedData.iCountry);
         }
      }
   }
}

void CCSMainContainer::SwitchViewL(TInt aViewId, TInt aCustomMessageId, 
                                   const TDesC8 &aCustomMessage)
{
   SaveFormDataL();
   iView.SwitchViewL(aViewId, aCustomMessageId, aCustomMessage);
}

void CCSMainContainer::SwitchViewL(TInt aViewId)
{
   SaveFormDataL();
   iView.SwitchViewL(aViewId);
}

TBool CCSMainContainer::CompareToResource( const TDesC& aString, 
                                           TInt aResourceId )
{
   HBufC* tmp = CCoeEnv::Static()->AllocReadResourceLC( aResourceId );
   TBool match = (aString.Find(*tmp) != KErrNotFound);
   CleanupStack::PopAndDestroy( tmp );
   return match;
}

void CCSMainContainer::SaveDataL()
{
   SaveFormDataL();
}

void CCSMainContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
   CEikLabel* label = ControlCaption(EWayFinderCSCountrySelect);
   if (aPointerEvent.iType == TPointerEvent::EButton1Down && 
       (iESearchCountry->Rect().Contains(aPointerEvent.iPosition) ||
        (label && label->Rect().Contains(aPointerEvent.iPosition)))) {
      // If user clicked on country field we should switch to country
      // select view.
      SwitchViewL(EWayFinderSearchableListBoxView);
   }
   CCoeControl::HandlePointerEventL(aPointerEvent);      
}

void CCSMainContainer::DisableEditorIndicators()
{
   if (iESearchWhat) {
      TInt flags = iESearchWhat->AknEdwinFlags() | EAknEditorFlagNoEditIndicators;
      iESearchWhat->SetAknEditorFlags(flags);
   }

   if (iESearchWhere) {
      TInt flags = iESearchWhere->AknEdwinFlags() | EAknEditorFlagNoEditIndicators;
      iESearchWhere->SetAknEditorFlags(flags);
   }
}

void CCSMainContainer::EnableEditorIndicators()
{
   if (iESearchWhat) {
      TInt flags = iESearchWhat->AknEdwinFlags() & ~EAknEditorFlagNoEditIndicators;
      iESearchWhat->SetAknEditorFlags(flags);
   }

   if (iESearchWhere) {
      TInt flags = iESearchWhere->AknEdwinFlags() & ~EAknEditorFlagNoEditIndicators;
     iESearchWhere->SetAknEditorFlags(flags);
   }
}
void CCSMainContainer::SetUpdate(TBool aState)
{
	iUpdate = aState;	
}
