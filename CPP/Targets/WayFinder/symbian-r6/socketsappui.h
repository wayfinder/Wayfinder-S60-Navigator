/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __SOCKETSAPPUI_H__
#define __SOCKETSAPPUI_H__

#include <aknappui.h>

class CSocketsAppView;
class CSocketsEngine;

/*! 
  @class CSocketsAppUi
  
  @discussion An instance of class CSocketsAppUi is the UserInterface part of the AVKON
  application framework for the Sockets example application
  */
class CSocketsAppUi : public CAknAppUi
    {
public:
/*!
  @function ConstructL
  
  @discussion Perform the second phase construction of a CSocketsAppUi object
  this needs to be public due to the way the framework constructs the AppUi 
  */
    void ConstructL();

/*!
  @function CSocketsAppUi
  
  @discussion Perform the first phase of two phase construction.
  This needs to be public due to the way the framework constructs the AppUi 
  */
    CSocketsAppUi();


/*!
  @function ~CSocketsAppUi
  
  @discussion Destroy the object and release all memory objects
  */
    ~CSocketsAppUi();

/*!
  @function SetServerName

  @discussion Set name of server to connect to
  @param aName new server name
  */
    void SetServerName(const TDesC& aName);

/*!
  @function ServerName

  @discussion Get server name
  @result name of server
  */
    const TDesC& ServerName() const;

/*!
  @function SetPort

  @discussion Set port number to connect to
  @param aPort new port number
  */
    void SetPortNumber(TInt aPort);

/*!
  @function Port

  @discussion Get port number
  @result port number
  */
    TInt PortNumber() const;

public: // from MEikMenuPaneObserver
/*!
  @function DynInitMenuPaneL

  @discussion Initialise a menu pane before it is displayed
  @param aMenuId id of menu
  @param aMenuPane handle for menu pane
  */
    void DynInitMenuPaneL(TInt aMenuId, CEikMenuPane* aMenuPane);


private: // from CEikAppUi
/*!
  @function HandleCommandL
  
  @discussion Handle user menu selections
  @param aCommand the enumerated code for the option selected
  */
    void HandleCommandL(TInt aCommand);

/*!
  @function HandleKeyEventL
  
  @discussion Handle user key input
  @param aKeyEvent event information
  @param aType event type
  @result EKeyWasConsumed
  */
    TKeyResponse HandleKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

private: // Member data
    /** The application view */
    CSocketsAppView*    iAppView;

    CSocketsEngine*     iSocketsEngine;
    };


#endif // __SOCKETSAPPUI_H__

