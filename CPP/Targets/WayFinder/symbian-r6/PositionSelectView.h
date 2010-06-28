/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef POSITIONSELECTVIEW_H
#define POSITIONSELECTVIEW_H

// INCLUDES
#include "ViewBase.h"
#include "WayFinderConstants.h"
#include "Log.h"
#include "memlog.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMessageSender.h"
#include "NavServerComEnums.h"
#include "EventGenerator.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CPositionSelectContainer;
class GuiDataStore;

// CLASS DECLARATION

/**
*  CPositionSelectView view class.
* 
*/
class CPositionSelectView
 : public CViewBase,
   public GuiProtMessageReceiver
{
public: // Constructors and destructor

   /**
    * The events for the event generator.
    */
   enum TRoutePlannerEvent { 
      ERoutePlannerEventShowRouteAsQueryDlg
   };

   CPositionSelectView(class CWayFinderAppUi* aUi, 
                       isab::Log* aLog, 
                       GuiProtMessageSender& aSender);

   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@param aSender, a reference to an instance of GuiProtMessageSender, used
   ///                for sending messages to Nav2.
   ///@return a new, completely constructed, CPositionSelectView object.
   ///        Note that this object is still on the CleanupStack.
   static class CPositionSelectView* NewLC(CWayFinderAppUi* aUi, 
                                           isab::Log* aLog,
                                           GuiProtMessageSender& aSender);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@param aSender, a reference to an instance of GuiProtMessageSender, used
   ///                for sending messages to Nav2.
   ///@return a new, completely constructed, CPositionSelectView object.
   static class CPositionSelectView* NewL(CWayFinderAppUi* aUi, 
                                          isab::Log* aLog,
                                          GuiProtMessageSender& aSender);

   /**
    * Destructor.
    */
   ~CPositionSelectView();

   /**
    * Enum describing different kinds of transportation modes. This enum
    * are used in the ui for lists etc where the index is going from zero
    * and so on. The NavServerComEnums::VehicleType has a lot of values in
    * it that aren't used hence the index for pedestrian mode is two and 
    * it's dangerous to take for granted that the enum isn't going to change.
    */
   enum TTransportationMode
   {
      EPassengerCar,
      EPedestrian,
      EInvalidVehicleType
   };


public: // New Functions

   GuiDataStore* GetGuiDataStore();

   TBool VectorMaps();
   TPoint GetOrigin();
   TPoint GetDestination();
   TBool HasOrigin();
   TBool HasDestination();

   void SetOrigin( isab::GuiProtEnums::PositionType aType,
         const TDesC& aName, const char* aId,
         TInt32 aLat = MAX_INT32, TInt32 aLon = MAX_INT32 );

   /**
    * Xxx doc!
    * n.b. The aName must contain tabs!!
    */
   void SetDestination( isab::GuiProtEnums::PositionType aType,
                        const TDesC& aName, 
                        const char* aId,
                        TInt32 aLat = MAX_INT32, 
                        TInt32 aLon = MAX_INT32, 
                        TBool aResetting = EFalse );

   void Route();

   TBool IsCurrentSet();
   TBool AreSet();
   TBool IsGpsConnected();

   /**
    * Resets origin in the listbox. All origin members such as
    * iOriginPoint is reset as well.
    */
   void ResetOriginL();

   /**
    * Resets destination in the listbox. All destination members such as
    * iDestPoint is reset as well.
    */
   void ResetDestinationL();

   /**
    * Resets the selected index in the listbox back to its initial
    * value.
    */
   void ResetSelection();

   /**
    * Calls wayfinderappuis member iPathFinder for receiving
    * the name and path to the mbm file.
    * @return The path to the mbm file.
    */
   TPtrC GetMbmName();

   /**
    * Sends the new vehicletype to Nav2, will end up in GuiProtReceiveMessage
    * and a route will be calculated.
    * 
    * @param aVehicleType, the new vehicletype, if EInvalidVehicleType no message
    *                      will be sent.
    */
   void SendVehicleType(isab::NavServerComEnums::VehicleType aVehicleType);

   /**
    * Convert from TTransportationMode to NavServerComEnums::VehicleType....
    * 
    * @param aMode, the transportation mode to be converted into NavServerComEnums::VehicleType.
    * @return, The converted transportation mode.
    */
   isab::NavServerComEnums::VehicleType GetNav2TransportMode(TTransportationMode aMode);

private:
   /**
    * Displays a selection list dialog that lets the user select if the route
    * should be calculated for car or for pedestrian.
    */
   void ShowRouteAsQueryDlgL();

   void ShowDetails();

public: // Functions from base classes

   /**
    * Generate events with the event generator.
    */
   void GenerateEvent(enum TRoutePlannerEvent aEvent);

   /**
    * Handles events from the event generator.
    */
   void HandleGeneratedEventL(enum TRoutePlannerEvent aEvent);

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

   CWayFinderAppUi* getAppUi() const {return iWayfinderAppUi;}

   /** 
    * From CViewBase
    * Returns the internal state of the view, to be used when
    * activating other views like favorites and search.
    *
    * @return   The internal state of the view.
    */
   TInt GetViewState();

   /**
    * Where messages from Nav2 ends up.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess* mess);

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

private: // Data

   /// Eventgenerator sending and receiving events
   typedef CEventGenerator<CPositionSelectView, enum TRoutePlannerEvent>
      CRoutePlannerEventGenerator;
   /// Event generator
   CRoutePlannerEventGenerator* iEventGenerator;

   /// The controls container.
   CPositionSelectContainer* iContainer;

   isab::Log* iLog;

   /// Message sender
   GuiProtMessageSender& iSender;

   isab::GuiProtEnums::PositionType iOriginType;

   isab::GuiProtEnums::PositionType iDestinationType;

   HBufC* iOriginName;

   HBufC* iDestinationName;

   char* iOriginId;

   char* iDestinationId;

   TPoint iOriginPoint;

   TPoint iDestinationPoint;

   TBool iIsOriginSet;

   TBool iIsDestinationSet;

   TBool iOriginSelected;

   TInt32 iCustomMessage;
};

#endif

// End of File
