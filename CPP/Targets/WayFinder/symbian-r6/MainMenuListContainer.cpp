/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <barsread.h>
#include <stringloader.h>
#include <aknlists.h>
#include <eikenv.h>
#include <akniconarray.h>
#include <eikclbd.h>
#include <aknviewappui.h>
#include <aknsdrawutils.h>
#include <aknsbasicbackgroundcontrolcontext.h>
#include <eikappui.h>
#include <gulicon.h>
#include <eikfrlb.h> 
#include <aknutils.h>
#include <aknsutils.h> 
#include <math.h>

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "wficons.mbg"
#include "WFLayoutUtils.h"
#include "MainMenuListContainer.h"
#include "MainMenuListView.h"

enum TControls
{
   ETitleLabel,
   EListBox
};

CMainMenuListContainer* CMainMenuListContainer::NewL(const TRect& aRect,
                                                     CMainMenuListView* aView, 
                                                     const TInt* aMbmImageIds,
                                                     const TInt* aMbmMaskIds,
                                                     const TInt* aMainMenuCommandIds,
                                                     const TInt* aMainMenuFirstLabelIds,
                                                     const TInt* aMainMenuSecondLabelIds)
{
   CMainMenuListContainer* self = NewLC(aRect, aView, 
                                        aMbmImageIds, aMbmMaskIds, 
                                        aMainMenuCommandIds,
                                        aMainMenuFirstLabelIds,
                                        aMainMenuSecondLabelIds);
   CleanupStack::Pop(self);
   return self;
}

CMainMenuListContainer* CMainMenuListContainer::NewLC(const TRect& aRect,
                                                      CMainMenuListView* aView, 
                                                      const TInt* aMbmImageIds,
                                                      const TInt* aMbmMaskIds,
                                                      const TInt* aMainMenuCommandIds,
                                                      const TInt* aMainMenuFirstLabelIds,
                                                      const TInt* aMainMenuSecondLabelIds)
{
   CMainMenuListContainer* self = new (ELeave) CMainMenuListContainer();
   CleanupStack::PushL(self);
   self->SetMopParent(aView);
   self->ConstructL(aRect, aView, 
                    aMbmImageIds, aMbmMaskIds, 
                    aMainMenuCommandIds,
                    aMainMenuFirstLabelIds,
                    aMainMenuSecondLabelIds);
   return self;
}

CMainMenuListContainer::CMainMenuListContainer()
{
}

_LIT(KDefaultTitle, "");
void CMainMenuListContainer::ConstructL(const TRect& aRect, 
                                        CMainMenuListView* aView, 
                                        const TInt* aMbmImageIds,
                                        const TInt* aMbmMaskIds,
                                        const TInt* aMainMenuCommandIds,
                                        const TInt* aMainMenuFirstLabelIds,
                                        const TInt* aMainMenuSecondLabelIds)
{
   CreateWindowL();

   InitComponentArrayL();

   iBgContext = 
      CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain,
                                               aRect, ETrue);

   iView = aView;
   iMainMenuCommandIds = aMainMenuCommandIds;

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   TRgb fgcolor(0,0,0);
   AknsUtils::GetCachedColor(skin, fgcolor, KAknsIIDQsnTextColors, 
                             EAknsCIQsnTextColorsCG6);
   
   CEikLabel* label = new (ELeave) CEikLabel();

   label->SetContainerWindowL(*this);
   Components().AppendLC(label, ETitleLabel);
   label->OverrideColorL(EColorLabelText, fgcolor);
   label->SetTextL(KDefaultTitle());
   label->SetLabelAlignment(ELayoutAlignCenter);
   HBufC* titleText = CEikonEnv::Static()->AllocReadResourceLC(R_MM_TITLE_TEXT);
   label->SetTextL(*titleText);
   CleanupStack::PopAndDestroy(titleText);
   CleanupStack::Pop(label);

   // Create and populate the listbox
   iListBox = new( ELeave ) CAknDoubleLargeStyleListBox();
   Components().AppendLC(iListBox, EListBox);
   
   iListBox->SetContainerWindowL(*this);
   iListBox->SetMopParent(this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC( reader, R_WAYFINDER_MAIN_MENU_LIST_VIEW_LISTBOX );
      iListBox->ConstructFromResourceL( reader );
      CleanupStack::PopAndDestroy(); // reader internal state
   }
	
   iListBox->SetFocus(ETrue);
   iListBox->SetListBoxObserver(this);

   iListBox->ItemDrawer()->FormattedCellData()->SetMarqueeParams(5, 3, 1000000, 200000);
   iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);

   for (TInt i = 0; iMainMenuCommandIds[i] != -1; ++i) {
      AddItemL(aMbmImageIds[i], aMbmMaskIds[i], 
               aMainMenuFirstLabelIds[i],
               aMainMenuSecondLabelIds[i]);
   }
   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   iListBox->UpdateScrollBarsL();

   iListBox->SetCurrentItemIndex(0);
   iListBox->DrawNow();
   CleanupStack::Pop(iListBox);
   
   //Activate view
   SetRect(aRect);
   ActivateL();
}


CMainMenuListContainer::~CMainMenuListContainer()
{
   delete iBgContext;
}

void CMainMenuListContainer::ActivateSelection()
{
   TInt selected = iListBox->CurrentItemIndex();
   iView->HandleCommandL(iMainMenuCommandIds[selected]);
}

void CMainMenuListContainer::SetActiveSelection(TInt aIndex)
{
   if (aIndex >= 0 && aIndex < iListBox->Model()->NumberOfItems()) {
      iListBox->SetCurrentItemIndexAndDraw(aIndex);
   }
}

TInt CMainMenuListContainer::GetActiveSelection()
{
   return iListBox->CurrentItemIndex();
}

_LIT(KItem, "%d\t%S\t%S");
void CMainMenuListContainer::AddItemL(TInt aBitmapId, TInt aMaskId, 
                                      TInt aFirstLabelId, TInt aSecondLabelId) 
{
   CArrayPtr<CGulIcon>* icons = 
      iListBox->ItemDrawer()->FormattedCellData()->IconArray();
   if (!icons) {
      icons = new ( ELeave ) CAknIconArray(3);
      iListBox->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
   }
   CleanupStack::PushL(icons);

   CFbsBitmap* bitmap;
   CFbsBitmap* mask;

   AknIconUtils::CreateIconL(bitmap, mask, iView->GetMbmName(), 
                             aBitmapId, aMaskId);

   CleanupStack::PushL(bitmap);
   CleanupStack::PushL(mask);

   icons->AppendL(CGulIcon::NewL(bitmap, mask));

   CleanupStack::Pop(mask);
   CleanupStack::Pop(bitmap);
   CleanupStack::Pop(icons);

   HBufC* firstLabel = CEikonEnv::Static()->AllocReadResourceLC(aFirstLabelId);
   HBufC* secondLabel = CEikonEnv::Static()->AllocReadResourceLC(aSecondLabelId);

   HBufC* itemText = HBufC::NewLC(firstLabel->Length() + 
                                  secondLabel->Length() + 16);

   itemText->Des().Format(KItem(), (icons->Count() - 1), firstLabel, secondLabel);

   CDesCArray* itemArray =
      static_cast<CDesC16ArrayFlat*>(iListBox->Model()->ItemTextArray());

   itemArray->AppendL(*itemText);

   CleanupStack::PopAndDestroy(itemText);
   CleanupStack::PopAndDestroy(secondLabel);
   CleanupStack::PopAndDestroy(firstLabel);

   iListBox->HandleItemAdditionL();
}

CFont*
CMainMenuListContainer::FindLargestPossibleFontL(const TDesC& aTextToFit,
                                                 TInt aMaxWidthInPixels,
                                                 enum TAknLogicalFontId aPreferredLogicalFontId) const
{
   TInt fontMinimizerFactorInPixels = 2;
   // Get the screen device so that we can calc the twips
   // per pixel
   TPixelsTwipsAndRotation twips;
   CWsScreenDevice* screenDev = CEikonEnv::Static()->ScreenDevice();
   screenDev->GetScreenModeSizeAndRotation(screenDev->CurrentScreenMode(),
                                           twips);

   // Calc the twips per pixel
   TReal twipsPerPixel = twips.iTwipsSize.iHeight / 
      TReal(twips.iPixelSize.iHeight);

   // Get the preferred logical font from the font store
   const CFont* preferredFont = AknLayoutUtils::
   FontFromId(aPreferredLogicalFontId);
   TFontSpec fontSpec = preferredFont->FontSpecInTwips();

   // Get the font that matches the fontspec the best
   CFont* fontToUse;
   screenDev->
      GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   TInt fontMinimizerFactorInTwips = 
      TInt(fontMinimizerFactorInPixels * twipsPerPixel);

   while (aMaxWidthInPixels < fontToUse->TextWidthInPixels(aTextToFit)) {
      // The text didnt fit within the given space, make the font
      // a bit smaller and try again
      screenDev->ReleaseFont(fontToUse);
      fontSpec.iHeight -= fontMinimizerFactorInTwips;
      screenDev->
         GetNearestFontToDesignHeightInTwips(fontToUse, fontSpec);
   }

   // Return the font, the caller has to release the font
   // from the CWsScreenDevice
   return fontToUse;
}

void CMainMenuListContainer::HideSecondRow(TBool aHide)
{
#if defined NAV2_CLIENT_SERIES60_V5 || defined NAV2_CLIENT_SERIES60_V32
   iListBox->HideSecondRow(aHide); // Works on s60v3fp1 and forward.
#endif
}

void CMainMenuListContainer::SizeChanged()
{
   CCoeControl::SizeChanged();

   if(iBgContext) {
      iBgContext->SetRect(Rect());
      if (&Window()) {
         iBgContext->SetParentPos(PositionRelativeToScreen());
      }
   }

   CCoeControlArray::TCursor cursor = Components().Find(ETitleLabel);
   CEikLabel* label = cursor.Control<CEikLabel>();
   const TDesC* titleText = label->Text();
   TRect rect = Rect();
   CFont* titleFont = FindLargestPossibleFontL(*titleText, rect.Width(), EAknLogicalFontPrimaryFont);
   label->SetFont(titleFont);
   CEikonEnv::Static()->ScreenDevice()->ReleaseFont(titleFont);

   // Set the size of the label, need to think of the descent as well.
   TSize size = label->MinimumSize();
   size.iWidth = rect.Width();
   TInt descent = label->Font()->DescentInPixels();
   size.iHeight += descent;

   // Set the pos for the listbox, use the height for the label when doing this.
   // Also add the descent in pixels to get some space between the label and the
   // listbox.
   rect.iTl.iY = descent;
   TPoint pos(0, descent);
   label->SetExtent(rect.iTl, size);
   rect.iTl.iY += size.iHeight;

   // As default display both rows and hide the scrollbars
   HideSecondRow(EFalse);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);

   // Need to set the rect to get the listbox to update its values
   iListBox->SetRect(rect);
   TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();

   if (listBoxHeight > rect.Height()) {
      // The listbox is to large to fint, hide the row and check again.
      HideSecondRow(ETrue);
      iListBox->SetRect(rect);
      listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
      if (listBoxHeight > rect.Height()) {
         // Show scroll bar only if the height of the listbox is larger then
         // the rect.
         iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                             CEikScrollBarFrame::EOn);
         iListBox->UpdateScrollBarsL();
      }
   }
   else {
      // Setting the first item gets the list box positioned correctly on the screen
      iListBox->ScrollToMakeItemVisible(0);
   }
}

TTypeUid::Ptr CMainMenuListContainer::MopSupplyObject(TTypeUid aId)
{
   if (iBgContext) {
      return MAknsControlContext::SupplyMopObject(aId, iBgContext);
   }
   return CCoeControl::MopSupplyObject(aId);
}

void CMainMenuListContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);
}

void CMainMenuListContainer::HandleResourceChange(TInt aType)
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
}

TKeyResponse CMainMenuListContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                    TEventCode aType)
{
   TKeyResponse retVal = EKeyWasNotConsumed;
   retVal = iListBox->OfferKeyEventL(aKeyEvent, aType);
   if (retVal == EKeyWasConsumed && aType == EEventKey) {
      iView->UpdateSelectedIndex(GetActiveSelection());
      iView->SetNaviPaneLabelL(GetActiveSelection());
   }
   return retVal;
}

void CMainMenuListContainer::HandleListBoxEventL(CEikListBox* aListBox,
                                                 TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventItemClicked ){
      iView->UpdateSelectedIndex(GetActiveSelection());
      iView->SetNaviPaneLabelL(GetActiveSelection());
   } else if (aListBox == iListBox && aEventType == EEventEnterKeyPressed ||
              aListBox == iListBox && aEventType == EEventItemDoubleClicked) {
      ActivateSelection();
   } 
}

// End of File
