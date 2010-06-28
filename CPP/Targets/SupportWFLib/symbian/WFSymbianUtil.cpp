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

#include "WFSymbianUtil.h"

#include <f32file.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <algorithm>

#include <utf.h>

#include <badesca.h>

#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(__WINSCW__) || defined(NAV2_CLIENT_UIQ) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
# define NO_SYSUTIL
#endif
#ifndef NO_SYSUTIL
# include <sysutil.h>
#endif
#include "machine.h"

#include "LogMem.h"
#include "WFTextUtil.h"

using namespace std;

TBool
WFSymbianUtil::doesFileExistKeepFs(RFs& fsSession, const TDesC& aFileName,
                                   TInt& aSymbianErrorCode  )
{
   aSymbianErrorCode = KErrNone;
   bool error = false;

   RFile theFile;
   LOGOPEN(&theFile); aSymbianErrorCode = theFile.Open(fsSession, aFileName,
      EFileRead | EFileShareReadersOnly);
   if (aSymbianErrorCode != KErrNone) {
      error = true;
   } else {
      LOGANDCLOSE(&theFile);
   }
   return !error;
}

TBool
WFSymbianUtil::doesFileExist(RFs& fsSession, const TDesC& aFileName,
                             TInt& aSymbianErrorCode)
{
   aSymbianErrorCode = KErrNone;
   bool error = false;

   aSymbianErrorCode = fsSession.Connect();
   if (aSymbianErrorCode != KErrNone) {
      error = true;
   } else {
      RFile theFile;
      aSymbianErrorCode = theFile.Open(fsSession, aFileName,
            EFileRead | EFileShareReadersOnly);
      if (aSymbianErrorCode != KErrNone) {
         error = true;
      } else {
         theFile.Close();
      }
   }
   fsSession.Close();
   return !error;
}

TBool
WFSymbianUtil::getBytesFromFile(class RFs& fsSession, const TDesC& aFileName,
                                TDes8& aFileContent, TInt& aSymbianErrorCode,
                                TBool aUseFsAsIs)
{
   aSymbianErrorCode = KErrNone;
   bool error = false;

   aSymbianErrorCode = aUseFsAsIs ? KErrNone : fsSession.Connect();
   if (aSymbianErrorCode != KErrNone) {
      error = true;
   } else {
      RFile theFile;
      aSymbianErrorCode = theFile.Open(fsSession, aFileName,
            EFileRead | EFileShareReadersOnly);
      if (aSymbianErrorCode != KErrNone) {
         error = true;
      }
      else{
         TInt fileSize = MAX_INT32;
         aSymbianErrorCode = theFile.Size(fileSize);
         if (aSymbianErrorCode != KErrNone) {
            error = true;
         } else {
            if( fileSize > aFileContent.MaxLength() ){
               /* Too much to read. */
               error = true;
            } else {
               aSymbianErrorCode = theFile.Read( aFileContent );
               if (aSymbianErrorCode != KErrNone){
                  error = true;
               }
            }
         }
      }
      theFile.Close();
   }
   if(!aUseFsAsIs){
      fsSession.Close();
   }
   if (error){
      _LIT8(emptyStr,"");
      aFileContent.Copy(emptyStr); //set an empty string as file content.
   }
   return !error;
}

TBool
WFSymbianUtil::getBytesFromFile(RFs& fsSession, const TDesC& aFileName,
                                TDes16& aFileContent, TInt& aSymbianErrorCode)
{
   aSymbianErrorCode = KErrNone;
   bool error = false;

   HBufC8* data = HBufC8::NewLC( aFileContent.MaxLength() );
   TPtr8 dataPtr = data->Des();

   aSymbianErrorCode = fsSession.Connect();
   if (aSymbianErrorCode != KErrNone) {
      error = true;
   } else {
      RFile theFile;
      aSymbianErrorCode = theFile.Open(fsSession, aFileName,
                                       EFileRead | EFileShareReadersOnly);
      if (aSymbianErrorCode != KErrNone) {
         error = true;
      }
      else{
         TInt fileSize = MAX_INT32;
         aSymbianErrorCode = theFile.Size(fileSize);
         if (aSymbianErrorCode != KErrNone) {
            error = true;
         } else {
            if( fileSize > aFileContent.MaxLength() ){
               // Too much to read.
               error = true;
            } else {
               aSymbianErrorCode = theFile.Read( dataPtr );
               if (aSymbianErrorCode != KErrNone){
                  error = true;
               }
               else{
                  aFileContent.Copy( dataPtr );
               }
            }
         }
      }
      theFile.Close();
   }
   fsSession.Close();
   if (error){
      _LIT8(emptyStr,"");
      aFileContent.Copy(emptyStr); //set an empty string as file content.
   }
   CleanupStack::PopAndDestroy(data);
   return !error;
}

int
WFSymbianUtil::FindBasePath(RFs& fsSession, const TDesC& aFileName,
                            TDes& aResourcePath, TBool doNotConnectDisconnect)
{
   if (!doNotConnectDisconnect) {
      if (fsSession.Connect() != KErrNone){
         return 0;
      }
   }

   /* Find the first device which has a file which matches. */
   TFindFile file_finder(fsSession);
   TParse op;
   if (op.SetNoWild(aFileName, NULL, NULL) != KErrNone) {
      if (!doNotConnectDisconnect) {
         fsSession.Close();
      }
      return 0;
   }

   if (file_finder.FindByDir(op.NameAndExt(), op.Path()) != KErrNone) {
      if (!doNotConnectDisconnect) {
         fsSession.Close();
      }
      return 0;
   }

   const TDesC & fullpath = file_finder.File();
   TInt lastSepPos = fullpath.LocateReverse('\\');
   if (lastSepPos == KErrNotFound) {
      if (!doNotConnectDisconnect) {
         fsSession.Close();
      }
      return 0;
   }

   aResourcePath.Copy( fullpath.Left(lastSepPos+1) );
   if (!doNotConnectDisconnect) {
      fsSession.Close();
   }
   return 1;
}

int
WFSymbianUtil::writeBytesToFile(RFs &fsSession,
      const char *filename, const uint8 *data, int32 length, isab::Log* memLog)
{
   TBuf<256> realFilename;
   WFTextUtil::char2TDes(realFilename, filename);
   return WFSymbianUtil::writeBytesToFile(fsSession,
         realFilename, data, length, memLog);
}

int
WFSymbianUtil::writeBytesToFile(RFs &fsSession, TDesC& realFilename, 
                                const uint8 *data, int32 length, 
                                isab::Log* /*iLog*/)
{
   if( fsSession.Connect() != KErrNone ) {
      return writeBytesToFileErrorFsSession;
   }
   RFile file;
   TBuf<256> tmpName;
   tmpName.Copy( realFilename );
   tmpName.Append( _L("_new") );
   TInt fileSize = 0;

   /* Create file. */
   if( file.Create( fsSession, tmpName, EFileWrite ) != KErrNone ){
      /* File exists already... */
      file.Close();
      /* Open it instead. */
      if( file.Open( fsSession, tmpName, EFileWrite ) != KErrNone ){
         /* File cannot be written. */
         file.Close();
         fsSession.Close();
         return writeBytesToFileErrorCantWrite;
      } else {
         // file existed already, how many bytes does it contain.
         if(KErrNone != file.Size(fileSize)){
            fileSize = 0;
         }
      }
   }
   /* File was created successfully. */

#ifndef NO_SYSUTIL
   //Check file system for space
   CTrapCleanup *tc = CTrapCleanup::New();
   TBool result = false;
   TInt drive = EDriveC;
   if (toupper(realFilename[0]) == 'C') {
      drive = EDriveC;
   } else if (toupper(realFilename[0]) == 'E') {
      drive = EDriveE;
   }
   TRAPD(leaveValue, 
         result = SysUtil::DiskSpaceBelowCriticalLevelL(&fsSession,
                                                        length - fileSize, 
                                                        drive));
   delete tc;
   if(leaveValue != KErrNone || result){
      file.Close();
      fsSession.Delete(tmpName);
      fsSession.Close();
      if(result){
         return writeBytesToFileErrorBelowCriticalSpace;
      } else if(leaveValue == KErrNotReady){
         //no MMC present
         return writeBytesToFileErrorNoMMCPresent;
      } else if(leaveValue == KErrNotSupported){
         //no MMC support on this device
         return writeBytesToFileErrorNoMMCSupport;
      } else {      
         return writeBytesToFileErrorCLCheckError;
/*          return -100+leaveValue; */
      }
   }
   //Enough disk space...
#endif
   /* Write all data (in one chunk no less... :-) */
   TPtrC8* ptr = new (ELeave) TPtrC8( (TUint8*)data, length );
   if( file.Write( *ptr, length ) == KErrNone  ){
      /* Flush file. */
      if( file.Flush() == KErrNone ){
         /* OK. */
      } else {
         /* Not OK? What to do? */
      }
   } else {
      /* File write error.*/
      file.Close();
      fsSession.Delete(tmpName);
      fsSession.Close();
      delete ptr;
      return writeBytesToFileErrorCantWrite;
   }
   file.Close();
   delete ptr;
   fsSession.Delete( realFilename );
   if (file.Open( fsSession, tmpName, EFileWrite ) == KErrNone ){
      /* Rename the new file to the original name. */
      if( file.Rename( realFilename ) != KErrNone ) {
         file.Close();
         fsSession.Close();
         return writeBytesToFileErrorCantRenameTemp;
      }
      file.Close();
   }
   fsSession.Close();
   return 0;
}

HBufC8* WFSymbianUtil::AppendReallocPopLC(HBufC8* aDst, const TDesC8& aSrc,
                                          TReal aIncBy)
{
   TPtr8 ptr = aDst->Des();
   if(aSrc.Length() > (ptr.MaxLength() - ptr.Length())){
      TInt newLength = Max(KMaxTInt,
                           Max(TInt(ptr.Length() * aIncBy), 
                               aSrc.Length() + ptr.Length()));
      if(newLength < (ptr.Length() + aSrc.Length())){
         User::Leave(KErrNoMemory);
      }
      HBufC8* tmp = aDst->ReAllocL(newLength);
      CleanupStack::Pop(/*aDst*/); //C++ prohibits reading a pointer
                                   //value after it has been deleted.
      CleanupStack::PushL(aDst = tmp);
   }
   aDst->Des().Append(aSrc);
   return aDst;
}

HBufC8* WFSymbianUtil::ReadFileL(class RFs& aFs, const TDesC& aFileName)
{
   //open file
   class RFile file;
   User::LeaveIfError(file.Open(aFs, aFileName, 
                                EFileRead | EFileShareReadersOnly));
   CleanupClosePushL(file);
   //find filesize
   TInt filesize = 0;
   if(KErrNone != file.Size(filesize)){
      filesize = 2048;
   }
   HBufC8* readbuf  = HBufC8::NewLC(1024);
   TPtr8 readPtr = readbuf->Des();
   //alloc a buffer large enough to hold the entir file (hopefully)
   HBufC8* data = HBufC8::NewLC(filesize + 128);
   //read the file a couple of bytes at a time. 
   do {
      User::LeaveIfError(file.Read(readPtr));
      //make sure that the data descriptor is large enough to hold
      //the data.
      data = AppendReallocPopLC(data, readPtr);
   } while(readPtr.Length() > 0);
   //all data read, destrpy temporary buffers
   CleanupStack::Pop(data);
   CleanupStack::PopAndDestroy(readbuf);
   CleanupStack::PopAndDestroy(&file);
   return data;
}

HBufC8* WFSymbianUtil::ReadFileLC(class RFs& aFs, const TDesC& aFileName)
{
   HBufC8* data = ReadFileL(aFs, aFileName);
   CleanupStack::PushL(data);
   return data;
}

HBufC16* WFSymbianUtil::ReadUtf8FileL(class RFs& aFs, const TDesC& aFileName)
{
   //read file as 8bit data
   HBufC8* utf8data = ReadFileLC(aFs, aFileName);

   //allocate target buffer
   HBufC16* ucs2data = HBufC::NewLC(utf8data->Length());
   TPtr16 ucs2ptr = ucs2data->Des();
   //convert to ucs2
   TInt ret = CnvUtfConverter::ConvertToUnicodeFromUtf8(ucs2ptr, *utf8data);
   if(ret != KErrNone){
      if(ret > 0){
      //unconverted bytes! never mind....
      } else {
         User::Leave(ret);
      }
   }
   
   CleanupStack::Pop(ucs2data);
   CleanupStack::PopAndDestroy(utf8data);
   return ucs2data;
}

HBufC16* WFSymbianUtil::ReadUtf8FileLC(class RFs& aFs, const TDesC& aFileName)
{
   HBufC* data = ReadUtf8FileL(aFs, aFileName);
   CleanupStack::PushL(data);
   return data;
}
