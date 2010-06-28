/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "ResourceFileName.h"

#include <f32file.h>
#include <badesca.h>

_LIT(KLangTxt, "lang.txt");
_LIT(KSharedDir, "shared");

enum TResourceFilePanics {
   EAppendDirNull          = 0,
   EListFileDirNull        = 1,   
   EListFilePartMissing    = 2,
   ESelectFromEmptyList    = 3,
   EFindAnyNoDrive         = 4,    
   EFindAnyNoPath          = 5
};
_LIT(KResource, "ResourceLang");
#define ASSERT_ALWAYS(cond_, panic_) __ASSERT_ALWAYS((cond_), User::Panic(KResource, (panic_)))
#define ASSERT_DEBUG(cond_, panic_) __ASSERT_DEBUG((cond_), User::Panic(KResource, (panic_)))

#ifdef __WINSCW__
# define INLINE
#else
# define INLINE inline
#endif

namespace {

   void AppendFiles(class CDesC16Array& aArray, const TDesC& aPath,
                    class CDir*& aDir, class TParse& aParser)
   {
      ASSERT_DEBUG(aDir != NULL, EAppendDirNull);
      for(TInt i = 0; i < aDir->Count(); ++i){
         aParser.Set((*aDir)[i].iName, &aPath, NULL);
         TRAPD(err, aArray.AppendL(aParser.FullName())); //ignore leaves.
      }
      delete aDir;
      aDir = NULL;
   }

   TBool PathNameAndExtPresent(const class TParseBase& aParser)
   {
      return (aParser.Path().Length() > 0 && 
              aParser.Name().Length() > 0 && 
              aParser.Ext().Length() > 0);
   }

   void ListFiles(class CDesC16Array& aArray, class TFindFile& aFinder,
                  class TParse& aParser)
   {
      ASSERT_DEBUG(PathNameAndExtPresent(aParser), EListFilePartMissing);
      class CDir* dir = NULL;
      TInt ret = 
         aFinder.FindWildByDir(aParser.NameAndExt(), aParser.Path(), dir);
      while(KErrNone == ret){
         ASSERT_DEBUG(dir != NULL, EListFileDirNull);
         AppendFiles(aArray, aFinder.File(), dir, aParser);
         ret = aFinder.FindWild(dir);
      }
   }

   INLINE TPtrC ToDesC(const TDesC& aArg)
   {
      return aArg;
   }

   INLINE TPtrC ToDesC(const class TEntry& aArg)
   {
      return aArg.iName;
   }

   template<class T>
   TFileName SelectResourceFile(const T& aFileList)
   {
      ASSERT_ALWAYS(aFileList.Count() > 0, ESelectFromEmptyList);
      const TInt KMaxLang = 99;
      for(TInt i = 1; i < KMaxLang; ++i){
         for(TInt j = 0; j < aFileList.Count(); ++j){
            TPtrC ext = TParsePtrC(ToDesC(aFileList[j])).Ext();
            TLex lex(ext);
            lex.Get(); // '.'
            lex.Get(); // 'r'
            TInt lang = 0;
            lex.Val(lang);
            if(lang == i){
               return ToDesC(aFileList[j]);
            }
         }
      }
      return KNullDesC();
   }

   TFileName FindAnyResourceFile(class RFs& aFs, const TDesC& aDriveAndPath, 
                                 const TDesC& aFileWithoutExt)
   {
      ASSERT_DEBUG(TParsePtrC(aDriveAndPath).DrivePresent(), EFindAnyNoDrive);
      ASSERT_DEBUG(TParsePtrC(aDriveAndPath).PathPresent(), EFindAnyNoPath);
      class TParse parser;
      _LIT(KAnyRExt, ".r??");
      parser.Set(aDriveAndPath, &aFileWithoutExt, &KAnyRExt);
      class TFindFile* finder = new TFindFile(aFs); //no ELeave
      if(finder){
         class CDesC16Array* filelist = new CDesC16ArrayFlat(32);
         if(filelist){
            ListFiles(*filelist, *finder, parser);
            return SelectResourceFile(*filelist);
         } else {
            //we have to select where to read from...
            //...how about the app drive?
            class CDir* dir = NULL;
            if(KErrNone == finder->FindWildByDir(parser.NameAndExt(), 
                                                 parser.DriveAndPath(), dir)){
               const TDesC& nameAndExt = SelectResourceFile(*dir);
               parser.Set(TParsePtrC(finder->File()).DriveAndPath(), 
                          &nameAndExt, NULL);
               return parser.FullName(); //???
            }
         }
         delete filelist;
      }
      delete finder;
      return parser.FullName();
   }

   TFileName FileExists(class RFs& aFs, const TDesC& aFilename)
   {
      class TFindFile finder(aFs);
      class TParsePtrC parser(aFilename);
      if(KErrNone == finder.FindByDir(parser.NameAndExt(), parser.Path())){
         return finder.File();
      }
      return KNullDesC();
   }

   TFileName AttemptResourceNameL(class RFs & aFs, 
                                  const TDesC & aStartName, 
                                  const TDesC & aSysDefName) 
   {
      TFileName retval;

      if((retval = FileExists(aFs, aStartName)) != KNullDesC){
         return retval;   // Prefered file existed. Use it.
      }

      // Try the system default
      if((retval = FileExists(aFs, aSysDefName)) != KNullDesC){
         return retval;// System default file existed. Use it.
      }

      // Try the same file but with an .rsc extension
      class TParse parser;
      _LIT(Krsc, ".rsc");
      parser.SetNoWild(Krsc, &aStartName, NULL);
      if((retval = FileExists(aFs, parser.FullName())) != KNullDesC){
         return retval;// .rsc file existed. Use it.
      }

      class TParsePtrC startParse(aStartName);
      return FindAnyResourceFile(aFs, startParse.DriveAndPath(),
                                 startParse.Name());
   }

   INLINE TFileName AttemptResourceNameL(class RFs& aFs, 
                                         const TDesC& aOnlyName)
   {
      return AttemptResourceNameL(aFs, aOnlyName, aOnlyName);
   }


   /**
    * This is the workhorse function in the FindResourceFileName function set.
    */
   TFileName FindResourceFileName(class RFs& aFs, 
                                  const TDesC& aSystemResourceFilename,
                                  const TDesC8& aLangNum ) 
   {
      // Attempt to respect the user language preference
      TLex8 lexLangNum(aLangNum);
      TUint uLangNum = 0;
      TInt status = lexLangNum.Val(uLangNum);
      if((aLangNum.Length() == 0) || (aLangNum.Length() > 2) || 
         (status != KErrNone)){

         // No lang.txt - check if the system default exists
         // otherwise find something that works...
         TFileName rfn = AttemptResourceNameL(aFs, aSystemResourceFilename);
         return rfn;
      }

      
      TBuf<4> ext;
      _LIT( KRscExt, ".r%02u" );
      ext.Format(KRscExt, uLangNum);

      class TParse resFile;
      resFile.Set(ext, &aSystemResourceFilename, NULL);
   
      return AttemptResourceNameL(aFs, resFile.FullName(),
                                  aSystemResourceFilename);
   }

   /**
    * This function attempts to find the lang.txt file, containing the
    * user preferred language. If found, the content of that file is
    * passed on to the 
    * FindResourceFileName(RFs&, const TDesC&, const TDesC&) overload.
    * If the file is not found, the function is called with aLangNum set
    * to KNullDesC.
    */
   INLINE TFileName FindResourceFileName(class RFs& aFs,
                                         const TDesC& aSystemResourceFilename, 
                                         const HBufC* aLangFileWFPath)
   {
      class TFindFile finder(aFs);
      TBuf8<2> symbianLang;
      HBufC* langPath = HBufC::NewL(256);

      TPtr pLangPath = langPath->Des();
      pLangPath.Copy(aSystemResourceFilename);
      class TParsePtr pathParser(pLangPath);
      pathParser.PopDir();
      pathParser.PopDir();
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
      pathParser.AddDir(KSharedDir);
#endif
      if (aLangFileWFPath) {
         pathParser.AddDir(*aLangFileWFPath);
      } else {
         pathParser.AddDir(pathParser.Name());
      }
      pLangPath.Copy(pathParser.Path());
      TInt res = finder.FindByDir(KLangTxt, pLangPath);
      delete langPath;
      if(res == KErrNone){
         class RFile langfile;
         res = langfile.Open(aFs, finder.File(), 
                             (EFileRead | EFileShareReadersOnly));
         if(res == KErrNone){
            TBuf8<2> tmp;
            res = langfile.Read(tmp);
            if(res == KErrNone){
               symbianLang = tmp;
            }
         }
      }
      return FindResourceFileName(aFs, aSystemResourceFilename, symbianLang);
   }
}

/**
 * Replace the extension with rSC.
 */
TFileName RscResourceFileName(const TDesC& aSystemResourceFilename)
{
   class TParse parser;
   _LIT(KRsc, ".rsc");
   parser.Set(KRsc, &aSystemResourceFilename, NULL);
   return parser.FullName();
}

//This function does the compile time 
TFileName FindResourceFileName(const TDesC& aSystemResourceFilename, 
                               const HBufC* aLangFileWFPath)
{
   class RFs fs;
   if(KErrNone != fs.Connect()){      //we're really in trouble if we can't
      return aSystemResourceFilename; //connect here!
   }
   TFileName ret = FindResourceFileName(fs, aSystemResourceFilename, 
                                        aLangFileWFPath);
   fs.Close();
   return ret;   
}
