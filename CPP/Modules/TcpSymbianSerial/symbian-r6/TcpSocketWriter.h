/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPSOCKETWRITER_H
#define TCPSOCKETWRITER_H
#include <e32base.h>
#include "TimeOutNotify.h"

namespace isab {
   class Log;
   class LogMaster;
}

/**
 * Active object that handles the writing part of a tcp connection.
 */
class CTcpSymbianWrite : public CActive, public MTimeOutNotify
{
public:
   /** Enumeration of possible states*/
   enum TWriteState{
      /** An asynchrnous write is pending. */
      ESending, 
      /** Waiting for a new write order. */
      EWaiting,
      /** Something timed out. We probably never leave this state. */
      ETimedOut,
   };

   /** @name Constructors and destructor. */
   //@{
   /** 
    * Static constructor.
    * @param aSocket  pointer to the socket handle. 
    * @param aConsole receives write reports.
    * @param aEngine  receives error reports.
    * @return a new CTcpSymbianWrite object.
    */
   static class CTcpSymbianWrite* NewL(class RSocket& aSocket, 
                                       class MTcpSymbianSocketUser& aConsole,
                                       class CTcpSymbianEngine& aEngine,
                                       class isab::LogMaster* aLogMaster);
   /** 
    * Static constructor.
    * @param aSocket  pointer to the socket handle. 
    * @param aConsole receives write reports.
    * @param aEngine  receives error reports.
    * @return a new CTcpSymbianWrite object. The object is pushed to the
    *         cleanupstack.
    */
   static class CTcpSymbianWrite* NewLC(class RSocket& aSocket, 
                                        class MTcpSymbianSocketUser& aConsole,
                                        class CTcpSymbianEngine& aEngine,
                                        class isab::LogMaster* aLogMaster);
   /** Virtual destructor. */
   virtual ~CTcpSymbianWrite();
private:
   /** Second phase constructor. 
    * @param aSocket  pointer to the socket handle. 
    * @param aConsole receives write reports.
    * @param aEngine  receives error reports.
    */
   void ConstructL(class isab::LogMaster* aLogMaster);
   /** Default constructor. */
   CTcpSymbianWrite(class RSocket& aSocket, 
                    class MTcpSymbianSocketUser& aConsole,
                    class CTcpSymbianEngine& aEngine);
   //@}
public:
   /**
    * Start a write operation. 
    * @param aData the data to write. The CTcpSymbianWrite object takes
    *              ownership of the buffer object. 
    */
   void IssueWrite(HBufC8* aData);
private:
   /** @name Implemented functions from CActive. */
   //@{
   void DoCancel();  
   void RunL(); 
   //@}

   /** @name Implemented functions from MTimeOutNotify. */
   //@{
   void TimerExpired(); 
   //@}
private:
   /** Pointer to the RSocket handle.  */
   class RSocket& iEchoSocket;
   /**
    * Pointer to the traffic and connection handling interface. Each
    * time a write completes CTcpSymbianWrite calls WriteComplete on this
    * pointer. 
    */
   class MTcpSymbianSocketUser& iConsole;
   /** Pointer to the owning CTcpSymbianEngine. Receives error reports. */
   class CTcpSymbianEngine& iEngine;
   /** A timer object so that we can handle write timeouts. */
   class CTimeOutTimer* iTimer;
   /** The timeout interval. */
   TInt iTimeOut;
   /** The current write status. */
   enum TWriteState iWriteStatus;
   /**
    * The data that is or has been written. This buffer is replaced on
    * each call to IssueWrite. 
    */
   HBufC8* iBuffer;
   class isab::Log* iLog;
};
#endif
