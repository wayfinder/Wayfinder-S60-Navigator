/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPSYMBIANENGINE_H
#define TCPSYMBIANENGINE_H

#include <e32std.h>
#include <e32base.h>
#include <es_sock.h>
#include <in_sock.h>
#ifndef SYMBIAN_9
# include <agentclient.h>
#endif
#include "TimeOutNotify.h"
#include "Module.h"
#include "DNSCache.h"

//#if defined NAV2_CLIENT_UIQ3 || defined __WINS__ || defined __WINSCW__
#if defined __WINS__ || defined __WINSCW__
//neither RCONNECTION nor RAGENT
#elif defined SYMBIAN_7S 
//symbian7.0s and later use rconnection
# define RCONNECTION
# include <commdbconnpref.h>
#elif defined SYMBIAN_7
//symbian 7 and earlier use ragent
# define RAGENT
#else
# error You want either RCONNECTION or RAGENT here.

namespace isab {
   class LogMaster;
   class Log;
}
#endif 

namespace isab {
   class LogMaster;
   class Log;
}

/** Timeout for all Tcp operations.  */
static const TInt KTimeOut = 30000000; // 130 seconds time-out

/** Panic string for CTcpSymbianEngine.*/
_LIT(KEchoEngine, "TSSEngine");
/**Panics for CTcpSymbianEngine, CTcpSymbianRead, and CTcpSymbianWrite.*/
enum TEchoPanic{
   /** trying to switch server while connected */
   ESwitchServer         = 0, 
   /** Write called while not connected */
   EWriteNotConnected    = 1,
   /** Write called while writer is active. */
   EWriteActive          = 2,
   /** Unhandled state in DoCancel. */
   EBadCancelStatus      = 3,
   /** Read called while not connected. */
   EReadNotConnected     = 4,
   /** Unhandled state in RunL. */
   EBadEngineState       = 5,
   /** Unhandled state in Stop. */
   EBadEngineStopState   = 6,
   /** Unhandled state in CTcpSymbianRead::RunL */
   EUnhandledReadStatus  = 7,
   /** Unhandled state in CTcpSymbianWrite::RunL */
   EUnhandledWriteStatus = 8,
   /** Unhandled write state in CTcpSymbianWrite::RunL. */
   EBadWriteState        = 9,
};


/** CTcpSymbianEngine: main engine class for connection and shutdown */
class CTcpSymbianEngine : public CActive, public MTimeOutNotify
{
public:
   /** Engine status enumeration */
   enum TEchoEngineState{
      EComplete      =  0,/** Ready for new connection.  */
      EConnecting    =  1,/** Waiting for socket connect */
      EConnected     =  2,/** Socket is connected, data transfer in progress.*/
      ETimedOut      =  3,/** An operation timed out. */
      ELinking       =  4,/** Connecting to link layer (GRPS) */
      ELinkFailed    =  5,/** Link layer connection failed.  */
      ELookingUp     =  6,/** Waiting on DNS lookup completion. */
      ELookUpFailed  =  7,/** DNS lookup has failed */
      EConnectFailed =  8,/** Socket connect failed. */
      EClosing       =  9,/** Waiting for socket shutdown. */
      EBroken        = 10,/** Connection was broken.  */
      ESwitching     = 11,/** Switching server. Set while the closing
                           *  part of the switching is in progress. */
      EClosed        = 12,/** Socket was closed by other end, or proxy. */
   };

   /** @name Constructors and destructor.*/ 
   //@{
private:
   ///Constructor.
   ///@param aConsole 
   CTcpSymbianEngine(class MTcpSymbianSocketUser& aConsole);
   ///Second phase constructor.
   void ConstructL(class isab::LogMaster* aLogMaster);
public:
   ///Static constructor.
   ///@param aConsole pointer to datareceiver class. 
   ///@return a new CTcpSymbianEngine object.
   static class CTcpSymbianEngine* NewL(class MTcpSymbianSocketUser& aConsole, class isab::LogMaster* aLogMaster);
   ///Static constructor.
   ///@param aConsole pointer to datareceiver class. 
   ///@return a new CTcpSymbianEngine object, that has been pushed to the 
   ///        cleanup stack.
   static class CTcpSymbianEngine* NewLC(class MTcpSymbianSocketUser& aConsole, class isab::LogMaster* aLogMaster);
   ///Virtual destructor. 
   virtual ~CTcpSymbianEngine();
   //@}

   ///Close any current connection. Report the connection as closed by
   ///request to iConsole.
   void Stop();
   ///Close any current connection. Report the reason for the closed
   ///connection based on arguments.
   ///@param aState the enginestate to set. Handled in the next
   ///              invocation of RunL.
   void Stop(enum TEchoEngineState aState);
   ///Start an asynchronous socket connection operation. Also start
   ///the timeout timer.
   ///@param aAddr the IP number of the remote host. 
   ///@param aPort the port to connect to. 
   void Connect(TUint32 aAddr, TUint aPort);
   ///Widens the host name to a UCS16 string and then calls the other 
   ///ConnectL function.
   ///@param aServerName the name of the remote host. 
   ///@param aPort the port to connect to. 
   ///@param aIAP the IAP to use for the link layer connection.
   ///@param aConsiderWLAN Whether to consider WLAN IAP/APN for this
   ///                     connection.
   void ConnectL(const TDesC8& aServerName, TUint aPort, TInt aIAP, 
                 TBool aConsiderWLAN);
   /**
    * Set the iHostName, iIAP, and iPort values and call ConnectL().
    * @param aServerName the name of the remote host. 
    * @param aPort the port to connect to. 
    * @param aIAP the IAP to use for the link layer connection. 
    * @param aConsiderWLAN Whether to consider WLAN IAP/APN for this
    *                      connection.
    */
   void ConnectL(const TDesC& aServerName, TUint aPort, TInt aIAP, 
                 TBool aConsiderWLAN);
private:
   /**
    * Start the connection sequence (link, lookup, connect) using the
    * current values of iHostName, iPort, and iIAP. If the enginstate
    * is anything but EComplete nothing will happen and the next
    * message on queue will be signalled.
    */
   void ConnectL();
public:
   ///Tests whether the current engine state is EConnected. 
   ///@return ETrue if the engine state is EConnected, EFalse otherwise.
   TBool IsConnected();
   ///Triggers a callback to MTcpSymbianSocketUser::ConnectionNotify.
   void Query();

private:
   ///Start an asynchronous DNS lookup operation of the host in the
   ///iHostName member variable. Uses a timeout timer. 
   void StartLookupL();

   ///Starts an asynchronous link layer connection operation and a
   ///companion timeout timer. This operation will use the IAP index
   ///stored in iIAP.
   void OpenLinkLayerL();
   ///Starts an asynchronous link layer connection operation using the
   ///RConnection class. This function is a no-op for the except UIQ
   ///and S60v1 platforms.
   void OpenLinkLayerConnectionL();
   ///Starts an asynchronous link layer connection operation using the
   ///RGenricAgent class. This function is a no-op for all platforms
   ///except UIQ and S60v1.
   void OpenLinkLayerAgentL();
   /**
    * Just completes immideately with KErrNone. Used when we don't
    * want neither RConnection nor RGenericAgent.
    */
   void OpenFakeLinkLayerL();
   
   ///Close the link layer. 
   void CloseLinkLayer();

   ///Opens a tcp sockets. Leaves if there is any error.
   void OpenSocketL();
   ///Opens a tcp socket.
   ///@return the return value of RSocket::Open.
   TInt OpenSocket();

public:
   ///Start an asynchronous write operation. If this funciton is
   ///called while a write operation is outstanding, the thread will
   ///panic.  
   ///@param aData the data to write. CTcpSymbianEngine takes ownership of 
   ///             the HBufC8 object.
   void Write(HBufC8* aData);
private:
   ///Start an asynchronous read operation. 
   void Read();

   void CompleteSelf(TInt aCompleteStatus);

   ///@name Implemented functions from MTimeOutNotify
   ///Used by the timeout timers for link, lookup, and connect operations.
   //@{
   void TimerExpired(); 
   //@}

   ///@name Implemented functions from CActive
   //@{
   void DoCancel();
   void RunL();
   TInt RunError(TInt aError);
   //@}
private:
   ///Reset the engine so that it is ready for new connections. 
   ///When this function returns, the state will be set to EComplete.
   ///May leave if RSocket::Open fails.
   ///@param aDuring the state during this function call
   void ResetL(enum TEchoEngineState aDuring);
   ///Reset the engine so that it is ready for new connections.  Calls
   ///MTcpSymbianSocketUser::ConnectionNotify with the connection state
   ///variable set to DISCONNECTING and the reason value from this
   ///function's argument.  When this function returns, the state will
   ///be set to EComplete.
   ///May leave if RSocket::Open fails.
   ///@param aDuring the state during this function call
   ///@param aReason the value of the reason argument in the call to 
   ////              ConnectionNotify
   void ResetL(enum TEchoEngineState aDuring,
               enum isab::Module::ConnectionNotifyReason aReason);

   ///Returns the current state. Protected by a RCriticalSection.
   ///@return the current value of iEngineStatus;
   enum TEchoEngineState EngineState();
   ///Sets the state. Protected by a RCriticalSection.
   ///@param aState the new value of iEngineStatus;
   void SetEngineState(enum TEchoEngineState aState);

   /**
    * Sets the member variables iHostName, iPort, and iIAP from the
    * three arguments.
    * @param aServerName The new servername. 
    * @param aPort The new port.
    * @param aIAP The new IAP.
    * @param aConsiderWLAN The new considerWLAN setting.
    */
   void SetConnectionParamsL(const TDesC& aServerName, 
                             TUint aPort, TInt aIAP, TBool aConsiderWLAN);


   ///Log object
   class isab::Log* iLog;
   ///Critical region used to protect iEngineStatus.
   class RCriticalSection iCritical;
   ///The state machine variable.
   enum TEchoEngineState iEngineStatus;
   ///Used to report read data, completed writes, and connection status.
   class MTcpSymbianSocketUser& iConsole;
   ///Active object for reads.
   class CTcpSymbianRead* iEchoRead;
   ///Active object for reads.
   class CTcpSymbianWrite* iEchoWrite;
   ///The Socket handle.
   class RSocket iEchoSocket;
   ///The socket server handle
   class RSocketServ iSocketServ;

# ifdef RCONNECTION
   ///The Link layer handle for S80, S90, and S60v2
   class RConnection iConnection;
   ///Preferences when starting the connection manager. 
   class TCommDbConnPref iPrefs;
# elif defined RAGENT
   ///The Link layer handle for UIQ and S60v2
   class RGenericAgent* iAgent;
   ///Link layes settings.
   class CStoreableOverrideSettings *iCommsOverrides;
   ///Preferences when starting the connection manager. 
   class CCommsDbConnectionPrefTableView::TCommDbIapConnectionPref iPrefs;
# else
   //use neither
# endif 

   /** Cache of looked-up hosts. */
   class RDNSCache iDNSCache;
   ///DNS lookup handle. 
   class RHostResolver iResolver; 
   ///A descriptor layer for the TNameRecord.
   TNameEntry iNameEntry; //this is a typedef for TPckgBuf<TNameRecord>
   ///Timer used to stop uncompleted link, lookup, or connect operations. 
   class CTimeOutTimer* iTimer;
   ///The timeout time in microseconds.
   TInt iTimeOut;
   ///The remote host IP address. 
   class TInetAddr iAddress;
   ///The remote host name. 
   HBufC* iHostName;
   ///The TCP port of the remote host. 
   TUint iPort;
   ///The IAP to use. 
   TInt iIAP;
   /** Whether to consider WLAN IAP/APN or not. */
   TBool iConsiderWLAN;
};

#endif

