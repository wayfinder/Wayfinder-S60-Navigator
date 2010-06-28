/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef NEWDESTDIALOG_H
#define NEWDESTDIALOG_H

// INCLUDES
#include <aknform.h>
#include <eikedwob.h>
#include "WayFinderConstants.h"
#include "Log.h"

class CNewDestView;

// CONSTANTS

// CLASS DECLARATION

/**
 *  CNewDestDialog  Dialog control class.
 *  
 */
class CNewDestDialog : public CAknForm, MEikEdwinObserver
{
public: // Constructors and destructor

   /**
    * Two-phased constructor.
    */
   static CNewDestDialog* NewL( CNewDestView* aView, isab::Log* aLog);

   /**
    * Destructor.
    */
   virtual ~CNewDestDialog();

   TInt ExecuteLD( TInt aResourceId );
   
   void PrepareLC( TInt aResourceId );

private:

   CNewDestDialog(isab::Log* aLog) : iLog(aLog) {}
   
   /**
    * Second-phase constructor.
    */
   void ConstructL( CNewDestView* aView );

public: // New functons

   void Close(TBool aProcessExit); 
          
   void RemoveInputFocus();

   void SetCountry( TDesC &aCountry );

   void SetCity( TDesC &aCity );
   void SetHouseNum( TDesC &aCity );

   void SetString( TDesC &aSearchString );

public: // Functions from base classes

   void HandleEdwinEventL( CEikEdwin* aEdwin,TEdwinEvent aEventType );

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

protected:
   TBool OkToExitL(TInt aButtonId);

private:  // Functions from base class
          
   /**
    * From CEikDialog 
    * Set default field value to member data.
    */
   void PostLayoutDynInitL();

   /**
    * From CEikDialog 
    * Load the correct bitmap.
    */
   void PreLayoutDynInitL();

   /**
    * From CAknForm 
    * Show save query. If user answers "No" to this query.
    * return field value to the value which is before editing.
    */
   TBool QuerySaveChangesL();

   /**
    * From CAknForm 
    * Save the contents of the form.
    */
   TBool SaveFormDataL();

   void DynInitMenuPaneL( TInt aMenuId, CEikMenuPane* aMenuPane );

   void ProcessCommandL( TInt aCommandId );

   void PrepareForFocusTransitionL();

private: //data

   TBuf<256> iCountry;
   TBuf<256> iCity;
   TBuf<256> iHouseNum;
   TBuf<256> iSearchString;

   TBool iHasFocus;
   TBool iEditKeyModifier;   
   CNewDestView* iView;
   isab::Log* iLog;
   TBool iProcessExit;
   TBool iShowHelp;
};

#endif

// End of File
