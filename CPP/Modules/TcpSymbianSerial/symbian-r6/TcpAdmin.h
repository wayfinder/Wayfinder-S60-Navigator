/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPADMIN_H
#define TCPADMIN_H
#include "ThreadOwner.h"
#include "TcpSocketEngine.h"
#include "Module.h" //enum Connection\(Control\|Notify\(Reason\)?\)
#include "SocketUser.h"
#include "TcpEventGenerator.h"
#include "TcpEvent.h"

//classes from other namespaces can't be tentatively declared.
namespace isab{
   class TcpSymbianSerial; 
   class LogMaster;
}

///Tests if the currently running thread has the same ID as the argument.
///@param aId the thread id to compare against.
///@retrun ETrue if the current thread's id matches aId.
inline TBool IsThread(class TThreadId aId) { 
   class RThread t; 
   return t.Id() == aId; 
}

///This is the class that administers the Tcp thread. It queues
///buffers from TcpSymbianSerial for sending, reads incoming data and
///writes outgoing data. Also handles connections and disconnections
///to the server.
class CTcpAdmin : public CThreadAdmin, 
                  public MTcpSymbianSocketUser, 
                  public MTcpEventHandler
{
   ///The receiver of incoming data.
   class isab::TcpSymbianSerial* iTss;
   ///LogMaster object.
   class isab::LogMaster* iLogMaster;
   ///
   class isab::Log* iLog;
   ///The id of the tcpthread.
   class TThreadId iThreadId;
   ///The engine that handles server connections. Has its own active
   ///objects that handles asynchronous reads and writes.
   class CTcpSymbianEngine* iSocketsEngine;
   ///The IAP we should use. 
   TInt iIAP;
   ///Critical region to protect the iIAP member variable. 
   class RCriticalSection iCritical;
   /**
    * Events from TcpSymbianSerial will be queued here. 
    */
   class CTcpEventGenerator* iGenerator;
   /**
    * If set will allow IAP/APN selection to select WLAN points. 
    */
   TBool iConsiderWLAN;
public:
   ///Constructor.  
   ///@param aTss a pointer to the TcpSymbian object that should receive 
   ///            data read from the network.
   CTcpAdmin(class isab::TcpSymbianSerial* aTss, class isab::LogMaster* aLogMaster);
   ///Virtual destructor. 
   virtual ~CTcpAdmin();
   ///@name From CThreadAdmin
   //@{
   virtual void StartL();
   virtual void StopL();
   //@}
   ///@name from MTcpSymbianSocketUser
   //@{
   virtual void ReceiveData(const TDesC8& aData);
   virtual void ConnectionNotify(enum isab::Module::ConnectionNotify,
                                 enum isab::Module::ConnectionNotifyReason);
   virtual void BlockEvents();
   ///This function is called by CTcpSymbianWrite::RunL.
   virtual void OperationComplete();
   virtual void Panic(TInt aReason);
   //@}

   ///Controls the tcp connections. A indirection layer from isab::Module.
   ///@param aAction what to do (Connect, Disconnect, Query).
   ///@param aMethod a string argument to the action.
   ///@return KErrNone if all is well, otherwise on of the system wide 
   ///        error codes. 
   TInt ConnectionControl(enum isab::Module::ConnectionCtrl aAction,
                          const char* aMethod);
   ///This function is called by CConnectionControl::RunL.
   TInt SocketsConnectL(enum isab::Module::ConnectionCtrl aAction,
                        const TDesC8* aMethod); 

   TBool IsConnected();
   ///Sets the connection parameter, which is the IAP to use.
   ///@param aIAP the IAP to use.
   void SetConnectionParam(TInt aIAP);
   ///Gets the connection parameter.
   ///@return the iap to use. 
   TInt ConnectionParam();

   void ConsiderWLAN(TBool aConsider = ETrue);

   ///Queues new data to send.
   ///@param aData pointer to the data to send. 
   ///@param aLength how much data to send. 
   TInt SendData(const TUint8* aData, TInt aLength);
   ///This function is called by CSendDataReceiver::RunL
   TInt WriteToTcp(HBufC8* aData);

   /**
    * @name From MTcpEventHandler.
    */
   //@{
   virtual TInt HandleConnectionEventL(class CTcpConnectionEvent& aEvent);
   virtual TInt HandleDataEventL(class CTcpDataEvent& aEvent);
   virtual TInt HandleRunError(TInt aError);
   //@}
};


/**
 * This class creates CTcpAdmin objects. 
 */
class CTcpAdminFactory : public CThreadAdminFactory
{
   /**
    * The LogMaster that will be passed on to the CTcpAdmin object.
    */
   class isab::LogMaster* iLogMaster;
   /**
    * The TcpSymbianSerial object that owns this CTcpAdminFactory.
    */
   class isab::TcpSymbianSerial* iTss;
   /**
    * The CTcpAdmin created by this factory. 
    */
   class CTcpAdmin* iAdmin;
   /**
    * Private constructor. 
    * @param aTss Pointer to the owning TcpSymbianSerial object. 
    */
   CTcpAdminFactory(class isab::TcpSymbianSerial* aTss);
public:
   /**
    * Static constructor. 
    * @param aTss Pointer to the owning TcpSymbianSerial object. 
    * @param aName The name that will be used for the thread. 
    * @param aLogMaster pointer to LogMaster that will be passed to the CTcpAdmin object. 
    * @return A new CTcpAdminFactory object. 
    */
   static class CTcpAdminFactory* NewL(class isab::TcpSymbianSerial* aTss, 
                                       const TDesC& aName,
                                      class isab::LogMaster* aLogMaster);
   /**
    * Non-leaving static constructor. 
    * @param aTss Pointer to the owning TcpSymbianSerial object. 
    * @param aName The name that will be used for the thread. 
    * @param aLogMaster pointer to LogMaster that will be passed to the CTcpAdmin object. 
    * @return A new CTcpAdminFactory object. 
    */
   static class CTcpAdminFactory* New(class isab::TcpSymbianSerial* aTss, 
                                      const TDesC& aName,
                                      class isab::LogMaster* aLogMaster);
   /**
    * Create a admin object. 
    */
   virtual class CThreadAdmin* CreateAdminL();
   /**
    * Return the created CTcpAdmin object that was created. 
    */
   class CTcpAdmin* CreatedObject() const;   
};


#endif
