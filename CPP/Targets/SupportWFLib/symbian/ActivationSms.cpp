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

#include <eikenv.h>
#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V1
# include <aknmessagequerydialog.h> 
# include <sysutil.h>
# include <plpvariant.h>
#elif defined NAV2_CLIENT_UIQ
# include <plpvariant.h>
#elif defined SYMBIAN_CKON || defined SYMBIAN_9
# include <etel3rdparty.h>
#else
# error check this, include correct files.
#endif

#include "GuiProt/ServerEnums.h"
#include "ActivationSms.h"
#include "WFTextUtil.h"

HBufC*
ActivationSms::FormatLC(isab::GuiProtEnums::WayfinderType aWfType, 
                        const TDesC &aClientType, 
                        const TDesC &aClientOption,
                        TInt aMajor, TInt aMinor, TInt aBuild,
                        TInt aLangCodeResId, char* aImeiNbr)
{
   //   _LIT(KSMSFormat, "\\WAYFS%S;%S;%S;%S;%d.%d.%d;%S;%S");

   HBufC* sms = HBufC::NewLC(150);
   sms->Des().Copy( _L("//WAYFS") );
#if defined __WINS__ || defined __WINSCW__
   _LIT(imeNum, "EMULATOR");
   sms->Des().Append( imeNum );
   sms->Des().Append( ';' );
#else
   if (!aImeiNbr) {
      // aImeiNbr is NULL, we need to get it ourself.
# if ! (defined SYMBIAN_CKON || defined SYMBIAN_9)
      TPlpVariantMachineId imeNum( _L("") );
      PlpVariant::GetMachineIdL(imeNum);
      sms->Des().Append( imeNum );
      sms->Des().Append( ';' );
# elif ! (defined NAV2_CLIENT_SERIES60_V3)
      CTelephony::TPhoneIdV1 v1;
      TPtrC imeNum(v1.iSerialNumber);
      {
         CTelephony::TPhoneIdV1Pckg pkg(v1);
         class CTelephony* telephony = CTelephony::NewLC();
         class TRequestStatus imeiStatus;
         telephony->GetPhoneId(imeiStatus, pkg);
         User::WaitForRequest( imeiStatus );
         User::LeaveIfError(imeiStatus.Int());
         CleanupStack::PopAndDestroy(telephony);
         sms->Des().Append( imeNum );
         sms->Des().Append( ';' );
      }
# endif
   } else {
      HBufC* imei = WFTextUtil::AllocLC( aImeiNbr );
      sms->Des().Append( *imei );
      CleanupStack::PopAndDestroy(imei);
      sms->Des().Append( ';' );
   }
#endif
   HBufC* langcode = CEikonEnv::Static()->AllocReadResourceLC(aLangCodeResId);
   sms->Des().Append( *langcode );
   CleanupStack::PopAndDestroy(langcode);
   sms->Des().Append( ';' );

   _LIT(KUnknown, "Unknown");
# if defined (NAV2_CLIENT_SERIES60_V2) || defined (NAV2_CLIENT_SERIES60_V1)
   HBufC* swVerC = HBufC::NewLC(128);
   TPtr swVer = swVerC->Des();
   SysUtil::GetSWVersion(swVer);
# elif defined NAV2_CLIENT_UIQ
   _LIT(KUIQ, "UIQ");
   TPtrC swVer(KUIQ);
# elif defined NAV2_CLIENT_UIQ3
   _LIT(KUIQ, "UIQ3");
   TPtrC swVer(KUIQ);
# elif defined NAV2_CLIENT_SERIES80
   _LIT(KS80, "Series80");
   TPtrC swVer(KS80);
# elif defined NAV2_CLIENT_SERIES90_V1
   _LIT(KS90, "Series90");
   TPtrC swVer(KS90);
# elif defined NAV2_CLIENT_SERIES60_V3
   _LIT(KS60v3, "Series60v3");
   TPtrC swVer(KS60v3);
# else
   TPtrC swVer(KUnknown);
# endif
   sms->Des().Append( swVer );
# if defined (NAV2_CLIENT_SERIES60_V2) || defined (NAV2_CLIENT_SERIES60_V1)
   CleanupStack::PopAndDestroy(swVerC);
# endif

   sms->Des().Append( ';' );
   _LIT(KGold, "gold");
   _LIT(KTrial, "trial");
   _LIT(KSilver, "silver");
   _LIT(KIron, "iron");
   if (aWfType == isab::GuiProtEnums::Gold) {
      sms->Des().Append(KGold);
   } else if (aWfType == isab::GuiProtEnums::Trial) {
      sms->Des().Append(KTrial);
   } else if (aWfType == isab::GuiProtEnums::Silver) {
      sms->Des().Append(KSilver);
   } else if (aWfType == isab::GuiProtEnums::Iron) {
      sms->Des().Append(KIron);
   } else {
      sms->Des().Append(KUnknown);
   }
   sms->Des().Append( ';' );
   sms->Des().AppendNum(aMajor, EDecimal);
   sms->Des().Append( '.' );
   sms->Des().AppendNum(aMinor, EDecimal);
   sms->Des().Append( '.' );
   sms->Des().AppendNum(aBuild, EDecimal);
   sms->Des().Append( ';' );
   sms->Des().Append(aClientType);
   sms->Des().Append( ';' );
   sms->Des().Append(aClientOption);

   return sms;
}

