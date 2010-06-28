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

#include "FileWriter.h"
CFileWriter::CFileWriter(class RFs& aRFs, 
                         class MFileWriterObserver* aObserver, TInt aFlags) :
   CActive(EPriorityStandard), iObserver(aObserver), iRFs(aRFs), iFlags(aFlags)
{
}

void CFileWriter::ConstructL(const TDesC& aFileName, const TDesC8& aData){
   iData = aData.AllocL();
   iTempFile = new (ELeave) TFileName();
   User::LeaveIfError(iFileParse.Set(aFileName, NULL, NULL));
   CActiveScheduler::Add(this);
}

class CFileWriter* CFileWriter::NewLC(class RFs& aRFs, 
                                      class MFileWriterObserver* aObserver, 
                                      const TDesC& aFilename, 
                                      const TDesC8& aData, TInt aFlags)
{
   class CFileWriter* self = new (ELeave) CFileWriter(aRFs, aObserver, aFlags);
   CleanupStack::PushL(self);
   self->ConstructL(aFilename, aData);
   return self;
}

class CFileWriter* CFileWriter::NewL(class RFs& aRFs, 
                                     class MFileWriterObserver* aObserver, 
                                     const TDesC& aFilename, 
                                     const TDesC8& aData, TInt aFlags)
{
   class CFileWriter* self = CFileWriter::NewLC(aRFs, aObserver, aFilename, 
                                                aData, aFlags);
   CleanupStack::Pop(self);
   return self;
}

class CFileWriter* CFileWriter::NewStartL(class RFs& aRFs, 
                                          class MFileWriterObserver* aObserver,
                                          const TDesC& aFilename, 
                                          const TDesC8& aData, 
                                          TInt aFlags)
{
   class CFileWriter* self = NewLC(aRFs, aObserver, aFilename, aData, aFlags);
   User::LeaveIfError(self->StartWrite());
   CleanupStack::Pop(self);
   return self;
}


CFileWriter::~CFileWriter()
{
   delete iData;
   delete iTempFile;
}

TInt CFileWriter::StartWrite()
{
   TInt fileFlags = EFileStream | EFileWrite | EFileShareExclusive;
   TInt res = iFile.Temp(iRFs, iFileParse.DriveAndPath(), *iTempFile, 
                         fileFlags);
   if(res == KErrNone){
      iFile.Write(*iData, iStatus);
      SetActive();
   }
   return res;
}

void CFileWriter::RunL()
{
   TInt res = iStatus.Int();
   if(iStatus == KErrNone){
      res = iFile.Flush();
      iFile.Close();
      if(res == KErrNone){
         res = iRFs.Replace(*iTempFile, iFileParse.FullName());
      }
   }
   if(iObserver){
      iObserver->FileWriteComplete(this, res);
   }
}

void CFileWriter::DoCancel()
{
   if(iObserver){
      iObserver->FileWriteComplete(this, KErrCancel);
   }   
}
