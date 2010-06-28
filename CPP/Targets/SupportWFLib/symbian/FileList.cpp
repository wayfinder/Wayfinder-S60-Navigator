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

#include "FileList.h"

#include <f32file.h>

#include "WFTextUtil.h"
#include "WFSymbianUtil.h"
#include "TDesCHolder.h"

CFileObject::CFileObject(TUint32 filesize,
                         TUint32 action) :
   m_filesize(filesize), m_action(action)
{
}

void CFileObject::ConstructL(const char* filename, const char* url, 
                             const char* aChecksum,
                             const char *version, const char *versionFile,
                             const char* aVersionCheck,
                             const char* aIsoLang)
{
   m_filename     = WFTextUtil::strdupL(filename);
   m_url          = WFTextUtil::strdupL(url);
   m_version      = WFTextUtil::strdupL(version);
   m_versionFile  = WFTextUtil::strdupL(versionFile);
   m_checksum     = WFTextUtil::strdupL(aChecksum);
   if(aVersionCheck) {
      m_versionCheck = WFTextUtil::strdupL(aVersionCheck);
   }
   if(aIsoLang){
      m_isoLang = WFTextUtil::strdupL(aIsoLang);
   }
}

class CFileObject* CFileObject::NewLC(const char* aFilename, const char* aUrl,
                                      TUint32 aFilesize, const char* aChecksum,
                                      TUint32 aAction, const char *aVersion,
                                      const char *aVersionfile, 
                                      const char *aVersionCheck,
                                      const char* aIsoLang)
{
   class CFileObject* self = new (ELeave) CFileObject(aFilesize, aAction);
   CleanupStack::PushL(self);
   self->ConstructL(aFilename, aUrl, aChecksum, 
                    aVersion, aVersionfile, aVersionCheck, aIsoLang);
   return self;
}

class CFileObject* CFileObject::NewL(const char* aFilename, const char* aUrl,
                                     TUint32 aFilesize, const char* aChecksum,
                                     TUint32 aAction, const char *aVersion,
                                     const char *aVersionfile,
                                     const char *aVersionCheck,
                                     const char* aIsoLang)
{
   class CFileObject* self = 
      CFileObject::NewLC(aFilename, aUrl, aFilesize, aChecksum, 
                         aAction, aVersion, aVersionfile, aVersionCheck, 
                         aIsoLang);
   CleanupStack::Pop(self);
   return self;
}


CFileObject::~CFileObject()
{
   delete[] m_filename;
   delete[] m_url;
   delete[] m_version;
   delete[] m_versionFile;
   delete[] m_checksum;
}

void CFileObject::Reset()
{
   m_currentSize = 0;
}



const char *
CFileObject::getUrl() const
{
   return (m_url);
}
const char *
CFileObject::getFilename() const
{
   return (m_filename);
}

void CFileObject::SetFilenameL(const TDesC& aFilename)
{
   char* tmp = WFTextUtil::newTDesDupL(aFilename);
   delete[] m_filename;
   m_filename = tmp;
}


TUint32
CFileObject::getFileSize() const
{
   return (m_filesize);
}
const char*
CFileObject::getCheckSum() const
{
   return (m_checksum);
}
TUint32
CFileObject::getAction() const
{
   return (m_action);
}
void CFileObject::SetAction(TUint32 aAction)
{
   m_action = aAction;
}
TUint32
CFileObject::getCurrentSize() const
{
   return m_currentSize;
}
void
CFileObject::setCurrentSize(TUint32 newSize)
{
   m_currentSize = newSize;
}
const char *
CFileObject::getVersion() const
{
   return m_version;
}
const char *
CFileObject::getVersionFile() const
{
   return m_versionFile;
}
const char *
CFileObject::getVersionCheck() const
{
   return m_versionCheck;
}
const char* 
CFileObject::getIsoCode() const
{
   return m_isoLang;
}
class CFileObjectHolder* CFileObjectHolder::NewLC(MDesCArray& aDownloadList,
                                                  const TDesC& aBasePath,
                                                  const TDesC& aBaseUrl)
{
   class CFileObjectHolder* self = new (ELeave) CFileObjectHolder();
   CleanupStack::PushL(self);
   for(TInt i = 0; i < aDownloadList.MdcaCount(); ++i){
      char* filename = (aBasePath + aDownloadList.MdcaPoint(i)).StrDupLC();
      char* url = (aBaseUrl + aDownloadList.MdcaPoint(i)).StrDupLC();

      //char* filename = WFTextUtil::newTDesDupLC(aDownloadList.MdcaPoint(i));

      class CFileObject* obj = CFileObject::NewLC(filename, url, 
                                                  0xffffffff, //filesize
                                                  "", //XXX checksum 
                                                  CFileObject::EDoNothing,
                                                  "", "", "");

      self->unshift(obj);

      CleanupStack::Pop(obj);
      CleanupStack::PopAndDestroy(2, filename);
   }
   return self;
}


class CFileObject* 
CFileObjectHolder::shift(void)
{
   class CFileObject *res = m_first;

   if (res) {
      // Not empty
      if (!res->next) {
         // Last item
         res->next = NULL;
         m_first = NULL;
         m_last = NULL;
      } else {
         // More items
         res->next->prev = NULL;
         m_first = res->next;
         res->next = NULL;
      }
      m_numItems--;
      m_totalSize -= res->getFileSize();
   }
   // No items == return NULL.
   return res;
}

void
CFileObjectHolder::unshift(class CFileObject *newItem)
{
   newItem->next = m_first;
   if (m_first) {
      // Items on the list.
      m_first->prev = newItem;
      newItem->prev = NULL;
      m_first = newItem;
   } else {
      // No items.
      m_first = m_last = newItem;
      newItem->prev = NULL;
      newItem->next = NULL;
   }
   m_numItems++;
   m_totalSize += newItem->getFileSize();
}

void
CFileObjectHolder::remove(class  CFileObject *target)
{
   /* XXX check if element is on list. */

   if (target->next) {
      target->next->prev = target->prev;
   } else {
      /* No more elements after this one. */
      m_last = target->prev;
   }
   if (target->prev) {
      target->prev->next = target->next;
   } else {
      /* No elements before this one. */
      m_first = target->next;
   }

   m_numItems--;
   m_totalSize -= target->getFileSize();

   target->next = NULL;
   target->prev = NULL;
}

int
CFileObjectHolder::CheckVersionsL(class RFs &fsSession)
{
   HBufC* fileName_buf = HBufC::NewLC(256);
   TPtr fileName = fileName_buf->Des();

   HBufC* basePath_buf = HBufC::NewLC(256);
   TPtr basePath = basePath_buf->Des();
   basePath.Zero(); //XXX probably not needed,

   TInt upToDate = 1;

   class CFileObject *tmp = m_first;
   while (tmp) {
      /* For each file in the download list, check if it is the */
      /* correct version. */
      const char *vfile = tmp->getVersionFile(); 
      if ( strlen(vfile) > 0 ) {
         /* Version file string is not empty. */
         WFTextUtil::char2TDes(fileName, vfile);

         /* First we need to find it. */
         if(WFSymbianUtil::FindBasePath(fsSession, fileName, basePath, ETrue)){
            /* Found the directory. */
            class TParse op;
            op.Set(fileName, NULL, NULL);
            basePath.Append(op.NameAndExt());
            fileName.Copy(basePath);
         } else {
            /* Can't find the directory. */
            /* Fall through, let the below code handle that the */
            /* version does not exist. */
         }

         /* We've got a version file, we can now check it. */
         const char *result = getVersionFromFile(fsSession, fileName);
         const char *serverVersion = tmp->getVersion();
         if (!result || !serverVersion || strcmp(result, serverVersion)) {
            /* Versions don't match. */
/* #define VERBOSE */
#ifdef VERBOSE
            fileName.Copy( _L("Version mismatch") );
            WFTextUtil::char2TDes(basePath, serverVersion? serverVersion : "");
            fileName.Append( _L(" =") );
            fileName.Append(*basePath);
            fileName.Append( _L("=!=") );
            WFTextUtil::char2TDes(basePath, result?result:"");
            fileName.Append(*basePath);
            fileName.Append( _L("=") );
            m_appui->ShowQuery(fileName);
#endif
            upToDate = 0;
            /* Next file. */
            tmp = tmp->next;
         } else {
            /* Versions match, remove the download. */
#ifdef VERBOSE
            fileName.Append( _L(" - Version match") );
            m_appui->ShowQuery(*fileName);
#endif
            /* Remove file from download list. */
            class  CFileObject* saveTmp = tmp;
            tmp = tmp->next;
            remove(saveTmp);
         }
      } else {
         /* No version file. */
         tmp = tmp->next;
         upToDate = 0;
      }
   }
   CleanupStack::PopAndDestroy(2, fileName_buf);
   return upToDate;
}

char *
CFileObjectHolder::getVersionFromFile(class RFs &fsSession, const TDesC &versionFile)
{       
   int i;
   TInt errorCode;
   TBuf8<256> tmpBuf;
   if (!WFSymbianUtil::getBytesFromFile(fsSession, versionFile,
         tmpBuf, errorCode, ETrue)) {
      /* An error occurred. */
      return NULL;
   }
   /* Got file content OK. */
   char *tmp = new char[tmpBuf.Length()+1];
   for (i = 0; i < tmpBuf.Length(); i++) {
      tmp[i] = tmpBuf[i];
      if (tmp[i] == '_') {
         break;
      }  
   }
   tmp[i] = 0;

   return (tmp);
}

void
CFileObjectHolder::clear(void)
{
   CFileObject *tmp;

   tmp = shift();
   while (tmp) {
      delete tmp;
      tmp = shift();
   }
}
