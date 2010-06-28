/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WFInstallerController.h"
#include "VersionFileReader.h"
#include <eikenv.h>

CWFInstallerController::CWFInstallerController(enum TCompareType aCompareType) :
   iCompareType(aCompareType)
{
}

void CWFInstallerController::ConstructL(const TDesC8& aVersion, 
                                        const TDesC& aVersionPath) 
{
   iVersion = aVersion.AllocL();
   iVersionPath = aVersionPath.AllocL();
   iVersionFileReader = CVersionFileReader::NewL();
}

CWFInstallerController::~CWFInstallerController() 
{
   delete iVersion;
   delete iVersionPath;
   delete iVersionFileReader;
}

class CWFInstallerController* 
CWFInstallerController::NewL(const TDesC8& aVersion,  
                             const TDesC& aVersionPath, 
                             enum TCompareType aCompareType)
{
   class CWFInstallerController* self = 
      CWFInstallerController::NewLC(aVersion, aVersionPath, aCompareType);
   CleanupStack::Pop(self);
   return self;
}

class CWFInstallerController* 
CWFInstallerController::NewLC(const TDesC8& aVersion, 
                              const TDesC& aVersionPath, 
                              enum TCompareType aCompareType)
{
   class CWFInstallerController* self = 
      new (ELeave) CWFInstallerController(aCompareType);
   CleanupStack::PushL(self);
   self->ConstructL(aVersion, aVersionPath);
   return self;
}

TBool CWFInstallerController::ControlSisInstallation()
{
   TParsePtrC pathParser(*iVersionPath);
   if (!pathParser.DrivePresent()) {
      TFindFile finder(CEikonEnv::Static()->FsSession());
      if (finder.FindByPath(pathParser.NameAndExt(), iVersionPath) == KErrNone) {
         iVersionPath = iVersionPath->ReAllocL(iVersionPath->Length()+8);
         iVersionPath->Des().Copy(finder.File());
#if defined __WINS__ || defined __WINSCW__
         iVersionPath->Des()[0] = 'c';
#endif
      } else {
         return EFalse;
      }
   }

   iVersionFileReader->ReadOldVersionFileL(*iVersionPath);

   switch (iCompareType) 
      {
      case EVersion:
         if (iVersionFileReader->GetOldVersion(CVersionFileReader::EOldMajorMinorVersion) != 
             *iVersion) {
            // Compare app versions, install failed.
            return EFalse;
         }
         break;
      case ETimeStamp:
         if (iVersionFileReader->GetOldVersion(CVersionFileReader::EOldMlfwVersion) != 
             *iVersion) {
            // Compare app versions, install failed.
            return EFalse;
         }
         break;
      }
   return ETrue;
}
