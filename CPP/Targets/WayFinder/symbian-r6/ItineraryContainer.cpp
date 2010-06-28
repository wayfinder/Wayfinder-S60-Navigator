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
#include <barsread.h>  // for resource reader
#include <aknlists.h>  // for avrell style listbox
#include <akniconarray.h> // for icon array
#include <gulicon.h>

#include "RsgInclude.h"
#include "wficons.mbg"

#include "GuiDataStore.h"
#include "ItineraryView.h"
#include "ItineraryContainer.h"
#include "wayfinder.hrh"

#include "WFTextUtil.h"
#include "WFLayoutUtils.h"

#include "DistancePrintingPolicy.h"

// Definitions
#define LISTBOX_POS     TPoint(10, 15)


// ---------------------------------------------------------
// CItineraryContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CItineraryContainer::ConstructL( const TRect& aRect, 
                                      CItineraryView* aView )
{
   CreateWindowL();

   iView = aView;
   iPrevious = 1;

   GuiDataStore* gds = iView->GetGuiDataStore();
   HBufC* wfmbmname = gds->iWayfinderMBMFilename;

   TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC( reader, R_WAYFINDER_ITINERARY_LIST );
   iListBox = new( ELeave ) CAknDoubleLargeStyleListBox();
   LOGNEW(iListBox, CAknDoubleLargeStyleListBox);
   iListBox->SetContainerWindowL(*this);
   iListBox->ConstructFromResourceL(reader);
   iListBox->SetListBoxObserver( this );

   iListBox->CreateScrollBarFrameL(ETrue);
   iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff,
         CEikScrollBarFrame::EAuto);
   iListBox->UpdateScrollBarsL();

   iListBox->ItemDrawer()->FormattedCellData()->SetMarqueeParams(5, 3, 1000000, 200000);
   iListBox->ItemDrawer()->FormattedCellData()->EnableMarqueeL(ETrue);

   CleanupStack::PopAndDestroy();  // Resource reader

   // Creates gul icon.
   CArrayPtr<CGulIcon>* icons = new( ELeave ) CAknIconArray( 2 );
   LOGNEW(icons, CAknIconArray);

   CleanupStack::PushL( icons );
#ifdef NAV2_CLIENT_SERIES60_V3
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_turn_blank,
                 EMbmWficonsSm_turn_blank_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_ferry,
                 EMbmWficonsSm_ferry_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_reached_destination,
                 EMbmWficonsSm_reached_destination_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_keep_left,
                 EMbmWficonsSm_keep_left_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_keep_right,
                 EMbmWficonsSm_keep_right_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_turn_left,
                 EMbmWficonsSm_turn_left_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_multiway_roundabout_right,
                 EMbmWficonsSm_multiway_roundabout_right_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_multiway_roundabout_left,
                 EMbmWficonsSm_multiway_roundabout_left_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_parking_area,
                 EMbmWficonsSm_parking_area_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_turn_right,
                 EMbmWficonsSm_turn_right_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_start,
                 EMbmWficonsSm_start_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_drive_straight_ahead1,
                 EMbmWficonsSm_drive_straight_ahead1_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_uturn_right,
                 EMbmWficonsSm_uturn_right_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSm_uturn_left,
                 EMbmWficonsSm_uturn_left_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsDetour_square,
                 EMbmWficonsDetour_square_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsSpeedcamera_square,
                 EMbmWficonsSpeedcamera_square_mask);
   AddToIconList(icons, *wfmbmname, 
                 EMbmWficonsTickmarker,
                 EMbmWficonsTickmarker_mask);

#else
   // Appends graphic data.
   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_blank,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_ferry,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_flag,
                                           EMbmWficonsMedium_flag_m ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_keep_left,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_keep_right,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_left_arrow,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_multiway_rdbt,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_multiway_rdbt_left,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_park_car,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_right_arrow,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_start,
                                           EMbmWficonsMedium_start_m ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_straight_arrow,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_u_turn,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_u_turn_left,
                                           EMbmWficonsMedium_mask ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_detour,
                                           EMbmWficonsMedium_speedcam_detour_m ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsMedium_speedcam,
                                           EMbmWficonsMedium_speedcam_detour_m ) );

   icons->AppendL( iEikonEnv->CreateIconL( *wfmbmname,
                                           EMbmWficonsTickmark,
                                           EMbmWficonsTickmark_m ) );
#endif
   // Sets graphics as ListBox icon.
   iListBox->ItemDrawer()->ColumnData()->SetIconArray( icons );

   CleanupStack::Pop(); //icons
   
   iListBox->SetExtent( LISTBOX_POS, iListBox->MinimumSize() );

   SetRect(aRect);
   ActivateL();
}

// Destructor
CItineraryContainer::~CItineraryContainer()
{
   LOGDEL(iListBox);
   delete iListBox;
   iListBox = NULL;
   iView = NULL;
}

void CItineraryContainer::SetListArray( TBool aHasRoute )
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
      itemArray->Reset();
      iListBox->HandleItemRemovalL();
      HBufC* item;
      if( aHasRoute ) {
         item = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_ITINERARY_CREATE_ROUTE );
      } else {
         item = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_ITINERARY_NOROUTE );
      }
      itemArray->AppendL( *item );
      CleanupStack::PopAndDestroy(item);
      iListBox->HandleItemAdditionL();
   }
}

// ----------------------------------------------------------------------------
// void CItineraryContainer::AddItemL()
// Adds list item to the list.
// ----------------------------------------------------------------------------
//
void CItineraryContainer::AddItemL( TPictures aTurn, TBool aRightTraffic,
                                    TInt aDistance, TInt aAcumulatedDistance,
                                    TInt aExit, TDesC &aText)
{
   TInt iconPos = 0; //Blank
   TBool multiwayRdbt = EFalse;
   switch( aTurn )
   {
   case EFerry:
      iconPos = 1; // Ferry
      break;
   case EFinishArrow:
   case EFinishFlag:
      iconPos = 2; // Flag
      break;
   case EEnterHighWay:
   case EEnterMainRoad:
      if (aRightTraffic) {
         iconPos = 3; // KeepLeft
      }
      else{
         iconPos = 4; //KeepRight
      }
      break;
   case EExitHighWay:
   case EExitMainRoad:
      if (aRightTraffic) {
         iconPos = 4; //KeepRight
      }
      else{
         iconPos = 3; // KeepLeft
      }
      break;
   case EExitHighWayLeft:
   case EExitMainRoadLeft:
   case EKeepLeft:
      iconPos = 3; // KeepLeft
      break;
   case EExitHighWayRight:
   case EExitMainRoadRight:
   case EKeepRight:
      iconPos = 4; //KeepRight
      break;
   case E3WayLeft:
   case E3WayTeeLeft:
   case E4WayLeft:
   case ELeftArrow:
   case ERdbLeft:
      iconPos = 5; // LeftArrow
      break;
   case EMultiWayRdb:
      if (aRightTraffic) {
         iconPos = 6; // MultiWayRdb
      } 
      else {
         iconPos = 7; // MultiWayRdbLeft
      }
      multiwayRdbt = ETrue;
      break;
   case EPark:
      iconPos = 8; // Park
      break;
   case E3WayRight:
   case E3WayTeeRight:
   case E4WayRight:
   case ERdbRight:
   case ERightArrow:
      iconPos = 9; // RightArrow
      break;
   case EStart:
      iconPos = 10; // Start
      break;
   case E4WayStraight:
   case EHighWayStraight:
   case EStraight:
   case EStraightArrow:
   case ERdbStraight:
      iconPos = 11; // StraightArrow
      break;
   case ERdbUTurn:
   case EUTurn:
      if (aRightTraffic) {
         iconPos = 12; // UTurn
      } else {
         iconPos = 13; // UTurnLeft
      }
      break;
   case EDetour:
      iconPos = 14; // Detour
      break;
   case ESpeedCam:
      iconPos = 15; // Speed camera
      break;
   case EOffTrack:
   case EWrongDirection:
   case ENoPicture:
      break;
   }
   TBuf<KBuf32Length> distance;

   if( aTurn == EStart ){
      iCoeEnv->ReadResource( distance, R_WAYFINDER_ITINERARY_START );
   } else if (aTurn == EDetour) {
      if (aRightTraffic) {
         /* Used as Begin. */
         iCoeEnv->ReadResource( distance, R_WAYFINDER_ITINERARY_DETOUR_START);
      } else {
         /* Used as End. */
         iCoeEnv->ReadResource( distance, R_WAYFINDER_ITINERARY_DETOUR_END);
      }
   } else if (aTurn == ESpeedCam) {
      if (aRightTraffic) {
         /* Used as Begin. */
         iCoeEnv->ReadResource( distance, R_WAYFINDER_ITINERARY_SPEEDCAM_START);
      } else {
         /* Used as End. */
         iCoeEnv->ReadResource( distance, R_WAYFINDER_ITINERARY_SPEEDCAM_END);
      }
   } else {

      GetDistance( aDistance, distance );

      if( multiwayRdbt ){
         distance.Append( _L(" ") );
         distance.AppendNum( aExit );
         TBuf<KBuf32Length> exit;
         iCoeEnv->ReadResource( exit, R_WAYFINDER_ITINERARY_EXIT );
         distance.Append( exit );
      }

      TBuf<KBuf32Length> acumulated;

      isab::DistancePrintingPolicy::DistanceMode mode =
         isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

      char* tmp2 = isab::DistancePrintingPolicy::convertDistance(
            aAcumulatedDistance, mode, isab::DistancePrintingPolicy::Round);
      if (tmp2) {
         WFTextUtil::char2TDes(acumulated, tmp2);
         delete[] tmp2;
      } else {
         acumulated.Copy(_L(" "));
      }
      distance.Append( _L("("));
      distance.Append( acumulated );
      distance.Append( _L(")"));
   }

   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
      
      TBuf<KBuf256Length> item( _L("") );
      item.Num( iconPos );
      item.Append( KTab );
      item.Append( distance );
      item.Append( KTab );
      if( aText.Length() < item.MaxLength()-item.Length() )
         item.Append( aText );
      else
         item.Append( aText.Ptr(), item.MaxLength()-item.Length() );
      itemArray->AppendL( item );
      iListBox->HandleItemAdditionL(); // Updates listbox.
   }
}

// ----------------------------------------------------------------------------
// void CItineraryContainer::RemoveAllItemsL()
// Remove all items.
// ----------------------------------------------------------------------------
//
void CItineraryContainer::RemoveAllItemsL()
{
   if ( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray *itemArray = STATIC_CAST( CDesCArray*, itemList );

      itemArray->Reset();

      iListBox->HandleItemRemovalL(); // Updates listbox.
      // Actually, the HandleItemRemovalL does not provoke a redraw...
      iListBox->Reset();
   }
}

void CItineraryContainer::SetSelection( TInt aCurrent, TInt aPrevious )
{
   if( iListBox ){
      MDesCArray* itemList = iListBox->Model()->ItemTextArray();
      CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
      aCurrent++;
      aPrevious++;
      if( aCurrent > 0 && aCurrent <= itemList->MdcaCount() ){
         TInt currItem = itemList->MdcaCount() - aCurrent;
         TInt prevItem = itemList->MdcaCount() - iPrevious;
         
         if( iView->IsGpsAllowed()){
            TBuf<KBuf256Length> item( _L("") );
            item.Copy( itemArray->MdcaPoint( prevItem ) );
            TInt idx = item.Find( _L("\t16") );
            if( idx != KErrNotFound ){
               item.SetLength( idx );
               item.PtrZ();
               itemArray->Delete( prevItem );
               itemArray->InsertL( prevItem, item );
            }
            _LIT(KSelectedItemEndText, "\t16");
            if(itemArray->MdcaPoint(currItem).Length() <= item.MaxLength() - KSelectedItemEndText().Length())
               item.Copy( itemArray->MdcaPoint( currItem ) );
            else
               item.Copy( itemArray->MdcaPoint( currItem ).Ptr(), item.MaxLength() - KSelectedItemEndText().Length());
            itemArray->Delete( currItem );
            item.Append( KSelectedItemEndText );
            itemArray->InsertL( currItem, item );
            iListBox->HandleItemAdditionL();
            iPrevious = aCurrent;
         }
         if( currItem != 0 ) {
            iListBox->ScrollToMakeItemVisible( currItem-1 );
         }
         if( currItem != itemList->MdcaCount()-1 ) {
            iListBox->ScrollToMakeItemVisible( currItem+1 );
         }
         iListBox->SetCurrentItemIndexAndDraw( currItem );
      } else {
         /* Remove old. */
         TInt prevItem = itemList->MdcaCount()-iPrevious;
         TBuf<KBuf256Length> item( _L("") );

         item.Copy( itemArray->MdcaPoint( prevItem ) );
         TInt idx = item.Find( _L("\t16") );
         if( idx != KErrNotFound ){
            item.SetLength( idx );
            item.PtrZ();
            itemArray->Delete( prevItem );
            itemArray->InsertL( prevItem, item );
         }
         iPrevious = 1;
      }
   }
}

TInt CItineraryContainer::GetCurrentTurn()
{
   // Inverts the current selected item from the listbox, since the 
   // turn list has start = 0 which is the last entry in the listbox 
   // and the listbox's first entry is the last entry in the turn list.
   if (iListBox) {
      return iListBox->Model()->NumberOfItems() - 1 - iListBox->CurrentItemIndex();
   }
   return 0;
}

void CItineraryContainer::GetDistance( TUint aDistance, TDes &aText )
{
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());

   char* tmp2 = isab::DistancePrintingPolicy::convertDistance(aDistance,
         mode, isab::DistancePrintingPolicy::Round);
   if (tmp2) {
      WFTextUtil::char2TDes(aText, tmp2);
   } else {
      aText.Copy(_L(" "));
   }
}

// ----------------------------------------------------------------------------
// void CItineraryContainer::HandleListBoxEventL( CEikListBox*,
//  TListBoxEvent )
// Handles listbox event.
// ----------------------------------------------------------------------------
void CItineraryContainer::HandleListBoxEventL( CEikListBox* aListBox,
                                               TListBoxEvent aEventType )
{
   if( aListBox == iListBox && aEventType == EEventEnterKeyPressed ){
   } else if ( aListBox == iListBox && aEventType == EEventItemDoubleClicked) {
      //CEikonEnv::Static()->InfoMsg(_L("Double click"));
   } /*else if (aListBox == iListBox && aEventType == EEventItemClicked) {
      iView->SetCurrentTurn(iListBox->CurrentItemIndex());
   }*/
}

// ---------------------------------------------------------
// CItineraryContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CItineraryContainer::SizeChanged()
{
   if ( iListBox ){
      iListBox->SetRect( Rect() ); // Sets rectangle of lstbox.
   }
}

// ---------------------------------------------------------
// CItineraryContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CItineraryContainer::CountComponentControls() const
{
   return 1; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CItineraryContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CItineraryContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case 0:
      return iListBox;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CItineraryContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CItineraryContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
}

// ---------------------------------------------------------
// CItineraryContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CItineraryContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                               TCoeEvent /*aEventType*/ )
{
   // TODO: Add your control event handler code here
}

// ----------------------------------------------------------------------------
// TKeyResponse CMyFavoritesContainer::OfferKeyEventL( const TKeyEvent&,
//  TEventCode )
// Handles the key events.
// ----------------------------------------------------------------------------
//
TKeyResponse CItineraryContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                  TEventCode aType )
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }

   switch ( aKeyEvent.iScanCode ) 
   {
   // Switches tab.
   case EStdKeyLeftArrow: // Left key.
   case EStdKeyRightArrow: // Right Key.
      return EKeyWasNotConsumed;
      break;
   case EStdKeyUpArrow:
      if( iListBox->CurrentItemIndex() != 0 )
         iView->UpdateCurrentTurn( ETrue );
      if( iView->IsGpsAllowed())
         return iListBox->OfferKeyEventL( aKeyEvent, aType );
      return EKeyWasConsumed;
      break;
   case EStdKeyDownArrow:
      if( iListBox->CurrentItemIndex() != iListBox->Model()->NumberOfItems()-1 )
         iView->UpdateCurrentTurn( EFalse );
      if( iView->IsGpsAllowed())
         return iListBox->OfferKeyEventL( aKeyEvent, aType );
      return EKeyWasConsumed;
      break;
      // We dont want this functionality now.
//    case EStdKeyDevice3:
//       iView->HandleCommandL( EWayFinderCmdItineraryReroute );
//       return EKeyWasConsumed;
//       break;
      // No hash for you, for now anyway
//    case EStdKeyHash:
//       iView->HandleCommandL( EWayFinderCmdMap ) ;
//       return EKeyWasConsumed;
//       break;
   }
   if ( iListBox ){
      return iListBox->OfferKeyEventL( aKeyEvent, aType );
   } else{
      return EKeyWasNotConsumed;
   }
}

#ifdef NAV2_CLIENT_SERIES60_V3
void CItineraryContainer::AddToIconList(CArrayPtr<CGulIcon>* aIcons, 
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
   
   // Determinate icon size
//    TSize iconSize;
//    iconSize.iWidth = iSizeOfCell.iWidth / 2;
//    iconSize.iHeight = iSizeOfCell.iHeight / 2;
   
   AknIconUtils::SetSize(bitmap, TSize(30,30));
   //Initializes the icon to the given size.
   //Note that this call sets the sizes of both bitmap and mask 
//    AknIconUtils::SetSize( iBitmap, iconSize );
   
   // Append the icon to icon array
   aIcons->AppendL(CGulIcon::NewL(bitmap, mask));
   
}

#endif


void CItineraryContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());
      iListBox->SetRect(Rect());
   }
}


// End of File  
