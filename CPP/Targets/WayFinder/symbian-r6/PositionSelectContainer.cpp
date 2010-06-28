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
#include <eikclbd.h>
#include <barsread.h>     // for resource reader
#include <aknlists.h>     // for avrell style listbox
#include <akniconarray.h> // for icon array
#include <aknpopup.h>     // for pop up menu
#include <stringloader.h> 
#include <coecontrolarray.h>
#include <aknsbasicbackgroundcontrolcontext.h>

#ifdef NAV2_CLIENT_SERIES60_V3
# include <gulicon.h>
# include <akniconutils.h>
#endif

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknstyluspopupmenu.h>
# include <aknbutton.h> 
#endif

#include <aknlistquerydialog.h> 

#include "RsgInclude.h"
#include "wficons.mbg"

#include "GuiDataStore.h"
#include "PositionSelectView.h"
#include "PositionSelectContainer.h"
#include "WayFinderConstants.h"
#include "wayfinder.hrh"

#include "PopUpList.h"
#include "WFLayoutUtils.h"
#include "PathFinder.h"
#include "NavServerComEnums.h"
// Definitions
#define LISTBOX_POS     TPoint(10, 15)

enum TControls
{
   EListBox,
   ERouteAsCarButton/*,
   ERouteAsPedButton*/
};

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPositionSelectContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CPositionSelectContainer::ConstructL( const TRect& aRect, 
                                           CPositionSelectView* aView )
{
   CreateWindowL();

   InitComponentArrayL();

   iView = aView;

   iBgContext = 
      CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain,
                                               aRect, ETrue);

   GuiDataStore* gds = iView->GetGuiDataStore();
   HBufC* wfmbmname = gds->iWayfinderMBMFilename;

   iListBox = new( ELeave ) CAknDoubleLargeStyleListBox();
   LOGNEW(iListBox, CAknDoubleLargeStyleListBox);
   Components().AppendLC(iListBox, EListBox);
   iListBox->SetContainerWindowL(*this);
   iListBox->SetMopParent(this);
   {
      TResourceReader reader;
      iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_POSITIONSELECT_LIST );
      iListBox->ConstructFromResourceL(reader);
      CleanupStack::PopAndDestroy();  // Resource reader
   }

   iListBox->SetListBoxObserver(this);
   iListBox->SetFocus(ETrue);

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                       CEikScrollBarFrame::EOff);
   iListBox->UpdateScrollBarsL();

   // Creates gul icon.
   CArrayPtr<CGulIcon>* icons = new( ELeave ) CAknIconArray( 2 );
   LOGNEW(icons, CAknIconArray);

   CleanupStack::PushL( icons );

   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSet_as_start, 
                 EMbmWficonsSet_as_start_mask);

   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSet_as_destination, 
                 EMbmWficonsSet_as_destination_mask);

   // Sets graphics as ListBox icon.
   iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );
   //iListBox->ItemDrawer()->FormattedCellData()->SetIconArrayL(icons);

   CleanupStack::Pop(icons); //icons
   CleanupStack::Pop(iListBox);

#ifdef NAV2_CLIENT_SERIES60_V5
   if(AknLayoutUtils::PenEnabled()) {
      TRect buttonRect = CreateAndPositionButtonsL();
      // Set the rect of the listbox to be the main pane rect minus the 
      // height of the complete button rect.
      TRect listBoxRect(aRect.iTl, TSize(aRect.Width(), aRect.Height() - buttonRect.Height()));
      iListBox->SetRect(listBoxRect);
   } else {
      iListBox->SetRect(aRect);
   }

   /*iTitleLabel = new (ELeave) CEikLabel();
   iTitleLabel->SetContainerWindowL(*this);
   Components().AppendLC(iTitleLabel);

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   TRgb fgcolor(0,0,0);
   AknsUtils::GetCachedColor(skin, fgcolor, KAknsIIDQsnTextColors, 
                             EAknsCIQsnTextColorsCG6);
   iTitleLabel->OverrideColorL(EColorLabelText, fgcolor);

   iTitleLabel->SetLabelAlignment(ELayoutAlignCenter);
   HBufC* titleText = StringLoader::LoadLC(R_WF_ROUTE_BUTTON_TITLE);
   iTitleLabel->SetTextL(*titleText);
   CleanupStack::PopAndDestroy(titleText);
   CleanupStack::Pop(iTitleLabel);*/

   iLongTapDetector = CAknLongTapDetector::NewL( this );
#else
   iListBox->SetRect(aRect);
#endif

   SetRect(aRect);
   ActivateL();
}

#ifdef NAV2_CLIENT_SERIES60_V5
TRect CPositionSelectContainer::CreateAndPositionButtonsL()
{
   HBufC* text = StringLoader::LoadLC(R_WF_ROUTE_CAR_BUTTON_TXT);
   iCarButton = CAknButton::NewL(iView->GetMbmName(),
                                 EMbmWficonsBy_car, EMbmWficonsBy_car_mask,
                                 EMbmWficonsBy_car, EMbmWficonsBy_car_mask,
                                 EMbmWficonsBy_car, EMbmWficonsBy_car_mask,
                                 EMbmWficonsBy_car, EMbmWficonsBy_car_mask,
                                 *text, KNullDesC, KAknButtonKeyRepeat, 0);
   CleanupStack::PopAndDestroy(text);
   Components().AppendLC(iCarButton, ERouteAsCarButton);
   iCarButton->SetTextAndIconAlignment(CAknButton::EIconBeforeText);
   iCarButton->SetContainerWindowL(*this);
   iCarButton->SetIconScaleMode(EAspectRatioPreserved);
   iCarButton->SetTextHorizontalAlignment(CGraphicsContext::ELeft);
   iCarButton->SetObserver(this);

   /*text = StringLoader::LoadLC(R_WF_ROUTE_PED_BUTTON_TXT);
   iPedButton = 
      CAknButton::NewL(iView->GetMbmName(),
                       EMbmWficonsPedestrian_mode, EMbmWficonsPedestrian_mode_mask,
                       EMbmWficonsPedestrian_mode, EMbmWficonsPedestrian_mode_mask,
                       EMbmWficonsPedestrian_mode, EMbmWficonsPedestrian_mode_mask,
                       EMbmWficonsPedestrian_mode, EMbmWficonsPedestrian_mode_mask,
                       *text, KNullDesC, KAknButtonKeyRepeat, 0);
   CleanupStack::PopAndDestroy(text);
   Components().AppendLC(iPedButton, ERouteAsPedButton);
   iPedButton->SetTextAndIconAlignment(CAknButton::EIconBeforeText);
   iPedButton->SetContainerWindowL(*this);
   iPedButton->SetIconScaleMode(EAspectRatioPreserved);
   iPedButton->SetTextHorizontalAlignment(CGraphicsContext::ELeft);
   iPedButton->SetObserver(this);

   TRect buttonRect = PositionButtonsL(iPedButton, iCarButton);*/
   TRect buttonRect = PositionButtonL(iCarButton);
   iCarButton->MakeVisible(ETrue);
   iCarButton->ActivateL();
   /*iPedButton->MakeVisible(ETrue);
   iPedButton->ActivateL();
   CleanupStack::Pop(iPedButton);*/
   CleanupStack::Pop(iCarButton);
   return buttonRect;
}
#endif

// Destructor
CPositionSelectContainer::~CPositionSelectContainer()
{
   iView = NULL;
   delete iBgContext;

#ifdef NAV2_CLIENT_SERIES60_V5
   delete iLongTapDetector;
   delete iPopUpMenu;
#endif
}

//------------------------------------------------------------------
// CPositionSelectContainer::ShowPopupList()
// shows "List pane for single item"
//------------------------------------------------------------------
void CPositionSelectContainer::ShowPopupListL()
{
   // Set listitems.
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(5);
   LOGNEW(descArray, CDesCArrayFlat);
   CleanupStack::PushL( descArray );
   HBufC* item;

   if (iView->AreSet()) {
      item = iCoeEnv->AllocReadResourceLC(R_POSITIONSELECT_GETROUTE_TEXT);
      descArray->AppendL(*item);
      CleanupStack::PopAndDestroy(item);
   }
   if (iView->IsGpsConnected() && (GetActiveSelection() == 0)) {
      item = iCoeEnv->AllocReadResourceLC( R_CONNECT_PERSONAL_CURRENT_POSITION );
      descArray->AppendL( *item );
      CleanupStack::PopAndDestroy(item);
   }
   item = iCoeEnv->AllocReadResourceLC( R_POSITIONSELECT_SEARCH_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   item = iCoeEnv->AllocReadResourceLC( R_POSITIONSELECT_FAVORITE_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);
   item = iCoeEnv->AllocReadResourceLC( R_POSITIONSELECT_FROM_MAP_TEXT );
   descArray->AppendL( *item );
   CleanupStack::PopAndDestroy(item);

   TBool okChosen;
   TInt index = PopUpList::ShowPopupListL(R_WAYFINDER_SOFTKEYS_OK_CANCEL, 
                                          *descArray, okChosen );

   // Show popup list
   if( okChosen ){
      if (!iView->AreSet()) {
         index++;
      }
      if ((!iView->IsGpsConnected() || (GetActiveSelection() != 0)) && index >= 1) {
         index++;
      }
      TInt command;
      switch( index )
      {
      case 0:
         command = EWayFinderCmdPositionSelectGetRoute;
         break;
      case 1:
         command = EWayFinderCmdPositionSelectFromGps;
         break;
      case 2:
         command = EWayFinderCmdPositionSelectSearch;
         break;
      case 3:
         command = EWayFinderCmdPositionSelectFavorite;
         break;
      case 4:
         command = EWayFinderCmdPositionSelectFromMap;
         break;
      default:
         command = EWayFinderCmdPositionSelectSearch;
      }
      iView->HandleCommandL( command );
   }
   CleanupStack::PopAndDestroy(descArray);
}

void CPositionSelectContainer::ShowStylusPopUpMenuL(const TPoint aLocation)
{
#if defined NAV2_CLIENT_SERIES60_V5
   iPopUpMenuDisplayed = ETrue;
   if ( !iPopUpMenu ) {
      // Launch stylus popup menu here
      // Construct the menu from resources      
      iPopUpMenu = CAknStylusPopUpMenu::NewL( iView, aLocation, NULL );
      TResourceReader reader;
      iCoeEnv->CreateResourceReaderLC( reader , R_PS_POP_UP_STYLUS_MENU );
      iPopUpMenu->ConstructFromResourceL( reader );
      CleanupStack::PopAndDestroy(); // reader
   }
   iPopUpMenu->SetPosition( aLocation );

   if (iView->IsGpsConnected() && (GetActiveSelection() == 0)) {
      iPopUpMenu->SetItemDimmed(EWayFinderCmdPositionSelectFromGps, EFalse);
   } else {
      iPopUpMenu->SetItemDimmed(EWayFinderCmdPositionSelectFromGps, ETrue);      
   }

   iPopUpMenu->ShowMenu();
#endif
}

_LIT ( KStringHeader, "%d\t%S" );
TInt CPositionSelectContainer::ShowRouteAsQueryDialogL()
{
   CAknIconArray* icons = new (ELeave) CAknIconArray(2);
   CleanupStack::PushL(icons);

   CDesCArrayFlat* optionsList = new (ELeave) CDesCArrayFlat(2);
   CleanupStack::PushL(optionsList);

   CFbsBitmap* bitmap;
   CFbsBitmap* mask;
   AknIconUtils::CreateIconL(bitmap, mask,
                             iView->GetMbmName(),
                             EMbmWficonsBy_car, 
                             EMbmWficonsBy_car_mask);
   // icons will take ownership of bitmap and mask
   icons->AppendL(CGulIcon::NewL(bitmap, mask));

   // Format the string to be inserted into the text array
   HBufC* text = StringLoader::LoadLC(R_WF_ROUTE_CAR_BUTTON_TXT);
   TBuf<128> tmp;
   tmp.Format(KStringHeader(), (icons->Count() - 1), text);
   CleanupStack::PopAndDestroy(text);
   // Insert the car text on the position corresponding to EPassengerCar
   // in TTransportationMode enum.
   optionsList->InsertL(TInt(CPositionSelectView::EPassengerCar), tmp);

   /*AknIconUtils::CreateIconL(bitmap, mask,
                             iView->GetMbmName(),
                             EMbmWficonsPedestrian_mode, 
                             EMbmWficonsPedestrian_mode_mask);
   
   // icons will take ownership of bitmap and mask
   icons->AppendL(CGulIcon::NewL(bitmap, mask));

   // Format the string to be inserted into the text array
   text = StringLoader::LoadLC(R_WF_ROUTE_PED_BUTTON_TXT);
   tmp.Format(KStringHeader(), (icons->Count() - 1), text);
   CleanupStack::PopAndDestroy(text);
   // Insert the car text on the position corresponding to EPedestrian
   // in TTransportationMode enum.
   optionsList->InsertL(CPositionSelectView::EPedestrian, tmp);*/
   
   TInt selectedIndex = 0;
   CleanupStack::Pop(optionsList);
   CleanupStack::Pop(icons);

   CAknListQueryDialog* dialog = new (ELeave) CAknListQueryDialog(&selectedIndex);
   dialog->PrepareLC(R_ROUTE_AS_QUERY_DIALOG);
   // dialog will take ownership of optionsList
   dialog->SetItemTextArray(optionsList);
   // dialog will take ownership of icons
   dialog->SetIconArrayL(icons);
   // dialog->RunLD will return the value of the ok button specified in the resource
   // for the dialog if user clicks ok. If user clicks cancel 0 will be returned
   TInt ret = dialog->RunLD();
   if (ret == EAknSoftkeyOk) {
      // Return the selected index, 0 or 1. This will be translated into 
      // TTransportationMode enum values in the view.
      return selectedIndex;
   }
   // Return EInvalidVehicleType, no route will be calculated.
   return CPositionSelectView::EInvalidVehicleType;
}

// Set the name of the origin in the list
void CPositionSelectContainer::SetOrigin( TDesC &aName )
{
   AddItemL( aName, EFalse );
}

// Set the name of the origin in the list
void CPositionSelectContainer::SetDestination( TDesC &aName )
{
   AddItemL( aName, ETrue );
}

TBool
CPositionSelectContainer::IsOriginSelected()
{
   return (iListBox->CurrentItemIndex() == 0);
}

TInt CPositionSelectContainer::GetActiveSelection()
{
   if (iListBox) {
      return iListBox->CurrentItemIndex();
   } else {
      return 0;
   }
}

void
CPositionSelectContainer::SetSelected(TInt index)
{
   if ( iListBox ){
      /* Get number of elements in list. */
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      TInt numElements = itemList->MdcaCount();

      if (index >= numElements) {
         /* Set the last item. */
         iListBox->SetCurrentItemIndexAndDraw(numElements-1);
      } else {
         iListBox->SetCurrentItemIndexAndDraw(index);
      }
   }
}

#include "debuggingdefines.h" // for Assert.

// ----------------------------------------------------------------------------
// void CMyDestContainer::AddItemL()
// Adds list item to the list.
// ----------------------------------------------------------------------------
//
void CPositionSelectContainer::AddItemL( TDesC &aListItem, TBool aIsDestination )
{
   if (aListItem.Locate('\t') == KErrNotFound) {
      Assert(false);
   }
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
      
      TBuf<KBuf256Length> item( _L("") );
      if( !aIsDestination ){ // Add item at the top of the list.
         //TBuf<KBuf32Length> origin;
         //iCoeEnv->ReadResource( origin, R_POSITIONSELECT_ORIGIN_TEXT );
         //item.Num(0);
         //item.Append( KTab );
         //item.Append( origin );
         //item.Append( KTab );
         if( aListItem.Length() < item.MaxLength()-2 )
            item.Copy( aListItem );
         else
            item.Copy( aListItem.Ptr(), item.MaxLength()-2 );
         item.Replace( 0, 1, _L("0") );
         itemArray->Delete(0);
         iListBox->HandleItemRemovalL();
         itemArray->InsertL( 0, item );
      }
      else{
         //TBuf<KBuf32Length> destination;
         //iCoeEnv->ReadResource( destination, R_POSITIONSELECT_DESTINATION_TEXT );
         //item.Num(1);
         //item.Append( KTab );
         //item.Append( destination );
         //item.Append( KTab );
         if( aListItem.Length() < item.MaxLength()-2 ) {
            item.Copy( aListItem );
         } else {
            item.Copy( aListItem.Ptr(), item.MaxLength()-2 );
         }
         item.Replace( 0, 1, _L("1") );
         itemArray->Delete(1);
         iListBox->HandleItemRemovalL();
         itemArray->AppendL( item );
      }
      iListBox->HandleItemAdditionL(); // Updates listbox.
   }
}

// ----------------------------------------------------------------------------
// void CPositionSelectContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CPositionSelectContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                                    TListBoxEvent aEventType )
{
   if (aListBox == iListBox && aEventType == EEventItemDoubleClicked || 
       aListBox == iListBox && aEventType == EEventEnterKeyPressed) {
#ifdef NAV2_CLIENT_SERIES60_V5
      if (AknLayoutUtils::PenEnabled()) {
         ShowStylusPopUpMenuL(iLastPointerPos);
      } else {
         ShowPopupListL();
      }
#else
      ShowPopupListL();
#endif
   }
}

void CPositionSelectContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
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
      
      if (aPointerEvent.iType == TPointerEvent::EButton1Up) {
         iLastPointerPos = aPointerEvent.iPosition;
      } else if (aPointerEvent.iType == TPointerEvent::EButton1Down) {
         iPopUpMenuDisplayed = EFalse;
      }
      if (iListBox && iListBox->Rect().Contains(aPointerEvent.iPosition)) {
         // Only enable long tap if clicking in the listbox
         iLongTapDetector->PointerEventL( aPointerEvent );   
      }
   }
#endif
}

void CPositionSelectContainer::HandleLongTapEventL( const TPoint& aPenEventLocation, 
                                                    const TPoint& aPenEventScreenLocation )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   ShowStylusPopUpMenuL(aPenEventScreenLocation);
#endif
}

#ifdef NAV2_CLIENT_SERIES60_V5
TRect CPositionSelectContainer::PositionButtonsL(CAknButton* aFirstButton, CAknButton* aSecondButton)
{
   // Declare variables needed for calcing the correct size and positions
   // of the buttons. The will be initialized to different values depending
   // on if phone is in landscape mode or in portrait mode.
   TInt heightDivider;
   TInt paddingBetweenButtonsFactor;
   TInt paddingAroundButtonsFactor;
   if (WFLayoutUtils::LandscapeMode()) {
      heightDivider = 7;
      paddingBetweenButtonsFactor = 5;
      paddingAroundButtonsFactor = 2;
   } else {
      heightDivider = 10;
      paddingBetweenButtonsFactor = 5;
      paddingAroundButtonsFactor = 2;      
   }

   // CompleteButtonRect will contain the complete rect for the buttons including
   // the space at the bottom and at the top.
   TRect completeButtonRect;
   // Calc the height of a button
   TInt buttonHeight = Rect().Height() / heightDivider;
   // Calc the padding between the buttons
   TInt paddingBetweenButtons = buttonHeight / paddingBetweenButtonsFactor; 
   // Calc the padding around the buttons, below the button at the bottom and
   // above the button at the at the top of the button rect and at the sides
   // of the buttons
   TInt paddingAroundButtons = paddingBetweenButtons * paddingAroundButtonsFactor;
   // Calc the size of a button.
   TSize buttonSize(Rect().Width() - paddingAroundButtons * 2, buttonHeight);
   // Button rect will contain the rect for each button at the time.
   TRect buttonRect;

   // Fix the first button, set size and position it
   buttonRect = TRect(TPoint(Rect().iTl.iX + paddingAroundButtons, 
                             Rect().Height() - buttonHeight - paddingAroundButtons), buttonSize);
   completeButtonRect = buttonRect;
   aFirstButton->SetRect(buttonRect);

   // Fix the second button
   buttonRect.Move(0, -(buttonHeight + paddingBetweenButtons));
   // Move the y coord of top left corner up, making the rect bigger
   completeButtonRect.iTl.iY -= buttonHeight + paddingBetweenButtons;

   aSecondButton->SetRect(buttonRect);
   // Move the y coord of top left corner up, making the rect bigger
   //completeButtonRect.iTl.iY -= paddingAroundButtons;

   // Dim or enable the buttons as well if the user has set origin and dest.
   if (iView->HasOrigin() && iView->HasDestination()) {
      aFirstButton->SetDimmed(EFalse);
      aSecondButton->SetDimmed(EFalse);
   } else {
      aFirstButton->SetDimmed(ETrue);
      aSecondButton->SetDimmed(ETrue); 
   }

   return completeButtonRect;
}

TRect CPositionSelectContainer::PositionButtonL(class CAknButton* aButton)
{
   // Declare variables needed for calcing the correct size and positions
   // of the button. The will be initialized to different values depending
   // on if phone is in landscape mode or in portrait mode.
   TInt heightDivider;
   TInt paddingBetweenButtonsFactor;
   TInt paddingAroundButtonsFactor;
   if (WFLayoutUtils::LandscapeMode()) {
      heightDivider = 4;
      paddingBetweenButtonsFactor = 5;
      paddingAroundButtonsFactor = 2;
   } else {
      heightDivider = 7;
      paddingBetweenButtonsFactor = 5;
      paddingAroundButtonsFactor = 2;      
   }

   // CompleteButtonRect will contain the complete rect for the button including
   // the space at the bottom and at the top.
   TRect completeButtonRect;
   // Calc the height of a button
   TInt buttonHeight = Rect().Height() / heightDivider;
   // Calc the padding between the buttons
   TInt paddingBetweenButtons = buttonHeight / paddingBetweenButtonsFactor; 
   // Calc the padding around the button.
   TInt paddingAroundButton = paddingBetweenButtons * paddingAroundButtonsFactor;
   // Calc the size of the button.
   TSize buttonSize(Rect().Width() - paddingAroundButton, buttonHeight);
   // Button rect will contain the rect for each button at the time.
   TRect buttonRect;

   // Fix the button, set size and position it
   buttonRect = TRect(TPoint(Rect().iTl.iX + (paddingAroundButton/2), 
                             Rect().Height() - buttonHeight - paddingAroundButton), buttonSize);
   completeButtonRect = buttonRect;
   aButton->SetRect(buttonRect);

   if (iView->HasOrigin() && iView->HasDestination()) {
      aButton->SetDimmed(EFalse);
   } else {
      aButton->SetDimmed(ETrue);
   }

   return completeButtonRect;
}
#endif

// ---------------------------------------------------------
// CPositionSelectContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CPositionSelectContainer::SizeChanged()
{
   if(iBgContext) {
      iBgContext->SetRect(Rect());
      if(&Window()) {
         iBgContext->SetParentPos(PositionRelativeToScreen());
      }
   }
#ifdef NAV2_CLIENT_SERIES60_V5
   if (AknLayoutUtils::PenEnabled()) {
      TRect buttonRect;
      if (!iCarButton /*&& !iPedButton*/) {
         buttonRect = CreateAndPositionButtonsL();
      } else {
         //buttonRect = PositionButtonsL(iPedButton, iCarButton);
        buttonRect = PositionButtonL(iCarButton);
         if (iCarButton && !iCarButton->IsVisible()) {
            iCarButton->MakeVisible(ETrue);
         }
         /*
         if (iPedButton && !iPedButton->IsVisible()) {
            iPedButton->MakeVisible(ETrue);
         }
         */
         /*
         if (iTitleLabel && !iTitleLabel->IsVisible()) {
            iTitleLabel->MakeVisible(ETrue);
         }
         */
      }

      /*TInt fontHeight = iTitleLabel->Font()->HeightInPixels();
      //TInt pixelGap = label->PixelGapBetweenLines();
      TInt descent = iTitleLabel->Font()->DescentInPixels();
      // The height of one label
      TInt labelHeight = fontHeight + descent;
      // Calculate the rect of the label it should be above the buttons and below
      // the listbox.
      TRect labelRect(TPoint(Rect().iTl.iX, buttonRect.iTl.iY - (labelHeight + descent)),
                      TSize(Rect().Width(), labelHeight));
      // Set the rect of the label
      iTitleLabel->SetRect(labelRect);*/

      // Disable scrollbars per default.
      iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                          CEikScrollBarFrame::EOff);

      // Calculate the rect of the listbox, top left should be at the top of the
      // main pane and the rect should end just above the title label.
      //TRect listBoxRect(Rect().iTl, TPoint(labelRect.iBr.iX, labelRect.iTl.iY));
      TRect listBoxRect(Rect().iTl, TPoint(buttonRect.iBr.iX, buttonRect.iTl.iY));
      iListBox->SetRect(listBoxRect);

      TInt listBoxHeight = iListBox->ItemHeight() * iListBox->Model()->NumberOfItems();
      if (listBoxHeight > listBoxRect.Height()) {
         // Show scroll bar only if the height of the listbox is larger then
         // the rect.
         iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
                                                             CEikScrollBarFrame::EOn);
         iListBox->UpdateScrollBarsL();
      }
   } else {
      // No touch screen, buttons and title label should not be visible.
      if (iCarButton) {
         iCarButton->MakeVisible(EFalse);
      }
      /*
      if (iPedButton) {
         iPedButton->MakeVisible(EFalse);
      }
      */
      /*
      if (iTitleLabel) {
         iTitleLabel->MakeVisible(EFalse);
      }
      */
      iListBox->SetRect(Rect());
   }
#else
   if ( iListBox ){
      iListBox->SetRect( Rect() ); // Sets rectangle of lstbox.
   }
#endif 
}

// ---------------------------------------------------------
// CPositionSelectContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CPositionSelectContainer::HandleControlEventL( CCoeControl* aControl,
                                                    TCoeEvent aEventType )
{
#ifdef NAV2_CLIENT_SERIES60_V5
   switch (aEventType) {
   case EEventStateChanged:
      if (iCarButton == aControl) {
         // Handle route as car
         iView->SendVehicleType(isab::NavServerComEnums::passengerCar);
      
      } /*else if (iPedButton == aControl) {
         // Handle route as pedestrian
         iView->SendVehicleType(isab::NavServerComEnums::pedestrian);
         }*/
      break;
   case CAknButton::ELongPressEvent:
   case CAknButton::ELongPressEndedEvent:
      break;
   default:
      break;
   }
#endif
}

// ----------------------------------------------------------------------------
// TKeyResponse CMyFavoritesContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CPositionSelectContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                  TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iCode ) // The code of key event is...
   {
   // Switches tab.
   case EKeyDevice3:
//       ShowPopupListL();
      /*if( iView->AreSet() ){
         iView->HandleCommandL( EWayFinderCmdPositionSelectNavigate );
         return EKeyWasConsumed;
      }*/
      break;
   case EKeyLeftArrow: // Left key.
   case EKeyRightArrow: // Right Key.
      return EKeyWasConsumed; //EKeyWasNotConsumed;
   }
   if ( iListBox ){
      return iListBox->OfferKeyEventL( aKeyEvent, aType );
   }
   else{
      return EKeyWasNotConsumed;
   }
}

#ifdef NAV2_CLIENT_SERIES60_V3
void CPositionSelectContainer::AddToIconList(CArrayPtr<CGulIcon>* aIcons, 
                                             const TDesC& aFilename, 
                                             TInt aIconId, 
                                             TInt aIconMaskId)
{
   CFbsBitmap* bitmap;
   CFbsBitmap* mask;
   AknIconUtils::CreateIconL(bitmap, 
                             mask, 
                             aFilename, 
                             aIconId, 
                             aIconMaskId);
   
   // Append the icon to icon array
   aIcons->AppendL(CGulIcon::NewL(bitmap, mask));
   
}
#endif

TTypeUid::Ptr CPositionSelectContainer::MopSupplyObject(TTypeUid aId)
{
   if (iBgContext) {
      return MAknsControlContext::SupplyMopObject(aId, iBgContext);
   }
   return CCoeControl::MopSupplyObject(aId);
}

void CPositionSelectContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   gc.DrawRect(iListBox->Rect());
   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);
}

void CPositionSelectContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
   }   
}

// End of File  
