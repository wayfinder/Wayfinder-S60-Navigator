/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "PathFinder.h"

#include <eikappui.h> 
#include <eikapp.h>
#include <eikenv.h>
#include <f32file.h>
#include <bautils.h>
#include <e32const.h>

#include "arch.h"
#include "WFTextUtil.h"
//#include "WayfinderSettings.h"

_LIT( KSysDataBasePath, "\\shared\\" );
// _LIT( KWFVoiceBasePath, "\\system\\data\\" );
_LIT( KWFVoiceBasePath, "\\resource\\" );
_LIT( KPathTrailer, "\\" );
_LIT( KShortResourceVersionFile, "resource_version.short" );
#if defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_UIQ3)
_LIT( KResourcePath, "\\resource\\apps\\" ); //Symbian9 specific.
#endif
_LIT( KTurnsMbmFile, "wfturns.mbm" ); //uiq specific.
_LIT( KWayfinderMbm, "wficons.mbm");
_LIT( KWFMapCacheBasePath, "\\system\\data\\wfmaps\\" );
_LIT( KWFAutoMapCacheBasePath, "\\system\\data\\" );
_LIT( KSyntaxExtension, ".syn");
_LIT( KCSIconPath, "csicons");
_LIT( KSyntaxName, "syntax.syn");
_LIT( KSyntaxPartName, "syntax.s");

CPathFinder::CPathFinder(class CEikAppUi* aAppUi, 
                         const TDesC& aLangCode, 
                         const TDesC& aAudioPrefix, 
                         /*class RFs& aFsSession,*/
                         TBool aUseRomBuild) :
   iAppUi(aAppUi),
   /*iFs(aFsSession),*/
   iLangCode(aLangCode),
   iAudioPrefix(aAudioPrefix),
   iUseRomBuild(aUseRomBuild)
// ,
//    iApplicationDllName(NULL),
//    iApplicationName(NULL),
//    iWayfinderPath(NULL),
//    iDataPath(NULL), 
//    iCommonDataPath(NULL), 
//    iLangResourcePath(NULL), 
//    iSysDataBasePath(NULL),
//    iAudioBasePath(NULL), 
//    iMbmName(NULL),
//    iTurnsMbmName(NULL),
//    iLogoMbmName(NULL)
{
}

CPathFinder::~CPathFinder()
{
   delete iApplicationDllName;
   delete iApplicationName;
   delete iApplicationNameBase;
   delete iWayfinderPath;
   delete iDataPath;
   delete iWritableDataPath;
   delete iCommonDataPath;
   delete iWritableCommonDataPath;
   delete iLangSyntaxPath;
   delete iLangResourcePath;
   delete iSysDataBasePath;
   delete iAudioBasePath;
   delete iMbmName;
   delete iTurnsMbmName;
   delete iLogoMbmName;
   delete iMapCacheBasePath;
   delete iWritableAutoMapCachePath;
   delete iResourceBasePath;
   delete iResourceNoExt;
   delete iCSIconPath;
//    delete iSyntaxPathAndName;
}

class CPathFinder* CPathFinder::NewLC(class CEikAppUi* aAppUi, 
                                      const TDesC& aLangCode, 
                                      const TDesC& aAudioPrefix,
                                      const TDesC& aLogoMbmFileName,
                                      const TDesC& aWfVoiceDir,
                                      const TInt aLangListResourceId,
                                      TBool aUseRomBuild)
{
   class CPathFinder* self = new (ELeave) CPathFinder(aAppUi, 
                                                      aLangCode, 
                                                      aAudioPrefix, 
                                                      /*fsSession,*/
                                                      aUseRomBuild);
   CleanupStack::PushL(self);
   self->ConstructL(aLogoMbmFileName, aWfVoiceDir, aLangListResourceId);
   return self;
}

class CPathFinder* CPathFinder::NewL(class CEikAppUi* aAppUi, 
                                     const TDesC& aLangCode, 
                                     const TDesC& aAudioPrefix,
                                     const TDesC& aLogoMbmFileName,
                                     const TDesC& aWfVoiceDir,
                                     const TInt aLangListResourceId,
                                     TBool aUseRomBuild)
{
   class CPathFinder* self = CPathFinder::NewLC(aAppUi, aLangCode,
         aAudioPrefix, aLogoMbmFileName, aWfVoiceDir, aLangListResourceId, aUseRomBuild);
   CleanupStack::Pop(self);
   return self;
}

void CPathFinder::ConstructL(const TDesC& aLogoMbmFileName,
                             const TDesC& aWfVoiceDir,
                             const TInt aLangListResourceId)
{
   iApplicationDllName = iAppUi->Application()->DllName().AllocL();
   SetupPathsL(aLogoMbmFileName, aWfVoiceDir, aLangListResourceId);
}

TPtrC StripLastUnderscoreToEnd(const TDesC& aString)
{
   TInt pos = aString.LocateReverse('_');
   if(pos == KErrNotFound){
      pos = aString.Length();
   }
   return aString.Left(pos);
}

void CPathFinder::SetupPathsL(const TDesC& aLogoMbmFileName,
                              const TDesC& aWfVoiceDir,
                              const TInt aLangListResourceId)
{
   iFs.Connect();

   /* Note: DLLName will include architechture. */
   class TParsePtrC pathParser(*iApplicationDllName);
   iApplicationName = pathParser.Name().AllocL();

   TInt sepPos = 0;
   sepPos = iApplicationName->LocateReverse('_');
   if (sepPos != KErrNotFound) {
      // Create a application name without the architecture type in it.
      iApplicationNameBase = HBufC::NewL(iApplicationName->Length());
      iApplicationNameBase->Des().Copy(iApplicationName->Left(sepPos));
   } else {
      iApplicationNameBase = iApplicationName->AllocL();
   }

   iDataPath = HBufC::NewL( pathParser.Drive().Length() +
                            KSysDataBasePath().Length() +
                            iApplicationName->Length() +
                            KPathTrailer().Length() );
   iDataPath->Des().Copy(pathParser.Drive());
   iDataPath->Des().Append(KSysDataBasePath);
   iDataPath->Des().Append(*iApplicationName);
   iDataPath->Des().Append(KPathTrailer);
   // Set up the writable data path
   iWritableDataPath = SetupWritablePathL(*iDataPath);

   iCommonDataPath = HBufC::NewL( pathParser.Drive().Length() +
                                  KSysDataBasePath().Length() +
                                  iApplicationNameBase->Length() +
                                  KPathTrailer().Length() );
   iCommonDataPath->Des().Copy(pathParser.Drive());
   iCommonDataPath->Des().Append(KSysDataBasePath);
   iCommonDataPath->Des().Append(*iApplicationNameBase);
   iCommonDataPath->Des().Append(KPathTrailer);
   // Set up the writable common data path
   iWritableCommonDataPath = SetupWritablePathL(*iCommonDataPath);

   /* XXX Weird setting the drive component to C on windows... */
   //This is since when using the SDK the app drive is Z.

   HBufC* writableAutoMapCachePath = 
      HBufC::NewLC(pathParser.Drive().Length() +
                   KWFAutoMapCacheBasePath().Length() +
                   iApplicationNameBase->Length() +
                   KPathTrailer().Length());
   writableAutoMapCachePath->Des().Copy(pathParser.Drive());
   writableAutoMapCachePath->Des().Append(KWFAutoMapCacheBasePath);
   writableAutoMapCachePath->Des().Append(*iApplicationNameBase);
   writableAutoMapCachePath->Des().Append(KPathTrailer);
   iWritableAutoMapCachePath = SetupWritablePathL(*writableAutoMapCachePath);
   CleanupStack::PopAndDestroy(writableAutoMapCachePath);

   TFileName tmpResourcePath(KWFVoiceBasePath);
   tmpResourcePath.Append(aWfVoiceDir);
   tmpResourcePath.Append(KPathTrailer);
   TFileName tmpAdditionalVoicesPath;
   tmpAdditionalVoicesPath.Copy(tmpResourcePath);

   CLanguageList* langList = CLanguageList::NewLC(aLangListResourceId);
   if (CheckForAdditionalVoicesL(tmpAdditionalVoicesPath, *langList)) {
      // Looks for additional voice packs. If found, iLangCode is
      // set to point to the correct language (only needed for
      // rom builds since for ordinary installations iLangCode
      // is correct hence no changes should be done).
      iLangSyntaxPath = tmpAdditionalVoicesPath.AllocL();
//       iSyntaxPathAndName = tmpResourcePath.AllocL();
   } else if (iUseRomBuild) {
      // If using rom build for wayfinder we need to look for the
      // correct syntax file using BaflUtils.
      // The syntax files will be located in \resource\wfvoice\syntax.s01, syntax.s02
      // depending on the language. The extension is based on the symbian language
      // codes.
      TFileName tmpSynName;
      tmpSynName.Copy(pathParser.Drive());
      tmpSynName.Append(tmpResourcePath);
      tmpSynName.Append(KSyntaxName);
      TLanguage lang;
      BaflUtils::NearestLanguageFile(iFs, 
                                     tmpSynName,
                                     lang);   
      char* tmp2 = WFTextUtil::newTDesDupLC(tmpSynName);
      CleanupStack::PopAndDestroy(tmp2);
      iLangSyntaxPath = tmpSynName.AllocL();
//       iSyntaxPathAndName = tmpSynName.AllocL();

      if (aLangListResourceId > 0) {
         // We have to set the iLanguageCode once again since we might have 
         // Wayfinder on Arabic with no voice instructions for Arabic. 
         // BaflUtils will then help us doing a fallback to the default language.
         // Get the correct iso name for the syntax language that BaflUtils
         // selected for us.
         if (lang == ELangNone) {
            // Probably just have a rsc file, not a r01 or similar.
            // Need to get the language code by calling User::Language
            lang = User::Language();
         }
         CLanguageList::TWfLanguageList & newLang =
            langList->FindBySymbianCode(lang);
         iLangCode = newLang.symbianname;
      }
   } else {
      // Put together the path to the syntax file depending on
      // the chosen language.
      TFileName tmpSynPath(KWFVoiceBasePath);
      tmpSynPath.Append(aWfVoiceDir);
      tmpSynPath.Append(KPathTrailer);
      tmpSynPath.Append(KSyntaxPartName);
      
      CLanguageList::TWfLanguageList & langListElem =
         langList->FindBySymbianName(iLangCode);
      if (langListElem.symbianCode < 10) {
         tmpSynPath.AppendNum(0);
      }
      tmpSynPath.AppendNum(langListElem.symbianCode);
      iLangSyntaxPath = tmpSynPath.AllocL();
   }
   CleanupStack::PopAndDestroy(langList);


   // Need to initialize following paths for rom build as well. 
   tmpResourcePath.Append(iLangCode);
   tmpResourcePath.Append(KPathTrailer);
   TFindFile fileFinder(iFs);
   // Search for the voice version file
   if( fileFinder.FindByDir( KShortResourceVersionFile,
                             tmpResourcePath ) != KErrNotFound ){
      // Correkt version file was found, set iLangResourcePath
      // to this path.
      tmpResourcePath.Copy(fileFinder.File());
      sepPos = tmpResourcePath.LocateReverse('\\');
      tmpResourcePath.SetLength(sepPos+1);
      iLangResourcePath = tmpResourcePath.AllocL();
   } else {
      delete iLangResourcePath;
      iLangResourcePath = NULL;
      delete iLangSyntaxPath;
      iLangSyntaxPath = NULL;
   }



   iSysDataBasePath = KSysDataBasePath().AllocL();

   iAudioBasePath = HBufC::NewL(KWFVoiceBasePath().Length() +
                                aWfVoiceDir.Length()+10);
   iAudioBasePath->Des().Copy(KWFVoiceBasePath);
   iAudioBasePath->Des().Append(aWfVoiceDir);
   iAudioBasePath->Des().Append(KPathTrailer);
   
   iMapCacheBasePath = KWFMapCacheBasePath().AllocL();

   // Create a temporary mapcache path that contains the installation drive
   // so we can do mkdirall on writeable drive further down below.
   HBufC* tmpMapCacheDrivePath = HBufC::NewLC(pathParser.Drive().Length() + 
                                              KWFMapCacheBasePath().Length());
   tmpMapCacheDrivePath->Des().Copy( pathParser.Drive() );
   tmpMapCacheDrivePath->Des().Append( KWFMapCacheBasePath );
   HBufC* mapCacheDrivePath = SetupWritablePathL(*tmpMapCacheDrivePath);
   CleanupStack::PopAndDestroy(tmpMapCacheDrivePath);

   iCSIconPath = HBufC::NewL(iWritableDataPath->Length() + KCSIconPath().Length() + 
                             KPathTrailer().Length());
   iCSIconPath->Des().Copy(*iWritableDataPath);
   iCSIconPath->Des().Append(KCSIconPath);
   iCSIconPath->Des().Append(KPathTrailer);

#if defined NAV2_CLIENT_UIQ3 
   iWayfinderPath = HBufC::NewL( iDataPath->Des().Length() );
   iWayfinderPath->Des().Copy( *iDataPath );

   iMbmName = HBufC::NewL( pathParser.Drive().Length() + 
                           KResourcePath().Length() +
                           iApplicationName->Length() +
                           KPathTrailer().Length() +
                           KWayfinderMbm().Length() );
   iMbmName->Des().Copy( pathParser.Drive() );
   iMbmName->Des().Append( KResourcePath );
   iMbmName->Des().Append( *iApplicationName );
   iMbmName->Des().Append( KPathTrailer );
   iMbmName->Des().Append(KWayfinderMbm);

   iTurnsMbmName = HBufC::NewL( pathParser.Drive().Length() + 
                                KResourcePath().Length() +
                                iApplicationName->Length() +
                                KPathTrailer().Length() +
                                KTurnsMbmFile().Length() );
   iTurnsMbmName->Des().Copy( pathParser.Drive() );
   iTurnsMbmName->Des().Append( KResourcePath );
   iTurnsMbmName->Des().Append( *iApplicationName );
   iTurnsMbmName->Des().Append( KPathTrailer );
   iTurnsMbmName->Des().Append( KTurnsMbmFile );

   iLogoMbmName = HBufC::NewL( pathParser.Drive().Length() + 
                               KResourcePath().Length() +
                               aLogoMbmFileName.Length() );
   iLogoMbmName->Des().Copy( pathParser.Drive() );
   iLogoMbmName->Des().Append( KResourcePath );
   iLogoMbmName->Des().Append( aLogoMbmFileName );

   iResourceBasePath = HBufC::NewL( pathParser.Drive().Length() + 
                                    KResourcePath().Length() );
   iResourceBasePath->Des().Copy( pathParser.Drive() );
   iResourceBasePath->Des().Append( KResourcePath );

#elif defined NAV2_CLIENT_SERIES60_V3
   iWayfinderPath = HBufC::NewL( iDataPath->Des().Length() );
   iWayfinderPath->Des().Copy( *iDataPath );

   // The mbm and mif file is located in a subdirectory
   // named after the application name.
   iMbmName = HBufC::NewL( pathParser.Drive().Length() + 
                           KResourcePath().Length() +
                           iApplicationName->Length() +
                           KPathTrailer().Length() +
                           KWayfinderMbm().Length() );
   iMbmName->Des().Copy( pathParser.Drive() );
   iMbmName->Des().Append( KResourcePath );
   iMbmName->Des().Append( *iApplicationName );
   iMbmName->Des().Append( KPathTrailer );
   iMbmName->Des().Append(KWayfinderMbm);

   iResourceBasePath = HBufC::NewL( pathParser.Drive().Length() + 
                                    KResourcePath().Length() );
   iResourceBasePath->Des().Copy( pathParser.Drive() );
   iResourceBasePath->Des().Append( KResourcePath );

#else
   iWayfinderPath = pathParser.DriveAndPath().AllocL();
# if defined __WINSCW__
   iWayfinderPath->Des()[0] = 'c';
# endif
   iMbmName = HBufC::NewL( pathParser.DriveAndPath().Length() +
                           KWayfinderMbm().Length() );
   iMbmName->Des().Copy(pathParser.DriveAndPath());
   iMbmName->Des().Append(KWayfinderMbm);

   iTurnsMbmName = HBufC::NewL( pathParser.DriveAndPath().Length() +
                                KTurnsMbmFile().Length() );
   iTurnsMbmName->Des().Copy(pathParser.DriveAndPath());
   iTurnsMbmName->Des().Append(KTurnsMbmFile);

   iLogoMbmName = HBufC::NewL( pathParser.DriveAndPath().Length() + 
                               aLogoMbmFileName.Length() );
   iLogoMbmName->Des().Copy( pathParser.DriveAndPath() );
   iLogoMbmName->Des().Append( aLogoMbmFileName );

   iResourceBasePath = HBufC::NewL( pathParser.DriveAndPath().Length() );
   iResourceBasePath->Des().Copy( pathParser.DriveAndPath() );
#endif

   iResourceNoExt = HBufC::NewL( iResourceBasePath->Length() + 
                                 iApplicationName->Length() );
   iResourceNoExt->Des().Copy( *iResourceBasePath );
   iResourceNoExt->Des().Append( *iApplicationName );

   if (!iApplicationDllName) {
      iApplicationDllName = KNullDesC().AllocL();
   }
   if (!iApplicationName) {
      iApplicationName = KNullDesC().AllocL();
   }
   if (!iWayfinderPath) {
      iWayfinderPath = KNullDesC().AllocL();
   }
   if (!iDataPath) {
      iDataPath = KNullDesC().AllocL();
   }
   if (!iWritableDataPath) {
      iWritableDataPath = KNullDesC().AllocL();
   }
   if (!iCommonDataPath) {
      iCommonDataPath = KNullDesC().AllocL();
   }
   if (!iWritableCommonDataPath) {
      iWritableCommonDataPath = KNullDesC().AllocL();
   }
   if (!iLangResourcePath) {
      iLangResourcePath = KNullDesC().AllocL();
   }
   if (!iSysDataBasePath) {
      iSysDataBasePath = KNullDesC().AllocL();
   }
   if (!iAudioBasePath) {
      iAudioBasePath = KNullDesC().AllocL();
   }
   if (!iMbmName) {
      iMbmName = KNullDesC().AllocL();
   }
   if (!iTurnsMbmName) {
      iTurnsMbmName = KNullDesC().AllocL();
   }
   if (!iLogoMbmName) {
      iLogoMbmName = KNullDesC().AllocL();
   }
   if (!iMapCacheBasePath) {
      iMapCacheBasePath = KNullDesC().AllocL();
   }
   if (!iWritableAutoMapCachePath) {
      iWritableAutoMapCachePath = KNullDesC().AllocL();
   }
   if (!iLangSyntaxPath) {
      iLangSyntaxPath = KNullDesC().AllocL();
   }
   if (!iCSIconPath) {
      iCSIconPath = KNullDesC().AllocL();
   }
   iFs.MkDirAll(*iDataPath);
   iFs.MkDirAll(*iCommonDataPath);
   iFs.MkDirAll(*iCSIconPath);
   iFs.MkDirAll(*mapCacheDrivePath);
   delete mapCacheDrivePath;
   if (iDataPath->CompareF(*iWritableDataPath) != 0) {
      // If the writabel data path is the same as the data path there
      // is no need to create the dir
      iFs.MkDirAll(*iWritableDataPath);
   }
   if (iCommonDataPath->CompareF(*iWritableCommonDataPath) != 0) {
      // If the writabel common data path is the same as the common 
      // data path there is no need to create the dir
      iFs.MkDirAll(*iWritableCommonDataPath);
   }
   iFs.Close();
}

HBufC* CPathFinder::SetupWritablePathL(const TDesC& aBasePath)
{
   HBufC* writablePathPtr = HBufC::NewL(256);
   TPtr writablePath = writablePathPtr->Des();
   writablePath.Copy(aBasePath);
   TChar drive(aBasePath[0]);
   TInt driveNbr;
   TInt res = iFs.CharToDrive(drive, driveNbr);
   if (res != KErrNone) {
      // Could not get the drive number based on the drive letter
      return writablePathPtr;
   }
   TDriveInfo driveInfo;
   res = iFs.Drive(driveInfo, driveNbr);
   if (res != KErrNone) {
      // Not able to extract information about the drive
      return writablePathPtr;
   }
   if (driveInfo.iMediaAtt == KMediaAttWriteProtected) {
      // The drive is write protected, change to c:
      writablePath[0] = 'c';
   }
   return writablePathPtr;
}

TBool CPathFinder::CheckForAdditionalVoicesL(TDes& aVoicePath, CLanguageList& aLangList)
{
   // Get the correct symbian code for the current phone language
   TBuf<4> langCode;
   TBuf<4> langName;
   if (iUseRomBuild) {
      TLanguage sysLang = User::Language();
      CLanguageList::TWfLanguageList& langListElem =
         aLangList.FindBySymbianCode(sysLang);
      if (langListElem.symbianCode < 10) {
         langCode.Num(0);
      }
      langCode.AppendNum(langListElem.symbianCode); 
      langName.Copy(langListElem.symbianname);
   } else {
      CLanguageList::TWfLanguageList& langListElem =
         aLangList.FindBySymbianName(iLangCode);
      if (langListElem.symbianCode < 10) {
         langCode.Num(0);
      }
      langCode.AppendNum(langListElem.symbianCode); 
      langName.Copy(langListElem.symbianname);
   }

   TDriveList driveList;
   iFs.DriveList(driveList);
   TChar driveLetter; 
   for(TInt i = EDriveA; i <= EDriveZ; i++) {
      // Iterate through all drives, search on drives that
      // isnt rom or ram for syntax files that might match
      // the current language.
      if (!driveList[i]) {
         // Next drive
         continue;
      }
      TDriveInfo driveInfo;
      // Get the info about the drive
      iFs.Drive(driveInfo, i);
      if (driveInfo.iType == EMediaHardDisk || driveInfo.iType == EMediaFlash || 
          driveInfo.iType == EMediaNANDFlash || driveInfo.iType == EMediaRam) {
         // If the drive is hard disk (probably c drive) for flash/nandflash 
         // (memory card) we should look for a syntax file that matches the
         // phone language.

         // Convert the drive to a char
         TChar driveLetter;
         iFs.DriveToChar(i, driveLetter);

         // Put together a complete path containing drive and filename
         // and use BaflUtils to see if the file exists.
         TFileName synPathAndName;
         synPathAndName.Append(driveLetter);
         synPathAndName.Append(_L(":"));
         synPathAndName.Append(aVoicePath);
         synPathAndName.Append(KSyntaxPartName);
         synPathAndName.Append(langCode);
         if (BaflUtils::FileExists(iFs, synPathAndName)) {
            // Found a matching syntax file on current drive. Set aVoicePath
            // to contain the path to this file and return true.
            aVoicePath = synPathAndName;
            if (iUseRomBuild) {
               // If rom build set iLangCode to the phone language.
               // This should not be done for an ordinary installation
               // since the user are not bound to have Wayfinder on 
               // the phone language.
               iLangCode = langName;
            }
            return ETrue;
         }
      }
   }
   return EFalse;      
}

TPtrC CPathFinder::GetApplicationDllName() const
{
   return TPtrC(*iApplicationDllName);
}

TPtrC CPathFinder::GetApplicationName() const
{
   return TPtrC(*iApplicationName);
}

TPtrC CPathFinder::GetWayfinderPath() const
{
   return TPtrC(*iWayfinderPath);
}

TPtrC CPathFinder::GetDataPath() const
{
   return TPtrC(*iDataPath);
}

TPtrC CPathFinder::GetWritableDataPath() const
{
   return TPtrC(*iWritableDataPath);
}

TPtrC CPathFinder::GetCommonDataPath() const
{
   return TPtrC(*iCommonDataPath);
}

TPtrC CPathFinder::GetWritableCommonDataPath() const
{
   return TPtrC(*iWritableCommonDataPath);
}

TPtrC CPathFinder::GetLangResourcePath() const
{
   return TPtrC(*iLangResourcePath);
}

TPtrC CPathFinder::GetLangSyntaxPath() const
{
//    if (iUseRomBuild) {
//       // Rom build, use the new path to the syntax file
//       return TPtrC(*iSyntaxPathAndName);
//    } else {
      return TPtrC(*iLangSyntaxPath);
//    }
}

TPtrC CPathFinder::GetSysDataBasePath() const
{
   return TPtrC(*iSysDataBasePath);
}

TPtrC CPathFinder::GetAudioBasePath() const
{
   return TPtrC(*iAudioBasePath);
}

TPtrC CPathFinder::GetMbmName() const
{
   return TPtrC(*iMbmName);
}

TPtrC CPathFinder::GetTurnsMbmName() const
{
   return TPtrC(*iTurnsMbmName);
}

TPtrC CPathFinder::GetLogoMbmName() const
{
   return TPtrC(*iLogoMbmName);
}

TPtrC CPathFinder::GetMapCacheBasePath() const
{
   return TPtrC(*iMapCacheBasePath);
}

TPtrC CPathFinder::GetWritableAutoMapCachePath() const
{
   return TPtrC(*iWritableAutoMapCachePath);
}

TPtrC CPathFinder::GetResourceBasePath() const
{
   return TPtrC(*iResourceBasePath);
}

TPtrC CPathFinder::GetResourceNoExt() const
{
   return TPtrC(*iResourceNoExt);
}

TPtrC CPathFinder::GetCSIconPath() const
{
   return TPtrC(*iCSIconPath);
}

