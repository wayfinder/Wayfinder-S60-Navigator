/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
* Description:
*     Class for connecting to Nav2
*/
#ifndef NAV2CONNECT_H
#define NAV2CONNECT_H

#include <e32base.h>
#include "nav2.h"
#include "TimeOutNotify.h"

namespace isab{
   class Buffer;
}

class MUIConnection;
class CTimeOutTimer;

using namespace isab;

class CNav2Connect : public CBase, public MTimeOutNotify
{
public:

   //Construtor
   CNav2Connect();

   // Destrutor
   ~CNav2Connect();
   
   /**
    * Creates and starts Nav2
    * @param aAppUi an instance of the interface MUIConnection containing
    *               the call back function ReceiveMessageL().
    * @param aResourcePath the path to the Wayfinder resources.
    * @param aWayfinderPath the path to the Wayfinder application.
    * @param aSimulatorHost the internet name of the host machine running
    *                       MapViewer and TcpListenConnect.
    * @param aSimulatorPort one of the ports of TcpListenConnect.
    *                       (MapViewer connects to the other one.
    */
   void ConstructL( MUIConnection* aAppUi,
                    const TUint8* aResourcePath,
                    const TUint8* aWayfinderPath,
                    const TUint8* aSimulatorHost,
                    TUint16 aSimulatorPort );

public: //New functions

   /**
    * Start Nav2 shutdown.
    */
   void Halt();
   
   /**
    * Write data received from the BT Gps to nav2.
    * @param aBytes a descriptor with the data.
    */
   void WriteBluetoothData( const TDesC8 &aBytes,TInt aLength  );

   /**
    * Write message buffers to Nav2.
    * @param aBuffer a buffer containing a message.
    */
   void WriteBuffer( Buffer* aBuffer );

public: // Functions from base classes

   // from MTimeOutNotify

   /**
    * The function to be called when a timeout occurs
    * Used for polling Nav2 for new messages.
    */
   void TimerExpired(); 

private:
   
   /// iAppUi connection to the UI.
   MUIConnection* iAppUi;

   /// An instance of Nav2
   Nav2* iNav2;
   
   /// Channel for communicating GUI messages over
   Nav2::Channel* iGuiChannel;
   
   /// Channel for sending BT data to Nav2
   Nav2::Channel* iBtChannel;
   
   /// Buffer used to send messages to Nav2
   Buffer* iGuiBuffer;

   /// Timer active object, times out when it's time
   /// to poll nav2 for messages.
   CTimeOutTimer* iPollTimer;
   
   TInt tickCount;

};

#endif
