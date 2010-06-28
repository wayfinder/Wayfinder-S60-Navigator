/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include "SprocketTalker.h"
#include "Nav2.h"

namespace isab {
   class QueueSerial;
   class Buffer;
}

/**
 * Message handling class.
 * 
 *  Description:
 *     Handles messages to and from Nav2.
 */
class CMessageHandler : public CBase, public MSprocketTalker
                        // : public MUIConnection
{
public: 
   /** @name Constructors and destructor. */
   //@{
   /** Default constructor. */
   CMessageHandler();

   /**
    * Second phase constructor. 
    * @param aAppUi the appui that should communicate with Nav2.
    */
   void ConstructL( class CWayFinderAppUi* aAppUi );
   /**
    * Second phase constructor. 
    * @param aAppUi the appui that should communicate with Nav2.
    * @param aNav2  the Nav2 communicatio channel. 
    */
   void ConstructL( class CWayFinderAppUi* aAppUi, isab::Nav2::Channel* aNav2);

   /** Virtual destructor. */
   virtual ~CMessageHandler();
   //@}
public:

   /** Connect to Nav2. */
   void ConnectL();

   /** 
    * Connect to Nav2 on a remote host.
    * @param aServerIP the ip-address of the remote host, as a string.
    * @param aPort     the port Nav2 uses on the remote host.
    */
   void ConnectL( const TDesC &aServerIP, TInt aPort );

   /** Disconnect from Nav2. */
   void Disconnect();

   /** 
    * Really disconnect. Only applies if SprocketsEngine is used.
    * Warning! once called, this object has forgotten its sprocket.
    */
   void Release();
   /**
    * Tests whether this CMessageHandler is connected to Nav2.
    * @return true if the GUI is connected to Nav2.
    */
   TBool IsConnected();

   /**
    * Set name of server to connect to
    * @param aName new server name
    */
   void SetServerName(const TDesC& aName);

   /**
    * Get server name
    * @return name of server
    */
   const TDesC& ServerName() const;

   /**
    * Set port number to connect to
    * @param aPort new port number
    */
   void SetPortNumber(TInt aPort);

   /**
    * Get port number
    * @return port number
    */
   TInt PortNumber() const;

   /**
    * Sends a message to Nav2.
    * @param aBuffer the data to send.
    */
   void SendMessageL( isab::Buffer* aBuffer );

   /**
    * Sends data to Nav2.
    * @param aBytes the data to send.
    */
   void WriteBytes(const TDesC8& aBytes );

   /**Called by SprocketsEngine when Nav2 is in trouble.*/
   void Panic();

public:
   /**
    @function ReceiveMessage

    @discussion Display text on console
    @param aDes text to display
    */
   void ReceiveMessageL( const TDesC8& aMessage, const TInt aMessageLength );

   /**
    *   Same as the other Rec_i_veMessage, but uses the length from
    *   <code>aMessage</code>.
    */
   virtual void ReceiveMessageL( const TDesC8& aMessage );

   /*!
    @function ErrorNotify

    @discussion Notify user of an error
    @param aErrMessage message associated with error
    @param aErrCode error code
    */
   void ErrorNotify( TInt aResourceId, TInt aErrCode );

   /*!
    @function SetStatus

    @discussion Change 'status' display.
    @param aStatus new status text to display
    */
   void SetStatus(const TDesC& aStatus);

   ///Used to signal connection complete.
   void ConnectedL()
   {
   }

public: //needed for memory logging
   class CWayFinderAppUi* iAppUi;
private:
   class isab::QueueSerial* m_serialQueue;

   class CSocketsEngine*  iSocketsEngine;
   class CSprocketsEngine*  iSprocketsEngine;

};

#endif

// End of File
