/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _BACKBROUND_TEXT_CONTAINER_H_
#define _BACKBROUND_TEXT_CONTAINER_H_

#include <coecntrl.h>

/**
 * This class prints a label with the given text centered within
 * its given rect. If the background should be transparent an
 * alpha value should be set in the background color and the
 * control should have a parent, this control should not be
 * window owning in this case.
 */
class CBackGroundTextContainer : public CCoeControl
{
public:
   enum TEdges {
      ELeftEdge = 0x1,
      ERightEdge = 0x2,
      ETopEdge = 0x4,
      EBottomEdge = 0x8
   };
public:
   /**
    * NewL.
    * Two-phased constructor.
    * Create a CBackGroundTextContainer object, which will draw itself to 
    * aRect.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aRect The rectangle this view will be drawn to.
    * @param aText The text that should be displayed in the label.
    * @param aBgColor The background color of this control.
    * @param aNightModeBgColor The background color of this control during nightmode.
    * @oaram aTextColor, The color of the text in the label.
    * @oaram aNightModeTextColor, The color of the text in the label during nightmode.
    */
    static CBackGroundTextContainer* NewLC(CCoeControl* aParent,
                                           const TRect& aRect,
                                           const TDesC& aText,
                                           const TRgb& aBgColor,
                                           const TRgb& aNightModeBgColor,
                                           const TRgb& aTextColor,
                                           const TRgb& aNightModeTextColor);

   /**
    * NewL.
    * Two-phased constructor.
    * Create a CBackGroundTextContainer object, which will draw itself to 
    * aRect.
    *
    * @param aParent The parent of this control if not window owning.
    * @param aRect The rectangle this view will be drawn to.
    * @param aText The text that should be displayed in the label.
    * @param aBgColor The background color of this control.
    * @param aNightModeBgColor The background color of this control during nightmode.
    * @param aTextColor, The color of the text in the label.
    * @param aNightModeTextColor, The color of the text in the label during nightmode.
    */
    static CBackGroundTextContainer* NewL(CCoeControl* aParent,
                                          const TRect& aRect,
                                          const TDesC& aText,
                                          const TRgb& aBgColor,
                                          const TRgb& aNightModeBgColor,
                                          const TRgb& aTextColor,
                                          const TRgb& aNightModeTextColor);

   /**
    * ~CBackGroundTextContainer
    * Virtual Destructor.
    */
   virtual ~CBackGroundTextContainer();

public: // New functions
   
   /**
    * Getter for the text of iLabel.
    */
   TPtrC Text() const;

   /**
    * Sets the text of iLabel to a new value.
    * 
    * @param aText, the new text to be displayed in the container.
    */
   void SetTextL(const TDesC& aText);

   /**
    * Sets which edges that shall be drawn outlining the rect.
    * @param aBorderEdgesFlags a combination of TEdges values that tells which borders that will be drawn.
    */
   void SetBorderEdges(TInt aBorderEdgesFlags);

   /**
    * Turns the night mode on or off.
    * @param aNightMode ETrue to turn the nightmode on, EFalse for off
    **/
   void SetNightModeL(TBool aNightMode);

protected:

   /**
    * CBackGroundTextContainer.
    * C++ default constructor.
    * @param aBgColor The background color of this control.
    * @param aNightModeBgColor The background color of this control during nightmode.
    */   
   CBackGroundTextContainer(const TRgb& aBgColor, const TRgb& aNightModeBgColor);

protected: // Functions from base classes

   /**
    * From CoeControl, SizeChanged.
    * Called by framework when the view size is changed.
    */
   virtual void SizeChanged();

   /**
    * Requests a font from the CWsScreenDevice server with
    * the given font height.
    *
    * @param aMaxFontHeight, the maximum height of the font.
    * @return CFont, the requested font. The caller has to
    *                release the font from the CWsScreenDevice
    *                by a call to CWsScreenDevice::ReleaseFont.
    */ 
   class CFont* GetFontL(TInt aMaxFontHeight);

private:

   /**
    * ConstructL
    * 2nd phase constructor.
    * Perform the second phase construction of a
    * CBackGroundTextContainer object.
    *
    * @param aRect The rectangle this view will be drawn to.
    * @param aText The text that should be displayed in the label.
    * @param aTextColor, The color of the text in the label.
    * @param aNightModeTextColor, The color of the text in the label during nightmode.
    */
   void ConstructL(CCoeControl* aParent,
                   const TRect& aRect,
                   const TDesC& aText,
                   const TRgb& aTextColor,
                   const TRgb& aNightModeTextColor);

private: // Functions from base classes

   /**
    * From CoeControl, Draw.
    * Called by framework when a redraw has been triggered.
    */
   void Draw(const TRect& aRect) const;

   /**
    * Re-Aligns the text when the text has changed or when the size has changed.
    * It makes it centered if it fits, otherwise left aligned.
    */
   void ReAlignText();
private:
   
   /// Color of the background.
   TRgb iBgColor;
   /// Color of the background during nightmode.
   TRgb iNightModeBgColor;
   /// Label containing the text to be drawn.
   class CDrawComponent* iLabel;
   /// Color of the text.
   TRgb iTextColor;
   /// Color of the text during nightmode.
   TRgb iNightModeTextColor;
   /// Tells if we are in nightmode or not.
   TBool iNightMode;
   /// Which borders of the backgroundtextcontainer that will be drawn.
   TInt iDrawBorderEdgesFlag;
};
#endif
