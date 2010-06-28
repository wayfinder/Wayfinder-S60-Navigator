/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <e32base.h>
#include <f32file.h>

#include "TCPHandler.h"
#include "TimeOutNotify.h"

class CFileDownloader : public CBase,
                        public MTimeOutNotify,
                        public MTCPHandler
{
   /** Enumerates the different parser states. */
   enum TReceiveState {
      EGetHeader,
      EGetStatus,
      EGetNeck1,
      EGetNeck2,
      EGetNeck3,
      EGetNeck4,
      EGetData,
      EGetItOn,
      EErrorState,
   };
public:
   /** @name Constructors and destructor. */
   //@{
   /** Virtual destructor. */
   virtual ~CFileDownloader();

   /**
    * Static constructor.
    * @param aNotifier
    * @param aFsSession
    * @param aSockServ
    * @return a new CFileDownloader object that is still pushed on the
    *         CleanupStack.
    */
   static 
   class CFileDownloader* NewLC(class MFileDownloaderNotifier *aNotifier,
                                class RFs& aFsSession,
                                class RSocketServ& aSockServ);
   /**
    * Static constructor.
    * @param aNotifier
    * @param aFsSession
    * @param aSockServ
    * @return a new CFileDownloader object.
    */
   static class CFileDownloader* NewL(class MFileDownloaderNotifier *aNotifier,
                                      class RFs& aFsSession,
                                      class RSocketServ& aSockServ);
   /**
    * Destroys the object and constructs a new one with the same
    * MFileDownloaderNotifier and RFs as the old object. 
    * The RSocketServ is supplied through the ReplaceLD call since it
    * is not stored locally by CFileDownloader but only passed on to
    * it's subobjects.

    * @param aToBeReplaced reference to this object. If aToBeReplaced
    *                      is not pointing to the same object as the
    *                      this pointer, a FileDownloader 1 panic is
    *                      raised.
    * @param aSockServ     needed to construct a new CFileDownloader 
    *                      object.
    */
   void ReplaceLD(class CFileDownloader*& aToBeReplaced,
                  class RSocketServ& aSockServ);

private:
   /**
    * Constructor.
    * @param aRfSession a reference to a RFs. The CFileDownloader
    *                   assumes that the RFs is already connected and
    *                   will neither connect nor close it.
    */
   CFileDownloader(class RFs& aRfSession);
   /**
    * Second phase constructor.
    * @param aNotifier
    * @param aSockServ
    */
   void ConstructL(class MFileDownloaderNotifier *aNotifier, 
                   class RSocketServ& aSockServ);
   //@}
public:

   void SetVersions(TUint aMajor, TUint aMinor, TUint aProto);
   int SetTarget( const char *aUrl, const char *aAdditional,
                  const char *aFilename, TUint32 aFilesize, 
                  const char* aChecksum,
                  class CConnectionSettings* aConnectionSettings = NULL );
   int SetTargetL( const char *url, const char *additional,
                   const TDesC& aFileName,
                   TUint32 fileSize, const char* aChecksum, 
                   class CConnectionSettings* aConnectionSettings = NULL );

   TInt SetFileNameL(const TDesC& aFile);
   void Start();
   void CancelDownload();

private:

   HBufC8* UserAgentLC();
   HBufC8* HttpRequestStringLC();

   /** @name From MTCPHandler. */
   //@{
   virtual void Receive(const TDesC8& aText, TInt aLength);
   virtual void ErrorNotify(TDesC& aErrMessage, TInt aErrCode);
   virtual void ErrorNotify(TInt aResourceId, TInt aErrCode);
   virtual void Connected();
   virtual void DownloadCancelled();
   //@}

   /**@name From MTimeOutNotify. */
   //@{
   /** The function to be called when a CTimeOutTimer timeout occurs. */
   virtual void TimerExpired();
   //@}

   /**
    * This is a scary function that parses an URL into host, port and
    * rest from the pattern <host>[:<port>]/<rest>. Note that the
    * first '/' in the string must be the one separating host (or
    * port) from the rest.
    * 
    * The parsed parts are assigned to *ret_host, *ret_rest, and
    * *ret_port.  *ret_host and *ret_rest are separately allocated
    * strings, BOTH OF WHICH ARE PUSHED ON THE CLEANUPSTACK.
    */
   int ParseUrlLCC(const char *org_url, char **ret_host,
                  char **ret_rest, TInt32 *ret_port);
   void SaveData(const TDesC8& aText);
   enum TReceiveState ParseByte(TInt dataByte);

public:
   void Cancel();
   void RequestFile();

private:

   /** The message engine */
   class CNewMessageHandler* iClient;

   /* Local cached data. */
   TBuf<512> iHeaderLine;
   HBufC *iHost;
   HBufC *iFileName;
   HBufC *iAdditional;
   HBufC *iLocalFileName;
   HBufC *iCheckSum;

   TUint32 iFileSize;
   TInt32  iDestPort;
   
   /** @name IAP, Proxy and User Agent settings. */
   //@{
   TInt32   iIAP;
   TBool    iProxyUsed;
   HBufC*   iProxyHost;
   TUint32  iProxyPort;
   HBufC*   iUAString;
   TBool    iUAStringSet;
   //@}

   /**
    * @name Version numbers for the X-wf-installer header 
    * The header is formatted: "X-wf-installer: <major>.<minor>, <proto>"
    */
   //@{
   /** Application major version*/
   TUint iMajor;
   /** Applicaiton minor version*/
   TUint iMinor;
   /** Packagelist version*/
   TUint iProto;
   //@}

   /** Pointer to the our master. */
   class MFileDownloaderNotifier *iNotifier;

   /** @name Internal state  */
   //@{
   TInt iCriticalError;
   TInt iGotHeader;
   TInt iDownloadComplete;
   enum TReceiveState iParseState;
   //@}

   /** File server session. */
   class RFs& iFsSession;
   /** @name File state. */
   //@{
   /** File server subsession, one per file. */
   class RFile iFile;
   //@}

   /** Timer active object */
   class CTimeOutTimer* iTimer;

   /** Indicates that download should be cancelled. */
   TBool iCancel;
};


#endif
