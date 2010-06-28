/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BT_ENGINE_OBSERVER_H
#define BT_ENGINE_OBSERVER_H
#include "BtEngineCodes.hrh"
/**
 * This interface class is used to interface with CBluetoothEngine.
 */
class MBluetoothEngineObserver
{
public:
   /**
    * Called whenever something happens that CBluetoothEngine thinks
    * its observer might be interested in.
    * @param aEvent the event.
    */
   virtual void BtEngineInfo(enum TBtEngineEvent aEvent) = 0;
   
   /**
    * During repeated searches for a new device, this function is
    * called each time a new search is started.
    * @param aNum   the order number of the current search. 
    * @param aTotal the total number of searches in this sequence.
    */
   virtual void BtEngineSearching(TInt aNum, TInt aTotal) = 0;

   /**
    * Each time a connection is completed the CBluetoothEngine will
    * call this function with the address and name of the connected
    * device.
    * @param aAddr the address of the connected device.
    * @param aName the name of the connected device. 
    */
   virtual void BtEngineDevice(const class TBTDevAddr& aAddr, 
                               const TDesC& aName) = 0;

   /**
    * Data from the remote device is passed to the observer through
    * this function.
    * @param aData the data.
    */
   virtual void BtEngineData(const TDesC8& aData) = 0;

   /**
    * This function will be called once whenever the Bluetooth
    * connection is unexpectedly lost. The implication is that the UI
    * may trigger an alert.
    */
   virtual void BtConnectionLost(TBool aWillTryToReconnect) = 0;
};

#endif
