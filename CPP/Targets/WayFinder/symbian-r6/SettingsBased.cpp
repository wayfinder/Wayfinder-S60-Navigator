/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikappui.h> 
#include <eikapp.h>
#include <bautils.h>

#include "WayFinderSettings.h"
#include "WayFinderAppUi.h"
#include "Dialogs.h"
#include "RsgInclude.h"
#include "memlog.h"
#include "IniFile.h"
#include "ResourceFileName.h"
#include "TDesCHolder.h"

#ifdef USE_LMPRO
    /* Create compatibility macros. Hack. */
# define MAKMObserver      MLCSObserver
# define TAKMStartupType   TStartupMode
# define EAKMStartupFull   EModeNormal
# define EAKMStartupTrial  EModeTrial
# define DoStartProgram    DoStartProgramL
# define CAKMClientWrapper CTrialWrapper
#endif




#ifdef USE_AKM_20040428
# include </usr/local/packages/AKMSeries60_20040428/sources/inc/akmwrapper.h>
#endif

#ifdef USE_AKM_20040602_EMEA
# include </usr/local/packages/AKMSeries60_20040602_EMEA/sources/inc/akmwrapper.h>
#endif

#ifdef USE_LMPRO
# include <lmclient.h>
# include <trialwrapper.h>
#endif

// This is the default for symbian applications (we will use bafl utils).
#ifdef ONLY_PHONE_LANG_ROM_BUILD
# undef IF_SPECIAL_USE_PREF_FILE
#else
# if defined (USE_ONLY_ONE_LANGUAGE)
// This means that the else of the ifdef below is hit,
// and the .rsc file is used instead of any other.
#  undef IF_SPECIAL_USE_PREF_FILE
# else
// This means that the application tries to read the value
// in the lang.txt file and use that to determine which
// language resource file to use (.r01 .r02 etc)
#  define IF_SPECIAL_USE_PREF_FILE
# endif
#endif

#if defined(MAPCENTERLAT) && defined(MAPCENTERLON)
const TInt32 DefaultVectorMapCenterLat = MAPCENTERLAT;
const TInt32 DefaultVectorMapCenterLon = MAPCENTERLON;
#elif VECTOR_MAP_CENTER == ARC_DE_TRIOMPHE 
const TInt32 DefaultVectorMapCenterLat = 85300832;
const TInt32 DefaultVectorMapCenterLon = 4006333;
#elif VECTOR_MAP_CENTER == WASHINGTON_MEMORIAL 
const TInt32 DefaultVectorMapCenterLat = 67874926;
const TInt32 DefaultVectorMapCenterLon = -134451858;
#elif VECTOR_MAP_CENTER == BRANDENBURGER_TOR 
const TInt32 DefaultVectorMapCenterLat = 91658141;
const TInt32 DefaultVectorMapCenterLon = 23348338;
#else
# error No default vector map center defined
#endif

// XXX: Copyright string hack for si.mobil:
bool hardcodeMonolitCopyrightString()
{
#ifdef RELEASE_SIMOBIL_SLOVENIA
 return true;
#else
 return false;
#endif
}
// XXX: Copyright string hack for airtel:
bool hardcodeMapmyIndiaCopyrightString()
{
#if defined RELEASE_AIRTEL || defined MAPMYINDIA_RESOURCES
   return true;
#else
   return false;
#endif
}

#if defined(USE_AKM_20040428) || defined(USE_AKM_20040602_EMEA)
/***** 
 * Local support class to handle AKM
 *****/
class CWfAkmObserver : MAKMObserver
{
   public:
   CWfAkmObserver(CWayFinderAppUi &wfappui) : iWfAppUi(wfappui) { };
   virtual ~CWfAkmObserver();
   void ConstructL();
   
   /// This function is mandatory in programs using Openbit's
   /// Application Key manager - AKM. To make development easier the
   /// function is included for all architectures, but called from
   /// different places.
   virtual void DoStartProgram(TAKMStartupType aStartupType);

   void StartAKM(const TDesC &id);

   private:
   CWayFinderAppUi &iWfAppUi;
   
   /// The OpenBit Application Key Manager
   class CAKMClientWrapper*                       iAKMWrapper;
};

CWfAkmObserver::~CWfAkmObserver()
{
   delete iAKMWrapper;
   //LOGDEL(iAKMWrapper);
}

void CWfAkmObserver::DoStartProgram(TAKMStartupType aStartupType) {
   switch (aStartupType) {
      case EAKMStartupFull:
         iWfAppUi.DoStartProgram(CWayFinderAppUi::EWfStartupFull);
         break;
      case EAKMStartupTrial:
         iWfAppUi.DoStartProgram(CWayFinderAppUi::EWfStartupTrial);
         break;
      default:
         iWfAppUi.DoStartProgram(CWayFinderAppUi::EWfStartupTrial);
         break;
   }
}

void CWfAkmObserver::ConstructL()
{
   iAKMWrapper = CAKMClientWrapper::NewL();
   //LOGNEW(iAKMWrapper, CAKMClientWrapper);
}

void CWfAkmObserver::StartAKM(const TDesC &id)
{
#ifdef USE_AKM_TRIAL
   iAKMWrapper->Interface()->StartApplicationKeyManagerL(this, id, EAkmManagedTrial);
#else
   iAKMWrapper->Interface()->StartApplicationKeyManagerL(this, id, EApplicationManagedTrial);
#endif
}

#endif /* USE_AKM_20040602_EMEA || USE_AKM_20040428 */


void CWayFinderAppUi::ConstructL()
{
#if defined NAV2_CLIENT_SERIES60_V28 
   BaseConstructL( EAknEnableSkinFlag );
#elif defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   BaseConstructL( EAknEnableSkin );
#else
   BaseConstructL();
#endif

   User::Free(User::AllocL(600*1024));
   if(!CheckMem(600*1024)){
      WFDialog::ShowErrorDialogL(R_WAYFINDER_MEMORY_LOW_EXIT_MSG, iCoeEnv);
      User::Exit(KErrNoMemory);
   }

   _LIT(KResourcePath, "\\resource\\apps\\");
   _LIT(KLangListResourceFileEnding, "_ll.rsc");
   TParse parser;
   parser.Set(Application()->DllName(), NULL, NULL);
   HBufC* rscName = AllocLC(parser.Drive() + 
                            KResourcePath + 
                            parser.Name() + 
                            KLangListResourceFileEnding);

   TFileName rscFileName = rscName->Des();
   RFs& fileServer = iCoeEnv->FsSession();
   BaflUtils::NearestLanguageFile(fileServer, rscFileName);
#if defined ONLY_PHONE_LANG_ROM_BUILD
   // we need to do this above and hence we dont need to do it again.
   //BaflUtils::NearestLanguageFile(iFileServer, rscFileName);
#elif defined IF_SPECIAL_USE_PREF_FILE
   HBufC* tmpWFDir = parser.Name().AllocLC();
   rscFileName = FindResourceFileName(rscFileName, tmpWFDir);
   CleanupStack::PopAndDestroy(tmpWFDir);
#else
   rscFileName = RscResourceFileName(rscFileName);
#endif
   iLanguageListRsc = iCoeEnv->AddResourceFileL(rscFileName);
   CleanupStack::PopAndDestroy(rscName);

   UpdateLangCodesL();

   /* Find all paths needed for the program to run. */
   InitPathsL();
   CheckResources();

   /* Read the ini-file settings. */
   iIniFile->ReadL();
#if defined(USE_AKM_20040428) || defined(USE_AKM_20040602_EMEA) || defined(USE_LMPRO)
      /* Call the AKM, let it decide if we're in the */
   /* correct mode. */
   iWfAkmObserver = new(ELeave) CWfAkmObserver(*this);
   CleanupStack::PushL(iWfAkmObserver);
   LOGNEW(iWfAkmObserver, CWfAkmObserver);
   iWfAkmObserver->ConstructL();

   ///Application ID for the wayfinder in OpenBit's License Manager
# if defined(USE_LMPRO)
   _LIT(KAID, "0010110");
# elif defined(USE_AKM_20040428)
   _LIT(KAID, "0050002");
# elif defined(USE_AKM_20040602_EMEA)
   _LIT(KAID, "02P0002");
# else
#  error "Unknown akm version, ckech the defines in WayFinderSettings"
# endif
   iWfAkmObserver->StartAKM(KAID);
   CleanupStack::Pop(iWfAkmObserver);

#ifdef HIDE_TRIAL_VIEW
   iStartupControl |= ENoTrial;
#endif
#elif defined(BRONZE_TRIAL)   /* not any of the akm versions */
   DoStartProgram(EWfStartupTrial);
#else                         /* nor trial support of any kind */
   DoStartProgram(EWfStartupFull);
#endif

}

