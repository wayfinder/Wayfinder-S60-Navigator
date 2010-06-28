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
#include "MapLayersCheckBox.h"
#include <akncheckboxsettingpage.h>
#include <eikenv.h>
#include "RsgInclude.h"
#include "WFTextUtil.h"

#define KCheckBoxItemListGranularity 5

CMapLayersCheckBox::CMapLayersCheckBox(TInt aIdentifier,
      CArrayFix<TInt>& aValueList,
      CArrayFix<char *>& aTextList)
   : CAknEnumeratedTextSettingItem(aIdentifier),
   iExternalValueArray(aValueList),
   iExternalTextArray(aTextList)
{
}

CMapLayersCheckBox::~CMapLayersCheckBox()
{
   if (iSelectionItemListData) {
      iSelectionItemListData->ResetAndDestroy();
      delete iSelectionItemListData;
   }
}

void
CMapLayersCheckBox::CompleteConstructionL()
{
   // finish constructing base class
   CAknEnumeratedTextSettingItem::CompleteConstructionL();

   // construct the item list array
   iSelectionItemListData = new (ELeave) CSelectionItemList(
         KCheckBoxItemListGranularity );

   // the number of variables in the external array (from data object)
/*    TInt numExternals = iExternalValueArray.Count(); */
   TInt count = iExternalTextArray.Count();

   for (TInt index = 0; index < count; index++ ) {
      HBufC* tmp = WFTextUtil::Utf8AllocLC(iExternalTextArray[index]);
      CSelectableItem* selectionItem = new(ELeave)CSelectableItem(
            *tmp,
            iExternalValueArray[index] == 1);

      CleanupStack::PushL(selectionItem);
      selectionItem->ConstructL();
      iSelectionItemListData->AppendL(selectionItem);
      CleanupStack::Pop(selectionItem);
      CleanupStack::PopAndDestroy(tmp);
   }
}


void
CMapLayersCheckBox::LoadL()
{
   // number of items in external data array
   TInt numExternals = iExternalValueArray.Count();

   // copy all items from the external data representation into the internal
   for (TInt i=0;i<numExternals;i++) {
      (*iSelectionItemListData)[i]->SetSelectionStatus(iExternalValueArray[i]);
   }

   // make sure the text reflects what's actually stored
   SetTextRepresentationL();
}

void
CMapLayersCheckBox::StoreL()
{
   // number of items in external data array
   TInt numExternals = iExternalValueArray.Count();

   for (TInt i=0;i<numExternals;i++) {
      iExternalValueArray[i] =
         (*iSelectionItemListData)[i]->SelectionStatus();
   }
}

void
CMapLayersCheckBox::EditItemL( TBool /* aCalledFromMenu */ )
{
   // construct the settings page based on
   // R_WAYFINDER_SETTINGS_MAP_LAYERS_CHECKBOX_PAGE resource
   CAknCheckBoxSettingPage* dlg =
      new ( ELeave )CAknCheckBoxSettingPage(
            R_WAYFINDER_SETTINGS_MAP_LAYERS_CHECKBOX_PAGE,
            iSelectionItemListData);
   // execute the dialog - will display the settings page
   if ( dlg->ExecuteLD(CAknSettingPage::EUpdateWhenAccepted) ) {
      // something changed - so update internal representation
      // of display text
      SetTextRepresentationL();
      // and update the text being displayed
      UpdateListBoxTextL();
   } else {
      /* User selected cancel, we need to restore previous state. */
      LoadL();
   }
}

const TDesC&
CMapLayersCheckBox::SettingTextL()
{
   // if external data is not available or the text representation has not
   // been created
   if ( (iExternalValueArray.Count() == 0) || !iTextRepresentation) {
      return EmptyItemText(); // no data
   } else {
      return *(iTextRepresentation);
   }
}

const TInt KInternalRepSize = 128;

void
CMapLayersCheckBox::SetTextRepresentationL()
{
   // if internal representation has not been created
   if (!iTextRepresentation) {
      iTextRepresentation = HBufC::NewL(KInternalRepSize);
   }
   // get modifiable pointer to descriptor
   TPtr text = iTextRepresentation->Des();

   // clear descriptor
   text.Zero();

   TBool addComma=EFalse;
   _LIT(KComma,", ");

   // number of items in selection list
   TInt nItems = iSelectionItemListData->Count();

   TBuf<128> tmp;
   for (TInt i=0; i<nItems; i++) {
      // if item is selected
      if ( (*iSelectionItemListData)[i]->SelectionStatus() ) {
         // add a comma if required
         if (addComma) {
            text.Append(KComma);
         }
         // add item's select string to display string
         const char *foo = iExternalTextArray[i];
         WFTextUtil::char2TDes(tmp, foo);
         if ( (text.Length() + tmp.Length()) >= (text.MaxLength()-4) ) {
            /* Not enough space. */
            break;
         } else {
            text.Append( tmp );
         }

         // any item other than the first should be preceded by a comma
         addComma=ETrue;
      }
   }

   if (text.Length() == 0) {
      CEikonEnv::Static()->ReadResource(text, R_WF_CATEGORIES_NONE);
   }
}



// End of File
