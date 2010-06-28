/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CS_DETAILED_RESULT_CONTAINER_H
#define CS_DETAILED_RESULT_CONTAINER_H

#include "arch.h"
#include <coecntrl.h>		
#include <vector>
#include <map>
#include <eiklbo.h>

#ifdef NAV2_CLIENT_SERIES60_V5
# include <aknlongtapdetector.h>
#endif

#include "GuiProt/SearchItem.h"
#include "STLStrComp.h"
#include "CombinedSearchDataHolder.h"

class CAknDoubleStyleListBox;
class CEikTextListBox;

/**
 * Container class for STestDetailedResultListBox
 * 
 * @class	CCSDetailedResultContainer STestDetailedResultListBox.h
 */
class CCSDetailedResultContainer : public CCoeControl,
                                   public MEikListBoxObserver
#ifdef NAV2_CLIENT_SERIES60_V5
                                 , public MAknLongTapDetectorCallBack
#endif
{
public:
   // constructors and destructor
   CCSDetailedResultContainer(class CCSDetailedResultView* aView, TInt aOffset, TInt aIndex );

   static CCSDetailedResultContainer* NewL( const TRect& aRect, 
                                            class CCSDetailedResultView* aView, 
                                            TInt aOffset,
                                            TInt aIndex);

   static CCSDetailedResultContainer* NewLC( const TRect& aRect, 
                                             class CCSDetailedResultView* aView, 
                                             TInt aOffset,
                                             TInt aIndex);

   void ConstructL( const TRect& aRect );

   virtual ~CCSDetailedResultContainer();

public:
   enum TCSStatus {
      EHasMoreHits,
      ERequestingHitsFromServer,
      ENoMoreHits
   };

   enum TControls {
      EListBox,
      ELastControl
   };

public: // from base class CCoeControl

   /**
    * Return the number of controls in the container (override)
    * @return count
    */
   TInt CountComponentControls() const;

   /**
    * Get the control with the given index (override)
    * @param aIndex Control index [0...n) (limited by #CountComponentControls)
    * @return Pointer to control
    */
   CCoeControl* ComponentControl( TInt aIndex ) const;

   /**
    *	Handle key events.
    */				
   TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, 
                                TEventCode aType );

   /** 
    * Observer function from MEikListBoxObserver, here we handle
    * events as double click etc.
    */ 
   void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aEventType );

   /**
    * Gets called by framework when a tap event has occured.
    * We are notifying the iLongTapDetector (only if the current item
    * is a top hit) that a tap has occured, if it is a long tap 
    * our HandleLongTapEventL will get called by the iLongTapDetector. 
    */
   void HandlePointerEventL( const TPointerEvent& aPointerEvent );

   /**
    * Gets called by CAknLongTapDetector (iLongTapDetector) when
    * a long tap has occured. We then launch a CAknStylusPopUpMenu
    * enabling the user to select navigate to, show information etc.
    */
   void HandleLongTapEventL( const TPoint& aPenEventLocation, 
                             const TPoint& aPenEventScreenLocation );

   /** 
    * Handle global resource changes, such as scalable UI or skin events (override)
    */
   void HandleResourceChange( TInt aType );
	
protected: // from base class CCoeControl

   void SizeChanged();

   /**
    * Needed to draw the background skin.
    */
   TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

private: // from base class CCoeControl

   void Draw( const TRect& aRect ) const;

private:

   /**
    * Creates and initializes all the controls
    * used in this container.
    */
   void InitializeControlsL(const TRect& aRect);

   /**
    * Loads the image (mif) aFileName. Sets size and returns
    * a CGulIcon. If the mif file isnt found, a fallback icon
    * is loaded.
    * @param aFileName, The name of the mif file containing the icon to be loaded.
    * @return The created CGulIcon.
    */
   CGulIcon* LoadAndScaleIconL( const TDesC& aFileName );

   /**
    * Loads and scales the a bitmap and its mask into the parameters.
    */
   void LoadAndScaleImageL(const TDesC& aFileName,
                           CFbsBitmap*& aBitmap,
                           CFbsBitmap*& aMask);

   /**
    * Called from AddResultsL for each result to be displayed
    * in the list. Loads icon from the CSImageCacheConverter
    * and displays this in the list together with the nanme
    * and the parsed information.
    * @param item, The searchItem to be added to the list.
    * @param aImageName, The name of the image to added to the list.
    */
   void AddListBoxItemL( const isab::SearchItem& item, const char* aImageName );

   /**
    * Adds a post to the list. Used when adding the
    * "Show next 25 results" and "Show previous 25 results"
    * @param aResourceHeaderId, The title id, id from the resource.
    * @param aResourceTextId, The text id, id from the resource.
    * @param aImageId, The id of the image to be displayed, MBM index.
    * @param aMaskId, The id of the image mask to be displayed, MBM index.
    */
   void AddListBoxItemL( TInt aResourceHeaderId, TInt aResourceTextId, TInt aImageId, TInt aMaskId );

   /**
    * Adds a post to the list. Used when adding the
    * "Show next 25 results" and "Show previous 25 results"
    * @param aResourceHeaderId, The title id, id from the resource.
    * @param aResourceTextId, The text id, id from the resource.
    * @param aImageMapId, The id in the map holding image names.
    */
   void AddListBoxItemL( TInt aResourceHeaderId, TInt aResourceTextId, TInt aImageMapId );

   /**
    * Checks whether list is displaying the last items in the list
    * and if there might be more items on the server for downloading.
    * @param aCatIndex, The index of the heading/category that is displayed.
    * @param aLowerBound, The index of the first item in this page.
    * @param aUpperBound, The index of the last item in this page.
    * return TCSStatus::EHasMoreHits if there are more hits downloaded.
    *        TCSStatus::ENoMoreHits if there are no more hits to be downloaded.
    *        TCSStatus::ERequestingHitsFromServer if there are no more downloaded hits
    *        to be displayed but there exists more hits on the server.
    */
   TInt LastPageGetMoreHits(TInt aCatIndex, const CSCatVec_t& aCatResults, 
                            TInt& aLowerBound, TInt& aUpperBound);

   /**
    * Compares a string with a resource string. Gets the string from
    * the resource and searches for that string within the aString.
    * @param aString, the string to search in.
    * @param aResourceTextId, the resource id for the string to search for.
    * @return ETrue if the aResourceTextId string was found within aString.
    *         EFalse if not found.
    */
   TBool CompareToResource( const TDesC& aString, TInt aResourceId );

public: 

   /**
    * Called when to populate the listbox with search hits for
    * a certain heading
    * @param aIndex, The index of the chosen heading/category.
    * @param aResults, Vector that contains all search results. Holding
    *                 CombinedSearchCategories which contains search results
    *                 for each heading.
    * @param aMoreServerHitsReceived Flag to know if we got more hits from server.
    */
   TInt AddResultsL(TInt aIndex, 
                    const CSCatVec_t& aResults,
                    TBool aMoreServerHitsReceived = EFalse );

   /**
    * Called when user clicked on "Show next 25 results",
    * simply incremeants iOffset. 
    * iOffset controls what page should be displayed.
    */
   void ShowNextResults();

   /**
    * Called when user clicked on "Show previous 25 results",
    * simply decremeants iOffset. 
    * iOffset controls what page should be displayed.
    */
   void ShowPreviousResults();

   /**
    * Checks if the listbox has any results in it.
    * @return ETrue if the listbox is created and if the listbox contains any matches.
    *         EFalse if any of the above is false.
    */
   TBool HasResults();

   /**
    * Calculates the correct index, not the index in the list, 
    * but the index in the vector, holding all results, of the 
    * current item. With correct index, means:
    * if the offset is 1 and the list index is 2, then the correct
    * index is 25 x 1 + list index, which gives us 27.
    * return The calculated index.
    */
   TInt GetCalculatedIndex();

   /**
    * Returns iOffset and iIndex,
    * @param aOffset, a reference to a variable that will contain iOffsets value.
    * @param aIndex, a reference to a variable that will contain iIndex value.
    */
   void GetIndexes(TInt& aOffset, TInt& aIndex);

   /**
    * Checks if current index is "Previous 25 results".
    * @return ETrue if current item is "Previous 25 results".
    *         EFalse if not.
    */
   TBool IndexIsPrev();

   /**
    * Checks if current index is "Next 25 results".
    * @return ETrue if current item is "Next 25 results".
    *         EFalse if not.
    */
   TBool IndexIsNext();

   /**
    * Set the correct texts and image in the preview pop up.
    */
   void UpdatePreviewContentL();

   /**
    * Show or hide the preview pop up controll.
    */
   void ShowPreviewContent(TBool aShow);

private: 
   /// This containers view
   class CCSDetailedResultView* iView;
   /// The listbox containing the search results
#if defined NAV2_CLIENT_SERIES60_V5
   class CAknSingleGraphicStyleListBox* iListBox;
#else
   class CAknDoubleGraphicStyleListBox* iListBox;
#endif
   /// Map that holds an image name mapped against a index
   /// in the icon list.
   std::map<HBufC*, TInt, strCompareLess> iImageMap;
   /// Array containing all loaded icons to be displayed in the list
   CArrayPtr<CGulIcon>* iIcons;
   /// Array holding all the text to be displayed in the list 
   CDesCArray* iList;
   /// The offset in the list, represents current page
   TInt iOffset;
   /// Current index in the list
   TInt iIndex;
   /// Total number of search hits for his search
   TInt iTotalNbrHits;
   /// Keeps track of how many hits to display on each page
   TInt iMaxHitsPerPage;

#if defined NAV2_CLIENT_SERIES60_V5
   /// The content shown in the popup controller
   class CPreviewPopUpContent* iPreviewContent;
   /// The popup controller displayed at the top of the listbox
   class CAknPreviewPopUpController* iPreviewController;
   /// Handles longtap events, calls HandleLongTapEventL when a longtap event is detected
   class CAknLongTapDetector* iLongTapDetector;
   /// Handles the context sensitiv menu that will be launched on longtap detection
   class CAknStylusPopUpMenu* iPopUpMenu;
#endif

   /// Bool flag for knowing when the popup menu has been launched.
   TBool iPopUpMenuDisplayed;

   /// To be able to draw a skinned background
   class CAknsBasicBackgroundControlContext* iBgContext;
};
				
#endif // STESTDETAILEDRESULTLISTBOX_H
