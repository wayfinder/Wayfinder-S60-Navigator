/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __GPSSOCKET_H__
#define __GPSSOCKET_H__

#include <in_sock.h>
#include "TimeOutNotify.h"
#include "Nav2.h"

class CTimeOutTimer;

/** 
 * CGpsSocket
  
 * This class is the main engine part of the sockets application.
 * It establishes a TCP connection using its server name and port number (performing a DNS lookup
 * operation first, if appropriate).
 * It creates instances of separate active objects to perform reading from, and writing to, the socket.
 */
class CGpsSocket : public CActive, 
                   public MTimeOutNotify    
{

public: // Constructors & Destructor

   static class CGpsSocket* CGpsSocket::NewL(isab::Nav2::Channel* aNav2Channel,
                                             const TDesC& aHost);
   /**
    * NewL
    * Create a CGpsSocket object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CGpsSocket
    */
   static CGpsSocket* NewL( isab::Nav2::Channel* aNav2Channel );

   /**
    * NewLC
    * Create a CGpsSocket object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CGpsSocket
    */
   static CGpsSocket* NewLC( isab::Nav2::Channel* aNav2Channel );

   /**
    * ~CGpsSocket
    * Destroy the object and release all memory objects
    */
   ~CGpsSocket();

public: // new methods

   /**
    * Initiate connection of socket, using iServerName and iPort
    */
   void ConnectL();

   /**
    * Disconnect socket
    */
   void Disconnect();

   /**
    * Return true if we're in state connected.
    */
   TBool IsConnected();

public: // from MTimeOutNotify

   /**
    * TimerExpired
    * The function to be called when a timeout occurs
    */
   void TimerExpired(); 

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

private: // New methods

   /**
    * Perform the first phase of two phase construction
    * @param aConsole the console to use for ui output
    */
   CGpsSocket( isab::Nav2::Channel* aNav2Channel );

   /**
    * Perform the second phase construction of a CGpsSocket 
    */
   void ConstructL();
   void ConstructL(const TDesC& aHost);
   void SetServerName(const TDesC& aName);

   /**
    * Write data to socket
    */
   void SendData();

   /**
    * Initiate read of data from socket
    */
   void ReadData();

   enum TSocketState 
   {
      ENotConnected,
      EConnecting,
      EConnected,
      EDisconnecting
   };

private: // Member variables

   /// This object's current status
   TSocketState   iState;

   /// Console for displaying text etc
   CWayFinderAppUi*       iWfAppUi;

   /// The actual socket only used for read.
   RSocket               iSocket;

   /// The socket server
   RSocketServ           iSocketServ;

   /// Timer active object
   CTimeOutTimer*        iTimer;
   
   TInt                  iCounter;

   TInetAddr             iAddress;

   /// Port number to connect to
   TInt                  iPort;

   /// Server name to connect to
   TBuf<64>    iServerName;
   
   /// Buffer for reciving the data
   TBuf8<64> iBuf;

   isab::Nav2::Channel* iNav2Channel;

};

#endif // __GPSSOCKET_H__
