/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#ifndef BT_SOCKETSREAD_H
#define BT_SOCKETSREAD_H

#include <es_sock.h>
#include "ActiveLog.h"



/**
 * This class handles reading data from the socket.  
 */
class CBtSocketsRead : public CActiveLog
{
   /** @name Constructors and destructor. */
   //@{
   /**
    * CBtSocketsRead
    * Perform the first phase of two phase construction 
    * @param aPeer the receiver of data and status messages.
    * @param aSocket socket to read from
    */
   CBtSocketsRead(class MBtSocketsPeer& aPeer, class RSocket& aSocket);

   /** Perform the second phase construction of a CBtSocketsRead. */
   void ConstructL();
public:
   /**
    * Create a CBtSocketsRead object
    * @param aPeer   receiver of data and status messages.
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CBtSocketsRead
    */
   static class CBtSocketsRead* NewL(class MBtSocketsPeer& aPeer,
                                     class RSocket& aSocket);

   /**
    * Create a CBtSocketsRead object
    * @param aPeer   receiver of data and status messages.
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CBtSocketsRead
    */
   static class CBtSocketsRead* NewLC(class MBtSocketsPeer& aPeer, 
                                      class RSocket& aSocket);

   /** Destroy the object and release all memory objects */
   virtual ~CBtSocketsRead();
   //@}

   /**
    * Start
    * Initiate a read from socket
    */
   void Start();

   /**
    * CancelWrite
    * Cancel read next time RunL is called
    */
   void CancelRead();

protected: 
   /**@name  from CActive */
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
private:

   /**
    * IssueRead
    * Initiate a read from socket
    */
   void IssueRead();
   void IssueTimer();

private:
   // Member variables
   enum { KReadBufferSize = 8192 };

   enum TBtReadetState {
      EBtSockRead,
      EBtSockWait,
   };

   enum TBtReadetState iReaderState;
   /// Socket to read data from
   class RSocket& iSocket;
   class RTimer iTimer;

   class MBtSocketsPeer& iSocketPeer;
   /// The length of data read is written here
   TSockXfrLength iReadLength;

   // Buffer where the read data will be put.
   TBuf8<KReadBufferSize> iBuffer;

   // Tells us to stop reading
   TBool iCancel;
};

#endif // __SOCKETSREAD_H__
