/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VIEW_BASE_H
#define VIEW_BASE_H

#include <aknview.h>
#include <aknviewappui.h>
#include <eikbtgpc.h> 

#include "WFCommonCommandConstants.h"

/**
 * The base class for all views on S60.
 * 
 */
class CViewBase : public CAknView
{
protected:
   /**
    * Class constructor
    *
    * @param aWayfinderAppUi   The Wayfinder AppUi.
    */
   CViewBase(class CWayFinderAppUi* aWayfinderAppUi);

   /**
    * Base second stage ConstructL.
    */
   void ViewBaseConstructL(TInt aResourceId);

   /**
    * The AppUi that all views wants to know about.
    */
   class CWayFinderAppUi* iWayfinderAppUi;

   /**
    * Previous view id.
    */
   TVwsViewId iPrevViewId;

   /**
    * The view's custom message id.
    */
   TInt iCustomMessageId;
   
   TBool iActivated;

public:
   /**
    * Class Destructor
    */
   virtual ~CViewBase();

   //virtual TVwsViewId ViewId() const = 0;

   virtual TInt GetViewState();

   virtual void DoActivateL(const TVwsViewId& aPrevViewId, 
                            TUid aCustomMessageId,
                            const TDesC8& aCustomMessage);

   void ShowMenuBar(TBool aShow);
   
   TBool IsActivated();
};

#endif
