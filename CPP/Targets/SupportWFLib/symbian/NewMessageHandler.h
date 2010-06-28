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

#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

// INCLUDES
#include "SymbianTcpConnection.h"

class CNewSocketsEngine;
class MTCPHandler;

// CLASS DECLARATION

/**
* Message handling class.
* 
*/
class CNewMessageHandler : public SymbianTcpConnection
{

public: // // Constructors and destructor

   CNewMessageHandler();

   static CNewMessageHandler* NewL( MTCPHandler* aTCPHandler, RSocketServ* aSockServ );

   static CNewMessageHandler* NewLC( MTCPHandler* aTCPHandler, RSocketServ* aSockServ );
   
   /**
    * EPOC default constructor.
    */
   void ConstructL( MTCPHandler* aTCPHandler, RSocketServ* aSockServ);

   /**
    * Destructor.
    */
   virtual ~CNewMessageHandler();

public: // New functions

   /**
    * Connect to NavCtrl.
    */
   void ConnectL( TDesC &aServerIP, TInt aPort, TInt aIAP );

   void Listen( TInt aPort );

   /**
    * Disconnect from NavCtrl.
    */
   void Disconnect();

   /**
    * @retrun true if the GUI is connected to NavCtrl.
    */
   TBool IsConnected();

   void Cancel();
   /*!
    @function ServerName

    @discussion Get server name
    @result name of server
    */
   const TDesC& ServerName() const;

   /*!
    @function Port

    @discussion Get port number
    @result port number
    */
   TInt PortNumber() const;

   /*!
    @function GetIAP

    @discussion Get number of used IAP
    @result IAP number
    */
   TInt GetIAP() const;

   void CancelDownload();
   
public: // Functions from base classes

   // From SymbianTcpConnection

   /*!
    @function WriteMessageL

    @discussion Sends a message to the navigator.
    @param aMessage the message to send.
    */
   void SendMessageL( TDesC8 &data, int length );

   /*!
    @function ReceiveMessage

    @discussion Display text on console
    @param aDes text to display
    */
   void ReceiveMessageL( const TDesC8& aMessage, const TInt aMessageLength );

   /*!
    @function ErrorNotify

    @discussion Notify user of an error
    @param aErrMessage message associated with error
    @param aErrCode error code
    */
   void ErrorNotify( TDesC& aErrMessage, TInt aErrCode );

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
   void SetStatus( TSocketsEngineState aNewStatus );

   void ConnectionCancelled();


private: //Data

   CNewSocketsEngine*  iSocketsEngine;

   MTCPHandler* iTCPHandler;

};

#endif

// End of File
