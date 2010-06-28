/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SYMBIAN_TCP_CONNECTION_H__
#define __SYMBIAN_TCP_CONNECTION_H__

/** 
 * @class SymbianTcpConnection
 * This class specifies the connection to the UI.
 */
class SymbianTcpConnection
{

public:

   enum TSocketsEngineState
   {  
      ENotConnected,
      EStartingConnection,
      EConnecting,
      EConnected,
      ETimedOut,
      ELookingUp,
      ELookUpFailed,
      EConnectFailed,
      EDisconnecting
   };


   /**
    * SendMessageL
    * Handles the sending of a message
    * @param aMessage the message to send
    */
   virtual void SendMessageL( TDesC8 &data, int length ) = 0;

   /**
    * ReceiveMessage
    * Handles the reception of a message.
    @param the text to be displayed
    */
   virtual void ReceiveMessageL( const TDesC8& aMessage, const TInt aMessageLength ) = 0;

   /**
    * ErrorNotify
    * Displays an error message
    * @param aErrMessage error message text
    * @param aErrCode error code number
    */
/*    virtual void ErrorNotify(const TDesC& aErrMessage, TInt aErrCode) = 0; */

   /**
    * ErrorNotify
    * Displays an error message
    */
   virtual void ErrorNotify( TInt aErrMessage, TInt aErrCode ) = 0;


   /**
    * SetStatus
    * Displays the status text
    * @param aStatus new status description text
    */
   virtual void SetStatus(TSocketsEngineState aNewStatus) = 0;

   /**
    * ConnectionCancelled
    * When download has been cancelled and all connections
    * and outstanding requests has been cancelled.
    */
   virtual void ConnectionCancelled() = 0;

};

#endif
