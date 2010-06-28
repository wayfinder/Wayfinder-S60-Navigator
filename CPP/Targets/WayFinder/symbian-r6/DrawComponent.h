/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _C_DRAW_COMPONENT_H_
#define _C_DRAW_COMPONENT_H_

#include <e32base.h>
#include <coecntrl.h>
#include <e32cmn.h>
#include <gdi.h>

class CDrawComponent : public CCoeControl 
{
public:

   /**
    * NewLC.
    * Two-phased constructor.
    * Creates a CDrawComponent object, which will draw itself to 
    * aRect.
    *
    * @param aParent         The parent of this control if not window owning.
    * @param aDrawRect           The rect of this control.
    * @param aTextColor      The pen color when drawing the text.
    * @param aBaselineOffset An offset from the top of the rectangle 
    *                        to the text baseline.
    * @param aText           The text that should be drawn to the window.
    * @param aAlign          The way to align the text within its rect. Could be
    *                        ECenter, ERight or ELeft
    */
   static CDrawComponent* NewLC(CCoeControl* aParent,
                                TRect aDrawRect,
                                const TRgb& aTextColor,
                                TInt aBaselineOffset,
                                const TDesC& aText,
                                CGraphicsContext::TTextAlign aAlign);

   /**
    * NewL.
    * Two-phased constructor.
    * Creates a CDrawComponent object, which will draw itself to 
    * aRect.
    *
    * @param aParent         The parent of this control if not window owning.
    * @param aDrawRect           The rect of this control.
    * @param aTextColor      The pen color when drawing the text.
    * @param aBaselineOffset An offset from the top of the rectangle 
    *                        to the text baseline.
    * @param aText           The text that should be drawn to the window.
    * @param aAlign          The way to align the text within its rect. Could be
    *                        ECenter, ERight or ELeft
    */
   static CDrawComponent* NewL(CCoeControl* aParent,
                               TRect aDrawRect,
                               const TRgb& aTextColor,
                               TInt aBaselineOffset,
                               const TDesC& aText,
                               CGraphicsContext::TTextAlign aAlign);


   /**
    * NewL.
    * Two-phased constructor.
    * Creates a CDrawComponent object, which will draw itself to 
    * aRect.
    *
    * @param aParent         The parent of this control if not window owning.
    * @param aTextColor      The pen color when drawing the text.
    * @param aAlign          The way to align the text within its rect. Could be
    *                        ECenter, ERight or ELeft
    */
   static CDrawComponent* NewL(CCoeControl* aParent,
                               const TRgb& aTextColor,
                               CGraphicsContext::TTextAlign aAlign);

   /**
    * NewL.
    * Two-phased constructor.
    * Creates a CDrawComponent object, which will draw itself to 
    * aRect.
    *
    * @param aParent         The parent of this control if not window owning.
    * @param aTextColor      The pen color when drawing the text.
    * @param aAlign          The way to align the text within its rect. Could be
    *                        ECenter, ERight or ELeft
    */
   static CDrawComponent* NewLC(CCoeControl* aParent,
                                const TRgb& aTextColor,
                                CGraphicsContext::TTextAlign aAlign);

   /**
    * ~CDrawComponent
    * Virtual Destructor.
    */
   virtual ~CDrawComponent();

public:

   /**
    * Getter for iBaselineOffset.
    */ 
   TInt BaselineOffset() const;

   /**
    * Sets the baseline of this control
    * 
    * @param aBaselineOffset, the baseline to set.
    */ 
   void SetBaselineOffset(TInt aBaselineOffset);

   /**
    * Getter for iText.
    */ 
   TPtrC Text() const;

   /**
    * Sets the text of this control
    * 
    * @param aText, the text to set.
    */ 
   void SetTextL(const TDesC& aText);

   /**
    * Getter for iFont.
    */ 
   const CFont* Font() const;

   /**
    * Sets the font of this control
    * 
    * @param aFont, the font to set.
    * @param aReleaseCurrentFont, Should be true if current iFont should
    *                             be released from the window server.
    */ 
   void SetFont(CFont* aFont, TBool aReleaseCurrentFont = EFalse);

   /**
    * Releases iFont from the WindowServer.
    */
   void ReleaseFont();

   /**
    * Sets the textcolor.
    * @param aTextColor The color that will be used to draw the text.
    */
   void SetTextColor(TRgb aTextColor);

   /**
    * Changes the alignment for the component
    * @param aAlign the alignment
    */
   void SetAlignment(CGraphicsContext::TTextAlign aAlign);
protected:

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CDrawComponent object.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aDrawRect   The rect of this control.
    * @param aText       The text that should be drawn to the window.
    */
   void ConstructL(CCoeControl* aParent,
                   TRect& aDrawRect,
                   const TDesC& aText);

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CDrawComponent object.
    *
    * @param aParent The parent of this control if not window owning.
    */
   void ConstructL(CCoeControl* aParent);

protected: // Functions from base classes

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();

private:
   
   /**
    * CDrawComponent.
    * C++ default constructor.
    * @param aTextColor The background color of this control.
    * @param aAlign The way to align the text within its rect. Could be
    *               ECenter, ERight or ELeft
    */   
   CDrawComponent(const TRgb& aTextColor,
                  CGraphicsContext::TTextAlign aAlign);

   /**
    * CDrawComponent.
    * C++ default constructor.
    * @param aTextColor The background color of this control.
    * @param aBaselineOffset An offset from the top of the rectangle 
    *                        to the text baseline.
    * @param aAlign The way to align the text within its rect. Could be
    *               ECenter, ERight or ELeft
    */   
   CDrawComponent(TInt aBaselineOffset,
                  const TRgb& aTextColor,
                  CGraphicsContext::TTextAlign aAlign);

private: // Functions from base classes

   /**
    * From CoeControl, Draw.
    * Called by framework when a redraw has been triggered.
    */
   void Draw(const TRect& aRect) const;

private:
   /// An offset from the top of the rect to the text baseline.
   TInt iBaselineOffset;
   /// The text that will be drawn to the window.
   HBufC* iText;    
   /// The font to use when drawing the text.
   class CFont* iFont; 
   /// The text color to use when drawing the text.
   TRgb iTextColor;
   CGraphicsContext::TTextAlign iAlign;
};
#endif
