/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "ExitQueryDlg.h"

#include "wayfinder.hrh"
#include "RsgInclude.h"

CExitQueryDlg::CExitQueryDlg() : 
   CAknQueryDialog()
{}


CExitQueryDlg::~CExitQueryDlg()
{
}

TInt CExitQueryDlg::RunDlgLD(const TDesC& aText)
{
   class CExitQueryDlg* self = new(ELeave) CExitQueryDlg();
   return self->ExecuteLD(R_WAYFINDER_QUERY_DIALOG, aText);
}

TKeyResponse CExitQueryDlg::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
                                           TEventCode aType)
{
   /*
     This code is removed because it caused the application to hang if the user 
     selects "Exit" in the Service Window View during startup.
     Instead we let the dialog close as it should when we change view.
     if (aKeyEvent.iCode == EKeyEscape) {
      return EKeyWasNotConsumed;
      }
   */
   return CAknQueryDialog::OfferKeyEventL(aKeyEvent, aType);
}


TBool CExitQueryDlg::OkToExitL(TInt aButtonId)
{
   if(aButtonId == EAknSoftkeyYes || aButtonId == EAknSoftkeyNo){
      return ETrue;
   }
   return EFalse;
}
