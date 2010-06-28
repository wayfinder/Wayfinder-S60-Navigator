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

#include "DeInstaller.h"

_LIT(KAppName, "Wayfinder");

CDeInstallerBase::CDeInstallerBase(class RFs& aFs, 
                                   enum TPriority aPriority) :
   CActive(aPriority), iFs(aFs)
{
}

CDeInstallerBase::~CDeInstallerBase()
{
   DeleteAndNullPathComponents();
   if(iFileList) {
      iFileList->Reset();
      delete iFileList;
   }
   if(iPathList) {
      iPathList->Reset();
      delete iPathList;
   }
   Cancel();
}

void CDeInstallerBase::ConstructL()
{
   CActiveScheduler::Add(this);
   iFileMan  = CFileMan::NewL(iFs, this);
   iCancel   = EFalse;
   iFinder   = new (ELeave) TFindFile(iFs);
   isRunning = EFalse;
}
void CDeInstallerBase::RemoveDir(CDesCArray* aFileList)
{
   if(isRunning) {
      iDIState = ERequestDenied;
      CompleteRequest(KErrNone);
   } else {
      iFileList = aFileList;
      isRunning = ETrue;
      if(!iFinder) {
         iFinder  = new (ELeave) TFindFile(iFs);
      }
      iDIState = EDeInstalling;
      CompleteRequest(KErrNone);
   }
}

void CDeInstallerBase::DeleteFilesNoDrive(const TDesC& aPath,
                                          const TDesC& aFileWithoutExt,
                                          const TDesC& aExt)
{
   if(isRunning) {
      iDIState = ERequestDenied;
      CompleteRequest(KErrNone);
   } else {
      isRunning = ETrue;
      iPath = aPath.AllocL();
      iFileWithoutExt = aFileWithoutExt.AllocL();
      iExt = aExt.AllocL();
      if(!iFinder) {
         iFinder  = new (ELeave) TFindFile(iFs);
      }
      iDIState = EResourceDelRequest;
      CompleteRequest(KErrNone);
   }
}

void CDeInstallerBase::CancelOperation()
{
   iDIState = EDeInstallCancelled;
}


void CDeInstallerBase::RunL()
{
   switch (iStatus.Int()) {
   case KErrNone:
      switch (iDIState) {
      case EDeInstalling:
         if(iPathList && iPathList->Count() > 0) {
            iPathList->Delete(0);
         }
         InternalRemoveDir();
         break;
      case EDeInstallTryAgain:
         InternalRemoveDir();
         break;
      case EDeInstallComplete:
         RequestCompleted();
         break;
      case EDeInstallCancelled:
         RequestAborted();
         break;
      case EResourceDelRequest:
         InternalDeleteFiles();
         break;
      case EResourceDelRunning:
         InternalDeleteFiles();
         break;
      case ERequestDenied:
         SignalDeInstallerBusy();
         break;
      }
      break;
   case KErrPathNotFound:
      if(iFileList && iFileList->Count() > 0) {
         iFileList->Delete(0);
      }
      InternalRemoveDir();
         break;
   case KErrInUse:
      KillProcess();
      break;
   default:
      RequestAborted();
      break;
   }
}
            
void CDeInstallerBase::DoCancel()
{
   if(IsActive()) {
      class TRequestStatus* p = &iStatus;
      User::RequestComplete(p, KErrCancel);
   }
}


enum MFileManObserver::TControl CDeInstallerBase::NotifyFileManStarted()
{
   if(iDIState == EDeInstallCancelled) {
      return EAbort;
   }
   iFileMan->GetCurrentSource(iFileName);
   return EContinue;
}

enum MFileManObserver::TControl CDeInstallerBase::NotifyFileManOperation()
{
   if(iDIState == EDeInstallCancelled) {
      return EAbort;
   }
   return EContinue;
}

enum MFileManObserver::TControl CDeInstallerBase::NotifyFileManEnded()
{
   enum TControl ret;
   switch(iDIState) {
   case EDeInstalling:
      ret = EContinue;
      break;
   case EDeInstallCancelled:
      ret = EAbort;
      break;
   case EResourceDelRunning:
      ret = EContinue;
      break;
   case EDeInstallComplete:
      ret = EAbort;
      break;
   default:
      ret = EContinue;
      break;
   }
   return ret;
}



TPtrC CDeInstallerBase::GetLastDir(const TDesC& aPath)
{
   TChar sep = '\\';
   if(aPath.LocateReverse(sep) == aPath.Length() - 1) {
      // Last char is "\"
      TPtrC path = aPath.Left(aPath.LocateReverse(sep));
      return path.Right(path.Length() - path.LocateReverse(sep) - 1);
   } else {
      return aPath.Right(aPath.Length() - aPath.LocateReverse(sep) - 1);
   }
}

void CDeInstallerBase::KillProcess()
{
   // XXX Kill application that is running this
   // file. 
   // Now we can get with help of the path get
   // the name of the application that is running
   // this file/directory. 
   // For now we just ignore this directory and go 
   // on with the next one.

   // XXX Later when we implement code for killing a
   // running process, we should set 
   // iDIState = EDeInstallingTryAgain
   iDIState = EDeInstalling;
   CompleteRequest(KErrNone);
}
   
void CDeInstallerBase::DeleteAndNullPathComponents()
{
   delete iPath;
   iPath = NULL;
   delete iFileWithoutExt;
   iFileWithoutExt = NULL;
   delete iExt; 
   iExt = NULL;
}

void CDeInstallerBase::InternalRemoveDir()
{
   if(iFileList && iFileList->Count() > 0) {
      TPtrC path;
      TParse parser;
      parser.Set(iFileList->MdcaPoint(0), NULL, NULL);
      if(parser.DrivePresent()) {
         SearchForDirWithDrive(parser);
         path.Set(parser.FullName());
      } else {
         if(iPathList && iPathList->Count() > 0) {
            path.Set(iPathList->MdcaPoint(0));
         } else {

            SearchForDirNoDrive(parser.FullName());
            if(iPathList && iPathList->Count() > 0) {
               path.Set(iPathList->MdcaPoint(0));
            }
         }
      }
      if(path != KNullDesC) {
         iDIState = EDeInstalling;
         // CAP: dependent
         if (KErrNone != iFileMan->RmDir(path, iStatus)) {
            CompleteRequest(KErrNone);
         } else {
            SetActive();
         }
      } else {
         iDIState = EDeInstallComplete;
         CompleteRequest(KErrPathNotFound);
      }
   } else {
      iDIState = EDeInstallComplete;
      CompleteRequest(KErrNone);
   }
}

void CDeInstallerBase::CompleteRequest(TInt aStatus)
{
   class TRequestStatus* status = &iStatus; 
   iStatus = KRequestPending;
   SetActive();
   User::RequestComplete(status, aStatus);
}

void CDeInstallerBase::InternalDeleteFiles()
{
   class CDir* dir = NULL;
   iParser.Set(*iExt, iFileWithoutExt, iPath);
   TInt ret = iFinder->FindWildByDir(iParser.NameAndExt(),
                                     iParser.Path(),
                                     dir);
   if(ret == KErrNone) {
      // CAP: dependent
      iFileMan->Delete(iFinder->File(), 0, iStatus);
      iDIState = EResourceDelRunning;
      SetActive();
   } else {
      iDIState = EDeInstallComplete;
      CompleteRequest(KErrNone);
   }
   delete dir;
}

TInt CDeInstallerBase::SearchForDirWithDrive(class TParse& aPath)
{
   class CDir* dir = NULL;
   class TParse parser;
   parser.Set(aPath.FullName(), NULL, NULL);
   // Remove last dir so that we can search for it
   // in its parent directory
   parser.PopDir();
   // Get a list of all dirs 
   TInt ret = iFs.GetDir(parser.FullName(), 
                         (KEntryAttDir | KEntryAttMatchExclusive), 
                         (ESortByName | EAscending), dir);
   if(ret == KErrNone) {
      for(TInt i=0; i<dir->Count(); i++) {
         TPtrC path = dir->operator[](i).iName;
         TPtrC lastDir = GetLastDir(aPath.FullName());
         if(path.MatchF(lastDir) >= 0) {
            // We found a match, add found dir to the path and 
            // return full path.
            parser.AddDir(path);
            aPath.Set(parser.FullName(),NULL,NULL);
            return KErrNone;
         }     
      }
      // Did not found a matching path, return "".
      delete dir;
      aPath.Set(KNullDesC, NULL, NULL);
      return KErrNotFound;
   } else {
      // Bad path return "".
      delete dir;
      aPath.Set(KNullDesC, NULL, NULL);
      return ret;
   }
} 
   
void CDeInstallerBase::SearchForDirNoDrive(const TDesC& aPath)
{
   _LIT(KColon, ":");
   class CDir* dir = NULL;
   class TParse parser;
   parser.Set(aPath, NULL, NULL);
   // No drive exists in the path, we have to look for the
   // path on all drives.
   class CDriveList* driveList = CDriveList::NewLC();
   //drivearray is owned by driveList
   CDesCArray* driveArray = driveList->GenerateDriveListL();
   if(iPathList) {
      // Have to remove leftovers, and have to reallocate
      // to be sure there is enough space.
      iPathList->Reset();
      delete iPathList;
      iPathList = NULL;
   }
   if(driveArray->Count() > 0){
      iPathList = new (ELeave) CDesCArrayFlat(driveArray->Count());
      for(TInt i = 0; i < driveArray->Count(); ++i) {
         // For each drive, search for matching directories.
         // Concatenate drive and path.
         //XXX Use TParse to build the path.
         HBufC* driveAndPath = HBufC::NewLC(aPath.Length() + 5);
         driveAndPath->Des().Copy(driveList->Get(i));
         driveAndPath->Des().Append(KColon);
         driveAndPath->Des().Append(aPath);
         parser.Set(*driveAndPath, NULL, NULL);
         parser.PopDir();
         // Get a list of all dirs 
         TInt ret = iFs.GetDir(parser.FullName(), 
                               KEntryAttDir | KEntryAttMatchExclusive, 
                               ESortByName | EAscending, dir);
         if(ret == KErrNone) {
            for(TInt i = 0; i < dir->Count(); ++i) {
               TPtrC path = (*dir)[i].iName;
               TPtrC lastDir = GetLastDir(aPath);
               if(path.MatchF(lastDir) >= 0) {
                  // We found a match, add found dir to the path and 
                  // add the full path to array containing matching 
                  // paths.
                  parser.AddDir(path);
                  iPathList->AppendL(parser.FullName());
                  parser.PopDir();
               }     
            }
         } else if(ret == KErrBadName) {
            // Bad path return "". No use in continue searching.
            delete dir;
            dir = NULL;
            CleanupStack::PopAndDestroy(driveAndPath);
            break;
         }
         CleanupStack::PopAndDestroy(driveAndPath);
      }
   }
   CleanupStack::PopAndDestroy(driveList);
   delete dir;
}

void CDeInstallerBase::SetRunning(TBool aIsRunning)
{
   isRunning = aIsRunning;
}

void CDeInstallerBase::LeaveHandled() 
{
   RequestAborted();
}
