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
#include <avkon.hrh>
#include <eikmenup.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wficons.mbg"
#include "wayfinder.hrh"
#include "GuiDataStore.h"
#include "StartPageView.h"
#include "StartPageContainer.h" 

#include "WAXParameterContainer.h"
#include "WAXParameter.h"
#include "WAXParameterTools.h"
#include "MC2Coordinate.h"

// ================= MEMBER FUNCTIONS =======================

CStartPageView::CStartPageView(isab::Log* aLog, TInt aMode) : 
   iLog(aLog), iMode(aMode)
{}
// ---------------------------------------------------------
// CStartPageView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CStartPageView::ConstructL( CWayFinderAppUi* aWayFinderUI )
{
   BaseConstructL( R_WAYFINDER_STARTPAGE_VIEW );
   iWayFinderUI = aWayFinderUI;
}

CStartPageView* CStartPageView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog, 
                                      TInt aMode)
{
   CStartPageView* self = new (ELeave) CStartPageView(aLog, aMode);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CStartPageView* CStartPageView::NewL(CWayFinderAppUi* aUi,
                                           isab::Log* aLog, 
                                           TInt aMode)
{
   CStartPageView *self = CStartPageView::NewLC(aUi, aLog, aMode);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CStartPageView::~CStartPageView()
// ?implementation_description
// ---------------------------------------------------------
//
CStartPageView::~CStartPageView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

// ---------------------------------------------------------
// TUid CStartPageView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CStartPageView::Id() const
{
   return KStartPageViewId;
}

GuiDataStore*
CStartPageView::GetGuiDataStore()
{
   return iWayFinderUI->GetGuiDataStore();
}
TBool
CStartPageView::IsGpsAllowed()
{
   return iWayFinderUI->IsGpsAllowed();
}
void
CStartPageView::GpsStatus(TBool /*onoff*/)
{
/*    if (iContainer) { */
/*       iContainer->SetList(onoff); */
/*    } */
}

void
CStartPageView::HandleCommandL(const class CWAXParameterContainer& aCont)
{
   iWayFinderUI->GotoStartViewL();
}

// ---------------------------------------------------------
// CStartPageView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CStartPageView::HandleCommandL(TInt aCommand)
{
   switch ( aCommand )
   {
   case EAknSoftkeyOk:
      {
         iEikonEnv->InfoMsg( _L("view1 ok") );
         break;
      }
   case EAknSoftkeyExit:
      {
         AppUi()->HandleCommandL(EAknSoftkeyExit);
         break;
      }
   case EWayFinderCmdStartPageOpen:
      {
         iContainer->GoToSelection();
         break;
      }
   case EWayFinderCmdStartPageUpgradeInfo:
   case EWayFinderCmdStartPageUpgradeInfo2:
   case EWayFinderCmdStartPageUpgrade:
      {
         iWayFinderUI->ShowUpgradeInfoL( CWayFinderAppUi::EUpgrade );
         break;
      }
   case EWayFinderCmdPositionSelect:
      {
         AppUi()->HandleCommandL( EWayFinderCmdPositionSelect );
         break;
      }
   case EWayFinderCmdStartPageNew:
      {
         AppUi()->HandleCommandL( EWayFinderCmdNewDest );
         break;
      }
   default:
      {
         AppUi()->HandleCommandL( aCommand );
         break;
      }
   }
}

// ---------------------------------------------------------
// CStartPageView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CStartPageView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CStartPageView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CStartPageView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CStartPageView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CStartPageView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                                  TUid /*aCustomMessageId*/,
                                  const TDesC8& /*aCustomMessage*/)
{
   if (!iContainer){
      iContainer = new (ELeave) CStartPageContainer(iLog, iMode);
      LOGNEW(iContainer, CStartPageContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect(), this );
      AppUi()->AddToStackL( *this, iContainer );
   }
//    if( aCustomMessageId.iUid == EShowUpgrade ){
//       iWayFinderUI->ShowUpgradeDialogL(CWayFinderAppUi::EActivate, Id());
//    }
}

void CStartPageView::DoDeactivate()
{
   // By not deleting the container the switch to main 
   // is much faster.
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

// End of File

