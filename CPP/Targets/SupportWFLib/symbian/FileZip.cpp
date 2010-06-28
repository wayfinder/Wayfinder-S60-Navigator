/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef NAV2_CLIENT_SERIES60_V1
// Include the class definition header
#include "FileZip.h"

// System headers
#include <zipfile.h>
#include <eikenv.h>

// user headers.
#include "WFTextUtil.h"
#include "DeInstallFileList.h"

_LIT(KFileZip, "FileZip");
enum TFileZipPanic {
   ENoDriveInZipFileName = 0,
   ECopyNoMem            = 1,
};
#define ASSERT_ALWAYS(cond_,panic_) __ASSERT_ALWAYS((cond_), User::Panic(KFileZip, (panic_)))
#define ASSERT_DEBUG(cond_,panic_) __ASSERT_DEBUG((cond_), User::Panic(KFileZip, (panic_)))

void CFileZip::ConstructL()
{
   //Doesn't do anything yet...should be here for the
   //two-phase-construcion??
}

class CFileZip* CFileZip::NewLC()
{
   //Get a fileserver that is allready open.
   class RFs& server = CEikonEnv::Static()->FsSession(); 
   class CFileZip* self = new (ELeave) CFileZip(server);
   CleanupStack::PushL(self);

   //At this point this call doesnt do anything. Its just a part of
   //the two-phase-construction
   self->ConstructL();
   return self;
}

class CFileZip* CFileZip::NewL()
{
   class CFileZip* self = CFileZip::NewLC();
   CleanupStack::Pop(self);
   return self;
}

void CFileZip::DeleteZipFileL(const TDesC& aFileName, const TDesC& aDrive)
{
   ASSERT_ALWAYS(TParsePtrC(aFileName).DrivePresent(), ENoDriveInZipFileName);
   class RFs& server = CEikonEnv::Static()->FsSession();
   HBufC* filename = HBufC::NewLC(256);   
   TPtr desPtr = filename->Des();
   desPtr.Copy(aFileName);
   _LIT(KColon, ":");
   _LIT(KBackslash, "\\");
   WFTextUtil::SearchAndReplace(desPtr, '/', '\\');
   if(! TParsePtrC(aFileName).DrivePresent()) {
      //No device name exists.
      TFileName* fn = new (ELeave) TFileName(256);
      CleanupStack::PushL(fn);
      _LIT(KFormatParams, "%c%S%S%S");
      fn->Format(KFormatParams, aDrive[0], &KColon, &KBackslash, filename);
      server.Delete(*fn);
      CleanupStack::PopAndDestroy(fn);
   } else {
      server.Delete(*filename);
   }
   CleanupStack::PopAndDestroy(filename);
}

void CFileZip::UnzipFileL(const TDesC& aFileName, 
                          const TDesC& aDrive,
                          class CDeInstallFileList* aDeInstFileList)
{
   ASSERT_ALWAYS(TParsePtrC(aFileName).DrivePresent(), ENoDriveInZipFileName);
   _LIT(KFileType, ".zip");
   if(aFileName.FindC(KFileType) != KErrNotFound) {
      class CZipFile* zipFile = 
         CZipFile::NewL(iFs, const_cast<TDesC&>(aFileName));
      CleanupStack::PushL(zipFile);
      User::LeaveIfError(zipFile->OpenL());
      class CZipFileMemberIterator* fileMembers = zipFile->GetMembersL();
      CleanupStack::PushL(fileMembers);
      class CZipFileMember* member;
      while ((member = fileMembers->NextL())) {
         CleanupStack::PushL(member);
         if (aDeInstFileList) {
            ExtractFileL(member, zipFile, aDrive, aDeInstFileList);
         } else {
            ExtractFileL(member, zipFile, aDrive);
         }
         CleanupStack::PopAndDestroy(member);
      }
      CleanupStack::PopAndDestroy(fileMembers);
      CleanupStack::PopAndDestroy(zipFile);
   }
}

void CFileZip::ExtractFileL(class CZipFileMember* aMember, 
                            class CZipFile* aZipFile, const TDesC& aDrive)
{
   HBufC* fileName = aMember->Name()->AllocLC();
   TPtr desPtr = fileName->Des();
   _LIT(KColon, ":");
   _LIT(KBackslash, "\\");

   WFTextUtil::SearchAndReplace(desPtr, '/', '\\');

   TFileName* fn = new (ELeave) TFileName(256);
   CleanupStack::PushL(fn);
   _LIT(KFormatParams, "%c%S%S%S"); 
   fn->Format(KFormatParams, aDrive[0], &KColon, &KBackslash, fileName);
   TInt err = iFs.MkDirAll(*fn);
   if (err != KErrNone && err != KErrAlreadyExists) {
      User::Leave(err);
   }
   if(!TParsePtrC(*fn).NamePresent()){
      //directory entry
   } else {
      CopyFileMemberL(*fn, *aZipFile, *aMember);
   }
   CleanupStack::PopAndDestroy(fn);
   CleanupStack::PopAndDestroy(fileName);
} 

void CFileZip::ExtractFileL(class CZipFileMember* aMember, 
                            class CZipFile* aZipFile, 
                            const TDesC& aDrive, 
                            class CDeInstallFileList* aDeInstFileList)
{
   HBufC* fileName = aMember->Name()->AllocLC();
   TPtr desPtr = fileName->Des();
   _LIT(KColon, ":");
   _LIT(KBackslash, "\\");

   WFTextUtil::SearchAndReplace(desPtr, '/', '\\');

   TFileName* fn = new (ELeave) TFileName(256);
   CleanupStack::PushL(fn);
   _LIT(KFormatParams, "%c%S%S%S"); 
   fn->Format(KFormatParams, aDrive[0], &KColon, &KBackslash, fileName);
   TInt err = iFs.MkDirAll(*fn);
   if (err != KErrNone && err != KErrAlreadyExists) {
      User::Leave(err);
   }
   if(!TParsePtrC(*fn).NamePresent()){
      //directory entry
   } else {
      CopyFileMemberL(*fn, *aZipFile, *aMember);
      aDeInstFileList->AddFileL(*fn);
   }
   CleanupStack::PopAndDestroy(fn);
   CleanupStack::PopAndDestroy(fileName);
} 

void CFileZip::CopyFileMemberL(const TDesC& aTargetFile,
                               class CZipFile& aZipFile,
                               class CZipFileMember& aMember)
{
   class RFile expandedMember;
   User::LeaveIfError(expandedMember.Replace(iFs, aTargetFile, 
                                             EFileShareAny|EFileWrite));
   CleanupClosePushL(expandedMember);
   class RZipFileMemberReaderStream* fileStream;
   
   // KCompressionMethodNotSupported is possible in decompressing file here
   User::LeaveIfError(aZipFile.GetInputStreamL(&aMember, fileStream));
   CleanupStack::PushL(fileStream);

   // Assume file contents are 8-bit data
   const TUint32 size = aMember.UncompressedSize();
   HBufC8* bytes = HBufC8::New(size);
   ASSERT_ALWAYS(bytes != NULL, ECopyNoMem);
   CleanupStack::PushL(bytes);
   TPtr8 ptr = bytes->Des(); //Obtain a modifiable descriptor
   TInt res;
   res  = fileStream->Read(ptr, size); //Later versions of Symbian use ReadL() instead here
  
   // save the unzipped contents to file
   User::LeaveIfError(expandedMember.Write(ptr));
   CleanupStack::PopAndDestroy(bytes);
   CleanupStack::PopAndDestroy(fileStream);
   CleanupStack::PopAndDestroy(&expandedMember);
}
#endif
