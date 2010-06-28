/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SocketsBaseDialog.h"
#include <eikbtgpc.h>
#include <eikseced.h>
#include <eikmfne.h>
#include <eikedwin.h>

CSocketsBaseDialog::CSocketsBaseDialog(CMessageHandler& aAppUi)
: iAppUi(aAppUi)
    {
    }

void CSocketsBaseDialog::SetTextL(TInt aControl, const TDesC& aText)
    {
    // Copy text into an edwin type control
    static_cast<CEikEdwin*>(Control(aControl))->SetTextL(&aText);
    Control(aControl)->DrawNow();
    }

void CSocketsBaseDialog::SetSecretText(TInt aControl, const TDesC& aText)
    {
    // Copy text into a secret editor type control
    static_cast<CEikSecretEditor*>(Control(aControl))->SetText(aText);
    Control(aControl)->DrawNow();
    }

void CSocketsBaseDialog::SetNumber(TInt aControl, TInt aNumber)
    {
    // Copy number into a number editor type control
    static_cast<CEikNumberEditor*>(Control(aControl))->SetNumber(aNumber);
    Control(aControl)->DrawNow();
    }

void CSocketsBaseDialog::GetText(TInt aControl, TDes& aText)
    {
    // Copy text out of an edwin type control
    static_cast<CEikEdwin*>(Control(aControl))->GetText(aText);
    }

void CSocketsBaseDialog::GetSecretText(TInt aControl, TDes& aText)
    {
    // Copy text out of a secret editor type control
    static_cast<CEikSecretEditor*>(Control(aControl))->GetText(aText);
    }

TInt CSocketsBaseDialog::GetNumber(TInt aControl)
    {
    // Copy number out of a number editor type control
    return static_cast<CEikNumberEditor*>(Control(aControl))->Number();
    }

CMessageHandler& CSocketsBaseDialog::AppUi() const
    {
    return(iAppUi);
    }

