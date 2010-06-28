/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "SocketsRead.h"
#include "MessageHandler.h"
#include "RsgInclude.h"
#define ILOG_POINTER aAppUI.iAppUi->iLog
#include "memlog.h"

CSocketsRead* CSocketsRead::NewL(CMessageHandler& aAppUI, RSocket& aSocket)
{
   CSocketsRead* self = CSocketsRead::NewLC(aAppUI, aSocket);
   CleanupStack::Pop();
   return self;
}

	
CSocketsRead* CSocketsRead::NewLC(CMessageHandler& aAppUI, RSocket& aSocket)
{
   CSocketsRead* self = new (ELeave) CSocketsRead(aAppUI, aSocket);
   LOGNEW(self, CSocketsRead);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}
#undef ILOG_POINTER
#define ILOG_POINTER iAppUI.iAppUi->iLog
#include "memlog.h"


CSocketsRead::CSocketsRead(CMessageHandler& aAppUI, RSocket& aSocket)
: CActive(CActive::EPriorityStandard),
  iSocket(aSocket),
  iAppUI(aAppUI)
{
}


CSocketsRead::~CSocketsRead()
{
   Cancel();
}


void CSocketsRead::ConstructL()
{
   iMessageBuf = NULL;
   iMessageLength = 0;
   iBytesRead = 0;
   CActiveScheduler::Add(this);
}

void CSocketsRead::DoCancel()
{
   // Cancel asychronous read request
   iSocket.CancelRead();
   iReadState = EGettingHeader;
   if( iMessageBuf != NULL ){
      iMessageBuf->Delete( 0, iBytesRead );
      LOGDEL(iMessageBuf);
      delete iMessageBuf;
      iMessageBuf = NULL;
   }
}

void CSocketsRead::RunL()
{
   // Active object request complete handler
   if (iStatus == KErrNone){
      // Character has been read from socket
      switch( iReadState )
      {
      case EGettingHeader:
         ReadHeader();
         break;
      case EGotHeader:
         iBytesRead = KHeaderSize;
         InitData();
         break;
      case EGettingData:
         if( iCurrentBuffer == KBigChunck )
            iMessageBuf->InsertL( iBytesRead, iBigBuf.Ptr(), iCurrentBuffer );
         else if( iCurrentBuffer == KChunck )
            iMessageBuf->InsertL( iBytesRead, iChunckBuf.Ptr(), iCurrentBuffer );
         else if( iCurrentBuffer == KSmallChunck )
            iMessageBuf->InsertL( iBytesRead, iSmallBuf.Ptr(), iCurrentBuffer );
         else
            iMessageBuf->InsertL( iBytesRead, iSingleBuf.Ptr(), iCurrentBuffer );
         iBytesRead += iCurrentBuffer;
         ReadData();
         break;
      }
   }
   else{
      // Error: pass it up to user interface
      iAppUI.ErrorNotify( R_WAYFINDER_INTERNALREADERROR_MSG, iStatus.Int() );
   }	
}


void CSocketsRead::ReadHeader()
{
   // Initiate a new read from socket into iBuffer
   ASSERT(!IsActive());
   iSocket.Read( iHeaderBuf, iStatus );
   if( iMessageBuf != NULL ){
      //iMessageBuf->Delete( 0, iBytesRead );
      LOGDEL(iMessageBuf);
      delete iMessageBuf;
      iMessageBuf = NULL;
   }
   iBytesRead = 0;
   iReadState = EGotHeader;
   SetActive();
}

void CSocketsRead::InitData()
{
   // Initiate a new read from socket into iBuffer
   ASSERT(!IsActive());
   TInt8 version = iHeaderBuf[0];
   TUint8 b1 = iHeaderBuf[1];
   TUint8 b2 = iHeaderBuf[2];
   TUint8 b3 = iHeaderBuf[3];
   if( version != 0 && version != 3 ){ // Wrong version
      iReadState = EGettingHeader;
      iBytesRead = 0;
      ASSERT( false );
   }
   else{
      TInt32 length = (b1 << 16) + (b2 << 8) + (b3);
      if ( (length > 65535) || (length < 4) ) {
         iReadState = EGettingHeader;
         iMessageLength = 0;
         iBytesRead = 0;
      }
      else{
         iMessageLength = length;
         iMessageBuf = CBufFlat::NewL( KHeaderSize );
         iMessageBuf->SetReserveL( length );
         iMessageBuf->InsertL( 0, iHeaderBuf.Ptr(), KHeaderSize );
         iReadState = EGettingData;
         ReadData();
      }
   }
}

void CSocketsRead::ReadData()
{
   // Initiate a new read from socket into iBuffer
   TInt remaining = iMessageLength - iBytesRead;
   if( remaining == 0 ){
      SendMessage();
   }
   else if( remaining >= KBigChunck ){
      iCurrentBuffer = KBigChunck;
      iSocket.Read( iBigBuf, iStatus );
      SetActive();
   }
   else if( remaining >= KChunck ){
      iCurrentBuffer = KChunck;
      iSocket.Read( iChunckBuf, iStatus );
      SetActive();
   }
   else if( remaining >= KSmallChunck ){
      iCurrentBuffer = KSmallChunck;
      iSocket.Read( iSmallBuf, iStatus );
      SetActive();
   }
   else{
      iCurrentBuffer = KSingle;
      iSocket.Read( iSingleBuf, iStatus );
      SetActive();
   }
}

void CSocketsRead::SendMessage()
{
   iAppUI.ReceiveMessageL( iMessageBuf->Ptr(0), iMessageLength );
   ReadHeader(); // Immediately start another read
}

void CSocketsRead::Start()
{
   // Initiate a new read from socket into iBuffer
   if (!IsActive()){
      ReadHeader();
   }
}

