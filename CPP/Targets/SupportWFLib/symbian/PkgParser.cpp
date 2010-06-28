/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <ctype.h>
#include <stdlib.h>
#include "PkgParser.h"
#include "WFTextUtil.h"
#include "FileList.h"

TPkgParser::TPkgParser() : 
   iVersion(1), iTotalDataSize(0)
{
}

TPkgParser::TPkgParser(const TDesC& aIsoLang) : 
   iVersion(1), iTotalDataSize(0)
{
   iSelectedLang = aIsoLang;
}

TUint TPkgParser::Version()
{
   return 1;
}


namespace {

#if 1
   template<class INT32>
   TBool Parse32bit_(INT32& aTarget, char*& aString, 
                     INT32(*aStrToInt)(const char *,char **,int), 
                     TInt aBase = 10)
   {
      char* endp = NULL;
      INT32 tmp = aStrToInt(aString, &endp, aBase);
      if(endp && (*endp == ';' || *endp == '\0')){
         aTarget = tmp;
         aString = endp;
         if(*aString == ';'){
            ++aString;
         }
         return ETrue;
      }
      return EFalse;
   }

   TBool ParseUint32_(TUint32& aTarget, char*& aString, TInt aBase = 10)
   {
      return Parse32bit_<TUint32>(aTarget, aString, strtoul, aBase);
   }

   TBool ParseInt32_(TInt32& aTarget, char*& aString, TInt aBase = 10)
   {
      return Parse32bit_<TInt32>(aTarget, aString, strtol, aBase);
   }
#endif
   TBool ParseUint32(TUint32& aTarget, char*& aString, TInt aBase = 10)
   {
      char* endp = NULL;
      TUint32 tmp = strtoul(aString, &endp, aBase);
      if(endp && (*endp == ';' || *endp == '\0')){
         aTarget = tmp;
         aString = endp;
         if(*aString == ';'){
            ++aString;
         }
         return ETrue;
      }
      return EFalse;
   }
   
   TBool ParseInt32(TInt32& aTarget, char*& aString, TInt aBase = 10)
   {
      char* endp = NULL;
      TInt32 tmp = strtol(aString, &endp, aBase);
      if(endp && (*endp == ';' || *endp == '\0')){
         aTarget = tmp;
         aString = endp;
         if(*aString == ';'){
            ++aString;
         }
         return ETrue;
      }
      return EFalse;
   }
   
   TBool ParseString(char*& aTarget, char*& aSrc)
   {
      char* tmp = WFTextUtil::strsep(&aSrc, ";");
      aTarget = tmp;
      return aSrc != NULL;
   }
   
}

TInt TPkgParser::ParseLineL(class CFileObjectHolder* &aFileList,  
                            char* line,
                            TUint32& aTotalDataSize)
{
   char *save_line = line;
   TInt ret = 0;

   while(line && isspace(*line) ){ //strip whitespace
      ++line;
   }
   if(*line == '\n' || *line == '\0'){
      ret = 1;
   } else if (!line) { //We need a valid string.
      ret = 0;
   } else if (line[0] == '!') {
      /* This is the line with the total number of */
      /* bytes to read. */
      if((ret = ParseUint32(iTotalDataSize, ++line))){
         aTotalDataSize = iTotalDataSize;
      }
      ret = 1;
   } else if(line[0] == '#'){ //skip comment lines.
      //end function
      ret = 1;
   } else if(line[0] == '@'){ //version line
      ret = ParseInt32(iVersion, ++line);
   } else {
      ret = ParseDataLineL(aFileList, line);
   }
   delete save_line;
   return ret;
}

TInt TPkgParser::ParseDataLineL(class CFileObjectHolder* &aFileList,  
                                char* line)
{
   char*  localName    = NULL;
   char*  url          = NULL;
   uint32 fileSize     = 0;
   char*  checksum     = NULL;
   uint32 action       = 0;
   char*  version      = NULL;
   char*  versionFile  = NULL;
   char*  versionCheck = NULL;
   char*  isoLang      = NULL;

   TBool ok = ETrue;
   ok = ok && ParseString(localName,    line);
   ok = ok && ParseString(url,          line);
   ok = ok && ParseUint32(fileSize,     line);
   ok = ok && ParseString(checksum,     line);
   ok = ok && ParseUint32(action,       line);
   ok = ok && ParseString(version,      line); //last mandatory field
   TBool lineCorrect = ok;
   ok = ok && ParseString(versionFile,  line); //may return EFalse if
                                               //there is no ending ';'
   ok = ok && ParseString(versionCheck, line); 
   ok = ok && ParseString(isoLang,      line);
   if(!lineCorrect){
      return 0;
   }

   //ignore lines that are language tagged but doesn't match the
   //selected language.
   if(isoLang && strlen(isoLang) > 0 && iSelectedLang != KNullDesC){
      HBufC* lang = WFTextUtil::AllocL(isoLang);
      if(0 != lang->CompareF(iSelectedLang)){
         action = 3; //Delete file.
      }
      delete lang;
   }


   class CFileObject *a = CFileObject::NewLC(localName, url, fileSize, 
                                             checksum, action, version,
                                             versionFile, versionCheck, 
                                             isoLang);

   if (!aFileList) {
      aFileList = new (ELeave) CFileObjectHolder();
   }
   CleanupStack::Pop(a);
   aFileList->unshift(a);
   return 1;
}
