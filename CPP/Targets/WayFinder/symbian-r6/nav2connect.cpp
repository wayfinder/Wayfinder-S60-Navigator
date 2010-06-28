/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <e32svr.h>
#include <eikdef.h>

#include "nav2connect.h"
#include "Buffer.h"
#include "UIConnection.h"
#include "TimeOutTimer.h"
#include "Nav2Error.h"
#include "Nav2ErrorXX.h"
#include "AudioCtrlEn.h"
#include "AudioCtrlSv.h"
#include "AudioCtrlDe.h"
#include "AudioCtrlDa.h"




CNav2Connect::CNav2Connect()
{
   iNav2 = NULL;
   iGuiBuffer = NULL;
   iGuiChannel = NULL;
   iBtChannel = NULL;
}

CNav2Connect::~CNav2Connect()
{
   Halt();
   if( iGuiBuffer != NULL )
      delete iGuiBuffer;
   if( iGuiChannel != NULL )
      delete iGuiChannel;
   if( iBtChannel != NULL )
      delete iBtChannel;
   if( iNav2 != NULL )
      delete iNav2;
}

void CNav2Connect::ConstructL( MUIConnection* aAppUi,
                               const TUint8* aResourcePath,
                               const TUint8* aWayfinderPath,
                               const TUint8* aSimulatorHost,
                               TUint16 aSimulatorPort )
{
   iAppUi = aAppUi;
   //Creates and starts Nav2.
   
   Nav2Error::Nav2ErrorTable *errorTable = new Nav2Error::Nav2ErrorTableEn();
   AudioCtrlLanguage *audioSyntax = new AudioCtrlLanguageEn();
   
   iNav2 = new  Nav2Develop( (const char*)aResourcePath, (const char*)aWayfinderPath,
                     errorTable, audioSyntax,
                     (const char*)aSimulatorHost, aSimulatorPort,
                     NULL,
                     NULL, 0);
   
   //Connect to the message channel
   iGuiChannel = iNav2->getGuiChannel();
   iGuiChannel->connect();
   
   iGuiBuffer = new Buffer( 1024 );
   
   //Connect to the bluetooth channel
   iBtChannel = iNav2->getBluetoothChannel();
   if( iBtChannel != NULL ){
      iBtChannel->connect();
   }

   //Create and start the poll timer
   iPollTimer = CTimeOutTimer::NewL(CActive::EPriorityHigh, *this);
   iPollTimer->After( iGuiChannel->getPollInterval() );
}
	
void CNav2Connect::Halt()
{
   //Shut down the connection and stop the execution of nav2
   iPollTimer->Cancel();
   iGuiChannel->disconnect();
   if( iBtChannel != NULL ) 
      iBtChannel->disconnect();
   iNav2->Halt();
}
   
void CNav2Connect:: WriteBluetoothData(const TDesC8 &aBytes, TInt aLength )
{
   const TUint8* data = aBytes.Ptr();
   TUint16 length = aBytes.Length();
   if(iBtChannel!=NULL)
	   iBtChannel->writeData( data, length );
  
	    
}   

void CNav2Connect::WriteBuffer( Buffer* aBuffer )
{
   TInt length = aBuffer->getLength();
   TUint8* data = new TUint8[length];
   aBuffer->setReadPos( 0 );
   for(TInt i=0; i < length; i++ )
      data[i] = aBuffer->readNext8bit();
   
   iGuiChannel->writeData( data, length);
   delete[] data;
}

void CNav2Connect::TimerExpired()
{
   //If the Gui channel has recieved data  
   if( !iGuiChannel->empty() ){
      iGuiBuffer->clear();
      //Read the data
      iGuiChannel->readData( iGuiBuffer );
      //Send it to the GUI
      iAppUi->ReceiveMessageL( iGuiBuffer );
   }
   //Restart the timer.
   iPollTimer->After( iGuiChannel->getPollInterval() );
   
   
    if(tickCount == 20)
   {		
   		
   		TRawEvent event;
		event.Set(TRawEvent::EActive);
		UserSvr::AddEvent(event);
		tickCount=0;
		
   		
   }
   tickCount = tickCount + 1;
   
}
