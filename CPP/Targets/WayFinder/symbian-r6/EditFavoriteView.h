/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef EDIT_FAVORITE_VIEW_H
#define EDIT_FAVORITE_VIEW_H

#include <aknview.h>

#include "WayFinderConstants.h"
#include "ViewBase.h"

namespace isab {
   class Favorite;
}

/**
 *
 */
class CEditFavoriteView : public CViewBase
{
public:
   /**
    * Symbian two-phase constructor.
    * This creates an instance then calls the second-phase constructor
    * without leaving the instance on the cleanup stack.
    * @return new instance of CEditFavoriteView
    */
   static CEditFavoriteView* NewL(class CWayFinderAppUi* aAppUi);

   /**
    * Symbian two-phase constructor.
    * This creates an instance, pushes it on the cleanup stack,
    * then calls the second-phase constructor.
    * @return new instance of CEditFavoriteView
    */
   static CEditFavoriteView* NewLC(class CWayFinderAppUi* aAppUi);

   /**
    * The view's destructor removes the container from the control
    * stack and destroys it.
    */
   virtual ~CEditFavoriteView();

protected:
   /**
    * First phase of Symbian two-phase construction. Should not contain any
    * code that could leave.
    */
   CEditFavoriteView(class CWayFinderAppUi* aAppUi);

private:
   /**
    * Second-phase constructor for view.  
    * Initialize contents from resource.
    */
   void ConstructL();

public:
   /**
    * @return The UID for this view
    */
   TUid Id() const;

   /**
    * Handle a command for this view (override).
    * @param aCommand Command id to be handled
    */
   void HandleCommandL( TInt aCommand );

protected:
   /**
    * Handles user actions during activation of the view, 
    * such as initializing the content.
    */
   void DoActivateL(const TVwsViewId& aPrevViewId,
                    TUid aCustomMessageId,
                    const TDesC8& aCustomMessage);

   /**
    * Handles user de-activation of the view.
    */
   void DoDeactivate();

public:
   /**
    *
    */
   void SetFavoriteDataL(class Favorite* aFav);

   /**
    *
    */
   TBool WaitingForFavoriteReply();

   /**
    *
    */
   void FavoriteReplyGot();

   /**
    *
    */
   void ShowConfirmationDialogL();

   /**
    * Returns true if the guiFavorite is a wayfinder favorite.
    */
   static TBool IsWayfinderFavorite(isab::GuiFavorite* aFav);

   /**
    * Returns true if the Favorite is a wayfinder favorite.
    */
   static TBool IsWayfinderFavorite(isab::Favorite* aFav);

private:
   /**
    *
    */
   void UpdateFavoriteL();

   /**
    *
    */
   void AddFavoriteL();

private:

   /// The edit favorite (form) container
   class CEditFavoriteContainer* iContainer;

   /// Local copy of the favorite to be edited.
   class isab::Favorite* iFav;

   /// Flag to know if we have sent a change/add favorite mess to Nav2.
   TBool iWaitingForFavoriteReply;
};

#endif // EDIT_FAVORITE_VIEW_H
