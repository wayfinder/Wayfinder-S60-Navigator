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


#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V3

#include "arch.h"
#include "PopUpList.h"
#include <aknlists.h> 
#include <aknpopup.h>


TInt
PopUpList::ShowPopupListL( TInt resourceId,
                           const CDesCArray& aDescArray,
                           TBool& aOkChosen,
                           TInt selection,
                           TBool aShowTitle,
                           TDesC* aTitle,
                           TInt popupType)
{

   TInt index = MAX_INT32;

   CEikTextListBox* list;
   AknPopupLayouts::TAknPopupLayouts aknPopupLayout;
   // Create listbox and PUSH it.
   switch (popupType) {
      case 0:
         list = new (ELeave) CAknDoublePopupMenuStyleListBox();
         aknPopupLayout = AknPopupLayouts::EDynMenuDoubleWindow;
         break;
      case 1:
         /** FALLTHROUGH **/
      default:
         list = new(ELeave) CAknSinglePopupMenuStyleListBox();
         aknPopupLayout = AknPopupLayouts::EMenuWindow;
         break;
   }
   //LOGNEW(list, CAknSinglePopupMenuStyleListBox);
   CleanupStack::PushL(list);

   // Create popup list and PUSH it.
   CAknPopupList* popupList = CAknPopupList::NewL(list,
         resourceId,
         aknPopupLayout);
   CleanupStack::PushL(popupList);

   // initialize listbox.
   list->ConstructL(popupList, CEikListBox::ELeftDownInViewRect);
   list->CreateScrollBarFrameL(ETrue);
   list->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,
                                                    CEikScrollBarFrame::EAuto);

   // Set listitems.
   CTextListBoxModel* model = list->Model();
   model->SetItemTextArray( const_cast<CDesCArray*>(&aDescArray) );
   model->SetOwnershipType(ELbmDoesNotOwnItemArray); // do not delete them

   if ((selection != MAX_INT32) &&
       (selection < aDescArray.Count()) && (selection >= 0)) {
      list->SetCurrentItemIndex(selection);
   }

   // Set title if it is needed.
   if (aShowTitle) {
      popupList->SetTitleL(*aTitle);
   }


   // Show popup list and then show return value.
   TBool popupOk = popupList->ExecuteLD();
   aOkChosen = popupOk;

   CleanupStack::Pop(popupList);
   if (popupOk){
      index = list->CurrentItemIndex();
   }
   CleanupStack::PopAndDestroy(list);

   return index;   
}

#endif
