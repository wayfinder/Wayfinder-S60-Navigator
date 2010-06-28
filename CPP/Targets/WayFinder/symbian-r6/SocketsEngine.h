/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __SOCKETSENGINE_H__
#define __SOCKETSENGINE_H__

#include <in_sock.h>
#include "TimeOutNotify.h"
#include "wayfinder.hrh"
#include "WayFinderConstants.h"

class CSocketsRead;
class CSocketsWrite;
class CTimeOutTimer;
class CMessageHandler;

namespace isab{
   class Buffer;
}

using namespace isab;

/** 
 * CSocketsEngine
  
 * This class is the main engine part of the sockets application.
 * It establishes a TCP connection using its server name and port number (performing a DNS lookup
 * operation first, if appropriate).
 * It creates instances of separate active objects to perform reading from, and writing to, the socket.
 */
class CSocketsEngine : public CActive, 
                       public MTimeOutNotify    
{

public: // Constructors & Destructor

   /**
    * NewL
    * Create a CSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CSocketsEngine
    */
   static CSocketsEngine* NewL( CMessageHandler& aConsole );

   /**
    * NewLC
    * Create a CSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CSocketsEngine
    */
   static CSocketsEngine* NewLC( CMessageHandler& aConsole);

   /**
    * ~CSocketsEngine
    * Destroy the object and release all memory objects
    */
   ~CSocketsEngine();

public: // new methods

   /**
    * ConnectL
    * Initiate connection of socket, using iServerName and iPort
    */
   void ConnectL();

   /**
    * Disconnect
    * Disconnect socket
    */
   void Disconnect();

   /**
    * WriteL
    * Write data to socket
    * @param aData data to be written
    */
   void WriteL( Buffer* aBuffer );

   /**
    * WriteL
    * Write data to socket
    */
   void WriteL( const TDesC8& aBytes, TInt aLength );

   /**
    * Read
    * Initiate read of data from socket
    */
   void Read();

   /**
    * SetServerName
    * Set name of server to connect to
    * @param aName new server name
    */
   void SetServerName(const TDesC& aName);

   /**
    * ServerName
    * Get server name
    * @return name of server
    */
   const TDesC& ServerName() const;

   /**
    * SetPort
    * Set port number to connect to
    * @param aPort new port number
    */
   void SetPort(TInt aPort);

   /**
    * Port
    * Get port number
    * @return port number
    */
   TInt Port() const;

   /**
    * Connected
    * Is socket fully connected?
    * @return true if socket is connected
    */
   TBool Connected() const;

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
    * CSocketsEngine
    * Perform the first phase of two phase construction
    * @param aConsole the console to use for ui output
    */
   CSocketsEngine( CMessageHandler& aConsole );

   /**
    * ConstructL
    * Perform the second phase construction of a CSocketsEngine 
    */
   void ConstructL();

   /**
    * ConnectL
    * initiate a connect operation on a socket
    * @param aAddr the ip address to connect to
    */
   void ConnectL( TUint32 aAddr );

   /**
    * Listen for connections.
    * XXX shuold become deprecated.
    */
   void Listen();

   enum TSocketsEngineState 
   {
      ENotConnected,
      EConnecting,
      EListening,
      EConnected,
      ETimedOut, 
		ELookingUp,
      ELookUpFailed,
      EConnectFailed,
      EDisconnecting
   };

   /**
    * ChangeStatus
    * handle a change in this object's status
    * @param aNewStatus new status
    */
   void ChangeStatus(TSocketsEngineState aNewStatus);

   /**
    * Print
    * display text on the console
    * @param aDes text to display
    */
   void Print(const TDesC& aDes);

private: // Member variables

   /// This object's current status
   TSocketsEngineState   iEngineStatus;

   /// Console for displaying text etc
   CMessageHandler&            iConsole;

   /// Socket reader active object
   CSocketsRead*         iSocketsRead;

   /// Socket writer active object
   CSocketsWrite*        iSocketsWrite;

   /// XXX A socket used for listening, will be removed
   /// when the connection protocol is changed.
   RSocket               iListenSocket;

   /// The actual socket XXX initially only used for writing.
   RSocket               iSocket;

   /// The socket server
   RSocketServ           iSocketServ;

   /// DNS name resolver
   RHostResolver         iResolver;

   /// Package for the TNameRecorder
   TNameEntry            iNameEntry;

   /// Result container of a name queries
   TNameRecord           iNameRecord;

   /// Timer active object
   CTimeOutTimer*        iTimer;

   TInetAddr             iAddress;

   /// Port number to connect to
   TInt                  iPort;

   /// Server name to connect to
   TBuf<KBuf64Length>      iServerName;

};

#endif // __SOCKETSENGINE_H__
