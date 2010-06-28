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

#if defined NAV2_CLIENT_SERIES60_V5
# include <aknpreviewpopupcontroller.h>
# include <aknstyluspopupmenu.h>
#endif

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "wficons.mbg"
//#include <STest.rsg>
//#include "STest.hrh"
//#include "STestForm.hrh"

#include "CSDetailedResultContainer.h"
#include "CSDetailedResultView.h"
#include "GfxUtility.h"
#include "MC2Coordinate.h"
#include "GuiProt/SearchPrintingPolicyFactory.h"
#include "WFTextUtil.h"
#include "WFLayoutUtils.h"
#include "WayFinderConstants.h"
#include "CombinedSearchDataTypes.h"
#include "PreviewPopUpContent.h"

#define MAX_HITS_PER_PAGE 25
#define ICON_SIZE TSize(10, 10)
#define PREV_TITLE R_CS_PREV_HITS_TITLE
#define NEXT_TITLE R_CS_NEXT_HITS_TITLE

CCSDetailedResultContainer::CCSDetailedResultContainer(CCSDetailedResultView* aView, 
                                                       TInt aOffset, 
                                                       TInt aIndex) : 
   iView(aView),
   iOffset(aOffset),
   iIndex(aIndex),
   iMaxHitsPerPage(MAX_HITS_PER_PAGE)
{
   iListBox = NULL;
}

CCSDetailedResultContainer::~CCSDetailedResultContainer()
{
   delete iListBox;
   iListBox = NULL;
   std::map<HBufC*, TInt, strCompareLess>::iterator it;
   for(it = iImageMap.begin(); it != iImageMap.end(); ++it) {
      delete it->first;
   }
   iImageMap.clear();
   if (iList) {
      iList->Reset();
      delete iList;
   }

#if defined NAV2_CLIENT_SERIES60_V5
   delete iPreviewContent;
   delete iPreviewController;
   delete iLongTapDetector;
   delete iPopUpMenu;
#endif

   delete iBgContext;
}
				
CCSDetailedResultContainer* CCSDetailedResultContainer::NewL( const TRect& aRect, 
                                                              CCSDetailedResultView* aView,
                                                              TInt aOffset,
                                                              TInt aIndex)
{
   CCSDetailedResultContainer* self = 
      CCSDetailedResultContainer::NewLC( aRect, aView, aOffset, aIndex );
   CleanupStack::Pop( self );
   return self;
}

CCSDetailedResultContainer* CCSDetailedResultContainer::NewLC( const TRect& aRect, 
                                                               CCSDetailedResultView* aView,
                                                               TInt aOffset,
                                                               TInt aIndex)
{
   CCSDetailedResultContainer* self = new ( ELeave ) CCSDetailedResultContainer(aView, 
                                                                                aOffset, 
                                                                                aIndex);
   CleanupStack::PushL( self );
   self->ConstructL( aRect );
   return self;
}
			
void CCSDetailedResultContainer::ConstructL( const TRect& aRect )
{
   CreateWindowL();

   iBgContext = 
      CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain,
                                               aRect, ETrue);

   InitializeControlsL(aRect);

#ifdef NAV2_CLIENT_SERIES60_V5
   iLongTapDetector = CAknLongTapDetector::NewL( this );
#endif

   SetRect( aRect );
   ActivateL();
}
			
TInt CCSDetailedResultContainer::CountComponentControls() const
{
   return ( int ) ELastControl;
}
				
CCoeControl* CCSDetailedResultContainer::ComponentControl( TInt aIndex ) const
{
   switch ( aIndex ) {
   case EListBox:
      return iListBox;
   }
	
   // handle any user controls here...
	
   return NULL;
}
				
void CCSDetailedResultContainer::SizeChanged()
{
   if(iBgContext) {
      iBgContext->SetRect(Rect());
      if (&Window()) {
         iBgContext->SetParentPos(PositionRelativeToScreen());
      }
   }
   CCoeControl::SizeChanged();

#if defined NAV2_CLIENT_SERIES60_V5
   if (iPreviewController) {
      TRect rect = Rect();
      TSize previewSize = iPreviewController->Size();
      TInt borderPadding = (previewSize.iWidth - 
                            iPreviewContent->Size().iWidth) / 2;
      if (WFLayoutUtils::LandscapeMode()) {
         iPreviewContent->SetAvailableWidth((rect.Width() / 2 - borderPadding * 2), 
                                            borderPadding, 
                                            CPreviewPopUpContent::EFourLinesTextImageTopLeftAbove,
                                            (rect.Height() - borderPadding * 2));
         iPreviewController->UpdateContentSize();
         TRect statusPaneRect;
         AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EStatusPane, 
                                           statusPaneRect);
         TPoint previewPos = rect.iTl;
         previewPos.iY += statusPaneRect.Height();
         // iPreviewController is not part of this container so we can't 
         // position it with 0,0 since that is the top of the screen and 
         // not the top corner of our container for the PreviewController.
         iPreviewController->SetPosition(previewPos);
         previewSize = iPreviewController->Size();
         // Reposition rect for the listbox.
         rect.iTl.iX += previewSize.iWidth;
      } else {
         iPreviewContent->SetAvailableWidth(rect.Width() - borderPadding * 2, 
                                            borderPadding,
                                            CPreviewPopUpContent::EFourLinesIndentedTextImageTopLeft);
         iPreviewController->UpdateContentSize();
         TRect statusPaneRect;
         AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EStatusPane, 
                                           statusPaneRect);
         TPoint previewPos = rect.iTl;
         previewPos.iY += statusPaneRect.Height();
         // iPreviewController is not part of this container so we can't 
         // position it with 0,0 since that is the top of the screen and 
         // not the top corner of our container for the PreviewController.
         iPreviewController->SetPosition(previewPos);
         previewSize = iPreviewController->Size();
         // Reposition rect for the listbox.
         rect.iTl.iY += previewSize.iHeight;
      }
      // iListBox is part of this container so position it relatively.
      iListBox->SetRect(rect);

      // Disable scrollbars per default.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOff);

      TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
      if (listBoxHeight > rect.Height()) {
         // Show scroll bar only if the height of the listbox is larger then
         // the rect.
         iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                             CEikScrollBarFrame::EOn);
         iListBox->UpdateScrollBarsL();
      }

      DrawDeferred();
   }
#else 
   TRect rect = TRect(TPoint(0, 0), iListBox->MinimumSize());
   iListBox->SetExtent(TPoint(0, 0), iListBox->MinimumSize());

   // Disable scrollbars per default.
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   
   TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
   if (listBoxHeight > rect.Height()) {
      // Show scroll bar only if the height of the listbox is larger then
      // the rect.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOn);
      iListBox->UpdateScrollBarsL();
   }
#endif
}

TTypeUid::Ptr CCSDetailedResultContainer::MopSupplyObject(TTypeUid aId)
{
   if (iBgContext) {
      return MAknsControlContext::SupplyMopObject(aId, iBgContext);
   }
   return CCoeControl::MopSupplyObject(aId);
}
			
TKeyResponse CCSDetailedResultContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                                          TEventCode aType )
{
   if( aKeyEvent.iCode == EKeyRightArrow ) {
      // Listbox takes all events even if it doesn't use them
      return EKeyWasNotConsumed;
   } else if ( aKeyEvent.iCode == EKeyLeftArrow ) {
      iView->HandleCommandL(EAknSoftkeyBack);
      return EKeyWasConsumed;
   }

   if ( iListBox ) {
      TKeyResponse retVal = EKeyWasNotConsumed;
      retVal = iListBox->OfferKeyEventL(aKeyEvent, aType);
      if (retVal == EKeyWasConsumed && aType == EEventKey) {
         UpdatePreviewContentL();
      }
      return retVal;
   }
   return CCoeControl::OfferKeyEventL( aKeyEvent, aType );
}

void CCSDetailedResultContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                                      TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventEnterKeyPressed || 
       aListBox == iListBox && aEventType == EEventItemDoubleClicked ) {
      TPtrC str = 
         iListBox->Model()->MatchableTextArray()->MdcaPoint(iListBox->CurrentItemIndex());
      if ( CompareToResource( str, PREV_TITLE ) ) {
         // If user clicked on the "previous" text, show 
         // the previous 25 results
         ShowPreviousResults();
      } else if ( CompareToResource( str, NEXT_TITLE ) ) {
         // If user clicked on the "next" text, show 
         // the next 25 results
         ShowNextResults();
      } else if( iView->SettingOrigin() ) {
         // In case of coming from the route planner and selected
         // set as origin, send this command to the view
         iView->HandleCommandL( EWayFinderCmdNewDestOrigin );
      } else if( iView->SettingDestination() ) {
         // In case of coming from the route planner and selected
         // set as destination, send this command to the view
         iView->HandleCommandL( EWayFinderCmdNewDestDestination );
      } else /*if (iView->IsGpsAllowed())*/ {
         // Let the view handle this command, if there is a gps
         // connected and we get valid positions from it, this command
         // will do a navigate to else we start the gps conn wizard.
#ifdef NAV2_CLIENT_SERIES60_V5
         if (WFLayoutUtils::IsTouchScreen()) {
            // If a dialog gets displayed the EButton1Up does not happen 
            // here in HandlePointerEventL so we need to manually cancel 
            // the long tap detector.
            iLongTapDetector->Cancel();
         }
#endif
         iView->HandleCommandL( EWayFinderCmdNewDestNavigate );
      } /*else {
         // If none of the above we show information in service window.
         iView->HandleCommandL( EWayFinderCmdNewDestShowInfo );
      }*/
   }
}

void CCSDetailedResultContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
#ifdef NAV2_CLIENT_SERIES60_V5
   if ( AknLayoutUtils::PenEnabled() ) {
      if (!(aPointerEvent.iType == TPointerEvent::EButton1Up && iPopUpMenuDisplayed)) {
         // If the type is EButton1Up and the popup menu has been launched this
         // event should not be forwarded to the listbox, if so the popup menu
         // will not be displayed and the listbox will interprete it as a click.
         // E.g. If item 4 is selected and you press a long tap on the already selected
         // item, the listbox will handle the button up as a double click.
         CCoeControl::HandlePointerEventL(aPointerEvent);
      }
      
      if (aPointerEvent.iType == TPointerEvent::EButton1Down) {
         UpdatePreviewContentL();
         iPopUpMenuDisplayed = EFalse;
      }
      if (!IndexIsPrev() && !IndexIsNext()) {
         // No long tap on prev or next
         iLongTapDetector->PointerEventL( aPointerEvent );   
      }
   }
#endif
}

void CCSDetailedResultContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, 
                                                      const TPoint& aPenEventScreenLocation )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   iPopUpMenuDisplayed = ETrue;
   if ( !iPopUpMenu ) {
       // Launch stylus popup menu here
       // Construct the menu from resources
       iPopUpMenu = CAknStylusPopUpMenu::NewL( iView, aPenEventScreenLocation, NULL );
       TResourceReader reader;
       iCoeEnv->CreateResourceReaderLC( reader , R_CS_POP_UP_STYLUS_MENU );
       iPopUpMenu->ConstructFromResourceL( reader );
       CleanupStack::PopAndDestroy(); // reader
    }
    iPopUpMenu->SetPosition( aPenEventScreenLocation );

    // Hide/show set as origin depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestOrigin, !iView->SettingOrigin());
    // Hide/show set as dest depending on the view state.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestDestination, !iView->SettingDestination());
    // Hide/show navigate to depending on the view state, should not be visible when
    // set as orig or set as dest is true.
    iPopUpMenu->SetItemDimmed(EWayFinderCmdNewDestNavigate, (iView->SettingOrigin() || 
                                                             iView->SettingDestination()));

    iPopUpMenu->ShowMenu();
#endif
}


TBool CCSDetailedResultContainer::CompareToResource( const TDesC& aString, 
                                                     TInt aResourceId )
{
   HBufC* tmp = CCoeEnv::Static()->AllocReadResourceLC( aResourceId );
   TBool match = (aString.Find(*tmp) != KErrNotFound);
   CleanupStack::PopAndDestroy( tmp );
   return match;
}

TBool CCSDetailedResultContainer::IndexIsPrev()
{
   TPtrC str = 
      iListBox->Model()->MatchableTextArray()->MdcaPoint(iListBox->CurrentItemIndex());
   return CompareToResource( str, PREV_TITLE );
}

TBool CCSDetailedResultContainer::IndexIsNext()
{
   TPtrC str = 
      iListBox->Model()->MatchableTextArray()->MdcaPoint(iListBox->CurrentItemIndex());
   return CompareToResource( str, NEXT_TITLE );
}

/**
 *	Initialize each control upon creation.
 */				
void CCSDetailedResultContainer::InitializeControlsL(const TRect& aRect)
{
#if defined NAV2_CLIENT_SERIES60_V5
   TRect contentRect(aRect.iTl, TSize(aRect.Width(), aRect.Height() / 3));
   iPreviewContent = CPreviewPopUpContent::NewL();
   iPreviewController = 
      CAknPreviewPopUpController::NewL(*iPreviewContent, 
                                       CAknPreviewPopUpController::EPermanentMode |
                                       CAknPreviewPopUpController::ELayoutDefault);
   iPreviewContent->InitialiseL(aRect,
                                     iView->GetMbmName(),
                                     4,
                                     EMbmWficonsTat_default, 
                                     EMbmWficonsTat_default_mask);
   iPreviewController->SetPopUpShowDelay(0);
   iPreviewController->ShowPopUp();
   TSize previewSize = iPreviewController->Size();
   TInt borderPadding = (previewSize.iWidth - contentRect.Width()) / 2;
   if (WFLayoutUtils::LandscapeMode()) {
      iPreviewContent->
         SetAvailableWidth(contentRect.Width() - borderPadding * 2, 
                           borderPadding,
                           CPreviewPopUpContent::EFourLinesTextImageTopLeftAbove);
   } else {
      iPreviewContent->
         SetAvailableWidth(contentRect.Width() - borderPadding * 2, 
                           borderPadding,
                           CPreviewPopUpContent::EFourLinesIndentedTextImageTopLeft);
   }
   //iPreviewContent->SetTextL(KFirstText, KSecondText, KThirdText);
   iPreviewController->UpdateContentSize();
   iPreviewController->SetPosition(aRect.iTl);
   iPreviewController->HidePopUp();
#endif

#if defined NAV2_CLIENT_SERIES60_V5
   iListBox = new( ELeave ) CAknSingleGraphicStyleListBox();
#else
   iListBox = new( ELeave ) CAknDoubleGraphicStyleListBox();
#endif
   iListBox->SetContainerWindowL( *this );
   iListBox->SetMopParent(this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC( reader, R_WAYFINDER_CS_DETAILED_RESULT_LISTBOX );
      iListBox->ConstructFromResourceL( reader );
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   // the listbox owns the items in the list and will free them
   iListBox->Model()->SetOwnershipType( ELbmDoesNotOwnItemArray );
	
   iListBox->SetFocus( ETrue );
   iListBox->SetListBoxObserver(this);

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EAuto);
#if defined NAV2_CLIENT_SERIES60_V5
   iListBox->ItemDrawer()->ColumnData()->SetMarqueeParams(5, 6, 1000000, 200000);
   iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
#else
   iListBox->ItemDrawer()->FormattedCellData()->SetMarqueeParams(5, 3, 1000000, 200000);
   iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);
#endif
   iListBox->UpdateScrollBarsL();

   iIcons = new (ELeave) CAknIconArray(5);

   CFbsBitmap* bitmap;
   CFbsBitmap* mask;

   AknIconUtils::CreateIconL( bitmap, mask, iView->GetMbmName(), 
                              EMbmWficonsBack, EMbmWficonsBack_mask );
   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );
   iIcons->AppendL(CGulIcon::NewL( bitmap, mask ));

   AknIconUtils::CreateIconL( bitmap, mask, iView->GetMbmName(), 
                              EMbmWficonsForward, EMbmWficonsForward_mask );
   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );
   iIcons->AppendL(CGulIcon::NewL( bitmap, mask ));
}

/** 
 * Handle global resource changes, such as scalable UI or skin events (override)
 */
void CCSDetailedResultContainer::HandleResourceChange( TInt aType )
{
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }
   CCoeControl::HandleResourceChange(aType);
}
				
/**
 *	Draw container contents.
 */				
void CCSDetailedResultContainer::Draw( const TRect& aRect ) const
{
   CWindowGc& gc = SystemGc();
   //gc.Clear( aRect );

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);
}				

CGulIcon* CCSDetailedResultContainer::LoadAndScaleIconL( const TDesC& aFileName )
{
   CFbsBitmap* bitmap;
   CFbsBitmap* mask;

   LoadAndScaleImageL(aFileName, bitmap, mask);

   return CGulIcon::NewL( bitmap, mask );
}

void CCSDetailedResultContainer::LoadAndScaleImageL(const TDesC& aFileName,
                                                    CFbsBitmap*& aBitmap,
                                                    CFbsBitmap*& aMask)
{
   TFileName completeName = iView->GetCSIconPath();
   completeName.Append(aFileName);
   completeName.Append(KMifExt);

   TInt bitmapId = 0;
   TInt maskId = 1;

   class RFs fs;
   fs.Connect();
   class RFile imgFile;
   TInt ret = imgFile.Open(fs, completeName, EFileRead);
   if(ret == KErrNone || ret == KErrInUse) {
      imgFile.Close();
      AknIconUtils::ValidateLogicalAppIconId(completeName, bitmapId, maskId);
      AknIconUtils::CreateIconL(aBitmap, aMask, completeName, 
                                bitmapId, maskId);
   } else {
      // Change completeName to fallback icon
      completeName.Copy(iView->GetMbmName());
      AknIconUtils::CreateIconL(aBitmap, aMask, completeName, 
                                EMbmWficonsTat_default, 
                                EMbmWficonsTat_default_mask); 
   }
   fs.Close();

   AknIconUtils::SetSize(aBitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                         EAspectRatioPreservedAndUnusedSpaceRemoved);
}

_LIT(KCommaSpace, ", ");
void CCSDetailedResultContainer::AddListBoxItemL( const isab::SearchItem& item, 
                                                  const char* aImageName )
{
   // Get or create the image.
   HBufC* imgName = WFTextUtil::AllocL(aImageName);
   std::map<HBufC*, TInt, strCompareLess>::const_iterator it = 
      iImageMap.find(imgName);
   std::pair<HBufC*, TInt> imgPair;
   if (it == iImageMap.end()) {
      iIcons->AppendL(LoadAndScaleIconL(*imgName));
      imgPair = std::make_pair(imgName, iIcons->Count()-1);
      iImageMap.insert( imgPair );
   } else {
      imgPair = *it;
      delete imgName;
   }

   HBufC* listString;
   char* dist = NULL;
   char* parsed = NULL;
#if defined NAV2_CLIENT_SERIES60_V5
   listString = HBufC::NewLC(strlen(item.getName()) + 32); // extra space for tab and number length
   TPtr listStringPtr = listString->Des();

   listStringPtr.Num(imgPair.second);
   listStringPtr.Append(KTab);
   WFTextUtil::TDesAppend(listStringPtr, item.getName());
#else
   parsed = 
      isab::SearchPrintingPolicyFactory::parseSearchItem(&item, 1,
                                                         isab::DistancePrintingPolicy::
                                                         DistanceMode(iView->GetDistanceMode()));
   MC2Coordinate searchPos = 
      MC2Coordinate(Nav2Coordinate(item.getLat(), item.getLon()));
   if (iView->IsGpsAllowed() && iView->ValidGpsStrength() && searchPos.isValid()) {
      // If we have valid gps coords and the search hit has valid coords, 
      // print the distance between them at the end of the description.
      TPoint nav2Pos = iView->GetCurrentPosition();
      MC2Coordinate currPos = 
         MC2Coordinate(Nav2Coordinate(nav2Pos.iY, nav2Pos.iX));
      dist = 
         isab::DistancePrintingPolicy::
         convertDistance(TUint32(GfxUtility::rtP2Pdistance_linear(currPos, searchPos)),
                         isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode()),
                         DistancePrintingPolicy::Round);
      listString = HBufC::NewLC(strlen(parsed) + strlen(dist) + 10);
   } else {
      listString = HBufC::NewLC(strlen(parsed) + 8);
   }

   TPtr listStringPtr = listString->Des();

   listStringPtr.Num(imgPair.second);
   WFTextUtil::TDesAppend(listStringPtr, parsed);
   if (dist) {
      if (item.noRegions() > 0) {
         listStringPtr.Append(KCommaSpace);
      }
      WFTextUtil::TDesAppend(listStringPtr, dist);
   }
#endif

   iList->AppendL( *listString );
   delete[] dist;
   delete[] parsed;
   CleanupStack::PopAndDestroy(listString);
}

#if defined NAV2_CLIENT_SERIES60_V5
_LIT( KStringHeader, "%d\t%S" );
#else
_LIT( KStringHeader, "%d\t%S\t%S" );
#endif
void CCSDetailedResultContainer::AddListBoxItemL( TInt aResourceHeaderId,
                                                  TInt aResourceTextId, 
                                                  TInt aImageId,
                                                  TInt aMaskId )
{
   HBufC* header = CCoeEnv::Static()->AllocReadResourceLC( aResourceHeaderId );
   HBufC* text = CCoeEnv::Static()->AllocReadResourceLC( aResourceTextId );
   HBufC* listString = HBufC::NewLC(header->Length() + text->Length() + 8);
   TPtr listStringPtr = listString->Des();

   CFbsBitmap* bitmap;
   CFbsBitmap* mask;
   AknIconUtils::CreateIconL( bitmap, mask, iView->GetMbmName(), aImageId, aMaskId );
   AknIconUtils::SetSize( bitmap, WFLayoutUtils::CalculateSizeUsingMainPane(ICON_SIZE), 
                          EAspectRatioPreservedAndUnusedSpaceRemoved );
   iIcons->AppendL(CGulIcon::NewL( bitmap, mask ));
   
#if defined NAV2_CLIENT_SERIES60_V5
   listStringPtr.Format(KStringHeader(), iIcons->Count()-1, header);
#else
   listStringPtr.Format(KStringHeader(), iIcons->Count()-1, header, text);
#endif

   iList->AppendL( *listString );

   CleanupStack::PopAndDestroy(listString);
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(header);
}

void CCSDetailedResultContainer::AddListBoxItemL( TInt aResourceHeaderId,
                                                  TInt aResourceTextId, 
                                                  TInt aImageMapId )
{
   HBufC* header = CCoeEnv::Static()->AllocReadResourceLC( aResourceHeaderId );
   HBufC* text = CCoeEnv::Static()->AllocReadResourceLC( aResourceTextId );
   HBufC* listString = HBufC::NewLC(header->Length() + text->Length() + 8);
   TPtr listStringPtr = listString->Des();

#if defined NAV2_CLIENT_SERIES60_V5
   listStringPtr.Format(KStringHeader(), aImageMapId, header);
#else
   listStringPtr.Format(KStringHeader(), aImageMapId, header, text);
#endif

   iList->AppendL( *listString );

   CleanupStack::PopAndDestroy(listString);
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(header);
}

TInt 
CCSDetailedResultContainer::AddResultsL( TInt aIndex, 
                                         const CSCatVec_t& aResults,
                                         TBool aMoreServerHitsReceived )
{
   if (aIndex >= (TInt)aResults.size()) {
      // Out of bounds
      return -1;
   }

   if (aMoreServerHitsReceived) {
      // We had to request hits from server before and then the offset update 
      // needs to be delayed until here (when we got more results).
      iOffset++;
   }

   CSSIVec_t results = aResults[aIndex]->getResults();

   if (iOffset == 0 && MAX_HITS_PER_PAGE > results.size()) {
      // Make sure we dont take for granted that each page
      // contains 25 hits. If doing that we will not request
      // correct indexes from the server
      iMaxHitsPerPage = results.size();
   } else if (iOffset == 0 && MAX_HITS_PER_PAGE <= results.size()) {
      iMaxHitsPerPage = MAX_HITS_PER_PAGE;
   }

   TInt lowerBound = iOffset * iMaxHitsPerPage;
   TInt upperBound = ( iOffset + 1 ) * iMaxHitsPerPage - 1;
   TInt ret = LastPageGetMoreHits(aIndex, aResults, lowerBound, upperBound);
   
   if (ret == EHasMoreHits || ret == ENoMoreHits) {
      if (iList) {
         iList->Reset();
         delete iList;
      }   
      iList = new (ELeave) CDesCArrayFlat(25);
      if (iOffset > 0) {
         AddListBoxItemL(PREV_TITLE, R_CS_PREV_HITS_TEXT, 0 );
      }
      for( TInt i = lowerBound; i <= upperBound; ++i) {
         // Get the search hits within a certain range. iOffset is 0 for the
         // first 25 hits, 1 for hits 26 - 50 and so on.
         isab::SearchItem item = results.at(i);
         AddListBoxItemL(item, item.getImageName());
      }
      if (ret != ENoMoreHits) {
         AddListBoxItemL(NEXT_TITLE, R_CS_NEXT_HITS_TEXT, 1 );
      }

      if (iList->Count() > 0 && iIcons->Count() > 0) {
         iListBox->ItemDrawer()->ColumnData()->SetIconArray( iIcons );
         CTextListBoxModel* model = iListBox->Model();
         model->SetItemTextArray(iList);
         iListBox->HandleItemAdditionL();
         iListBox->SetCurrentItemIndex(iIndex);
         iListBox->DrawNow();
      }
   } else if (ret == ERequestingHitsFromServer) {
      
   } else if (ret == ENoMoreHits) {
      
   }

   UpdatePreviewContentL();
   
   // Disable scrollbars per default.
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   TRect rect = TRect(TPoint(0, 0), iListBox->Size());
   TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
   if (listBoxHeight > rect.Height()) {
      // Show scroll bar only if the height of the listbox is larger then
      // the rect.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOn);
   }
   iListBox->UpdateScrollBarsL();

   return ret;
}

TInt 
CCSDetailedResultContainer::LastPageGetMoreHits(TInt aCatIndex, 
                                                const CSCatVec_t& aCatResults, 
                                                TInt& aLowerBound,
                                                TInt& aUpperBound)
{
   // Get the search results for the specified category
   iTotalNbrHits = aCatResults.at(aCatIndex)->getTotalNbrHits();
   CSSIVec_t searchItemsVec = aCatResults.at(aCatIndex)->getResults();
   TInt size = searchItemsVec.size();
   if (aUpperBound >= size) {
      // Make sure we not get an out of bounds later when iterating through
      // the results in AddResultsL.
      aUpperBound = size-1;
   }
   if (aLowerBound >= size) {
      // Last page is displayed, check if there are more hits on the server.
      if ( size < iTotalNbrHits ) {
         // Request from the server;
         aLowerBound = size - 1;
         iView->RequestMoreHits(size, aCatResults.at(aCatIndex)->getHeadingNo());
         return ERequestingHitsFromServer;
      } else {
         return ENoMoreHits;
      }
   }
   if (  size < iTotalNbrHits || 
         aUpperBound < (size-1) ) {
      // number of fetched search items is less than total number of hits
      // for the specified category
      return EHasMoreHits;
   } else {
      return ENoMoreHits;
   }
}

void CCSDetailedResultContainer::ShowNextResults()
{
   iIndex = 0;
   iOffset++;
   if (AddResultsL(iView->GetCSCategoryCurrentIndex(), 
                   iView->GetSearchResults()) == ERequestingHitsFromServer) {
      // Since there is a delay when requesting more hits from the server we 
      // still show the current page with results until the additional hits 
      // has been received from server.
      // This means that we have to set back the offset again and then 
      // increase it when the search hits are received in AddResultsL.
      iOffset--;
   }
}

void CCSDetailedResultContainer::ShowPreviousResults()
{
   iIndex = 0;
   iOffset--;
   AddResultsL(iView->GetCSCategoryCurrentIndex(), iView->GetSearchResults());
}

TBool CCSDetailedResultContainer::HasResults()
{
   return iListBox && (iListBox->Model()->NumberOfItems() > 0);
}

TInt CCSDetailedResultContainer::GetCalculatedIndex()
{
   TInt index = iOffset * iMaxHitsPerPage + iListBox->CurrentItemIndex();
   if (iOffset > 0) {
      // We need to compensate for the prev text in the list box, this
      // is located on index 0 if offset is larger than 0.
      index--;
   }
   return index;
}

void CCSDetailedResultContainer::GetIndexes(TInt& aOffset, TInt& aIndex)
{
   aOffset = iOffset;
   if (iListBox) {
      aIndex = iListBox->CurrentItemIndex();
   } else {
      aIndex = 0;
   }
}

void CCSDetailedResultContainer::UpdatePreviewContentL()
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (IndexIsPrev()) {
      // If user clicked on the "previous" text.
      HBufC* header = CCoeEnv::Static()->AllocReadResourceLC(PREV_TITLE);
      HBufC* text = CCoeEnv::Static()->AllocReadResourceLC(R_CS_PREV_HITS_TEXT);
      iPreviewContent->SetImageL(EMbmWficonsBack, EMbmWficonsBack_mask);
      iPreviewContent->SetTextL(*header, *text, KNullDesC());
      CleanupStack::PopAndDestroy(text);
      CleanupStack::PopAndDestroy(header);
   } else if (IndexIsNext()) { 
      // If user clicked on the "next" text.
      HBufC* header = CCoeEnv::Static()->AllocReadResourceLC(NEXT_TITLE);
      HBufC* text = CCoeEnv::Static()->AllocReadResourceLC(R_CS_NEXT_HITS_TEXT);
      iPreviewContent->SetImageL(EMbmWficonsForward, EMbmWficonsForward_mask);
      iPreviewContent->SetTextL(*header, *text, KNullDesC());
      CleanupStack::PopAndDestroy(text);
      CleanupStack::PopAndDestroy(header);
   } else {
      // User selected a real search item.
      const isab::SearchItem& item = 
         iView->GetSelectedSearchItem(GetCalculatedIndex());

      HBufC* imgName = WFTextUtil::AllocL(item.getImageName());
      CFbsBitmap* bitmap;
      CFbsBitmap* mask;
      LoadAndScaleImageL(*imgName, bitmap, mask);
      // SetImageL takes ownership.
      iPreviewContent->SetImageL(bitmap, mask);
      bitmap = NULL;
      mask = NULL;
      delete imgName;

      HBufC* name = WFTextUtil::AllocL(item.getName());
      if (!name) {
         name = KNullDesC().AllocL();
      }
      // Parse out the city and adress fields from the searh item.
      char* descTxt = 
         isab::SearchPrintingPolicyFactory::parseSearchItem(&item, false,
                                   isab::DistancePrintingPolicy::
                                   DistanceMode(iView->GetDistanceMode()),
                                   false);
      HBufC* desc = WFTextUtil::AllocL(descTxt);
      if (!desc) {
         desc = KNullDesC().AllocL();
      }
      delete[] descTxt;

      HBufC* distance = NULL;
      MC2Coordinate searchPos = 
         MC2Coordinate(Nav2Coordinate(item.getLat(), item.getLon()));
      if (iView->IsGpsAllowed() && iView->ValidGpsStrength() && 
          searchPos.isValid()) {
         // If we have valid gps coords and the fav has valid coords,
         // print the distance between them.
         TPoint nav2Pos = iView->GetCurrentPosition();
         MC2Coordinate currPos = 
            MC2Coordinate(Nav2Coordinate(nav2Pos.iY, nav2Pos.iX));
         char* dist = 
            isab::DistancePrintingPolicy::
            convertDistance(TUint32(GfxUtility::rtP2Pdistance_linear(currPos, searchPos)),
                            isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode()),
                            DistancePrintingPolicy::Round);
         distance = WFTextUtil::AllocL(dist);
         delete[] dist;
      }
      if (!distance) {
         distance = KNullDesC().AllocL();
      }

      iPreviewContent->SetTextL(*name, *desc, *distance);
      delete distance;
      delete desc;
      delete name;
   }
   iPreviewContent->DrawDeferred();
#endif
}

void CCSDetailedResultContainer::ShowPreviewContent(TBool aShow)
{
#if defined NAV2_CLIENT_SERIES60_V5
   if (!iPreviewController) {
      return;
   }
   if (aShow) {
      iPreviewController->ShowPopUp();
   } else {
      iPreviewController->HidePopUp();
   }
#else
   aShow = aShow;
#endif
}

