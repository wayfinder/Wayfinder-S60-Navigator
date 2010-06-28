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

#include "FileDownloader.h"
#if !(defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
# include <sysutil.h>
#endif
#include <stdlib.h>
#include "TimeOutTimer.h"
#include "NewMessageHandler.h"
#include "ConnectionSettings.h"
#include "FileDownloaderNotifier.h"

#include "WFTextUtil.h"
#include "SocketConstants.h"

extern "C" {
   int      atoi    (const char *_nptr);
}

enum TFileDownloaderPanics {
   EOrgUrlNull = 0,
   EBadReplace = 1,
};
_LIT(KFileDownloader, "FileDownloader");

#define ASSERT_ALWAYS(cond__, panic__) \
 __ASSERT_ALWAYS((cond__), User::Panic(KFileDownloader, (panic__)))


static const TInt KTimeOut = 60000000; /* 60 seconds. */

CFileDownloader::CFileDownloader(class RFs& aFsSession) :
   iParseState(EGetStatus), iFsSession(aFsSession)
{
}

void
CFileDownloader::ConstructL(class MFileDownloaderNotifier *aNotifier,
                            class RSocketServ& aSockServ)
{
   iNotifier = aNotifier;

   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   iClient = CNewMessageHandler::NewL( this, &aSockServ );
   iCancel = EFalse;
}

class CFileDownloader*
CFileDownloader::NewLC(class MFileDownloaderNotifier *appui,
                       class RFs& aFsSession,
                       class RSocketServ& sockServ)
{
   class CFileDownloader *me = new (ELeave) CFileDownloader(aFsSession);
   CleanupStack::PushL(me);
   me->ConstructL(appui, sockServ);
   return me;
}

class CFileDownloader*
CFileDownloader::NewL(class MFileDownloaderNotifier *appui,
                      class RFs& fsSession, class RSocketServ& sockServ)
{
   class CFileDownloader *me =
      CFileDownloader::NewLC(appui, fsSession, sockServ);
   CleanupStack::Pop(me);
   return me;
}

void CFileDownloader::ReplaceLD(class CFileDownloader*& aToBeReplaced,
                                class RSocketServ& aSockServ)
{
   ASSERT_ALWAYS(aToBeReplaced == this, EBadReplace);
   class MFileDownloaderNotifier* nofifier = iNotifier;
   class RFs& fs = iFsSession;
   delete aToBeReplaced; //NOTE: no accesses of this beyond this point.
   aToBeReplaced = NULL;
   aToBeReplaced = NewL(nofifier, fs, aSockServ);
   
}

CFileDownloader::~CFileDownloader()
{
   delete iClient;

   delete iLocalFileName;
   delete iAdditional;
   delete iFileName;
   delete iHost;
   delete iProxyHost;

   delete iTimer;
}

void
CFileDownloader::Cancel()
{
   iCriticalError = 1;
   if (iClient) {
      iClient->Cancel();
   }
}

int
CFileDownloader::SetTargetL( const char *url, const char *additional,
                             const TDesC& aFileName,
                             TUint32 fileSize, const char* aChecksum,
                             class CConnectionSettings* aConnectionSettings )
{
   char *filename = WFTextUtil::newTDesDupL(aFileName);
   CleanupStack::PushL( filename );
   TInt ret = SetTarget(url, additional, filename,
                        fileSize, aChecksum, aConnectionSettings);
   CleanupStack::PopAndDestroy(filename);
   return ret;
}

int
CFileDownloader::SetTarget( const char *url, const char *additional,
                            const char *fileName,
                            TUint32 fileSize, const char* aChecksum,
                            class CConnectionSettings* aConnectionSettings )
{
   char *host = NULL;
   char *rest = NULL;
   TInt32 portNum;
   if (iClient == NULL || ParseUrlLCC(url, &host, &rest, &portNum) < 0) {
      return -1;
   }

   iLocalFileName = WFTextUtil::AllocL(fileName);
   iAdditional    = WFTextUtil::AllocL(additional);
   iHost          = WFTextUtil::AllocL(host);
   iFileName      = WFTextUtil::AllocL(rest);
   iCheckSum      = WFTextUtil::AllocL(aChecksum);

   CleanupStack::PopAndDestroy( rest );
   CleanupStack::PopAndDestroy( host );

   iFileSize = fileSize;
   iDestPort = portNum;

   /* Initialize variables */
   iIAP = -1; // default: Always ask
   iProxyUsed = false;
   iUAStringSet = false;

   /* If supplied, override settings */
   if ( aConnectionSettings ) {
      iIAP = aConnectionSettings->GetIAP();
      iProxyUsed = aConnectionSettings->IsProxyUsed();
      iProxyHost = WFTextUtil::AllocL(aConnectionSettings->GetProxyHost());
      iProxyPort = aConnectionSettings->GetProxyPort();
      iUAStringSet = aConnectionSettings->IsUAStringSet();
      iUAString = aConnectionSettings->GetUAString(); // does not transfer ownership
   }
#ifdef __WINS__
   iIAP = -2;
#endif

   class TParse parsed;
   parsed.Set(*iLocalFileName, NULL, NULL);
   /* Create directory if it does not exist. */
   TInt res = iFsSession.MkDirAll(parsed.DriveAndPath());

   /* Replace the target with an empty file. */
   res = iFile.Replace(iFsSession, *iLocalFileName, EFileWrite);
   if (KErrNone != res ) {
      /*          TBuf<280> error( _L("X:") ); */
      /*          error.AppendNum(res); */
      /*          error.Append( _L(":") ); */
      /*          error.Append( *iLocalFileName ); */
      /*          error.Append( _L(":") ); */
      /*          iNotifier->ShowQuery( error ); */
      iNotifier->ShowSocketError( socket_errors_fileerror_msg );
      iNotifier->DownloadFailed(this);
      iCriticalError = 1;
      return res;
   }
   return 0;
}

TInt CFileDownloader::SetFileNameL(const TDesC& aFile)
{
   HBufC* tmp = aFile.AllocL();
   delete iLocalFileName;
   iLocalFileName = tmp;
   return KErrNone;
}



void
CFileDownloader::Start()
{
   if ( !iHost || iHost->Length() == 0 ) {
      /* No initialized. */
      return;
   }
   iDownloadComplete = 0;
   if (iClient) {
      if ( iProxyUsed ) { /* if proxy settings was found */
         iClient->ConnectL( *iProxyHost, iProxyPort, iIAP );
      } else {
         iClient->ConnectL( *iHost, iDestPort, iIAP );
      }
   }
}

void
CFileDownloader::Connected()
{
   if (iClient && iClient->IsConnected() ) {
      RequestFile();
   }
   /* Start timer. */
   iTimer->After(KTimeOut);
}

void
CFileDownloader::TimerExpired()
{
   if (!iCriticalError) {
      /* Timer timed out! */
      ErrorNotify( socket_errors_connection_timeout_msg, KErrTimedOut );
      iCriticalError = 1;
   } else {
      /* Don't do anything, we've already returned a critical error. */
   }
}

enum CFileDownloader::TReceiveState
CFileDownloader::ParseByte(TInt dataByte)
{
   enum TReceiveState retval = EGetHeader;

   switch (iParseState) {
      case EGetStatus:
         iHeaderLine.Append(dataByte);
         if (dataByte == '\n') {

            /* Full line gotten. Compare it to the wanted status line. */
            TInt res = iHeaderLine.Find( _L("HTTP") );
            if (res == KErrNotFound) {
/*                iNotifier->ShowSocketError( iHeaderLine ); */
               /* Not the one. */
            } else {
/*                iNotifier->ShowSocketInformation( iHeaderLine ); */
               /* You are the one who is. */
               /* Get status of request. */
               /* XXX Get the status code and use it for error codes! XXX */
               if (iHeaderLine.Find(_L("200")) != KErrNotFound) {
                  iParseState = EGetNeck1;
               } else {
                  /* Failed request. */
                  iCriticalError = 1;
                  iParseState = EErrorState;
                  retval = EErrorState;
               }
            }
            /* Reset the header line descriptor to zero bytes. */
            iHeaderLine.Zero();
         }

      break;
      case EGetNeck1:
         if (dataByte == '\r') {
            iParseState = EGetNeck2;
         }
         break;

      case EGetNeck2:
         if (dataByte == '\n') {
            iParseState = EGetNeck3;
         } else {
            iParseState = EGetNeck1;
         }
         break;

      case EGetNeck3:
         if (dataByte == '\r') {
            iParseState = EGetNeck4;
         } else {
            iParseState = EGetNeck1;
         }
         break;

      case EGetNeck4:
         if (dataByte == '\n') {
            iParseState = EGetItOn;
            retval = EGetItOn;
/*             iNotifier->ShowSocketInformation( _L( "Got neck!" ) ); */
         } else {
            iParseState = EGetNeck1;
         }
         break;

      default:
         break;
   }

   return retval;
}

void
CFileDownloader::SaveData(const TDesC8& aText)
{
   iFile.Write( aText );
   iFile.Flush();
}

/**
 * CFileDownloader::Receive()
 *
 * This function is called in the active object context
 * when a chunk of data is received on TCP.
 */
void
CFileDownloader::Receive( const TDesC8& aText, TInt aLength )
{

   if(iCancel) {
      iNotifier->DownloadCancelled();
      return;
   }
   /* Check if we've received any error that makes it */
   /* necessary to discard the data. */
   if (iCriticalError) {
      if (iClient) {
         iClient->Disconnect();
      }
      return;
   }
   iTimer->Cancel();

   /* Check if we've already gotten the header. */
   if (!iGotHeader) {
      /* For each character, call the header parser. */
      TInt i = 0;
      while (i < aLength) {
         enum CFileDownloader::TReceiveState status = ParseByte(aText[i]);
         if (status == EGetItOn) {
            /* This means that the rest of aText is */
            /* data that should be saved. */
            iGotHeader = 1;
            if( i < (aLength-1) ){
               /* Last byte would have an index of aLength-1... */
               SaveData(aText.Mid(i+1));
               iNotifier->DownloadProgress(this, aLength-(i+1));
               /* Exit loop. */
               break;
            }
         } else if( status == EErrorState ){
            iGotHeader = 1;
            i = aLength;
         }
         i++;
      }

   } else {
      /* Save data to file. */
      SaveData(aText);
      iNotifier->DownloadProgress(this, aLength);
   }

   iTimer->After(KTimeOut);
}

void
CFileDownloader::ErrorNotify( TDesC& aErrMessage, TInt aErrCode )
{
   iTimer->Cancel();
   //since the download is either complete or failed, we close the file.
   iFile.Close();
   // Check if the error was a KErrEof, which signals that the
   // connection was closed by the other end.
   if (aErrCode == KErrEof) {
      iDownloadComplete = 1;
      iNotifier->DownloadCompletedL(this);
   } else {
      iNotifier->ShowSocketError( aErrMessage );
      iNotifier->DownloadFailed(this);
      iCriticalError = 1;
   }
}

void
CFileDownloader::ErrorNotify( TInt aResourceId, TInt aErrCode )
{
   iTimer->Cancel();
   //since the download is either complete or failed, we close the file.
   iFile.Close();

   /* Check if the error was a KErrEof, which signals that */
   /* the connection was closed by the other end. */
   if (aErrCode == KErrEof) {
      if( iParseState != EErrorState ){
         iDownloadComplete = 1;
         iNotifier->DownloadCompletedL(this);
      }else{
         iNotifier->DownloadFailed(this);
      }
   } else {
      iNotifier->ShowSocketError( aResourceId );
      iNotifier->DownloadFailed(this);
      iCriticalError = 1;
   }
}

void
CFileDownloader::DownloadCancelled()
{
   iTimer->Cancel();
   iFile.Close();
   iNotifier->DownloadCancelled();
}

void
CFileDownloader::CancelDownload()
{
   iCancel = ETrue;
   iClient->CancelDownload();
}

int
CFileDownloader::ParseUrlLCC(const char *org_url, char **ret_host,
                             char **ret_rest, TInt32 *ret_port)
{
   ASSERT_ALWAYS(org_url, EOrgUrlNull);
   char *host = NULL;
   char *rest = NULL;
   char *url = NULL;

   //save is the pointer to the originally alloced string.
   char* save = url = WFTextUtil::strdupLC(org_url);

   //split at the first '/'. Note that the slash is replaced by '\0'.
   //
   //Before strsep:
   // url ------|
   // tmp_rest--|
   //           v
   //          "host.domain.tld:port/path/file.ext\0"
   //
   //After strsep
   //  tmp_rest ------------------------|
   //  url -------|                     |
   //  tmp_host --|                     |
   //             v                     v
   //            "host.domain.tld:port\0path/file.ext\0"
   //
   char* tmp_rest = url;
   char* tmp_host = WFTextUtil::strsep(&tmp_rest, "/");
   if (!tmp_rest){
      /* Error parsing url. */
      CleanupStack::PopAndDestroy(save);
      return -1;
   }

   //alloc a string containg the host part.
   host = WFTextUtil::strdupLC(tmp_host);
   //alloc a string containing everything efter the first slash.
   rest = WFTextUtil::strdupLC(tmp_rest);

   //find if there is an explicit port set in the url. If so, it's
   //hiding behind the first ':' in the string.
   TInt32 tmp_port = 80;
   char* port = host;
   WFTextUtil::strsep(&port, ":");
   if (port) {
      char* endp = NULL;
      tmp_port = strtol(port, &endp, 10);
      if(endp == port || *endp != '\0'){
         //no digits or digits followed by garbage.
         CleanupStack::PopAndDestroy(3, save);
         return -1;
      } else {
         //only digits.
      }
   }
 
   CleanupStack::Pop( rest );
   CleanupStack::Pop( host );
   CleanupStack::PopAndDestroy( save );
   CleanupStack::PushL( host ); //strictly speaking we could loose the
   CleanupStack::PushL( rest ); //rest pointer if the PushL(host) 
                                //leaves..                        
   *ret_host = host;
   *ret_rest = rest;
   *ret_port = tmp_port;
   return 1;
}

HBufC8* CFileDownloader::UserAgentLC()
{
   HBufC8* ret = NULL;
   if(iUAStringSet){
      ret = WFTextUtil::NarrowLC(*iUAString);
   } else {
#if defined NAV2_CLIENT_SERIES60_V2 || defined SYMBIAN_CKON
      ret = SysUtil::UserAgentStringL();
      CleanupStack::PushL(ret);
#elif defined NAV2_CLIENT_UIQ
      ret = _L8("UIQ2.1").AllocLC();
#elif defined NAV2_CLIENT_UIQ3
      ret = _L8("UIQ3").AllocLC();
#elif defined NAV2_CLIENT_SERIES60_V1
      ret = _L8("S60v1").AllocLC();
#elif defined NAV2_CLIENT_SERIES60_V3
      ret = _L8("S60v3").AllocLC();
#else
      ret = _L8("Unknown").AllocLC();
#endif
   }
   return ret;
}

HBufC8* CFileDownloader::HttpRequestStringLC()
{
   _LIT8(KHttpFormat, 
         "GET /%S%c%S HTTP/1.0\r\nUser-Agent: %S\r\nX-wf-installer: %u.%u, %u\r\nHost: %S%S\r\n\r\n");
   _LIT8(KHttpProxyFormat, 
         "GET http://%S%S/%S%c%S HTTP/1.0\r\nConnection: Close\r\nUser-Agent: %S\r\nX-wf-installer: %u.%u, %u\r\nHost: %S%S\r\n\r\n");
   HBufC8* getString = HBufC8::NewLC(384);
   HBufC8* userAgent = UserAgentLC();

   HBufC8* portString = HBufC8::NewLC(16);
   portString->Des().Format(_L8(":%u"), iDestPort);
   const TDesC8* usePortStr = &KNullDesC8;
   if(iDestPort != 80){
      usePortStr = portString;
   }

   TPtr8 getPtr = getString->Des();
   TText8 separator = '?';
   if(iFileName->Locate('?') != KErrNotFound){
      separator = '&';
   }

   HBufC8* file8 = WFTextUtil::NarrowLC(*iFileName);
   HBufC8* addi8 = WFTextUtil::NarrowLC(*iAdditional);
   HBufC8* host8 = WFTextUtil::NarrowLC(*iHost);

   
   if(iProxyUsed){
      //use appendformat with an overflow handler.
      getPtr.Format(KHttpProxyFormat, host8, usePortStr,
                    file8, separator, addi8, userAgent, 
                    iMajor, iMinor, iProto, host8, usePortStr);
   } else {
      getPtr.Format(KHttpFormat, file8, TUint8(separator), addi8, userAgent, 
                    iMajor, iMinor, iProto, host8, usePortStr);
   }
   CleanupStack::PopAndDestroy(5, userAgent);
   return getString;
}

void
CFileDownloader::RequestFile()
{
   if (!iFileName) {
      iCriticalError = 1;
      return;
   }
   HBufC8* request = HttpRequestStringLC();
   if (iClient) {
      iClient->SendMessageL(*request, request->Length() );
   }
   CleanupStack::PopAndDestroy(request);
   iNotifier->DownloadProgress(this, 0);
}

void CFileDownloader::SetVersions(TUint aMajor, TUint aMinor, TUint aProto)
{
   iMajor = aMajor;
   iMinor = aMinor;
   iProto = aProto;
}
