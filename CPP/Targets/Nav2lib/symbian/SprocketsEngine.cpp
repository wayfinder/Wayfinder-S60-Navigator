/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "SprocketsEngine.h"
#include "Buffer.h"
#include "SprocketTalker.h"

#define LOGPOINTER if(iLog)iLog
#include "LogMacros.h"
#undef SPROCKET_DBG


CSprocketsEngine* CSprocketsEngine::NewL( MSprocketTalker* aConsole,
                                          isab::Nav2::Channel* nav2,
                                          const char* name)
{
   CSprocketsEngine* self = CSprocketsEngine::NewLC(aConsole, nav2, name);
   CleanupStack::Pop(self);
   return self;
}

CSprocketsEngine* CSprocketsEngine::NewLC( MSprocketTalker* aConsole,
                                           isab::Nav2::Channel* nav2,
                                           const char* name)
{
   CSprocketsEngine* self = new (ELeave) CSprocketsEngine(aConsole, nav2);
   CleanupStack::PushL(self);
   self->ConstructL(name);
   return self;
}

CSprocketsEngine::CSprocketsEngine(MSprocketTalker* aConsole,
                                   isab::Nav2::Channel* nav2) :  
   CActive(EPriorityStandard),
   iEngineStatus(ENotConnected),
   iConsole(aConsole),
   iToNav2(nav2), iLog(NULL)
{
}

CSprocketsEngine::~CSprocketsEngine()
{
   DBG("~CSprocketsEngine");
   Disconnect();
   DBG("Disconnected");
   delete iBuffer;
   DBG("~CSProcketsEngine complete - deleting log.");
   delete iLog;
}


void CSprocketsEngine::ConstructL(const char* name, class isab::LogMaster* aLogMaster)
{
   name = name; // To remove varnings.
   SetLogMasterL(aLogMaster, name);
   iBuffer = new (ELeave) isab::Buffer();
   CActiveScheduler::Add(this);
}

void CSprocketsEngine::SetLogMasterL(class isab::LogMaster* aLogMaster, 
                                     const char* name)
{
#if !defined NO_LOG_OUTPUT && defined SPROCKET_DBG
   delete iLog;
   iLog = NULL;
   iLog = new (ELeave) isab::Log(name ? name : "Sprocket", 
                                 isab::Log::LOG_ALL, aLogMaster);
   DBG("New Log object set");
#else
   aLogMaster = aLogMaster;
   name = name;
#endif
}

void CSprocketsEngine::ConnectL()
{
   DBG("ConnectL()");
   // Initiate connection process
   DBG("iToNav2: %p iEngineStatus %c= Enotconnected",
       iToNav2, iEngineStatus == ENotConnected ? '=' : '!');
   if(iToNav2 && iEngineStatus == ENotConnected){
      iToNav2->connect();
      DBG("Called Nav2::Channel->connect()");
      ChangeStatus(EConnected);
      iToNav2->armReader(&iStatus);
      iConsole->ConnectedL(); //XXX
      SetActive();
   }
}

void CSprocketsEngine::Disconnect()
{
   DBG("Disconnect() this:%p", (void*)this);
   if (IsActive()) {
      Cancel();
   }
   if(iToNav2) {
      iToNav2->disconnect();
   }
   ChangeStatus(ENotConnected);
   DBG("Disconnect done");
}

void CSprocketsEngine::Release()
{
   DBG("Release()");
   Disconnect();
   delete iToNav2;
   iToNav2 = NULL;
   DBG("Release() done");
}
// from CActive
void CSprocketsEngine::DoCancel()
{
   DBG("DoCancel");
   if(iToNav2){
      iToNav2->cancelArm();
   }
   DBG("DoCancel complete");
}

void CSprocketsEngine::WriteL(class isab::Buffer* aBuffer )
{
   DBG("WriteL(Buffer* aBuffer == %p)", (void*)aBuffer);
   //DBGDUMP("WriteL", aBuffer->accessRawData(0), aBuffer->getLength());
   if(iToNav2 && (! iToNav2->writeData(aBuffer->accessRawData(0), 
                                       aBuffer->getLength()))){
      iEngineStatus = ENotConnected;
   }
}

void CSprocketsEngine::Write( const TDesC8& aBytes )
{
   DBG("WriteL(const TDesC8& aBytes, TInt aLength == %d", aBytes.Length());
   // Write data to socket
   if(iToNav2 && iEngineStatus == EConnected){
      if(! iToNav2->writeData(aBytes.Ptr(), aBytes.Length())){
         iEngineStatus = ENotConnected;
      }
   } else {
      WARN("Sprocket not connected!");
      DBG("iToNav2: %p iEngineStatus: %d", iToNav2, iEngineStatus);
      DBG("(iToNav2 && iEngineStatus == EConnected) is %s",
          (iToNav2 && iEngineStatus == EConnected) ? "TRUE" : "FALSE");
   }
}

TBool CSprocketsEngine::OneMoreMessage(TUint& aLength)
{
   if(iBuffer->remaining() > 4){
      aLength = iBuffer->readNextUnaligned32bit() & 0x00ffffff;
      iBuffer->jumpReadPos(-4);
      if(iBuffer->remaining() >= (aLength - 4)){
         return ETrue;
      }
   }
   return EFalse;
}

void CSprocketsEngine::ArmOrCompleteL()
{
   TUint dummy = 0; //we don't need the length value.
   if (OneMoreMessage(dummy)) {
      //complete self immediatly;
      iStatus = KRequestPending;
      SetActive();
      TRequestStatus* myStatus = &iStatus;
      User::RequestComplete(myStatus, KErrNone);
   } else {
      //shift buffer
      ///add shiftdown function to buffer. for some fun compiling times!
      if(iBuffer->remaining() > 0){
         int len = iBuffer->remaining();
         uint8* tmp = new (ELeave) uint8[len];
         iBuffer->readNextByteArray(tmp, len);
         iBuffer->clear();
         iBuffer->writeNextByteArray(tmp, len);
         iBuffer->setReadPos(0);
         delete[] tmp;
      } else {
         iBuffer->clear();
      }
      //arm reader to wait for new data from Nav2.
      iToNav2->armReader(&iStatus);
      SetActive();
   }
}


void CSprocketsEngine::RunL()
{
   DBG("RunL");
   TUint length = 0;
   if(OneMoreMessage(length)){
      TPtrC8 temp = TPtrC8(iBuffer->accessRawData(), length);
      iBuffer->jumpReadPos(length);

      //need to arm here since ReceiveMessageL may block and cause
      //multiple active schedulers. iBuffer needs to be consistent and
      //the reader rearmed.
      ArmOrCompleteL();

      //handle message. WARNING! may block and become reentrant. Weird huh!
      iConsole->ReceiveMessageL( temp );
      DBG("iBuffer->remaining() : %"PRIu32, iBuffer->remaining());
      return; //all done
   } else {
      // Can only get here if iBuffer did not contain one message last run -
      // reader must have been armed
      if(iToNav2) {
         if (!iToNav2->readData(iBuffer)){
            //XXXhandle gracefully
            iEngineStatus = ENotConnected;
            iConsole->Panic();
            return;
         }
         //wait for new data from nav2.
         ArmOrCompleteL();
      }
   }
   DBG("RunL ends.");
}

void CSprocketsEngine::ChangeStatus(TSprocketsEngineState aNewStatus)
{
#define LOGCHANGE(state) DBG("ChangeStatus to " #state)
   // Update the status (and the status display)
   switch (aNewStatus){
   case ENotConnected:
      LOGCHANGE(ENotConnected);
      break;
   case EConnected:
      LOGCHANGE(EConnected);
      break;
   }
   iEngineStatus = aNewStatus;
}

TBool CSprocketsEngine::IsConnected() const
{
   DBG("%sconnected", (iToNav2 && iEngineStatus == EConnected) ? "" : "Not ");
   return (iToNav2 && iEngineStatus == EConnected);
}


