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
#include <eiklabel.h>	// for labels
#include <eikedwin.h>	// for edwin text boxes
#include <eikmfne.h>
#include <barsread.h>	// for resource reader
#include <gulcolor.h>	// for color
#include <aknutils.h>	// for fonts
#include <txtfmlyr.h>
#include <aknlists.h>
#include <math.h>			//floor
#include "FormatDirection.h"


#if defined NAV2_CLIENT_SERIES60_V2
#include <aknsdrawutils.h>
#endif

#include "RsgInclude.h"
#include "wficons.mbg"
#include "InfoContainer.h"
#include "InfoView.h"
#include "WayFinderConstants.h"
#include "InfoCompass.h"
#include "memlog.h"
#include "WFTextUtil.h"
#include "PositionDisplay.h"
#include "RectTools.h"

#include "DistancePrintingPolicy.h"
#include "WFLayoutUtils.h"



#define MARGIN_Y           4


#define SPEED_LABEL_POS       TPoint(  8, 60)
#define SPEED_EDWIN_POS       TPoint(  8, 78)

#define ALTITUDE_LABEL_POS    TPoint(  8, 102)
#define ALTITUDE_EDWIN_POS    TPoint(  8, 120)

#define COMPASS_CTRL_POS      TPoint( 96, 84)
#define COMPASS_SIZE          TSize( 75, 75 ) 

#define ANGLE_LABEL_CENTER    TPoint( 100, 136 )

#if defined NAV2_CLIENT_SERIES60_V3
#define COMPASS_IMAGE         EMbmWficonsCompass
#define COMPASS_IMAGE_M       EMbmWficonsCompass_mask
#else
#define COMPASS_IMAGE         EMbmWficonsCompass75
#define COMPASS_IMAGE_M       EMbmWficonsCompass75_mask
#endif

#define LISTBOX_POS     TPoint(10, 15)

using namespace isab;

_LIT( K1Number, "  %i" );
_LIT( K2Numbers, " %i" );
_LIT( K3Numbers, "%i" );

// Enumerations
enum TControls
{
	EBlindModeList,
	ENumberControls
};

// ================= MEMBER FUNCTIONS =======================
HBufC*
CInfoContainer::GetSpeedLabel() const
{
   switch (iView->GetDistanceMode()) {
      case isab::DistancePrintingPolicy::ModeImperialYards:
      case isab::DistancePrintingPolicy::ModeImperialFeet:
         return iCoeEnv->AllocReadResourceL( R_INFO_MPH_LABEL );
         break;
      case isab::DistancePrintingPolicy::ModeInvalid:
      case isab::DistancePrintingPolicy::ModeMetric:
      default:
         return iCoeEnv->AllocReadResourceL( R_INFO_KMH_LABEL );
         break;
   }
}

class CInfoContainer * CInfoContainer::NewL(class CInfoView* aParent,
                                            const TRect& aRect,
                                            const TDesC& aMbmFile,
                                            isab::Log* aLog) 
{
   class CInfoContainer* self = new (ELeave) CInfoContainer(aLog);
   CleanupStack::PushL(self);
   self->SetMopParent(aParent);
   self->ConstructL(aParent, aRect, aMbmFile);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CInfoContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CInfoContainer::ConstructL(CInfoView* aView,
                                const TRect& aRect,
                                const TDesC& aMbmFile) 
{
   iView = aView;
   CreateWindowL();

   
   TResourceReader reader;
   iCoeEnv->CreateResourceReaderLC(reader, R_WAYFINDER_BLIND_MODE_LIST_2);
   iListBox = new (ELeave) CAknDoubleStyleListBox();
   LOGNEW(iListBox, CAknDoubleStyleListBox);
   iListBox->SetContainerWindowL(* this);
   iListBox->ConstructFromResourceL(reader);
   CleanupStack::PopAndDestroy();
	
   iListBox->SetExtent(LISTBOX_POS, iListBox->MinimumSize());
   
   SetRect(aRect);
   SetListArray();
   ActivateL();
}

// Destructor
CInfoContainer::~CInfoContainer() 
{
	delete iListBox;
}

void CInfoContainer::SetListArray() 
{
   if(iListBox) {
      CTextListBoxModel * model = iListBox->Model();
      CDesCArray * itemArray = STATIC_CAST(CDesCArray *, model->ItemTextArray());
      itemArray->Reset();

      iListBox->HandleItemRemovalL();

      HBufC* text;
      HBufC* temp;

      HBufC* loading = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_LOADING);
      _LIT(KTabulator, "\t");

      // Current latitude (index 0)
      text = HBufC::NewLC(512);
      TPtr connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_WF_INFO_LATITUDE);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);

      // Current longitude (index 1)
      text = HBufC::NewLC(512);
      connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_WF_INFO_LONGITUDE);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);
      
      // Altitude (index 2)
      text = HBufC::NewLC(512);
      connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_WF_GPS_ALTITUDE);
      connector.Append(* temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);

      // Heading (index 3)
      text = HBufC::NewLC(512);
      connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_HEADING);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);

      // Speed (index 4)
      text = HBufC::NewLC(512);
      connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_INFO_SPEED_LABEL);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(*text);
      CleanupStack::PopAndDestroy(text);

      // GPS state (index 5)
      // TODO: implement
      text = HBufC::NewLC(512);
      connector = text->Des();
      connector.Append(KTabulator);
      temp = iCoeEnv->AllocReadResourceLC(R_GPS_STATE_MSG);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
//       connector.Append(_L("\tloading..."));
      connector.Append(KTabulator);
      connector.Append(*loading);
      itemArray->AppendL(* text);
      CleanupStack::PopAndDestroy(text);

      CleanupStack::PopAndDestroy(loading);

      iListBox->HandleItemAdditionL();
   }
}

static const double KPI = 3.14159265358979323846264338327950288419716939937510;

/**
 * Helper function that converts a coordinate value from the gps into part 
 * needed to display the value.
 *
 * @param aCoord the value from the gps.
 * @param aSign reference to the sign (plus or minus)
 * @param reference to the degrees
 * @param reference to the minutes
 * @param reference to the seconds
 * @param reference to the then still left parts
 */
void SetCoordinate (TInt32 aCoord,
                    TInt &aSign,
                    TInt &aDeg,
                    TInt &aMin,
                    TInt &aSec,
                    TInt &aParts) 
{
   TReal coord = (aCoord / KPI) * 180;
   coord /= 100000000;
	
   aSign = coord < 0 ? -1 : 1;
   coord *= aSign;
	
   aDeg = TUint(floor(coord));
   coord = (coord-aDeg)*60;
   aMin = TUint(floor(coord));
	
   coord = (coord-aMin)*60;
   aSec = TUint(floor(coord));
   coord = (coord-aSec)*100;
   aParts = TUint(floor(coord+0.5));
   if( aParts >= 99 ){
      aParts = 0;
   }
}

/**
 * Note: The below string cannot be set as a _LIT,
 *       since the macro does not do conversion of
 *       UTF-8, and the compiler does not accept the
 *       iso-8859-1 degree sign.
 */
#define TOTAL_STRING "%+ 3u°%+02u'%+02u.%+02u\""
#define LONGITUDE_TYPE 0
#define LATITUDE_TYPE 1

/**
 * Helper function to convert the given integer into a human readable format.
 *
 * @param aVal coordinate value that come from the gps.
 * @param aType type that sets it to longitude or latitude.
 */
void Format(TInt aVal, 
            TBuf<2> plus, 
            TBuf<2> minus, 
            TPtr connector) 
{
   TInt sign;
   TInt deg;
   TInt min;
   TInt sec;
   TInt parts;
   SetCoordinate(aVal, sign, deg, min, sec, parts);
	
   HBufC * format = WFTextUtil::AllocLC(TOTAL_STRING);

   connector.Format(* format, deg, min, sec, parts);
   connector.Insert(0, sign == 1 ? plus : minus);
   CleanupStack::PopAndDestroy(format);
}

void CInfoContainer::SetLatOrLon(TInt32 aLatOrLon, TInt32 index)
{
   _LIT(Tabulator, "\t");

   HBufC * text;
   HBufC * temp;
   
   text = HBufC::NewLC(512);
   TPtr connector = text->Des();
   connector.Append(Tabulator);
   
   if(index == 1) // latitude
      temp = iCoeEnv->AllocReadResourceLC(R_WF_INFO_LATITUDE);
   else // longitude
      temp = iCoeEnv->AllocReadResourceLC(R_WF_INFO_LONGITUDE);
   
   connector.Append(* temp);   
   CleanupStack::PopAndDestroy(temp);

   connector.Append(Tabulator);
   temp = HBufC::NewLC(512);
   TPtr format = temp->Des();
   TBuf<2> plus;
   TBuf<2> minus;

   if(index == 1) {
      iCoeEnv->ReadResource(plus, R_INFO_LAT_PLUS_SIGN);
      iCoeEnv->ReadResource(minus, R_INFO_LAT_MINUS_SIGN);
   } else if(index == 2) {
      iCoeEnv->ReadResource(plus, R_INFO_LON_PLUS_SIGN);
      iCoeEnv->ReadResource(minus, R_INFO_LON_MINUS_SIGN);
   }

   Format(aLatOrLon, plus, minus, format);
   connector.Append(* temp);
   CleanupStack::PopAndDestroy(temp);

   ReplaceListEntryL(index, * text);
   CleanupStack::PopAndDestroy(text);	
}

void CInfoContainer::SetPosL(TInt32 aLatitude, TInt32 aLongitude) 
{
   if(iListBox) {
      SetLatOrLon(aLatitude, 1);
      SetLatOrLon(aLongitude, 2);
   }
}

void CInfoContainer::SetPadding(HBufC* text, TInt num) 
{
   if( num < 10 ) {
      text->Des().Copy(_L("  "));
   } else if( num < 100 ) {
      text->Des().Copy(_L(" "));
   } else {
      text->Des().Copy(_L(""));
   }
}

void CInfoContainer::SetAltitude( TInt aAltitude ) 
{
   if(iListBox) {
      HBufC * text;
      HBufC * temp;

      _LIT(Tabulator, "\t");

      isab::DistancePrintingPolicy::DistanceMode mode =
         isab::DistancePrintingPolicy::DistanceMode(iView->GetDistanceMode());
		
      text = HBufC::NewLC(512);
      TPtr connector = text->Des();
      connector.Append(Tabulator);

      temp = iCoeEnv->AllocReadResourceLC(R_WF_GPS_ALTITUDE);
      connector.Append(* temp);
      CleanupStack::PopAndDestroy(temp);

      connector.Append(Tabulator);

      aAltitude /= 10;
      char* tmp2 = isab::
         DistancePrintingPolicy::convertAltitude(aAltitude, 
                                                 mode, 
                                                 isab::
                                                 DistancePrintingPolicy::Exact);
      if (tmp2) {
         temp = WFTextUtil::AllocLC(tmp2);
         delete[] tmp2;
      } else {
         temp = HBufC::NewLC(16);
         temp->Des().Copy(_L(" "));
      }
      connector.Append(* temp);
      CleanupStack::PopAndDestroy(temp);

      ReplaceListEntryL(3, * text);
      CleanupStack::PopAndDestroy(text);
   }
}

void CInfoContainer::SetHeading(TInt aHeading) 
{
   if(iListBox) {
      HBufC * text = HBufC::NewLC(512);
      TPtr connector = text->Des();
      connector.Append(_L("\t"));
					    
      HBufC* temp = iCoeEnv->AllocReadResourceLC(R_HEADING);
      connector.Append(*temp);
      CleanupStack::PopAndDestroy(temp);
		
      connector.Append(_L("\t"));
      
      if ( iView->IsHeadingUsable() ) {
      
         MC2Direction direction(aHeading);
         
         HBufC* degrees =
            FormatDirection::ParseLC(direction.CompassPoint());
         
         connector.Append(*degrees);
         CleanupStack::PopAndDestroy(degrees);

         // Also add actual degrees.
         char* tmp = new char[10];
         sprintf( tmp, ", %d", direction.GetAngle() );
         HBufC* angle = WFTextUtil::AllocLC( tmp );

         connector.Append( *angle );

         HBufC* degreeSign = CEikonEnv::Static()->AllocReadResourceLC(R_WF_DEGREE_SIGN);
         connector.Append( *degreeSign );
         
         delete[] tmp;
         CleanupStack::PopAndDestroy( degreeSign );
         CleanupStack::PopAndDestroy( angle );

      } else {
         // Unrealiable heading.
         HBufC* unreliableHeading =
            CEikonEnv::Static()->AllocReadResourceLC(R_WF_UNRELIABLE_HEADING);
         connector.Append( *unreliableHeading );
         CleanupStack::PopAndDestroy( unreliableHeading );
      }

      ReplaceListEntryL(4, *text);
      CleanupStack::PopAndDestroy(text);
   }
}

void CInfoContainer::SetSpeed(TInt aSpeed) 
{
   if(iListBox) {
      HBufC * text = HBufC::NewLC(512);
      TPtr connector = text->Des();
      connector.Append(_L("\t"));
		
      HBufC * temp = iCoeEnv->AllocReadResourceLC(R_INFO_SPEED_LABEL);
      connector.Append(* temp);
      CleanupStack::PopAndDestroy(temp);
		
      connector.Append(_L("\t"));
      if(aSpeed > -1) {
         connector.AppendNum(aSpeed);
         switch (iView->GetDistanceMode()) {
         case DistancePrintingPolicy::ModeImperialYards:
         case DistancePrintingPolicy::ModeImperialFeet:
            temp = iCoeEnv->AllocReadResourceLC(R_INFO_MPH_LABEL);
            break;
         case DistancePrintingPolicy::ModeInvalid:
         case DistancePrintingPolicy::ModeMetric:
         default:
            temp = iCoeEnv->AllocReadResourceLC(R_INFO_KMH_LABEL);
            break;
         }
         connector.Append(* temp);
         CleanupStack::PopAndDestroy(temp);
      } else {
         connector.Append(_L("-"));
      }
	
      ReplaceListEntryL(5, * text);
      CleanupStack::PopAndDestroy(text);
	
      iListBox->HandleItemAdditionL();
   }
}

void CInfoContainer::SetGpsState(TInt aState)
{					   

   // aState:
   //   QualityMissing = 0,
   //   QualitySearching,
   //   QualityUseless,
   //   QualityPoor,
   //   QualityDecent,
   //   QualityExcellent,
   //   QualityDemohx,
   //   QualityDemo1x,
   //   QualityDemo2x,
   //   QualityDemo4x

   TInt GpsStrength[] = { -2, -1, 0, 33, 66, 100, -3, -3, -3, -3, };
      
   TInt size = TInt(sizeof(GpsStrength) / sizeof (TInt));
   
   if (aState >= size) {
      // Out of bounds, do nothing.
      return;
   }

   TInt state = GpsStrength[aState];

   _LIT(KPercentage, "%");
   _LIT(KTabulator, "\t");

   HBufC* formState = HBufC::NewLC(256);
   HBufC* title = iCoeEnv->AllocReadResourceLC(R_GPS_STATE_MSG);
   formState->Des().Append(KTabulator);
   formState->Des().Append(*title);
   formState->Des().Append(KTabulator);
   if (state == -2) {
      HBufC* tmp = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_GPS_NOT_CONNECTED);
      formState->Des().Append(*tmp);
      CleanupStack::PopAndDestroy(tmp);
   } else if (state == -1){
      HBufC* tmp = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_NO_GPS_POSITIONS);
      formState->Des().Append(*tmp);
      CleanupStack::PopAndDestroy(tmp);
   } else if (state == -3){
      HBufC* tmp = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_DEMO);
      formState->Des().Append(*tmp);
      CleanupStack::PopAndDestroy(tmp);
   } else {
      formState->Des().AppendNum(state, EDecimal);
      formState->Des().Append(KPercentage);      
   }
   ReplaceListEntryL(0, *formState);
   CleanupStack::PopAndDestroy(title);
   CleanupStack::PopAndDestroy(formState);
#if 0
   HBufC* state;
   if(aState) {
      state = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_GPS_CONNECTED);
   } else {
      state = iCoeEnv->AllocReadResourceLC(R_WAYFINDER_GPS_NOT_CONNECTED);
   }
   HBufC* title = iCoeEnv->AllocReadResourceLC(R_GPS_STATE_MSG);
   _LIT(KTabulator, "\t");

   HBufC* text = HBufC::NewLC(KTabulator().Length() + title->Length() + 
                              KTabulator().Length() + state->Length());
   TPtr connector = text->Des();

   connector.Append(KTabulator);
   connector.Append(*title);
   connector.Append(KTabulator);
   connector.Append(*state);

   ReplaceListEntryL(6, *text);
   CleanupStack::PopAndDestroy(text);
   CleanupStack::PopAndDestroy(title);
   CleanupStack::PopAndDestroy(state);
#endif
}

void CInfoContainer::SetSpeedLimit( TInt /*aLimit*/ ) 
{
}

// ---------------------------------------------------------
// CInfoContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CInfoContainer::SizeChanged() 
{
   if(iListBox) {
      iListBox->SetRect(Rect());
   }
}

void
CInfoContainer::PositionOfControlChanged(CCoeControl *control, TPoint pos)
{
   control->SetExtent(pos, control->MinimumSize());
}

// ---------------------------------------------------------
// CInfoContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CInfoContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CInfoContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CInfoContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex ) {
   case EBlindModeList:
      return iListBox;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CInfoContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CInfoContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   gc.SetClippingRect(aRect);
   gc.SetBrushColor(TRgb(KBackgroundRed, KBackgroundGreen, KBackgroundBlue));
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   
   gc.Clear(aRect);
}

// ---------------------------------------------------------
// CInfoContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CInfoContainer::HandleControlEventL (CCoeControl * /*aControl*/,
                                          TCoeEvent /*aEventType*/) 
{
   // TODO: Add your control event handler code here
}

void CInfoContainer::HandleResourceChange(TInt aType) {
}

TKeyResponse CInfoContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                             TEventCode aType ) 
{
   if ( aType != EEventKey ){ // Is not key event?
      return EKeyWasNotConsumed;
   }
	
   switch (aKeyEvent.iScanCode) {
      // Switches tab.
   case EStdKeyLeftArrow: // Left key.
   case EStdKeyRightArrow: // Right Key.
      return EKeyWasNotConsumed;
   case EStdKeyUpArrow:
   case EStdKeyDownArrow:
      return iListBox->OfferKeyEventL(aKeyEvent, aType);
   }

   if(iListBox) {
      return iListBox->OfferKeyEventL( aKeyEvent, aType );
   } else {
      return EKeyWasNotConsumed;
   }
}

void CInfoContainer::ReplaceListEntryL(TInt aNumber, const TDesC& aNewEntry) 
{
   if(iListBox) {
      CTextListBoxModel * model = iListBox->Model();
      if(model->NumberOfItems() > aNumber) {
         CDesCArray * itemArray = STATIC_CAST(CDesCArray *, 
                                              model->ItemTextArray());
         
         itemArray->Delete(aNumber);
         itemArray->InsertL(aNumber, aNewEntry);
         
         ActivateL();
      }	
   }
}

void CInfoContainer::UpdateReading()
{
   iListBox->SetCurrentItemIndex(0);
   iListBox->ItemDrawer()->SetItemMarkPosition(0);
   iListBox->ItemDrawer()->SetTopItemIndex(0);

   iListBox->SetFocus(ETrue);
   iListBox->DrawDeferred();
}

// End of File  

