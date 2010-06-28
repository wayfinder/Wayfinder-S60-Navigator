/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_STARTUP_EVENT_ENUM_H
#define WF_STARTUP_EVENT_ENUM_H

enum StartupParameterType {
   SP_None                    = 0x00,
   SP_FavoriteSyncFailed      = 0x01,
   SP_FavoriteSyncError       = 0x02,
   SP_UsernameFailed          = 0x03,
   SP_NoNetwork               = 0x04,
   SP_Upgrade                 = 0x05,
   SP_UpgradeFailed           = 0x06,
   SP_ExpiredUser             = 0x07,
   SP_ChangeUin               = 0x08,
   SP_SetServerList           = 0x09,
};

enum StartupState {
   SS_Start                   = 0x0,   /* First event. */
   SS_CheckFlightMode         = 0x1,
   SS_CheckFlightModeOk       = 0x2,
   SS_GetWFType               = 0x3,   /* Sends request for wayfinder type. */
   SS_WFTypeReceived          = 0x4,   /* Wayfinder type has been received. */
   SS_InternetUsagePopup      = 0x5,
   SS_GetIap                  = 0x6,   /* Get the IAP parameter. */
   SS_IapReceived             = 0x7,   /* IAP parameter has been received. */
   SS_IapOk                   = 0x8,   /* IAP parameter is ok. */
   SS_IapSearch               = 0x9,   /* Start IAP search. */
   SS_IapSearchOk             = 0xa,   /* IAP search worked. */
   SS_IapSearchNotOk          = 0xb,   /* IAP search failed. */
   SS_IapSearchOkButNotReally = 0xc,   /* IAP search ok but something failed. */
   SS_IapContinueAnyway       = 0xd,   /* Continue even without working IAP. */
   SS_SendFavoriteSync        = 0xe,   /* Send favorite sync message. */
   SS_FavoriteSyncReceived    = 0xf,   /* Favorite sync message received. */
   SS_GetParams               = 0x10,  /* Get all parameters needed */
   SS_AudioScriptsInit        = 0x11,  /* Send message to init audio scripts. */
   SS_GetUserTermsParam       = 0x12,  /* Get parameter for User Terms. */
   SS_StartupFailed           = 0x13,
   SS_IapSearchRestart        = 0x14,  /* Restart the IAP search. */
   SS_GetUsername             = 0x15,
   SS_SendUsername            = 0x16,  
   SS_UsernameReceived        = 0x17,
   SS_UsernameNotReceived     = 0x18,
   SS_StartupError            = 0x19,  /* Something went wrong, will trigger SS_StartupErrorReal */
   SS_StartupErrorReal        = 0x1a,  /* Real event for error dialog. */
   SS_SendParamSync           = 0x1b,  /* Send param sync message. */
   SS_ParamSyncReceived       = 0x1c,  /* Param sync message received. */

   SS_UserTerms               = 0x20,  /* Trigger event. */
   SS_UserTermsNeeded         = 0x21,  /* Or event */
   SS_UserTermsNotNeeded      = 0x22,  /* Or event */
   SS_ShowUserTerms           = 0x23,  /* Result of SS_UserTermsNeeded */
   SS_UserTermsAccepted       = 0x24,  /* Sent from SW when terms accepted */
   SS_UserTermsNotAccepted    = 0x25,  /* Sent from SW when terms rejected */
   SS_UserTermsDone           = 0x26,  /* All user terms work complete */

   SS_ShowTrialView           = 0x30,
   SS_ShowRegistration        = 0x31,
   SS_ShowActivation          = 0x32,
   SS_ShowReactivation        = 0x33,
   SS_ShowActivationCongrats  = 0x34,

   SS_USDisclaimerTest        = 0x40,  /* Trigger event. */
   SS_USDisclaimer            = 0x41,  /* Trigger event. */
   SS_USDisclaimerNeeded      = 0x42,  /* Need to show US disclaimer. */
   SS_USDisclaimerNotNeeded   = 0x43,  /* No need to show US disclaimer. */
   SS_ShowUsDisclaimer        = 0x44,  /* Show US disclaimer in SW. */
   SS_USDisclaimerAccept      = 0x45,  /* US disclaimer accepted. */
   SS_USDisclaimerReject      = 0x46,  /* US disclaimer rejected. */
   SS_USDisclaimerDoTest      = 0x47,  /* Do show US disclaimer again. */
   SS_USDisclaimerDontTest    = 0x48,  /* Don't show US disclaimer again. */
   SS_USDisclaimerNeverShow   = 0x49,  /* Save don't show US disclaimer. */

   SS_AccountExpired          = 0x50,  /* Account has expired. */
   SS_AccountNotExpired       = 0x51,  /* Account not expired. */
   SS_ExpiredTest             = 0x52,  /* Trigger for expired test. */
   SS_ShowExpired             = 0x53,  /* Show expired page in SW. */

   SS_Upgrade                 = 0x54,  /* Start upgrading */
   SS_UpgradeOk               = 0x55,  /* Upgrade okay */
   SS_UpgradeFailed           = 0x56,  /* Upgrade failed */
   SS_UpgradeOkNotStartup     = 0x57,  /* Upgrade okay, outside startup */
   SS_UpgradeOkStartup        = 0x58,  /* Upgrade okay, startup */
   SS_UpgradeChoices          = 0x59,  /* Must choose region, then retry. */

   SS_TrialMode               = 0x60,  /* Wayfinder mode is trial. */
   SS_SilverMode              = 0x61,  /* Wayfinder mode is silver. */
   SS_GoldMode                = 0x62,  /* Wayfinder mode is gold. */
   SS_IronMode                = 0x63,  /* Wayfinder mode is iron (earth). */
   SS_ModeTest                = 0x64,  /* Trigger for mode test. */
   SS_StartTrial              = 0x65,  /* Start in trial mode. */
   SS_ShowTrialMenu           = 0x66,  /* Show trial menu in SW */
   SS_TrialRun                = 0x67,  /* Trial mode selected in SW. */
   SS_Activated               = 0x68,  /* User activated in SW. */
   SS_Reactivated             = 0x69,  /* User reactivated in SW. */
   SS_StartSilver             = 0x6a,  /* Start in silver mode. */
   SS_StartGold               = 0x6b,  /* Start in gold mode. */
   SS_StartIron               = 0x6c,  /* Start in iron mode. */
   SS_WFModeDone              = 0x6d,  /* All WFmode events done. */

   SS_RetryUpgrade            = 0x6e,  /* Retry an upgrade after UpgradeChoices. */

   SS_ShowNewsTest            = 0x70,  /* Trigger for show news test. */
   SS_ShowNewsStartup         = 0x71,  /* Show news, if any. */
   SS_NewsNeeded              = 0x72,  /* News should be shown. */
   SS_NewsNotNeeded           = 0x73,  /* News should not be shown. */
   SS_ShowNewsComplete        = 0x74,  /* Sent from SW when news shown. */
   SS_ShowNews                = 0x75,  /* Show news always. */

   SS_SendRegistrationSmsTest       = 0x76,  /* Trigger for test if reg sms is sent */
   SS_SendRegistrationSmsNeeded     = 0x77,  /* Reg sms dialog should be shown */
   SS_SendRegistrationSmsNotNeeded  = 0x78,  /* Dont show reg sms dialog */
   SS_ShowRegistrationSmsStartup    = 0x79,  /* Show reg sms dialog */
   SS_ActivateSplashView            = 0x7a,  /* Activate splash screen */
   SS_SplashViewActivated           = 0x7b,  /* Splash screen was activated */
   SS_SendNopTest                   = 0x7c,  /* Send nop to server to see if iap worked */

   SS_ChangeUin               = 0x80,  /* Change the UIN */
   SS_NewServerList           = 0x85,  /* Change the server list */

   SS_StartupComplete         = 0xef,  /* Wayfinder Id service window startup if finnished */
   SS_StartupFinished         = 0xf0,  /* Startup finished */
   SS_GotoMainMenu            = 0xf1,  /* Start application */
   SS_Exit                    = 0xf2,  /* Exit application */
   SS_ShowUpgradePage         = 0xf3,  /* Show upgrade page */
   SS_UpgradeFinished         = 0xf4,  /* Upgrade finished */
   SS_ClearBrowserCache       = 0xf5,  /* Clear the browser cache. */
};

#endif
