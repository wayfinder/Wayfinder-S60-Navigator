/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef SYMBIANR6_OUTPUT_FACTORY_H
#define SYMBIANR6_OUTPUT_FACTORY_H
#include "arch.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "Output.h"

#include <f32file.h>
#ifdef _MSC_VER
# include <e32svr.h>
#endif
#include "nav2util.h"
#include "WFTextUtil.h"


namespace isab{
   namespace {

      /** Output class for files using the fopen, fwrite, and fclose.
       * Use this class with causion as it opens and closes the file
       * on each call to realPuts.
       */
      class FileOutput : public Output{
      public:
         /** Constructor. Attempts to open the file.  
          * @param name the name of the file where output goes. 
          *        If a file with this name already exists, 
          *        a serial number from 0 to 49 is appended. 
          *        If all 50 files already exists, the file with
          *        the original name is overwritten.
          */
         FileOutput(const char* name);
         /** Destructor. */
         virtual ~FileOutput();
      protected:
         /** Does tha actual outputting. Is called in a thread 
          * safe manner by the Output::puts function.
          * NOTE: this function opens and closes the file. It's better to
          *       perform string concatenation before calling this function.
          * @param txt the string to print to file. Note that no
          *            newline is appended.
          * @return the number of charcters written.
          */
         virtual size_t realPuts(const char* txt);
      private:
         /** The file name used.*/
         char* m_name;
      };

      /** 
       * Output class for files using the Symbian RFile APIs. 
       * This class should be preferred over FileOutput, as this one
       * is more efficient due to the fact that the file is only 
       * opened once.
       * Note that even though this class has a name that start with
       * R, it's not a Symbian R-class.
      */
      class RFileOutput: public Output {

         /** Constructor. Attempts to open the file.  
          * @param name the name of the file where output goes. 
          *        If a file with this name already exists, 
          *        a serial number from 0 to 49 is appended. 
          *        If all 50 files already exists, the file with
          *        the original name is overwritten.
          */
         RFileOutput(bool numberedLogs, bool rotateLogs, 
                     uint32 numFiles, uint32 maxSize);
         TBool Construct(const char* name);
      public:
         static class RFileOutput* New(const char* aName, 
                                       TBool aNumberedLogs,
                                       TBool aRotateLogs, TUint32 aNumFiles,
                                       TUint32 aMaxSize);
         /** Destructor. Closes any open file.*/
         virtual ~RFileOutput();
      protected:
         /** Does tha actual outputting. Is called in a thread 
          * safe manner by the Output::puts function.
          * @param txt the string to print to file. Note that no
          *            newline is appended.
          * @return the number of charcters written.
          */
         virtual size_t realPuts(const char* txt);
         /** (Re)opens the output file, possibly rotating the files
          * as well. Operation depends on the parameter passed to the
          * constructor.
          */
         void reopenFile();

      private:

         TBool RotateLogs(const TDesC& aBaseName);
         TBool NextFileName(TDes& aNextName);

         /** The file server session required by the RFile API.*/
         class RFs iFs;
         /** The file object. */
         class RFile iFile;
         /** Indicates if the file is open and writable.*/
         TBool iOpen;
         /** The base name for the log file(s).*/
         HBufC* iLogFileName;
         /** Indicates whether to rotate logs.*/
         TBool iRotateLogs;
         /** How many files to use.*/
         TInt iNumFiles;
         /** How much data has been written to the open file.*/
         TUint32 iSizeSoFar;
         /** Max size for each file. */
         TUint32 iMaxSize;
      };

      //========================================================
      //==  Inlines for RFileOutput ============================

      /**
       * Abstracts away the difference between Symbian 9 and previous
       * versions regarding the sharing of a Rfs between RThreads. 
       * @param aFs the Rfs to share.
       * @return KErrNone or one of the system wide error codes.
       */
      TInt AutoShare(class RFs& aFs)
      {
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
         return aFs.ShareAuto();
#else
         return aFs.Share(RSessionBase::EAutoAttach);
#endif
      }

      RFileOutput::RFileOutput(bool /*numberedLogs*/, 
                               bool rotateLogs, uint32 numFiles, 
                               uint32 maxSize) : 
         iOpen(EFalse), iLogFileName(NULL),
         iRotateLogs(rotateLogs), iNumFiles(numFiles), 
         iSizeSoFar(0), iMaxSize(maxSize)
      {
      }

      TBool RFileOutput::Construct(const char* aName)
      {
         iLogFileName = WFTextUtil::Alloc(aName);
         if(iLogFileName && (KErrNone == iFs.Connect() &&
                             KErrNone == AutoShare(iFs))){
            //the session is opened and shared
            //open the file.
            reopenFile();
         }
         return iLogFileName != NULL;
      }

      class RFileOutput* RFileOutput::New(const char* aName, 
                                          TBool aNumberedLogs,
                                          TBool aRotateLogs, TUint32 aNumFiles,
                                          TUint32 aMaxSize)
      {
         class RFileOutput* self = new RFileOutput(aNumberedLogs, aRotateLogs, 
                                                   aNumFiles, aMaxSize);
         if(self && !self->Construct(aName)){
            delete self;
            self = NULL;
         }
         return self;
      }
      
      TPtrC DrivePathAndName(const class TParseBase& aParser)
      {
         TInt len = aParser.DriveAndPath().Length() + 
            aParser.Name().Length();
         return aParser.FullName().Left(len);
      }
      
      void FormatNumName(TDes& aDst, const TDesC& aWithoutExt, 
                         TInt aNum, const TDesC& aExt)
      {
         _LIT(KFormatNumName, "%S%d%S");      
         __ASSERT_ALWAYS((aExt.Length() == 0) || (aExt[0] == '.'),
                         User::Panic(_L("FormatNumName"), 0));
         aDst.Format(KFormatNumName, 
                     &aWithoutExt, aNum, &aExt);
         if(aExt.Length() < 1){
            //remove the period at the end of the name.
            aDst.SetLength(aDst.Length() - 1);
         }   
      }

      void FormatNumName(TDes& aDst, const TDesC& aFileName, TInt aNum)
      {
         class TParsePtrC parse(aFileName);
         FormatNumName(aDst, DrivePathAndName(parse), aNum, parse.Ext());
      }

      TBool RFileOutput::RotateLogs(const TDesC& aBaseName)
      {
         TFileName* numName1 = new TFileName;
         TFileName* numName2 = new TFileName;
         TBool ret = EFalse;
         if((ret = (numName1 && numName2))){
            class TParsePtrC parser(aBaseName);
            TPtrC noExt = DrivePathAndName(parser);
            TPtrC ext   = parser.Ext();
            
            FormatNumName(*numName1, noExt, iNumFiles - 1, ext);
            iFs.Delete(*numName1);
            *numName2 = *numName1;
            for(TInt i = iNumFiles - 2; i >= 0; --i){
               FormatNumName(*numName1, noExt, i, ext);
               iFs.Rename(*numName1, *numName2);
               *numName2 = *numName1;
            }
            iFs.Rename(aBaseName, *numName2);                
         }
         delete numName1;
         delete numName2;
         return ret;
      }

      TBool RFileOutput::NextFileName(TDes& aNextName)
      {
         TFileName* numName = new TFileName;
         TBool ret = numName != NULL;
         if(numName){
            TInt i = 0;
            for(i = 0; i < iNumFiles; ++i){
               FormatNumName(*numName, *iLogFileName, i);
               if(KErrNone == iFile.Open(iFs, *numName, 
                                         EFileRead|EFileStream)){
                  iFile.Close();
               } else {
                  break;
               }
            }
            if(i < iNumFiles){
               aNextName = *numName;
            } else {
               ret = EFalse;
            }
         }
         delete numName;
         return ret;
      }

      TBool Replace(class RFs& aFs, class RFile& aFile, 
                    const TDesC& aFilename)
      {
         const TInt options = EFileWrite | EFileStream | EFileShareAny;
         return KErrNone == aFile.Replace(aFs, aFilename, options);
      }

      TBool Exists(class RFs& aFs, const TDesC& aFilename)
      {
         class RFile file;
         TInt res = file.Open(aFs, aFilename, EFileRead | EFileStream);
         if(res == KErrNone){
            file.Close();
         }
         return res == KErrNone;
      }

      void RFileOutput::reopenFile(void)
      {
         TFileName* tName = new TFileName(*iLogFileName);

         //close the old file
         if (iOpen) {
            iFile.Flush();
            iFile.Close();
            iOpen = EFalse;
         }
         iSizeSoFar = 0;
         // what name shall we use?
         if((iNumFiles > 1) && Exists(iFs, *tName)){
            //file exists
            if (iRotateLogs) {
               RotateLogs(*iLogFileName);
            } else {
               NextFileName(*tName);
            }
         }
         iOpen = Replace(iFs, iFile, *tName);
         delete tName;
      }

      RFileOutput::~RFileOutput()
      {
         if(iOpen){
            iFile.Close();
            iOpen = EFalse;
         }
         delete iLogFileName;
         iFs.Close();
      }

      size_t RFileOutput::realPuts(const char* txt)
      {
         size_t ret = 0;
         if(iOpen){
            ret = strlen(txt);
            const uint8* uTxt = reinterpret_cast<const uint8*>(txt);
            TPtrC8 tTxt(uTxt, ret);
            iFile.Write(tTxt);
            iFile.Flush();
            iSizeSoFar += ret;
            if (iSizeSoFar > iMaxSize) {
               reopenFile();
            }
         }
         return ret;
      }

      //===== end of RFileOutput inlines ==================
      //==================================================


      //==================================================
      //===== inlines for FileOutput =====================

      FileOutput::FileOutput(const char* name) :
         Output(), m_name(NULL)
      {
         if(name){
            FILE* file = fopen(name, "rt");
            if(file != NULL){
               fclose(file);
               file = NULL;
               // file already exists
               char* copyName = new char[strlen(name) + 1];
               strcpy(copyName, name);
               char* ending = strrchr(copyName, '.');
               if(ending != NULL){
                  *ending++ = '\0';
               }
               char* numName = new char[strlen(name) + 10];
               int i;
               for(i = 0; i < 50; ++i){
                  if(ending){
                     sprintf(numName, "%s%d.%s", copyName, i, ending);
                  } else {
                     sprintf(numName, "%s%d", copyName, i);
                  }
                  file = fopen(numName, "rt");
                  if(file == NULL){
                     break;
                  }
                  fclose(file);
               }
               if(i >= 50){
                  strcpy(numName, name);
               }
               file = fopen(numName, "wt");
               if(file){
                  m_name = numName;
                  fclose(file);
               } else {
                  delete[] numName;
               }
               delete[] copyName;
            } else {
               file = fopen(name, "wt");
               if(file != NULL){
                  m_name = new char[strlen(name) + 1];
                  strcpy(m_name, name);
                  fclose(file);
               }
            }
         } 
      }

      FileOutput::~FileOutput()
      {
         delete[] m_name;
      }   

      size_t FileOutput::realPuts(const char* txt)
      {
         int len = strlen(txt);
         if(m_name){
            FILE* file = fopen(m_name, "at");
            if(file){
               fwrite(txt, sizeof(char), len, file);
               fflush(file);
               fclose(file);
            }
         }
         return len;
      }


      //===== end of FileOutput inlines ==================
      //==================================================


   }
      

   Output* OutputFactory::createOutput(const char* target) 
   {
      Output* ret = NULL;
      if(target == NULL) target = "null";
      char* memref = new char[strlen(target) + 1];
      char* remaining = memref;
      strcpy(remaining, target);
      const char delim[] = ":";
      char* type = strsep(&remaining, delim);
      if(strequ(type, "file")){

#define TENK 102400
#ifdef _MSC_VER
# define NUMFILES 1
# define FILESIZE (100 * TENK)
#else
# define NUMFILES 4
# define FILESIZE (6 * TENK)
#endif
         ret = RFileOutput::New(remaining, false, false, NUMFILES, FILESIZE);
      }else if(strequ(type, "tcp")){
         uint16 thePort = 0;
         char* host = NULL;
         char* threadName = new char[strlen(remaining) + 1];
         strcpy(threadName, remaining);
         if(!isdigit(*remaining)){
            host = strsep(&remaining, delim);
         } 
         long port = strtol(remaining, NULL, 10);
         if(port > 1 && port <= MAX_UINT16){
            thePort = uint16(port & 0x0ffff);
         }
         if(host && thePort != 0){
            ret = new SocketOutput(threadName, host, thePort);
         } else if(thePort != 0){
            /* NOT POSSIBLE! */
/*             ret = new SocketOutput(threadName, thePort); */
         }
         delete[] threadName;
      }
      delete[] memref;
      return ret;
   }

}
#endif
