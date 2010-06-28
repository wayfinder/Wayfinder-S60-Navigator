/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DEINSTALLFILELIST_H
#define DEINSTALLFILELIST_H

#include <e32base.h>
#include <badesca.h>

_LIT(KUnInstallFile, "uninstall.txt");

class CDeInstallFileList : public CBase
{
private:
   CDeInstallFileList(RFs& aFsSession);
   void ConstructL();
   
public:
   ~CDeInstallFileList();
   static class CDeInstallFileList* NewL();
   static class CDeInstallFileList* NewLC();
   
   //TPtrC GetFileList() const;
   TPtrC GetFile(TInt aIndex) const;
   TInt AddFileL(const TDesC& aFile);
   void ClearFile(TInt aIndex);
   void ClearFileList();
   TInt WriteFileListToDisc(const TDesC& aPath);
   TInt WriteFileListToDisc(const TDesC& aDrive, const TDesC& aPath);
   TInt DeleteFileListFromDisc(const TDesC& aDrive, const TDesC& aPath);

private:
   RFs& iFsSession;

   /** Array containing all files to deinstall. */
   CDesCArray* iFileList;

};

#endif /* DEINSTALLFILELIST_H */
