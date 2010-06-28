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

#include "VersionFileReader.h"

#include <e32std.h>
#include <f32file.h>
#include <coemain.h>
#include <stdio.h>
#include <badesca.h>
#include "WFTextUtil.h"

CVersionFileReader::CVersionFileReader()
{
}

CVersionFileReader::~CVersionFileReader()
{
   if(iOldVersionArray) {
      iOldVersionArray->Reset();
      delete iOldVersionArray;
   }
   if(iNewVersionArray) {
      iNewVersionArray->Reset();
      delete iNewVersionArray;
   }
}

class CVersionFileReader* CVersionFileReader::NewLC()
{
   class CVersionFileReader* self = new (ELeave) CVersionFileReader();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CVersionFileReader* CVersionFileReader::NewL()
{
   class CVersionFileReader* self = CVersionFileReader::NewLC();
   CleanupStack::Pop(self);
   return self;
}

void CVersionFileReader::ConstructL()
{
   iNewVersionArray = new (ELeave) CDesC8ArrayFlat(ENewVersionArraySize);
   iOldVersionArray = new (ELeave) CDesC8ArrayFlat(3);
}

TInt CVersionFileReader::ReadNewVersionFileL(const TDesC& versionFile)
{
   iNewVersionArray->Reset();
   class RFs& fs = CCoeEnv::Static()->FsSession();
   class RFile file;
   TInt ret = file.Open(fs, versionFile, 
                        EFileStreamText | EFileRead);
   if(ret != KErrNone) {
      /* Could not open file, return the error code */
      return ret;
   }
   
   HBufC8* line = HBufC8::NewLC(40);
   TInt pos = -1;
   file.Seek(ESeekStart, pos);
   TPtr8 pLine = line->Des();
   ret = file.Read(pLine);
   if(line->Length() == 0) {
      // Empty file
      file.Close();
      return KErrEof;
   }
   file.Close();

   // The file contains a string that should match this regex:
   // [0-9]+\.[0-9]+\.[0-9]\+:[0-9]+\.[0-9]+\.[0-9]\+:[0-9]\+
   // The string is separated into three parts by ':'. 
   // The first part is the application version
   // The second part is the resource version
   // The third part is the mlfw version.
   const TChar colon = ':';
   const TChar dot = '.';
   while(line->Length() > 0) {
      if(line->Locate(colon) != KErrNotFound) {
         TPtrC8 part = line->Left(line->Locate(colon));
         TPtrC8 version = part.Left(part.LocateReverse(dot));
         TPtrC8 misc = part.Right(part.Length() - part.LocateReverse(dot) - 1);
         iNewVersionArray->AppendL(part);
         iNewVersionArray->AppendL(version);
         iNewVersionArray->AppendL(misc);
         line->Des().CopyF(line->Right(line->Length() - line->Locate(colon) - 1));
      } else {
         iNewVersionArray->AppendL(*line);
         break;
      }
   }
   CleanupStack::PopAndDestroy(line);
   return 0;
}

TInt CVersionFileReader::ReadOldVersionFileL(const TDesC& aFilename)
{
   // The file contains a string matching this regex
   // [0-9]\+\.[0-9]\+_[0-9]\+
   iOldVersionArray->Reset();
   class RFs& fs = CCoeEnv::Static()->FsSession();
   class RFile versionFile;
   TInt ret = versionFile.Open(fs, aFilename, 
                               EFileStreamText | EFileRead);
   if(ret != KErrNone) {
      /* Could not open file, return the error code */
      return ret;
   }
   HBufC8* line = HBufC8::NewLC(30);
   TInt pos = -1;
   versionFile.Seek(ESeekStart, pos);
   TPtr8 pLine = line->Des();
   ret = versionFile.Read(pLine);
   iOldVersionArray->AppendL(*line);
   if(line->Length() == 0) {
      versionFile.Close();
      return KErrEof;
   }
   versionFile.Close();
   if(line->Locate('_') != KErrNotFound) {
      iOldVersionArray->AppendL(line->Left(line->Locate('_')));
      iOldVersionArray->AppendL(line->Right(line->Length() - line->Locate('_') - 1));
   } else {
      iOldVersionArray->AppendL(*line);
   }

   CleanupStack::PopAndDestroy(line);

   return ret;
}


TPtrC8 CVersionFileReader::GetNewVersion(enum TNewVersionType aVersionType)
{
   if(aVersionType < iNewVersionArray->Count()) {
      return iNewVersionArray->MdcaPoint(aVersionType);
   }
   return TPtrC8();
}

TPtrC8 CVersionFileReader::GetOldVersion(enum TOldVersionType aVersionType)
{
   if(aVersionType < iOldVersionArray->Count()) {
      return iOldVersionArray->MdcaPoint(aVersionType);
   }
   return TPtrC8();
}

namespace {
   inline TLex8& SkipToNextDigit(TLex8& aLex)
   {
      if(!aLex.Eos()){
         TChar gotten = aLex.Get();
         while(!(gotten.IsDigit() || gotten.Eos())){
            gotten = aLex.Get();
         }
         aLex.UnGet();
      }
      return aLex;
   }

   inline TLex8& SkipToNextDigitAndReadVal(TLex8& aLex, TUint aVal)
   {
      SkipToNextDigit(aLex).Val(aVal);
      return aLex;
   }

   class TVersion ExtractVersions(const TDesC8& aVersionString)
   {
      TUint major = 0, minor = 0, misc = 0;
      major = minor = misc = 0;
      TLex8 lex(aVersionString);
      SkipToNextDigitAndReadVal(lex, major);
      SkipToNextDigitAndReadVal(lex, minor);
      SkipToNextDigitAndReadVal(lex, misc);

//       if(KErrNone == lex.Val(major)){
//          SkipToNextDigit(lex);
//          if(!lex.Eos()){
//             if(KErrNone == lex.Val(minor)){
//                SkipToNextDigit();
//                if(!lex.Eos()){
//                   if(KErrNone == lex.Val(minor)){
//                      return TVersion(major, minor, misc);
//                   }
//                }
//             }
//          }
//       }
      return TVersion(major, minor, misc);
   }
}

class TVersion 
CVersionFileReader::NewVersion(enum TNewVersionType aVersionType)
{
   TPtrC8 verStr = GetNewVersion(aVersionType);
   if(verStr.Length() == 0){
      return TVersion();
   }
   return ExtractVersions(verStr);
}

class TVersion 
CVersionFileReader::OldVersion(enum TOldVersionType aVersionType)
{
   TPtrC8 verStr = GetOldVersion(aVersionType);
   if(verStr.Length() == 0){
      return TVersion();
   }
   return ExtractVersions(verStr);
}

