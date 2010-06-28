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

#ifndef SOCKETSREAD_H
#define SOCKETSREAD_H

#include <in_sock.h>

class SymbianTcpConnection;

/**
 * CNewSocketsRead
 * This class handles reading data from the socket.
 * In this implementation, any data read is simply displayed as text on the AppUI.
 */
class CNewSocketsRead : public CActive
{

public:

   /**
    * NewL
    * Create a CNewSocketsRead object
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CNewSocketsRead
    */
   static class CNewSocketsRead* NewL(class SymbianTcpConnection& aAppUI,
                                      class RSocket& aSocket);

   /**
    * NewLC
    * Create a CNewSocketsRead object
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CNewSocketsRead
    */
   static class CNewSocketsRead* NewLC(class SymbianTcpConnection& aAppUI, 
                                       class RSocket& aSocket);

   /**
    * ~CNewSocketsRead
    * Destroy the object and release all memory objects
    */
   virtual ~CNewSocketsRead();

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

protected: // from CActive

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

private:

   /**
    * CNewSocketsRead
    * Perform the first phase of two phase construction 
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    */
   CNewSocketsRead(class SymbianTcpConnection& aAppUI, class RSocket& aSocket);

   /**
    * ConstructL
    * Perform the second phase construction of a CNewSocketsRead 
    */
   void ConstructL();

   /**
    * IssueRead
    * Initiate a read from socket
    */
   void IssueRead();

private: // Member variables
   enum { KReadBufferSize = 8192 };

   /// Socket to read data from
   class RSocket& iSocket;

   /// AppUI for displaying text etc
   class SymbianTcpConnection& iAppUI;

   /// The length of data read is written here
   TSockXfrLength iReadLength;

   // Buffer where the read data will be put.
   TBuf8<KReadBufferSize> iBuffer;

   // Tells us to stop reading
   TBool iCancel;
};

#endif // __SOCKETSREAD_H__
