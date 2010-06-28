/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ACTIVE_DELAY_H
#define ACTIVE_DELAY_H
#include <e32base.h>
#include "Completer.h"

/** Panics for the CActiveDelay and CActiveDelayBase class.*/
enum TActiveDelayPanics {
   /** RunL was called when it was not supposed to. */
   EBadState     = 0,
   /** Tried to start the asynchronous request without specifying one. */
   ENoActiveCall = 1,
   /** Tried to cancel the asynchronous request without cancel fucntion. */
   ENoCancelCall = 2,
};

/**
 * This class is only supposed to be used as a base class for the
 * template class CActiveDelay. The CActiveDelayBase class
 * encapsulates all aspects of the CActiveDelay class that doesn't
 * depend on the type CActiveDelay is instantiated on. This is the
 * Thin Template pattern.
 */
class CActiveDelayBase : public CTimer, private MCompleter
{
protected:   
   /**
    * Virtual destructor. 
    */
   virtual ~CActiveDelayBase();
   /**
    * Constructor. 
    * Active object priority is set to EPriorityStandard.
    * @param aSeconds The number of seconds the asynchronous call will
    *                 be delayed.
    */
   CActiveDelayBase(class TTimeIntervalSeconds aSeconds);
   /**
    * Second phase constructor. 
    */
   void ConstructL();
   /**
    * Sets up the MCompleter and starts the delay.
    * @param aStatus The TRequestStatus variable used for the
    *                asynchronous event.
    * @return One of the system wide error codes. 
    */
   TInt BaseStart(class TRequestStatus* aStatus);
   /**
    * Handles the cancel if it occurs during the delay. If called
    * during the asynchronous call it will call CancelRealCall.
    */
   void BaseCancelDelayedCall();
   /**
    * Cancels the asynchronous request.
    */
   virtual void CancelRealCall() = 0;
   /**
    * Starts the asynchronous request.
    */
   virtual void StartRealCall() = 0;
   /** @name From CActive. */
   //@{
   virtual void DoCancel();
   virtual void RunL();
   //@}

   /**
    * Used for CActiveDelay panics.
    * @param aPanic The panic code. 
    */
   static void Panic(enum TActiveDelayPanics aPanic);

private:
   /** The states of a CActiveDelayBase object. */
   enum TDelayState {
      /** No request running. */
      EIdle,
      /** The delay is active. */
      EWaiting,
      /** The asynchronous request is active. */
      ERunning,
   };
   /** The current state. */
   enum TDelayState iDelayState;
   /** The delay used. */
   class TTimeIntervalMicroSeconds32 iDelay;
};

/**
 * This thin templat class is used to delay an asynchrnous function
 * call. The supported asynchronous and cancel fucntions are typedefed
 * as TActiveCall and TCancelCall. The cancel function is not necessary.
 */
template<class T>
class CActiveDelay : public CActiveDelayBase
{
public:
   /** Typdef for the asynchronous function. */
   typedef void (T::*TActiveCall)(class TRequestStatus*);
   /** Typdef for the cancel function. */
   typedef void (T::*TCancelCall)();
private:
   /**
    * Constructor.
    * @param aOther The active object that the delayed asynchronous
    *               function will be called upon.
    * @param aSeconds The number of seconds the call should be delayed. 
    */
   CActiveDelay(T* aOther, class TTimeIntervalSeconds aSeconds) :
       CActiveDelayBase(aSeconds), iOther(aOther)
   {}
public:
   /**
    * Static constructor.
    * @param aOther The active object that the delayed asynchronous
    *               function will be called upon.
    * @param aSeconds The number of seconds the call should be delayed. 
    */
   static CActiveDelay* NewL(T* aOther, 
                             class TTimeIntervalSeconds aSeconds)
   {
      CActiveDelay* self = new (ELeave) CActiveDelay(aOther, aSeconds);
      CleanupStack::PushL(self);
      self->ConstructL();
      CleanupStack::Pop(self);
      return self;
   }

   /**
    * Access the actual active object. 
    */
   T& operator()()
   {
      return *iOther;
   }
   /**
    * Access the actual active object. 
    */
   T const& operator()() const
   {
      return *iOther;
   }
   /**
    * Access the actual active object. 
    */
   T* Other()
   {
      return iOther;
   }
   /**
    * Access the actual active object. 
    */
   T const* Other() const
   {
      return iOther;
   }




   /**
    * Start the delay-asynchronous cycle. 
    * If the iActiveCall pointer has not been set this function will
    * panic with status code. ActiveDelay 1.
    * @param aStatus The status variable that will receive the
    *                completion event.
    * @return One of the system wide error codes. 
    */
   TInt Start(class TRequestStatus* aStatus)
   {
      return BaseStart(aStatus);
   }
   
   /**
    * Set the iActiveCall pointer and starts the delay-asynchronous
    * cycle.
    * @param aActiveCall Pointer to the member function of T that will
    *                    start the asynchronous event.
    * @param aStatus The status variable that will receive the
    *                completion event.
    * @return One of the system wide error codes. 
    */
   TInt Start(TActiveCall aActiveCall, class TRequestStatus* aStatus)
   {
      iActiveCall = aActiveCall;
      return BaseStart(aStatus);
   }

   /**
    * Set the iActiveCall pointer and starts the delay-asynchronous
    * cycle.
    * @param aActiveCall Pointer to the member function of T that will
    *                    start the asynchronous event.
    * @param aCancelCall Pointer to the member function of T that may
    *                    be used to cancel outstanding asynchronous
    *                    request.
    * @param aStatus The status variable that will receive the
    *                completion event.
    * @return One of the system wide error codes. 
    */
   TInt Start(TActiveCall aActiveCall, TCancelCall aCancelCall, 
              class TRequestStatus* aStatus)
   {
      iCancelCall = aCancelCall;
      return Start(aActiveCall, aStatus);
   }
private:
   /**
    * Cancel the outstanding asynchronous call.  Panics with status
    * code ActiveDelay 2 if no cancel function is set.
    */
   virtual void CancelRealCall()
   {
      if(iCancelCall){
         (iOther->*iCancelCall)();
      } else {
         Panic(ENoCancelCall);
      }
   }
   /**
    * Called when the delay has expired. Starts the asynchronous call.
    * Panics with status code ActiveDelay 1 if no call function is
    * set.
    */
   virtual void StartRealCall()
   {
      if(iActiveCall){
         (iOther->*iActiveCall)(&iStatus);
         SetActive();
      } else {
         Panic(ENoActiveCall);
      }
   }
public:
   /**
    * Sets the cancel function and cancels a delayed asynchronous
    * request. It doesn't matter whether the the delay has passed or
    * not.
    * @param aCancelCall Pointer to the member function of T that may
    *                    be used to cancel outstanding asynchronous
    *                    request.
    */
   void CancelDelayedCall(TCancelCall aCancel)
   {
      iCancelCall = aCancel;
      BaseCancelDelayedCall();
   }
   /**
    * Cancels a delayed asynchronous request. It doesn't matter
    * whether the the delay has passed or not.  If no cancel function
    * is set and the delay has passed the function will panic with
    * status ActiveDelay 2.
    */
   void CancelDelayedCall()
   {
      BaseCancelDelayedCall();
   }
private:
   /** Pointer to redirect active object. */
   T* iOther;
   /** Asynchronous function. */
   TActiveCall iActiveCall;
   /** Cancel function. */
   TCancelCall iCancelCall;
};

#endif
