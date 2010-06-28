/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "MapFileFinder.h"
#include "TileMapControl.h"
#include "MapLib.h"
#include "WFTextUtil.h"
#include "TDesCHolder.h"

#include <eikenv.h>

_LIT8(KControlText, "storkafinger");

template<class T>
void DeleteAndNull(T*& aPtr)
{
   delete aPtr;
   aPtr = NULL;
}

CMapFileFinderBase::CMapFileFinderBase(class RFs& aFs, 
                                       class MapLib* aMapLib,
                                       enum TPriority aPriority) :
   CActive(aPriority), iFs(aFs), iMapLib(aMapLib)  
{
}

CMapFileFinderBase::~CMapFileFinderBase()
{
   DeleteAndNullPathComponents();
   if (iFileList) {
      iFileList->Reset();
      delete iFileList;
   }
   if (iDirList) {
      iDirList->Reset();
      delete iDirList;
   }
   delete iFinder;
   iFinder = NULL;
   DeleteAndNull(iFinder);
   DeleteAndNull(iFileMan);
   Cancel();
}

void CMapFileFinderBase::ConstructL()
{
   CActiveScheduler::Add(this);
   iFileMan         = CFileMan::NewL(iFs, this);
   iCancel          = EFalse;
   iFinder          = new (ELeave) TFindFile(iFs);
   SetRunning(EFalse);
}

void CMapFileFinderBase::DeleteAndNullPathComponents()
{
   DeleteAndNull(iTargetPath);
   DeleteAndNull(iFileName);
   DeleteAndNull(iFileExt);
   DeleteAndNull(iNameAndExt);
}

void CMapFileFinderBase::FindAndMoveFilesL(const MDesCArray& aDirList,
                                           const TDesC& aTargetPath,
                                           const TDesC& aFileName,
                                           const TDesC& aFileExt)
{
   if (isRunning) {
      // Already running, requester needs to wait until
      // we are finished.
      iDIState = ERequestDenied;
      CompleteRequest(KErrNone);
   } else {
      // DeleteAndNull on all pointer member.
      // Initialize members.
      iFirstFile = ETrue;
      SetRunning(ETrue);
      iNbrFoundFiles = 0;
      DeleteAndNullPathComponents();
      InitializeDirListL(&aDirList);
      if (iFileList) {
         iFileList->Reset();
         DeleteAndNull(iFileList);
      }
      iTargetPath = aTargetPath.AllocL();
      iFileName   = aFileName.AllocL();
      iFileExt    = aFileExt.AllocL();
      _LIT(KDot, ".");
      iNameAndExt = HBufC::NewL(aFileName.Size() + aFileExt.Size() + 1);
      iNameAndExt->Des().Copy(aFileName);
      iNameAndExt->Des().Append(KDot);
      iNameAndExt->Des().Append(aFileExt);
      if (!iFinder) {
         iFinder  = new (ELeave) TFindFile(iFs);
      }
      iDIState = EFindFiles;
      CompleteRequest(KErrNone);
   }
}

void CMapFileFinderBase::DeleteFileL(const class MapLib::CacheInfo* info)
{
   if (isRunning) {
      // Already running, requester needs to wait until
      // we are finished.
      iDIState = ERequestDenied;
      CompleteRequest(KErrNone);      
   } else {
      SetRunning(ETrue);
      DeleteAndNullPathComponents();
      HBufC* path = WFTextUtil::Utf8AllocLC(info->getPathUTF8());
      if (!iMapLib->disconnectCache(info)) {
         // Cache is now disconnected and ready to be
         // deleted.
#ifdef NAV2_CLIENT_UIQ
         if (KErrNone == (iState = iFs.Delete(*path))) {
            iDIState = EFileDeleted;
            CompleteRequest(KErrNone);
         }
#else
         if (KErrNone == (iState = iFileMan->Delete(*path, 
                                                    CFileMan::ERecurse, 
                                                    iStatus))) {
            iDIState = EFileDeleted;
            SetActive();
         }
#endif
         else {
            iDIState = EDeleteFileFailed;
            CompleteRequest(KErrNone);
         }
      } else {
         // Cache could not be disconnected, we cant
         // delete the file.
         iDIState = EDeleteFileFailed;
         iState = KErrInUse;
      }
      CleanupStack::PopAndDestroy(path);
   }
}

void CMapFileFinderBase::CopyFile(const TDesC& aNewNameAndPath,
                                  const TDesC& aOldNameAndPath) 
{
   if (isRunning) {
      // Already running, requester needs to wait until
      // we are finished.
      iDIState = ERequestDenied;
      CompleteRequest(KErrNone);      
   } else {
      SetRunning(ETrue);
      DeleteAndNullPathComponents();
      if (KErrNone == (iState = iFileMan->Copy(aNewNameAndPath, 
                                               aOldNameAndPath,
                                               CFileMan::EOverWrite,
                                               iStatus))) {
         iDIState = EFileCopied;
         SetActive();
      }
      else {
         iDIState = ECopyFileFailed;
         CompleteRequest(KErrNone);
      }
   } 
}

TInt CMapFileFinderBase::InitializeDirListL(const MDesCArray* aDirList)
{
   // Initialize iDirList, copy all elements from
   // aDirList to iDirList.
   if (iDirList) {
      iDirList->Reset();
      DeleteAndNull(iDirList);
   } 
   iDirList = new (ELeave) CDesCArrayFlat(aDirList->MdcaCount());
   for (TInt i = 0; i < aDirList->MdcaCount(); i++) {
      iDirList->AppendL(aDirList->MdcaPoint(i));
   }
   return KErrNone;
}

void CMapFileFinderBase::CancelOperation()
{
   iDIState = ECancelRequest;
}


void CMapFileFinderBase::RunL()
{
   switch (iStatus.Int()) {
   case KErrNone:
      switch (iDIState) {
      case EFindFiles:
         if (!iFirstFile && iDirList && iDirList->Count() > 0) {
            // We have done at least one search, we can
            // remove item 0 in iDirList and continue 
            // searching in next dir.
           iDirList->Delete(0);
         }
         InternalFindFilesL();
         break;
      case EMoveFiles:
         // Report to the observer how many files that were found.
         ReportNbrMapFilesFound(iFileList ? iFileList->MdcaCount() : 0);
         // Search has been done in at least one 
         // directory, move the files that were 
         // found, one at a time.
         InternalMoveFiles();
         break;
      case EAddCacheFile:
         AddCacheFile();
         break;
      case EFileCacheAdded: 
         if (iFileList && iFileList->Count() > 0) {
            // Delete the recently moved file from the 
            // file list. Continue moving the next file.
            iFileList->Delete(0);
         }
         InternalMoveFiles();
         break;
      case EFileManMoveError:
         if (iFileList && iFileList->Count() > 0) {
            // Delete the file that couldnt be moved
            // and continue with the next one.
            iFileList->Delete(0);
         }
         InternalMoveFiles();
         break;
      case EFindAndMoveFilesRequestCompleted:
         SetRunning(EFalse);
         FindAndMoveFilesRequestCompleted();
         break;
      case ERequestDenied:
         SignalMapFileFinderBusy();
         break;
      case ECancelRequest:
         RequestAborted();
         break;
      case EDeleteFileFailed:
      case EFileDeleted:
         DeleteFileRequestCompleted(iState);
         break;
      case ECopyFileFailed:
      case EFileCopied:
         CopyFileRequestCompleted(iState);
         break;
      }
      break;
   case KErrPathNotFound:
   case KErrInUse:
      // Either a path is wrong or a file is in 
      // use. We should try with the next one, or
      // should we inform the user that an error
      // has occured?
      iDIState = EFileCacheAdded;
      CompleteRequest(KErrNone);
      break;
   default:
      RequestAborted();
      break;
   }
}
            
void CMapFileFinderBase::DoCancel()
{
   if(IsActive()) {
      class TRequestStatus* p = &iStatus;
      User::RequestComplete(p, KErrCancel);
   }
}

void CMapFileFinderBase::InternalFindFilesL()
{
   if (!iFileList) {
      iFileList = new (ELeave) CDesCArrayFlat(5);
   }
   if (iDirList && iDirList->Count() > 0) {
      TPtrC path = iDirList->MdcaPoint(0);
      CDir* dir;
      TInt err = iFinder->FindWildByDir(*iNameAndExt, path, dir);
      if (path != KNullDesC && err == KErrNone) {
         while (err == KErrNone) {
            // We have found at least one match in path.
            class TParse parser;
            for(TInt i=0; i<dir->Count(); i++) {
               // Iterate through dir and check all matching files.
               parser.Set(dir->operator[](i).iName, 
                          &iFinder->File(), NULL);
               TPtrC path = parser.FullName();
               if(CheckFile(path)) {
                  // The file is a real map file, add it to
                  // our list.
                  iFileList->AppendL(path);
               }     
            }
            DeleteAndNull(dir);
            err = iFinder->FindWild(dir);
         }
      }
      if (iFirstFile) {
         iFirstFile = EFalse;
      }
      iDIState = EMoveFiles;
   } else {
      // No more directories to search in, we are
      // finished.
      iDIState = EFindAndMoveFilesRequestCompleted;
   }
   CompleteRequest(KErrNone);
} 

void CMapFileFinderBase::InternalMoveFiles()
{
   // XXX Create a correct path if it does not exist.
   // XXX Should we remove already existing files?
   if (iFileList && iFileList->Count() > 0) {
      class TParsePtrC parsePtrC(iFileList->MdcaPoint(0));
      class TParse parse;
      TPtrC test = parsePtrC.Drive();
      parse.Set(*iTargetPath, &test, NULL);
      TInt ret = iFs.MkDirAll(parse.FullName()); 
      if (!(ret == KErrNone || ret == KErrAlreadyExists)) {
         CompleteRequest(KErrPathNotFound);
         return;
      }
//       if (iFirstFile) {
//          iFirstFile = EFalse;
//       }
      // CAP: dependent
      if(iFileMan->Move(iFileList->MdcaPoint(0), 
                        parse.FullName(), 
                        CFileMan::EOverWrite, iStatus) ==  KErrNone) {
         SetActive();
         iDIState = EAddCacheFile;
         return;
      } else {
         // iFileMan->Move returned an error, continue with the
         // next file.
         iDIState = EFileManMoveError;
         CompleteRequest(KErrNone);
      }
   } else {
      // We can not be sure that search is done in all dirs.
      // Let state machine decide what to do.
      iDIState = EFindFiles;
      CompleteRequest(KErrNone);
   }
}

void CMapFileFinderBase::AddCacheFile()
{
   TParsePtrC parsePtr(iFileList->MdcaPoint(0));
   HBufC* newNamePath = AllocLC(parsePtr.Drive() + 
                                *iTargetPath + 
                                parsePtr.NameAndExt());
   char* newNamePathUtf8 = WFTextUtil::TDesCToUtf8LC(*newNamePath);
   if (iMapLib->addSingleFileCache(newNamePathUtf8, NULL) != 0) {
      // Could not add the file.
      // XXX Do somethin??
   } else {
      iNbrFoundFiles++;
   }
   CleanupStack::PopAndDestroy(newNamePathUtf8);
   CleanupStack::PopAndDestroy(newNamePath);
   iDIState = EFileCacheAdded;
   CompleteRequest(KErrNone);
}

TBool CMapFileFinderBase::CheckFile(const TDesC& aFileName)
{
   class RFile mapFile;
   if (KErrNone == mapFile.Open(iFs, aFileName, EFileRead)) {
      TBuf8<12> controlText;
      if (mapFile.Read(controlText) == KErrNone && 
          controlText.CompareF(KControlText()) == 0) {
         // First 12 chars in the file is "storkafinger"
         // we now know it is our map file.
         mapFile.Close();
         return ETrue;
      }
      mapFile.Close();
      return EFalse;
   }
   return EFalse;
}

void CMapFileFinderBase::SetRunning(TBool aIsRunning)
{
   isRunning = aIsRunning;
}

void CMapFileFinderBase::CompleteRequest(TInt aStatus)
{
   class TRequestStatus* status = &iStatus; 
   iStatus = KRequestPending;
   SetActive();
   User::RequestComplete(status, aStatus);
}

enum MFileManObserver::TControl CMapFileFinderBase::NotifyFileManStarted()
{
   if(iDIState == ECancelRequest) {
      return EAbort;
   }
   return EContinue;
}

enum MFileManObserver::TControl CMapFileFinderBase::NotifyFileManOperation()
{
   if(iDIState == ECancelRequest) {
      return EAbort;
   }
   return EContinue;
}

enum MFileManObserver::TControl CMapFileFinderBase::NotifyFileManEnded()
{
   enum TControl ret;
   switch(iDIState) {
   case EFindFiles:
   case EMoveFiles:
      ret = EContinue;
      break;
   case ECancelRequest:
   case EFindAndMoveFilesRequestCompleted:
      ret = EAbort;
      break;
   default:
      ret = EContinue;
      break;
   }
   return ret;
}
