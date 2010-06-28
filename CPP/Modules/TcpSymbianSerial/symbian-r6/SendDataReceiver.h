/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SENDDATARECEIVER_H
#define SENDDATARECEIVER_H

#include <e32base.h>

/**
 * This class enqueues the data that should be sent to a remote host
 * and portions it out in a suitable pace.
 * The queue contains data that is pushed in one thread and popped in
 * another.
 */
class CSendDataReceiver : public CActive
{
   /** A critical section for serialized access to the queue.*/
   class RCriticalSection iCritical;
   /** Pointer to the "gateway" to the main module class. */
   class CTcpAdmin* iAdmin;
   /** 
    * The thread if of the thread this object is running in. This is
    * the same thread that is popping data from thge queue. 
    */
   class TThreadId iThreadId;
   /** The queue of data. */
   CArrayPtrSeg<HBufC8>* iQueue;

   /** @name Constructors and destructor. */
   //@{
   /**
    * Constructor.
    * @param aAdmin    the owner.
    * @param aThreadId the thread id of the thread this object is running in.
    */
   CSendDataReceiver(class CTcpAdmin* aAdmin, class TThreadId aThreadId);
   /** Second phase constructor. */
   void ConstructL();
public:
   /**
    * Static constructor. 
    * @param aAdmin    the owner.
    * @param aThreadId the thread id of the thread this object is running in.
    */
   static class CSendDataReceiver* NewL(class CTcpAdmin* aAdmin,
                                        class TThreadId aThreadId);
   /** Virtual destructor. */
   virtual ~CSendDataReceiver();
   //}@
private:
   /** @name From CActive. */
   //@{
   virtual void RunL();
   virtual void DoCancel();
   //@}
public:
   /**
    * Sets the object as active. The request will complete when there
    * is data available for writing.
    */
   void RequestData();
   /** 
    * The SendData function is available as leaving and non-leaving
    * function.
    * The function is supposed to be called from another thread with
    * data that will be written by the thread this object runs in.
    * The data is enqueued and if the object is currently waiting for
    * data the object will be marked for completion in the proper
    * thread.
    * @param aData the data.
    * @param aLength the amount of data. 
    */
   //@{
   /** @return One of the system wide error codes. */
   TInt SendData(const TUint8* aData, TUint aLength);
   void SendDataL(const TUint8* aData, TUint aLength);
   //@}
};
#endif

