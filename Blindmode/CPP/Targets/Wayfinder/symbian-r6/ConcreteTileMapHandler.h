/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef CONCRETETILEMAPHANDLER_H
#define CONCRETETILEMAPHANDLER_H

// INCLUDES
#include <aknview.h>
#include <aknwaitdialog.h>
#include <badesca.h>
//#include "MapContainer.h"
#include "WayFinderConstants.h" 
#include "Log.h"

#include "MapEnums.h"
#include "RouteEnums.h"
#include <deque>
#include <vector>

#include "EventGenerator.h"
#include "TileMapEventListener.h"
#include "TurnPictures.h"
#include "MapFileFinder.h"
#include "MapViewEvent.h"
#include "WayfinderCommandCallback.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CWayFinderAppUi;
class CTileMapControl;

// CLASS DECLARATION

/**
 *  CConcreteTileMapHandler view class.
 * 
 */
class CConcreteTileMapHandler :
   public TileMapEventListener
{
public: // Constructors and destructor

   /**
    * EPOC default constructor.
    */
   void 
	ConstructL(CWayFinderAppUi * aWayFinderUI);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///left there when this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   ///        Note that this object is still on the CleanupStack.
   static class CConcreteTileMapHandler * 
	NewLC(CWayFinderAppUi * aUi);

   ///Factory method. Creates a new object and calls its ConstructL method.
   ///The object is pushed onto the CleanupStack before ConstructL and is
   ///popped before this function returns.
   ///@param aUi pointer to the applications single CWayFinderAppUi object.
   ///@param aLog pointer to the memory log Log object. May be NULL.
   ///@return a new, completely constructed, CMapView object.
   static class CConcreteTileMapHandler * 
	NewL(CWayFinderAppUi * aUi);

public: // Functions from base classes

   void 
	handleTileMapEvent(const class TileMapEvent &event);

public:
   /// A pointer to the creating UI
   CWayFinderAppUi*   iWayFinderUI;

   //MapLib::CacheInfo * * iCacheInfo;
   typedef const class MapLib::CacheInfo* const_cacheinfo_p;
   typedef const const_cacheinfo_p* const_cacheinfo_p_p;
   const_cacheinfo_p_p iCacheInfo;

	TInt * iNbrCacheFiles;
private: // Data

   CTileMapControl* iMapControl;
};

#endif

// End of File
