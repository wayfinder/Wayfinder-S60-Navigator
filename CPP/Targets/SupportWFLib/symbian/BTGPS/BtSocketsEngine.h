/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BT_SOCKETS_ENGINE_H
#define BT_SOCKETS_ENGINE_H

#include <es_sock.h>
#include <bt_sock.h>
#include "Completer.h"
#include "ActiveLog.h"

class CBtSocketsEngine : public CActiveLog, public MCompleter
{
   /** @name Constructors & Destructor*/
   //@{
   /**
    * CBtSocketsEngine
    * Perform the first phase of two phase construction
    * @param aConsole the console to use for ui output
    */
   CBtSocketsEngine(class MBtSocketsPeer& aPeer);

   /**
    * ConstructL
    * Perform the second phase construction of a CBtSocketsEngine 
    */
   void ConstructL(const class TBTDevAddr& aAddress, TInt aPort);

public:
   /**
    * NewL
    * Create a CBtSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CBtSocketsEngine
    */
   static class CBtSocketsEngine* NewL(class MBtSocketsPeer& aPeer,
                                       const class TBTDevAddr& aAddress,
                                       TInt aPort);

   /**
    * NewLC
    * Create a CBtSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CBtSocketsEngine
    */
   static class CBtSocketsEngine* NewLC(class MBtSocketsPeer& aPeer,
                                        const class TBTDevAddr& aAddress,
                                        TInt aPort);

   /**
    * ~CBtSocketsEngine
    * Destroy the object and release all memory objects
    */
   virtual ~CBtSocketsEngine();
   //@}
public: // new methods

   void CancelAll();

   /**
    * ConnectL
    * Initiate connection of socket, using iServerName and iPort
    */
   void ConnectL(class TRequestStatus* aStatus);
   void CancelConnect();

private:
   /** 
    * Common code for the other two disconnect functions.
    * This function calls RSocket::Shutdown if the IsConnected
    * function returns true. The argument aStatus is used as an
    * ergument to the RSocket::Shutdown function. SetActive is _not_
    * called. That is left for the callee. This means that this
    * function can be used asynchronously or synchronously.
    * @param aStatus reference to the TRequestStatus variable that
    *                should be used or the RSocket::Shutdown function.
    * @return ETrue if RSocket::Shutdown was called.
    */
   TBool DisconnectInternal(class TRequestStatus& aStatus);
public:
   /** Disconnect socket syncronously. */
   void Disconnect();
   /**
    * Asynchrnous disconnect. The disconnect is actually completed as soon as the Disconnect function returns, but
    */
   void Disconnect(class TRequestStatus* aStatus);
   void CancelDisconnect();
   /**
    * WriteL
    * Write data to socket
    * @param aData data to be written
    */
   void WriteL(const TDesC8 &data);

   /**
    * Read
    * Initiate read of data from socket
    */
   void Read();

   /**
    * IsConnected
    * Is socket fully connected?
    * @return true if socket is connected
    */
   TBool IsConnected() const;

protected:
   /** @name from CActive */
   //@{
   /**
    * DoCancel
    * cancel any outstanding operation
    */
   void DoCancel();

   /**
    * RunL
    * called when operation complete
    */
   void RunL();
   //@}

   /** @name From CActiveLog. */
   //@{
   virtual CArrayPtr<CActiveLog>* SubLogArrayLC();
   //@}
   
   enum TBtSocketEngineState {
      ENotConnected,
      EConnecting,
      EConnected,
      EDisconnecting,
   };

private:
   /** @name Member variables. */
   //@{
   enum TBtSocketEngineState iState;
   /// Socket reader active object
   class CBtSocketsRead*      iSocketsRead;

   /// Socket writer active object
   class CBtSocketsWrite*     iSocketsWrite;

   /// The actual socket
   class RSocket               iSocket;

   /// The socket server
   class RSocketServ          iSocketServ;
   
   ///The socket address for the remote device. 
   class TBTSockAddr iAddress;

   class MBtSocketsPeer& iPeer;
   //@}
};

#endif // BT_SOCKETS_ENGINE_H
