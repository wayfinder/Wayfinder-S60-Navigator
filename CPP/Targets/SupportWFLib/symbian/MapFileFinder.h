/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DE_INSTALLER
#define DE_INSTALLER

#include <e32base.h>
#include <f32file.h>
#include <badesca.h>
#include <bamdesca.h>
#include <eikenv.h>
#include "MapLib.h"

class CMapFileFinderBase : public CActive, public MFileManObserver
{
protected:
   /** @name Constructors and destructor. */
   //@{
   CMapFileFinderBase(class RFs& aFs, 
                      class MapLib* aMapLib,
                      enum TPriority aPriority = EPriorityStandard);
   virtual ~CMapFileFinderBase();
   void ConstructL();
   //@}

   enum TSearchState {
      EFindFiles                        = 0,
      EMoveFiles                        = 1,
      EAddCacheFile                     = 2,
      EFileCacheAdded                   = 3,
      EFileManMoveError                 = 4,
      EFindAndMoveFilesRequestCompleted = 5,
      ERequestDenied                    = 6,
      ECancelRequest                    = 7,   
      EDeleteFileFailed                 = 8,
      EFileDeleted                      = 9,
      EFileCopied                       = 10,
      ECopyFileFailed                   = 11,
   };

public:
   
    /**
    * Finds all files that matches the specification
    * in all drives specified to search in and moves
    * them to the drive specified in aTargetPath.
    * Wildcards is allowed in aFileExt and aFileName, 
    * ? represents one character, * represents one or 
    * more charactes. The search is done case insensitive. 
    * @param aDirList    list of dirs to search for 
    *                    files in.
    * @param aTargetPath the dir to move all matching 
    *                    files to.
    * @param aFileName   name of the files to search for. 
    * @param aFileExt    the extension of the files to 
    *                    search for.
    */
   void FindAndMoveFilesL(const MDesCArray& aDirList,
                          const TDesC& aTargetPath,
                          const TDesC& aFileName,
                          const TDesC& aFileExt);

   /**
    * Disconnects a wfd map cache file and deletes
    * it from the phone.
    * @param aInfo Containing the path and filename,
                   need a CacheInfo to disconnect the
                   cache from the server before deleting
                   the file.
   */
   void DeleteFileL(const class MapLib::CacheInfo* aInfo);

   /**
    * Copies a file.
    * @param aNewNameAndPath Where the file should be
    *                        copied to.
    * @param aOldNameAndPath Current location of the file.
    */
   void CopyFile(const TDesC& aNewNameAndPath,
                 const TDesC& aOldNameAndPath); 

   void CancelOperation();

protected: // From CActive
   virtual void RunL();
   virtual void DoCancel();

protected: // From MFileManObserver
   enum TControl NotifyFileManStarted();
   enum TControl NotifyFileManOperation();
   enum TControl NotifyFileManEnded();

protected: // From CMapFileFinderBase
   virtual void SignalMapFileFinderBusy() = 0;
   virtual void ReportNbrMapFilesFound(TInt) = 0;
   virtual void FindAndMoveFilesRequestCompleted() = 0;
   virtual void DeleteFileRequestCompleted(TInt aResult) = 0;
   virtual void CopyFileRequestCompleted(TInt aResult) = 0;
   virtual void RequestAborted() = 0;

protected:
   void DeleteAndNullPathComponents();
   void InternalFindFilesL();
   void InternalMoveFiles();
   void CompleteRequest(TInt aStatus);
   void AddCacheFile();
   void SetRunning(TBool aIsRunning);
   TInt InitializeDirListL(const MDesCArray* aDirList);
   TBool CheckFile(const TDesC& aFileName);

protected:
   class RFs&            iFs;
   class MapLib*         iMapLib;
   class CFileMan*       iFileMan;
   class TFindFile*      iFinder;
   class TParse          iParser;
   CDesCArray*           iDirList;
   CDesCArray*           iFileList;
   enum TSearchState     iDIState;
   HBufC*                iTargetPath;
   HBufC*                iFileName;
   HBufC*                iFileExt;
   HBufC*                iNameAndExt; 
   TBool                 iCancel;
   TBool                 isRunning;
   TBool                 iFirstFile;
   TInt                  iNbrFoundFiles;
   TInt                  iState;
};

template <class MapFileFindRequester>
class CMapFileFinder : public CMapFileFinderBase
{

   CMapFileFinder(MapFileFindRequester* aRequester, 
                  class RFs& aFs,
                  class MapLib* aMapLib,
                  enum TPriority aPriority = EPriorityStandard) : 
      CMapFileFinderBase(aFs, aMapLib, aPriority), 
      iRequester(aRequester)
   {
   }

public:
   static CMapFileFinder<MapFileFindRequester>* NewL(MapFileFindRequester* aRequester,
                                                     class MapLib* aMapLib,
                                                     enum TPriority aPriority = 
                                                     EPriorityStandard)
   {
      class RFs& fs = CEikonEnv::Static()->FsSession();
      CMapFileFinder<MapFileFindRequester>* self = 
         new (ELeave) CMapFileFinder(aRequester, fs, aMapLib, aPriority);
      CleanupStack::PushL(self);
      self->ConstructL();
      CleanupStack::Pop(self);
      return self;
   }

   virtual void SignalMapFileFinderBusy()
   {
      iRequester->MapFileFinderBusy();
   }

   virtual void ReportNbrMapFilesFound(TInt aCount) 
   {
      iRequester->NbrMapFilesFound(aCount);
   }

   virtual void FindAndMoveFilesRequestCompleted()
   {
      DeleteAndNullPathComponents();
      iRequester->FindAndMoveFilesCompleted(iNbrFoundFiles);
      SetRunning(EFalse);
   }

   virtual void DeleteFileRequestCompleted(TInt aResult)
   {
      iRequester->DeleteFileCompleted(aResult);
      SetRunning(EFalse);
   }

   virtual void CopyFileRequestCompleted(TInt aResult)
   {
      iRequester->CopyFileCompleted(aResult);
      SetRunning(EFalse);
   }

   virtual void RequestAborted()
   {
      DeleteAndNullPathComponents();
      iRequester->RequestAborted();
      SetRunning(EFalse);         
   }

private:

private:
   MapFileFindRequester*  iRequester;
};

#endif
