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

/* The following code finds compressed zip files in a folder c:\test\zip 
 * and unzips their contents into a c:\test\zip\extracts. Each zip file 
 * is unzipped into a subfolder named after the file itself. Note 
 * that zip-related errors and other enums are defined in ziparchive.h.
 */

#ifndef _FILEZIP_H_
#define _FILEZIP_H_

#include <e32base.h>
#include <e32std.h>

class CFileZip : public CBase  
{
public:
   static CFileZip* NewLC();
   static CFileZip* NewL();
   void DeleteZipFileL(const TDesC& aFileName, const TDesC& aDrive);
   void UnzipFileL(const TDesC& aFileName, 
                   const TDesC& aDrive,
                   class CDeInstallFileList* aDeInstFileList = NULL);
   void ExtractFileL(class CZipFileMember* aMember, 
                     class CZipFile* aZipFile, 
                     const TDesC& aDrive);
   void ExtractFileL(class CZipFileMember* aMember, 
                     class CZipFile* aZipFile, 
                     const TDesC& aDrive, 
                     class CDeInstallFileList* aDeInstFileList);
   ~CFileZip() {};

private:
   CFileZip::CFileZip(class RFs& aFs) : iFs(aFs) {}
   void ConstructL();
   void CopyFileMemberL(const TDesC& aTargetFile,
                        class CZipFile& aZipFile,
                        class CZipFileMember& aMember);

private:
   class RFs& iFs;
}; // Class CFileZip

#endif
