/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

// INCLUDES
#include <coecntrl.h>
#include <aknwaitnotewrapper.h>

#include "ViewBase.h"
#include "WelcomeContainer.h"
#include "Log.h"
#include "memlog.h"

// FORWARD DECLARATIONS
class CWayFinderAppUi;
// CLASS DECLARATION

/**
*  CWelcomeView  container control class.
*  
*/
class CWelcomeView : 
   public CViewBase
{
public: // Constructors and destructor

   CWelcomeView(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL();

   static class CWelcomeView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);
   static class CWelcomeView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);


   /**
    * Destructor.
    */
   ~CWelcomeView();

public: // New functions
   TBool SetConStatusImage(class CFbsBitmap* bmp, class CFbsBitmap* mask);
   void SetImage(CGuidePicture* aStartupImage);
   void SetImage(class CImageHandler* aImageHandler);

/*    void PictureError( TInt aError ); */

public: // Functions from base classes

   TRgb GetNewsBgColor();
   TPoint GetNewsConStatusPos();
   /**
    * From AknView.
    * @return the ID of the view.
    */
   TUid Id() const;

   /**
    * From AknView, takes care of command handling.
    * @param aCommand command to be handled
    */
   void HandleCommandL(TInt aCommand);

   /**
    * From AknView, handles a change in the view.
    */
   void HandleClientRectChange();

//    void HandleResourceChange(TInt aType);
private: // New Functions

private: // Functions from base classes

   /**
    * From AknView, Called when the option softkey is pressed.
    * @param aResourceId the focused resource.
    * @param aMenuPane the current menu.
    */
   void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoActivateL( const TVwsViewId& aPrevViewId,
                     TUid aCustomMessageId,
                     const TDesC8& aCustomMessage);

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: //data

   CWelcomeContainer* iContainer;
   CGuidePicture* iStartupImage;
   class CImageHandler* iImageHandler;

   isab::Log* iLog;

   //TBool iNav2Started;
   

};

#endif

// End of File
