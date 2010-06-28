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
#include <eikenv.h>
#include "DriveList.h"

class CDeInstallerBase : public CActive, public MFileManObserver
{
protected:
   /** @name Constructors and destructor. */
   //@{
   CDeInstallerBase(class RFs& aFs, 
                    enum TPriority aPriority = EPriorityStandard);
   virtual ~CDeInstallerBase();
   void ConstructL();
   //@}

   enum TDeInstallState {
      EDeInstalling        = 0,
      EDeInstallTryAgain   = 1,
      EDeInstallComplete   = 2,
      EDeInstallCancelled  = 3,
      EResourceDelRequest  = 4,
      EResourceDelRunning  = 5,
      ERequestDenied       = 6,
   };

public:
   /* Removes all dirs that is listed in aFileList.
    * Drive is optional but it is of course more
    * efficient to specify a drive. 
    * Wildcards is allowed in paths, ? represents
    * one character, * represents one or more charactes.
    * Search for and removal of paths is done case 
    * insensitive. 
    * Every path should end with \.
    */
   void RemoveDir(CDesCArray* aFileList);

   /* Removes one or more files that is listed in
    * aPath and fits matches aFileWithoutExt and
    * aExt. Wildcards are ollowed in filename and
    * in the extension. Wildcars should not be
    * used in the path. If no drive is specified
    * in aPath, every file that matches the description
    * on all drives are removed.
    */
   void DeleteFilesNoDrive(const TDesC& aPath,
                           const TDesC& aFileWithoutExt,
                           const TDesC& aExt);

   /**
    * Function that gets called from LeaveHandler
    * when processing of the leave is done.
    */
   void LeaveHandled();

protected:
   void CancelOperation();
   virtual void RunL();
   virtual void DoCancel();
   enum TControl NotifyFileManStarted();
   enum TControl NotifyFileManOperation();
   enum TControl NotifyFileManEnded();
   TPtrC GetLastDir(const TDesC& aPath);
   void KillProcess();
   void DeleteAndNullPathComponents();
   void InternalRemoveDir();
   void CompleteRequest(TInt aStatus);
   void InternalDeleteFiles();
   TInt SearchForDirWithDrive(class TParse& aPath);
   void SearchForDirNoDrive(const TDesC& aPath);
   void SetRunning(TBool aIsRunning);

   virtual void SignalDeInstallerBusy() = 0;
   virtual void RequestAborted() = 0;
   virtual void RequestCompleted() = 0;


private:
   class CFileMan*      iFileMan;
   CDesCArray*          iFileList;
   CDesCArray*          iPathList;
   class RFs&           iFs;
   enum TDeInstallState iDIState;
   TBool                iCancel;
   TFileName            iFileName;
   class TFindFile*     iFinder;
   class TParse         iParser;
   HBufC*               iPath;
   HBufC*               iFileWithoutExt;
   HBufC*               iExt; 
   TBool                isRunning;
};

template <class DeInstallRequester>
class CDeInstaller : public CDeInstallerBase
{

   CDeInstaller(DeInstallRequester* aRequester, class RFs& aFs,
                enum TPriority aPriority = EPriorityStandard) : 
      CDeInstallerBase(aFs, aPriority), iRequester(aRequester)
   {
   }

public:
   static CDeInstaller<DeInstallRequester>* NewL(DeInstallRequester* aRequester,
                                                 enum TPriority aPriority = EPriorityStandard)
   {
      class RFs& fs = CEikonEnv::Static()->FsSession();
      CDeInstaller<DeInstallRequester>* self = 
         new (ELeave) CDeInstaller(aRequester, fs, aPriority);
      CleanupStack::PushL(self);
      self->ConstructL();
      CleanupStack::Pop(self);
      return self;
   }

   virtual void SignalDeInstallerBusy()
   {
      iRequester->DeInstallerBusy();
   }
   virtual void RequestCompleted()
   {
      DeleteAndNullPathComponents();
      iRequester->DeInstallationCompleted();
      SetRunning(EFalse);
   }

   virtual void RequestAborted()
   {
      DeleteAndNullPathComponents();
      iRequester->DeInstallationAborted();
      SetRunning(EFalse);
   }

private:

private:
   DeInstallRequester*  iRequester;
};

#endif
