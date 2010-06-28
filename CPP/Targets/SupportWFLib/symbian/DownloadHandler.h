/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef DOWNLOAD_HANDLER_H
#define DOWNLOAD_HANDLER_H

#include <e32base.h> //CBase
#include <es_sock.h> //RSocketServ
#include "FileDownloaderNotifier.h"
#include "ProcessWaiter.h"
#include "IAPTester.h"
#include "DownloadGuiObserver.h"
#include "DeInstaller.h"


class CDownloadHandler : public CBase,
                         public MFileDownloaderNotifier,
                         public MProcessWaiterReceiver,
                         public MIAPTester,
                         public MDownloadGuiObserver
{
   ///@name Constructors and destructor.
   //@{
   CDownloadHandler(class RFs& aFsSession);
   void ConstructL(class MDownloadGui* aGui,
         const TDesC& aBasepath,
         class MDownloadObserver* aDownloadObserver = NULL,
         TBool aUseDefaultIAP = EFalse);

public:
   static class CDownloadHandler *NewLC(class MDownloadGui* aGui, 
         const TDesC& aBasepath,
         class MDownloadObserver* aDownloadObserver = NULL,
         TBool aUseDefaultIAP = EFalse);
   static class CDownloadHandler *NewL(class MDownloadGui* aGui,
         const TDesC& aBasepath,
         class MDownloadObserver* aDownloadObserver = NULL,
         TBool aUseDefaultIAP = EFalse);
   virtual ~CDownloadHandler();
   //@}

   void SetVersions(const class TVersion& aVersion);
public: 
   /** @name From MIAPTester */
   //@{
   void IAPSearchStart();
   void IAPSearchCancel();
   void IAPSearchSetIAP(const class CConnectionSettings& aIAP);
   void IAPSearchSetIAPHandler(class MIAPHandler* aHandler);
   //@}
private: 
   ///@name From MFileDownloaderNotifier.
   //@{
   void DownloadCompletedL(class CFileDownloader *download);
   void DownloadFailed(class CFileDownloader *download);
   void DownloadProgress(class CFileDownloader *download, TUint32 downloaded);

   void ShowSocketError( const TDesC& aText );
   void ShowSocketError( TInt aResourceId );
   void ShowSocketInformation( TInt aResourceId );
   void DownloadCancelled();
   //@}

   ///@name From MProcessWaiterReceiver
   //@{
   void waitDone(class TRequestStatus iStatus);
   //@}
   //
public: /* MDownloadGuiObserver */
   void UserCancel();

public:
   void ContinueDownloadL();
   void DownloadFromOverrideListL();
   void DownloadFromListL();
   void DownloadFromListMainL();
   void HandleDownloadAndContinueL();
   void downloadNewPackageList();
   ///Sets the package list and starts downloading.
   ///@param aPackageList The list of files to download. The
   ///                    CDownloadHandler object takes ownership of
   ///                    the CFileObjectHolder object.
   void SetPackageList(class CFileObjectHolder* aPackageList);

   void CancelInstall();
   void SetConnectionsSettings(class CConnectionSettings* aConnectionSettings);
   void SetBaseUrl(const TDesC& baseUrl);
   void startDownload(TBool aShiftToNext = ETrue);
   void TextMsgShown();
   void InstallMsgShown();
   void ZipDriveSelectedL(const TDesC& aDrive, 
                          class CDeInstallFileList* aDeInstFileList);
   void DownloadDriveSelectedL(const TDesC& aDrive);
   void SetDriveL(const TDesC& aDrive);
   void SetOverridePackageListL(const TDesC& aOverride);
   void SetSelectedLanguage(const TDesC& aIsoCode);

public: // Called from CDeInstaller
   void DeInstallationCompleted();
   void DeInstallerBusy();
   void DeInstallationAborted();

private:
#ifdef USE_TCPSOCKET
   TInt downloadFile(char *url, char *localFileName,
                    uint32 fileSize = 0, uint32 checksum = 0);
   TInt parseUrl(const char *org_url, char **ret_host, char **ret_rest);
#endif
   TInt ParseLineL( char* line );
   TInt InitFileListL(TBool aShowErrors, TBool aUseDLFileList = ETrue);
   TInt InitFileListL(TBool aShowErrors, const TDesC& aFileName, TBool aUseDLFileList);
   TInt ReadPkgListL(TBool aShowErrors, const TDesC& aFileName);

   void installFile(class CFileObject* current);
   void InstallSisL(class CFileObject* currnet);
   TInt DeleteFileL();
   void DeleteFile(const char* aFilename);
   TInt64 GetFreeMemory(const TDesC& aDrive);

public:
   TInt initUser();
   TInt parseUserFileLine( char* line );

   TBool IsDownloadInProgress();
   void CancelDl();


   //Enum that contains all the different action
   //params for field 5 in a pgklist.
   enum TPkgActionParams {
      EDoNothing    = 0,
      EInstallSis   = 1,
      EUninstallSis = 2,
      EDeleteFile   = 3,
      EInstallZip   = 4,
      EShowTextMsg  = 5,
      EGoToURL      = 6,
      EQuit         = 7,
   };

private:
   TInt m_downloadInProgress;
   class TThreadId m_currentInstallerThreadId;
   class RThread iInstallerThread;
   TInt m_numDownloads;
   class RSocketServ m_sockServ;
   class RFs& m_fsSession;

   enum TUpdateState {
      EUpdateStateWaitingForInstaller = 1,
      EUpdateStateIdle = 2,
      EUpdateStateSelectingDownloadDrive = 3,
      EUpdateStateWaitingForDeInstaller  = 4,
      EUpdateStateTxtMsgShown = 5, 
   };

   enum TUpdateState m_state;

   char *m_username;
   char *m_language;
   char *m_additional;
   char *m_packageUrl;

   class MIAPHandler* iHandler;
   class MDownloadGui* m_gui;
   HBufC *m_basePath;
   TBuf<1> iDrive;

   //#define USE_TCPSOCKET
#ifndef USE_TCPSOCKET
   class CFileDownloader* iFileDl;
   class CConnectionSettings* iConnectionSettings;
#endif
   class CFileObject* m_currentDownload;
   class CFileObjectHolder *m_fileList;
   class CFileObjectHolder *m_installList;
   TUint32 m_totalDataSize;

   class ProcessWaiter *m_processWait;
   class CSisInstaller* iSisInstaller;
   class CFileZip* iFileZip;
   class MDownloadObserver* iDownloadObserver;
   HBufC* iOverrideList;
   TBuf<2> iUserSelectedLang;
   TBool iCancel;
   TBool iUseDefaultIap;
   class TVersion iVersion;
   class CWFInstallerController* iWFController;

   typedef CDeInstaller<CDownloadHandler> CInstallerDeInstaller;
   CInstallerDeInstaller* iDeInstaller;
};

#endif /* DOWNLOAD_HANDLER_H */
