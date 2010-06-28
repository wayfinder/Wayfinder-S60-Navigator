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
#include <barsread.h>  // for resource reader
#include <gdi.h>
#include <aknutils.h>  // for fonts

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "MyDestDialog.h"
#define ILOG_POINTER aLog
#include "memlog.h"
// Definitions

//Constants

// ================= MEMBER FUNCTIONS =======================
// ----------------------------------------------------------------------------
// CMyDestForm::NewL()
// Two-phased constructor.
// ----------------------------------------------------------------------------
//
CMyDestDialog* CMyDestDialog::NewL(isab::Log* aLog)
{
   CMyDestDialog* self = new (ELeave) CMyDestDialog(aLog);
   LOGNEW(self, CMyDestDialog);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop();
   return self;
}

#undef  ILOG_POINTER 
#include "memlog.h"

// C++ default constructor can NOT contain any code, that
// might leave.
//
CMyDestDialog::CMyDestDialog(isab::Log* aLog) : iLog(aLog)
{
}


CMyDestDialog::~CMyDestDialog()
{
}

void CMyDestDialog::ConstructL()
{
   iLatSign = NULL;
   iLonSign = NULL;
   iName = NULL;
   iDescription = NULL;
   iCategory = NULL;
   iAlias = NULL;
}

void CMyDestDialog::SetName( TDes* aName )
{
   iName = aName;
}

void CMyDestDialog::SetDescription( TDes* aDescription )
{
   iDescription = aDescription;
}

void CMyDestDialog::SetCategory( TDes* aCategory )
{
   iCategory = aCategory;
}

void CMyDestDialog::SetAlias( TDes* aAlias )
{
   iAlias = aAlias;
}

void CMyDestDialog::SetLatitude( TInt32* aLatitude )
{
   iLatitude = aLatitude;
}

void CMyDestDialog::SetLongitude( TInt32* aLongitude )
{
   iLongitude = aLongitude;
}

TKeyResponse CMyDestDialog::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
   if ( aType == EEventKey ){ // Is not key event?   
	   switch (aKeyEvent.iScanCode)
      {
      case EStdKeyDownArrow:
         if ( iNameDescriptionControl->IsSecondFocused() ){
            iNameDescriptionControl->SetFocus(EFalse, EDrawNow);
            iCoordinatesControl->SetFocus(ETrue, EDrawNow);
            iCoordinatesControl->SetLatFocus( ETrue, EDrawNow  );
            return EKeyWasConsumed;
         } else if (iCoordinatesControl->IsLonFocused()){
            iCoordinatesControl->SetFocus( EFalse, EDrawNow );
            iNameDescriptionControl->SetFocus( ETrue, EDrawNow );
            return EKeyWasConsumed;
         } else if (iCoordinatesControl->IsLatFocused()){
            iCoordinatesControl->SetFocus(ETrue, EDrawNow);
            iCoordinatesControl->SetLatFocus( EFalse, EDrawNow );
            iCoordinatesControl->SetLonFocus( ETrue, EDrawNow );
            return EKeyWasConsumed;
         } else {
            if ( iNameDescriptionControl->IsFocused() ) {
               return iNameDescriptionControl->OfferKeyEventL(aKeyEvent, aType);
            } else if ( iCoordinatesControl->IsFocused() ) {
               return iCoordinatesControl->OfferKeyEventL(aKeyEvent, aType);
            }
         }
         break;
      case EStdKeyUpArrow:
         if (iCoordinatesControl->IsLatFocused()){
            iNameDescriptionControl->SetFocus( ETrue, EDrawNow );
            iNameDescriptionControl->SetSecondFocused( ETrue, EDrawNow  );
            iCoordinatesControl->SetFocus( EFalse, EDrawNow );
            return EKeyWasConsumed;
         } else if (iCoordinatesControl->IsLonFocused()){
            iCoordinatesControl->SetFocus(ETrue, EDrawNow);
            iCoordinatesControl->SetLonFocus( EFalse, EDrawNow );
            iCoordinatesControl->SetLatFocus( ETrue, EDrawNow );
            return EKeyWasConsumed;
         } else if ( iNameDescriptionControl->IsFirstFocused() ){
            iNameDescriptionControl->SetFocus( EFalse, EDrawNow );
            iCoordinatesControl->SetFocus(ETrue, EDrawNow);
            iCoordinatesControl->SetLonFocus( ETrue, EDrawNow  );
            return EKeyWasConsumed;
         } else {
            if ( iNameDescriptionControl->IsFocused() ) {
               return iNameDescriptionControl->OfferKeyEventL(aKeyEvent, aType);
            } else if ( iCoordinatesControl->IsFocused() ) {
               return iCoordinatesControl->OfferKeyEventL(aKeyEvent, aType);
            }
         }
         break;
      case EStdKeyDevice3:
         return CEikDialog::OfferKeyEventL( aKeyEvent, aType );
         break;
      default:
         if ( iNameDescriptionControl->IsFocused() ) {
            return iNameDescriptionControl->OfferKeyEventL( aKeyEvent, aType );
         } else if ( iCoordinatesControl->IsFocused() ) {
            return iCoordinatesControl->OfferKeyEventL( aKeyEvent, aType );
         }
         break;
      }
   } else {
      if ( iNameDescriptionControl->IsFocused() ) {
         return iNameDescriptionControl->OfferKeyEventL( aKeyEvent, aType );
      } else if ( iCoordinatesControl->IsFocused() ) {
         return iCoordinatesControl->OfferKeyEventL( aKeyEvent, aType );
      } else {
         return EKeyWasNotConsumed;
      }
   }
   return EKeyWasNotConsumed;
}

// ----------------------------------------------------------
// CMyDestDialog::PreLayoutDynInitL()
// Initializing the dialog dynamically
// ----------------------------------------------------------
//
void CMyDestDialog::PreLayoutDynInitL()
{
   if( iName != NULL )
      iNameDescriptionControl->SetFirstEdwinL( iName );
   if( iDescription != NULL )
      iNameDescriptionControl->SetSecondEdwinL( iDescription );
   if( iLatitude != NULL )
      iCoordinatesControl->SetLatitudeL( *iLatitude );
   if( iLongitude != NULL )
      iCoordinatesControl->SetLongitudeL( *iLongitude );
}

void CMyDestDialog::PostLayoutDynInitL()
{
}

// ----------------------------------------------------------
// CMyDestDialog::OkToExitL()
// This function ALWAYS returns ETrue
// ----------------------------------------------------------
//
TBool CMyDestDialog::OkToExitL(TInt /*aButtonId*/)
{
   iNameDescriptionControl->GetFirstEdwin( iName );
   iNameDescriptionControl->GetSecondEdwin( iDescription );

   TBool good = iCoordinatesControl->GetLatitudeL( *iLatitude );
   if (good) {
      good = iCoordinatesControl->GetLongitudeL( *iLongitude );
   }
   if (good) {
      return ETrue;
   } else {
      return ETrue;
   }
}

// This function is used by CEikForm::ConstructByTypeL() to create the custom 
// control within the dialog.
SEikControlInfo CMyDestDialog::CreateCustomControlL( TInt aControlType )
{
   SEikControlInfo controlInfo;
   controlInfo.iControl = NULL;
   controlInfo.iTrailerTextId = 0;
   controlInfo.iFlags = 0;

   // XXX I have no idéa why we suddenly have to do this to make it work.
   TInt type = (aControlType & 0xffff);

   switch (type)
   {
   case EWayFinderCtCoordinates:
      iCoordinatesControl = new(ELeave) CCoordinatesControl(KRgbWhite, KRgbBlack, EFalse );
      LOGNEW(iCoordinatesControl, CCoordinatesControl);
      controlInfo.iControl = iCoordinatesControl;
      break;
   case EWayFinderCtNameDescription:
      iNameDescriptionControl = 
         new(ELeave) CTwoEdwinControl( R_MYDEST_NAME_LABEL,
                                       R_MYDEST_DESCRIPTION_LABEL,
                                       R_MYDEST_NAME_EDWIN,
                                       R_MYDEST_DESCRIPTION_EDWIN );
      LOGNEW(iNameDescriptionControl, CTwoEdwinControl);
      controlInfo.iControl = iNameDescriptionControl;
      break;
   default:
      break;
   }
   return controlInfo;
}

// End of File  
