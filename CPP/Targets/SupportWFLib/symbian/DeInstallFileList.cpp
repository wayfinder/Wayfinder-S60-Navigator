/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <eikenv.h>
#include <f32file.h>

#include "DeInstallFileList.h"

CDeInstallFileList::CDeInstallFileList(RFs& aFsSession) :
   iFsSession(aFsSession)
{
}

CDeInstallFileList::~CDeInstallFileList()
{
   if(iFileList) {
      iFileList->Reset();
      delete iFileList;
   }
}

CDeInstallFileList* CDeInstallFileList::NewLC()
{
   RFs& fsSession = CEikonEnv::Static()->FsSession();
   CDeInstallFileList* self = new (ELeave) CDeInstallFileList(fsSession);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CDeInstallFileList* CDeInstallFileList::NewL()
{
   CDeInstallFileList* self = CDeInstallFileList::NewLC();
   CleanupStack::Pop(self);
   return self;
}

void CDeInstallFileList::ConstructL()
{
   iFileList = new (ELeave) CDesCArraySeg(10);
}

TPtrC CDeInstallFileList::GetFile(TInt aIndex) const
{
   if (aIndex > 0 && aIndex < iFileList->Count()) {
      return iFileList->MdcaPoint(aIndex);
   } else {
      return TPtrC();
   }
}

TInt CDeInstallFileList::AddFileL(const TDesC& aFile) 
{
   if (aFile.Length() > 0) {
      iFileList->AppendL(aFile);
   } else {
      return -1;
   }
   return 0;
}

void CDeInstallFileList::ClearFile(TInt aIndex) 
{
   if (aIndex > 0 && aIndex < iFileList->Count()) {
      iFileList->Delete(aIndex);
   }
}

void CDeInstallFileList::ClearFileList()
{
   iFileList->Reset();
}

TInt CDeInstallFileList::WriteFileListToDisc(const TDesC& aPath) 
{
   HBufC* fileName = HBufC::NewLC(256);
   TPtr pFileName = fileName->Des();
   pFileName.Copy(aPath);
   //TParsePtrC parser(pFileName);
   //pFileName.Copy(parser.DriveAndPath());
   TInt ret = iFsSession.MkDirAll(pFileName);
   if (ret == KErrNone || ret == KErrAlreadyExists) {
      pFileName.Append(KUnInstallFile);
      RFile file;
      ret = file.Open(iFsSession, *fileName, EFileStreamText | EFileWrite);
      if (ret == KErrNotFound) {
         ret = file.Create(iFsSession, *fileName, EFileStreamText | EFileWrite);
      }
      CleanupStack::PopAndDestroy(fileName);
      if (ret != KErrNone) {
         /* Could not open file, return the error code */
         return ret;
      }
      TFileText textFile;
      textFile.Set(file);
      for (int i = 0; i < iFileList->Count(); i++) {
         textFile.Write(iFileList->MdcaPoint(i));
      }
      file.Close();
   } else {
      CleanupStack::PopAndDestroy(fileName);
      return ret;
   }
   return KErrNone;
}

TInt CDeInstallFileList::WriteFileListToDisc(const TDesC& aDrive, 
                                             const TDesC& aPath) 
{
   _LIT(KColon, ":");
   HBufC* aFileName = HBufC::NewLC(256);
   aFileName->Des().Copy(aDrive);
   aFileName->Des().Append(KColon);
   aFileName->Des().Append(aPath);
   TInt ret = iFsSession.MkDirAll(*aFileName);
   if (ret == KErrNone || ret == KErrAlreadyExists) {
      aFileName->Des().Append(KUnInstallFile);
      RFile file;
      ret = file.Open(iFsSession, *aFileName, EFileStreamText | EFileWrite);
      if (ret == KErrNotFound) {
         ret = file.Create(iFsSession, *aFileName, EFileStreamText | EFileWrite);
      }
      CleanupStack::PopAndDestroy(aFileName);
      if (ret != KErrNone) {
         /* Could not open file, return the error code */
         return ret;
      }
      TFileText textFile;
      textFile.Set(file);
      for (int i = 0; i < iFileList->Count(); i++) {
         textFile.Write(iFileList->MdcaPoint(i));
      }
      file.Close();
   } else {
      CleanupStack::PopAndDestroy(aFileName);
      return ret;
   }
   return KErrNone;
}

TInt CDeInstallFileList::DeleteFileListFromDisc(const TDesC& aDrive, 
                                                const TDesC& aPath)
{
   _LIT(KColon, ":");
   HBufC* aFileName = HBufC::NewLC(256);
   aFileName->Des().Copy(aDrive);
   aFileName->Des().Append(KColon);
   aFileName->Des().Append(aPath);
   aFileName->Des().Append(KUnInstallFile);
   TInt ret = iFsSession.Delete(*aFileName);
   CleanupStack::PopAndDestroy(aFileName);
   if (ret == KErrNone || ret == KErrNotFound) {
      return KErrNone;
   }
   return ret;
}
