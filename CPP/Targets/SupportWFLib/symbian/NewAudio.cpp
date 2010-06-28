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

#include "NewAudio.h"
#include "NewAudioServer.h"
// #include "TraceMacros.h"

// #ifndef USE_TRACE
//  #define USE_TRACE
// #endif

#undef USE_TRACE

#ifdef USE_TRACE
#include "TraceMacros.h"
#endif

namespace {
   _LIT( KNewAudioServerName, "WayfinderNewAudio");
   _LIT( KSemicolon, ";" );
}

_LIT(KNewAudio, "NewAuido" );



#define USE_AUDIO_EXCEPTIONHANDLER

#ifdef USE_AUDIO_EXCEPTIONHANDLER
static void exceptionHandler(TExcType arg)
   {
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif
      const char* eName = NULL;
      switch(arg){
      case EExcGeneral:              eName = "EExcGeneral";              break;
      case EExcIntegerDivideByZero:  eName = "EExcIntegerDivideByZero";  break;
      case EExcSingleStep:           eName = "EExcSingleStep";           break;
      case EExcBreakPoint:           eName = "EExcBreakPoint";           break;
      case EExcIntegerOverflow:      eName = "EExcIntegerOverflow";      break;
      case EExcBoundsCheck:          eName = "EExcBoundsCheck";          break;
      case EExcInvalidOpCode:        eName = "EExcInvalidOpCode";        break;
      case EExcDoubleFault:          eName = "EExcDoubleFault";          break;
      case EExcStackFault:           eName = "EExcStackFault";           break;
      case EExcAccessViolation:      eName = "EExcAccessViolation";      break;
      case EExcPrivInstruction:      eName = "EExcPrivInstruction";      break;
      case EExcAlignment:            eName = "EExcAlignment";            break;
      case EExcPageFault:            eName = "EExcPageFault";            break;
      case EExcFloatDenormal:        eName = "EExcFloatDenormal";        break;
      case EExcFloatDivideByZero:    eName = "EExcFloatDivideByZero";    break;
      case EExcFloatInexactResult:   eName = "EExcFloatInexactResult";   break;
      case EExcFloatInvalidOperation:eName = "EExcFloatInvalidOperation";break;
      case EExcFloatOverflow:        eName = "EExcFloatOverflow";        break;
      case EExcFloatStackCheck:      eName = "EExcFloatStackCheck";      break;
      case EExcFloatUnderflow:       eName = "EExcFloatUnderflow";       break;
      case EExcAbort:                eName = "EExcAbort";                break;
      case EExcKill:                 eName = "EExcKill";                 break;
      case EExcUserInterrupt:        eName = "EExcUserInterrupt";        break;
      case EExcDataAbort:            eName = "EExcDataAbort";            break;
      case EExcCodeAbort:            eName = "EExcCodeAbort";            break;
      case EExcMaxNumber:            eName = "EExcMaxNumber";            break;
      case EExcInvalidVector:        eName = "EExcInvalidVector";        break;
      }
   #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf( errStr, "Caught %s in thread", eName );
      TRACE_FUNC1( errStr );
      delete errStr;
   #endif
      if( arg != EExcPageFault ){
         User::Panic( KNewAudio, arg );
      }
      else{
         User::Exit( arg );
      }
   }
#endif

/**************************************
 * CNewAudioOwner
 **************************************/
class CNewAudioOwner : CBase
{
   public:
      static CNewAudioOwner* NewLC();
      static CNewAudioOwner* NewL();
      virtual ~CNewAudioOwner();
      void ConstructL();

   protected:
      struct NewAudioThreadData {
         const TDesC * serverName;

         // Used by the server thread to signal the 
         // original thread that it is ready to receive
         // requests.
         TRequestStatus *startupStatus;
         TThreadId startupStatusThread;

         // Used by the original thread to signal the 
         // server thread to stop
         TRequestStatus *stopNowStatus;
         TThreadId stopNowStatusThread;
      };

      class CStopWaiter : public CActive
      {
         public:
            static CStopWaiter* NewLC(CNewAudioServer & aServer, NewAudioThreadData & threadData);
            virtual ~CStopWaiter();
            TRequestStatus * getStatusVar();
         protected:
            CStopWaiter(CNewAudioServer & aServer, NewAudioThreadData & threadData);
            void DoCancel();
            void RunL();
            TInt RunError(TInt aError);
         public:
            CNewAudioServer & iServer;
      };

   protected:
      CNewAudioOwner();
      static TInt StartServerThreadFunc(TAny *aPtr);
      static void StartServerThreadFuncStage2(NewAudioThreadData & threadData, 
                                              CNewAudioServer ** server,
                                              CStopWaiter **stopWaiter);
      static void SignalStartupOkFailed(TInt result, NewAudioThreadData & threadData);

   protected:
      // Communication area to the new thread.
      // May be NULL if this object does not own the server.
      // Currently never NULL since that functionallity is not implemented.
      struct NewAudioThreadData *iThreadData;

      RThread iServerThread;
      TRequestStatus iServerExitStatus;
};


void CNewAudioOwner::SignalStartupOkFailed(TInt result, CNewAudioOwner::NewAudioThreadData & threadData)
{
   // NOTE! There may not be a cleanup stack active here and the heap
   // may be full. Be very very careful.
   RThread otherThread;
   otherThread.Open(threadData.startupStatusThread);
   otherThread.RequestComplete(threadData.startupStatus, result);
   otherThread.Close();
}

TInt CNewAudioOwner::StartServerThreadFunc(TAny *aPtr)
{
   NewAudioThreadData * threadData = static_cast<NewAudioThreadData *>(aPtr);
   CTrapCleanup * cleanupStack = CTrapCleanup::New();
   if (NULL == cleanupStack) {
      // Out of memory - return an error message carefully since we 
      // have no cleanupstack.
      SignalStartupOkFailed(KErrNoMemory, *threadData);
      return KErrNoMemory;
   }

   CNewAudioServer * server = NULL;  
   CStopWaiter * stopWaiter = NULL;
   
   TRAPD(trapRes, StartServerThreadFuncStage2(*threadData, &server, &stopWaiter));
   if (trapRes != KErrNone) {
   #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Trap error#: %i", trapRes);
      TRACE_FUNC1( errStr );
      delete errStr;
   #endif
      SignalStartupOkFailed(trapRes, *threadData);
      return trapRes;
   }

   // If we get here then the second stage must have signalled the parent thread.
   // Start the active scheduler to process incoming requests.
   CActiveScheduler::Start();

   ///uninstall and delete the active scheduler.
   class CActiveScheduler* pA = CActiveScheduler::Current();
   CActiveScheduler::Install(NULL);
   delete pA;

   delete stopWaiter;
   delete server;
   delete cleanupStack;
   return KErrNone;
}

void CNewAudioOwner::StartServerThreadFuncStage2(CNewAudioOwner::NewAudioThreadData & threadData,
                                                 CNewAudioServer ** server,
                                                 CNewAudioOwner::CStopWaiter **stopWaiter)
{
   *server = NULL;
   *stopWaiter = NULL;
   
   CActiveScheduler *pA=new (ELeave) CActiveScheduler;
   CActiveScheduler::Install(pA);

   // Increase priority
   {
      RThread thisThread;                // Not pushed on the cleanup stack
      thisThread.SetPriority(EPriorityAbsoluteHigh);
   }

#ifdef USE_AUDIO_EXCEPTIONHANDLER
 #if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   User::SetExceptionHandler(exceptionHandler, MAX_UINT32);
 #else
   RThread me;
   me.SetExceptionHandler(exceptionHandler, MAX_UINT32);
 #endif
#endif

   // Create the server
   CNewAudioServer* tmpServer = CNewAudioServer::NewLC();
   tmpServer->StartL(*(threadData.serverName));

   // Create the StopWaiter and share the data with the 
   // creating thread.
   CStopWaiter* tmpStopWaiter = CStopWaiter::NewLC(*tmpServer, threadData);
   threadData.stopNowStatus = tmpStopWaiter->getStatusVar();
   {
      RThread thisThread;
      threadData.stopNowStatusThread = thisThread.Id();
   }

   // Let the thread that started us know we are up and 
   // running.
   SignalStartupOkFailed(KErrNone, threadData);

   // Hand over ownership to the StartServerThreadFunc function.
   CleanupStack::Pop(tmpStopWaiter);
   CleanupStack::Pop(tmpServer);
   *server = tmpServer;
   *stopWaiter = tmpStopWaiter;
}






// CNewAudioOwner::CStopWaiter helper class
CNewAudioOwner::CStopWaiter* CNewAudioOwner::CStopWaiter::NewLC(
      CNewAudioServer & aServer, CNewAudioOwner::NewAudioThreadData & threadData)
{
   CStopWaiter *ptr = new(ELeave) CStopWaiter(aServer, threadData);
   CleanupStack::PushL(ptr);
   return ptr;
}

CNewAudioOwner::CStopWaiter::~CStopWaiter()
{
   if ( IsActive() ) {
      Cancel();
   }
}

CNewAudioOwner::CStopWaiter::CStopWaiter(
      CNewAudioServer & aServer, CNewAudioOwner::NewAudioThreadData & threadData)
   : CActive(EPriorityStandard), 
     iServer(aServer)
{
   CActiveScheduler::Add(this);
   iStatus = KRequestPending;
   SetActive();

   threadData.stopNowStatus = &iStatus;
   RThread thisThread;
   threadData.stopNowStatusThread = thisThread.Id();
   thisThread.Close();
}

TRequestStatus * CNewAudioOwner::CStopWaiter::getStatusVar()
{
   return &iStatus;
}

void CNewAudioOwner::CStopWaiter::DoCancel()
{
   TRequestStatus* ptr = &iStatus;
   User::RequestComplete( ptr, KErrCancel );   
}

void CNewAudioOwner::CStopWaiter::RunL()
{
   if ( iStatus != KErrCancel ) {
      iServer.StopServerL();
   }
}


TInt CNewAudioOwner::CStopWaiter::RunError(TInt aError)
{
   if( aError != KErrNone ){
   #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Error#: %i", aError);
      TRACE_FUNC1(errStr);
      delete errStr;
   #endif
   }
   return aError;
}



CNewAudioOwner* CNewAudioOwner::NewLC()
{
   CNewAudioOwner *ptr = new (ELeave) CNewAudioOwner();
   CleanupStack::PushL(ptr);
   ptr->ConstructL();
   return ptr;
}

CNewAudioOwner* CNewAudioOwner::NewL()
{
   CNewAudioOwner *ptr = NewLC();
   CleanupStack::Pop(ptr);
   return ptr;
}

CNewAudioOwner::CNewAudioOwner()
   : CBase(),
     iThreadData(NULL)
{
}

void CNewAudioOwner::ConstructL()
{
   TRequestStatus serverStartupStatus;

   iThreadData = new (ELeave) NewAudioThreadData;
   iThreadData->serverName = &KNewAudioServerName;
   iThreadData->startupStatus = &serverStartupStatus;
   RThread thisThread;
   iThreadData->startupStatusThread = thisThread.Id();
   thisThread.Close();

   serverStartupStatus = KRequestPending;

   TInt result;

   result = iServerThread.Create( KNewAudioServerName, 
                         StartServerThreadFunc, 
                         KDefaultStackSize,
                         NULL,     /* RHeap - NULL means use parent thread heap */
                         static_cast<TAny *>(iThreadData) );
   if (result != KErrNone) {
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(result);
   }

   // Set up the server logon. This needs to be before the 
   // resume since the Logon() call signals oom through
   // the request status. If it is done after the Resume 
   // we can not know if a KErrNoMemory comes from the call
   // or the thread.
   iServerThread.Logon(iServerExitStatus);
   if (iServerExitStatus == KErrNoMemory) {
      // Not enough memory to logon to the thread. Kill
      // it and leave.
      iServerThread.Kill(KErrNoMemory);
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(KErrNoMemory);
   }

   iServerThread.Resume();

   // Wait for the server to be established (or failed)
   User::WaitForRequest(serverStartupStatus);

   // Check if the server failed to start. The thread should be dead by 
   // now if so, deallocate it.
   if (serverStartupStatus != KErrNone) {
      // Reap the thread
      User::WaitForRequest(iServerExitStatus);
      delete iThreadData;
      iThreadData = NULL;
      User::Leave(serverStartupStatus.Int());
   }
}

CNewAudioOwner::~CNewAudioOwner()
{
   if (iThreadData) {
      // Stop the server

      // FIXME - race here. What if the server has died and we have not 
      // noticed yet? The threadData.stopNowStatus may be invalid. 
      RThread otherThread;
      if (otherThread.Open(iThreadData->stopNowStatusThread) == KErrNone) {
         otherThread.RequestComplete(iThreadData->stopNowStatus, KErrNone);
      }
      otherThread.Close();
      
      // Wait for the thread to die
      User::WaitForRequest(iServerExitStatus);

      delete iThreadData;
   }
}

/**************************************
 * CNewAudioNoServer
 * Connects to an already existing server
 * thread.
 **************************************/

const TDesC&
CNewAudioNoServer::getTimingMarker()
{
   return CNewAudioServer::getTimingMarker();
}

const TDesC&
CNewAudioNoServer::getEndMarker()
{
   return CNewAudioServer::getEndMarker();
}

CNewAudioNoServer* CNewAudioNoServer::NewLC(MNewAudioObserver & aObs)
{
   CNewAudioNoServer *ptr = new (ELeave) CNewAudioNoServer(aObs);
   CleanupStack::PushL(ptr);
   ptr->ConstructL();
   return ptr;
}

CNewAudioNoServer::CNewAudioNoServer(MNewAudioObserver & aObs)
   : RSessionBase(),
     CActive(EPriorityStandard),
     iObs(aObs),
     iSoundLengthPtr( NULL, 0, 0 ),
     iMsgBuffer(NULL),
     iMsgBufferPtr( NULL, 0, 0 )
{
   iSoundLengthPtr.Set( (TUint8*)&iSoundLength,
                        sizeof(iSoundLength), sizeof(iSoundLength) );
}

void CNewAudioNoServer::ConstructL()
{
   // Connect to the server
   CActiveScheduler::Add(this);
   User::LeaveIfError(CreateSession(KNewAudioServerName, 
                              TVersion(1,0,0), 
                              2)     // Max 2 outstanding requests - one for
                                     // the request and one for a cancel.
               );
}

void
CNewAudioNoServer::Disconnect()
{
   if ( IsActive() ) {
      Cancel();
   }
   
   Close();
   delete iMsgBuffer;
   iMsgBuffer = NULL;
}

CNewAudioNoServer::~CNewAudioNoServer()
{
   Disconnect();
}

void CNewAudioNoServer::DoCancel()
{
   // Send a cancel message to the server
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   TIpcArgs foo(0,0,0,0);       // Dummy data - unused
   TInt err = Send(NewAudioServerTypes::CancelAnyRequest, foo);
#else
   TInt foo[4] = {0,0,0,0};       // Dummy data - unused
   TInt err = Send(NewAudioServerTypes::CancelAnyRequest, foo);
#endif
   if( err != KErrNone ){
      if( err == KErrServerTerminated ){
       #ifdef USE_TRACE
         TRACE_FUNC1( "KErrServerTerminated" );
       #endif
         iObs.ServerDied();
         // FIXME - handle a dead thread here?
      }
      else{
       #ifdef USE_TRACE
         char* errStr = new char[32];
         sprintf( errStr, "Error number: %i", err );
         TRACE_FUNC1( errStr );
         delete errStr;
       #endif
         User::Panic( KNewAudio, err );
      }
   }
}

void CNewAudioNoServer::RunL()
{
   // The server notified us that a request was complete.
   // Deal with it.
   switch ( iOutstandingRequest ) {
      case NewAudioServerTypes::PrepareSoundRequest:
      {
         iObs.PrepareSoundCompleted(iSoundLength, iStatus.Int() );
         break;
      }
      case NewAudioServerTypes::PlaySoundRequest:
      {
         iObs.PlaySoundCompleted( iStatus.Int() );
         break;
      }
      default:
         // This is tricky and should never happen. Ignore it
         // for now.
         break;
   }
}


TInt CNewAudioNoServer::RunError(TInt aError)
{
   if( aError != KErrNone ){
    #ifdef USE_TRACE
      char* errStr = new char[32];
      sprintf(errStr, "Error#: %i", aError);
      TRACE_FUNC1(errStr);
      delete errStr;
    #endif
   }
   return aError;
}


void CNewAudioNoServer::PrepareSoundL( CDesCArray & clips )
{
   if (IsActive()) {      
    #ifdef USE_TRACE
      TRACE_FUNC1( "Cancel()" );
    #endif
      Cancel();
   }
   // Load some sounds in anticipation of playing them
   delete iMsgBuffer;
   iMsgBuffer = NULL;

   // Calculate total length of the clips.
   int totalLength = 0;
   {
      for ( int i = 0; i < clips.MdcaCount(); ++i ) {
         totalLength += clips[i].Length() + 2;
      }
   }
   
   iMsgBuffer    = HBufC::NewL(totalLength);
   iMsgBufferPtr.Set(iMsgBuffer->Des());

   // The NewAudioServer expects the clip filenames separated
   // by a semicolon.
   {
      for ( int i = 0; i < clips.MdcaCount(); ++i ) {
         iMsgBufferPtr.Append( clips[i] );
         iMsgBufferPtr.Append( KSemicolon );
      }
   }
   
   iMsgBufferLength = iMsgBufferPtr.Length();
   
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   TIpcArgs foo((TAny*)iMsgBufferLength,
                       &iMsgBufferPtr,
                       &iSoundLengthPtr,
                       NULL);
   SendReceive(NewAudioServerTypes::PrepareSoundRequest, foo, iStatus);
#else
   TAny* foo[4] = { (TAny*)iMsgBufferLength,
                    &iMsgBufferPtr,
                    &iSoundLengthPtr,
                    NULL};
   SendReceive(NewAudioServerTypes::PrepareSoundRequest, foo, iStatus);
#endif
   iOutstandingRequest = NewAudioServerTypes::PrepareSoundRequest;
   SetActive();
}

void CNewAudioNoServer::PlaySound()
{
   if (IsActive()) {
    #ifdef USE_TRACE
      TRACE_FUNC1( "Cancel()");
    #endif
      Cancel();
   }
   // Tell the server to play the last prepared sounds
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   TIpcArgs foo(0,0,0,0);       // Dummy data - unused
   SendReceive(NewAudioServerTypes::PlaySoundRequest, foo, iStatus);
#else
   TInt foo[4] = {0,0,0,0};       // Dummy data - unused
   SendReceive(NewAudioServerTypes::PlaySoundRequest, foo, iStatus);
#endif
   iOutstandingRequest = NewAudioServerTypes::PlaySoundRequest;
   SetActive();
}

void CNewAudioNoServer::StopSound()
{
 #ifdef USE_TRACE
   TRACE_FUNC1( "Cancel()");
 #endif
   Cancel();
}

void CNewAudioNoServer::SetVolume( int volumePercent )
{
   // Send message to the server, but do not wait for the answer.
#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   TIpcArgs foo( volumePercent, 0, 0, 0 );       // Dummy data - unused
   TInt err = Send( NewAudioServerTypes::VolumeRequest, foo );
#else
   TInt foo[4] = { volumePercent, 0, 0, 0 };
   TInt err = Send( NewAudioServerTypes::VolumeRequest, foo );
#endif
   switch( err )
   {
   case KErrNone:
   case KErrServerBusy:
      break;
   default:
   {
      User::Panic( KNewAudio, err );
   }
   }
}



/**************************************
 * CNewAudio
 * Combines CNewAudioNoServer with a 
 * CNewAudioOwner to automatically manage
 * the server thread.
 **************************************/

CNewAudio* CNewAudio::NewLC(MNewAudioObserver & aObs)
{
   CNewAudio *ptr = new (ELeave) CNewAudio(aObs);
   CleanupStack::PushL(ptr);
   ptr->ConstructL();
   return ptr;
}

CNewAudio* CNewAudio::NewL(MNewAudioObserver & aObs)
{
   CNewAudio *ptr = CNewAudio::NewLC(aObs);
   CleanupStack::Pop(ptr);
   return ptr;
}

CNewAudio::CNewAudio(MNewAudioObserver & aObs)
   : CNewAudioNoServer(aObs),
     iServerOwner(NULL)
{
}

void CNewAudio::ConstructL()
{
   // Create the server and attach to it.
   iServerOwner = CNewAudioOwner::NewL();
   CNewAudioNoServer::ConstructL();
}


CNewAudio::~CNewAudio()
{
   Disconnect();
   delete iServerOwner;
}
