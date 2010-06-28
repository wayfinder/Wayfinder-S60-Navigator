/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <aknlists.h>
#include <akniconarray.h>
#include <akngrid.h>
#include <gulicon.h>
#include <w32std.h>

#include "wayfinder.hrh"
#include "WFLayoutUtils.h"

#include "MainMenuGridContainer.h"
#include "MainMenuGridView.h"

#include "TraceMacros.h"

#define CELL_ICON_PADDING 0.9
#define CELL_ICON_SHRINK 0.1

enum TMainMenuControls
{
   EGrid = 0,
   ENumberControls
};

CMainMenuGridContainer* CMainMenuGridContainer::NewL(const TRect& aRect,
                                                     CMainMenuGridView* aView, 
                                                     const TInt* aMbmImageIds,
                                                     const TInt* aMbmMaskIds,
                                                     const TInt* aMainMenuCommandIds)
{
   CMainMenuGridContainer* self = NewLC(aRect, aView, 
                                        aMbmImageIds, aMbmMaskIds, 
                                        aMainMenuCommandIds);
   CleanupStack::Pop(self);
   return self;
}

CMainMenuGridContainer* CMainMenuGridContainer::NewLC(const TRect& aRect,
                                                      CMainMenuGridView* aView, 
                                                      const TInt* aMbmImageIds,
                                                      const TInt* aMbmMaskIds,
                                                      const TInt* aMainMenuCommandIds)
{
   CMainMenuGridContainer* self = new (ELeave) CMainMenuGridContainer();
   CleanupStack::PushL(self);
   self->ConstructL(aRect, aView, 
                    aMbmImageIds, aMbmMaskIds, 
                    aMainMenuCommandIds);
   return self;
}

CMainMenuGridContainer::CMainMenuGridContainer()
   : iGrid(NULL)
{
}

void CMainMenuGridContainer::ConstructL(const TRect& aRect, 
                                        CMainMenuGridView* aView, 
                                        const TInt* aMbmImageIds,
                                        const TInt* aMbmMaskIds,
                                        const TInt* aMainMenuCommandIds)
{
   CreateWindowL();

   iView = aView;
   iMbmImageIds = aMbmImageIds;
   iMbmMaskIds = aMbmMaskIds;
   iMainMenuCommandIds = aMainMenuCommandIds;

   //Activate view
   SetRect(aRect);
   ActivateL();
}

void CMainMenuGridContainer::MakeGridL(const TRect& aRect)
{
   
   if (iGrid) {
      delete iGrid;
      iGrid = NULL;
   }
   iGrid = new (ELeave) CAknGrid;
   iGrid->SetContainerWindowL(*this);   

   iGridM = new (ELeave) CAknGridM;
   iGrid->SetModel(iGridM);

   iGridType = EAknListBoxSelectionGrid;
   iGrid->ConstructL(this,  iGridType);
   iGrid->SetListBoxObserver( this );
   iGrid->SetRect(aRect);

   iVerticalScrollingType = CAknGridView::EScrollIncrementLineAndLoops;
   iHorizontalScrollingType = CAknGridView::EScrollIncrementLineAndLoops;
   iVerticalOrientation = EFalse;
   iTopToBottom = ETrue;
   iLeftToRight = ETrue;
   iNumOfRows = 3;
   iNumOfColumns = 3;

   iGrid->ItemDrawer()->SetSkinEnabledL(ETrue);
   iGrid->ItemDrawer()->ColumnData()->SetSkinEnabledL(ETrue);

   iCellSize.iWidth = (aRect.iBr.iX - aRect.iTl.iX) / iNumOfColumns;
   iCellSize.iHeight = (aRect.iBr.iY - aRect.iTl.iY) / iNumOfRows;

   TSize iconShrink = TSize(TInt(iCellSize.iWidth * CELL_ICON_SHRINK), 
                            TInt(iCellSize.iHeight * CELL_ICON_SHRINK));
   iIconRect = TRect(iCellSize);
   iIconRect.Shrink(iconShrink);
 
   iGrid->SetLayoutL(iVerticalOrientation, 
                     iLeftToRight, iTopToBottom, 
                     iNumOfColumns, iNumOfRows,
                     iCellSize);
   iGrid->SetPrimaryScrollingType(iVerticalScrollingType);
   iGrid->SetSecondaryScrollingType(iHorizontalScrollingType);

   SetupGridItemsL();

   iGrid->MakeVisible(ETrue);
   iGrid->SetFocus(ETrue);
   iGrid->ActivateL();
   iGrid->DrawNow(); 
}

CMainMenuGridContainer::~CMainMenuGridContainer()
{
   delete iGrid;
}

void CMainMenuGridContainer::ActivateSelection()
{
   TInt selected = iGrid->CurrentDataIndex();
   iView->HandleCommandL(iMainMenuCommandIds[selected]);
}

void CMainMenuGridContainer::SetActiveSelection(TInt aIndex)
{
   if (aIndex >= 0 && aIndex < iGrid->GridModel()->NumberOfData()) {
      iGrid->SetCurrentDataIndex(aIndex);
   }
}

TInt CMainMenuGridContainer::GetActiveSelection()
{
   if (iGrid) {
      return iGrid->CurrentDataIndex();
   } else {
      return 0;
   }
}

void CMainMenuGridContainer::SetupGridItemsL() 
{
   TInt i = 0;
   while (iMainMenuCommandIds[i] != -1) {
      AddItemL(iMbmImageIds[i], iMbmMaskIds[i]);
      i++;
   }
   if (i <= 4) {
      iNumOfRows = 2;
      iNumOfColumns = 2;
   } else if (i <= 6) {
      if (WFLayoutUtils::LandscapeMode()) {
         iNumOfRows = 2;
         iNumOfColumns = 3;
      } else {
         iNumOfRows = 3;
         iNumOfColumns = 2;
      }
   } else {
      iNumOfRows = 3;
      iNumOfColumns = 3;
   }
   iGrid->GridView()->SetGridCellDimensions(TSize(iNumOfRows, iNumOfColumns));
   SetGridGraphicStyleL();
}

void CMainMenuGridContainer::AddItemL(TInt aBitmapId, TInt aMaskId, 
                                      TInt aIndex) 
{
   CArrayPtr<CGulIcon>* icons = 
      iGrid->ItemDrawer()->FormattedCellData()->IconArray();
   if (!icons) {
      icons = new ( ELeave ) CAknIconArray(9);
      iGrid->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);
   }
   CleanupStack::PushL(icons);

   CFbsBitmap* bitmap;
   CFbsBitmap* mask;

#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(bitmap, mask, iView->GetMbmName(), 
                             aBitmapId, aMaskId);
   AknIconUtils::SetSize(bitmap, iIconRect.Size(),
                         EAspectRatioPreservedAndUnusedSpaceRemoved);
#else
   bitmap = iEikonEnv->CreateBitmapL(iView->GetMbmName(), aBitmapId);
   mask = iEikonEnv->CreateBitmapL(iView->GetMbmName(), aMaskId);
#endif

   CleanupStack::PushL(bitmap);
   CleanupStack::PushL(mask);

   if (aIndex >= 0 && aIndex < icons->Count()) {
      icons->InsertL(aIndex, CGulIcon::NewL(bitmap, mask));
   } else {
      icons->AppendL(CGulIcon::NewL(bitmap, mask));
   }
   CleanupStack::Pop(mask);
   CleanupStack::Pop(bitmap);
   CleanupStack::Pop(icons);

   _LIT(KItem, "%d\t\t0");
   TBuf<8> buf;
   MDesCArray* array = iGrid->Model()->ItemTextArray();
   CDesCArray* cArray = (CDesCArray*)array;
   if (aIndex >= 0 && aIndex < icons->Count()) {
      buf.Format(KItem, aIndex);
      cArray->InsertL(aIndex, buf);
      aIndex++;
      //We need to delete and re-index the array since we inserted
      //a new icon in the icon array.
      cArray->Delete(aIndex, icons->Count() - aIndex);
      for (TInt i = aIndex; i < icons->Count(); i++) {
         buf.Format(KItem, i);
         cArray->AppendL(buf);
      }
   } else {
      buf.Format(KItem, (icons->Count() - 1));
      cArray->AppendL(buf);
   }
   //Inform list box that data was added.
   iGrid->HandleItemAdditionL();
}

void CMainMenuGridContainer::DeleteItemL(TInt aIndex) 
{
   CArrayPtr<CGulIcon>* icons = 
      iGrid->ItemDrawer()->FormattedCellData()->IconArray();
   if (icons && aIndex >= 0 && aIndex < icons->Count()) {
      //Delete item if index is valid.
      _LIT(KItem, "%d\t\t0");
      TBuf<8> buf;
      MDesCArray* array = iGrid->Model()->ItemTextArray();
      CDesCArray* cArray = (CDesCArray*)array;

      //Remove icon and items.
      icons->Delete(aIndex);
      cArray->Delete(aIndex, (cArray->Count() - aIndex));
      iGrid->HandleItemRemovalL();

      //Re-add the items behind, needed since we changed the indexes.
      for (TInt i = aIndex; i < icons->Count(); i++) {
         buf.Format(KItem, i);
         cArray->AppendL(buf);
      }
      //Inform list box that data was added.
      iGrid->HandleItemAdditionL();

      SetGridGraphicStyleL();
   }
}

void CMainMenuGridContainer::SetGridLayoutL() 
{
   if (iGrid) {
      TRect rect = Rect();
      iCellSize.iWidth = (rect.iBr.iX - rect.iTl.iX) / iNumOfColumns;
      iCellSize.iHeight = (rect.iBr.iY - rect.iTl.iY) / iNumOfRows;

      TSize iconShrink = TSize(TInt(iCellSize.iWidth * CELL_ICON_SHRINK), 
                               TInt(iCellSize.iHeight * CELL_ICON_SHRINK));
      iIconRect = TRect(iCellSize);
      iIconRect.Shrink(iconShrink);

      iGrid->SetLayoutL(iVerticalOrientation, 
                        iLeftToRight, iTopToBottom, 
                        iNumOfColumns, iNumOfRows,
                        iCellSize);
      iGrid->SetPrimaryScrollingType(iVerticalScrollingType);
      iGrid->SetSecondaryScrollingType(iHorizontalScrollingType);
      DrawNow();
   }
}

void CMainMenuGridContainer::SetGridGraphicStyleL()
{
   if (iGrid) {
      AknListBoxLayouts::SetupStandardGrid(*iGrid);
      AknListBoxLayouts::SetupFormGfxCell(*iGrid, 
                                          iGrid->ItemDrawer(),
                                          0, iIconRect.iTl.iX, iIconRect.iTl.iY, 0, 0,  
                                          iIconRect.Width(),
                                          iIconRect.Height(),
                                          TPoint(0, 0), 
                                          TPoint(iIconRect.Width(),
                                                 iIconRect.Height()));
      DrawNow();
   }
}

void CMainMenuGridContainer::SetIconSizes()
{
#ifdef NAV2_CLIENT_SERIES60_V3
   CArrayPtr<CGulIcon>* icons = 
      iGrid->ItemDrawer()->FormattedCellData()->IconArray();
   if (icons) {
      for (TInt i = 0; i < icons->Count(); i++) {
         AknIconUtils::SetSize((*icons)[i]->Bitmap(), iIconRect.Size(),
                               EAspectRatioPreservedAndUnusedSpaceRemoved);
      }
   }
#endif
}

void CMainMenuGridContainer::UpdateGridLayoutL()
{
   SetGridLayoutL();
   SetGridGraphicStyleL();
   SetIconSizes();
}

void CMainMenuGridContainer::SizeChanged()
{
//    if (iGrid) {
//       iGrid->SetRect(Rect());
//       SetupGridItemsL();
      MakeGridL(Rect());

      UpdateGridLayoutL();

      iGrid->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
      
      iView->UpdateActiveSelection();
      //iGrid->ScrollBarFrame()->VerticalScrollBar()->MakeVisible(EFalse);
      //iGrid->ScrollBarFrame()->HorizontalScrollBar()->MakeVisible(EFalse);
//    }
}

void CMainMenuGridContainer::HandleResourceChange(TInt aType)
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
}

TInt CMainMenuGridContainer::CountComponentControls() const
{
   //Return number of controls inside this container
   return ENumberControls;
}

CCoeControl* CMainMenuGridContainer::ComponentControl(TInt aIndex) const
{
   switch (aIndex)
      {
      case EGrid:
         return iGrid;
      default:
         return NULL;
      }
}

void CMainMenuGridContainer::Draw(const TRect& /*aRect*/) const
{
   //No implementation required
}

TKeyResponse CMainMenuGridContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                                    TEventCode aType)
{
   if (aType == EEventKey && (aKeyEvent.iScanCode == EStdKeyEnter ||
                              aKeyEvent.iScanCode == EStdKeyDevice3)) {
      ActivateSelection();
      return EKeyWasConsumed;
   }
   TKeyResponse retVal = EKeyWasNotConsumed;
   if (iGrid) {
      retVal = iGrid->OfferKeyEventL(aKeyEvent, aType);
      if (retVal == EKeyWasConsumed && aType == EEventKey) {
         iView->UpdateSelectedIndex(GetActiveSelection());
         iView->SetNaviPaneLabelL(GetActiveSelection());
      }
   }
   return retVal;
}

void CMainMenuGridContainer::HandleControlEventL(CCoeControl* /*aControl*/, 
                                                 TCoeEvent /*aEventType*/)
{
   //No implementation required
}

void CMainMenuGridContainer::HandleListBoxEventL(CEikListBox* aListBox,
                                                 TListBoxEvent aEventType )
{
   if( aListBox == iGrid && aEventType == EEventItemClicked ){
      iView->UpdateSelectedIndex(GetActiveSelection());
      iView->SetNaviPaneLabelL(GetActiveSelection());
      // In new s60v5 the grids should activate directly instead of 
      // select first and then activate.
      ActivateSelection();
   } /* else if ( aListBox == iGrid && aEventType == EEventItemDoubleClicked) {
      ActivateSelection();
   } */
}

// End of File
