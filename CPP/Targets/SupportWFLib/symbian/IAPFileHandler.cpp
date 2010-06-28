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

#include "IAPFileHandler.h"
#include "IAPObject.h"

#include <coemain.h>
#include <f32file.h>


int32
IAPFileHandler::CheckIAPFile(const TDesC& aFilename,
         const TDesC& aImei)
{
   TInt ret = -1;
   class CIAPObject* iap = CIAPObject::NewLC();
   if(IAPFileHandler::ReadIAPFile(aFilename, iap) != KErrEof){
      /* IAP was saved, set it instead of -1! */
      if (!iap->GetImeiNbr().Compare(aImei)) {
         /* Same phone, use the IAP in the file. */
         TLex lexer (iap->GetIAPNbr());
         TInt32 iapid = -1;
         if (lexer.Val(iapid) == KErrNone) {
            ret = iapid;
         } else {
            /* Overflow or other error. Fudge it. */
         }
      }
   }
   CleanupStack::PopAndDestroy(iap);
   return ret;
}

int32
IAPFileHandler::ReadIAPFile(const TDesC& aFilename, 
                                 class CIAPObject* aIAPObject)
{
   class RFs& fs = CCoeEnv::Static()->FsSession();
   class RFile iapFile;
   TInt ret = iapFile.Open(fs, aFilename, EFileStreamText | EFileRead);
   if(ret == KErrNone) {

      class TFileText textFile;
      textFile.Set(iapFile);
      textFile.Seek(ESeekStart);
      
      HBufC* buffer = HBufC::New(256);
      if(buffer){
         typedef void (CIAPObject::*TSetFunc)(const TDesC&);
         TSetFunc functions[] = {&CIAPObject::SetImeiNbr, 
                                 &CIAPObject::SetIAPNbr, 
                                 &CIAPObject::SetIAPName};
         TPtr ptr = buffer->Des();
         for(TUint i = 0; i < (sizeof(functions)/sizeof(*functions)); ++i){
            if(KErrNone != (ret = textFile.Read(ptr))){
               break;
            }
            TSetFunc func = functions[i];
            ((aIAPObject)->*(func))(ptr);
         }
         delete buffer;
      } else {
         ret = KErrNoMemory;
      }
      iapFile.Close();
   }
   return ret;
}

int32
IAPFileHandler::WriteIAPFile(const TDesC& aFilename, 
                                  const class CIAPObject& aIAPObject)
{
   class RFs& fs = CCoeEnv::Static()->FsSession();
   class RFile iapFile;
   // If file already exists, replace it with an empty one, 
   // if not replace creates a new file.
   TInt ret = iapFile.Replace(fs, aFilename, EFileStreamText | EFileWrite);
   if(ret == KErrNone) {
      class TFileText textFile;
      textFile.Set(iapFile);
      textFile.Seek(ESeekStart);
      
      textFile.Write(aIAPObject.GetImeiNbr());
      textFile.Write(aIAPObject.GetIAPNbr());
      textFile.Write(aIAPObject.GetIAPName());
      
      iapFile.Close();
   }
   return ret;
}

int32
IAPFileHandler::WriteIAPFile()
{
   return 0;
}

int32
IAPFileHandler::DeleteIAPFile(const TDesC& aFilename)
{
   class RFs& fs = CCoeEnv::Static()->FsSession();
   TInt ret = fs.Delete(aFilename);
   if(ret == KErrNotFound){
      ret = KErrNone;
   }
   return ret;
}
