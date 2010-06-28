/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYDESTDIALOG_H
#define MYDESTDIALOG_H

// INCLUDES
#include <coecntrl.h>  // for coe control
#include <aknview.h>
#include <eikdialg.h>
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for edwin text boxes

#include "CoordinatesControl.h"
#include "TwoEdwinControl.h"

#include "Log.h"

// CLASS DECLARATION

/**
*  Dialog class
*  Test various controls.
*/
class CMyDestDialog:public CEikDialog
{
public: // Constructors and destructor

   /**
    * Two-phased constructor.
    */
   static CMyDestDialog* NewL(isab::Log* aLog);

   /**
    * Destructor
    */
   ~CMyDestDialog();

private:

   void ConstructL();

   /**
    * Constructor
    */
   CMyDestDialog(isab::Log* aLog);

public: // New functions

   void SetName( TDes* aName );

   void SetDescription( TDes* aDescription );

   void SetCategory( TDes* aCategory );

   void SetAlias( TDes* aAlias );

   void SetLatitude( TInt32* aLatitude );

   void SetLongitude( TInt32* aLongitude );

   /**
    * Set the latitude value.
    * @param aSign the latitude sign N or S.
    * @param aDeg the degree value 0-90.
    * @param aMin the minute value 0-59.
    * @param aSec the second value 0-59.
    */
   //void SetLatitude( TChar* aSign, TInt* aDeg, TInt* aMin, TInt* aSec );

   /**
    * Set the longitude value.
    * @param aSign the longitude sign E or W.
    * @param aDeg the degree value 0-90.
    * @param aMin the minute value 0-59.
    * @param aSec the second value 0-59.
    */
   //void SetLongitude( TChar* aSign, TInt* aDeg, TInt* aMin, TInt* aSec );

public: // Functions from base classes

	TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

   /**
    * From CAknDialog set parameters before showing dialog.
    */
   void PreLayoutDynInitL();

   void PostLayoutDynInitL();

   /**
    * From CAknDialog update member variables of CMyDestDialog.
    * @param aButtonId The ID of the button that was activated.
    * @return Should return ETrue if the dialog should exit,
    *    and EFalse if it should not
    */
   TBool OkToExitL(TInt aButtonId);

private: // Functions from base classes

   /**
    *
    */
	SEikControlInfo CreateCustomControlL(TInt aControlType);

private:

   CEikEdwin*           iEdwinName;   
   /// The name description control
   CTwoEdwinControl*    iNameDescriptionControl;
   
   /// The category alias control
   //CTwoEdwinControl*    iCategoryAliasControl;
   
   /// The coordinates control
   CCoordinatesControl*    iCoordinatesControl;

   TDes* iName;

   TDes* iDescription;

   TDes* iCategory;

   TDes* iAlias;

   TInt32* iLatitude;

   TInt32* iLongitude;

   /// Latitude sign
   TChar* iLatSign;

   /// Latitude degrees
   TInt* iLatDeg;

   /// Latitude minutes
   TInt* iLatMin;

   /// Latitude seconds
   TInt* iLatSec;

   /// Longitude sign
   TChar* iLonSign;

   /// Longitude degrees
   TInt* iLonDeg;

   /// Longitude minutes
   TInt* iLonMin;

   /// Longitude seconds
   TInt* iLonSec;

   isab::Log* iLog;
};

#endif //MyDestDIALOG_H

// End of File
