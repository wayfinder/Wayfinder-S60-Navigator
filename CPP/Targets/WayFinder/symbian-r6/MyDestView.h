/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYDESTVIEW_H
#define MYDESTVIEW_H

// INCLUDES
#include "ViewBase.h"

#include "GuiProt/Favorite.h"
#include "WFTextUtil.h"

/* For RApaLsSession */
#include <apgcli.h>
#if defined NAV2_CLIENT_SERIES60_V5
# include <akntoolbarobserver.h>
#endif 
// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CMyDestContainer;

namespace isab{
   class Favorite;
   class GuiFavorite;
   class GetFavoritesReplyMess;
   class GetFavoritesAllDataReplyMess;
   class GetFavoriteInfoReplyMess;
   class FullSearchDataReplyMess;
   class AdditionalInfo;
}

// CLASS DECLARATION

/**
 *  CMyDestView view class.
 * 
 */
class CMyDestView : public CViewBase
#if defined NAV2_CLIENT_SERIES60_V5
                    , public MAknToolbarObserver
#endif
{
public: // Constructors and destructor

   CMyDestView(class CWayFinderAppUi* aUi, class isab::Log* aLog);
   /**
    * EPOC default constructor.
    */
   void ConstructL();

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMyDestView object.
   ///        Note that this object is still on the CleanupStack.
   static class CMyDestView* NewLC(class CWayFinderAppUi* aUi, class isab::Log* aLog);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMyDestView object.
   static class CMyDestView* NewL(class CWayFinderAppUi* aUi, class isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CMyDestView();

public: // New Functions

   /**
    * Hack to make sure the findbox/selectlist does not crash with 
    * eikcoctl 63 the second time you set focus to true or offer 
    * any key event to the find box in a multiview application.
    * Needs to be called before making a view change.
    */
   void SetFindBoxFocus(TBool aFocus);

   /**
    * Handle a favorites reply.
    */
   void HandleFavoritesReply( isab::GetFavoritesReplyMess* aMessage );

   void HandleLmsImportReplyL(std::vector<isab::GuiFavorite*>* aLmsList);

   /**
    * Handle a favorites reply.
    */
   void HandleFavoritesAllDataReply( isab::GetFavoritesAllDataReplyMess* aMessage );

   /**
    * Handle a favorites reply.
    */
   void HandleFavoriteInfoReply( isab::GetFavoriteInfoReplyMess* aMessage );
  
   /**
    * Route to a favorites.
    */
   void Route();

   /**
    * Add a favorite.
    */
   void AddFavorite(TInt32 aLat, TInt32 aLon,
         const TDesC& aName, const TDesC& aDescription = KNullDesC);

   /**
    * Add a favorite.
    */
   void AddFavoriteD( isab::Favorite* fav );

   /**
    * Add a favorite using the current position.
    */
   void AddFavorite();
  
   /**
    * Checks if the selected item is a favorite or if it is a
    * landmark not from our lms. If it is an landmark ShowEditFavoriteDialogL
    * will be launched and the changes will be updated in both the lms and to 
    * Nav2. If the selected item is a favorite EditFavorite will be called
    * resulting in a reply from Nav2 containing all information about the favorite.
    * The ShowEditFavoriteDialogL will then be launched from HandleFavoriteInfoReply
    * and the changes will be stored in the lms and in Nav2. 
    */
   void UpdateFavoriteL();

   /**
    * Remove a favorites.
    */
   void RemoveFavorite();

   /**
    * Send favorite to someone using communicate dialog.
    * @param favorite The favorite to send.
    */
   void SendFavoriteToFriend(const isab::Favorite* favorite);

   /**
    * Set state booleans to false.
    */
   void SetStatesFalse();

   /**
    * Sets the oritin, calls WayfinderAppUi that calls
    * route planner view with the supplied variables.
    *
    * @param aLat, the latitude of the new origin. 
    * @param aLon, the longitude of the new origin. 
    */
   void SetAsOrigin( TInt32 aLat, TInt32 aLon );

   /**
    * Sets the destination, calls WayfinderAppUi that calls
    * route planner view with the supplied variables.
    *
    * @param aLat, the latitude of the new destination. 
    * @param aLon, the longitude of the new destination. 
    */
   void SetAsDestination( TInt32 aLat, TInt32 aLon );

   /**
    * Displayes information about the favorite in service window.
    * 
    * @param aFavorite, the favorite to be displayed in the service window.
    */
   void ShowInfoInServiceWindow(const isab::Favorite& aFavorite);

   /**
    * Displayes the favorite in the map.
    * 
    * @param aFavorite, the favorite to be displayed in the map.
    */
   void ShowInfoInMap(const isab::Favorite& aFavorite);

   /**
    * Send favorite to someone using communicate dialog.
    *
    * @param aFavorite, the favorite to send to a friend.
    */
   void SendToFriend(const isab::Favorite& aFavorite);

   TBool SettingOrigin();
   
   TBool SettingDestination();

   TBool IsEditing();

   TBool IsAdding();

   void ShowConfirmationDialogL();

   TBool IsGpsAllowed();

   TBool ValidGpsStrength();

   TPoint GetCurrentPosition();

   TInt GetDistanceMode();

   // Used by MyDestContainer.cpp
   CWayFinderAppUi* GetWayFinderUI() { return iWayfinderAppUi; }

   /**
    * Checks if current app type is iron or not
    * 
    * @return ETrue if iron version
    */
   TBool IsIronVersion();

   /**
    * Calls wayfinderappuis member iPathFinder for receiving
    * the name and path to the mbm file.
    * @return The path to the mbm file.
    */
   TPtrC GetMbmName();

   /**
    * Returns the favorite id for the selected favorite.
    *
    * @return The id of the selected favorite.
    */
   TUint GetSelectedId();

   /**
    * Returns the landmark id for the selected favorite.
    *
    * @return The lm id of the selected favorite.
    */
   TUint GetSelectedLmId();

   /**
    * Returns the currently selected gui favorite.
    */
   isab::GuiFavorite* GetSelectedGuiFav();

   /**
    * A static wrapper as a callback to be able to call the real 
    * NavigateToCallback() function from appui
    * by a function pointer to this wrapper. This is used in the gps wizard 
    * to be able to know if/when lbs connected to the gps so it is okay
    * to order the route.
    * This is necessary since we have different objects with different 
    * member functions that can call the gps wizard.
    */
   static void WrapperToNavigateToCallback(void* pt2Object);

private: // New Functions
   /**
    * Merge lms/nav2 vectors together into the iFavVec member.
    * @param aAddVec   The vector to be merged with iMergeVec into iFavVec.
    */
   void MergeNav2LmsFavVectors(std::vector<isab::GuiFavorite*>* aAddVec);

   isab::Favorite* CreateFavorite( TUint aId, TInt aLat, TInt aLon,
                                   TDesC &aName, TDesC &aDescription,
                                   TDesC &aCategory, TDesC &aAlias,
                                   const char* aIcon, TBool aHasId, uint32 aLmID, 
                                   const char* aImeiCrcHex, const char* aLmsID );

   /**
    * Sends a request to Nav2 for information about a specific favorite.
    */
   void EditFavorite();

   /**
    * Displays information about a supplied favorite in the edit
    * favorites dialog. The changes are returned back in a newly
    * created favorite.
    *
    * @param aFav, the favorite to edit.
    * @return A newly created favorite containg the changes made.
    */
   isab::Favorite* ShowEditFavoriteDialogL(isab::Favorite* aFav);

   /**
    * Sorts the iFavVec and iterates through the list and adds
    * one item at the time to the listbox.
    */
   void AddFavoritesToListL();

   /**
    * Checks if the current selected item is a wayfinder favorite or
    * if it is a landmark from a lms other than the wayfinder lms.
    * If it is a landmark, we will fetch the information from the lms
    * manager and perfom the desired operation based on what flag that 
    * is set.
    * If it is a wayfinder favorite, we will request information about
    * the favorite from Nav2 and in HandleFavoriteInfoReply we will perform
    * the desired operation based on what flag that is set. 
    */
   void HandleFavoriteOperationsL();

   /**
    * Removes an item from the listbox and from the iFavVec.
    * 
    * @param aIndex, the index to be removed.
    */
   void RemoveItemL(TInt aIndex);

   /**
    * NavigateToCallback() function from called form appui via the 
    * static WrapperToNavigateToCallback() function.
    */
   void NavigateToCallback();

   /**
    * Sets up the toolbar. Creates buttons and adds to
    * the toolbar if not already added before.
    */
   void InitToolbarL();

public: // Functions from base classes

   /**
    * Called when receiving an event from the toolbar.
    *
    * @param aCommandId, the id of the button that was pressed.
    */
   void OfferToolbarEventL(TInt aCommmandId);

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
                     const TDesC8& aCustomMessage);

   /**
    * From AknView, Called when the view is activated.
    * @param aPrevViewId the id of the previous view.
    * @param aCustomMessageId id of a message sent to the view.
    * @param aCustomMessage a message sent to the view.
    */
   void DoDeactivate();

private: // Data

   /// The controls container.
   CMyDestContainer* iContainer;

   /// True if a favorite is being saved
   TBool iSaving;

   /// True if a favorite is being edited
   TBool iEditing;

   /// True if a favorite is being removed
   TBool iRemoving;

   TBool iIsOrigin;

   /// True if a returned favorite is to be displayed in a map.
   TBool iShowInMap;

   TBool iSetOrigin;

   TBool iSetDestination;

   // If to show favorite
   TBool iShowFavorite;

   /// True if a returned favorite is to be sent to a friend.
   TBool iSendToFriend;

   /// True of we have gotten favorites from the lms
   TBool iGotLmsFavs;

   /// True of we have gotten favorites from nav2
   TBool iGotNav2Favs;

   /// Phone call
   RApaLsSession iApaLsSession;

   isab::Log* iLog;

   /// List holding all gui favorites
   std::vector<isab::GuiFavorite*>* iFavVec;

   std::vector<isab::GuiFavorite*>* iMergeVec;

   /// Holding the the selected index from the list, used
   /// when the listbox and the container is deleted, e.g
   /// when the this view isnt the active view.
   TInt iListIndex;
};

/**
 * This class to compare favorites is necessary up here in the gui to 
 * be able to sort the vector of favorites after importing them all from 
 * LMS and Nav2.
 */
class UiGuiFavoriteCmp
{
public:
   /**
    * The comparison function. Implements (first < second).
    * @param first The left side part of the less-than comparison.
    * @param second The right side part of the less-than comparison.
    * @return true if first < second.
    */
   bool operator()(const class isab::GuiFavorite* first,
                   const class isab::GuiFavorite* second) const {
      TBuf<KBuf256Length> firstStr;
      TBuf<KBuf256Length> secondStr;
      WFTextUtil::char2TDes(firstStr, first->getName());
      WFTextUtil::char2TDes(secondStr, second->getName());
      return (firstStr.CompareC(secondStr) < 0);
   }
};

#endif

// End of File
