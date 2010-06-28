/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_COMMON_STARTUP_EVENTS_H
#define WF_COMMON_STARTUP_EVENTS_H

#include <arch.h>
#include "WFStartupEvent.h"                  // WFStartupEventCallback
#include "GuiProt/GuiProtMessageSender.h"    // GuiProtMessageReceiver
#include "IAPDataStore.h"
#include "WFServiceViewHandler.h"            // WFServiceViewHandler
#include <deque>
#include <list>

void startup_panic();

#define PANIC startup_panic

namespace isab {
   class GuiProtMess;
}

/*
 * WFStartupSendParamBase  -  Base class for WFStartupEvents that
 *    Sends a parameter request to Nav2.
 */
class WFStartupSendParamBase
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor
    * @param   sender         Pointer to the GuiProtMessageSender which will
    *                         handle sending of the message.
    * @param   nextEvent      Which event to trigger when the reply that
    *                         matches the message sent is received.
    */
   WFStartupSendParamBase(class GuiProtMessageSender* sender, int32 nextEvent) :
         m_sender(sender), m_nextEvent(nextEvent) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        To be implemented by the subclasses.
    *                         Called when the event has occurred, should trigger
    *                         the sending of the message via the
    *                         GuiProtMessageSender.
    * @param   eventType      Event that has occurred.
    * @param   param          Optional parameter.
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param) = 0;
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply. May be any type of GuiProtMess.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupSendParamBase();

   /**
    * m_sender    -           Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_nextEvent -           Id of event to trigger when the reply of the
    *                         message has been received.
    */
   int32 m_nextEvent;
};

/**
 *  WFStartupSendUsername  -  WFStartupEventCallback for taking care of
 *                            wayfinder username parameter.
 */
class WFStartupSendUsername
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupSendUsername(class GuiProtMessageSender* sender, 
                         class WFAccountData* accountData, 
                         int32 successEvent, 
                         int32 failEvent) :
        m_sender(sender), m_accountData(accountData), 
        m_successEvent(successEvent), m_failEvent(failEvent),
        m_nbrRetries(10) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for the wayfinder username
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupSendUsername();
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_accountData     -     Pointer to username data store.
    */
   class WFAccountData* m_accountData;
   /**
    * m_nextEvent -           Id of event to trigger when the reply of the
    *                         message has been received.
    */
   int32 m_successEvent;
   int32 m_failEvent;

   /**
    * m_nbrRetries -          Number of times to try to get the username 
    *                         before giving up.
    */
   int32 m_nbrRetries;
};

/**
 *  WFStartupUpgrade       -  WFStartupEventCallback for taking care of
 *                            wayfinder client upgrade.
 */
class WFStartupUpgrade
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupUpgrade(class GuiProtMessageSender* sender) :
      m_sender(sender), m_upgradeParams(NULL) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 with the wayfinder license
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. Used for upgrade parameters.
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupUpgrade();
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_upgradeParams -       Parameter containing the upgrade account info. 
    */
   class WFStartupEventUpgradeCallbackParameter* m_upgradeParams;
};

/**
 *  WFStartupUpgradeChoose    -  WFStartupEventCallback for taking care of
 *                               wayfinder client upgrade choosing region.
 */
class WFStartupUpgradeChoose
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupUpgradeChoose( class GuiProtMessageSender* sender,
                           class WFServiceViewHandler* viewHandler,
                           const char *url, const char *langCode, 
                           int32 backAction = BackIsHistoryThenView );

   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 with the wayfinder
    *                         license parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. Used for upgrade parameters.
    */
   virtual void EventCallback( int32 eventType, 
                               class WFStartupEventCallbackParameter* param );

   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage( isab::GuiProtMess *mess );

   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupUpgradeChoose();

   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;

   /**
    * m_viewHandler  -        Pointer to an object that can perform the action
    *                         of changing views to show the URL.
    */
//   class WFServiceViewHandler* m_viewHandler;

   /**
    * m_upgradeParams -       Parameter containing the upgrade account info. 
    */
   class WFStartupEventUpgradeCallbackParameter* m_upgradeParams;

   /**
    * m_url          -        The url to show.
    */
   char *m_url;

   /**
    * m_backAction   -        Action to perform when back is selected.
    */
   int32 m_backAction;
};

/**
 * WFStartupGetWfType   -     WFStartupEventCallback for taking care of
 *                            wayfinder type parameter.
 */
class WFStartupGetWfType
   : public WFStartupSendParamBase
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupGetWfType(class GuiProtMessageSender* sender, int32 nextEvent,
         class WFAccountData* accountData) :
        WFStartupSendParamBase(sender, nextEvent),
        m_accountData(accountData) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for the wayfinder type
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);

   class WFAccountData* m_accountData;
};
/**
 * WFStartupLatestNewsShown   -     Startup event class for getting the
 *                                  parameter which shows if latest news
 *                                  should be shown.
 *                                  Note that this object will not register
 *                                  for the reply.
 */
class WFStartupLatestNewsShown
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -        Initiate member variables.
    * @param   nextEvent      Next event to trigger.
    * @param   sender         GuiProtMessageSender
    * @param   newsData       Pointer to latest news data.
    */
   WFStartupLatestNewsShown(int32 nextEvent,
         class GuiProtMessageSender* sender,
         class WFNewsData* newsData) :
        m_nextEvent(nextEvent), m_sender(sender), m_newsData(newsData) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for the latest news 
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_nextEvent    -        Event to trigger when request has been sent.
    */
   int32 m_nextEvent;
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_newsData     -        Pointer to object holding the latest news data.
    */
   class WFNewsData* m_newsData;
};

/**
 *  WFStartupGetIAPId   -     Startup event for getting IAP id from Nav2.
 *                            The object will trigger either the
 *                            SS_IapSearch or the SS_IapOk event when
 *                            the reply is received.
 */
class WFStartupGetIAPId
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate member variables.
    * @param   sender         GuiProtMessageSender
    * @param   iapStore       Pointer to IAP holder data.
    */
   WFStartupGetIAPId(class GuiProtMessageSender* sender,
                     class IAPDataStore* iapStore) :
        m_sender(sender), m_iapStore(iapStore) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for the iap id parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupGetIAPId();

   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_iapStore     -        Pointer to IAP data store.
    */
   class IAPDataStore* m_iapStore;
};

/**
 * WFStartupFavoriteSync   -     Startup event for sending favorite sync
 *                               to Nav2 and receiving the result.
 */
class WFStartupFavoriteSync
   : public WFStartupSendParamBase
{
public:
   /**
    * Constructor    -        Initiate base class
    * @param   sender         GuiProtMessageSender
    * @param   nextEvent      Event to be triggered when reply is received.
    */
   WFStartupFavoriteSync(class GuiProtMessageSender* sender, int32 nextEvent) :
        WFStartupSendParamBase(sender, nextEvent), m_nbrSyncs(0) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for favorite
    *                         synchronization.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);

   /**
    * m_nbrSyncs -            Number of times to sync the favorites before 
    *                         giving up, used to delay the startup a little bit.
    */
   int32 m_nbrSyncs;
};

/**
 * WFStartupAudioScripts   -  Startup event for sending audio script
 *                            initiation message to Nav2.
 */
class WFStartupAudioScripts
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -        Initiate object
    * @param   sender         GuiProtMessageSender
    * @param   audioPath      Path to audio files
    */
   WFStartupAudioScripts(class GuiProtMessageSender* sender, char *audioPath) :
        m_sender(sender), m_audioPath(audioPath) {}
   /**
    * Destructor
    */
   virtual ~WFStartupAudioScripts();
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 to initiate audio scripts.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_audioPath    -        Path to audio files.
    */
   char* m_audioPath;
};

/**
 * WFStartupUserTermsAccepted - Used for sending request for user terms
 *                              parameter.
 */
class WFStartupUserTermsAccepted
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -        Initiate object
    * @param   sender         GuiProtMessageSender
    * @param   nextEvent      Next event to trigger
    */
   WFStartupUserTermsAccepted(class GuiProtMessageSender* sender,
         int32 nextEvent)
    : m_sender(sender), m_nextEvent(nextEvent) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 to get the user terms
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_nextEvent -           Id of event to trigger when the message has
    *                         been sent.
    */
   int32 m_nextEvent;
};

/**
 * WFStartupNextEvent   -     Simple object which just triggers another event.
 *                            The object can also be initiated with a list
 *                            of events, these will be triggered in succession.
 *                            Note: parameter pointer is copied which means that
 *                            all triggered events receives the same data and makes
 *                            it impossible to easily handle deletion. Beware!
 */
class WFStartupNextEvent
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -        Initiate object
    * @param   nextEvent      Next event to trigger
    */
   WFStartupNextEvent(int32 nextEvent);
   /**
    * Constructor    -        Initiate object
    * @param   eventList      List of event to trigger
    */
   WFStartupNextEvent(std::deque<int32>& eventList);
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Triggers the events specified in the
    *                         m_eventPending list in succession.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * Destructor     -        Erases m_eventPending list.
    */
   virtual ~WFStartupNextEvent();
private:
   /**
    * m_eventPending -        List of events to trigger.
    */
   std::deque<int32> m_eventPending;
};

/**
 * WFStartupUiCallback  -     Startup event for doing a callback to
 *                            the UI.
 */
class WFStartupUiCallback
   : public WFStartupEventCallback
{
public:
   /**
    * WFStartupUiCallback  -  Constructor.
    */
   WFStartupUiCallback() {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Performs a callback to the UI with the same
    *                         event id.
    * @param   eventType      Event used as parameter to the callback.
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
};

/**
 * WFStartupGotoServiceViewUrl - StartupEvent for showing a specific webpage.
 */
// class WFStartupGotoServiceViewUrl
//    : public WFStartupEventCallback
// {
// public:
//    /**
//     * WFStartupGotoServiceViewUrl - Constructor. Performs a copy of the url
//     *                               and fills in the correct language code
//     *                               by replacing the first occurrence of XX.
//     * @param viewHandler      Handles the view changing
//     * @param url              URL to show
//     * @param langCode         Language code to insert into URL
//     * @param backAction       Which action to perform when "back" is clicked.
//     *                         -1 means BackIsExit.
//     */
//    WFStartupGotoServiceViewUrl(class WFServiceViewHandler* viewHandler,
//       const char *url, const char *langCode, int32 backAction = -1);
//    /**
//     * From WFStartupEventCallback:
//     * EventCallback  -        Will use the m_viewHandler to show the URL to
//     *                         the user.
//     * @param   eventType      Event type. (unused)
//     * @param   param          Optional parameter. (unused)
//     */
//    virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
//    /**
//     * Destructor     -        Cleans up m_url.
//     */
//    virtual ~WFStartupGotoServiceViewUrl();
// private:
//    /**
//     * m_viewHandler  -        Pointer to an object that can perform the action
//     *                         of changing views to show the URL.
//     */
//    class WFServiceViewHandler* m_viewHandler;
//    /**
//     * m_url          -        The url to show.
//     */
//    char *m_url;
//    /**
//     * m_backAction   -        Action to perform when back is selected.
//     */
//    int32 m_backAction;
// };

/**
 * WFStartupChoiceEvent -     Startup event which can take two list of events,
 *                            and when an event on the first list is received,
 *                            the event with the same index in the second list
 *                            is triggered.
 */
class WFStartupChoiceEvent
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -           Initiate object
    * @param   eventList         List of events to receive
    * @param   resultEventList   List of events to trigger
    */
   WFStartupChoiceEvent(
         std::list<int32>& eventList,
         std::list<int32>& resultEventList) :
         m_eventList(eventList), m_resultList(resultEventList)
         { }
   /**
    * Destructor     -        Cleans up lists.
    */
   virtual ~WFStartupChoiceEvent();
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Will trigger an event with the same index from
    *                         the second list.
    * @param   eventType      Event to lookup in first list.
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_eventList    -        First list (incoming events)
    */
   std::list<int32> m_eventList;
   /**
    * m_resultList   -        Second list (outgoing events)
    */
   std::list<int32> m_resultList;
};

/**
 * WFStartupChoiceEvent -     Startup event which will trigger an event when
 *                            all the events in a list has been received.
 */
class WFStartupAndEvent
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -           Initiate object
    * @param   nextEvent         Event to trigger when all events in 
    * @param   eventList         List of events to receive
    */
   WFStartupAndEvent(int32 nextEvent,
         std::list<int32>& eventList) :
         m_nextEvent(nextEvent),
         m_eventList(eventList)
         { }
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Will trigger the next event when all the events
    *                         in the list has been received.
    * @param   eventType      Event which has been received.
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * Destructor     -        Cleans up list.
    */
   virtual ~WFStartupAndEvent();
   /**
    * m_nextEvent    -        Event to trigger when all the events in the list
    *                         has been received.
    */
   int32 m_nextEvent;
   /**
    * m_eventList    -        List of incoming events that needs to be received
    *                         before the next event is triggered.
    */
   std::list<int32> m_eventList;
};

/**
 * WFStartupOrEvent  -        Startup event which can be used as an "if"
 *                            statement.
 *                            The class remembers which of the events in
 *                            the first list that has occurred last and
 *                            the event with the same index in the second list
 *                            is set as next event.
 *                            However, it triggers the next event only when the
 *                            the event specified as trigger is received.
 *                            If the trigger is received before any of the
 *                            events in the first list, that is remembered,
 *                            and the correct event is triggered when the
 *                            next event in the first list is received.
 *                            Uses the WFStartupChoiceEvent as a base class.
 */
class WFStartupOrEvent
 : public WFStartupChoiceEvent
{
public:
   /**
    * Constructor    -           Initiate object
    * @param   nextEvent         Event to trigger when all events in 
    * @param   eventList         List of events to receive
    */
   WFStartupOrEvent(int32 trigger,
         std::list<int32> &eventList, std::list<int32> &resultList) :
      WFStartupChoiceEvent(eventList, resultList),
      m_trigger(trigger), m_nextEvent(-1) { }
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Will trigger an event with the same index from
    *                         the second list, but only when the m_trigger
    *                         event has been received.
    * @param   eventType      Event which has been received.
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_trigger      -        Trigger event
    */
   int32 m_trigger;
   /**
    * m_nextEvent    -        Event to trigger
    */
   int32 m_nextEvent;
};

/**
 * WFStartupUserTermsAccepted - Used for saving parameter to avoid showing
 *                              US disclaimer again.
 */
class WFStartupSaveUSDisclaimer
   : public WFStartupEventCallback
{
public:
   /**
    * Constructor    -        Initiate object
    * @param   sender         GuiProtMessageSender
    * @param   nextEvent      Next event to trigger
    */
   WFStartupSaveUSDisclaimer(class GuiProtMessageSender* sender,
         int32 nextEvent)
    : m_sender(sender), m_nextEvent(nextEvent) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Save parameter to indicate that US disclaimer
    *                         should never be shown again.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_nextEvent -           Id of event to trigger when the message has
    *                         been sent.
    */
   int32 m_nextEvent;
};

/**
 * WFStartupParamSync   -     Startup event for sending a param sync
 *                            to Nav2 and receiving the result.
 */
class WFStartupParamSync
   : public WFStartupSendParamBase
{
public:
   /**
    * Constructor    -        Initiate base class
    * @param   sender         GuiProtMessageSender
    * @param   nextEvent      Event to be triggered when reply is received.
    */
   WFStartupParamSync(class GuiProtMessageSender* sender, int32 nextEvent) :
        WFStartupSendParamBase(sender, nextEvent) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 for parameter
    *                         synchronization.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);

   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
};

/**
 *  WFStartupChangeUin       -  WFStartupEventCallback for taking care of
 *                              wayfinder change uin requests from web.
 */
class WFStartupChangeUin
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupChangeUin(class GuiProtMessageSender* sender,
                      class WFServiceViewHandler* viewHandler) :
      m_sender(sender), m_viewHandler(viewHandler), m_changeUinParams(NULL) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 with the wayfinder license
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. Used for uin parameters.
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupChangeUin();
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_viewHandler  -        Pointer to an object that can perform the action
    *                         of changing views to show the URL.
    */
   class WFServiceViewHandler* m_viewHandler;
   /**
    * m_upgradeParams -       Parameter containing the uin and url info. 
    */
   class WFStartupEventStringAndUrlsCallbackParameter* m_changeUinParams;
};

/**
 *  WFStartupSetServerList   -  WFStartupEventCallback for taking care of
 *                              wayfinder change server list requests.
 */
class WFStartupSetServerList
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupSetServerList(class GuiProtMessageSender* sender,
                          class WFServiceViewHandler* viewHandler) :
      m_sender(sender), m_viewHandler(viewHandler), m_serverListParams(NULL) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 with the wayfinder license
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. Used for server list parameters.
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupSetServerList();
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_viewHandler  -        Pointer to an object that can perform the action
    *                         of changing views to show the URL.
    */
   class WFServiceViewHandler* m_viewHandler;
   /**
    * m_upgradeParams -       Parameter containing the server list and url info. 
    */
   class WFStartupEventStringAndUrlsCallbackParameter* m_serverListParams;
};

/**
 *  WFStartupSendNop       -  WFStartupEventCallback for taking care of
 *                            wayfinder NOP requests.
 */
class WFStartupSendNop
   : public WFStartupEventCallback,
     public GuiProtMessageReceiver
{
public:
   /**
    * Constructor    -        Initiate base class
    */
   WFStartupSendNop(class GuiProtMessageSender* sender,
                    class WFServiceViewHandler* viewHandler,
                    int32 nextEvent) :
      m_sender(sender), m_viewHandler(viewHandler), m_nextEvent(nextEvent) {}
   /**
    * From WFStartupEventCallback:
    * EventCallback  -        Sends a request to Nav2 with the wayfinder license
    *                         parameter.
    * @param   eventType      Event type. (unused)
    * @param   param          Optional parameter. (unused)
    */
   virtual void EventCallback(int32 eventType, class WFStartupEventCallbackParameter* param);
   /**
    * From GuiProtMessageReceiver:
    * GuiProtReceiveMessage - Called by the GuiProtMessageSender when the
    *                         reply that matched the sent message arrives.
    * @param   mess           Message reply.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess *mess);
   /**
    * Destructor  -           Takes care of cleanup, and most importantly,
    *                         makes sure that this object is removed from
    *                         GuiProtMessageSender's list of objects waiting
    *                         for reply.
    */
   virtual ~WFStartupSendNop();
   /**
    * m_sender       -        Pointer to GuiProtMessageSender
    */
   class GuiProtMessageSender* m_sender;
   /**
    * m_viewHandler  -        Pointer to an object that can perform the action
    *                         of changing views to show the URL.
    */
   class WFServiceViewHandler* m_viewHandler;
   /**
    * m_nextEvent -           Id of event to trigger when the reply of the
    *                         message has been received.
    */
   int32 m_nextEvent;
};

#endif  /* WF_COMMON_STARTUP_EVENTS_H */


