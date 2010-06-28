/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CONNECTIONCONTROL_H
#define CONNECTIONCONTROL_H
#include "Module.h"
class CConnectionControl : public CActive
{
   class CConnectionParams : public CBase {
      CConnectionParams(enum isab::Module::ConnectionCtrl aAction);
      void ConstructL(const char* aMethod);
      TInt Construct(const char* aMethod);
   public:
      enum isab::Module::ConnectionCtrl iAction;
      HBufC8* iMethod;

      static class CConnectionParams* 
      NewLC(enum isab::Module::ConnectionCtrl aAction, const char* aMethod);
      static class CConnectionParams*
      New(enum isab::Module::ConnectionCtrl aAction, const char* aMethod);
      virtual ~CConnectionParams();
   };

   /** Data members*/
   //@{
   /** Synchronization semaphore. */
   class RCriticalSection iCritical;
   /** XXX */
   class CTcpAdmin* iAdmin;
   /** The thread id of the Tcp comm thread.*/
   class TThreadId iThreadId;
   /** Used to queue commands. */
   CArrayPtrSeg<CConnectionParams>* iCommands;
   /** Helps keep track of whether this object is active or not. */
   class TRequestStatus* iStatusPtr;
   //@}
   /** Constructors and destructor */
   //@{
   /**
    * Constructor.
    * @param aAdmin    the owner.
    * @param aThreadId the thread id of the thread this object is running in.
    */
   CConnectionControl(class CTcpAdmin* aAdmin, class TThreadId aThreadId);
   /** Second phase constructor. */
   void ConstructL();
public:
   /**
    * Static constructor. 
    * @param aAdmin    the owner.
    * @param aThreadId the thread id of the thread this object is running in.
    */
   static class CConnectionControl* NewL(class CTcpAdmin* aAdmin, 
                                         const class TThreadId& aThreadId);
   /** Virtual destructor. */
   virtual ~CConnectionControl();
   //@}
private:
   /** From CActive. */
   //@{
   virtual void RunL();
   virtual TInt RunError(TInt aError);
   virtual void DoCancel();
   //@}

   TBool ActivateSelf();
   TBool CompleteSelf(TInt aStatus);

   void Request();
public:
   ///Controls the tcp connections. A indirection layer from isab::Module.
   ///@param aAction what to do (Connect, Disconnect, Query).
   ///@param aMethod a string argument to the action.
   ///@return KErrNone if all is well, otherwise the leave code of ConnectionL.
   TInt Connection(enum isab::Module::ConnectionCtrl aAction,
                   const char* aMethod);
   ///Controls the tcp connections.
   ///@param aAction what to do (Connect, Disconnect, Query).
   ///@param aMethod a string argument to the action.
   void ConnectionL(enum isab::Module::ConnectionCtrl aAction,
                    const char* aMethod);

};
#endif
