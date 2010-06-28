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
#include  <avkon.hrh>

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
/* #include "PdaNavigatorProt.h" */
#include "GuiProt/GuiProtEnums.h"
#include  "WayFinderAppUi.h"

#include  "RsgInclude.h"
#include  "ContactsView.h"

#include  "ContactsContainer.h" 
#include "memlog.h"
// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CContactsView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CContactsView::ConstructL()
{
   BaseConstructL( R_WAYFINDER_CONTACTS_VIEW );
}

// ---------------------------------------------------------
// CContactsView::~CContactsView()
// ?implementation_description
// ---------------------------------------------------------
//
CContactsView::~CContactsView()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
}

// ---------------------------------------------------------
// TUid CContactsView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CContactsView::Id() const
{
   return KContactsViewId;
}

// ---------------------------------------------------------
// CContactsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CContactsView::HandleCommandL(TInt aCommand)
{   
   switch ( aCommand )
   {
   case EAknSoftkeyOk:
      {
         iEikonEnv->InfoMsg( _L("view2 ok") );
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
// CContactsView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CContactsView::HandleClientRectChange()
{
   if ( iContainer ){
      iContainer->SetRect( ClientRect() );
   }
}

// ---------------------------------------------------------
// CContactsView::DynInitMenuPaneL()
// ---------------------------------------------------------
//
void CContactsView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
{
   AppUi()->DynInitMenuPaneL( aResourceId, aMenuPane );
}

// ---------------------------------------------------------
// CContactsView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CContactsView::DoActivateL( const TVwsViewId& aPrevViewId,
                               TUid aCustomMessageId,
                               const TDesC8& aCustomMessage)
{
   CViewBase::DoActivateL(aPrevViewId, aCustomMessageId, aCustomMessage);
   if ( !iContainer ){
      iContainer = new (ELeave) CContactsContainer(iLog);
      LOGNEW(iContainer, CContactsContainer);
      iContainer->SetMopParent(this);
      iContainer->ConstructL( ClientRect() );
      AppUi()->AddToStackL( *this, iContainer );
   }
}

// ---------------------------------------------------------
// CContactsView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CContactsView::DoDeactivate()
{
   if ( iContainer ){
      AppUi()->RemoveFromViewStack( *this, iContainer );
   }

   LOGDEL(iContainer);
   delete iContainer;
   iContainer = NULL;
}

// End of File
