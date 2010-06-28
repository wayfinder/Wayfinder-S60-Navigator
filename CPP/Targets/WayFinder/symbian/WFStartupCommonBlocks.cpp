/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <e32std.h>

#include <list>
#include "WFStartupCommonBlocks.h"
#include "WFCommonStartupEvents.h"
#include "WFStartupEventEnum.h"
#include "WFStartupHandler.h"
#include "TraceMacros.h"
#include "BackActionEnum.h"
#include "WFDataHolder.h"


//void
//WFStartupCommonBlocks::UpgradePage(WFStartupHandler* startupHandler,
//      const char *languageIsoTwoChar,
//      WFServiceViewHandler* viewHandler,
//      GuiProtMessageSender* sender)
//{
//   startupHandler->RegisterStartupEvent(SS_ShowUpgradePage,
//         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//            WF_UPGRADE_URL, languageIsoTwoChar, BackIsHistoryThenView));
//
//   startupHandler->RegisterStartupEvent(SS_UpgradeFinished,
//         new (ELeave) WFStartupNextEvent(SS_GotoMainMenu));
//}
//
//void
//WFStartupCommonBlocks::UpgradeClient(WFStartupHandler* startupHandler,
//      const char *languageIsoTwoChar,
//      WFServiceViewHandler* viewHandler,
//      GuiProtMessageSender* sender)
//{
//   startupHandler->RegisterStartupEvent(SS_Upgrade,
//         new WFStartupUpgrade(sender));
//
//   startupHandler->RegisterStartupEvent(SS_UpgradeOkNotStartup,
//         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//            WF_UPGRADE_SUCCESS_URL, languageIsoTwoChar, BackIsHistoryThenView));
//
//   startupHandler->RegisterStartupEvent(SS_UpgradeOkStartup,
//         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//            WF_UPGRADE_SUCCESS_URL_STARTUP, languageIsoTwoChar,
//            BackIsHistoryThenView));
//
//   startupHandler->RegisterStartupEvent( SS_UpgradeOk,
//         new (ELeave) WFStartupUiCallback());
//
//   startupHandler->RegisterStartupEvent(SS_UpgradeFailed,
//         new (ELeave) WFStartupUiCallback());
//
//   WFStartupUpgradeChoose* wfUc = 
//      new (ELeave) WFStartupUpgradeChoose( 
//         sender, viewHandler, WF_UPGRADE_CHOICES_URL, languageIsoTwoChar );
//   startupHandler->RegisterStartupEvent( SS_UpgradeChoices, wfUc );
//   startupHandler->RegisterStartupEvent( SS_RetryUpgrade, wfUc );
//}

// void
// WFStartupCommonBlocks::ShowNews(WFStartupHandler* startupHandler,
//       int32 nextEvent, const char *languageIsoTwoChar,
//       WFServiceViewHandler* viewHandler,
//       WFNewsData* newsData,
//       GuiProtMessageSender* sender)
// {
//    std::list<int32> in;
//    in.push_back(SS_NewsNeeded);
//    in.push_back(SS_NewsNotNeeded);
//    std::list<int32> out;
//    out.push_back(SS_ShowNewsStartup);
//    out.push_back(nextEvent);
// /*    out.push_back(SS_ShowNewsStartup); */

//    WFStartupOrEvent *news =
//       new (ELeave) WFStartupOrEvent(SS_ShowNewsTest, in, out);

//    startupHandler->RegisterStartupEvent(SS_ShowNewsTest, news);
//    startupHandler->RegisterStartupEvent(SS_NewsNeeded, news);
//    startupHandler->RegisterStartupEvent(SS_NewsNotNeeded, news);

// //   startupHandler->RegisterStartupEvent(SS_ShowNewsStartup,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_SHOW_NEWS_URL, languageIsoTwoChar));
// //
// //   startupHandler->RegisterStartupEvent(SS_ShowNews,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_SHOW_NEWS_URL_NO_REFRESH, languageIsoTwoChar,
// //            BackIsHistoryThenView));

//    startupHandler->RegisterStartupEvent(SS_ShowNewsComplete,
//          new (ELeave) WFStartupLatestNewsShown(nextEvent, sender, newsData));
// }

// void
// WFStartupCommonBlocks::WFMode(WFStartupHandler* startupHandler,
//       int32 nextEvent, const char *languageIsoTwoChar,
//       WFServiceViewHandler* viewHandler)
// {
//    std::list<int32> in;
//    in.push_back(SS_TrialMode);
//    in.push_back(SS_SilverMode);
//    in.push_back(SS_GoldMode);
//    in.push_back(SS_IronMode);
//    std::list<int32> out;
//    out.push_back(SS_StartTrial);
//    out.push_back(SS_StartSilver);
//    out.push_back(SS_StartGold);
//    out.push_back(SS_StartIron);
//    WFStartupOrEvent *tmp =
//       new (ELeave) WFStartupOrEvent( SS_ModeTest, in, out );

//    startupHandler->RegisterStartupEvent( SS_ModeTest, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_TrialMode, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_SilverMode, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_GoldMode, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_IronMode, tmp, true);

//    startupHandler->RegisterStartupEvent(SS_StartTrial,
//          new (ELeave) WFStartupNextEvent(SS_ShowTrialMenu), true );

// //   startupHandler->RegisterStartupEvent(SS_ShowTrialMenu,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_TRIAL_MENU, languageIsoTwoChar), true );

//    startupHandler->RegisterStartupEvent(SS_Activated,
//          new (ELeave) WFStartupNextEvent(SS_TrialRun), true );
//    startupHandler->RegisterStartupEvent(SS_Reactivated,
//          new (ELeave) WFStartupNextEvent(SS_TrialRun), true );

//    startupHandler->RegisterStartupEvent(SS_TrialRun,
//          new (ELeave) WFStartupNextEvent(SS_WFModeDone), true );
//    startupHandler->RegisterStartupEvent(SS_StartSilver,
//          new (ELeave) WFStartupNextEvent(SS_WFModeDone), true );
//    startupHandler->RegisterStartupEvent(SS_StartGold,
//          new (ELeave) WFStartupNextEvent(SS_WFModeDone), true );

// //   startupHandler->RegisterStartupEvent(SS_StartIron,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_EARTH_MENU, languageIsoTwoChar), true );

//    startupHandler->RegisterStartupEvent(SS_WFModeDone,
//          new (ELeave) WFStartupNextEvent(nextEvent), true );
// }

// void
// WFStartupCommonBlocks::ExpiredTest(WFStartupHandler* startupHandler,
//       int32 nextEvent, const char *languageIsoTwoChar,
//       WFServiceViewHandler* viewHandler)
// {
       
//    // SS_AccountExpired          = 0x2a,  /* Account has expired. */
//    // SS_AccountNotExpired       = 0x2b,  /* Account not expired. */
//    // SS_ExpiredTest             = 0x2c,  /* Trigger for expired test. */
//    // SS_ShowExpired             = 0x2d,  /* Show expired page in SW. */

//    std::list<int32> in;
//    in.push_back(SS_AccountExpired);
//    in.push_back(SS_AccountNotExpired);
//    std::list<int32> out;
//    out.push_back(SS_ShowExpired);
//    out.push_back(nextEvent);
//    WFStartupOrEvent *tmp =
//       new (ELeave) WFStartupOrEvent(SS_ExpiredTest, in, out);

//    startupHandler->RegisterStartupEvent( SS_ExpiredTest, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_AccountExpired, tmp, true );
//    startupHandler->RegisterStartupEvent( SS_AccountNotExpired, tmp, true );

// //   startupHandler->RegisterStartupEvent(SS_ShowExpired,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_ACCOUNT_EXPIRED, languageIsoTwoChar), true );
// }

// void
// WFStartupCommonBlocks::CombinedUserTerms(WFStartupHandler* startupHandler,
//       int32 nextEvent, const char *languageIsoTwoChar,
//       WFServiceViewHandler* viewHandler,
//       GuiProtMessageSender* sender)
// {
//    {
//       std::list<int32> in;
//       in.push_back(SS_USDisclaimerNeeded);
//       in.push_back(SS_USDisclaimerNotNeeded);
//       std::list<int32> out;
//       out.push_back(SS_USDisclaimer);
//       out.push_back(SS_UserTerms);

//       WFStartupOrEvent *tmp =
//          new (ELeave) WFStartupOrEvent(SS_USDisclaimerTest, in, out);
//       startupHandler->RegisterStartupEvent(SS_USDisclaimerTest, tmp);
//       startupHandler->RegisterStartupEvent(SS_USDisclaimerNeeded, tmp);
//       startupHandler->RegisterStartupEvent(SS_USDisclaimerNotNeeded, tmp);
//    }
//    {
//       std::list<int32> in;
//       in.push_back(SS_UserTermsNeeded);
//       in.push_back(SS_UserTermsNotNeeded);
//       std::list<int32> out;
//       out.push_back(SS_ShowUserTerms);
//       out.push_back(nextEvent);
//       WFStartupOrEvent *tmp =
//          new (ELeave) WFStartupOrEvent(SS_UserTerms, in, out);

//       startupHandler->RegisterStartupEvent(SS_UserTerms, tmp);
//       startupHandler->RegisterStartupEvent(SS_UserTermsNeeded, tmp);
//       startupHandler->RegisterStartupEvent(SS_UserTermsNotNeeded, tmp);
//    }
//    {
//       std::list<int32> in;
//       in.push_back(SS_USDisclaimerDoTest);
//       in.push_back(SS_USDisclaimerDontTest);
//       std::list<int32> out;
//       out.push_back(SS_ShowUserTerms);
//       out.push_back(nextEvent);
//       WFStartupOrEvent *tmp =
//          new (ELeave) WFStartupOrEvent(SS_USDisclaimer, in, out);

//       startupHandler->RegisterStartupEvent(SS_USDisclaimer, tmp);
//       startupHandler->RegisterStartupEvent(SS_USDisclaimerDoTest, tmp);
//       startupHandler->RegisterStartupEvent(SS_USDisclaimerDontTest, tmp);
//    }

// //   startupHandler->RegisterStartupEvent(SS_ShowUserTerms,
// //         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
// //            WF_COMBINED_USER_TERMS, languageIsoTwoChar));

//    /* Commands from web-page. */
//    startupHandler->RegisterStartupEvent(SS_USDisclaimerNeverShow,
//          new (ELeave) WFStartupSaveUSDisclaimer(sender, SS_UserTermsAccepted));

//    startupHandler->RegisterStartupEvent(SS_USDisclaimerReject,
//          new (ELeave) WFStartupNextEvent(SS_Exit));

//    startupHandler->RegisterStartupEvent(SS_USDisclaimerAccept,
//          new (ELeave) WFStartupNextEvent(SS_UserTermsAccepted));

//    startupHandler->RegisterStartupEvent(SS_UserTermsAccepted,
//          new (ELeave) WFStartupUserTermsAccepted(sender, nextEvent));
// }

void
WFStartupCommonBlocks::IAPSearch(WFStartupHandler* startupHandler,
      int32 nextEvent, const char *languageIsoTwoChar,
      WFServiceViewHandler* viewHandler,
      GuiProtMessageSender* sender,
      IAPDataStore* iapDataStore)
{
   /* Create event for getting the iap id and starting */
   /* iap search if no iap id is set. The event handles a */
   /* number of states. */
   WFStartupGetIAPId* tmp = new (ELeave) WFStartupGetIAPId(sender,
         iapDataStore);
   startupHandler->RegisterStartupEvent( SS_GetIap, tmp, true );
   startupHandler->RegisterStartupEvent( SS_IapReceived, tmp, true );
   startupHandler->RegisterStartupEvent( SS_IapSearchRestart,
         new (ELeave) WFStartupUiCallback(), true );
   startupHandler->RegisterStartupEvent(SS_IapSearch,
         new (ELeave) WFStartupUiCallback(), true );

   /* Add event for going to Service Window if IAP search fails. */
//   startupHandler->RegisterStartupEvent(SS_IapSearchNotOk,
//         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//            WF_IAP_SEARCH_FAILED, languageIsoTwoChar), true );
   /* Add event for going to Service Window if IAP search fails. */
//   startupHandler->RegisterStartupEvent(SS_IapSearchOkButNotReally,
//         new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//            WF_IAP_SEARCH_OK_BUT_FAILED, languageIsoTwoChar), true );

   startupHandler->RegisterStartupEvent(SS_IapContinueAnyway,
         new WFStartupNextEvent(SS_StartupFinished), true );

   /* Unnecessary, but keep until startup work is done. */
   startupHandler->RegisterStartupEvent(SS_IapSearchOk,
         new WFStartupNextEvent(nextEvent), true );
}

void WFStartupCommonBlocks::SendRegistrationSms(WFStartupHandler* startupHandler,
                                                int32 nextEvent, 
                                                GuiProtMessageSender* guiProtHandler)
{
   std::list<int32> in;
   in.push_back(SS_SendRegistrationSmsNeeded);
   in.push_back(SS_SendRegistrationSmsNotNeeded);
   std::list<int32> out;
   out.push_back(SS_ShowRegistrationSmsStartup);
   out.push_back(nextEvent);

   WFStartupOrEvent *regsms =
      new (ELeave) WFStartupOrEvent(SS_SendRegistrationSmsTest, in, out);

   startupHandler->RegisterStartupEvent(SS_SendRegistrationSmsTest, regsms);
   startupHandler->RegisterStartupEvent(SS_SendRegistrationSmsNeeded, regsms);
   startupHandler->RegisterStartupEvent(SS_SendRegistrationSmsNotNeeded, regsms);

   startupHandler->RegisterStartupEvent(SS_ShowRegistrationSmsStartup, 
                                        new (ELeave) WFStartupUiCallback());
}

void 
WFStartupCommonBlocks::SetupStartup(WFStartupHandler* startupHandler,
      WFServiceViewHandler* viewHandler,
      GuiProtMessageSender* guiProtHandler,
      CWayfinderDataHolder* dataHolder,
      WFNewsData* newsData,
      WFAccountData* accountData,
      IAPDataStore* iapDataStore,
      const char* languageIsoTwoChar,
      char* langSyntaxPath,
      bool useWFID, 
      bool useSilentStartup)
{
   /* Add event for exiting. */
   startupHandler->RegisterStartupEvent(SS_Exit,
         new WFStartupUiCallback());
   /* Add event for clearing browser cache. */
   startupHandler->RegisterStartupEvent(SS_ClearBrowserCache,
         new WFStartupUiCallback());

   startupHandler->RegisterStartupEvent(SS_Start,
         new WFStartupNextEvent(SS_CheckFlightMode), true );

   startupHandler->RegisterStartupEvent(SS_CheckFlightMode,
         new (ELeave) WFStartupUiCallback(), true );

   startupHandler->RegisterStartupEvent(SS_CheckFlightModeOk,
         new WFStartupNextEvent(SS_GetWFType), true );

   /* Add event for getting the WFtype on startup. */
   startupHandler->RegisterStartupEvent(SS_GetWFType,
         new WFStartupGetWfType(guiProtHandler,
            SS_WFTypeReceived, dataHolder->iWFAccountData), true  );

   /* Unnecessary, but keep until startup work is done. */
   startupHandler->RegisterStartupEvent(SS_WFTypeReceived,
         new WFStartupNextEvent(SS_GetIap), true );

   WFStartupCommonBlocks::IAPSearch(startupHandler,
         SS_IapOk, languageIsoTwoChar,
         viewHandler,
         guiProtHandler,
         iapDataStore);

   std::deque<int32> params;
   // if (useWFID) { // #ifdef USE_WF_ID
   //    params.push_back(SS_SendNopTest);
   // } else {
      params.push_back(SS_SendFavoriteSync);
   // }
   params.push_back(SS_AudioScriptsInit);

   startupHandler->RegisterStartupEvent(SS_IapOk,
         new WFStartupNextEvent(params), true );

   /* Event for sending audio scripts initialization message. */
   startupHandler->RegisterStartupEvent(SS_AudioScriptsInit,
         new WFStartupAudioScripts(guiProtHandler, langSyntaxPath), true );

   startupHandler->RegisterStartupEvent(SS_StartupFailed,
         new (ELeave) WFStartupUiCallback(), true );

   startupHandler->RegisterStartupEvent(SS_StartupError,
         new (ELeave) WFStartupNextEvent(SS_StartupErrorReal), true );

   startupHandler->RegisterStartupEvent(SS_StartupErrorReal,
         new (ELeave) WFStartupUiCallback(), true );

   // if (useSilentStartup) { // #ifdef USE_SILENT_STARTUP
   //    // This is the new startup so we need to know if we actually have 
   //    // a username (uin) or not.
      startupHandler->RegisterStartupEvent(SS_GetUsername,
         new WFStartupSendUsername(guiProtHandler, 
                                   accountData,
                                   SS_UsernameReceived, 
                                   SS_UsernameNotReceived), true );
   // } else {
      // We don't care if we have username or not so set both as received.
      // startupHandler->RegisterStartupEvent(SS_GetUsername,
      //    new WFStartupSendUsername(guiProtHandler, 
      //                              accountData,
      //                              SS_UsernameReceived, 
      //                              SS_UsernameReceived), true );
   // }

   //#ifdef USE_WF_ID sent in from AppUi since this is not settings dependent.
//   if (useWFID) {
//      startupHandler->RegisterStartupEvent(SS_SendNopTest,
//         new (ELeave) WFStartupSendNop(guiProtHandler, 
//                                       viewHandler,
//                                       SS_GetUsername), true);
//
//      if (useSilentStartup) { // #ifdef USE_SILENT_STARTUP
//         // This is the new startup and does not show the service window 
//         // if we're a registred user.
//         startupHandler->RegisterStartupEvent(SS_UsernameReceived,
//            new WFStartupNextEvent(SS_StartupComplete), true);
//
//         // If we have no username (uin) we need to show service window.
//         startupHandler->RegisterStartupEvent(SS_UsernameNotReceived,
//            new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//                                                     WF_SILENT_STARTUP_FIRSTPAGE,
//                                                     languageIsoTwoChar), 
//                                              true);
//      } else {
//         // This is standard WFID startup so always show the service window.
//         startupHandler->RegisterStartupEvent(SS_UsernameReceived,
//            new (ELeave) WFStartupGotoServiceViewUrl(viewHandler,
//                                                     WF_ID_FIRSTPAGE, 
//                                                     languageIsoTwoChar), 
//                                              true);
//      }

//      startupHandler->RegisterStartupEvent(SS_StartupComplete,
//         new WFStartupNextEvent(SS_SendParamSync), true);
//
//      startupHandler->RegisterStartupEvent(SS_SendParamSync,
//         new (ELeave) WFStartupParamSync(guiProtHandler, SS_ParamSyncReceived),
//                                           true);
//
//      startupHandler->RegisterStartupEvent(SS_ParamSyncReceived,
//         new WFStartupNextEvent(SS_ActivateSplashView), true);
//
//      startupHandler->RegisterStartupEvent(SS_SplashViewActivated,
//         new WFStartupNextEvent(SS_StartupFinished), true);
//   } else { // #else we dont USE_WF_ID
      /* Send favorite synchronization message. */
      startupHandler->RegisterStartupEvent(SS_UsernameReceived,
         new WFStartupNextEvent(SS_StartupComplete), true);
      startupHandler->RegisterStartupEvent(SS_UsernameNotReceived,
                                           new WFStartupNextEvent(SS_StartupComplete), true);
      startupHandler->RegisterStartupEvent(SS_SendFavoriteSync,
         new WFStartupFavoriteSync(guiProtHandler, SS_FavoriteSyncReceived), 
                                           true );

      startupHandler->RegisterStartupEvent(SS_FavoriteSyncReceived,
                                           new WFStartupNextEvent(SS_StartupFinished), true );

      // startupHandler->RegisterStartupEvent(SS_UsernameReceived,
      //                                      new WFStartupNextEvent(SS_ExpiredTest), true );
      // startupHandler->RegisterStartupEvent(SS_UsernameReceived,
      //                                      new WFStartupNextEvent(SS_UserTerms), true );

      // WFStartupCommonBlocks::CombinedUserTerms(startupHandler,
      //                                          SS_ExpiredTest, languageIsoTwoChar,
      //                                          viewHandler, guiProtHandler);

      // WFStartupCommonBlocks::ShowNews(startupHandler,
      //                                 SS_ActivateSplashView,
      //                                 languageIsoTwoChar, viewHandler,
      //                                 newsData,
      //                                 guiProtHandler);

      // startupHandler->RegisterStartupEvent(SS_SplashViewActivated,
      //    new WFStartupNextEvent(SS_SendRegistrationSmsTest), true);
      // startupHandler->RegisterStartupEvent(SS_UsernameReceived,
      //                                      new WFStartupNextEvent(SS_SendRegistrationSmsTest), true);

      // WFStartupCommonBlocks::ExpiredTest(startupHandler, SS_ModeTest, 
      //                                    languageIsoTwoChar, viewHandler);
      
      // WFStartupCommonBlocks::WFMode(startupHandler, SS_ShowNewsTest, 
      //                               languageIsoTwoChar, viewHandler);
      
      // WFStartupCommonBlocks::SendRegistrationSms(startupHandler, 
      //                                            SS_StartupFinished,
      //                                            guiProtHandler);
//   }

   /* Events to handle upgrade of client */
//   WFStartupCommonBlocks::UpgradeClient(startupHandler, languageIsoTwoChar, 
//                                        viewHandler, guiProtHandler);

/*    WFStartupCommonBlocks::UserTerms(startupHandler, */
/*          SS_USDisclaimer, languageIsoTwoChar, viewHandler, guiProtHandler); */

/*    WFStartupCommonBlocks::UsDisclaimer(startupHandler, */
/*          SS_ExpiredTest, languageIsoTwoChar, viewHandler, guiProtHandler); */

   // startupHandler->RegisterStartupEvent(SS_ActivateSplashView, 
   //                                      new (ELeave) WFStartupUiCallback());

   startupHandler->RegisterStartupEvent(SS_StartupFinished,
         new (ELeave) WFStartupUiCallback());

   startupHandler->RegisterStartupEvent(SS_GotoMainMenu,
         new (ELeave) WFStartupUiCallback());

   // startupHandler->RegisterStartupEvent(SS_ChangeUin,
   //       new (ELeave) WFStartupChangeUin(guiProtHandler, 
   //                                       viewHandler));

   startupHandler->RegisterStartupEvent(SS_NewServerList,
         new (ELeave) WFStartupSetServerList(guiProtHandler, 
                                             viewHandler));

//   WFStartupCommonBlocks::UpgradePage(startupHandler, languageIsoTwoChar, 
//                                      viewHandler, guiProtHandler);   
}
