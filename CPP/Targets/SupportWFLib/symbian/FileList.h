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

#ifndef FILE_OBJECT_H
#define FILE_OBJECT_H
#include <e32base.h> //CBase 
#include <badesca.h> //CDesCArray
class CFileObject : public CBase
{
public:
   enum TFileObjectAction {
      EDoNothing        = 0,
      EInstallSisFile   = 1,
   };
private:
   ///@name Constructor and destructor.
   //@{
   CFileObject(TUint32 aFilesize, 
               TUint32 aAction);

   void ConstructL(const char* aFilename, const char* aUrl, 
                   const char *aChecksum,
                   const char *aVersion, const char *aVersionFile,
                   const char *aVersionCheck = NULL,
                   const char* aIsoLang = NULL);
public:
   static class CFileObject* NewLC(const char* aFilename, const char* aUrl,
                                   TUint32 aFilesize, const char* aChecksum,
                                   TUint32 aAction, const char *aVersion,
                                   const char *aVersionfile, 
                                   const char* aVersionCheck = NULL, 
                                   const char* aIsoLang = NULL);
   static class CFileObject* NewL(const char* aFilename, const char* aUrl,
                                  TUint32 aFilesize, const char* aChecksum,
                                  TUint32 aAction, const char *aVersion,
                                  const char *aVersionfile, 
                                  const char* aVersionCheck = NULL, 
                                  const char* aIsoLang = NULL);
   virtual ~CFileObject();
   //@}

   void Reset();

   const char *getUrl() const;
   const char *getFilename() const;
   TUint32 getFileSize() const;
   const char* getCheckSum() const;
   TUint32 getAction() const;
   void SetAction(TUint32 aAction);
   const char *getVersionFile() const;
   const char *getVersion() const;
   const char *getVersionCheck() const;
   const char* getIsoCode() const;

   TUint32 getCurrentSize() const;
   void setCurrentSize(TUint32 newSize);

   void SetFilenameL(const TDesC& aFilename);

private:
   char *m_filename;
   char *m_url;
   TUint32 m_filesize;
   char* m_checksum;
   TUint32 m_action;
   TUint32 m_currentSize;
   char *m_version;
   char *m_versionFile;
   char *m_versionCheck;
   char* m_isoLang;
public:
   class CFileObject *next;
   class CFileObject *prev;
};

class CFileObjectHolder : public CBase {
public:

   CFileObjectHolder(void) {
   };

   virtual ~CFileObjectHolder() {
      clear();
   };

   static class CFileObjectHolder* NewLC(MDesCArray& aDownloadList,
                                         const TDesC& aBasepath,
                                         const TDesC& aBaseUrl);

   /*
    * Dequeues the first object or returns NULL
    * if the list is empty.
    */
   class CFileObject* shift(void);

   /*
    * Adds the object at the beginning of the list.
    */
   void unshift(class CFileObject *newItem);
   void remove(class CFileObject *target);

   TInt CheckVersionsL(class RFs& fsSession);
   char *getVersionFromFile(class RFs& fsSession, const TDesC &versionFile);

   TInt getNumItems() {
      return m_numItems;
   };
   TInt getTotalSize() {
      return m_totalSize;
   };
   void clear(void);

private:
   class CFileObject *m_first;
   class CFileObject *m_last;
   TUint32 m_numItems;
   TUint32 m_totalSize;
};

#endif /* FILE_OBJECT_H */
