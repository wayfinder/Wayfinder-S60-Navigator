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

#ifndef HELP_URL_HANDLER_H 
#define HELP_URL_HANDLER_H

#include <e32base.h>
#include "machine.h"

/** 
 * Struct used to contain the view uid mapped to 
 * the views help page name. 
 */
struct ViewNameMapping {
   TInt iViewId;
   char* iViewName;
};

class CHelpUrlHandler : public CBase
{
private:
   /**
    * Class constructor
    *
    * @param 
    */
   CHelpUrlHandler(TInt aNoOfMappings);

   /**
    * Symbian second stage ConstructL.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    */
   void ConstructL(const ViewNameMapping* aViewNameMappings, TInt aLangCodeId);

   /**
    * Symbian second stage ConstructL.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    * @param aLangCode            The used language code in iso format.
    */
   void ConstructL(const ViewNameMapping* aViewNameMappings, const TDesC& aLangCode);

public:
   /**
    * Class Destructor
    */
   virtual ~CHelpUrlHandler();

   /**
    * Symbian static NewLC function.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    * @return                     A HelpUrlHandler instance.
    */
   static CHelpUrlHandler* NewLC(const ViewNameMapping* aViewNameMappings, 
                                 TInt aNoOfMappings, 
                                 TInt aLangCodeId);

   /**
    * Symbian static NewL function.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    * @return                     A HelpUrlHandler instance.
    */
   static CHelpUrlHandler* NewL(const ViewNameMapping* aViewNameMappings, 
                                TInt aNoOfMappings, 
                                TInt aLangCodeId);

   /**
    * Symbian static NewLc function.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    * @param aNoOfMappings        
    * @param aLangCode            The used language code in iso format.
    * @return                     A HelpUrlHandler instance.
    */
   static CHelpUrlHandler* NewLC(const ViewNameMapping* aViewNameMappings, 
                                 TInt aNoOfMappings, 
                                 const TDesC&  aLangCode);

   /**
    * Symbian static NewL function.
    *
    * @param aViewNameMappings    Array of the applications view-uids mapped 
    *                             to the help page name.
    * @param aNoOfMappings        
    * @param aLangCode            The used language code in iso format.
    * @return                     A HelpUrlHandler instance.
    */
   static CHelpUrlHandler* NewL(const ViewNameMapping* aViewNameMappings, 
                                TInt aNoOfMappings, 
                                const TDesC& aLangCode);

public:
   /** 
    * Returns a formatted string for use as help url.
    * Allocates and leaves the string on the cleanup stack.
    *
    * @param aViewId       The id of the current view.
    * @param aLangCode     The current language code.
    * @return              The full url to the help file.
    */
   HBufC* FormatLC(TInt aViewId,
                   const TDesC& aAnchorName);

   /** 
    * Returns a formatted string for use as help url.
    * Allocates and leaves the string on the cleanup stack.
    *
    * @param aViewId       The id of the current view.
    * @param aLangCode     The current language code.
    * @return              The full url to the help file.
    */
   HBufC* FormatLC(TInt aViewId, const HBufC* aAnchorString = NULL);

   /** 
    * Returns a formatted string for use as help url.
    * Allocates the string but does not leave it on the cleanup stack.
    *
    * @param aViewId       The id of the current view.
    * @param aLangCode     The current language code.
    * @return              The full url to the help file.
    */
   HBufC* FormatL(TInt aViewId,
                  const TDesC& aAnchorName);

   /** 
    * Returns a formatted string for use as help url.
    * Allocates the string but does not leave it on the cleanup stack.
    *
    * @param aViewId       The id of the current view.
    * @param aLangCode     The current language code.
    * @return              The full url to the help file.
    */
   HBufC* FormatL(TInt aViewId, const HBufC* aAnchorString = NULL);

   TInt HelpFromView();

   /**
    * Get the help page name for view.
    *
    * @param aViewId The view.
    * @return The name for the view.
    */
   HBufC* GetHelpPageNameL( TInt aViewId );

private:
   ViewNameMapping** iViewNameMappings;
   TInt iNoOfMappings;
   HBufC* iLangCode;

};

#endif /* HELP_URL_HANDLER_H */
