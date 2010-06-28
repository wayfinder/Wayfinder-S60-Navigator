/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef __SETTINGSVIEW_H__
#define __SETTINGSVIEW_H__

#include "ViewBase.h"

#include "NavServerComEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "DistancePrintingPolicy.h"
#include "SettingsData.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CSettingsContainer;
class CSettingsData;

class CAknTabGroup;
class CAknNavigationDecorator;

namespace isab{
   class TrackPINList;
   class SimpleParameterMess;
   class GeneralParameterMess;
}

// CLASS DECLARATION

/**
*  CSettingsView view class.
* 
*/
class CSettingsView : public CViewBase
{

public: // Constructors and destructor


   CSettingsView(class CWayFinderAppUi* aUi);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CSettingsView object.
   ///        Note that this object is still on the CleanupStack.
   static class CSettingsView* NewLC(CWayFinderAppUi* aUi, isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CSettingsView object.
   static class CSettingsView* NewL(CWayFinderAppUi* aUi, isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CSettingsView();

   /**
    * Set cache object pointer.
    */
   void SetCache(CSettingsData* aData);

public: // New Functions

   void InitData();

   void SetUsername( TDesC &aUsername );

   void SetPassword( TDesC &aPassword );

   void SetServer( TDesC &aServer );

   void SetVolume( TInt aVolume );

   void SetDistanceMode( isab::DistancePrintingPolicy::DistanceMode aMode );

   void SetLanguage( TInt language );

   void SetUseSpeaker( TBool aUseSpeaker );
   void SetIAP( TInt iap );

   void SetMapCacheSize( TInt aSize );
   void SetMapType( MapType aMapType );
   void SetPreferredGuideMode( enum preferred_guide_mode aGuideMode );
   void SetGPSAutoConnect( TInt aAuto );
   void SetBtGpsAddressL(const char** aString, TInt aCount);
   void SetTrackingType( TrackingType aMapTrackingOrientation );

   void setTrackingPIN( const TDesC &aPIN );
   void setTrackPINList( isab::TrackPINList* l );

   CWayFinderAppUi & GetWayFinderAppUi() {
      return *iWayfinderAppUi;
   }

   void ToggleServer();

   /**
    * Calls the private version of SendVectorMapSettings with
    * the values stored in the settings data. Convenience function
    * for access to this from outside the class.
    */
   void SendVectorMapSettings();

private:  // New Functions

   void SendGuiProtMessage(class isab::GuiProtMess* message);
   void SendGeneralParam(int32 paramId, int32 value);
   void SendVectorMapSettings(int32 cacheSize, int32 mapType,
      int32 trackingOrientation, int32 preferredGuideMode);
   void SaveSettings();

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
                     const TDesC8& aCustomMessage );

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: // Data
   
   /// The controls container.
   CSettingsContainer* iContainer;

   CSettingsData*     iData;

   CAknNavigationDecorator *      iTabsDecorator;
   CAknTabGroup * iTabGroup;

   TBool iShutdownNeeded;

};

#endif // __SETTINGSVIEW_H__
