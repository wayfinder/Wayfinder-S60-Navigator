/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <e32std.h>               // TDesC 
#include "LaunchWapBrowserUtil.h" // LaunchWapBrowserUtil
#include <eikenv.h>               // CEikonEnv
#include <f32file.h>              // RFs
#include <apacmdln.h>             // CApaCommandLine
#include <apgcli.h>               // RApaLsSession 
  
// Path to browser
_LIT(KBrowserPath,"\\system\\apps\\opera\\opera.app");

TBool
LaunchWapBrowserUtil::LaunchWapBrowser(const TDesC& aUrl)
{
#if defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES80

   TFileName fnAppPath(KBrowserPath);
   // File server session
   class RFs& fsSession = CEikonEnv::Static()->FsSession(); 
   // Find web browser
   class TFindFile findFile(fsSession);

   User::LeaveIfError(findFile.FindByDir(fnAppPath, KNullDesC));

   class CApaCommandLine* cmdLine = CApaCommandLine::NewLC();
   cmdLine->SetLibraryNameL(findFile.File());
   cmdLine->SetCommandL(EApaCommandOpen);
   cmdLine->SetDocumentNameL(aUrl);

   class RApaLsSession ls;
   User::LeaveIfError(ls.Connect());
   CleanupClosePushL(ls);
   User::LeaveIfError(ls.StartApp(*cmdLine));
   // Destroy ls and cmdLine
   CleanupStack::PopAndDestroy(&ls);
   CleanupStack::PopAndDestroy(cmdLine);

#else
# error This code not implemented!
#endif
   return ETrue;
}
