/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "SisInstaller.h"
#include "SisInstallerController.h"

//Include systemheaders
#include <apacmdln.h>
#include <eikdll.h>

#if defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
_LIT(KInstallerApp, "z:\\system\\apps\\appinst\\appinst.app");
#elif defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
_LIT(KInstallerApp, "z:\\system\\apps\\instapp\\instapp.app");
#elif defined NAV2_CLIENT_UIQ
_LIT(KInstallerApp, "z:\\system\\apps\\qappinst\\qappinst.app");
#elif defined NAV2_CLIENT_UIQ3
_LIT(KInstallerApp, "z:\\sys\\bin\\qappinst\\qappinst.exe");
#else
# error "Unknown system"
#endif

CSisInstaller::CSisInstaller(class MProcessWaiterReceiver* aAppUI) :
   CActive(EPriorityStandard), 
   iAppUI(aAppUI)
{
   CActiveScheduler::Add(this);
}

class CSisInstaller* CSisInstaller::NewLC(class MProcessWaiterReceiver* aAppUI)
{
   class CSisInstaller* sisInstaller = new (ELeave) CSisInstaller(aAppUI);
   CleanupStack::PushL(sisInstaller);
   return sisInstaller;
}

class CSisInstaller* CSisInstaller::NewL(class MProcessWaiterReceiver* aAppUI)
{
   class CSisInstaller* sisInstaller = CSisInstaller::NewLC(aAppUI);
   CleanupStack::Pop(sisInstaller);
   return sisInstaller;
}

void CSisInstaller::ConstructL()
{

}

TBool CSisInstaller::StartInstallL(const TDesC& file, class MSisInstallerController* aController)
{
   iController = aController;
   file.Length();
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   return EFalse;
#else
   TBool retVal = ETrue;
#if defined NAV2_CLIENT_UIQ
   _LIT8(KTailEnd, "INSTALL");
   class CApaCommandLine* cmdLine=CApaCommandLine::NewLC();
   cmdLine->SetLibraryNameL(KInstallerApp);
   cmdLine->SetDocumentNameL(file);
   cmdLine->SetTailEndL(KTailEnd);
   cmdLine->SetCommandL(EApaCommandOpen);
#else
   _LIT8(KTailEnd, "");
   class CApaCommandLine* cmdLine=CApaCommandLine::NewLC();
   cmdLine->SetLibraryNameL(KInstallerApp);
   cmdLine->SetDocumentNameL(file);
   cmdLine->SetTailEndL(KTailEnd);
   cmdLine->SetCommandL(EApaCommandRun);
#endif
   TThreadId installerThreadId = EikDll::StartAppL(*cmdLine);
   if (iInstallerThread.Open(installerThreadId) == KErrNone) {
      iInstallerThread.Logon(iStatus);
      SetActive();
   } else {
      retVal = EFalse;
   }
   CleanupStack::PopAndDestroy(cmdLine);
   return retVal;
#endif
}

void CSisInstaller::RunL()
{
   if(iStatus != KErrNone){
      //XXX Something went wrong, do something! What??
      //This will never happen unless the thread crashes.
      iAppUI->waitDone(iStatus); //???
   }
   else{
      if (iController) {
         if (iController->ControlSisInstallation()) {
            iAppUI->waitDone(iStatus);
         } else {
            iAppUI->waitDone(KErrCancel);
         }
      } else {
         iAppUI->waitDone(KErrNone);
      }
   }
}

void CSisInstaller::DoCancel()
{
   iInstallerThread.LogonCancel(iStatus);
}

