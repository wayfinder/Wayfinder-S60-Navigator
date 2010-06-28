/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __SOCKETSWRITE_H__
#define __SOCKETSWRITE_H__

#include <in_sock.h>
#include "WayFinderConstants.h"
#include "TimeOutNotify.h"

class CTimeOutTimer;
class CMessageHandler;

/*! 
  @class CSocketsWrite
  
  @discussion This class handles writing data to the socket.
  Data to be written is accumulated in iTransferBuffer, and is then transferred
  to iWriteBuffer for the actual write to the socket.
  */
class CSocketsWrite : public CActive, public MTimeOutNotify
{

public:

   /*!
    @function NewL
   
    @discussion Create a CSocketsWrite object
    @param aConsole console to use for ui output
    @param aSocket socket to write to
    @result a pointer to the created instance of CSocketsWrite
    */
   static CSocketsWrite* NewL(CMessageHandler& aConsole, RSocket& aSocket);

   /*!
    @function NewLC
   
    @discussion Create a CSocketsWrite object
    @param aConsole console to use for ui output
    @param aSocket socket to write to
    @result a pointer to the created instance of CSocketsWrite
    */
   static CSocketsWrite* NewLC(CMessageHandler& aConsole, RSocket& aSocket);

   /*!
    @function ~CSocketsWrite
  
    @discussion Destroy the object and release all memory objects
    */
   ~CSocketsWrite();

   /*!
    @function IssueWrite

    @discussion Write the data to the socket (buffered)
    @param aData the data to be written
    */
   void IssueWriteL(const TDesC8& aData);

protected: // from CActive

   /*!
    @function DoCancel
   
    @discussion cancel any outstanding operation
    */
   void DoCancel();

   /*!
    @function RunL
   
    @discussion called when operation complete
    */
   void RunL();	

   // From MTimeOutNotify
   void TimerExpired(); 

private:

   /*!
    @function CSocketsWrite
  
    @discussion Perform the first phase of two phase construction 
    @param aConsole console to use for ui output
    @param aSocket socket to read from
    */
   CSocketsWrite(CMessageHandler& aConsole, RSocket& aSocket);

   /*!
    @function ConstructL
  
    @discussion Perform the second phase construction of a CSocketsWrite 
    */
   void ConstructL();

   /*!
    @function SendNextPacket

    @discussion Handle a 'write buffer empty' situation.
    */    
   void SendNextPacket();

private: // Member data

   enum TWriteState 
   {
      ESending,
      EWaiting,
      ECommsFailed
   };

   // Member variables
   RSocket&                 iSocket;
   CMessageHandler&           iConsole; // console for displaying text etc
   TBuf8<KMaxMessageLength> iTransferBuffer; // Accumulate data to send in here
   TBuf8<KMaxMessageLength> iWriteBuffer; // Holds data currently being sent to socket
   CTimeOutTimer*           iTimer;
   TInt                     iTimeOut;
   TWriteState              iWriteStatus;
};

#endif // __SOCKETSWRITE_H__
