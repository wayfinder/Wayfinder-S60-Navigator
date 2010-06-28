/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __SOCKETSBASEDIALOG_H__
#define __SOCKETSBASEDIALOG_H__

#include <aknform.h>

/*! 
  @class CSocketsBaseDialog
  
  @discussion Virtual base class for dialogs in this application
  */
class CSocketsBaseDialog : public CAknDialog
{
public:
/*!
  @function ExecuteLD

  @discussion Execute (display and run) the dialog
  @result zero if dialog was cancelled, otherwise ID of button which closed dialog
  */
    virtual TInt ExecuteLD() = 0;

protected:
/*!
  @function CSocketsBaseDialog
  
  @discussion Perform the first phase of two phase construction 
  */
	CSocketsBaseDialog(class CMessageHandler& aAppUi);

/*!
  @function AppUi

  @discussion Get handle to appUi
  @result handle to appUi
  */
    class CMessageHandler& AppUi() const;

/*!
  @function OkToExitL

  @discussion Handle attempt to dismiss dialog
  @param aKeycode keycode used to close dialog
  @result true if OK to close dialog
  */
    virtual TBool OkToExitL(TInt aKeycode) = 0;

/*!
  @function SetText

  @discussion Copy text into an edwin type control
  @param aControl control to change
  @param aText text to set
  */
    void SetTextL(TInt aControl, const TDesC& aText);

/*!
  @function SetSecretText

  @discussion Copy text into a secret editor type control
  @param aControl control to change
  @param aText text to set
  */
    void SetSecretText(TInt aControl, const TDesC& aText);

/*!
  @function SetNumber

  @discussion Copy number into a number editor type control
  @param aControl control to change
  @param aNumber number to set
  */
    void SetNumber(TInt aControl, TInt aNumber);

/*!
  @function GetText

  @discussion Copy text out of an edwin type control
  @param aControl control to copy from
  @param aText descriptor to copy to
  */
    void GetText(TInt aControl, TDes& aText);

/*!
  @function GetSecretText

  @discussion Copy text out of a secret editor type control
  @param aControl control to copy from
  @param aText descriptor to copy to
  */
    void GetSecretText(TInt aControl, TDes& aText);

/*!
  @function GetNumber

  @discussion Copy number out of a number editor type control
  @param aControl control to copy from
  @result number from control
  */    
    TInt GetNumber(TInt aControl);

private: // Member variables
    class CMessageHandler&   iAppUi;
};

#endif // __SOCKETSBASEDIALOG_H__

