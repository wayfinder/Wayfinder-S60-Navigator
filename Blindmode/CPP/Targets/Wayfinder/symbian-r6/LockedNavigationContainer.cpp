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
#include <aknlists.h>
#include <hal.h>

#include "WayFinderConstants.h"		// view id constants
#include "RsgInclude.h"					// dodona strings and resources

#include "WayFinderAppUi.h"			// parent app
#include "TileMapControl.h"			// CTileMapControl
#include "MapFeatureHolder.h"			// CMapFeatureHolder
#include "MapInfoControl.h"			// CMapInfoControl
#include "VectorMapConnection.h"		// CVectorMapConnection
#include "DistancePrintingPolicy.h"
#include "UserDefinedScaleFeature.h"
#include "PathFinder.h"
#include "WFLayoutUtils.h"
#include "TimeOutTimer.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include <aknlists.h>  // for avrell style listbox
#include <aknpopup.h>  // for pop up menu
#include "memlog.h"
#include "TraceMacros.h"
#include "Dialogs.h"
#include "WFTextUtil.h"
#include "LockedNavigationContainer.h"
#include "FormatDirection.h"
#include "DirectionHelper.h"

#define KEY_SOFTKEY_LEFT    EStdKeyDevice0

enum TLockedNavControls {
   ELocationLabel = 0,
   ENumberControls
};

void CLockedNavigationContainer::SizeChanged() 
{
//   if(iLocationLabel)
//      iLocationLabel->SetExtent(TPoint(0, 0), TSize(1, 1)); 
}

TInt CLockedNavigationContainer::CountComponentControls() const
{
   return ENumberControls;
}

void CLockedNavigationContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();

   
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor(TRgb(KBackgroundRed, KBackgroundGreen, KBackgroundBlue));
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);    
}

void CLockedNavigationContainer::HandleControlEventL(CCoeControl * aControl, TCoeEvent aEventType)
{
    
}

void CLockedNavigationContainer::HandleResourceChange(TInt aType)
{ 
   CCoeControl::HandleResourceChange(aType);
   if (aType == KEikDynamicLayoutVariantSwitch) {
      SetRect(WFLayoutUtils::GetMainPaneRect());

      //iVicinityList->SetRect(Rect());
   }
}

void CLockedNavigationContainer::TimerExpired()
{
    
}

void
CLockedNavigationContainer::ConstructL(const TRect& aRect, 
                                       CLockedNavigationView* aView,
                                       CWayFinderAppUi * aWayFinderUI)
{
   iWayFinderUI = aWayFinderUI;
      
   CreateWindowL();
   
   SetRect(aRect);

   iLocationLabel = new (ELeave) CEikLabel;

   LOGNEW(iLocationLabel, CEikLabel);

   iLocationLabel->OverrideColorL( EColorLabelTextEmphasis, KRgbBlack );
   iLocationLabel->OverrideColorL( EColorLabelHighlightFullEmphasis, KRgbWhite );
   iLocationLabel->SetEmphasis( CEikLabel::EFullEmphasis );

   iLocationLabel->SetContainerWindowL(* this);
   iLocationLabel->SetTextL(_L(""));
   iLocationLabel->SetRect(Rect());

   
   ActivateL();
}

CLockedNavigationContainer::~CLockedNavigationContainer()
{
   LOGDEL(iLocationLabel);
   delete iLocationLabel;
   iLocationLabel = NULL;
}

TKeyResponse CLockedNavigationContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                                        TEventCode aType)
{
   //To prevent hanging of application
   if(aKeyEvent.iScanCode == KEY_SOFTKEY_LEFT)
      return EKeyWasConsumed;
   else
      return EKeyWasNotConsumed;
}

CCoeControl* CLockedNavigationContainer::ComponentControl(TInt aIndex) const
{
   switch(aIndex) 
      {
      case ELocationLabel:
         return iLocationLabel;
         break;
      default:
         return NULL;
      }
   return 0;
}


inline void CLockedNavigationContainer::GetDistance(TUint aDistance,
                                                    TDes &aText)
{
   isab::DistancePrintingPolicy::DistanceMode mode =
      isab::DistancePrintingPolicy::DistanceMode(GetDistanceModel());
   
   // Get the formated string with the correct metrics unit
   char* tmp = isab::
      DistancePrintingPolicy::convertDistance(aDistance,
                                              mode, 
                                              isab::DistancePrintingPolicy::Round);
   if (tmp) {
      WFTextUtil::char2TDes(aText, tmp);
      delete[] tmp;
   } else {
      aText.Copy(_L(" "));
   }
}


void
CLockedNavigationContainer::AppendDirection(   TPtr& connector,
                                               const MC2Direction& direction,
                                               int heading)
{
   DirectionHelper::AppendDirection( connector,
                                     direction,
                                     heading,
                                     iWayFinderUI->IsHeadingUsable(),
                                     iWayFinderUI->getDirectionType() );
}

void
CLockedNavigationContainer::UpdateText( HBufC* aTargetName,
                                        HBufC* aWhereAmI,
                                        TInt distance,
                                        MC2Direction targetDirection,
                                        MC2Direction userDirection)
{   
   HBufC* text = HBufC::NewLC(1024);
   TPtr connector = text->Des();
   
   
   _LIT(KBreak, "\n");
   _LIT(KSpace, " ");

   HBufC* yourPosition =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_LOCATION);

   HBufC* yourHeading =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_HEADING);
   
   HBufC* destinationReached =
      CEikonEnv::Static()->AllocReadResourceLC(R_WF_DESTINATION_REACHED);
   
   
   //Append target name on first row
   connector.Append(*aTargetName);
   connector.Append(KSpace);
   connector.Append(KBreak);

   //Append target direction
   AppendDirection(connector, targetDirection, userDirection.GetAngle() );
   
   connector.Append(KSpace);

   //Append target distance
   TBuf<50> distStr;
   GetDistance(distance, distStr);
   connector.Append(distStr);

   connector.Append(KBreak);
   connector.Append(KBreak);

   if(distance <= 5) {
      connector.Append(*destinationReached);
   } else {
      //Append user position
      connector.Append(*yourPosition);
      connector.Append(KBreak);
      connector.Append(*aWhereAmI);
      connector.Append(KBreak);
      
      if(iWayFinderUI->getDirectionType() == MC2Direction::DirectionBased) {
         //Append user direction
         connector.Append(*yourHeading);   
         connector.Append(KSpace);
         //Append target direction
         AppendDirection(connector, userDirection);
         
      }
   }
   
   iLocationLabel->SetTextL(*text);
   iLocationLabel->SetFocus(ETrue);
   iLocationLabel->DrawDeferred();
   
   CleanupStack::PopAndDestroy(destinationReached);
   CleanupStack::PopAndDestroy(yourHeading);
   CleanupStack::PopAndDestroy(yourPosition);
   CleanupStack::PopAndDestroy(text);
}

TInt
CLockedNavigationContainer::GetDistanceModel()
{
   return iWayFinderUI->GetDistanceMode();
}
