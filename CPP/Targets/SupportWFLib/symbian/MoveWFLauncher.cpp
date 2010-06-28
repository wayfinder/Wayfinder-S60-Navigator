/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
#include "MoveWFLauncher.h"

#include <apacmdln.h>
#include <eikdll.h>

TBool CMoveWayfinderLauncher::DoLaunchL(const TDesC & aParamFileDir, const TDesC & aApplicationPath)
{
   TRAPD( res,
         CMoveWayfinderLauncher * l = new(ELeave) CMoveWayfinderLauncher(aParamFileDir);
         CleanupStack::PushL(l);
         l->ConstructL(aApplicationPath);
         CleanupStack::PopAndDestroy(l);
   );
   if (res != KErrNone) {
      return ETrue;
   }
   return EFalse;
}

CMoveWayfinderLauncher::CMoveWayfinderLauncher(const TDesC & aParamFileDir) :
   iParamFileDir(), iFs()
{
   iParamFileDir.Copy(aParamFileDir);
}

CMoveWayfinderLauncher::~CMoveWayfinderLauncher()
{
   iFs.Close();
}

void CMoveWayfinderLauncher::FindMoveWayfinder(const TDesC & aApplicationPath)
{
   TFindFile finder(iFs);
   TParsePtrC parser(aApplicationPath);

   User::LeaveIfError(finder.FindByDir(parser.NameAndExt(), parser.Path() ));
   iMoveWfApp.Copy(finder.File());
}

void CMoveWayfinderLauncher::CreateCommandLine()
{
   RThread thisThread;
   TThreadId threadId = thisThread.Id();

#if   defined NAV2_CLIENT_SERIES60_V1
   _LIT8(KArch, "s60r1");
#elif defined NAV2_CLIENT_SERIES60_V2
   _LIT8(KArch, "s60r2");
#elif defined NAV2_CLIENT_SERIES60_V3
   _LIT8(KArch, "s60r3");
#elif defined NAV2_CLIENT_SERIES80
   _LIT8(KArch, "s80");
#elif defined NAV2_CLIENT_SERIES90_V1
   _LIT8(KArch, "s90");
#else
# error "Unknown architecture in CMoveWayfinderLauncher::CreateCommandLine"
#endif

   _LIT8(KSemicolon, ";");
   TBuf8<15> threadAsHex;
   threadAsHex.Num(static_cast<TUint>(threadId), EHex);

   iTailEnd.Zero();
   iTailEnd.Copy(KArch);
   iTailEnd.Append(KSemicolon);
   iTailEnd.Append(iParamFileDir);
   iTailEnd.Append(KSemicolon);
   iTailEnd.Append(threadAsHex);
}

void CMoveWayfinderLauncher::ConstructL(const TDesC & aApplicationPath)
{
   User::LeaveIfError(iFs.Connect());
   FindMoveWayfinder(aApplicationPath);
   CreateCommandLine();

   CApaCommandLine* cmdLine=CApaCommandLine::NewL();
   CleanupStack::PushL(cmdLine);

   cmdLine->SetLibraryNameL(iMoveWfApp);
   cmdLine->SetTailEndL(iTailEnd);
   cmdLine->SetCommandL(EApaCommandRun);

   EikDll::StartAppL(*cmdLine);

   CleanupStack::PopAndDestroy(cmdLine);
}

//functions from the WayfinderHelper namespace.

TBool WayfinderHelper::IsFilePresentL(const TDesC& aPath, const TDesC& aFile)
{
   class RFs fs;
   User::LeaveIfError(fs.Connect());
   CleanupClosePushL(fs);
   class TFindFile finder(fs);
   TInt res = finder.FindByDir(aPath, aFile);
   CleanupStack::PopAndDestroy(&fs);
   return res == KErrNone;
}

TBool WayfinderHelper::ShowMoveWayfinder()
{
#if   defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V2
   _LIT(KGetWayfinderApp, "MoveWayfinder.app");
   _LIT(KGetWayfinderDir, "\\system\\apps\\MoveWayfinder\\");
#elif defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
   _LIT(KGetWayfinderApp, "MoveWayfinder80.app");
   _LIT(KGetWayfinderDir, "\\system\\apps\\MoveWayfinder80\\");
#else
#error Unknown arch in ShowMoveWayfinder!
#endif
   return IsFilePresentL(KGetWayfinderDir, KGetWayfinderApp);
}

TBool WayfinderHelper::ShowGetWayfinder()
{
   _LIT(KGetWayfinderApp, "GetWayfinder.app");
   _LIT(KGetWayfinderDir, "\\system\\apps\\GetWayfinder\\");
   return IsFilePresentL(KGetWayfinderDir, KGetWayfinderApp);
}


#endif
