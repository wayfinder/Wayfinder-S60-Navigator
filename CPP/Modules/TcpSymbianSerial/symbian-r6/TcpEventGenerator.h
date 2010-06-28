/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPEVENTGENERATOR_H
#define TCPEVENTGENERATOR_H
// #include <e32base.h>
// #include <e32std.h>
#include "TcpEvent.h"

namespace isab{
   class LogMaster;
   class Log;
}

class CTcpEventGenerator : public CActive
{
   ///@name Constants and member variables. 
   //@{
   ///The granularity of the eventvalue array. 
   enum { KGranularity = 4 };
   ///The target object. This object will receive all generated events
   ///in its HandleGeneratedEventL(EventType) function.
   class MTcpEventHandler& iTarget;
   ///The values of the pending events.
   RPointerArray<CTcpEvent> iEventQueue;
   ///Set on entry of RunL and cleared on exit. Used to determine
   ///whether this active object is currently running when queueing
   ///new events.
   TBool iIsRunning;
   TBool iIsReady;
   ///The is the thread that should receive the completions.
   class RThread iReceiverThread;
   ///Critical section used to protect any thread transistions.
   class RCriticalSection iCritical;
   //@}

   class isab::Log* iLog;

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
   static void ClearBool(TAny* aAny);
   static void SetBool(TAny* aAny);
   ///Sets iIsRunning to ETrue and pushes a clearing operaion on the
   ///cleanupstack. This function should always be called first in RunL.
   void PushAndSetRunningL();
   ///Pops the clearing operation from the cleanupstack with a
   ///PopAndDestroy call which also clears the iIsRunning variable.
   void PopAndClearRunning();

   void PushAndClearReadyL();
   void PopReady();
   //@}
public:
   ///@name Constructors and destructors
   //@{
   ///Static constructor. 
   ///@param aTarget the object that will receive the generated events.
   ///@return a new CTcpEventGenerator object.
   static class CTcpEventGenerator* NewL(MTcpEventHandler& aTarget);

   ///Static constructor. 
   ///@param aTarget the object that will receive the generated events.
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   ///@return a new CTcpEventGenerator object.
   static class CTcpEventGenerator* NewL(class MTcpEventHandler& aTarget, class TThreadId aThreadId, class isab::LogMaster* aLogMaster);

   ///Static constructor that leaves the object on the CleanupStack.
   ///@param aTarget the object that will receive the generated events.
   ///@return a new CTcpEventGenerator object.
   static class CTcpEventGenerator* NewLC(class MTcpEventHandler& aTarget);

   ///Static constructor that leaves the object on the CleanupStack.
   ///@param aTarget the object that will receive the generated events.
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   ///@return a new CTcpEventGenerator object.
   static class CTcpEventGenerator* NewLC(class MTcpEventHandler& aTarget, class TThreadId aThreadId);

   ///Virtual destructor. Cancels the active object. 
   virtual ~CTcpEventGenerator();
protected:
   ///Constructor. Private since users are supposed to use the static
   ///NewL and NewLC functions to construct new objects of this class.
   ///@aTarget the object that will receive the generated events.
   CTcpEventGenerator(class MTcpEventHandler& aTarget);
private:
   ///Contains code common to all ConstructL functions. Should be
   ///called first in any ConstructL function.
   void ConstructCommonL();
protected:
   ///Second phase constructor. Protected since users are supposed to
   ///use the static NewL and NewLC functions to construct new objects
   ///of this class. Adds this object to the active
   ///scheduler. Generated events will be completed in the thread that
   ///this function is called from.
   void ConstructL();
   ///Second phase constructor. Private since users are supposed to
   ///use the static NewL and NewLC functions to construct new objects
   ///of this class. Adds this object to the active scheduler.
   ///Events will be generated in the indicated thread. 
   ///@param aThreadId the id of the thread where the RunL function of 
   ///       this object will be run. 
   void ConstructL(class TThreadId aThreadId);
   //@}

private:
   ///@name Active object related functions
   //@{
   ///Completes the first outstanding request. This function is
   ///synchronized on an RCriticalSection. Completion is only
   ///performed if this object is not active and not running.
   ///@param aStatus the status the event should complete with. 
   ///               Should just be KErrNone or KErrCancel.
   void Complete(TInt aStatus);
protected:
   ///@name Derived from CActive
   //@{
   ///Called by the framework when Cancel is called on this object.
   virtual void DoCancel();
   ///Called by the framework whenever the outstanding request
   ///completes.  Calls HandleGeneratedEventL on the target object,
   ///and then completes the first of any queued events.
   virtual void RunL();
   virtual TInt RunError(TInt aError);

   //@}
   //@}
public:
   void SetReady(TBool aReady = ETrue);
   ///Enqueue an event. The event is added to the end of the event
   ///queue. If the queue is empty, the event will be completed
   ///immediately. This function may leave with any of the leave codes
   ///of CArrayFixSeg<T>::AppendL.
   ///@param aEventValue the event value that will be reported to the 
   ///                   target object.
   ///@return the number of queued events, including the argument. 
   TInt SendEventL(const class CTcpEvent* aEvent);
   ///Enqueue an event. The event is added to the end of the event
   ///queue. If the queue is empty, the event will be completed
   ///immediately.
   ///@param aEventValue the event value that will be reported to the 
   ///                   target object.
   ///@return KErrNone if all went well, otherwise the leave code of
   ///        CArrayFixSeg<T>::AppendL.
   TInt SendEvent(class CTcpEvent* aEvent);
};

#endif
