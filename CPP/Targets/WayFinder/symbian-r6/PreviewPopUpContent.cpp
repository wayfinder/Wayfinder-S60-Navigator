/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#if defined NAV2_CLIENT_SERIES60_V5

// INCLUDE FILES
#include <coemain.h>
#include <aknutils.h>
#include <eiklabel.h>
#include <aknsutils.h> 
#include <gdi.h>
#include <eikapp.h>
#include <gulicon.h>

#include "WFBitmapUtil.h"
#include "PreviewPopUpContent.h"

_LIT(KDefaultText, "");

CPreviewPopUpContent* CPreviewPopUpContent::NewL()
{
   CPreviewPopUpContent* self = 
      CPreviewPopUpContent::NewLC();
   CleanupStack::Pop(self);
   return self;
}

CPreviewPopUpContent* CPreviewPopUpContent::NewLC()
{
   CPreviewPopUpContent* self = new (ELeave) CPreviewPopUpContent();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

void CPreviewPopUpContent::ConstructL()
{
   /* Do not call CreateWindowL() as the parent CAknPreviewPopUpController has a 
      window. But when ConstructL() is called this has not yet been created (as 
      the CAknPreviewPopUpController has not been created) so defer all construction
      which requires a window to InitialiseL() which is called after 
      CAknPreviewPopUpController has been constructed. */
}
   
void CPreviewPopUpContent::InitialiseL(const TRect& aRect,
                                      const TDesC& aMbmName,
                                      TInt aNbrOfRows, 
                                      TInt aImageId,
                                      TInt aMaskId)
{
   // Do not call CreateWindowL() as parent CAknPreviewPopUpController owns window
   
   InitComponentArrayL();
   
   iMbmName = aMbmName.AllocL();

   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   TRgb fgcolor(0,0,0);
   AknsUtils::GetCachedColor(skin, fgcolor, KAknsIIDQsnTextColors, 
                             EAknsCIQsnTextColorsCG55);

   if (aImageId != -1 && aMaskId != -1) {
      AknIconUtils::CreateIconL(iBitmap, iMask, *iMbmName, aImageId, aMaskId);
      AknIconUtils::SetSize(iBitmap, TSize(10, 10), 
                            EAspectRatioPreservedAndUnusedSpaceRemoved);
   }

   iStringLengths = new (ELeave) CArrayFixFlat<TInt>(aNbrOfRows);
   iLabelContainer.Reset();
 
   // Create one label with standard font size.
   iStringLengths->AppendL(aRect.Width());
   CEikLabel* label = new (ELeave) CEikLabel();
   label->SetContainerWindowL(*this);
   Components().AppendLC(label);
   label->OverrideColorL(EColorLabelText, fgcolor);
   label->SetTextL(KDefaultText);
   CleanupStack::Pop(label);
   iLabelContainer.AppendL(TLabelData(label));
   // The rest of the labels with a smaller font.
   const CFont* font = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);
   for (TInt i = 0; i < aNbrOfRows-1; ++i) {
      iStringLengths->AppendL(aRect.Width());
      CEikLabel* label = new (ELeave) CEikLabel();
      label->SetContainerWindowL(*this);
      Components().AppendLC(label);
      label->SetFont(font);
      label->OverrideColorL(EColorLabelText, fgcolor);
      label->SetTextL(KDefaultText);
      CleanupStack::Pop(label);
      iLabelContainer.AppendL(TLabelData(label));
   }
   //CEikonEnv::Static()->ScreenDevice()->ReleaseFont(font);
   Components().SetControlsOwnedExternally(EFalse);

   // Set the windows size
   SetRect(aRect);

   // Activate the window, which makes it ready to be drawn
   ActivateL();
}

CPreviewPopUpContent::CPreviewPopUpContent() : 
   iBitmap(NULL),
   iMask(NULL)
{
}

CPreviewPopUpContent::~CPreviewPopUpContent()
{
   delete iMbmName;
   delete iBitmap;
   delete iMask;
   if (iStringLengths) {
      iStringLengths->Reset();
      delete iStringLengths;
   }
   iLabelContainer.Reset();
}

void 
CPreviewPopUpContent::SetAvailableWidth(TInt aWidth, TInt aPadding, 
                                        CPreviewPopUpContent::TContentLayout aLayout,
                                        TInt aHeight)
{
   iLayout = aLayout;

   TInt numLabels = iLabelContainer.Count();
   if (numLabels == 0) {
      return;
   }
   
   // The large font header label.
   CEikLabel* labelLa = iLabelContainer[0].iLabel;
   // The small font text label.
   CEikLabel* labelSm = iLabelContainer[1].iLabel;

   // Font height in pixels
   TInt fontHeightLa = labelLa->Font()->HeightInPixels();
   // Font descent in pixels
   TInt descentLa = labelLa->Font()->DescentInPixels();
   // The height of one label
   TInt labelHeightLa = fontHeightLa + descentLa;
   // Font height in pixels
   TInt fontHeightSm = labelSm->Font()->HeightInPixels();
   // Font descent in pixels
   TInt descentSm = labelSm->Font()->DescentInPixels();
   // The height of one label
   TInt labelHeightSm = fontHeightSm + descentSm;

   // totLabelsHeight will hold the total height of all labels that 
   // will be drawn (one large font and the rest small fonts).
   // We additionaly remove a descent on the last line since that 
   // it seems otherwise we're over compensating.
   TInt totLabelsHeight = 
      labelHeightLa + labelHeightSm * (numLabels - 1); // - descentSm;

   // Calculate the image size.
   // This calc will give us the height of two label rows.
   iIconSize = fontHeightLa + fontHeightSm + descentLa;
   TInt iconSize = iIconSize;
   TInt padding = aPadding;
   if (iBitmap) {
      AknIconUtils::SetSize(iBitmap, TSize(iconSize, iconSize), 
                            EAspectRatioPreservedAndUnusedSpaceRemoved);
      iconSize = iBitmap->SizeInPixels().iWidth;
   } else {
      // We have no bitmap set so dont indent the text around the bitmap.
      iconSize = 0;
      padding = 0;
   }

   // controlRect will contain the rect for our control to draw in, 
   // including padding.
   TRect controlRect;
   if (iLayout == EFourLinesIndentedTextImageTopLeft) {
      if (aHeight > 0) {
         controlRect = TRect(TPoint(aPadding, aPadding), 
                             TSize(aWidth, aHeight));
      } else {
         controlRect = TRect(TPoint(aPadding, aPadding), 
                             TSize(aWidth, totLabelsHeight + aPadding));
      }
   } else if (iLayout == EFourLinesTextImageTopLeftAbove) {
      if (aHeight > 0) {
         controlRect = TRect(TPoint(aPadding, aPadding), 
                             TSize(aWidth, aHeight));
      } else {
         controlRect = TRect(TPoint(aPadding, aPadding), 
                             TSize(aWidth, 
                                   totLabelsHeight + iconSize + descentLa + aPadding));
      }
   }

   // iComponentRect will contain the drawing area for our controls 
   // (image and labels).
   iComponentRect = controlRect;
   iComponentRect.Shrink(aPadding, aPadding);

   // Calculate where the text should wrap and
   // calculate the positions and rects for the labels in the control.
   TRect labelRectLa;
   TRect labelRectSm;
   if (iLayout == EFourLinesIndentedTextImageTopLeft) {
      // The first line will bew positioned next to the image and 
      // therefore the width will be a bit shorter.
      iStringLengths->At(0) = (iComponentRect.Width() - (iconSize + padding));
      // The width of the last line should not be wrapped but rather set to 
      // cropped later on.
      iStringLengths->At(1) = (iComponentRect.Width() + 10000);

      // The rect for the first label in the layout mode.
      labelRectLa = TRect(TPoint(iComponentRect.iTl.iX + iconSize + padding, 
                               iComponentRect.iTl.iY),
                          TSize(iComponentRect.Width() - (iconSize + padding), 
                                labelHeightLa));
      // The rect for the labels with small font as well.
      labelRectSm = TRect(TPoint(iComponentRect.iTl.iX + iconSize + padding, 
                               iComponentRect.iTl.iY),
                          TSize(iComponentRect.Width() - (iconSize + padding), 
                                labelHeightLa));
   } else if (iLayout == EFourLinesTextImageTopLeftAbove) {
      // We can use the full length of the container.
      iStringLengths->At(0) = iComponentRect.Width();
      // The width of the last line should not be wrapped but rather set to 
      // cropped later on.
      iStringLengths->At(1) = (iComponentRect.Width() + 10000);

      // The rect for the first label in the layout mode.
      labelRectLa = TRect(TPoint(iComponentRect.iTl.iX, 
                               iComponentRect.iTl.iY + iconSize + descentLa),
                          TSize(iComponentRect.Width(), 
                                labelHeightLa));
      // The rect for the labels with small font as well.
      labelRectSm = TRect(TPoint(iComponentRect.iTl.iX, 
                               iComponentRect.iTl.iY + iconSize + descentLa),
                          TSize(iComponentRect.Width(), 
                                labelHeightLa));
   }

   iLabelContainer[0].SetRect(labelRectLa);
   
   labelRectSm.Move(0, labelHeightLa);
   iLabelContainer[1].SetRect(labelRectSm);

//    if (iLayout == EFourLinesIndentedTextImageTopLeft) {
//       // We need to move back the third label in this layout mode.
//       labelRect.iTl.iX -= (iconSize + padding);
//    }
   labelRectSm.Move(0, labelHeightSm);
   iLabelContainer[2].SetRect(labelRectSm);

   labelRectSm.Move(0, labelHeightSm);
   iLabelContainer[3].SetRect(labelRectSm);
   
   SetRect(controlRect);
}

void CPreviewPopUpContent::SizeChanged()
{

}

void CPreviewPopUpContent::Draw(const TRect& /*aRect*/) const
{
   // Get the standard graphics context
   CWindowGc& gc = SystemGc();
   TRect rect(Rect());
   gc.SetClippingRect(rect);
   
   //gc.DrawRect(rect);
   
   if (iBitmap && iMask) {
      gc.BitBltMasked(iComponentRect.iTl, iBitmap, 
                      TRect(TPoint(0, 0), iBitmap->SizeInPixels()), 
                      iMask, EFalse);
   }
}

void CPreviewPopUpContent::SetTextL(const TDesC& aFirstText, 
                                    const TDesC& aSecondText, 
                                    const TDesC& aThirdText)
{
   TBuf<256> wrappedText;
   AknTextUtils::WrapToStringL(aSecondText, *iStringLengths, 
                               *iLabelContainer[1].iLabel->Font(), 
                               wrappedText);
   
   iLabelContainer[0].iLabel->SetTextL(aFirstText);
   iLabelContainer[0].iLabel->CropText();

   _LIT(KNewLine, "\n");
   TInt pos = wrappedText.Find(KNewLine);
   if (pos != KErrNotFound) {
      TPtrC leftText = wrappedText.Left(pos);
      iLabelContainer[1].iLabel->SetTextL(leftText);
      TPtrC rightText = wrappedText.Mid(pos + 1);
      if (rightText.Length() < 1) {
         // WrapToStringL always seems to add a newline so we need to 
         // check for length as well.
         iLabelContainer[2].iLabel->SetTextL(aThirdText);
         iLabelContainer[2].iLabel->CropText();
         iLabelContainer[3].iLabel->SetTextL(KDefaultText);
      } else {
         iLabelContainer[2].iLabel->SetTextL(rightText);
         iLabelContainer[2].iLabel->CropText();
         iLabelContainer[3].iLabel->SetTextL(aThirdText);
         iLabelContainer[3].iLabel->CropText();
      }
   } else {
      // WrapToStringL didnt add a newline (we dont need to wrap) but it
      // seems WrapToStringL always adds newline (see above comment).
      iLabelContainer[1].iLabel->SetTextL(aSecondText);
      iLabelContainer[2].iLabel->SetTextL(aThirdText);
      iLabelContainer[2].iLabel->CropText();
      iLabelContainer[3].iLabel->SetTextL(KDefaultText);      
   }
   //iLabel->SetRect(iLabelRect);
   //iLabel->DrawDeferred();
}

void CPreviewPopUpContent::SetImageL(TInt aImageId, TInt aMaskId)
{
   delete iBitmap;
   iBitmap = NULL;
   delete iMask;
   iMask = NULL;

   if (aImageId != -1 && aMaskId != -1) {
      AknIconUtils::CreateIconL(iBitmap, iMask, *iMbmName, aImageId, aMaskId);
      AknIconUtils::SetSize(iBitmap, TSize(iIconSize, iIconSize), 
                            EAspectRatioPreservedAndUnusedSpaceRemoved);
   }
}

void CPreviewPopUpContent::SetImageL(CFbsBitmap* aBitmap, CFbsBitmap* aMask)
{
   delete iBitmap;
   iBitmap = NULL;
   delete iMask;
   iMask = NULL;

   if (aBitmap && aMask) {
      iBitmap = aBitmap;
      iMask = aMask;
      AknIconUtils::SetSize(iBitmap, TSize(iIconSize, iIconSize), 
                            EAspectRatioPreservedAndUnusedSpaceRemoved);
   }
}

#endif //NAV2_CLIENT_SERIES60_V5
