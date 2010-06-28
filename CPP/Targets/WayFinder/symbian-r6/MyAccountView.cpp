/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// S60 inlcudes
#include <aknviewappui.h>
#include <eikmenub.h>
#include <avkon.hrh>
#include <barsread.h>
#include <stringloader.h>
#include <eiklabel.h>
#include <eikenv.h>
#include <gdi.h>
#include <eikseced.h>
#include <akncontext.h>
#include <akntitle.h>
#include <eikbtgpc.h>

// project includes
#include <wayfinder8_s60v5.rsg>
#include "wayfinder.hrh"
#include "WayFinderConstants.h"
#include "WayFinderAppUi.h"
#include "WFTextUtil.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiParameterEnums.h"

#include "MyAccountView.h"
#include "MyAccountContainer.h"

using namespace isab;

/**
 * First phase of Symbian two-phase construction. Should not contain any
 * code that could leave.
 */
CMyAccountView::CMyAccountView(CWayFinderAppUi* aUi) :
   CViewBase(aUi)
{
   iMyAccountContainer = NULL;

}

/** 
 * The view's destructor removes the container from the control
 * stack and destroys it.
 */
CMyAccountView::~CMyAccountView()
{
   delete iMyAccountContainer;
   iMyAccountContainer = NULL;

   iPassword.Close();
   iUsername.Close();
}

/**
 * Symbian two-phase constructor.
 * This creates an instance then calls the second-phase constructor
 * without leaving the instance on the cleanup stack.
 * @return new instance of CMyAccountViewView
 */
CMyAccountView* CMyAccountView::NewL(CWayFinderAppUi* aUi, 
                                         isab::Log* aLog)
	{
	CMyAccountView* self = CMyAccountView::NewLC(aUi,aLog);
	CleanupStack::Pop( self );
	return self;
	}

/**
 * Symbian two-phase constructor.
 * This creates an instance, pushes it on the cleanup stack,
 * then calls the second-phase constructor.
 * @return new instance of CMyAccountViewView
 */
CMyAccountView* CMyAccountView::NewLC(CWayFinderAppUi* aUi, isab::Log* /*aLog*/)
	{
	CMyAccountView* self = new ( ELeave ) CMyAccountView(aUi);
	CleanupStack::PushL( self );
	self->ConstructL();
	return self;
	}

/**
 * Second-phase constructor for view.  
 * Initialize contents from resource.
 */
void
CMyAccountView::ConstructL()
{
   BaseConstructL(R_MY_ACCOUNT_VIEW);

   iPassword.Create(KBuf64Length);
   iUsername.Create(KBuf64Length);
}

/**
 * @return The UID for this view
 */
TUid
CMyAccountView::Id() const
{
   return KMyAccountViewId;
}

/**
 * Handle a command for this My Account view 
 * The user can select either Update or Cancel
 * @param aCommand command id to be handled
 */
void
CMyAccountView::HandleCommandL(TInt aCommand)
{
   switch (aCommand) {
   case EAknSoftkeyCancel:
      // user has selected Cancel
      iWayfinderAppUi->pop();
      break;
   case EAknSoftkeyOk:
      // user has selected update, so check if password has changed
      iMyAccountContainer->GetPassword(iPassword);
      if (iPassword.Length()) {
         char* tmp_str = WFTextUtil::newTDesDupL(iPassword);
         GeneralParameterMess* gen_mess;
         gen_mess = new (ELeave) GeneralParameterMess(
               GuiProtEnums::paramWebPassword, tmp_str);

         iWayfinderAppUi->SendMessageL(gen_mess);
         gen_mess->deleteMembers();
         delete gen_mess; 
      }
      // user has selected update
      iWayfinderAppUi->pop();
      break;
   default:
      {
         CAknView::HandleCommandL(aCommand);
         break;
      }
   }
}

/**
 * Handles user actions during activation of the view, 
 * such as initializing the content.
 */
void CMyAccountView::DoActivateL(const TVwsViewId& /*aPrevViewId*/,
                            TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
{
   SetupStatusPaneL();
   HBufC* titleText = StringLoader::LoadLC(R_S60_MY_ACCOUNT_TXT);
   iWayfinderAppUi->setTitleText(titleText->Des());
   CleanupStack::PopAndDestroy(titleText);
   
   if (iMyAccountContainer == NULL) {
      iMyAccountContainer = CreateContainerL();
      iMyAccountContainer->SetMopParent(this);
      AppUi()->AddToStackL(*this, iMyAccountContainer);
      
      CEikButtonGroupContainer* buttonGroupContainer=CEikButtonGroupContainer::Current();
      buttonGroupContainer->MakeCommandVisible(EAknSoftkeyOk,EFalse);
 
   }

}

/*
 * Called when de-activating view
 */
void
CMyAccountView::DoDeactivate()
{
   CleanupStatusPane();

   if (iMyAccountContainer != NULL) {
      AppUi()->RemoveFromViewStack(*this, iMyAccountContainer);
      delete iMyAccountContainer;
      iMyAccountContainer = NULL;
   }

}

/** 
 * Handle status pane size change for this view (override)
 */
void
CMyAccountView::HandleStatusPaneSizeChange()
{
   CAknView::HandleStatusPaneSizeChange();

   // this may fail, but we're not able to propagate exceptions here
   TVwsViewId view;
   AppUi()->GetActiveViewId(view);
   if (view.iViewUid == Id()) {
      TInt result;
      TRAP( result, SetupStatusPaneL() );
   }

}

void
CMyAccountView::SetupStatusPaneL()
{
   // reset the context pane
   TUid contextPaneUid = TUid::Uid(EEikStatusPaneUidContext);
   CEikStatusPaneBase::TPaneCapabilities subPaneContext =
         StatusPane()->PaneCapabilities(contextPaneUid);
   if (subPaneContext.IsPresent() && subPaneContext.IsAppOwned()) {
      CAknContextPane* context =
            static_cast<CAknContextPane*> (StatusPane()->ControlL(
                  contextPaneUid));
      context->SetPictureToDefaultL();
   }
}

void
CMyAccountView::CleanupStatusPane()
{
}

/**
 * Creates the top-level container for the view.  You may modify this method's
 * contents and the CMyAccountView::NewL() signature as needed to initialize the
 * container, but the signature for this method is fixed.
 * @return new initialized instance of CMyAccountView
 */
CMyAccountContainer*
CMyAccountView::CreateContainerL()
{
   return CMyAccountContainer::NewL(ClientRect(), NULL, this, iUsername);
}

void
CMyAccountView::SetUserName(const char* aUser)
{
   // copy the char* username and password into RBuf
   TInt i = 0;
   iUsername.Zero();
   while (aUser[i]) {
      iUsername.Append(aUser[i]);
      i++;
   }
}


