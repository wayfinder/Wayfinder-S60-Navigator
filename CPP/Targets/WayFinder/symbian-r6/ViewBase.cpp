/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ViewBase.h"
#include "WayFinderAppUi.h"

CViewBase::CViewBase(class CWayFinderAppUi* aWayfinderAppUi) :
   iWayfinderAppUi(aWayfinderAppUi),
   iCustomMessageId(ENoMessage),
   iActivated(ETrue)
{
}

void CViewBase::ViewBaseConstructL(TInt aResourceId)
{
   BaseConstructL(aResourceId);
}

CViewBase::~CViewBase()
{
}

TInt CViewBase::GetViewState()
{
   return iCustomMessageId;
}

void CViewBase::DoActivateL(const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& /*aCustomMessage*/)
{
   if (iWayfinderAppUi->GetAppUid() == aPrevViewId.iAppUid) {
      iPrevViewId = aPrevViewId;
   } 
   if (IsWFCustomCommandId(aCustomMessageId.iUid) &&
       (aCustomMessageId.iUid != EKeepOldStateMessage)) {
      iCustomMessageId = aCustomMessageId.iUid;
   } 

   // If the exit dialog is up we should not display the cba
   // buttons, if so the view buttons will be above the dialog
   // buttons. This will happen if the user presses the application
   // key and goes to the main menu and then puts wayfinder to the 
   // foreground again.
   if(iWayfinderAppUi->IsDialogDisplayed()) {
      ShowMenuBar(EFalse);
   } else {
      ShowMenuBar(ETrue);
   }
}

void CViewBase::ShowMenuBar(TBool aShow)
{
   Cba()->MakeVisible(aShow);   
}

TBool CViewBase::IsActivated() 
{
	return iActivated;
}
