/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BTGPSCONNECTOR_H
#define BTGPSCONNECTOR_H

#include <e32base.h>
#include "PositionInterface.h"
#include "SprocketTalker.h"
#include "Nav2.h"
#include "BluetoothEngine.h"
#include "BtEngineObserver.h"
namespace isab{
   class Log;
   class LogMaster;
}

/** 
 * This interface handles all feedback from the CBtGpsConnector class
 * to the user class. It does NOT handle the actual data.
 */
class MGpsConnectionObserver {
public:
   /** 
    * Sends a debug message as text to the user. This function has a
    * default impelementation, which is do nothing.
    * @param aDbgMsg the debug message. 
    */
   virtual void BluetoothDebug(const TDesC& aDbgMsg);
   /**
    * Sends an error message as text to the user.
    * @param aErrorMsg the error message text.
    */
   virtual void BluetoothError(const TDesC& aErrorMsg) = 0;
   /**
    * Sends an error message as an int to the user.
    * TODO: It is very unclear what error messages are available. Some
    * formal mechanism for the user and CBtGpsConnector to agree on
    * error messages is needed.
    * @param aErrorMsg the int.
    */
   virtual void BluetoothError(TInt aErrorMsg) = 0;
   /**
    * This silly function is called periodically to inform the user if
    * Bluetooth is working or not.
    * TODO: Remove this function. 
    * @param aOk ETrue if CBtGpsConnector is connected. 
    */
   virtual void BluetoothStatus(TBool aOk) = 0;

   /**
    * This function will be called once whenever the Bluetooth
    * connection is unexpectedly lost. The implication is that the UI
    * may trigger an alert.
    */
   virtual void BluetoothConnectionLost(TBool aWillTryToReconnect) = 0;

   /** 
    * These two functions are both called when a GPS has been
    * selected. The only difference is the format of the Bluetooth
    * address. The user class probably only needs to override one of
    * the functions since both have a default do-nothing
    * implementation. 
    */
   //@{
   /**
    * This function is used to tell the user which Bluetooth device
    * has been chosen. This function has a default implementation
    * which is to do nothing.
    * @param aAddr the Bluetooth address of the chose device. 
    * @param aName the name of the chosen device. 
    */
   virtual void BluetoothDeviceChosen(TInt64 aAddr, const TDesC& aName);
   /**
    * This function is used to tell the user which Bluetooth device
    * has been chosen. This function has a default implementation
    * which is to do nothing.
    * @param aAddr the Bluetooth address of the chose device. 
    * @param aName the name of the chosen device. 
    */
   virtual void BluetoothDeviceChosen(const class TBTDevAddr& aAddr, 
                                      const TDesC& aName);
   //@}
};

/**
 * This is one of the classes that can be used to collect data from a
 * CBtGpsConnector object. It also implements the
 * MGpsConnectionObserver interface, and CBtGpsConnector uses the same
 * object for both status and data when a MGpsObserver object is used.
 */
class MGpsObserver : virtual public MGpsConnectionObserver {
public:
   /**
    * Receives serial data from the connected Bluetooth unit. 
    * @param aData the serial data.
    * @return not used. 
    */
   virtual TBool Write(const TDesC8& aData) = 0;
};

/** Maps errors to strings. */
struct TBtErrorToResource {
   /** The error code. */
   /*enum TBluetoothError*/TInt iError;
   /** The resource id. */
   TInt iResourceString;
};


/**
 * BT handling class.  
 * Should enacapsulate everything a Wayfinder application need to do
 * with bluetooth, inluding connecting, disconnecting, reading,
 * writing searching for known or new devices, ...
 */
class CBtGpsConnector : public CBase, 
                        public MPositionInterface, //interface toward user
                        public MSprocketTalker,    //data from sprocket
                        public MBluetoothEngineObserver //new btengine
{
private:
   /** @name Constructors and destructor */
   //@{
   /**
    * Constructor.
    * @param aObserver the MgpsConnectionObserver.
    * @param aChannel  the Nav2 channel. 
    */
   CBtGpsConnector(class MGpsConnectionObserver* aObserver);

   /** 
    * Second phase constructor.
    * @param aChannel the way to send data to and receive data from Nav2. 
    */
   void ConstructL(class isab::Nav2::Channel* aChannel);
   /**
    * Second phase constructor. Note that there is no way to send data
    * to the Bluetooth unit if you use this constructor.
    * @param aObserver the way to send data to something else than Nav2. 
    */
   void ConstructL(class MGpsObserver* aObserver);
public: 
   /**
    * Static constructor. 
    * @param aObserver the MGpsConnectionObserver.
    * @param aChannel  the Nav2 channel.
    * @return a new CBtGpsConnector object that is pushed to the
    *         cleanupstack.
    */
   static class CBtGpsConnector* NewLC(class MGpsConnectionObserver* aObserver,
                                       class isab::Nav2::Channel* aChannel);
   /** 
    * Static constructor.
    * @param aObserver the MGpsConnectionObserver and MGpsObserver
    *                  rolled into one.
    * @return a new CBtGpsConnector object that is pushed to the
    *         cleanupstack.
    */
   static class CBtGpsConnector* NewLC(class MGpsObserver* aObserver);

   /**
    * Static constructor. 
    * @param aObserver the MGpsConnectionObserver.
    * @param aChannel  the Nav2 channel. 
    * @return a new CBtGpsConnector object. 
    */
   static class CBtGpsConnector* NewL(class MGpsConnectionObserver* aObserver,
                                      class isab::Nav2::Channel* nav2 );
   /**
    * Static constructor. 
    * @param aObserver the MGpsConnectionObserver.
    * @param aMapping  mapping of errors to resource strings to 
    *                  display to the user. 
    * @param  aMappingSize number of mappings in aMapping.
    * @param aChannel  the Nav2 channel. 
    * @return a new CBtGpsConnector object. 
    */
   static class CBtGpsConnector* NewL(class MGpsConnectionObserver* aObserver,
                                      struct TBtErrorToResource* aMapping,
                                      TInt aMappingSize,
                                      class isab::Nav2::Channel* aChannel );

   /**
    * Static constructor. 
    * @param aObserver     the MGpsConnectionObserver and MGpsObserver 
    *                      rolled into one.
    * @param aMapping      mapping of errors to resource strings to 
    *                      display to the user. 
    * @param  aMappingSize number of mappings in aMapping.
    * @return a new CBtGpsConnector object. 
    */
   static class CBtGpsConnector* NewL(class MGpsObserver* aObserver,
                                      struct TBtErrorToResource* aMapping,
                                      TInt aMappingSize);

   static class CBtGpsConnector* NewL(class MGpsConnectionObserver* aObserver,
                                      TInt aMappingResource,
                                      class isab::Nav2::Channel* aChannel);

   static class CBtGpsConnector* NewL(class MGpsObserver* aObserver,
                                      TInt aMappingResource);


   /**Virtual destructor. */
   virtual ~CBtGpsConnector();
   //@}
public:

   /** 
    * Replace the current error message mapping with a new one. 
    * @param aMapping the array of new mappings. 
    * @param aNum     the length of aMapping.
    */
   void SetErrorMappingL(const struct TBtErrorToResource* aMapping, TInt aNum);
   void SetErrorMappingL(TInt aMappingResource);
   /**@name From MPositionInterface.*/
   //@{
   virtual void ConnectL();
   virtual void DisconnectL();
   virtual TBool IsConnected() const;
   virtual TBool IsBusy() const;
   /**
    * <ul>
    * <li>aNum shall be 3.</li>
    * <li>aData[0] should contain the high bits of the bt address as a
    *     hex number starting with 0x.</li>
    * <li>aData[1] shall contain the low bits in the same way.</li>
    * </ul>
    */
   virtual TBool SetAddress(const char **aData, TInt aNum);
   virtual TBool SetAddress(const class MDesC8Array& aData);
   virtual void ReConnect();
   //@}
private:
   /**
    * Convenience function used by the SetAddress functions. 
    * Sets the address in iBtEngine and might call connect. 
    * @param aAddress the address to use. 
    */
   void SetAddressAndConnectL(const class TBTDevAddr& aAddress);

   /** Release the Sprocket and disconnect the GPS.*/
   void ReleaseL(); //???


   /**
    * Forward btaddress and bt device name to the
    * BluetoothDeviceChosen functions of MGpsConnectionObserver.
    * @param aAddress the address.
    * @param aName    the remote device's name.
    */
   void ForwardAddress(const class TBTDevAddr& aAddress, const TDesC& aName);

   
   /** @name From MBluetoothEngineObserver. */
   //@{
   /**
    * Called whenever something happens that CBluetoothEngine thinks
    * its observer might be interested in.
    * @param aEvent the event.
    */
   virtual void BtEngineInfo(enum TBtEngineEvent aEvent);
   /**
    * During repeated searches for a new device, this function is
    * called each time a new search is started.
    * @param aNum   the order number of the current search. 
    * @param aTotal the total number of searches in this sequence.
    */
   virtual void BtEngineSearching(TInt aNum, TInt aTotal);
   /**
    * Each time a connection is completed the CBluetoothEngine will
    * call this function with the address and name of the connected
    * device.
    * @param aAddr the address of the connected device.
    * @param aName the name of the connected device. 
    */
   virtual void BtEngineDevice(const class TBTDevAddr& aAddr, 
                               const TDesC& aName);
   /**
    * Data from the remote device is passed to the observer through
    * this function.
    * @param aData the data.
    */
   virtual void BtEngineData(const TDesC8& aData);

   /**
    * This function will be called once whenever the Bluetooth
    * connection is unexpectedly lost. The implication is that the UI
    * may trigger an alert.
    */
   virtual void BtConnectionLost(TBool aWillTryToReconnect);
   //@}   

public:
   /** 
    * Called by CBluetoothEngine when a disconnect order has
    * completed. This is necessary when the disconnect operation is a
    * asynchronous operation.
    */
   void DisconnectComplete();

private:
   /**@name  From MSprocketTalker */
   //@{
   /**
    * Receive data from the Sprocket. The data should be passed on to
    * the remote unit.
    * @param aData the data.
    */
   virtual void ReceiveMessageL(const TDesC8& aData);
   //@}
public:
   /**
    * Sets a new LogMaster. 
    * @param aNewMaster the new LogMaster.
    */
   void SetLogMaster(class isab::LogMaster* aNewMaster);
private:
   /**@name Member variables. */
   //@{

   /**
    * Pointer to the object that receives status information about
    * this CBtGpsConnector.
    */
   class MGpsConnectionObserver* iObserver;
   /** Handles bluetooth connection. */
   class CBluetoothEngine* iBtEngine;


   /** 
    * @name Connection to data consumer.
    * Only one of the member variables iSprocket and iReceptor should
    * be set.
    */
   //@{
   /** Handles Nav2 connection. */
   class CSprocketsEngine* iSprocket;
   /**
    * As an alternative, we could use a MGpsConnector instead of a
    * CSprocketsEngine.
    */
   class MGpsObserver* iReceptor;
   //@}

   /** Keeps the error-to-error message mapping. */
   CArrayFix<TBtErrorToResource>* iMapping;
   /**
    * Keeps track of if we are disconnecting just because we want to
    * select a new GPS.
    */
   TBool iReconnecting;
   
   /** The log object. */
   class isab::Log* iLog;
   //@}
#if defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1
   class CCknFloatingProgressBar* iBar;
#endif
};

#endif


