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

#ifndef FILEWRITER_H
#define FILEWRITER_H
#include <e32base.h>
#include <f32file.h>

///Interface for the CFileWriter observer. 
class MFileWriterObserver
{
public:
   ///Called when the file write has completed or failed. 
   ///@param aWriter pointer to the reporting CFileWriter instance. 
   ///@param aErrCode KErrNone if no problems occured, otherwise one 
   ///       of the system wide error codes.
   virtual void FileWriteComplete(class CFileWriter* aWriter, 
                                  TInt aErrCode) = 0;
};

///This class is a one-shot object that replaces a file with new
///content. If the file doesn't already exist it is created. The file
///is treated as binary.
class CFileWriter : public CActive
{
   class MFileWriterObserver* iObserver;
   ///File server session object. The CFileWriter uses a already
   ///existing, already open file server session.
   class RFs& iRFs;
   ///The data to write. 
   HBufC8* iData;
   ///Flags. For future expansion.
   TInt iFlags;
   ///The file object. For internal use.
   class RFile iFile;
   ///This object stores the name and path of the file to replace. 
   class TParse iFileParse;
   ///For internal use. Name of a temp file where the data is
   ///originally written.
   TFileName* iTempFile;
   ///Constructor. 
   ///@param aRFs an already Connected RFs File Server Session.
   ///@param aObserver the obeserver of this write.
   ///@param aFlags for future expansion.
   CFileWriter(class RFs& aRFs, class MFileWriterObserver* aObserver, 
               TInt aFlags);
   ///Second phase constructor.
   ///@param aFileName  the full path of the file to create. 
   ///@param aData the binary data to write.
   void ConstructL(const TDesC& aFileName, const TDesC8& aData);
public:
   ///Static constructor.
   ///@param aRFs an already Connected RFs File Server Session.
   ///@param aObserver the obeserver of this write.
   ///@param aFileName  the full path of the file to create. 
   ///@param aData the binary data to write.
   ///@param aFlags for future expansion.
   ///@return a new CFileWriter object, that is pushed to the cleanupstack. 
   static class CFileWriter* NewLC(class RFs& aRFs, 
                                   class MFileWriterObserver* aObserver, 
                                   const TDesC& aFilename, 
                                   const TDesC8& aData, TInt aFlags = 0);
   ///Static constructor.
   ///@param aRFs an already Connected RFs File Server Session.
   ///@param aObserver the obeserver of this write.
   ///@param aFileName  the full path of the file to create. 
   ///@param aData the binary data to write.
   ///@param aFlags for future expansion.
   ///@return a new CFileWriter object. 
   static class CFileWriter* NewL(class RFs& aRFs, 
                                  class MFileWriterObserver* aObserver, 
                                  const TDesC& aFilename, 
                                  const TDesC8& aData, TInt aFlags = 0);

   ///Static constructor that calls StartWrite on the new object.
   ///@param aRFs an already Connected RFs File Server Session.
   ///@param aObserver the obeserver of this write.
   ///@param aFileName  the full path of the file to create. 
   ///@param aData the binary data to write.
   ///@param aFlags for future expansion.
   ///@return a new CFileWriter object. 
   static class CFileWriter* NewStartL(class RFs& aRFs, 
                                       class MFileWriterObserver* aObserver, 
                                       const TDesC& aFilename, 
                                       const TDesC8& aData, TInt aFlags = 0);
   ///Virtual destructor.
   virtual ~CFileWriter();
   ///Start the write.
   ///@return KErrNone if no error occured. Otherwise one of the
   ///        system wide error codes returned by RFile::Temp.
   TInt StartWrite();
   ///Called when the write is complete.
   void RunL();
   ///Called when the asynchronous request has been canceled.
   virtual void DoCancel();
};

#endif
