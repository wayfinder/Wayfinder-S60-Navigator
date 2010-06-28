/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TcpSocketEngine.h"
#ifdef RCONNECTION
# if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
#  include <apengineconsts.h>
# endif
#endif
#include "TimeOutTimer.h"
#include "TcpSocketReader.h"
#include "TcpSocketWriter.h"
#include "WFTextUtil.h"
#include "CleanupSupport.h"
#include "SocketUser.h"
#include "DNSCache.h"

#include "Log.h"
#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"

CTcpSymbianEngine::CTcpSymbianEngine(class MTcpSymbianSocketUser& aConsole) : 
   CActive(EPriorityStandard), iEngineStatus(EComplete), iConsole(aConsole),
   iTimeOut(KTimeOut)
{
}

class CTcpSymbianEngine* CTcpSymbianEngine::NewL(class MTcpSymbianSocketUser& aConsole, 
                                                 class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianEngine* self = NewLC(aConsole, aLogMaster);
   CleanupStack::Pop(self);
   return self;
}

class CTcpSymbianEngine* CTcpSymbianEngine::NewLC(class MTcpSymbianSocketUser& aConsole, 
                                                  class isab::LogMaster* aLogMaster)
{
   class CTcpSymbianEngine* self = new(ELeave) CTcpSymbianEngine(aConsole);
   CleanupStack::PushL(self);
   self->ConstructL(aLogMaster);
   return self;
}

CTcpSymbianEngine::~CTcpSymbianEngine()
{
   DBG("~CTcpSymbianEngine");
   iDNSCache.Reset();
   iDNSCache.Close();
   iEchoSocket.Close();
   CloseLinkLayer();
   delete iEchoRead;
   delete iEchoWrite;
   delete iTimer;
   iSocketServ.Close();
   iCritical.Close();
   DBG("~CTcpSymbianEngine done");
   delete iLog;
}

void CTcpSymbianEngine::ConstructL(class isab::LogMaster* aLogMaster)
{
   if(aLogMaster){
      iLog = new (ELeave) isab::Log("TcpSymbianEngine", isab::Log::LOG_ALL, aLogMaster);
   }

   User::LeaveIfError(iCritical.CreateLocal());
   // Open channel to Socket Server
   User::LeaveIfError(iSocketServ.Connect());

   //best to have these after the socket is opened. just in case. 
   iTimer = CTimeOutTimer::NewL(EPriorityHigh, *this);
   iEchoRead = CTcpSymbianRead::NewL(iEchoSocket, iConsole, *this, aLogMaster);
   iEchoWrite = CTcpSymbianWrite::NewL(iEchoSocket, iConsole, *this, aLogMaster);

   CActiveScheduler::Add(this);
   DBG("Fully constructed");
}

void CTcpSymbianEngine::OpenSocketL()
{
   User::LeaveIfError(OpenSocket());
}

TInt CTcpSymbianEngine::OpenSocket()
{
#if defined NAV2_CLIENT_SERIES60_V3 && !defined __WINS__ && !defined __WINSCW__
   TInt ret = iEchoSocket.Open(iSocketServ, KAfInet, 
                               KSockStream, KProtocolInetTcp, iConnection);
#else
   TInt ret = iEchoSocket.Open(iSocketServ, KAfInet, 
                               KSockStream, KProtocolInetTcp);
#endif
   DBG("OpenSocket returns %d", ret);
   return ret;
}

void CTcpSymbianEngine::DoCancel()
   // Cancel asychronous requests
{
   DBG("DoCancel");
   iTimer->Cancel();
   // Cancel appropriate request to socket
   //XXX handle bad opens!
   switch (EngineState()){
   case EConnecting:
      DBG("While EConnecting");
      iEchoSocket.CancelConnect();
      break;
   case ELookingUp:
      DBG("While ELookingUp");
      // Cancel look up attempt
      iResolver.Cancel();
      iResolver.Close();
      break;
   case ELinking:
      DBG("While ELinking");
      CloseLinkLayer();
      break;
   case EClosing:
      DBG("While EClosing");
      break;
   default:
      ERR("PANIC KEchoEngine, EBadCancelStatus (%d)", TInt(EBadCancelStatus));
      User::Panic(KEchoEngine, EBadCancelStatus);
      break;
   }
   DBG("DoCancel comnplete");
}

void CTcpSymbianEngine::Connect(TUint32 aAddr, TUint aPort)
   // Connect to an Echo Socket by IP address	
{
   DBG("Connect(0x%"PRIx32", %u)", aAddr, aPort);
   iAddress.SetPort(aPort);
   iAddress.SetAddress(aAddr);

   iEchoSocket.Connect(iAddress, iStatus);
   SetEngineState(EConnecting);
   //beware deadlocks! notify outside critical regions. 
   iConsole.ConnectionNotify(isab::Module::CONNECTING,
                              isab::Module::REQUESTED);

   SetActive();
   iTimer->After(iTimeOut);
}

void CTcpSymbianEngine::ConnectL(const TDesC8& aServerName, TUint aPort, 
                                 TInt aIAP, TBool aConsiderWLAN)
{
   DBG("ConnectL(%*s, %u, %d)", aServerName.Length(), aServerName.Ptr(),
       aPort, aIAP);
   HBufC* wide = HBufC::NewLC(aServerName.Length() + 4);
   TPtr wide_p = wide->Des();
   WFTextUtil::TDesWiden(wide_p, aServerName);
   ConnectL(*wide, aPort, aIAP, aConsiderWLAN);
   CleanupStack::PopAndDestroy(wide);
}

namespace {
   /**
    * This function replaces the content of aDst with the content of
    * aSrc, reallocating if necessary. If aDst is NULL a new HBufC
    * will be allocated. Otherwise reallocation will only occur if the
    * content of aSrc will not fit in aDst.
    * @param aDst The destination HBufC. Note that this is a pointer
    *             reference.
    * @param aSrc The source descriptor.
    */
   void ReplaceL(HBufC*& aDst, const TDesC& aSrc)
   {
      if(!aDst || *aDst != aSrc){
         if(aDst && aDst->Des().MaxLength() >= aSrc.Length()){
            aDst->Des() = aSrc;
         } else {
            delete aDst;
            aDst = NULL;
            aDst = aSrc.AllocL();
         }
      }
   }
}

void CTcpSymbianEngine::SetConnectionParamsL(const TDesC& aServerName, 
                                             TUint aPort, TInt aIAP, 
                                             TBool aConsiderWLAN)
{
   iPort = aPort;
   iIAP = aIAP;
   iConsiderWLAN = aConsiderWLAN;
   ReplaceL(iHostName, aServerName);
}

void CTcpSymbianEngine::ConnectL(const TDesC& aServerName, TUint aPort, 
                                 TInt aIAP, TBool aConsiderWLAN)
{
   DBG("ConnectL(HOST, %u, %d)", aPort, aIAP);
   //see if state is EComplete. used several times below
   const TBool stateComplete = (EngineState() == EComplete);
   //The changeServer variable indicates that we must close the
   //current connection and open a new one.
   const TBool changeServer = !stateComplete && iHostName && 
      (*iHostName != aServerName || iPort != aPort);
   DBG("stateComplete: %d, changeServer: %d", stateComplete, changeServer);
   if(changeServer){
      WARN("Switching server, danger will robinson");
      Stop(ESwitching);
      SetConnectionParamsL(aServerName, aPort, aIAP, aConsiderWLAN);      
   } else {
      DBG("No need to change server");
      if(stateComplete){
         DBG("update params");
         SetConnectionParamsL(aServerName, aPort, aIAP, aConsiderWLAN);
      } 
      ConnectL();
   }
}

void CTcpSymbianEngine::ConnectL()
{
   DBG("ConnectL()");
   if(EngineState() == EComplete){
      iConsole.ConnectionNotify( isab::Module::CONNECTING,
                                 isab::Module::REQUESTED );
      OpenLinkLayerL(); //setactive here
   } else {
      DBG("Do nothing, but handle the next request.");
      if(!IsActive()){ //don't think this is needed
         DBG("Signal OperationComplete");
         iConsole.OperationComplete();
      } else {
         WARN("This was unexpected!");
      }
   }   
}

void CTcpSymbianEngine::StartLookupL()
{
   DBG("StartLookupL");
   SetEngineState(ELookingUp);
   // Initiate DNS
   if(KErrNone == 
#if defined NAV2_CLIENT_SERIES60_V3  && !defined __WINS__ && !defined __WINSCW__
      iResolver.Open(iSocketServ, KAfInet,
                     KProtocolInetUdp, iConnection)
#else
      iResolver.Open(iSocketServ, KAfInet, KProtocolInetUdp)
#endif
      ){

      if(KErrNone == iDNSCache.CacheHostLookup(*iHostName, iNameEntry, iPort)){
         DBG("Host found in cache");
         CompleteSelf(KErrNone);
      } else {
         DBG("Host not found in cache, looking up in DNS");
         // DNS request for name resolution
         iResolver.GetByName(*iHostName, iNameEntry, iStatus);
         SetActive();
      }
      // Request time out
      iTimer->After(iTimeOut);
      //signal state
      iConsole.ConnectionNotify(isab::Module::CONNECTING,
                                 isab::Module::REQUESTED);
      DBG("Starting asynchronous name lookup");
   } else {
      WARN("Lookup failed");
      ResetL(ELookUpFailed, isab::Module::LOOKUP_FAILED);
   }
}

void CTcpSymbianEngine::OpenLinkLayerL()
{
   DBG("OpenLinkLayerL");
   // Select IAP
   SetEngineState(ELinking);
#if defined RCONNECTION || defined RAGENT
   if(iIAP == -1){
      iConsole.ConnectionNotify(isab::Module::WAITING_FOR_USER,
                                 isab::Module::REQUESTED);
   }
#endif

#ifdef RCONNECTION
   OpenLinkLayerConnectionL(); //setactive here
#elif defined RAGENT
   OpenLinkLayerAgentL();
#else
   OpenFakeLinkLayerL();
#endif
   SetEngineState(ELinking);
   // Request time out
   iTimer->After(iTimeOut);
}

void CTcpSymbianEngine::OpenFakeLinkLayerL()
{
   DBG("Faking Link layer by async completion");
   CompleteSelf(KErrNone);
}

void CTcpSymbianEngine::OpenLinkLayerConnectionL()
{
#ifdef RCONNECTION
   DBG("OpenLinkLayerConnectionL");
# if defined __WINS__ || defined __WINSCW__
   OpenFakeLinkLayerL();
# else
   User::LeaveIfError(iConnection.Open(iSocketServ));

   iPrefs = TCommDbConnPref();
   
   DBG("iIAP == %d", iIAP);
   if (iIAP >= 0) {
      iPrefs.SetIapId(iIAP);
      iPrefs.SetDialogPreference(ECommDbDialogPrefDoNotPrompt);
      iConnection.Start(iPrefs, iStatus);
   } else if (iIAP == -2) {
      // Debug mode - use system default
      iConnection.Start(iStatus);
   } else { //iIAP == -1
      // Use Always ask
      iPrefs.SetDialogPreference(ECommDbDialogPrefPrompt);
      iPrefs.SetDirection(ECommDbConnectionDirectionOutgoing);
#  if   defined NAV2_CLIENT_SERIES60_V2 
      iPrefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD );
#  elif defined NAV2_CLIENT_SERIES60_V3
      iPrefs.SetBearerSet( EApBearerTypeGPRS | EApBearerTypeHSCSD | 
                           EApBearerTypeCDMA );
#  elif defined NAV2_CLIENT_UIQ3
      iPrefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma | 
                           ECommDbBearerCdma2000 | ECommDbBearerPSD | 
                           (iConsiderWLAN ? (ECommDbBearerWLAN | ECommDbBearerLAN) : 0) );                          
#  else //s80, s90
      iPrefs.SetBearerSet( ECommDbBearerCSD | ECommDbBearerWcdma | 
                           ECommDbBearerCdma2000 | ECommDbBearerPSD );
      
#  endif
      iConnection.Start(iPrefs, iStatus);
   }
   SetActive();
   DBG("Starting asynchronous link layer setup");
# endif
#else
   ERR("OpenLinkLayerConnectionL");
#endif
}

void CTcpSymbianEngine::OpenLinkLayerAgentL()
{
#ifdef RAGENT
   DBG("OpenLinkLayerAgentL");
   iAgent = new (ELeave) RGenericAgent;
   User::LeaveIfError(iAgent->Open());
   // Series60 release 1 (eg. Nokia 7650/3650)
   //
   TBool isUp=EFalse;
   iAgent->NetworkActive(isUp);
   if (!isUp) {
      // Attempt to open a connection using the selected IAP
      delete iCommsOverrides;
      iCommsOverrides = NULL;
# ifdef NAV2_CLIENT_UIQ
      iCommsOverrides = 
         CStoreableOverrideSettings::NewL(
            CStoreableOverrideSettings::EParamListPartial );
# else //s60v1
      iCommsOverrides = 
         CStoreableOverrideSettings::NewL(
            CStoreableOverrideSettings::EParamListPartial,
            EDatabaseTypeIAP);
# endif
      iPrefs = CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref();
      iPrefs.iRanking = 1;
      iPrefs.iDirection = ECommDbConnectionDirectionOutgoing;
      iPrefs.iBearer.iBearerSet = ECommDbBearerUnknown;
      if(iIAP != -2){
         if (iIAP >= 0) {
            iPrefs.iDialogPref = ECommDbDialogPrefDoNotPrompt;
            iPrefs.iBearer.iIapId = iIAP;
         } else {
            // Use Always ask
            iPrefs.iDialogPref = ECommDbDialogPrefPrompt;
            iPrefs.iBearer.iIapId = 0;
         }
         iCommsOverrides->SetConnectionPreferenceOverride(iPrefs);
         iAgent->StartOutgoing(*iCommsOverrides, iStatus);
         SetActive();
         DBG("Starting async link layer connection");
      } 
   }
   if(!IsActive()){
      //if IsActive returns EFalse the network is either already up or
      //we should use system default (iap = -2), in either case
      //complete immediately
      DBG("Link layer already active, complete immediately. ");
      CompleteSelf(KErrNone);
   }
#else 
   ERR("OpenLinkLayerAgentL");
#endif
}

void CTcpSymbianEngine::CloseLinkLayer()
{
   DBG("CloseLinkLayer");
#ifdef RCONNECTION
   iConnection.Close();
#elif defined RAGENT
   if(iAgent){
      iAgent->Close();
      delete iAgent;
      iAgent = NULL;
   }
#else
   //do nothing 
#endif
   DBG("CloseLinkLayer complete");
}

TBool CTcpSymbianEngine::IsConnected()
{
   return EngineState() == EConnected;
}

void CTcpSymbianEngine::Query()
{
   DBG("Query");
   //TODO: keep the disconnect reasons somewhere, we can give better
   //indication that way.
   enum isab::Module::ConnectionNotify state = isab::Module::CLEAR;
   enum isab::Module::ConnectionNotifyReason reason = isab::Module::REQUESTED;
   switch(EngineState()){
   case EComplete:                                               break;
   case EConnecting:    state  = isab::Module::CONNECTING;       break;
   case EConnected:     state  = isab::Module::CONNECTED;        break;
   case ETimedOut:      state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::UNSPECIFIED;      break;
   case ELinking:       state  = isab::Module::CONNECTING;       break;
   case ELinkFailed:    state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::TRANSPORT_FAILED; break;
   case ELookingUp:     state  = isab::Module::CONNECTING;       break;
   case ELookUpFailed:  state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::LOOKUP_FAILED;    break;
   case EConnectFailed: state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::NO_RESPONSE;      break;
   case EClosing:       state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::REQUESTED;        break;
   case EBroken:        state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::CHANNEL_FAILED;   break;
   case ESwitching:     state  = isab::Module::CONNECTING;
                        reason = isab::Module::REQUESTED;        break;
   case EClosed:        state  = isab::Module::DISCONNECTING;
                        reason = isab::Module::BY_PEER;   break;
   }
   iConsole.ConnectionNotify(state, reason);
}


void CTcpSymbianEngine::Write(HBufC8* aData)
{
   if(EngineState() != EConnected || iEchoWrite->IsActive()){
      ERR("EngineState: %d, Echowrite is %sactive", EngineState(), iEchoWrite->IsActive() ? "": "not");
   }
   __ASSERT_ALWAYS(EngineState() == EConnected, 
                   User::Panic(KEchoEngine, EWriteNotConnected));
   __ASSERT_ALWAYS(!iEchoWrite->IsActive(), 
                   User::Panic(KEchoEngine, EWriteActive));
   //we should never get here unless CTcpSymbianEngine is ready for
   //another write.  See assertations above.
   if ((EngineState() == EConnected) && !iEchoWrite->IsActive()){
      iEchoWrite->IssueWrite(aData);
   } else {
      ERR("Write issued while not connected or echowrite is active");
   }
}

void CTcpSymbianEngine::Read()
{
   __ASSERT_ALWAYS(EngineState() == EConnected, 
                   User::Panic(KEchoEngine, EReadNotConnected));
   ///if we are already active, that's not a problem.
   if ((EngineState() == EConnected) && (!iEchoRead->IsActive())) {
      iEchoRead->IssueRead();
   } else {
      ERR("Read issued while not connected ot echoread is active");
   }
}

void CTcpSymbianEngine::CompleteSelf(TInt aCompleteStatus)
{
   iStatus = KRequestPending;
   SetActive();
   class TRequestStatus* p = &iStatus;		
   User::RequestComplete(p, aCompleteStatus);   
}


void CTcpSymbianEngine::RunL()
{
   DBG("RunL, EngineState: %d, iStatus: %d", EngineState(), iStatus.Int());
   iTimer->Cancel(); // Cancel TimeOut timer before handling completion
   switch(EngineState()){
   case EConnecting:
      // IP connection request
      if (iStatus == KErrNone){
         DBG("Connection completed Ok");
         // Connection completed sucessfully   
         iConsole.ConnectionNotify(isab::Module::CONNECTED,
                                    isab::Module::REQUESTED);
         SetEngineState(EConnected);
         Read(); //Start CTcpSymbianRead Active object
      } else {
         WARN("Unable to connect, removing host from cache.");
         iDNSCache.Remove(*iHostName);
         if (iStatus == KErrCouldNotConnect){
            ResetL(EConnectFailed, isab::Module::NO_RESPONSE);
         } else if (iStatus == KErrTimedOut) {
            ResetL(ETimedOut, isab::Module::NO_RESPONSE);
         } else{
            WARN("EConnectFailed, INTERNAL_ERROR");
            ResetL(EConnectFailed, isab::Module::INTERNAL_ERROR);
            //XXX handle the different cases
         }
      }
      break;
   case ELookingUp:
      iResolver.Close();
      if (iStatus == KErrNone) {
         DBG("Name lookup ok");
         // DNS look up successful
         // Extract domain name and IP address from name record
         class TInetAddr addr = TInetAddr::Cast(iNameEntry().iAddr);
         //cache the ip for later.
         TInt cacheStat = iDNSCache.CacheHost(*iHostName, addr.Address());
         DBG("Host cached with result %d", cacheStat);
         cacheStat = cacheStat;
         // And connect to the IP address
         Connect(addr.Address(), iPort); //setactive here
      } else { //includes KErrTimedOut
         WARN("Lookup failed");
         ResetL(ELookUpFailed, isab::Module::LOOKUP_FAILED);
      }
      break;
   case ELinking:
      if(iStatus == KErrNone){
         DBG("Link layer setup ok");
         OpenSocketL();  //cannot open socket until here. We need RConnection.
         StartLookupL(); //setactive here
      } else { //includes KErrCanceled and KErrTimedOut
         WARN("Link layer setup failed");
         ResetL(ELinkFailed, isab::Module::TRANSPORT_FAILED);
      }
      break;
   case EBroken:
      DBG("Broken");
      ResetL( EngineState(), isab::Module::CHANNEL_FAILED );
      break;
   case EClosing:
      if(iStatus == KErrNone){
         DBG("Closing ok");
         ResetL(EComplete, isab::Module::REQUESTED);
      } else if(iStatus == KErrTimedOut){
         ResetL(ETimedOut);
      } else {
         ResetL(EngineState());
         //XXX ???
      }
      break;
   case ESwitching:
      if(iStatus == KErrNone){
         //the connection has been closed, start a new connection
         //using the values in iHostName, iPort, and iIAP.
         SetEngineState(EComplete);
         ConnectL();
      } else if(iStatus == KErrTimedOut){
         ResetL(ETimedOut);
      } else {
         ResetL(EngineState());
      }
      break;
   case EClosed:
      DBG("Closed");
      ResetL( EngineState(), isab::Module::BY_PEER );
      break;
   default:
      ERR("PANIC KEchoEngine EBadEngineState (%d)", TInt(EBadEngineState));
      User::Panic(KEchoEngine, EBadEngineState);
      break;
   };
   if(!IsActive()){
      DBG("Signal OperationComplete");
      iConsole.OperationComplete();
      DBG("Signaled OperationComplete");
   }
}

TInt CTcpSymbianEngine::RunError(TInt aError)
{
   //RunL contains two functions that may leave: ResetL and StartLookupL.
   //ResetL will leave if RSocket::Open fails. 
   //StartLookupL will leave if ResetL leaves. 
   //Hey, I see a pattern here!
   
   //In other words, any time we end up here it's because a call to
   //RSocket::Open has failed. This is very, very bad. It's very
   //probable that we will not be able to recover from this. We might
   //as well give up and start farming chipmunks.

   ERR("RunError %d", aError);
   iConsole.Panic(aError);

   //Return KErrNone here to placate the active scheduler. 
   return KErrNone;
}


void CTcpSymbianEngine::TimerExpired()
{
   DBG("TimerExpired");
   Cancel();
   CompleteSelf(KErrTimedOut);
}

void CTcpSymbianEngine::Stop()
   // Shutdown connection request
{
   Stop(EClosing);
}

void CTcpSymbianEngine::Stop(enum TEchoEngineState aState)
{
   enum TEchoEngineState state = EngineState();
   DBG("Stop, Enginestate: %d, aState: %d", state, aState);
   if(state != EComplete){
      iConsole.BlockEvents();
   }
   switch (state){
   case EConnected:
      // Stop live connection
      iEchoRead->Cancel();
      iEchoWrite->Cancel();
      SetEngineState(aState);
      iEchoSocket.Shutdown(RSocket::EImmediate, iStatus);
      SetActive();
      iTimer->After(iTimeOut);
      break;
   case EConnecting:
   case ELookingUp:
   case ELinking:
      // if request to CTcpSymbianEngine, then stop it
      Cancel();
      break;
   case ESwitching:
      SetEngineState(EClosing);
      break;
   case EClosing:
      ///already closing. 
      break;
   case EComplete:
      //all set.
      if(!IsActive()){
         DBG("Signal OperationComplete");
         iConsole.OperationComplete();
         DBG("Signaled OperationComplete");
      }
      break;
   default:
      ERR("PANIC KEchoEngine EBadEngineStopState (%d)", EBadEngineStopState);
      User::Panic(KEchoEngine, EBadEngineStopState);
      break;
   }
}

void CTcpSymbianEngine::ResetL(enum TEchoEngineState aDuring)
{
   DBG("ResetL(%d)", aDuring);
   SetEngineState(aDuring);   //temporary state
   iEchoSocket.Close();       //close the socket..
   CloseLinkLayer();          //..and the link layer
   SetEngineState(EComplete); //ready for new connection attempts
}


void CTcpSymbianEngine::ResetL(enum TEchoEngineState aDuring, 
                         enum isab::Module::ConnectionNotifyReason aReason)
{
   SetEngineState(aDuring);
   iConsole.ConnectionNotify(isab::Module::DISCONNECTING, aReason);
   ResetL(aDuring);
}


enum CTcpSymbianEngine::TEchoEngineState CTcpSymbianEngine::EngineState()
{
   iCritical.Wait();
   enum TEchoEngineState state = iEngineStatus;
   iCritical.Signal();
   return state;
}

void CTcpSymbianEngine::SetEngineState(enum TEchoEngineState aState)
{
   iCritical.Wait();
   iEngineStatus = aState;
   iCritical.Signal();
}


