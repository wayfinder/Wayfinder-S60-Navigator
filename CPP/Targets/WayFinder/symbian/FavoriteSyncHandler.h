/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef C_FAVORITE_SYNC_HANDLER_H
#define C_FAVORITE_SYNC_HANDLER_H

#include "arch.h"
#include <vector>

#include "GuiProt/GuiProtMessageSender.h"
#include "FavoriteSyncManagerObserver.h"
#include "WFLMSObserver.h"
#include "FavoriteSyncHandlerObserver.h"
#include "EventGenerator.h"

namespace isab {
   class GuiFavorite;
   class Favorite;
}

using namespace isab;

class CFavoriteSyncHandler : public CBase,
                             public GuiProtMessageReceiver, 
                             public MFavoriteSyncManagerObserver, 
                             public MWFLMSObserver
{
public:
   /**
    * Standard NewLC function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aSender     The message sender.
    * @param aLmsId      The id of the wayfinder LMS.
    * @param aImeiCrcHex This phones IMEI in crc and hex format.
    * @return   The newly created instance of CFavoriteSyncHandler.
    */
   static CFavoriteSyncHandler* NewLC(MFavoriteSyncHandlerObserver& aObserver, 
                                      GuiProtMessageSender& aSender, 
                                      const TDesC& aLmsId, 
                                      const char* aImeiCrcHex);

   /**
    * Standard NewL function.
    *
    * @param aObserver   The observer that receives status updates.
    * @param aSender     The message sender.
    * @param aLmsId      The id of the wayfinder LMS.
    * @param aImeiCrcHex This phones IMEI in crc and hex format.
    * @return   The newly created instance of CFavoriteSyncHandler.
    */
   static CFavoriteSyncHandler* NewL(MFavoriteSyncHandlerObserver& aObserver, 
                                     GuiProtMessageSender& aSender, 
                                     const TDesC& aLmsId, 
                                     const char* aImeiCrcHex);

   /**
    * Destructor
    */
   virtual ~CFavoriteSyncHandler();

   /**
    * Events that can be triggered in the state machine.
    */
   enum TWFFavSyncManEvent {
      EWFGetLmsFavs,            // Get the LMS Favs
      EWFGetClientFavs,         // Get the Client Favs
      EWFGetServerFavs,         // Get the Server Favs
      EWFDoLmsSync,             // Start sync of LMS - Client
      EWFDoServerSync,          // Start sync of Server - Client
      EWFDoNoWFLMSSync,           // No WFLMS, add to lms from server list
      EWFRequestServerFavsSync, // Get the Server - Nav2 sync
      EWFAddToLms,              // Add to LMS
      EWFUpdateToLms,           // Update to LMS
      EWFDeleteFromLms,         // Delete from LMS
      EWFAddToServer,           // Add to Server
      EWFUpdateToServer,        // Update to Server
      EWFDeleteFromServer,      // Delete from Server
      EWFUpdateNav2ToServer,    // Request Nav2 - Server sync
      EWFCheckEmptyLms,         // Check for empty lms
      EWFSyncCompelete          // All done event
   };

   /**
    * States for the state machine.
    */
   enum TWFFavSyncManState {
      EWFFavSyncInvalid               = 0x0, // Starting invalid state
      EWFFavSyncStart                 = 0x1, // Start the sync process
      EWFGotLmsFavs                   = 0x2, // Got LMS Favs
      EWFGotClientFavs                = 0x4, // Got Nav2 Favs
      EWFGotServerFavs                = 0x8, // Got Server Favs
      EWFLmsSyncDone                  = 0x10, // LMS - Client sync done
      EWFServerSyncDone               = 0x20, // Server - Client sync done
      EWFGotServerFavsSynced          = 0x40, // Got Server - Nav2 sync done
      EWFNoWFLMStore                  = 0x80, // No WFLMS existed
      EWFWaitingForFavSync            = 0x100, // Internal state
      EWFWaitingForServerFavs         = 0x200, // Internal state
      EWFSyncingLms                   = 0x400, // Internal state
      EWFAddToLmsDone                 = 0x800, // Add to LMS done
      EWFUpdateToLmsDone              = 0x1000, // Update to LMS done
      EWFDeleteFromLmsDone            = 0x2000, // Delete from LMS done
      EWFAddToServerDone              = 0x4000, // Add to Server done
      EWFUpdateToServerDone           = 0x8000, // Update to Server done
      EWFDeleteFromServerDone         = 0x10000, // Delete from Server done
      EWFServerQueueDone              = 0x20000, // All Server messages done
      EWFWaitingForServerFavReplies   = 0x40000, // Internal state
      EWFWaitingForNav2ToServerUpdate = 0x80000, // Internal state
      EWFGotNav2ToServerUpdate        = 0x100000, // Nav2 - Server sync done
      EWFEmptyLmsCheckDone            = 0x200000, // Check for empty lms done
      EWFSyncRunning                  = 0x400000, // Flag for knowing when lms sync is running
   };

   /** 
    * Sets iLMSManager, takes ownership
    *
    * @param aLMSManager, the new iLMSManager
    */ 
   void SetLMSManager(class MWFLMSManagerBase* aLMSManager);


   /**
    * Called by event generator
    *
    * @param aEvent   The event to generate
    */
   void GenerateEvent(enum TWFFavSyncManEvent aEvent);

   /**
    * Callback from event generator
    *
    * @param aEvent   The event to handle
    */
   void HandleGeneratedEventL(enum TWFFavSyncManEvent aEvent);

   /**
    * Starts the process of syncing the LMS - Client - Server.
    */
   void DoSyncL();

   /**
    * Where messages from Nav2 ends up.
    */
   virtual bool GuiProtReceiveMessage(isab::GuiProtMess* mess);

   /**
    * Status report from the MFavoriteSyncManagerObserver.
    */
   virtual void FavoriteComparedL(TInt aNbrCompared, TInt aTotalNbrToCompare);

   /**
    * Status report from the MFavoriteSyncManagerObserver.
    */
   virtual void SyncDoneL(TInt aNbrFavoritesSynced);

   /**
    * Status report from the MFavoriteSyncManagerObserver.
    */
   virtual void SyncError(TInt aError);

   /**
    * Status report from the MWFLMSObserver.
    */
   virtual void LandmarkImportCompleteL();

   /**
    * Status report from the MWFLMSObserver.
    */
   virtual void GuiLandmarkImportCompleteL();

   /**
    * Status report from the MWFLMSObserver.
    */
   virtual void LandmarkImportedL(TInt aLmsNbrImported, 
                                  TInt aTotalNbrLmsToImport,
                                  TInt aLmNbrImported, 
                                  TInt aTotalNbrLmToImport);

   /**
    * Status report from the MWFLMSObserver.
    */
   virtual void LmsError(TInt aError);

   /**
    * Called dby CWFLMSManager when a database has been initialized,
    * if the initialization is done async.
    */
   virtual void DatabaseInitialized();

   /**
    * Checks if a sync is already running.
    * 
    * @return ETrue if running the sync process.
    */
   TBool SyncIsRunning();
   
   /**
    * Convenience function for the state bit map.
    */
   TBool IsWaitingForFavSync();

   /**
    * Convenience function for the state bit map.
    */
   TBool IsWaitingForServerFavReplies();

protected:
   /**
    * Standard ConstrucL function.
    *
    * @param aLmsId      The id of the wayfinder LMS.
    */
   void ConstructL(const TDesC& aLmsId);

   /**
    * Constructor
    *
    * @param aSender     The message sender.
    * @param aImeiCrcHex This phones IMEI in crc and hex format.
    */
   CFavoriteSyncHandler(MFavoriteSyncHandlerObserver& aObserver, 
                        GuiProtMessageSender& aSender,
                        const char* aImeiCrcHex);

   /**
    * The function that advances the state machine, triggers events based 
    * on the state that we're in.
    */
   virtual void ProcessNextState();

   /**
    * Adds synced favorites to the Wayfinder LMS.
    */
   virtual void AddToLmsL();

   /**
    * Updates synced favorites to the Wayfinder LMS.
    */
   virtual void UpdateToLmsL();

   /**
    * Deletes synced favorites from the Wayfinder LMS.
    */
   virtual void DeleteFromLmsL();

   /**
    * Adds synced favorites to the Wayfinder Server.
    */
   virtual void AddToServerL();

   /**
    * Updates synced favorites to the Wayfinder Server.
    */
   virtual void UpdateToServerL();

   /**
    * Deletes synced favorites from the Wayfinder Server.
    */
   virtual void DeleteFromServerL();

   /**
    * Checks if wayfinder lms is empty, if so the lms is removed.
    */
   virtual void CheckEmptyLmsL();


private:
   /// The vector of favorites that the client contained
   std::vector<class Favorite*>* iFavoriteList;

   /// The vector of favorites that the server contained
   std::vector<class Favorite*>* iServerFavList;

   /// The message ids sent to Nav2 to control that all is received
   RArray<TUint> iServerMessageQueue;

   /// The Observer
   MFavoriteSyncHandlerObserver& iObserver;

   /// Message sender
   GuiProtMessageSender& iSender;

   /// The LMS Manager
   class MWFLMSManagerBase* iLMSManager;

   /// The Async sync manager
   class CFavoriteSyncManager* iSyncManager;

   /// Eventgenerator sending and receiving events
   typedef CEventGenerator<CFavoriteSyncHandler, enum TWFFavSyncManEvent> CWFSyncManEventGenerator;

   /// Event generator
   CWFSyncManEventGenerator* iEventGenerator;

   /// WF LMS ID
   HBufC* iLmsId;

   /// The current state
   TUint32 iState;

   /// The IMEI in crc and hex format
   const char* iImeiCrcHex;
};

#endif
