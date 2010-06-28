/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef NAV2_CLIENT_SERIES60_V28

// INCLUDE FILES
#include  <avkon.hrh>

#include <eikmenup.h>
#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "WayFinderAppUi.h"

#include "RsgInclude.h"
#include "wayfinder.hrh"
#include "ContentWindowView.h"
#include "MapView.h"
#include "ContentWindowContainer.h"
/* #include "NavPacket.h" */
#include "memlog.h"
#include "BufferArray.h"
#include "DataHolder.h"
#include "Quality.h"

#include "DistancePrintingPolicy.h"

#include "PathFinder.h"

// ================= MEMBER FUNCTIONS =======================

CContentWindowView::CContentWindowView(isab::Log* aLog) : iLog(aLog) {}
// ---------------------------------------------------------
// CContentWindowView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CContentWindowView::ConstructL( CWayFinderAppUi* aWayFinderUI )
{
   BaseConstructL( R_WAYFINDER_INFO_VIEW );
   iWayFinderUI = aWayFinderUI;
   iDistance = -1;
}

CContentWindowView* CContentWindowView::NewLC(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CContentWindowView* self = new (ELeave) CContentWindowView(aLog);
   CleanupStack::PushL(self);
   self->ConstructL(aUi);
   return self;
}

class CContentWindowView* CContentWindowView::NewL(CWayFinderAppUi* aUi, isab::Log* aLog)
{
   CContentWindowView *self = CContentWindowView::NewLC(aUi, aLog);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CContentWindowView::~CContentWindowView()
// ?implementation_description
// ---------------------------------------------------------
//
CContentWindowView::~CContentWindowView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

// ---------------------------------------------------------
// TUid CContentWindowView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CContentWindowView::Id() const
{
   return KContentWindowViewId;
}

// ---------------------------------------------------------
// CContentWindowView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CContentWindowView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
   {
   case EWayFinderCmdInfoShow:
      {
         iContainer->Debug();
         break;
      }
   case EAknSoftkeyBack:
      {
         AppUi()->HandleCommandL( aCommand );
         break;
      }
   default:
      break;
   }
}

void
CContentWindowView::DataReceived(const isab::DataGuiMess* aMess,
      const char *aUrl)
{
   if (iContainer) {
      iContainer->DataReceived(aMess, aUrl);
   }
}

void
CContentWindowView::RequestData(const TDesC& aUrl)
{
   iWayFinderUI->RequestData(aUrl);
}

// ---------------------------------------------------------
// CContentWindowView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CContentWindowView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CContentWindowView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CContentWindowView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CContentWindowView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CContentWindowView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& /*aCustomMessage*/)
{
   if( aPrevViewId.iViewUid != KSettingsViewId &&
       aPrevViewId.iViewUid != KContentWindowViewId)
       iPrevViewId = aPrevViewId;
   
   iCustomMessageId = aCustomMessageId;

   if ( !iContainer ){
      iContainer = CContentWindowContainer::NewL(
            this,
            ClientRect(),
            iWayFinderUI->iPathManager->GetMbmName(),
            iLog
            );
      AppUi()->AddToStackL( *this, iContainer );
   }
}

// ---------------------------------------------------------
// CContentWindowView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CContentWindowView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}
#endif

// End of File
