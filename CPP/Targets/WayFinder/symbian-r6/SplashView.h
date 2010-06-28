/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SPLASHVIEW_H
#define SPLASHVIEW_H

// INCLUDES
#include <coecntrl.h>
#include <aknview.h>
#include <aknwaitnotewrapper.h>

#include "IAPSearchGui_old.h"

#include "SplashContainer.h"
#include "Log.h"
#include "memlog.h"
#include "ProgressDlgObserver.h"

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class GuiDataStore;
// CLASS DECLARATION

/**
*  CSplashView  container control class.
*  
*/
class CSplashView : 
   public CAknView,
   public IAPSearchGui_old,
   public MProgressDlgObserver
{
public: // Constructors and destructor

   CSplashView(isab::Log* aLog);

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( CWayFinderAppUi* aWayFinderUI );

   static class CSplashView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);
   static class CSplashView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CSplashView();

public: // New functions
   HBufC* GetApplicationNameLC();
   TRgb GetNewsBgColor();
   void SetImage(CGuidePicture* aStartupImage);
   void SetImage(class CImageHandler* aImageHandler);

   GuiDataStore* GetGuiDataStore();

   TBool CheckAudioResource();
   void AskForIAP();
   void AskForIAPDone();
   void CheckIAP();
   void SearchingForIAP(TBool aOn);
   TBool SearchingForIAP();
   void ReportProgress(TInt aVal, TInt aMax, HBufC* aName);

   void CheckGoToStart();

   void SetShownNewsChecksum();
/*    TBool ShowSplash(); */

   TBool DoShowIAPProgress();
   TBool DoShowLatestNewsImage();
   TBool DoShowNews();
   TBool DoCheckAudioResource();

   TPtrC16 ProgramPath();

   void PictureError( TInt aError );

public: // Functions from base classes

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

   /**
    * From MProgressDlgObserver
    */
   void ProgressDlgAborted();

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
   
   CWayFinderAppUi* iWayFinderUI;

   CSplashContainer* iContainer;
   CGuidePicture* iStartupImage;
   class CImageHandler* iImageHandler;
   class CProgressDlg* iProgressDlg;

   TInt iMode;

   TBool iShowNews;
   TBool iShowUpgrade;
   TBool iDontTimeout;
   TBool iSearchingForIAP;
   TBool iAskForIAP;
   isab::Log* iLog;
   

};

#endif

// End of File
