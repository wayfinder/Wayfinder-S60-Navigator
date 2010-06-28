/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _PREVIEW_POP_UP_CONTENT_H_
#define _PREVIEW_POP_UP_CONTENT_H_

#include <coecntrl.h>

class CPreviewPopUpContent : public CCoeControl
{
public:
   
   /**
    * Struct that contains information about the labels in the control.
    */
   struct TLabelData {
      /// The label for this struct, its rect is controlled by iRect
      CEikLabel* iLabel;
      /// The rect of the iLabel in this struct
      TRect iRect;

      /**
       * Default constructor, sets iLabel to NULL and iRects
       * top left and bottom right corners to (0,0). 
       */
      TLabelData() :
         iLabel(NULL), iRect() {}
      
      /**
       * Constructor, sets iLabel to point to aLabe. Note that
       * the struct takes ownership of aLabel.
       */
      TLabelData(CEikLabel* aLabel, TRect aRect) :
         iLabel(aLabel), iRect(aRect) {}

      /**
       * Constructor, sets iLabel to point to aLabe. Note that
       * the struct takes ownership of aLabel.
       */
      TLabelData(CEikLabel* aLabel ) :
         iLabel(aLabel), iRect() {}

      /**
       * Sets the rect for the struct and the label.
       */
      void SetRect(TRect aRect) {
         iLabel->SetRect(aRect);
         iRect = aRect;
      }
   };

   enum TContentLayout {
      ENoLayout = 0,
      EFourLinesIndentedTextImageTopLeft,
      EFourLinesTextImageTopLeftAbove
   };

   /**
    * NewL.
    * Two-phased constructor.
    * Create a CPreviewPopUpContent object, which will draw itself to aRect.
    * @param aRect The rectangle this view will be drawn to.
    * @return a pointer to the created instance of CPreviewPopUpContent.
    */
   static CPreviewPopUpContent* NewL();

   /**
    * NewLC.
    * Two-phased constructor.
    * Create a CPreviewPopUpContent object, which will draw itself
    * to aRect.
    * @param aRect Rectangle this view will be drawn to.
    * @return A pointer to the created instance of CPreviewPopUpContent.
    */
   static CPreviewPopUpContent* NewLC();

   /**
    * ~CPreviewPopUpContent
    * Virtual Destructor.
    */
   virtual ~CPreviewPopUpContent();

protected:

   /**
    * CPreviewPopUpContent.
    * C++ default constructor.
    */
   CPreviewPopUpContent();

private: 

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CPreviewPopUpContent object.
    *
    */
   void ConstructL();

private: // Functions from base classes
   
   /**
    * From CCoeControl, Draw
    * Draw this CPreviewPopUpContent to the screen.
    * If the user has given a text, it is also printed to the center of
    * the screen.
    * @param aRect the rectangle of this view that needs updating
    */
   virtual void Draw(const TRect& aRect) const;
   
protected: // Functions from base classes

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();
   
public: // Public functions

    /* InitialiseL
    * deferred construction
    * Perform the second phase construction of a
    * CPreviewPopUpContent object.
    *
    * @param aRect The initial rect for the content.
    */
   void InitialiseL(const TRect& aRect,
                   const TDesC& aMbmName,
                   TInt aNbrOfRows, 
                   TInt aImageId,
                   TInt aMaskId);

   /**
    * @param aHeight   Make sure its at least larger than the content (labels
    *                  and image).
    */
   void SetAvailableWidth(TInt aWidth, TInt aPadding, 
                          CPreviewPopUpContent::TContentLayout aLayout,
                          TInt aHeight = -1);

   void SetTextL(const TDesC& aFirstText, 
                 const TDesC& aSecondText, 
                 const TDesC& aThirdText);
   
   void SetImageL(TInt aImageId, TInt aMaskId);

   void SetImageL(CFbsBitmap* aBitmap, CFbsBitmap* aMask);

private: // Private members

   HBufC* iMbmName;

   RArray<TLabelData> iLabelContainer;
   CArrayFix<TInt>* iStringLengths;
   
   CFbsBitmap* iBitmap;
   CFbsBitmap* iMask; 
   TInt iIconSize;

   TRect iComponentRect;

   CPreviewPopUpContent::TContentLayout iLayout;
};

#endif // _PREVIEW_POP_UP_CONTAINER_H_
