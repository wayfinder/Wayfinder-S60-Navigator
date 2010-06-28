/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MD5_SYMBIAN_H
#define MD5_SYMBIAN_H
#include "MD5Class.h"
#include <e32des8.h>
#include <e32des16.h>

#define KMd5NoMatch KErrCorrupt

class TMD5Checker : public isab::MD5 {
public:
   TInt CheckFile(class RFs& aFs, const TDesC& aFile, const TDesC16& aHash);
   TInt CheckFile(class RFs& aFs, const TDesC& aFile, const TDesC8& aHash);
   TInt CheckFile(class RFs& aFs, const TDesC& aFile, const char* aHash); 
   TInt CheckFile(class RFs& aFs, const char* aFile, const char* aHash);

   template<class F, class H>
   static TInt CheckFileStatic(class RFs& aFs, F aFile, H aHash)
   {
      TMD5Checker local;
      return local.CheckFile(aFs, aFile, aHash);
   }
   void Append(const TDesC8& aData); 
private:
   TInt ReadAndHashFileL(class RFs& aFs, const TDesC& aFile);
   TInt CheckFileL(class RFs& aFs, const TDesC& aFile, const TDesC16& aHash);
   TInt CheckFileL(class RFs& aFs, const TDesC& aFile, const TDesC8& aHash);
};
#endif
