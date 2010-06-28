/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WFStartupEvent.h"
#include "WFCommonStartupEvents.h"
#include "WFStartupHandler.h"

#include "NavRequestType.h"
#include "NavServerComProtoVer.h"
#include "NParamBlock.h"
#include "GuiProt/GuiParameterEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtMessageSender.h"
#include "GuiProt/UserTermsEnums.h"
#include "StringUtility.h"
#include "WFNewsData.h"
#include "WFAccountData.h"
#include "WFStartupEventCallbackParam.h"
#include "Nav2Error.h"
#include "TraceMacros.h"
#include <string.h>

void
startup_panic()
{
   /* Boom. */
   char *foo = NULL;
   foo[0] = 0;
}

bool
WFStartupSendParamBase::GuiProtReceiveMessage(isab::GuiProtMess * /*mess*/)
{
   /* The parameter has now been received. */
   /* We ignore the real message and just trigger the next event. */
   /* Note that even a "request failed" will do this, but on the other */
   /* hand, a parameter get should never fail. */
   m_handler->TriggerEvent(m_nextEvent);

   /* Mark message as handled. */
   return true;
}

WFStartupSendParamBase::~WFStartupSendParamBase()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
}

void
WFStartupSendUsername::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   /* Send parameter get for wayfinder username. */
   class isab::GeneralParameterMess gpm(
         (uint16)isab::GuiProtEnums::paramUserAndPassword);
   m_sender->SendMessage(gpm, this);
   /* No need to use deleteMembers(), no data allocated. */
}

bool
WFStartupSendUsername::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if (mess->getMessageType() == isab::GuiProtEnums::SET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
         static_cast<isab::GeneralParameterMess*>(mess);
      uint16 id = gpm->getParamId();
      switch (id) {
      case (uint16)isab::GuiProtEnums::paramUserAndPassword:
         {
            // type_and_string
            const char* username = gpm->getStringData()[0];
            // strlen generates a Kern-Exec 3 if username is null
            if (( username == NULL || strlen(username) < 1) && (m_nbrRetries > 0)) {
               m_handler->TriggerEvent(SS_GetUsername, NULL, (10*1000)); //10 ms delay
               m_nbrRetries--;
            } else {
               if ( username != NULL ) {
                  m_accountData->setUserName(strdup(username));
                  char* passw = gpm->getStringData()[1];
                  if (passw != NULL) {
                     // strdup generates a Kern-Exec 3 if passw is null
                     m_accountData->setPassword(strdup(gpm->getStringData()[1]));
                  }
                  // We have a uin so use it and continue startup.
                  m_handler->TriggerEvent(m_successEvent);
               } else {
                  // Did not have any uin in client so we need to get one.
                  m_handler->TriggerEvent(m_failEvent);
               }
            }
         }
         break;
      }
   } else {
      isab::RequestFailedMess* rf =
         static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      uint32 error = rf->getErrorNbr();

      class WFStartupEventCallbackParameter* tmp =
         new WFStartupEventCallbackParameter(SP_UsernameFailed,
                                             error, reqType);
      m_handler->TriggerEvent(SS_StartupFailed, tmp);
   }
   return true;
}

WFStartupSendUsername::~WFStartupSendUsername()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
}

void
WFStartupUpgrade::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   if (param->m_type == SP_Upgrade) {
      if ( m_upgradeParams && param != m_upgradeParams ) {
         delete m_upgradeParams;
         m_upgradeParams = NULL;
      }
      m_upgradeParams = 
         static_cast<WFStartupEventUpgradeCallbackParameter*>(param);

      if ((m_upgradeParams->m_keyStr != NULL) &&
          (strlen(m_upgradeParams->m_keyStr) == 0)) {
         class WFStartupEventUpgradeFailedCallbackParameter* params =
            new WFStartupEventUpgradeFailedCallbackParameter(false, true, true, 
                  true, true);
         m_handler->TriggerEvent(SS_UpgradeFailed, params);
      } else if ((m_upgradeParams->m_phone != NULL) && (strlen(m_upgradeParams->m_phone) == 0)) {
         class WFStartupEventUpgradeFailedCallbackParameter* params =
            new WFStartupEventUpgradeFailedCallbackParameter(true, false, true, 
                  true, true);
         m_handler->TriggerEvent(SS_UpgradeFailed, params);
      } else if ((m_upgradeParams->m_name != NULL) && (strlen(m_upgradeParams->m_name) == 0)) {
         class WFStartupEventUpgradeFailedCallbackParameter* params =
            new WFStartupEventUpgradeFailedCallbackParameter(true, true, false, 
                  true, true);
         m_handler->TriggerEvent(SS_UpgradeFailed, params);
      } else if ((m_upgradeParams->m_email != NULL) && (strlen(m_upgradeParams->m_email) == 0)) {
         class WFStartupEventUpgradeFailedCallbackParameter* params =
            new WFStartupEventUpgradeFailedCallbackParameter(true, true, true, 
                  false, true);
         m_handler->TriggerEvent(SS_UpgradeFailed, params);
      } else {
         isab::LicenseKeyMess lkm(m_upgradeParams->m_keyStr,
               m_upgradeParams->m_phone, m_upgradeParams->m_name,
               m_upgradeParams->m_email, m_upgradeParams->m_optional,
               m_upgradeParams->m_countryId);
         m_sender->SendMessage(lkm, this);
         lkm.deleteMembers();
      }
   }
}

bool
WFStartupUpgrade::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   //      Handle errormessage with upgrade multiple choices error. By
   //      going to a webpage. With m_upgradeParams->m_keyStr in url...
   //      New event for this, that uses params, SS_UpgradeChoices

   if ( mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED ) {
      isab::RequestFailedMess* rf = 
         static_cast<isab::RequestFailedMess*>( mess );
      if ( rf->getFailedRequestMessageNbr() == 
           isab::GuiProtEnums::REQUEST_LICENSE_UPGRADE ) {
         if ( rf->getErrorNbr() == 
              isab::Nav2Error::NSC_UPGRADE_MUST_CHOOSE_REGION ) {
            // Must show page to let user choose region.
            m_handler->TriggerEvent( SS_UpgradeChoices, m_upgradeParams );
            return true;
         }
      }
   } else if ( mess->getMessageType() == 
               isab::GuiProtEnums::LICENSE_UPGRADE_REPLY ) {
      isab::LicenseReplyMess* lrp = 
         static_cast<isab::LicenseReplyMess*>(mess);
      if (!lrp->allOk()) {
         //Un-used fields in a form are recieved as NULL, so if a field is 
         //un-used we have to okay it even if the server says it is not ok.
         bool okAnyway = 
            ((lrp->isKeyOk()) || (m_upgradeParams->m_keyStr == NULL)) &&
            ((lrp->isPhoneOk()) || (m_upgradeParams->m_phone == NULL)) &&
            ((lrp->isNameOk()) || (m_upgradeParams->m_name == NULL)) &&
            ((lrp->isEmailOk()) || (m_upgradeParams->m_email == NULL)) &&
            ((lrp->isRegionOk()) || (m_upgradeParams->m_countryId == MAX_INT32));
         if (okAnyway) {
            m_handler->TriggerEvent(SS_UpgradeOk);
         } else {
            class WFStartupEventUpgradeFailedCallbackParameter* params =
               new WFStartupEventUpgradeFailedCallbackParameter(
                     lrp->isKeyOk(), lrp->isPhoneOk(), 
                     lrp->isNameOk(), lrp->isEmailOk(), 
                     lrp->isRegionOk());
            m_handler->TriggerEvent(SS_UpgradeFailed, params);
         }
      } else {
         m_handler->TriggerEvent(SS_UpgradeOk);
      }
      return true;
   }
   return false;  
}

WFStartupUpgrade::~WFStartupUpgrade()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
   delete m_upgradeParams;
}

//WFStartupUpgradeChoose::WFStartupUpgradeChoose( 
//   class GuiProtMessageSender* sender,
//   class WFServiceViewHandler* viewHandler,
//   const char *url, const char *langCode, 
//   int32 backAction ) :
//      m_sender( sender ), m_viewHandler( viewHandler ), 
//      m_upgradeParams( NULL ), 
//      m_url( StringUtility::replaceString( url, "XX", langCode ) ),
//      m_backAction( backAction ) 
//{
//}
//
//void
//WFStartupUpgradeChoose::EventCallback(
//   int32 eventType, WFStartupEventCallbackParameter* param )
//{
//   if ( param->m_type == SP_Upgrade ) {
//      if ( eventType == SS_RetryUpgrade ) {
//         m_upgradeParams->m_countryId = 
//            static_cast<WFStartupEventUpgradeCallbackParameter*>( param )
//            ->m_countryId;
//         delete param;
//         m_handler->TriggerEvent( SS_Upgrade, m_upgradeParams );
//      } else if ( eventType == SS_UpgradeChoices ) {
//         m_upgradeParams = 
//            static_cast<WFStartupEventUpgradeCallbackParameter*>( param );
//         if ( m_upgradeParams->m_keyStr == NULL ||
//              strlen( m_upgradeParams->m_keyStr ) == 0 ) {
//            class WFStartupEventUpgradeFailedCallbackParameter* params =
//               new WFStartupEventUpgradeFailedCallbackParameter(
//                  false, true, true, true, true);
//            m_handler->TriggerEvent( SS_UpgradeFailed, params );
//         } else {
//            // Make url and launch it
//            char* keyStr = new char[ strlen( 
//                                        m_upgradeParams->m_keyStr ) *3 + 3 ];
//            StringUtility::URLEncode( keyStr, m_upgradeParams->m_keyStr );
//            char* url1 = StringUtility::replaceString( 
//               m_url, "[AC]", keyStr );
//            const char *phoneStr = NULL;
//            if ( m_upgradeParams->m_phone == NULL ||
//                 strlen( m_upgradeParams->m_phone ) == 0 ) {
//               phoneStr = "";
//            } else {
//               phoneStr = m_upgradeParams->m_phone;
//            }
//            char* phoneNbr = new char[ strlen( phoneStr ) *3 + 3 ];
//            StringUtility::URLEncode( phoneNbr, phoneStr );
//            char* url = StringUtility::replaceString( 
//               url1, "[phone]", phoneNbr );
//            // URL Escape it
//            m_viewHandler->GotoServiceViewUrl(
//               url, static_cast<BackActionEnum>( m_backAction ) );
//            delete [] keyStr;
//            delete [] url1;
//            delete [] phoneNbr;
//            delete [] url;
//         }
//      } // End else if SS_RetryUpgrade
//   }
//}
//
//bool
//WFStartupUpgradeChoose::GuiProtReceiveMessage(isab::GuiProtMess *mess)
//{
//   // No GuiProt messages used, yet.
//   return false;  
//}
//
//WFStartupUpgradeChoose::~WFStartupUpgradeChoose()
//{
//   /* Make sure that the GuiProtHandler does not have any */
//   /* pointers to us after we have been deleted. */
//   if ( m_sender ) {
//      m_sender->DisownMessage( this );
//   }
//   delete[] m_url;
//}


void
WFStartupGetWfType::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   /* Send parameter get for wayfinder type. */
   class isab::GeneralParameterMess gpm(
         isab::GuiProtEnums::paramWayfinderType);
   m_sender->SendMessage(gpm, this);
   /* No need to use deleteMembers(), no data allocated. */
}

bool
WFStartupGetWfType::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{ 
   if (mess->getMessageType() == isab::GuiProtEnums::SET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
         static_cast<isab::GeneralParameterMess*>(mess);

      if (gpm->getParamId() == isab::GuiProtEnums::paramWayfinderType) {

         isab::GuiProtEnums::WayfinderType wft =
               isab::GuiProtEnums::WayfinderType(*(gpm->getIntegerData()));

         m_accountData->setWfType(wft);
         /* The parameter has now been received. */
         m_handler->TriggerEvent(m_nextEvent);
         return true;
      }
   }
   return false;
}

bool
WFStartupFavoriteSync::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if ( mess->getMessageType() == isab::GuiProtEnums::PARAMETERS_SYNC_REPLY ) {
      // Reply received
      m_handler->TriggerEvent(SS_AccountNotExpired);
      m_handler->TriggerEvent(m_nextEvent);
      /* Mark as handled. */
      return true;
   } else if (mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED) {
      /* Check for accepted errors. */
      isab::RequestFailedMess* rf =
            static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      uint32 error = rf->getErrorNbr();
      if ( rf->getErrorURL()[ 0 ] != '\0' ) {
         // I let GuiProtMessageErrorHandler take this
         // Goto URL on server and then exit app.
         // Should I return true? So WFAppUI don't handles it too!
         return true;
      } else if ( reqType == isab::GuiProtEnums::PARAMETERS_SYNC ) {
         if (error == isab::Nav2Error::NSC_EXPIRED_USER) {
            /* Request failed due to expired user. */
            /* We need to show the real page here. */
            m_handler->TriggerEvent(SS_ShowExpired, NULL, (1*1000*1000)); //1 s delay
         } else if (isab::Nav2Error::Nav2ErrorTable::acceptableServerError(error)) {
            /* Request failed in some fashion that lets us know that the */
            /* server was reached. */
            /* In this case, the IAP is still OK, but the user cannot run */
            /* Wayfinder. */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_FavoriteSyncFailed,
                  error, reqType);
            m_handler->TriggerEvent(SS_StartupFailed, tmp);
         } else if (error == isab::Nav2Error::NSC_NO_NETWORK_AVAILABLE ||
                    error == isab::Nav2Error::NSC_FLIGHT_MODE) {

            m_handler->TriggerEvent(SS_IapSearchNotOk);
            /* Trigger event that notifies the observer what went wrong */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_NoNetwork,
                  error, reqType, rf->getErrorString());
            m_handler->TriggerEvent(SS_StartupError, tmp);
         } else {
            /* The request failed in some fashion that indicates that the */
            /* server was unreachable. */
            /* In this case, we need to do the IAP search again. */
            m_handler->TriggerEvent(SS_IapSearchRestart);
         }
         /* Mark as handled. */
         return true;
      }
   }
   /* Mark as not handled. */
   return false;
}

void
WFStartupFavoriteSync::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   /* Send sync favorites. No Server Info has favorites crc*/
   class isab::GenericGuiMess sync(isab::GuiProtEnums::/*SYNC_FAVORITES*/PARAMETERS_SYNC); 
   /* Will register us in GuiProtSender. */
   m_sender->SendMessage(sync, this);
}

void
WFStartupLatestNewsShown::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   const char* csum = m_newsData->getChecksum();
   if (!csum || !strlen(csum)) {
      /* Do not save. */
      m_handler->TriggerEvent(m_nextEvent);
      return;
   }
   /* Make sure that the news data checksum is reset. */
   m_newsData->setShown();

   /* Update checksum for shown news in Nav2. */
   char* checksum[1];
   checksum[0] = const_cast<char *>(csum);
   class isab::GeneralParameterMess gpm(
         isab::GuiProtEnums::paramShownNewsChecksum,
               checksum, 1);

   m_sender->SendMessage(gpm);
   /* No need to use deleteMembers(), no data allocated. */

   m_handler->TriggerEvent(m_nextEvent);
}

bool
WFStartupGetIAPId::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   /* Reply to the get IAP id parameter. */
   bool unset = false;
   isab::GuiProtEnums::MessageType type = mess->getMessageType();
   switch( type ) {
      case isab::GuiProtEnums::GET_GENERAL_PARAMETER:
         /* IAP unset. */
         unset = true;
         /* FALLTHROUGH */
      case isab::GuiProtEnums::SET_GENERAL_PARAMETER:
      {
         /* IAP data found. */
         isab::GeneralParameterMess* gpm =
               static_cast<isab::GeneralParameterMess*>(mess);
         switch (gpm->getParamId()) {
            case isab::GuiProtEnums::paramSelectedAccessPointId:
               {
                  /* Yes, the correct parameter. */
                  int32 iapid = -1;
                  /* Mark next event as start iap search. */
                  int32 nextEvent = SS_IapSearch;
                  if (!unset) {
                     iapid = *(gpm->getIntegerData());
                     if (iapid >= 0) {
                        /* Mark next event as iap search ok. */
                        nextEvent = SS_IapOk;
                     }
                  }
                  /* Store the value. */
                  m_iapStore->SetIap(iapid);
                  m_handler->TriggerEvent(nextEvent);
               }
               break;
            default:
               /* Should never happen, crash */
               PANIC();
               break;
         }
         break;
      }
      default:
         /* Should never happen, crash */
         PANIC();
         break;
   }
   /* Mark message as handled, it should be safe since it's the reply */
   /* to our request. */
   return true;
}

void
WFStartupGetIAPId::EventCallback(int32 eventType,
   WFStartupEventCallbackParameter* param)
{
   /* Send get iap id message. */
   class isab::GeneralParameterMess gpm(
         isab::GuiProtEnums::paramSelectedAccessPointId);
   m_sender->SendMessage(gpm, this);
}

WFStartupGetIAPId::~WFStartupGetIAPId()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
}

WFStartupAudioScripts::~WFStartupAudioScripts()
{
   delete[] m_audioPath;
}
void
WFStartupAudioScripts::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   /* Send request to load the audio syntax stuff, but don't bother */
   /* about the reply, it is not interesting for us. */
   /* Thus, the reply will be handled by the AppUi. */
   isab::GenericGuiMess loadAudio(isab::GuiProtEnums::LOAD_AUDIO_SYNTAX,
         m_audioPath);
   m_sender->SendMessage(loadAudio);
}

void
WFStartupUserTermsAccepted::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{

   // XXX Sending this parameter here makes the application to show
   //     user terms only once, even if the user didnt check the
   //     dont show again box. Moved to WFStartupSaveUSDisclaimer::EventCallback

   /* Send request for user terms parameter, but don't bother about */
   /* the reply. Thus, the reply will be handled in the AppUi. */
//    class isab::GeneralParameterMess gpm(
//          isab::GuiProtEnums::paramUserTermsAccepted,
//          int32(isab::GuiProtEnums::UserTermsAccepted));
//    m_sender->SendMessage(gpm);
//    /* Need to use deleteMembers(), data was allocated by the constructor. */
//    gpm.deleteMembers();

   /* Trigger next event. */
   m_handler->TriggerEvent(m_nextEvent);
}

void
WFStartupNextEvent::EventCallback(int32 /* eventType */, WFStartupEventCallbackParameter* param)
{
   /* Loop over the pending events and trigger them in succession. */
   std::deque<int32>::iterator it = m_eventPending.begin();
   while (it != m_eventPending.end()) {
      m_handler->TriggerEvent(*it, param);
      it++;
   }
}

WFStartupNextEvent::WFStartupNextEvent(std::deque<int32>& eventList)
{
   /* Copy parameter to member. */
   m_eventPending = eventList;
}
WFStartupNextEvent::WFStartupNextEvent(int32 nextEvent)
{
   m_eventPending.push_back(nextEvent);
}
WFStartupNextEvent::~WFStartupNextEvent()
{
   m_eventPending.clear();
}

void
WFStartupUiCallback::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   m_handler->BaseUiCallback(eventType, param);
}


// WFStartupGotoServiceViewUrl::WFStartupGotoServiceViewUrl(
//       class WFServiceViewHandler* viewHandler,
//       const char *url, const char *langCode, int32 backAction)
//  : m_viewHandler(viewHandler)
// {
//    m_url = StringUtility::replaceString(url, "XX", langCode);
//    if (m_url == NULL) {
//       // replaceString returns NULL if there are no XX to replace.
//       // Then we just copy the url sent to us.
//       m_url = StringUtility::newStrDup(url);
//    }
//    if (backAction < 0) {
//       m_backAction = BackIsExit;
//    } else {
//       m_backAction = backAction;
//    }
// }

// WFStartupGotoServiceViewUrl::~WFStartupGotoServiceViewUrl()
// {
//    delete[] m_url;
// }

// void
// WFStartupGotoServiceViewUrl::EventCallback(int32 /* eventType */, WFStartupEventCallbackParameter* param)
// {
//    m_viewHandler->GotoServiceViewUrl(m_url,
//       static_cast<BackActionEnum>(m_backAction));
// }

WFStartupChoiceEvent::~WFStartupChoiceEvent()
{
   m_resultList.clear();
   m_eventList.clear();
}

void
WFStartupChoiceEvent::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   std::list<int32>::iterator it = m_eventList.begin();
   uint32 i = 0;
   while (it != m_eventList.end()) {
      if (eventType == *it) {
         it = m_resultList.begin();
         uint32 j = 0;
         while (it != m_resultList.end()) {
            if (j != i) {
               m_handler->TriggerEvent(*it);
               return;
            }
            it++;
            j++;
         }
         /* If we get here, the event lists was not the same size. */
         return;
      }
      it++;
      i++;
   }
}

void
WFStartupAndEvent::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   std::list<int32>::iterator it = m_eventList.begin();
   uint32 i = 0;
   while (it != m_eventList.end()) {
      if (eventType == *it) {
         m_eventList.erase(it);
      }
      it++;
      i++;
   }
   /* All events received. */
   if (m_eventList.size() == 0) {
      m_handler->TriggerEvent(m_nextEvent);
   }
}
WFStartupAndEvent::~WFStartupAndEvent()
{
   m_eventList.clear();
}

void
WFStartupOrEvent::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   if (eventType == m_trigger) {
      /* Have any other event been received? */
      if (m_nextEvent < 0) {
         /* No, indicate that. */
         m_nextEvent = -2;
      } else {
         /* Perform event */
         m_handler->TriggerEvent(m_nextEvent);
      }
   } else {
      std::list<int32>::iterator it = m_eventList.begin();
      uint32 i = 0;
      while (it != m_eventList.end()) {
         if (eventType == *it) {
            it = m_resultList.begin();
            uint32 j = 0;
            while (it != m_resultList.end()) {
               if (j == i) {
                  if (m_nextEvent == -2) {
                     /* Trigger already received. */
                     m_nextEvent = *it;
                     m_handler->TriggerEvent(m_nextEvent);
                  } else {
                     /* Save trigger event. */
                     m_nextEvent = *it;
                  }
                  return;
               }
               it++;
               j++;
            }
            /* If we get here, the event lists was not the same size. */
            return;
         }
         it++;
         i++;
      }
   }
}

void
WFStartupSaveUSDisclaimer::EventCallback(int32 eventType,
      WFStartupEventCallbackParameter* param)
{
   class isab::GeneralParameterMess gpm(
      isab::GuiProtEnums::paramNeverShowUSDisclaimer, (int32)1);
   m_sender->SendMessage(gpm);
   /* Need to use deleteMembers(), data was allocated by the constructor. */
   gpm.deleteMembers();


   // XXX Moved from WFStartupUserTermsAccepted::EventCallback.
   /* Send request for user terms parameter, but don't bother about */
   /* the reply. Thus, the reply will be handled in the AppUi. */
   class isab::GeneralParameterMess gpm2(
         isab::GuiProtEnums::paramUserTermsAccepted,
         int32(isab::GuiProtEnums::UserTermsAccepted));
   m_sender->SendMessage(gpm2);
   /* Need to use deleteMembers(), data was allocated by the constructor. */
   gpm2.deleteMembers();

   /* Trigger next event. */
   m_handler->TriggerEvent(m_nextEvent);
}

void
WFStartupParamSync::EventCallback(int32 eventType, WFStartupEventCallbackParameter* param)
{
   /* Send sync params. */
   class isab::GenericGuiMess sync(isab::GuiProtEnums::PARAMETERS_SYNC); 
   /* Will register us in GuiProtSender. */
   m_sender->SendMessage(sync, this);
}

bool
WFStartupParamSync::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if ( mess->getMessageType() == isab::GuiProtEnums::PARAMETERS_SYNC_REPLY ) {
      // Reply received
      m_handler->TriggerEvent(m_nextEvent);
      /* Mark as handled. */
      return true;
   } else if (mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED) {
      /* Check for accepted errors. */
      isab::RequestFailedMess* rf =
            static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      uint32 error = rf->getErrorNbr();
      if ( rf->getErrorURL()[ 0 ] != '\0' ) {
         // I let GuiProtMessageErrorHandler take this
         // Goto URL on server and then exit app.
         // Should I return true? So WFAppUI don't handles it too!
         return true;
      } else if ( reqType == isab::GuiProtEnums::PARAMETERS_SYNC ) {
         if (isab::Nav2Error::Nav2ErrorTable::acceptableServerError(error)) {
            /* Request failed in some fashion that lets us know that the */
            /* server was reached. */
            /* In this case, the IAP is still OK, but the user cannot run */
            /* Wayfinder. */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_FavoriteSyncFailed,
                  error, reqType);
            m_handler->TriggerEvent(SS_StartupFailed, tmp);
         } else if (error == isab::Nav2Error::NSC_NO_NETWORK_AVAILABLE ||
                    error == isab::Nav2Error::NSC_FLIGHT_MODE) {
            /* Trigger event that notifies the observer what went wrong */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_NoNetwork,
                  error, reqType, rf->getErrorString());
            m_handler->TriggerEvent(SS_StartupError, tmp);
         }
         /* Mark as handled. */
         return true;
      }
   }
   /* Mark as not handled. */
   return false;
}

void
WFStartupChangeUin::EventCallback(int32 eventType, 
                                  WFStartupEventCallbackParameter* param)
{
   if (param->m_type == SP_ChangeUin) {
      if ( m_changeUinParams && param != m_changeUinParams ) {
         delete m_changeUinParams;
         m_changeUinParams = NULL;
      }
      m_changeUinParams = 
         static_cast<WFStartupEventStringAndUrlsCallbackParameter*>(param);

      if (m_changeUinParams->m_stringData == NULL) {
         // Goto failure url.
         if (m_changeUinParams->m_failureUrl) {
            // Goto service window url.
            m_viewHandler->GotoServiceViewUrl(m_changeUinParams->m_failureUrl, 
                                              BackIsHistoryThenView);
         }
      } else if ((m_changeUinParams->m_stringData != NULL) &&
                 (strlen(m_changeUinParams->m_stringData) == 0)) {
         // Goto failure url.
         if (m_changeUinParams->m_failureUrl) {
            // Goto service window url.
            m_viewHandler->GotoServiceViewUrl(m_changeUinParams->m_failureUrl, 
                                              BackIsHistoryThenView);
         }
      } else {
         // Everything is fine as expected send the new uin to Nav2.
         char* uinstr[2];
         uinstr[0] = m_changeUinParams->m_stringData; // The uin to set
         uinstr[1] = "ChangeMe"; // NavServerCom also wants a password.
         class isab::GeneralParameterMess 
            setgpm(isab::GuiProtEnums::paramUserAndPassword, uinstr, 2);
         // Send the message to set the uin.
         m_sender->SendMessage(setgpm);

         class isab::GeneralParameterMess 
            getgpm(isab::GuiProtEnums::paramUserAndPassword);
         // Send a message to also get the uin so we end up in 
         // our GuiProtReceiveMessage function below.
         m_sender->SendMessage(getgpm, this);
      }
   }
}

bool
WFStartupChangeUin::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if (mess->getMessageType() == isab::GuiProtEnums::SET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
         static_cast<isab::GeneralParameterMess*>(mess);
      if (gpm->getParamId() == isab::GuiProtEnums::paramUserAndPassword) {
         const char* username = gpm->getStringData()[0];
         if (username != NULL) {
            if (strcmp(m_changeUinParams->m_stringData, username) == 0) {
               // Goto success url.
               if (m_changeUinParams->m_successUrl) {
                  m_viewHandler->GotoServiceViewUrl(m_changeUinParams->m_successUrl, 
                                                    BackIsHistoryThenView);
               }
            } else {
               // Goto failure url.
               if (m_changeUinParams->m_failureUrl) {
                  m_viewHandler->GotoServiceViewUrl(m_changeUinParams->m_failureUrl, 
                                                    BackIsHistoryThenView);
               }
            }
         }
         return true;
      }
   } else if (mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED) {
      /* Check for accepted errors. */
      isab::RequestFailedMess* rf =
            static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      //uint32 error = rf->getErrorNbr();
      if (reqType == isab::GuiProtEnums::paramUserAndPassword) {
         // Goto failure url.
         if (m_changeUinParams->m_failureUrl) {
            m_viewHandler->GotoServiceViewUrl(m_changeUinParams->m_failureUrl, 
                                              BackIsHistoryThenView);
         }
         /* Mark as handled. */
         return true;
      }
   }

   /* Mark as not handled. */
   return false;
}

WFStartupChangeUin::~WFStartupChangeUin()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
   delete m_changeUinParams;
}

void
WFStartupSetServerList::EventCallback(int32 eventType, 
                                      WFStartupEventCallbackParameter* param)
{
   if (param->m_type == SP_SetServerList) {
      if ( m_serverListParams && param != m_serverListParams ) {
         delete m_serverListParams;
         m_serverListParams = NULL;
      }
      m_serverListParams = 
         static_cast<WFStartupEventStringAndUrlsCallbackParameter*>(param);

      if (m_serverListParams->m_stringData == NULL) {
         // Goto failure url.
         if (m_serverListParams->m_failureUrl) {
            // Goto service window url.
            m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_failureUrl,
                                              BackIsHistoryThenView);
         }
      } else if ((m_serverListParams->m_stringData != NULL) &&
                 (strlen(m_serverListParams->m_stringData) == 0)) {
         // Goto failure url.
         if (m_serverListParams->m_failureUrl) {
            // Goto service window url.
            m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_failureUrl,
                                              BackIsHistoryThenView);
         }
      } else {
         // Everything is fine as expected, send the new server list to Nav2.
         // The server list to set
         char* serverString = m_serverListParams->m_stringData;

         if (!serverString) {
            // No servers in list, not allowed.
            return;
         }

         // At least one string, gets incremented for each
         int32 numEntries = 1;
         for (int i = 0; i < int(strlen(serverString)); ++i) {
            // Just count the number of server:port:type strings.
            if (serverString[i] == ';' || serverString[i] == ',') {
               numEntries++;
            }
         }
         char** serverArray = new char*[numEntries];

         int idx = 0;
         char delims[] = ";,";
         char* token = strtok(serverString, delims);
         while(token != NULL) {
            // Extract the server:port string from server:port:type.
            char* pch = strrchr(token, ':');
            int pos = pch - token;
            char* serverAndPort = new char[pos+1];
            serverAndPort = strncpy(serverAndPort, token, pos);
            serverAndPort[pos] = '\0'; // zero terminate
            serverArray[idx++] = serverAndPort;
            token = strtok(NULL, delims);
         }

         class isab::GeneralParameterMess 
            mess(isab::GuiProtEnums::paramServerNameAndPort, 
                 serverArray, numEntries);
         // Send the message to set the new server list.
         m_sender->SendMessage(mess);

         for (int i = 0; i < numEntries; ++i) {
            // Delete server strings from array.
            delete[] serverArray[i];
         }
         delete[] serverArray;

         class isab::GeneralParameterMess 
            getgpm(isab::GuiProtEnums::paramServerNameAndPort);
         // Send a message to also get the server list so we end up in 
         // our GuiProtReceiveMessage function below.
         m_sender->SendMessage(getgpm, this);
      }
   }
}

bool
WFStartupSetServerList::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if (mess->getMessageType() == isab::GuiProtEnums::SET_GENERAL_PARAMETER) {
      isab::GeneralParameterMess* gpm =
         static_cast<isab::GeneralParameterMess*>(mess);
      uint16 id = gpm->getParamId();
      if (id == (uint16)isab::GuiProtEnums::paramServerNameAndPort) {
         int numEntries = gpm->getNumEntries();
         if (numEntries > 0) {
            // The server list contains entries.
            for (int i = 0; i < numEntries; ++i) {
               if (strstr(m_serverListParams->m_stringData, 
                          gpm->getStringData()[i]) == NULL) {
                  // This server list entry was not part of the server string 
                  // sent to us from service window. So goto failureUrl.
                  if (m_serverListParams->m_failureUrl) {
                     m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_failureUrl, 
                                                       BackIsHistoryThenView);
                  }
                  return true;
               }
            }
            // We found all entries in service list in the server string 
            // sent to us from service window. So goto successUrl.
            if (m_serverListParams->m_successUrl) {
               m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_successUrl, 
                                                 BackIsHistoryThenView);
            }
            return true;
         }
         // The server list contained no entries, goto failureUrl.
         if (m_serverListParams->m_failureUrl) {
            m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_failureUrl, 
                                              BackIsHistoryThenView);
         }
         return true;
      }
   } else if (mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED) {
      /* Check for accepted errors. */
      isab::RequestFailedMess* rf =
            static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      //uint32 error = rf->getErrorNbr();
      if (reqType == isab::GuiProtEnums::paramServerNameAndPort) {
         // Goto failure url.
         if (m_serverListParams->m_failureUrl) {
            m_viewHandler->GotoServiceViewUrl(m_serverListParams->m_failureUrl, 
                                              BackIsHistoryThenView);
         }
         /* Mark as handled. */
         return true;
      }
   }
   /* Mark as not handled. */
   return false;
}

WFStartupSetServerList::~WFStartupSetServerList()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
   delete m_serverListParams;
}

void
WFStartupSendNop::EventCallback(int32 eventType, 
                                WFStartupEventCallbackParameter* param)
{
   // Send a NOP message to server and see if comes back, this way we 
   // can test the internet connection without having to authenticate 
   // the client agains the server, used for wayfinder id.
   isab::Buffer *buf = new isab::Buffer( 10 );
   isab::NParamBlock params;

   isab::Buffer tmpbuf( 128 );

   std::vector<byte> bbuf;

   params.writeParams(bbuf, NSC_PROTO_VER, false /* NO GZIP */);
   tmpbuf.writeNextByteArray( &bbuf.front(), bbuf.size() );
   const uint8* data = tmpbuf.accessRawData();
   uint32 size = tmpbuf.getLength();

   buf->writeNextUnaligned16bit(isab::navRequestType::NAV_NOP_REQ);

   class isab::DataGuiMess mess(isab::GuiProtEnums::GUI_TO_NGP_DATA,
                                buf->getLength(), buf->accessRawData(),
                                size, data);
   m_sender->SendMessage(mess, this);

   delete buf;
}

bool
WFStartupSendNop::GuiProtReceiveMessage(isab::GuiProtMess *mess)
{
   if (mess->getMessageType() == isab::GuiProtEnums::GUI_TO_NGP_DATA_REPLY) {
      // We got the message we expected.
      isab::DataGuiMess *dgm = 
         static_cast<isab::DataGuiMess*>(mess);

      int32 type = -1;
      isab::NonOwningBuffer mess_type_data(const_cast<uint8*>(dgm->getData()),
                                           dgm->getSize());
      
      if (mess_type_data.remaining() >= 2) {
         type = mess_type_data.readNextUnaligned16bit();
      }

      if (type == isab::navRequestType::NAV_NOP_REPLY) {
         // It was a nop reply so we everything is fine.
         m_handler->TriggerEvent(m_nextEvent);
      }
      /* Mark as handled. */
      return true;
   } else if (mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED) {
      /* Check for accepted errors. */
      isab::RequestFailedMess* rf =
            static_cast<isab::RequestFailedMess*>(mess);
      uint8 reqType = rf->getFailedRequestMessageNbr();
      uint32 error = rf->getErrorNbr();
      if ( rf->getErrorURL()[ 0 ] != '\0' ) {
         // I let GuiProtMessageErrorHandler take this
         // Goto URL on server and then exit app.
      } else if ( reqType == isab::GuiProtEnums::GUI_TO_NGP_DATA ) {
         if (isab::Nav2Error::Nav2ErrorTable::acceptableServerError(error)) {
            /* Request failed in some fashion that lets us know that the */
            /* server was reached. */
            /* In this case, the IAP is still OK, but the user cannot run */
            /* Wayfinder. */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_FavoriteSyncFailed,
                  error, reqType);
            m_handler->TriggerEvent(SS_StartupFailed, tmp);
         } else if (error == isab::Nav2Error::NSC_NO_NETWORK_AVAILABLE ||
                    error == isab::Nav2Error::NSC_FLIGHT_MODE) {

            m_handler->TriggerEvent(SS_IapSearchNotOk);
            /* Trigger event that notifies the observer what went wrong */
            class WFStartupEventCallbackParameter* tmp =
               new WFStartupEventCallbackParameter(SP_NoNetwork,
                  error, reqType, rf->getErrorString());
            m_handler->TriggerEvent(SS_StartupError, tmp);
         } else {
            /* The request failed in some fashion that indicates that the */
            /* server was unreachable. */
            /* In this case, we need to do the IAP search again. */
            m_handler->TriggerEvent(SS_IapSearchRestart);
         }
      }
      /* Mark as handled. */
      return true;
   }
   /* Mark as not handled. */
   return false;
}

WFStartupSendNop::~WFStartupSendNop()
{
   /* Make sure that the GuiProtHandler does not have any */
   /* pointers to us after we have been deleted. */
   if (m_sender) {
      m_sender->DisownMessage(this);
   }
}
