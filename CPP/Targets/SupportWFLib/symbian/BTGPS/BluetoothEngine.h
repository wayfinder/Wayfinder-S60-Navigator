/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NEWBLUETOOTHENGINE_H
#define NEWBLUETOOTHENGINE_H

#include "ActiveLog.h"
#include <bttypes.h> //TBTDevAddr
#include "BtSocketsPeer.h"
#include "SdpRepeaterObserver.h"
#include "SdpRepeater.h"
#include "ActiveDelay.h"

//This forward declartion makes it possible to use the CActiveDelay
//templat class with this type
class CBtSelector;

/** 
 * Connects to and communicates with a remote deviceusing bluetooth.
 */
class CBluetoothEngine : public CActiveLog,// public MTimeOutNotify,
                         public MBtSocketsPeer,
                         public MSdpRepeaterObserver 
{
   /**@name Constuctors and destuctor. */
   //@{
   /**
    * Constructor. 
    * @param aReceiver Receiver of data as well as status and error messages. 
    */
   CBluetoothEngine(class MBluetoothEngineObserver& aObserver);

   /** Second phase constructor. */
   void ConstructL();
public:
   /**
    * Static constructor. 
    * @param aReceiver the receiver of output and error and status messages. 
    * @return a pointer to the created instance of CBluetoothEngine.
    */
   static class CBluetoothEngine* 
   NewL(class MBluetoothEngineObserver& aObserver);

   /**
    * Static constructor that leaves the new object on the cleanupstack. 
    * @param aReceiver the receiver of output and error and status messages. 
    * @return a pointer to the created instance of CBluetoothEngine.
    */
   static class CBluetoothEngine* 
   NewLC(class MBluetoothEngineObserver& aObserver);

   /**
    * Virtual destructor. 
    * Closes any open sockets or server sessions. 
    */
   virtual ~CBluetoothEngine();
   //@}
public:

//    enum {
//       /** The maximum length of any message that can be read. */
//       KMaximumMessageLength = 1024 
//    };

   /**
    * Tests whether a Bluetooth session is active.
    * @result ETrue if the client is connected.
    */
   TBool IsConnected();

   /**
    * Tests whether the object is busy with some operation. 
    * @result ETrue if the client is performing some operation.
    */
   TBool IsBusy();

   /** @name Get and set remote device address and name. */
   //@{
   /**
    * Fetches the name of any connected remote device.
    * @param aName will contain the name of the remote device. 
    */
   void GetDeviceName(TDes& name) const;
   /**
    * Fetches the Bluetooth address of any connected remote device. 
    * @param aAddress will constain the address. Must be at least 6
    *                 characters long.
    */
   void GetDeviceAddr(TDes8& addr) const;
   const class TBTDevAddr GetDeviceAddress() const;
   /**
    * Set the address of the preferred remote device.  No new
    * connection is started, and existsing connections are not
    * affected.
    * @param aAddr the address.
    */
   void SetDeviceAddr(const class TBTDevAddr& aAddr);
   //@}

   /**
    * Initializes the connection to a remote device.  If a Bluetooth
    * address have been set, that address will be used.  Otherwise a
    * search for a device will be started.
    * @return KErrNone if the search was started, KErrInUse if the
    *         IsBusy function returns ETrue.
    */
   TInt ConnectL(const class TSdpRepeatSettings& aRepeats = TSdpRepeatSettings());
   /**
    * Disconnect from the remote device. 
    */
   void DisconnectL();

public:
   /**
    * Send data to the remote device. 
    * If the connection is not currently in the data transfer stage,
    * an error will be generated and everything will go
    * Kahoona-shaped.
    * @param aData the data to send.
    */
   TInt SendDataL(const TDesC8& aData);

private:
   /** @name  from CActive */
   //@{
   /** Cancel any outstanding requests. */
   void DoCancel();

   /** Respond to an event */
   void RunL();

   /** Called whenever RunL leaves.  */
   TInt RunError( TInt aError );
   //@}

private:
   /** @name From MBtSocketsPeer. */
   //@{
   virtual void ConnectionCancelled();
   virtual void ConnectionLost(TBool aReconnect = ETrue);
   virtual void ReceiveMessageL(const class TDesC8& aData);
   virtual void ErrorNotify(enum TBtSocketsErrors aError, TInt aStatus);
   //@}

   /** @name From MSdpRepeaterObserver. */
   //@{
   virtual void SdpRepeatInfo(TInt aNum, TInt aTotal);
   //@}

   /** Creates and starts a CBtSelector. Calls SetActive. */
   void StartBtSelectorL();
   /**
    * Creates and starts a CSdpExaminer to search for a specific
    * device. Calls SetActive.
    * @param aAddress the address of the device we want.
    */
   void StartSdpExaminerL(const class TBTDevAddr& aAddress);
   /**
    * Creates a CBtSocketsEngine and starts a connection through it.
    * Calls SetActive.
    * @param aAddr the BT address of the wanted device.
    * @param aPort the RFCOMM port to use in the connection.
    */
   void StartSocketsEngineL(const class TBTDevAddr& aAddr, TInt aPort);
   /**
    * Creates and starts a CSdpRepeater. The repeat settings are set
    * from iRepeatSettings. Calls SetActive.
    */
   void StartSdpRepeaterL();


   void LogComponents();

private:
   /** The Bluetooth address of the preferred remote device. */
   class TBTDevAddr iDeviceAddr;
   /**
    * The Bluetooth address of a remote device that the user has
    * selected but that we have not yet managed to connect to.
    */
   class TBTDevAddr iMaybeDeviceAddr;
   /**
    * The Bluetooth name of a remote device that the user has selected
    * but that we have not yet managed to connect to.
    */
   HBufC* iMaybeDeviceName;
   /**
    * The Bluetooth name of a remote device that the user has selected
    * that we have managed to connect to.
    */
   HBufC* iDeviceName;
   /** the Reciver to send output to */
   //   class CBtGpsConnector& iReciver;
   class MBluetoothEngineObserver& iObserver;

   /**@name The objects that do the work in the different steps.*/
   //@{
   /** The class that searches for new devices. */
   CActiveDelay<CBtSelector>* iDelayedSelector;
   /**
    * The class that searches for a serial port on a remote device.
    */
   class CSdpExaminer* iSdpExaminer;
   /** The class that maintains the connection to a remote device. */
   class CBtSocketsEngine* iSocketsEngine;
   /** The class that tests the Bluetooth power setting. */
   class CBtPowerChecker* iPower;
   /**
    * The class that repeatedly searches for a serial port on a remote
    * device.
    */
   class CSdpRepeater* iRepeater;
   class TSdpRepeatSettings iRepeatSettings;
   //@}
   /** @name From CActiveLog. */
   //@{
   virtual CArrayPtr<CActiveLog>* SubLogArrayLC();
   //@}
};

#endif

