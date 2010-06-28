/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYACCOUNTVIEW_H
#define MYACCOUNTVIEW_H

// S60 includes
#include <aknview.h>

// project includes
#include "ViewBase.h"
#include "Log.h"
#include "memlog.h"

class CMyAccountContainer;

/**
 * Avkon view class for MyAccountView. 
 * It owns the container control CMyAccountContainer.
 * @class	CMyAccountView MyAccountView.h
 */

class CMyAccountView : public CViewBase
{

public:
	// constructors and destructor
	CMyAccountView(CWayFinderAppUi* aUi);
	static CMyAccountView* NewL(CWayFinderAppUi* aUi,isab::Log* aLog);
	static CMyAccountView* NewLC(CWayFinderAppUi* aUi,isab::Log* /* aLog */);        
   void
   ConstructL();
   virtual
   ~CMyAccountView();

   // from base class CAknView
   TUid Id() const;
   void HandleCommandL(TInt aCommand);

   void SetUserName(const char* aUser);
   const unsigned char* GetPassword();

   CMyAccountContainer* CreateContainerL();
protected:
   // from base class CAknView
   void DoActivateL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId,
               const TDesC8& aCustomMessage);
   void DoDeactivate();
   void HandleStatusPaneSizeChange();

private:
   void
   SetupStatusPaneL();
   void
   CleanupStatusPane();

   CMyAccountContainer* iMyAccountContainer;
   RBuf iUsername;
   RBuf iPassword;
};

#endif // MYACCOUNTVIEWVIEW_H
