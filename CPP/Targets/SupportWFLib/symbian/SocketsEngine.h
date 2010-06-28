/* Copyright (c) 2001, Nokia Mobile Phones. All rights reserved */

#ifndef SOCKETSENGINE_H
#define SOCKETSENGINE_H

#include <in_sock.h>
#include "TimeOutNotify.h"
#include "SymbianTcpConnection.h"

#include "SocketsRead.h"
#include "SocketsWrite.h"
#include "TimeOutTimer.h"
#include "machine.h"

#if defined(NAV2_CLIENT_SERIES60_V2) || defined(NAV2_CLIENT_SERIES60_V3) || defined(NAV2_CLIENT_SERIES90_V1) || defined(NAV2_CLIENT_SERIES80) || defined(NAV2_CLIENT_UIQ3)
//symbian 7.0s and later
#  define SYMBIAN_GTE_7S
#elif defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_UIQ)
//symbian 7.0 and earlier
#  define SYMBIAN_LTE_7
#endif

#if defined SYMBIAN_LTE_7
# include <agentclient.h>
#endif

/** 
 * CNewSocketsEngine
  
 * This class is the main engine part of the sockets application.
 * It establishes a TCP connection using its server name and port number (performing a DNS lookup
 * operation first, if appropriate).
 * It creates instances of separate active objects to perform reading from, and writing to, the socket.
 */
class CNewSocketsEngine : public CActive, 
                          public MTimeOutNotify
{

public: // Constructors & Destructor

   /**
    * NewL
    * Create a CNewSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CNewSocketsEngine
    */
   static class CNewSocketsEngine* NewL(class SymbianTcpConnection& aConsole, 
                                        class RSocketServ* aSockServ );

   /**
    * NewLC
    * Create a CNewSocketsEngine object
    * @param aConsole console to use for ui output
    * @return a pointer to the created instance of CNewSocketsEngine
    */
   static class CNewSocketsEngine* NewLC(class SymbianTcpConnection& aConsole, 
                                         class RSocketServ* aSockServ);

   /**
    * ~CNewSocketsEngine
    * Destroy the object and release all memory objects
    */
   virtual ~CNewSocketsEngine();

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
   void WriteL( TDesC8 &data, int length );

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
    * SetIAP
    * Set used IAP
    * @param aPort number of new IAP
    */
   void SetIAP(TInt aIAP);

   /**
    * GetIAP
    * Get the used IAP
    * @return IAP number
    */
   TInt GetIAP() const;
   
   /**
    * IsConnected
    * Is socket fully connected?
    * @return true if socket is connected
    */
   TBool IsConnected() const;

   /**
    * SocketsEngineCancel
    * Is called when an active connection/download
    * should be cancelled.
    */
   void SocketsEngineCancel();

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
    * CNewSocketsEngine
    * Perform the first phase of two phase construction
    * @param aConsole the console to use for ui output
    */
   CNewSocketsEngine( class SymbianTcpConnection& aConsole, 
                      class RSocketServ* aSockServ );

   /**
    * ConstructL
    * Perform the second phase construction of a CNewSocketsEngine 
    */
   void ConstructL();

   /**
    * FinalizeConnectionL
    * finalize the connetion by resolving the address (if needed)
    * and connecting the socket using ConnectL(TUint32 aAddr)
    */
   void FinalizeConnectionL();
   
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

   /**
    * ChangeStatus
    * handle a change in this object's status
    * @param aNewStatus new status
    */
   void ChangeStatus(enum SymbianTcpConnection::TSocketsEngineState aNewStatus);

private: // Member variables

   /// This object's current status
   enum SymbianTcpConnection::TSocketsEngineState   iEngineStatus;

   /// Console for displaying text etc
   class SymbianTcpConnection& iConsole;
      
   /// The RConnection Connection Manager objects
# if defined SYMBIAN_GTE_7S
   class RConnection           iConnMgr;
# elif defined SYMBIAN_LTE_7
   class RGenericAgent         iNetAgent;
   class CStoreableOverrideSettings*   iCommsOverrides;
# endif
      
   /// Socket reader active object
   class CNewSocketsRead*      iSocketsRead;

   /// Socket writer active object
   class CNewSocketsWrite*     iSocketsWrite;

   /// XXX A socket used for listening, will be removed
   /// when the connection protocol is changed.
   class RSocket               iListenSocket;

   /// The actual socket XXX initially only used for writing.
   class RSocket               iSocket;

   /// The socket server
   class RSocketServ*          iSocketServ;

   /// DNS name resolver
   class RHostResolver         iResolver;

   /// Package for the TNameRecorder
   TNameEntry            iNameEntry;

   /// Result container of a name queries
   class TNameRecord           iNameRecord;

   /// Timer active object
   class CTimeOutTimer*        iTimer;

   class TInetAddr             iAddress;

   /// Port number to connect to
   TInt                  iPort;

   /// IAP number to use
   TInt                  iIAP;

   /// Server name to connect to
   TBuf<128>             iServerName;

   // Indicates that a cancel request 
   // has been received
   TBool iCancel;

};

#endif // __SOCKETSENGINE_H__
