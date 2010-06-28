/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PATHFINDER_H
#define PATHFINDER_H

#include <e32base.h>
#include <f32file.h>
#include "LanguageList.h"

class CPathFinder : public CBase
{
private:
   CPathFinder(class CEikAppUi* aAppUi, 
               const TDesC& aLangCode, 
               const TDesC& aAudioPrefix, 
               /*class RFs& aFsSession,*/
               TBool aUseRomBuild);
   void ConstructL(
         const TDesC& aLogoMbmFileName,
         const TDesC& aWfVoiceDir,
         const TInt aLangListResourceId);
   
public:
   ~CPathFinder();
   static class CPathFinder* NewL(class CEikAppUi* aAppUi, 
         const TDesC& aLangCode, 
         const TDesC& aAudioPrefix,
         const TDesC& aLogoMbmFileName,
         const TDesC& aWfVoiceDir,
         const TInt aLangListResourceId,
         TBool aUseRomBuild = EFalse);
   static class CPathFinder* NewLC(class CEikAppUi* aAppUi, 
         const TDesC& aLangCode, 
         const TDesC& aAudioPrefix,
         const TDesC& aLogoMbmFileName,
         const TDesC& aWfVoiceDir,
         const TInt aLangListResourceId,
         TBool aUseRomBuild = EFalse);
   
   TPtrC GetApplicationDllName() const;
   TPtrC GetApplicationName() const;
   TPtrC GetWayfinderPath() const;
   TPtrC GetDataPath() const;

   /**
    * Returns the data path that is guaranteed to be be writable
    * @return The writable data path
    */
   TPtrC GetWritableDataPath() const;

   TPtrC GetCommonDataPath() const;

   /**
    * Returns the common data path that is guaranteed to be be writable
    * @return The writable common data path
    */
   TPtrC GetWritableCommonDataPath() const;

   TPtrC GetLangResourcePath() const;
   TPtrC GetLangSyntaxPath() const;
   TPtrC GetSysDataBasePath() const;
   TPtrC GetAudioBasePath() const;
   TPtrC GetMbmName() const;
   TPtrC GetTurnsMbmName() const;
   TPtrC GetLogoMbmName() const;
   TPtrC GetMapCacheBasePath() const;
   TPtrC GetWritableAutoMapCachePath() const;
   TPtrC GetResourceBasePath() const;
   TPtrC GetResourceNoExt() const;
   TPtrC GetCSIconPath() const;

private:
   void SetupPathsL(
         const TDesC& aLogoMbmFileName,
         const TDesC& aWfVoiceDir,
         TInt aLangListResourceId);

   /**
    * Sets up the writabel paths based on an already existing
    * path. If the existing path has a drive that is writable
    * then a copy of the existing path is done and returned. 
    * If the path has a drive that is read only then the new
    * path is a copy of the existing path, but the drive
    * is changed to c
    * @param The path to base the new writable path on.
    * @return A guaranteed writable path
    */
   HBufC* SetupWritablePathL(const TDesC& aBasePath);

   /**
    * Looks for voice files on all the writable paths to see
    * if there might exist voice packs for current language.
    * @param aVoicePath, path to where voices is supposed to be located.
    *                    Without drive and filename.
    * @param aLangList, a reference to a CLanguageList.
    * @return ETrue if a voice pack is found on any drive.
    * @return EFalse if no found.
    */
   TBool CheckForAdditionalVoicesL(TDes& aVoicePath, CLanguageList& aLangList);

   class CEikAppUi* iAppUi;
   class RFs iFs;

   TBufC<4> iLangCode;
   TBufC<16> iAudioPrefix;

   /** Full path and name of the wayfinder
    * application dll. */
   HBufC* iApplicationDllName;

   /** Only the name (without extension) of the wayfinder
    * application dll. */
   HBufC* iApplicationName;

   HBufC* iApplicationNameBase;
   
   /** Path (including devicename) where the wayfinder 
    * application is installed. */
   HBufC* iWayfinderPath;

   /** Path (including devicename) where the wayfinder 
    * specific branded data can be found. */
   HBufC* iDataPath;

   /** Path (including devicename) where the wayfinder 
    * specific branded data can be found. The difference
    * form the above path is that this path is guaranted 
    * to be writable */
   HBufC* iWritableDataPath;

   /** Path (including devicename) where the wayfinder 
    * common data can be found (without arch tag). */
   HBufC* iCommonDataPath;

   /** Path (including devicename) where the wayfinder 
    * common data can be found. The difference
    * form the above path is that this path is guaranted 
    * to be writable */
   HBufC* iWritableCommonDataPath;

   /** Path (including devicename) where the language dependant 
    * resource files are installed. */
   HBufC* iLangResourcePath;

   /** Path (including devicename) where the language syntax 
    * file are installed. */
   HBufC* iLangSyntaxPath;

   /** Data base path (excluding devicename) where the apps
    * data files are stored. */
   HBufC* iSysDataBasePath;

   /** Base path (excluding devicename) where the language 
    * dependant resource files are installed. */
   HBufC* iAudioBasePath;

   /** Full Path and name of the mbm file for the application. */
   HBufC* iMbmName;

   /** Full Path and name of the turns mbm file
    * currently only used by the UIQ application. */
   HBufC* iTurnsMbmName;

   /** Full Path and name of the logo mbm file
    * currently only used by the UIQ application. */
   HBufC* iLogoMbmName;

   /** Base path (excluding devicename) where the single
    * map file cache (.wfd) are installed. */
   HBufC* iMapCacheBasePath;
   
   HBufC* iWritableAutoMapCachePath;

   /** Base path where the resource (rsc)
    * files are located. */
   HBufC* iResourceBasePath;

   /** Path and filename where the resource (rsc)
    * files are located. No extension. */
   HBufC* iResourceNoExt;

   /** Path for the combined search icons 
    */
   HBufC* iCSIconPath;

//    HBufC* iSyntaxPathAndName;
   TBool iUseRomBuild;
};

#endif /* PATHFINDER_H */
