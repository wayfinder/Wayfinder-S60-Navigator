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

#ifndef EVENTGENERATOR_H
#define EVENTGENERATOR_H

#include <e32base.h>

//Forward declaration of template class
template<class Target, class EventType>
class CDelayedEvent;

///This template class is used to create user driven events in a
///Symbian program. It's templated on the receiving class and on the
///type of event.
///
///Example:
///<code>
///struct TMyEvent { TInt index; TReal quota; };
///class EventReceiver{
///  CEventGenerator<EventReceiver>* iIntEventGenerator;
///  CEventGenerator<EventReceiver,TMyEvent>* iMyEventGenerator;
/// public:
///  void HandleGeneratedEventL(TInt aEvent);
///  void HandleGeneratedEventL(const TMyEvent& aEvent);
///  EventReceiver()
///  {
///    iIntEventGenerator = CEventGenerator<EventReceiver>::NewL(*this);
///    iMyEventGenerator =CEventGenerator<EventReceiver,TMyEvent>::NewL(*this);
///  }
///  void SendEventsL()
///  {
///    iIntGenerator->SendEventL(1);
///    iIntGenerator->SendEventL(KErrNone);
///    TMyEvent event = { 3, 7.5};
///    iMyEventGenerator->SendEventL(event);
///  }
///};
///</code>
///The example shows a couple of possible uses for CEventGenerator. 
///<ul>
///<li> Each class that defines a function HandleGeneratedEventL(<one arg>) 
///     can be used as an event recipient.
///<li> A class can receive any number of event types by overloading the
///     HandleGeneratedEventL function with different type arguments.
///     By using clever overloading it's possible to handle different 
///     types of events in the same HandleGeneratedEventL function, but 
///     this is of questionable merit. 
///<li> Any type with copy semantics (POD-type or a class with copy 
///     constructor and assignment operator) can be used as event type. 
///     Even pointers can be used, but it's important to keep track of 
///     ownership since the CEventGenerator doesn't delete any external 
///     objects upon destruction. 
///<li> The CEventGenerator object queues event requests. That makes it 
///     possible to add several requests without waiting for request 
///     completion.
///<ul>
///
///Template parameters:
///Target is the event receiving class.
///EventType is the type of event sent to the receiver. Defaults to TInt. 
///
///<h2>Possible extensions<h2>
///By deriving from this class and override the virtual function
///<code>TInt RunError(TInt aError)</code> you can add a handler for
///any Leaves occuring in your HandleGeneratedEventL.
template<typename Target, typename EventType = TInt>
class CEventGenerator : public CActive
{
   ///@name Constants and member variables. 
   //@{
   ///The granularity of the eventvalue array. 
   enum { KGranularity = 4 };
   ///The target object. This object will receive all generated events
   ///in its HandleGeneratedEventL(EventType) function.
   Target& iTarget;
   ///The values of the pending events.
   CArrayFixSeg<EventType>* iEventQueue;
   /** Delayed events are stored here. */
   RPointerArray<CDelayedEvent<Target, EventType> > iDelayedEvents;
   ///Set on entry of RunL and cleared on exit. Used to determine
   ///whether this active object is currently running when queueing
   ///new events.
   TBool iIsRunning;
   ///The is the thread that should receive the completions.
   class RThread iReceiverThread;
   ///Critical section used to protect any thread transistions.
   class RCriticalSection iCritical;
   //@}

   ///@name Functions handling the iIsRunning variable.
   ///These functions deals with the leave safe hanling of the
   ///iIsRunning variable used to make sure that no events are
   ///completed while we are still handling the last completed one.
   //@{
   
   ///This function matches the TCleanupOperation typedef and is used
   ///to make sure that the iIsRunning member variable is cleared if
   ///RunL leaves.
   ///@param aAny a TAny* that is cast to a TBool*. The TBool is then
   ///            cleared.
   static void ClearRunning(TAny* aAny)
   {
      TBool* isRunning = static_cast<TBool*>(aAny);
      *isRunning = EFalse;
   }

   ///Sets iIsRunning to ETrue and pushes a clearing operaion on the
   ///cleanupstack. This function should always be called first in RunL.
   void PushAndSetRunningL()
   {
      iIsRunning = ETrue;
      CleanupStack::PushL(TCleanupItem(ClearRunning, &iIsRunning));
   }
   ///Pops the clearing operation from the cleanupstack with a
   ///PopAndDestroy call which also clears the iIsRunning variable.
   void PopAndClearRunning()
   {
      CleanupStack::PopAndDestroy();
   }
   //@}
public:
   ///@name Constructors and destructors
   //@{
   ///Static constructor. 
   ///@param aTarget the object that will receive the generated events.
   ///@return a new CEventGenerator object.
   static CEventGenerator* NewL(Target& aTarget)
   {
      CEventGenerator* self = CEventGenerator::NewLC(aTarget);
      CleanupStack::Pop(self);
      return self;
   }

   ///Static constructor. 
   ///@param aTarget the object that will receive the generated events.
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   ///@return a new CEventGenerator object.
   static CEventGenerator* NewL(Target& aTarget, class TThreadId aThreadId)
   {
      CEventGenerator* self = CEventGenerator::NewLC(aTarget, aThreadId);
      CleanupStack::Pop(self);
      return self;
   }


   ///Static constructor that leaves the object on the CleanupStack.
   ///@param aTarget the object that will receive the generated events.
   ///@return a new CEventGenerator object.
   static CEventGenerator* NewLC(Target& aTarget)
   {
      CEventGenerator* self = new CEventGenerator(aTarget);
      CleanupStack::PushL(self);
      self->ConstructL();
      return self;
   }

   ///Static constructor that leaves the object on the CleanupStack.
   ///@param aTarget the object that will receive the generated events.
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   ///@return a new CEventGenerator object.
   static CEventGenerator* NewLC(Target& aTarget, class TThreadId aThreadId)
   {
      CEventGenerator* self = new CEventGenerator(aTarget);
      CleanupStack::PushL(self);
      self->ConstructL(aThreadId);
      return self;
   }

   ///Virtual destructor. Cancels the active object. 
   ~CEventGenerator()
   {
      iDelayedEvents.ResetAndDestroy();
      iDelayedEvents.Close();
      Cancel();
      iCritical.Close();
      delete iEventQueue;
   }

protected:
   ///Constructor. Private since users are supposed to use the static
   ///NewL and NewLC functions to construct new objects of this class.
   ///@aTarget the object that will receive the generated events.
   CEventGenerator(Target& aTarget) :
      CActive(EPriorityStandard), iTarget(aTarget), 
      iDelayedEvents(KGranularity)
   {
   }

private:
   ///Contains code common to all ConstructL functions. Should be
   ///called first in any ConstructL function.
   void ConstructCommonL()
   {
      iEventQueue = new (ELeave) CArrayFixSeg<EventType>(KGranularity);
      User::LeaveIfError(iCritical.CreateLocal());
      CActiveScheduler::Add(this);      
   }

protected:
   ///Second phase constructor. Protected since users are supposed to
   ///use the static NewL and NewLC functions to construct new objects
   ///of this class. Adds this object to the active
   ///scheduler. Generated events will be completed in the thread that
   ///this function is called from.
   void ConstructL()
   {
      ConstructCommonL();
      iReceiverThread.Duplicate(RThread());
   }

   ///Second phase constructor. Private since users are supposed to
   ///use the static NewL and NewLC functions to construct new objects
   ///of this class. Adds this object to the active scheduler.
   ///Events will be generated in the indicated thread. 
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   void ConstructL(class TThreadId aThreadId)
   {
      ConstructCommonL();
      iReceiverThread.Open(aThreadId);
   }
   //@}

private:
   ///@name Active object related functions
   //@{
   ///Completes the first outstanding request. This function is
   ///synchronized on an RCriticalSection. Completion is only
   ///performed if this object is not active and not running.
   ///@param aStatus the status the event should complete with. 
   ///               Should just be KErrNone or KErrCancel.
   void Complete(TInt aStatus)
   {
      iCritical.Wait();
      if(iEventQueue->Count() > 0 && !IsActive()){
         iStatus = KRequestPending;
         SetActive();
         class TRequestStatus *tmpPtr = &iStatus;
         iReceiverThread.RequestComplete(tmpPtr, aStatus);
      }
      iCritical.Signal();
   }

protected:
   ///@name Derived from CActive
   //@{
   ///Called by the framework when Cancel is called on this object.
   virtual void DoCancel()
   {
   }
   ///Called by the framework whenever the outstanding request
   ///completes.  Calls HandleGeneratedEventL on the target object,
   ///and then completes the first of any queued events.
   virtual void RunL()
   {
      PushAndSetRunningL();
      if(iStatus != KErrCancel){
         iCritical.Wait();
         EventType event = iEventQueue->At(0);
         iEventQueue->Delete(0);
         if(iEventQueue->Count() >= KGranularity){
            iEventQueue->Compress();
         }
         iCritical.Signal();
         iTarget.HandleGeneratedEventL(event);
      }
      Complete(KErrNone);
      PopAndClearRunning();
   }
   //@}
   //@}
public:
   ///Enqueue an event. The event is added to the end of the event
   ///queue. If the queue is empty, the event will be completed
   ///immediately. This function may leave with any of the leave codes
   ///of CArrayFixSeg<T>::AppendL.
   ///@param aEventValue the event value that will be reported to the 
   ///                   target object.
   ///@return the number of queued events, including the argument. 
   TInt SendEventL(EventType const& aEvent){
      iCritical.Wait();
      iEventQueue->AppendL(aEvent);
      TInt count = iEventQueue->Count();
      iCritical.Signal();
      if(!iIsRunning){
         Complete(KErrNone);
      }
      return count;
   }

   /**
    * Enqueue a delayed event. The event will be added to the end of
    * the event queue once the specified time has passed. If the delay
    * is zero the event will be added immedeately using the
    * <code>SendEventL(EventType const&)</code> function. This
    * function may leave with any of the leave codes of <code>TInt
    * SendEventL(EventType const& aEvent)</code> or
    * <code>RpointerArray::InsertInAddressOrder</code>
    * @param aEvent the event value that will be reported to the 
    *               target object.
    * @param aDelay The delay in seconds.
    * @return The number of the events in the queue
    *         (delayed/nondelayed) that the new event was inserted into.
    */
   TInt SendEventL(EventType const& aEvent, class TTimeIntervalSeconds aDelay)
   {
      if(aDelay.Int() > 0){
         CDelayedEvent<Target, EventType>* delayedEvent = 
            CDelayedEvent<Target, EventType>::NewLC(*this, aEvent, aDelay);
         User::LeaveIfError(iDelayedEvents.InsertInAddressOrder(delayedEvent));
         CleanupStack::Pop(delayedEvent);
      } else {
         return SendEventL(aEvent);
      }
      return iDelayedEvents.Count();
   }

   TInt SendEventL(EventType const& aEvent, class TTimeIntervalMicroSeconds32 aDelay)
   {
      if(aDelay.Int() > 0){
         CDelayedEvent<Target, EventType>* delayedEvent = 
            CDelayedEvent<Target, EventType>::NewLC(*this, aEvent, aDelay);
         User::LeaveIfError(iDelayedEvents.InsertInAddressOrder(delayedEvent));
         CleanupStack::Pop(delayedEvent);
      } else {
         return SendEventL(aEvent);
      }
      return iDelayedEvents.Count();
   }

   ///Enqueue an event. The event is added to the end of the event
   ///queue. If the queue is empty, the event will be completed
   ///immediately.
   ///@param aEventValue the event value that will be reported to the 
   ///                   target object.
   ///@return KErrNone if all went well, otherwise the leave code of
   ///        CArrayFixSeg<T>::AppendL.
   TInt SendEvent(EventType const& aEvent)
   {
      TRAPD(ret, SendEventL(aEvent));
      return ret;
   }

   /**
    * 
    */
   void DeleteDelay(CDelayedEvent<Target,EventType>* aDelayedEvent)
   {
      TInt pos = iDelayedEvents.FindInAddressOrder(aDelayedEvent);
      __ASSERT_ALWAYS(pos != KErrNotFound, 
                      User::Panic(_L("EventGenerator"), 0));
      iDelayedEvents.Remove(pos);
      delete aDelayedEvent;
   }
};

template<typename Target, class EventType>
class CDelayedEvent : public CTimer
{
public:
   typedef CEventGenerator<Target, EventType> CReceiver;
private:
   CDelayedEvent(CReceiver& aRecipient, EventType const& aEvent, 
                 class TTimeIntervalSeconds aDelay) : 
      CTimer(EPriorityStandard), iRecipient(aRecipient), 
      iEvent(aEvent), iDelay(aDelay.Int() * 1000 * 1000)
   {
   }

   CDelayedEvent(CReceiver& aRecipient, EventType const& aEvent, 
                 class TTimeIntervalMicroSeconds32 aDelay) : 
      CTimer(EPriorityStandard), iRecipient(aRecipient), 
      iEvent(aEvent), iDelay(aDelay.Int())
   {
   }

   void ConstructL()
   {
      CTimer::ConstructL();
      CActiveScheduler::Add(this);      
      After(iDelay);
   }
public:
   static CDelayedEvent* NewLC(CReceiver& aRecipient, 
                               EventType const& aEvent, 
                               class TTimeIntervalSeconds aDelay)
   {
      CDelayedEvent* self = 
         new (ELeave) CDelayedEvent(aRecipient, aEvent, aDelay);
      CleanupStack::PushL(self);
      self->ConstructL();
      return self;
   }

   static CDelayedEvent* NewLC(CReceiver& aRecipient, 
                               EventType const& aEvent, 
                               class TTimeIntervalMicroSeconds32 aDelay)
   {
      CDelayedEvent* self = 
         new (ELeave) CDelayedEvent(aRecipient, aEvent, aDelay);
      CleanupStack::PushL(self);
      self->ConstructL();
      return self;
   }
private:
   virtual TInt RunError(TInt)
   {
      iRecipient.DeleteDelay(this);  //will delete this
      return KErrNone;
   }

   virtual void RunL()
   {
      iRecipient.SendEventL(iEvent);
      //      iRecipient.DeleteDelay(this); //will delete this
   }

   CReceiver& iRecipient;
   EventType iEvent;
   class TTimeIntervalMicroSeconds32 iDelay;
};




#endif
