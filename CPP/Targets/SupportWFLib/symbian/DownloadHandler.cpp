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

#include "IAPHandler.h"
#include "DownloadHandler.h"

#include <eikdll.h>
#include <eikenv.h>

#include "DownloadGui.h"
#include "FileDownloader.h"
#include "FileList.h"
#include "SisInstaller.h"
#ifndef NAV2_CLIENT_SERIES60_V1
# include "FileZip.h"
#endif
#include "DownloadObserver.h"
#include "DownloadConstants.h"
#include "WFTextUtil.h"
#include "PkgParser.h"

#include "WFInstallerController.h"

#include "ConnectionSettings.h"
#include "MD5Symbian.h"

#include "LaunchWapBrowserUtil.h"
#include "WaitDialogEnum.h"

/* #define atoi _atoi */
extern "C" {
   int      atoi    (const char *_nptr);
}

_LIT( KLocalFileName, "package.list");
_LIT( KLocalTempName, "temp\\");
_LIT( KLocalUserName, "user.txt");
_LIT( KHandler, "DownloadHandler");

namespace {
   enum TInstallHandlerPanics {
      ELongIso                     = 0,
      ELongDrive                   = 1,
      EStateNotWaitingForInstaller = 2,
   };

}

#define PACKAGE_LIST_URL "PACKAGE_LIST_URL_HERE";

#undef DL_EVEN_IF_OVERRIDE_FILE_EXISTS

CDownloadHandler::CDownloadHandler(class RFs& aFsSession) : 
   m_fsSession(aFsSession),
   iWFController(NULL)
{
}

void
CDownloadHandler::ConstructL(class MDownloadGui* aGui,
                             const TDesC& aBasepath,
                             class MDownloadObserver* aDownloadObserver,
                             TBool aUseDefaultIAP)
{
   m_gui = aGui;
   m_gui->SetDownloadObserver(this);
   m_basePath = aBasepath.AllocL();
   
   m_state = EUpdateStateIdle;
   m_additional = new (ELeave) char[10];
   m_additional[0]=0;

   m_sockServ.Connect();
   
   iSisInstaller = CSisInstaller::NewL(this);
   iCancel = EFalse;
#ifndef NAV2_CLIENT_SERIES60_V1
   iFileZip = CFileZip::NewL();
#endif
   iDownloadObserver = aDownloadObserver;
   iUseDefaultIap = aUseDefaultIAP;
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
   m_sockServ.Share(RSessionBase::EAutoAttach);
#else
   m_sockServ.ShareAuto();
#endif
}

class CDownloadHandler*
CDownloadHandler::NewLC(class MDownloadGui* aGui,
                        const TDesC& aBasepath,
                        class MDownloadObserver* aDownloadObserver,
                        TBool aUseDefaultIAP)
{
   class RFs& fs = CEikonEnv::Static()->FsSession();
   class CDownloadHandler* tmp = new (ELeave) CDownloadHandler(fs);
   CleanupStack::PushL(tmp);
   tmp->ConstructL(aGui, aBasepath, aDownloadObserver, aUseDefaultIAP);
   return tmp;
}

class CDownloadHandler*
CDownloadHandler::NewL(class MDownloadGui* aGui,
                       const TDesC& aBasepath,
                       class MDownloadObserver* aDownloadObserver,
                       TBool aUseDefaultIAP)
{
   CDownloadHandler* tmp = CDownloadHandler::NewLC(aGui, aBasepath, 
                                                   aDownloadObserver,
                                                   aUseDefaultIAP);
   CleanupStack::Pop(tmp);
   return tmp;
}

CDownloadHandler::~CDownloadHandler()
{
   delete m_fileList;
   delete m_installList;
   if (iFileDl) {
      iFileDl->Cancel();
      delete iFileDl;
   }
   m_sockServ.Close();
   delete[] m_username;
   delete[] m_language;
   delete[] m_additional;
   delete[] m_packageUrl;
   delete iSisInstaller;
#ifndef NAV2_CLIENT_SERIES60_V1
   delete iFileZip;
#endif
   delete iOverrideList;
   delete m_basePath;
   //dont know if we actually should delete these.
   delete iConnectionSettings;
   delete m_currentDownload;
   //delete m_fileList;
   //delete m_installList;
   delete m_processWait;
   if (iWFController) {
      delete iWFController;
      iWFController = NULL;
   }
}

void CDownloadHandler::SetVersions(const class TVersion& aVersion)
{
   iVersion = aVersion;
}


void CDownloadHandler::SetPackageList(class CFileObjectHolder* aPackageList)
{
   if(!m_downloadInProgress){
      delete(m_fileList);
      m_fileList = aPackageList;
      m_numDownloads = m_fileList->getNumItems();
      startDownload();
   }
}

void CDownloadHandler::SetSelectedLanguage(const TDesC& aIsoCode)
{
   __ASSERT_ALWAYS(aIsoCode.Length() == 2, User::Panic(KHandler, ELongIso));
   iUserSelectedLang = aIsoCode;
}


void
CDownloadHandler::UserCancel()
{
   iCancel = ETrue;
   iFileDl->CancelDownload();
}
/**
 * This routine is called when the package list is downloaded correctly.
 */
void
CDownloadHandler::DownloadCompletedL(class CFileDownloader* /*download*/)
{
   if (iHandler) {
      if(iUseDefaultIap) {
         ContinueDownloadL();
      } else {
         iHandler->IAPSearchSuccess();
      }
   } else {
      ContinueDownloadL();
   }
}

namespace {
   /**
    * Verifies that a file matches its checksum.
    * The type of checksum is specified in the beginning of the
    * checksum argument.
    * Currently supported checksum algorithms are: md5.
    * @param aFs the file server session used to read the file. 
    * @param aFilename the name of the file to verify. 
    * @param aChecksum consists of two ':'-separated fields. The first
    *                  field specifies the checksum type, the second
    *                  field is a string representation of the
    *                  checksum. MD5 is assumed if there is no first
    *                  field and no ':'.
    * @return KErrNone         if the checksum is correct.
    *         KErrCorrupt      if the checksum is wrong. 
    *         KErrNotSupported if the checksum type is unknown. 
    *         KErrNoMemory     if we run out of memory.
    *         KErrNotFound     if the file is not found. 
    * 
    */
   TInt CheckCheckSum(class RFs& aFs, const char* aFilename, 
                      const char* aCheckSum)
   {
      _LIT8(KMd5, "md5");

      TFileName tmpName;
      WFTextUtil::char2TDes(tmpName, aFilename);

      class TParsePtrC pTmp(tmpName);
      if(! pTmp.DrivePresent()){
         class TFindFile ff(aFs);
         if(KErrNone == ff.FindByDir(pTmp.NameAndExt(), pTmp.Path())){
            tmpName = ff.File();
         }
      }
      
      if(strcmp(aCheckSum, "-1") == 0) {
         // MD5 is -1, no checksum control should
         // be done.
         return KErrNone;
      } 
      char* sep = strchr(aCheckSum, ':');
      if(sep){
         //the checksum contains a checksum type specifier.
         TPtrC8 type(reinterpret_cast<const TUint8*>(aCheckSum), 
                     sep - aCheckSum);
         type.Set(reinterpret_cast<const TUint8*>(aCheckSum), 
                  sep - aCheckSum);
         if(KMd5().CompareF(type) == 0){
            return TMD5Checker::CheckFileStatic(aFs, tmpName, aCheckSum);
         } else {
            return KErrNotSupported;
         }
      } else {
         return TMD5Checker::CheckFileStatic(aFs, tmpName, aCheckSum);
      }
   }
}

void
CDownloadHandler::ContinueDownloadL()
{
   if (m_currentDownload == NULL) {
      DownloadFromListL();
   } else {
      HandleDownloadAndContinueL();
   }
}

void CDownloadHandler::DownloadFromOverrideListL()
{
   /* This means it's a override list download. (Not registered) */
   delete iFileDl;
   iFileDl = NULL;
   iFileDl = CFileDownloader::NewL(this, m_fsSession, m_sockServ);
   iFileDl->SetVersions(iVersion.iMajor, iVersion.iMinor,
                        TPkgParser::Version());
   iHandler->OverrideSearchDone();

   InitFileListL(0, EFalse);
   DownloadFromListMainL();
}

void CDownloadHandler::DownloadFromListL()
{
   /* This means it's a package list download. (Not registered) */
   InitFileListL(0);
   DownloadFromListMainL();
}

void CDownloadHandler::DownloadFromListMainL()
{
   m_gui->setDownloadStatus( download_handler_readpackagelist_text );
   m_downloadInProgress = 0;
   
   /* XXX CHECK VERSION!!! XXX */
   if (!m_fileList) {
      m_fileList = new (ELeave) CFileObjectHolder();
   }
   
   int upToDate = m_fileList->CheckVersionsL(m_fsSession);
   if (!upToDate) {
      if (m_gui->ShowDownloadQuery(download_handler_start_the_download)) {
         if(iDownloadObserver) {
            iDownloadObserver->SelectDownloadDriveL();
         } else {
            startDownload();
         }
      } else {
         /* User don't want to download. */
         if (m_gui) {
            m_gui->DownloadComplete();
         }
      }
   } else {
      /* Nothing to download. */
      m_gui->ShowDownloadInformation( download_handler_download_not_needed_version_ok );
      m_gui->setDownloadStatus( download_handler_downloadcomplete_text );
      m_gui->setDownloadInstr(download_handler_empty_text);
      m_gui->DownloadComplete();
   }
}

void CDownloadHandler::HandleDownloadAndContinueL()
{     
   if(iCancel) {
      // User has requested to cancel download and all 
      // connections has now been closed.
      DeleteFile(m_currentDownload->getFilename());
      return;
   }
   m_gui->ProgressFinishedL();

   TInt csStatus = CheckCheckSum(m_fsSession,
                                 m_currentDownload->getFilename(), 
                                 m_currentDownload->getCheckSum());
   if(csStatus == KErrCorrupt){
      DeleteFileL();
      iDownloadObserver->BadChecksum();
      return; //callback later
   }

   /* Check if we should do anything to the file just downloaded. */
   switch(TPkgActionParams(m_currentDownload->getAction())) {
   case EDoNothing: 
      /* Try to download the next part. */
      m_gui->ProgressFinishedL();
      delete m_currentDownload;
      m_currentDownload = NULL;
      iFileDl->Cancel();
      
      startDownload();
      break;
   case EInstallSis:
      InstallSisL(m_currentDownload);
      break;
   case EUninstallSis:
      //uninstall file
      break;
   case EDeleteFile:
      DeleteFile(m_currentDownload->getFilename());
      break;
   case EInstallZip:
      if(iDownloadObserver) {
         iDownloadObserver->SelectZipDriveL();
      }
      break;
   case EShowTextMsg:
      if(iDownloadObserver) {
         TFileName tmpName;
         WFTextUtil::char2TDes(tmpName, m_currentDownload->getFilename());
         iDownloadObserver->ShowTextMsgL(tmpName);
      }
      break;
   case EGoToURL:
      {
         HBufC* url = WFTextUtil::AllocLC(m_currentDownload->getUrl());
         LaunchWapBrowserUtil::LaunchWapBrowser(*url);
         CleanupStack::PopAndDestroy(url);
         
         /* Try to download the next part. */
         m_gui->ProgressFinishedL();
         delete m_currentDownload;
         m_currentDownload = NULL;
         iFileDl->Cancel();
         
         startDownload();
      }
      break;
   case EQuit:
      //iFileDl->Cancel();
      if(iDownloadObserver) {
         iDownloadObserver->ShowQuitMsgL();
      }
      break;
   }
}
   


void
CDownloadHandler::DownloadProgress(class CFileDownloader* /*download*/,
                                   TUint32 downloaded)
{
   TInt progress;

   if (m_currentDownload) {
      TUint32 curr = m_currentDownload->getCurrentSize()+downloaded;
      m_currentDownload->setCurrentSize(curr);
      progress = 100-((( m_currentDownload->getFileSize() - curr) * 100) /
                        m_currentDownload->getFileSize());
      if ( progress < 0) {
         progress = 0;
      }
      if (progress > 100) {
         progress = 100;
      }
      TBuf<80> line;
      line.AppendNum(progress);
      line.Append(_L("%"));
      m_gui->setDownloadStatus(download_handler_downloading_prompt_text, line);
      m_gui->ProgressStatusL(progress);
   }
}

void
CDownloadHandler::DownloadFailed(class CFileDownloader* /*download*/)
{
   if (iHandler) {
      iHandler->IAPSearchFailed();
   }
   if (m_currentDownload == NULL) {
      if (m_downloadInProgress) {
         /* This means it's a package list download. (Not registered) */
         m_gui->setDownloadStatus(download_handler_nopackagelist_text);
         m_gui->DownloadFailed();
      } else {
         /* Trailing message, ignore. */
         /* We can get these events spuriously after complete download. */
      }
   } else {
      delete m_currentDownload;
      m_currentDownload = NULL;
      /* File download failed, what to do? */
      /* Cancel all downloads and reinitialize the package list. */
      InitFileListL(0);
      m_gui->setDownloadStatus(download_handler_downloadfailed_text);
      m_gui->ProgressFinishedL();
      m_gui->DownloadFailed();
   }
   m_downloadInProgress = 0;
   iFileDl->Cancel();
}

void
CDownloadHandler::DownloadCancelled()
{
   iCancel = ETrue;
   ContinueDownloadL();
} 

void
CDownloadHandler::downloadNewPackageList(void)
{
   TBuf<256> tmpName;
   tmpName.Copy(*m_basePath);
   tmpName.Append(KLocalFileName);

   char *filename = WFTextUtil::newTDesDupL(tmpName);
   CleanupStack::PushL( filename );
   char *URL = PACKAGE_LIST_URL;
   if (m_packageUrl) {
      URL = m_packageUrl;
   }

#ifndef USE_TCPSOCKET
   if (m_downloadInProgress) {
      //TBuf<64> error( _L("Already downloading, please retry") );
      m_gui->ShowDownloadError( download_handler_alreadydownloading_msg );
      CleanupStack::PopAndDestroy( filename );
      return;
   }
   
   delete iFileDl;
   iFileDl = NULL;
   iFileDl = CFileDownloader::NewL( this, m_fsSession, m_sockServ );
   iFileDl->SetVersions(iVersion.iMajor, iVersion.iMinor,
                        TPkgParser::Version());

   int res;
   if(iUseDefaultIap) {
      res = iFileDl->SetTarget(URL, m_additional, filename, 0, 0, NULL);
   } else {
      res = iFileDl->SetTarget(URL, m_additional, filename, 0, 0, iConnectionSettings);
   }
   if (res < 0) {
      //TBuf<64> error( _L("Download failed, no package list, please retry") );
      m_gui->ShowDownloadError( download_handler_nopackagelist_msg );
      m_gui->setDownloadStatus( download_handler_nopackagelist_text );
      delete iFileDl;
      iFileDl = NULL;
   } else {
      m_downloadInProgress = 1;
      iFileDl->Start();
   }
#else
   if (downloadFile(URL, filename) < 0) {
      /* Error */
      //TBuf<64> error( _L("Download failed, no package list, please retry") );
      m_gui->ShowDownloadError( download_handler_nopackagelist_msg );
      m_gui->setDownloadStatus( download_handler_nopackagelist_text );
   }
#endif
   CleanupStack::PopAndDestroy( filename );
}

void
CDownloadHandler::startDownload(TBool aShiftToNext)
{
   if (m_fileList) {
      if (m_downloadInProgress) {
         /* Download in progress. */
         //TBuf<64> error( _L("Download in progress, please wait."));
         m_gui->ShowDownloadError( download_handler_downloading_msg );
         return;
      }

      //we should always shift EXCEPT when we 
      // a) already have a m_currentDownload AND
      // b) aShiftToNext is false
      if(!(m_currentDownload && !aShiftToNext)){
         m_currentDownload = m_fileList->shift();
      } 

      if (!m_currentDownload) {
         /* Nothing more to download! */
         //TBuf<64> error( _L("Download complete."));
         m_gui->ShowDownloadInformation(download_handler_downloadcomplete_msg);
         m_gui->setDownloadStatus( download_handler_downloadcomplete_text );
         m_gui->setDownloadInstr(download_handler_empty_text);
         m_gui->DownloadComplete();

         if(iOverrideList){ //the overridelist has been processed. Delete it
            m_fsSession.Delete(*iOverrideList);
            delete iOverrideList;
            iOverrideList = NULL;
         }

         return;
      }
      //reset download counters and stuff.
      m_currentDownload->Reset();

      if(iDownloadObserver) {
         iDownloadObserver->PrepareDownloadGuiL();
      }

      m_gui->SetInstrDownloadPartXOfYL(
            m_numDownloads - m_fileList->getNumItems(),
            m_numDownloads);

      iFileDl->ReplaceLD(iFileDl, m_sockServ);
      
      TFileName tmpName;
      WFTextUtil::char2TDes(tmpName, m_currentDownload->getFilename());

      if (iWFController) {
         delete iWFController;
         iWFController = NULL;
      }
      TBuf<16> tmpVersion;
      TBuf8<16> version;
      TBuf<256> versionFile;
      WFTextUtil::char2TDes(tmpVersion, m_currentDownload->getVersion());
      WFTextUtil::char2TDes(versionFile, m_currentDownload->getVersionFile());
      version.Copy(tmpVersion);
      if (strcmp(m_currentDownload->getVersionCheck(), "V") == 0) {  
         iWFController = CWFInstallerController::NewL(version, versionFile, 
                                                      CWFInstallerController::EVersion);
         if (iWFController->ControlSisInstallation()) {
            //App with correct version already exists, don't download.
            m_currentDownload->SetAction(EDoNothing);
            m_gui->ProgressStatusL(100); //100%
            DownloadCompletedL(iFileDl);
            return;
         }
      }

      if(strncmp(m_currentDownload->getCheckSum(), "md5:", 4) == 0){
         class TParsePtrC pTmp(tmpName);
         if(! pTmp.DrivePresent()){
            class TFindFile* ff = new (ELeave) TFindFile(m_fsSession);
            CleanupStack::PushL(ff);
            if(KErrNone == ff->FindByDir(pTmp.NameAndExt(), pTmp.Path())){
               tmpName = ff->File();
               iFileDl->SetFileNameL(tmpName);
               m_currentDownload->SetFilenameL(tmpName);
            }
            CleanupStack::PopAndDestroy(ff);
         }
         //if the file is not present, CheckFileStatic will not return
         //KErrNone.
         if(KErrNone == 
            TMD5Checker::CheckFileStatic(m_fsSession, tmpName, 
                                         m_currentDownload->getCheckSum())){
            //md5 match, don't download
            m_gui->ProgressStatusL(100); //100%
            DownloadCompletedL(iFileDl);
            return;
         }
      }

      {//some actions require no download.
         enum TPkgActionParams action = 
            TPkgActionParams(m_currentDownload->getAction());
         if(action == EDeleteFile
            || action == EGoToURL
            || action == EUninstallSis
            ) {
            m_gui->ProgressStatusL(100); //100%
            DownloadCompletedL(iFileDl);
            return;            
         }
      }

      { //assume an empty url means that we don't need to download
         if(!m_currentDownload->getUrl() || 
            strlen(m_currentDownload->getUrl()) == 0){
            m_gui->ProgressStatusL(100); //100%
            DownloadCompletedL(iFileDl);
            return;            
         }
      }

         
      { //Add a drive if we don't have it
         class TParse op;
         op.Set(tmpName, m_basePath, NULL);
         TInt ret = m_fsSession.MkDirAll(op.DriveAndPath());
         ret = ret;
         tmpName = op.FullName();
         m_currentDownload->SetFilenameL(tmpName);
      }

      TInt res = iFileDl->SetTargetL(m_currentDownload->getUrl(), m_additional,
                                     tmpName, m_currentDownload->getFileSize(),
                                     m_currentDownload->getCheckSum(),
                                     iConnectionSettings);

      if (res < 0) {
         //TBuf<64> error( _L("Download failed, please retry") );
         m_gui->ShowDownloadError( download_handler_downloadfailed_msg );
         m_gui->setDownloadStatus( download_handler_downloadfailed_text );
         m_downloadInProgress = 0;
      } else {
         m_gui->ProgressStartL();
         iFileDl->Start();
      }
   } else {
      //TBuf<64> error( _L("No package list, please select update package list") );
      m_gui->ShowDownloadError( download_handler_nopackagelist_msg );
   }
}

void
CDownloadHandler::installFile(class CFileObject *current)
{
   /* Start SIS installer... */
   TBuf<256> tmp;
   TBuf<256> tmpName;
   const char *str_tmp = current->getFilename();
   WFTextUtil::char2TDes(tmpName, str_tmp);
   if (str_tmp[1] == ':') {
      /* There is a device name at the front, */
      /* use the path as it is. */
      tmp.Copy(tmpName);
   } else {
      /* Just add the drivename. */
      TParse op;
      op.Set(*m_basePath, NULL, NULL);
      tmp.Copy(op.Drive());
      tmp.Append(tmpName);
   }

   m_gui->setDownloadLabel(download_handler_installing_text);

#if !(defined NAV2_CLIENT_UIQ3  || defined NAV2_CLIENT_SERIES60_V3)
   CApaCommandLine* cmdLine=CApaCommandLine::NewL();

   cmdLine->SetLibraryNameL(_L("z:\\System\\apps\\appinst\\appinst.app"));
   cmdLine->SetDocumentNameL(tmp);
   cmdLine->SetCommandL(EApaCommandRun);
   m_currentInstallerThreadId = EikDll::StartAppL(*cmdLine);
#endif

   /* XXX We need to wait for this install to complete before */
   /* starting another one... XXX */
   TInt res = iInstallerThread.Open(m_currentInstallerThreadId);

   if (res == KErrNone) {
      /* Ok, just start waiting for the install to finish. */
   } else {
      /* Failed to open handle... */
      //TBuf<64> error( _L("Error waiting for application install!" ) );
      m_gui->ShowDownloadError( download_handler_installwaiterror_msg );
   }
   if (!m_processWait) {
      m_processWait = ProcessWaiter::NewL(this, &iInstallerThread);
   }

   m_state = EUpdateStateWaitingForInstaller;
   m_processWait->Start();
}

void
CDownloadHandler::waitDone(class TRequestStatus aStatus)
{
   if(iCancel) {
      m_gui->DownloadComplete();
      return;
   }
   switch (m_state) {
   case EUpdateStateWaitingForInstaller:
      /* Install application exited. */
      if (aStatus == KErrNone) {
         m_state = EUpdateStateIdle;
         m_gui->setDownloadLabel(download_handler_empty_text);
         /* Assume that the installer exited successfully. */
         //CFileObject* tmp = m_installList->shift();
         //delete tmp;
         /* Start next download. */
         DeleteFileL();
         startDownload();
      } else {
         if(iDownloadObserver) {
            iDownloadObserver->ShowInstallMsgL();
         }
      }
      break;
   case EUpdateStateWaitingForDeInstaller:
      m_gui->setDownloadLabel(download_handler_empty_text);
      
      /* File removed */
      if (aStatus == KErrNone) {
         startDownload();
      } else {
         // XXX Do something?
         // m_gui->ShowDownloadError( download_handler_installreturnederror_msg );
      }
      m_state = EUpdateStateIdle;
      break;
   case EUpdateStateTxtMsgShown:
      m_state = EUpdateStateIdle;
      DeleteFileL();
      startDownload();
      break;
   default:
      User::Panic(KHandler, EStateNotWaitingForInstaller);
      /* Should never happen. */
      /* m_gui->ShowDownloadError(download_handler_internal_error_msg); */
      break;
   }
}

void CDownloadHandler::TextMsgShown() 
{
   m_state = EUpdateStateTxtMsgShown;
   waitDone(KErrNone);
}

void CDownloadHandler::InstallMsgShown() 
{
   InstallSisL(m_currentDownload);
}

void CDownloadHandler::ZipDriveSelectedL(const TDesC& aDrive, 
                                         class CDeInstallFileList* aDeInstFileList)
{
   __ASSERT_ALWAYS(aDrive.Length() == 1, User::Panic(KHandler, ELongDrive));
   iDrive = aDrive;
   TBuf<256> filename;
   WFTextUtil::char2TDes(filename, m_currentDownload->getFilename());
#ifndef NAV2_CLIENT_SERIES60_V1
   iFileZip->UnzipFileL(filename, aDrive, aDeInstFileList);
   m_state = EUpdateStateWaitingForInstaller;
   iFileZip->DeleteZipFileL(filename, aDrive);
#endif
   waitDone(KErrNone);
}

void CDownloadHandler::DownloadDriveSelectedL(const TDesC& aDrive)
{
   __ASSERT_ALWAYS(aDrive.Length() == 1, User::Panic(KHandler, ELongDrive));
   SetDriveL(aDrive);
   // Check free memory!
   TInt64 free = GetFreeMemory(aDrive);
   if ( m_fileList != NULL && free < m_fileList->getTotalSize() ) {
      m_gui->ShowSpaceWarningL(free, m_fileList->getTotalSize());
   }
   m_state = EUpdateStateWaitingForInstaller;
   startDownload();
}

TInt64 CDownloadHandler::GetFreeMemory(const TDesC& aDrive)
{
   TVolumeInfo volumeInfo;
   TDriveList driveList;
   User::LeaveIfError(m_fsSession.DriveList(driveList));
   TChar driveLetter;
   for(TInt driveNumber = EDriveA; driveNumber<=EDriveZ; driveNumber++) {
      if(driveList[driveNumber]) {
         m_fsSession.DriveToChar(driveNumber,driveLetter);
         HBufC* tmpDrive = HBufC::NewLC(1);
         tmpDrive->Des().Append(driveLetter);
         if(aDrive.CompareF(*tmpDrive) == 0) {
            CleanupStack::PopAndDestroy(tmpDrive);
            TInt err = m_fsSession.Volume(volumeInfo, driveNumber);
            if (err == KErrNone) {
               return volumeInfo.iFree;
            }
         }
         CleanupStack::PopAndDestroy(tmpDrive);
      }
   }
   return 0;
}

void CDownloadHandler::InstallSisL(class CFileObject *current)
{
   //install sis
   TBuf<256> tmp;
   TBuf<256> tmpName;
   const char *str_tmp = current->getFilename();
   WFTextUtil::char2TDes(tmpName, str_tmp);
   if (str_tmp[1] == ':') {
      /* There is a device name at the front, */
      /* use the path as it is. */
      tmp.Copy(tmpName);
   } else {
      /* Just add the drivename. */
      class TParse op;
      op.Set(*m_basePath, NULL, NULL);
      tmp.Copy(op.Drive());
      tmp.Append(tmpName);
   }

   if (iWFController) {
      delete iWFController;
      iWFController = NULL;
   }
   TBuf<16> tmpVersion;
   TBuf8<16> version;
   TBuf<256> versionFile;
   WFTextUtil::char2TDes(tmpVersion, current->getVersion());
   WFTextUtil::char2TDes(versionFile, current->getVersionFile());
   version.Copy(tmpVersion);

   if (strcmp(current->getVersionCheck(), "V") == 0) {  
      iWFController = CWFInstallerController::NewL(version, 
                                                   versionFile, 
                                                   CWFInstallerController::EVersion);
   } else if (strcmp(current->getVersionCheck(), "T") == 0) {
      /*
      iWFController = CWFInstallerController::NewL(version, 
                                                   versionFile, 
                                                   CWFInstallerController::ETimeStamp);
      */
      //XXX Since timestamp is not implemented we disable the check!!
      iWFController = NULL;
   } else {
      iWFController = NULL;
   }
   m_state = EUpdateStateWaitingForInstaller;
   iSisInstaller->StartInstallL(tmp, iWFController);
}

void CDownloadHandler::SetDriveL(const TDesC& aDrive) 
{
   __ASSERT_ALWAYS(aDrive.Length() == 1, User::Panic(KHandler, ELongDrive));
   TPtr basePtr = m_basePath->Des();
   _LIT(KFormatParams, "%c%S%S"); 
   _LIT(KColon, ":");
   _LIT(KBackslash, "\\");
   if(basePtr[1] == ':') {
      //There is a device name at the front, 
      //change the drive to the new one.     
      //if(m_basePath[0] == aDrive[0]) {
      if(basePtr.Left(1).Compare(aDrive) != 0) {
         //Not the same drives, change!         
         basePtr.Replace(0, 1, aDrive);
      }
   } else {
      //Just add the drivename.
      TFileName* fn = new (ELeave) TFileName(3);
      CleanupStack::PushL(fn); 
      fn->Format(KFormatParams, aDrive[0], &KColon, &KBackslash);
      basePtr.Insert(0, *fn);
      CleanupStack::PopAndDestroy(fn);
   }
}

TInt CDownloadHandler::DeleteFileL()
{
   //install sis
   const char *str_tmp = m_currentDownload->getFilename();
   HBufC* tmp = HBufC::NewLC(256);
   HBufC* tmpName = WFTextUtil::AllocLC(str_tmp);

   if (str_tmp[1] == ':') {
      /* There is a device name at the front, */
      /* use the path as it is. */
      tmp->Des() = *tmpName;
   } else {
      /* Just add the drivename. */
      class TParse op;
      op.Set(*m_basePath, NULL, NULL);
      tmp->Des().Append(op.Drive());
      tmp->Des().Append(*tmpName);
   }
   class RFs& fs = CEikonEnv::Static()->FsSession();
   TInt ret = fs.Delete(*tmp); 
   CleanupStack::PopAndDestroy(tmpName);
   CleanupStack::PopAndDestroy(tmp);
   return ret;
}

void CDownloadHandler::DeleteFile(const char* aFilename)
{
   class TParse parser;
   HBufC* tmpName = WFTextUtil::AllocLC(aFilename);
   parser.Set(*tmpName, NULL, NULL);
   if(!iDeInstaller) {
      iDeInstaller = CInstallerDeInstaller::NewL(this);
   }
   iDeInstaller->DeleteFilesNoDrive(parser.DriveAndPath(),
                                    parser.Name(),
                                    parser.Ext());
   m_state = EUpdateStateWaitingForInstaller;
   CleanupStack::PopAndDestroy(tmpName);
}

void CDownloadHandler::SetOverridePackageListL(const TDesC& aOverride)
{
   iOverrideList = aOverride.AllocL();
}

#ifdef USE_TCPSOCKET
int
CDownloadHandler::parseUrl(const char *org_url,
                                char **ret_host,
                                char **ret_rest)
{
   char *host;
   char *rest;
   char *url;
   char *save;
   char *tmp;
   char **tmpp;
   save = url = new (ELeave) char[strlen(org_url)+1];
   strcpy(url, org_url);

   tmpp = &url;
   tmp = WFTextUtil::strsep(tmpp, "/");
   if (!tmp || !tmpp || !*tmpp) {
      /* Error parsing url. */
      return -1;
   }
   host = new (ELeave) char[strlen(tmp)+1];
   strcpy(host, tmp);
   rest = new (ELeave) char[strlen(*tmpp)+1];
   strcpy(rest, *tmpp);

   delete save;

   *ret_host = host;
   *ret_rest = rest;
   return 1;
}

#include "TCPSocket.h"
using namespace isab;
int
CDownloadHandler::downloadFile(char *url,
                                    char *localFileName,
                                    uint32 fileSize,
                                    uint32 aChecksum)
{
   char *host;
   char *filename;

   parseUrl(url, &host, &filename);

   TCPSocket *my_sock;
   my_sock = new (ELeave) TCPSocket( 5 );
   my_sock->create();
   my_sock->connect(host, 80);

   while (my_sock->getState() != TCPSocket::CONNECTED) {
      RTimer timer;
      if ( timer.CreateLocal() != KErrNone ) {
          // Error
          timer.Close();
         /*           return false; */
      } else {

         TRequestStatus status;

         timer.After( status, 5*1000*1000 );
         User::WaitForRequest( status );
         timer.Close();
         if ( status != KErrNone ) {
             return false;
         }
      }
   }
   char *get_str = "GET /";
   char *end_str = " HTTP/1.0\r\n\r\n";

   my_sock->write(reinterpret_cast<const uint8*>(get_str), strlen(get_str));
   my_sock->write(reinterpret_cast<const uint8*>(filename), strlen(filename));
   my_sock->write(reinterpret_cast<const uint8*>(end_str), strlen(end_str));

   uint8 *buf;
   buf = new (ELeave) uint8[1024+5];
   buf[1024] = 0;
   buf[1025] = 0;
   buf[1026] = 0;
   buf[1027] = 0;
   FILE * saveFile = fopen(localFileName, "w");
   if (!saveFile) {
      my_sock->close();
      delete my_sock;
      delete buf;
      return false;
   }

   int res = 0;
   int found_header = 0;
   int i = 0;
   while (my_sock->getState() == TCPSocket::CONNECTED) {
      res = my_sock->read(buf, 1024, 20 * 1000*1000);
      if (res < 0 && res != -2) {
         break;
      }
      if (res == 0 || res == -2) {
         continue;
      }
      i = 0;
      while (i < res && !found_header) {
         if (buf[i] == '\r') {
            if (buf[i+1] == '\n') {
               if (buf[i+2] == '\r') {
                  if (buf[i+3] == '\n') {
                     found_header = 1;
                     i = i+3;
                  }
               }
            }
         }
         i++;
      }
      if (found_header) {
         fwrite(buf+i, res-i, 1, saveFile);
      }
   }
   fclose(saveFile);
   my_sock->close();
   delete buf;
   delete my_sock;

   delete host;
   delete filename;
   return 0;
}
#endif

TInt CDownloadHandler::InitFileListL(TBool aShowErrors, TBool aUseDLFileList)
{
   return InitFileListL(aShowErrors, KLocalFileName, aUseDLFileList);
}

TInt
CDownloadHandler::InitFileListL(TBool aShowErrors, 
                                const TDesC& aFileName, 
                                TBool aUseDLFileList)
{
   TInt res;

   /* Clear the old content of the file list. */
   if (m_fileList) {
      m_fileList->clear();
   }
   if (m_installList) {
      m_installList->clear();
   }

   /* Make sure our temporary directory exists and is empty. */
   HBufC* tmpNameC = HBufC::NewLC(256);
   TPtr tmpName = tmpNameC->Des();
   tmpName.Copy(*m_basePath);
   tmpName.Append(KLocalTempName);
   res = m_fsSession.MkDirAll(tmpName);
   if (!(res == KErrNone || res == KErrAlreadyExists)) {
      User::Leave(res); //XXX can we do error handling here?
   }

#ifdef DL_EVEN_IF_OVERRIDE_FILE_EXISTS
   if (aUseDLFileList) {
      tmpName.Copy(*m_basePath);
      tmpName.Append(aFileName);
      res = ReadPkgListL(aShowErrors, tmpName);
      if (!res) {
         CleanupStack::PopAndDestroy(tmpNameC);
         return res;
      }
   }
#endif 

   if (iOverrideList) { //use overridelist if set.
      tmpName = *iOverrideList;
      res = ReadPkgListL(aShowErrors, tmpName);
      if (!res) {
         CleanupStack::PopAndDestroy(tmpNameC);
         return res;
      }
   }
#ifndef DL_EVEN_IF_OVERRIDE_FILE_EXISTS
   else {
      if (aUseDLFileList) {
         tmpName.Copy(*m_basePath);
         tmpName.Append(aFileName);
         res = ReadPkgListL(aShowErrors, tmpName);
         if (!res) {
            CleanupStack::PopAndDestroy(tmpNameC);
            return res;
         }
      }
   }
#endif

   CleanupStack::PopAndDestroy(tmpNameC);
   return res;
}

TInt
CDownloadHandler::ReadPkgListL(TBool aShowErrors, const TDesC& aFileName)
{
   TInt res;

   m_numDownloads = 0;

   class RFile file;   

   if (KErrNone != file.Open(m_fsSession, aFileName, EFileShareExclusive)) {
      if (aShowErrors) {
         //No package list, please select update package list
         m_gui->ShowDownloadError( download_handler_nopackagelist_msg );
      }
      return 0;
   }

   /* Read the contents of the package list. */
   HBufC8* readBuf = HBufC8::NewLC(256);
   TPtr8 readBufPtr = readBuf->Des();
   char* tmpBuf = new (ELeave) char[512];
   CleanupStack::PushL( tmpBuf );
   int saved_pos;
   char *saved_str = NULL;
   char *curr;
   class TPkgParser pkgParser(iUserSelectedLang);
   TBool done = false;
   while (!done) {

      /* Read a chunk of the file. */
      res = file.Read(readBufPtr);
      if (res != KErrNone) {
         if (aShowErrors) {
            m_gui->ShowDownloadError( download_handler_unreadablepackagelist_msg );
         }
         file.Close();
         CleanupStack::PopAndDestroy( tmpBuf );
         CleanupStack::PopAndDestroy( readBuf );
         return 0;
      }

      /* If we have any data saved from the last chunk, */
      /* add it first. */

      int len = readBuf->Size();
      if (len == 0) {
         done = true;
         break;
      }
      int i = 0;
      if (saved_str) {
         strcpy(tmpBuf, saved_str);
         i = strlen(saved_str);
         len += i;
      }
      saved_pos = 0;

      /* Convert to char * */
      int j = 0;
      while (i < len) {
         tmpBuf[i++] = (*readBuf)[j++];
      }
      i = 0;

      curr = tmpBuf;
      while (i < len) {
         /* Find the next newline character. */
         if (curr[i] == '\n') {
            /* Send the line to the parsing routine. */
            char *tmp = new (ELeave) char[i+2-saved_pos];
            strncpy(tmp, curr+saved_pos, i+1-saved_pos);
            tmp[i-saved_pos] = 0;

            if(!m_fileList) {
               m_fileList = new CFileObjectHolder();
            }
            if(!pkgParser.ParseLineL(m_fileList, tmp, m_totalDataSize)) {
               if (aShowErrors) {
                  m_gui->ShowDownloadError( download_handler_badpackagelist_msg );
               }
               file.Close();
               delete[] saved_str;
               CleanupStack::PopAndDestroy( tmpBuf );
               CleanupStack::PopAndDestroy( readBuf );
               return 0;
            }

            saved_pos = i+1;
            tmp = NULL;
            /* Set saved string to character after newline. */
            if (saved_str) {
               /* Deleting and new:ing a saved_str every time */
               /* we find a newline is wasteful. It should only */
               /* be done when the last character in the buffer */
               /* isn't a newline... */
               delete[] saved_str;
            }
            saved_str = new (ELeave) char[len-saved_pos+1];
            strncpy(saved_str, &tmpBuf[saved_pos], len-(saved_pos));
            /* Nul terminate. */
            saved_str[len-saved_pos] = 0;
         }
         /* Next character. */
         i++;
      }

      /* If this was the last character in the chunk, but */
      /* there was no end of file, save the remaining unparsed */
      /* data for the next turn of the loop. */

   }

   CleanupStack::PopAndDestroy( tmpBuf );
   CleanupStack::PopAndDestroy( readBuf );

   if(done && aShowErrors){
      m_gui->ShowDownloadInformation( download_handler_packagelistread_msg );
   }
   file.Close();
   /* Make sure everything is deleted. */
   delete[] saved_str;
   
   if( m_fileList ) {
      m_numDownloads = m_fileList->getNumItems();
   } else {
      m_numDownloads = 0;
   }

   m_gui->SetInstrDownloadNumPartsL(m_numDownloads);
   return 1;
}

int
CDownloadHandler::parseUserFileLine( char* line )
{
   char *tmp;
   char *save_line = line;

   if (!line) {
      return 0;
   }
   char **line_p = &line;
   /* Replace newline with nul. */
   if (line[strlen(line)-1] == '\n') {
      line[strlen(line)-1] = 0;
   }

   if (!strncmp(line, "user=", 5)) {
      /* USER line. */
      tmp = m_additional;
      m_additional = new (ELeave) char[strlen(line)+strlen(tmp)+2];
      if (tmp[0] == 0) {
         /* Zero length, ignore. */
         m_additional[0] = 0;
      } else {
         /* Copy old data. */
         strcpy(m_additional, tmp);
         m_additional[strlen(tmp)] = '&';
         m_additional[strlen(tmp)+1] = 0;
      }
      strcat(m_additional, line);
      delete tmp;

      tmp = WFTextUtil::strsep(line_p, "=");
      m_username = new (ELeave) char[strlen(*line_p)+1];
      strcpy(m_username, *line_p);

   } else if (!strncmp(line, "lang=", 5)) {
      /* Language line. */
      tmp = m_additional;
      m_additional = new (ELeave) char[strlen(line)+strlen(tmp)+2];
      if (tmp[0] == 0) {
         /* Zero length, ignore. */
         m_additional[0] = 0;
      } else {
         /* Copy old data. */
         strcpy(m_additional, tmp);
         m_additional[strlen(tmp)] = '&';
         m_additional[strlen(tmp)+1] = 0;
      }
      strcat(m_additional, line);
      delete tmp;

      tmp = WFTextUtil::strsep(line_p, "=");
      m_language = new (ELeave) char[strlen(*line_p)+1];
      strcpy(m_language, *line_p);
   } else if (!strncmp(line, "url=", 4)) {
      /* Package list URL. */
      tmp = WFTextUtil::strsep(line_p, "=");
      m_packageUrl = new (ELeave) char[strlen(*line_p)+1];
      strcpy(m_packageUrl, *line_p);
   } else {
      /* Unknown, drop. */
   }

   /* Strsep destroys the line pointer, and so we delete the */
   /* previously copied pointer instead. Presto, no memory leak. */
   delete save_line;
   return 1;
}

void
CDownloadHandler::SetBaseUrl(const TDesC& baseUrl)
{
   _LIT(KQMark, "?");
   TInt res = baseUrl.Find(KQMark);
   if (res != KErrNotFound ) {
      m_packageUrl = WFTextUtil::newTDesDupL(baseUrl.Left(res));
   } else {
      m_packageUrl = WFTextUtil::newTDesDupL(baseUrl);
   }
   m_language   = WFTextUtil::strdupL("");
   m_username   = WFTextUtil::strdupL("");
   if (res != KErrNotFound) {
      m_additional = WFTextUtil::newTDesDupL(baseUrl.Mid(res+1));
   } else {
      m_additional = WFTextUtil::strdupL("");
   }
}

int
CDownloadHandler::initUser()
{
   TInt res;

   class RFile file;

   {
      HBufC* tmpName = HBufC::NewLC(256);
      tmpName->Des().Copy(*m_basePath);
      tmpName->Des().Append(KLocalUserName);
      if (KErrNone != file.Open(m_fsSession,
            *tmpName, EFileShareExclusive)) {
         m_gui->ShowDownloadErrorDialog( download_handler_no_user_file_msg );
         CleanupStack::PopAndDestroy(tmpName);
         return 0;
      }
      CleanupStack::PopAndDestroy(tmpName);
   }

   /* Read the contents of the package list. */
   int saved_pos;
   char* saved_str = NULL;
   char* curr;
   TBool done = false;
   HBufC8* readBuf = HBufC8::NewLC(256);
   TPtr8 readBufPtr = readBuf->Des();
   char* tmpBuf = new (ELeave) char[512];
   CleanupStack::PushL( tmpBuf );
   while (!done) {

      /* Read a chunk of the file. */
      res = file.Read(readBufPtr);
      if (res != KErrNone) {
         m_gui->ShowDownloadErrorDialog( download_handler_unreadable_user_file_msg );
         file.Close();
         CleanupStack::PopAndDestroy( readBuf );
         CleanupStack::PopAndDestroy( tmpBuf );
         return 0;
      }

      /* If we have any data saved from the last chunk, */
      /* add it first. */

      int len = readBuf->Size();
      if (len == 0) {
         done = true;
         break;
      }
      int i = 0;
      if (saved_str) {
         strcpy(tmpBuf, saved_str);
         i = strlen(saved_str);
         len += i;
      }
      saved_pos = 0;

      /* Convert to char * */
      int j = 0;
      while (i < len) {
         tmpBuf[i++] = (*readBuf)[j++];
      }
      i = 0;

      curr = tmpBuf;
      while (i < len) {
         /* Find the next newline character. */
         if (curr[i] == '\n') {
            /* Send the line to the parsing routine. */
            int line_length = i + 2 - saved_pos;
            char *tmp = new (ELeave) char[line_length];
            strncpy(tmp, curr+saved_pos, i+1-saved_pos);
            tmp[i-saved_pos] = 0;

            if (!parseUserFileLine(tmp)) {
               m_gui->ShowDownloadErrorDialog( download_handler_unreadable_user_file_msg );
               file.Close();
               delete[] saved_str;
               CleanupStack::PopAndDestroy( readBuf );
               CleanupStack::PopAndDestroy( tmpBuf );
               return 0;
            }

            saved_pos = i+1;
            tmp = NULL;
            /* Set saved string to character after newline. */
            if (saved_str) {
               /* Deleting and new:ing a saved_str every time */
               /* we find a newline is wasteful. It should only */
               /* be done when the last character in the buffer */
               /* isn't a newline... */
               delete[] saved_str;
            }
            saved_str = new (ELeave) char[len-saved_pos+1];
            strncpy(saved_str, &tmpBuf[saved_pos], len-(saved_pos));
            /* Nul terminate. */
            saved_str[len-saved_pos] = 0;
         }
         /* Next character. */
         i++;
      }

      /* If this was the last character in the chunk, but */
      /* there was no end of file, save the remaining unparsed */
      /* data for the next turn of the loop. */

   }

   file.Close();
   /* Make sure everything is deleted. */
   delete[] saved_str;
   CleanupStack::PopAndDestroy( readBuf );
   CleanupStack::PopAndDestroy( tmpBuf );

   return 1;
}

TBool
CDownloadHandler::IsDownloadInProgress()
{
   return m_currentDownload != NULL;
}

void
CDownloadHandler::CancelDl()
{
   if (iFileDl) {
      iFileDl->Cancel();
   }
}

void
CDownloadHandler::CancelInstall()
{
   if (m_state == EUpdateStateWaitingForInstaller) {
      m_processWait->Cancel();
   }
}

void
CDownloadHandler::ShowSocketError( const TDesC &aText )
{
   m_gui->ShowSocketError(aText);
}
void
CDownloadHandler::ShowSocketError( TInt socketError )
{
   m_gui->ShowSocketError(socketError);
}
void
CDownloadHandler::ShowSocketInformation( TInt socketError )
{
   m_gui->ShowSocketInformation(socketError);
}

void CDownloadHandler::IAPSearchStart() { 
   if (iHandler) { 
      downloadNewPackageList(); 
   } 
}

void CDownloadHandler::IAPSearchCancel() 
{
}

void
CDownloadHandler::IAPSearchSetIAP(const CConnectionSettings& aIAP)
{
   if (iHandler) {
      CConnectionSettings *tmp = CConnectionSettings::NewL(aIAP);
      if (iConnectionSettings) {
         delete iConnectionSettings;
      }
      iConnectionSettings = tmp;

      iHandler->IAPIdSet();
   }
}

void CDownloadHandler::IAPSearchSetIAPHandler(class MIAPHandler* aHandler) 
{ 
   iHandler = aHandler;
}

void 
CDownloadHandler::SetConnectionsSettings( CConnectionSettings* aConnectionSettings )
{
   iConnectionSettings = aConnectionSettings;
}

void 
CDownloadHandler::DeInstallationCompleted()
{
   waitDone(KErrNone);
}

void
CDownloadHandler::DeInstallerBusy()
{
   // XXX Try again later?
   waitDone(KErrNone);
}

void 
CDownloadHandler::DeInstallationAborted()
{
   // XXX Generate something else
   waitDone(KErrNone);
}
