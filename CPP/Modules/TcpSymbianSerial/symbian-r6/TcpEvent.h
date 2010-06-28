/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TCPEVENT_H
#define TCPEVENT_H
#include <e32base.h>
#include "Module.h"

/**
 * Intrface class that handles CTcpEvent-derived classes.
 */
class MTcpEventHandler
{
public:
   /**
    * Handling function for CTcpConnectionEvents. 
    * @param aEvent The object to handle. 
    * @return One of the system wide error codes. 
    */
   virtual TInt HandleConnectionEventL(class CTcpConnectionEvent& aEvent) = 0;
   /**
    * Handling function for CTcpDataEvents. 
    * @param aEvent The object to handle. 
    * @return One of the system wide error codes. 
    */
   virtual TInt HandleDataEventL(class CTcpDataEvent& aEvent) = 0;	
   virtual TInt HandleRunError(TInt aError) = 0;
};

/**
 * Classes derived from this class will represent the different kind
 * of events that CTcpAdmin will receive.
 */
class CTcpEvent : public CBase 
{
public:
   /**
    * Pure virtual function that will call the correct function of
    * MTcpEventHandler.
    * @param aHandler The MTcpEventHandler object that will hendle
    *                 this event object.
    * @return One of the system wide error codes. 
    */
   virtual TInt ProcessL(class MTcpEventHandler& aHandler) = 0;
protected:
   /** Default constructor. */
   CTcpEvent();
private:
   //noncopyable object
   CTcpEvent(const CTcpEvent&);
   //noncopyable object
   const CTcpEvent& operator=(const CTcpEvent&);
};

/**
 * This class represents data sending events. Each of these events
 * will trigger a TCP write operation.
 */
class CTcpDataEvent : public CTcpEvent 
{
public:
   /**
    * Static constructor. 
    * @param aData The data associated with this event. 
    * @return A new CTcpDataEvent object. 
    */
   static class CTcpDataEvent* NewL(const TDesC8& aData);
   /**
    * Static constructor. 
    * @param aData The data associated with this event. 
    * @param aLength The length of the data.
    * @return A new CTcpDataEvent object. 
    */
   static class CTcpDataEvent* NewL(const TUint8* aData, TInt aLength);
   /** @name From CTcpEvent. */
   //@{
   /**
    * Will call MTcpEventHandler::HandleDataEventL. 
    * @param aHandler The object handler.
    * @return One of the system wide error codes. 
    */
   virtual TInt ProcessL(class MTcpEventHandler& aHandler);
   //@}
   /**
    * Retrieves the events data.
    * @return The data.
    */
   const TDesC8& Data() const;
   /**
    * Takes the data away from the event. Once this function has been
    * called the event no longer has any reference to it's data.
    * @return The data as a HBufC8*.
    */
   HBufC8* ReleaseData();
private:
   /** The data. */
   class HBufC8* iData;
};

/**
 * This class represents connection control events. 
 */
class CTcpConnectionEvent : public CTcpEvent {
public:
   /**
    * Static constructor. 
    * @param aControl The connection action that is to be taken.
    * @param aMethod  Extra information associated with the action. 
    * @return A new CTcpConnectionEvent object. 
    */
   static class CTcpConnectionEvent* NewL(enum isab::Module::ConnectionCtrl aControl, const char* aMethod);
   static class CTcpConnectionEvent* NewLC(enum isab::Module::ConnectionCtrl aControl, const char* aMethod);
   /**
    * Static constructor. 
    * @param aControl The connection action that is to be taken.
    * @param aMethod  Extra information associated with the action. 
    * @return A new CTcpConnectionEvent object. 
    */
   static class CTcpConnectionEvent* NewL(enum isab::Module::ConnectionCtrl aControl, const TText8* aMethod);
   /**
    * Constructor. 
    * @param aControl The connection action that is to be taken.
    * @param aMethod  Extra information associated with the action. 
    * @return A new CTcpConnectionEvent object. 
    */
   CTcpConnectionEvent(enum isab::Module::ConnectionCtrl aControl, const TDesC8& aMethod);
   /** @name From CTcpEvent. */
   //@{
   /**
    * Will call MTcpEventHandler::HandleConnectionEventL.
    * @param aHandler The object handler. 
    * @return One of the system wide error codes. 
    */
   virtual TInt ProcessL(class MTcpEventHandler& aHandler);
   //@}
   /**
    * Retreive the action.
    * @return The connection action of this event. 
    */
   enum isab::Module::ConnectionCtrl Action() const;
   /**
    * Retreive the method.
    * @return The method of this event. 
    */
   const TDesC8* Method() const;   
private:
   /** The action. */
   enum isab::Module::ConnectionCtrl iControl;
   /** The method. */
   TBuf8<256> iMethod;
};

#endif
