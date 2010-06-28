/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "MD5Symbian.h"
#include <f32file.h>
#include "WFTextUtil.h"

TInt TMD5Checker::CheckFile(class RFs& aFs, const TDesC& aFile, const TDesC16& aHash)
{
   TInt ret = KErrNone;
   TRAPD(err, ret = CheckFileL(aFs, aFile, aHash));
   return err == KErrNone ? ret : err;
}

TInt TMD5Checker::CheckFile(class RFs& aFs, const TDesC& aFile, const TDesC8& aHash)
{
   TInt ret = KErrNone;
   TRAPD(err, ret = CheckFileL(aFs, aFile, aHash));
   return err == KErrNone ? ret : err;
}

TInt TMD5Checker::CheckFile(class RFs& aFs, const TDesC& aFile, const char* aHash)
{
   TPtrC8 md5hash(reinterpret_cast<const TUint8*>(aHash));
   md5hash.Set(reinterpret_cast<const TUint8*>(aHash), strlen(aHash));
   return CheckFile(aFs, aFile, md5hash);
}

TInt TMD5Checker::CheckFile(class RFs& aFs, const char* aFile, 
                            const char* aHash)
{
   TInt ret = KErrNoMemory;
   HBufC* filename = WFTextUtil::Alloc(aFile);
   if(filename){
      ret = CheckFile(aFs, *filename, aHash);
      delete filename;
   }
   return ret;
}

void TMD5Checker::Append(const TDesC8& aData)
{
   MD5::Append(aData.Ptr(), aData.Length());
}

TInt TMD5Checker::ReadAndHashFileL(class RFs& aFs, const TDesC& aFile)
{
   TInt ret = KErrNone;

   class RFile file;
   // CAP: dependent
   User::LeaveIfError(file.Open(aFs, aFile, EFileRead));
   CleanupClosePushL(file);

   HBufC8* readBuf = HBufC8::NewLC(6*1024);
   TPtr8 buf = readBuf->Des();

   Reset();

   User::LeaveIfError(file.Read(buf));
   while(buf.Length() > 0){
      Append(buf);
      User::LeaveIfError(file.Read(buf));
   }

   Finish();
      
   CleanupStack::PopAndDestroy(readBuf);
   CleanupStack::PopAndDestroy(/*file*/);
   return ret;
}

TInt TMD5Checker::CheckFileL(class RFs& aFs, const TDesC& aFile, const TDesC16& aHash)
{
   TInt ret = ReadAndHashFileL(aFs, aFile);

   _LIT16(KMd5Prefix, "md5:");
   TInt offset = 0;
   if(aHash.Left(KMd5Prefix().Length()).CompareF(KMd5Prefix) == 0){
      offset = KMd5Prefix().Length();
   }

   for(TUint i = 0; i < sizeof(m_hex_md5_checksum) && ret == KErrNone; ++i){
      ret = (aHash[i+offset] == m_hex_md5_checksum[i]) ? KErrNone :KMd5NoMatch;
   }
   return ret;
}


TInt TMD5Checker::CheckFileL(class RFs& aFs, const TDesC& aFile, const TDesC8& aHash)
{
   TInt ret = ReadAndHashFileL(aFs, aFile);

   _LIT8(KMd5Prefix, "md5:");
   TInt offset = 0;
   if(aHash.Left(KMd5Prefix().Length()).CompareF(KMd5Prefix) == 0){
      offset = KMd5Prefix().Length();
   }

   const TUint numChars = sizeof(m_hex_md5_checksum) - 1;

   for(TUint i = 0; i < numChars && ret == KErrNone; ++i){
      ret = (aHash[i+offset] == m_hex_md5_checksum[i]) ? KErrNone :KMd5NoMatch;
   }
   return ret;
}
