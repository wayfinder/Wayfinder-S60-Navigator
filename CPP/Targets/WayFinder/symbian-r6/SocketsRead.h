/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __SOCKETSREAD_H__
#define __SOCKETSREAD_H__

#include <in_sock.h>

#include "WayFinderConstants.h"

const TInt KSingle      = 1;
const TInt KSmallChunck = 8;
const TInt KChunck      = 32;
const TInt KBigChunck   = 128;

class CMessageHandler;

/**
 * CSocketsRead
 * This class handles reading data from the socket.
 * In this implementation, any data read is simply displayed as text on the AppUI.
 */
class CSocketsRead : public CActive
{

public:

   /**
    * NewL
    * Create a CSocketsRead object
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CSocketsRead
    */
   static CSocketsRead* NewL(CMessageHandler& aAppUI, RSocket& aSocket);

   /**
    * NewLC
    * Create a CSocketsRead object
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    * @result a pointer to the created instance of CSocketsRead
    */
   static CSocketsRead* NewLC(CMessageHandler& aAppUI, RSocket& aSocket);

   /**
    * ~CSocketsRead
    * Destroy the object and release all memory objects
    */
   ~CSocketsRead();

   /**
    * Start
    * Initiate a read from socket
    */
   void Start();

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
    * CSocketsRead
    * Perform the first phase of two phase construction 
    * @param aAppUI AppUI to use for ui output
    * @param aSocket socket to read from
    */
   CSocketsRead(CMessageHandler& aAppUI, RSocket& aSocket);

   /**
    * ConstructL
    * Perform the second phase construction of a CSocketsRead 
    */
   void ConstructL();

   /**
    * ReadHeader
    * Read the header of a message.
    */
   void ReadHeader();

   /**
    * InitData
    * Initiate the reception of a message.
    */
   void InitData();

   /**
    * ReadData
    * Initiate a read from socket
    */
   void ReadData();

   /**
    * Send the recived message to the CMessageHandler.
    */
   void SendMessage();

private: // Member variables

   enum TSocketsReadState 
   {
      EGettingHeader,
      EGotHeader,
      EGettingData
   };

   /// The socket current read state
   TSocketsReadState iReadState;

   /// Socket to read data from
   RSocket& iSocket;

   /// AppUI for displaying text etc
   CMessageHandler& iAppUI;

   /// Buffer for storing the message
   CBufFlat* iMessageBuf;

   /// Length of the message
   TInt iMessageLength;

   /// The number of bytes read
   TInt iBytesRead;

   /// Buffer for receiving the header
   TBuf8<KHeaderSize> iHeaderBuf;

   /// Buffer for reciving the data
   TBuf8<KSingle> iSingleBuf;

   /// Buffer for reciving the data
   TBuf8<KSmallChunck> iSmallBuf;

   /// Buffer for reciving the data
   TBuf8<KChunck> iChunckBuf;

   /// Buffer for reciving the data
   TBuf8<KBigChunck> iBigBuf;

   /// The buffer currently used.
   TInt iCurrentBuffer;

};

#endif // __SOCKETSREAD_H__
