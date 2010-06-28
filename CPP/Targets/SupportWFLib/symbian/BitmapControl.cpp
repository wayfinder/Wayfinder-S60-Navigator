/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "BitmapControl.h"
#include "WFBitmapUtil.h"

CBitmapControl* CBitmapControl::NewLC( CCoeControl* parent,
                                       const TPoint& topLeft,
                                       CFbsBitmap* img,
                                       CFbsBitmap* mask,
                                       TBool maskInvert,
                                       TBool ownBitmaps,
                                       TBool ownWindow )
{
   CBitmapControl* self = new (ELeave) CBitmapControl();
   CleanupStack::PushL(self);   
   self->ConstructL(topLeft, parent, img, mask, maskInvert, ownBitmaps,
                    ownWindow );
   return self;
}

CBitmapControl* CBitmapControl::NewL( CCoeControl* parent,
                                      const TPoint& topLeft,
                                      CFbsBitmap* img,
                                      CFbsBitmap* mask,
                                      TBool maskInvert,
                                      TBool ownBitmaps,
                                      TBool ownWindow )
{
   CBitmapControl* self =
         CBitmapControl::NewLC(parent, topLeft, img, mask, maskInvert,
                               ownBitmaps, ownWindow );
   CleanupStack::Pop(self);
   return self;
}

CBitmapControl::CBitmapControl()
{
}

void CBitmapControl::ConstructL( const TPoint& topLeft,
                                 CCoeControl* parent,
                                 CFbsBitmap* img,
                                 CFbsBitmap* mask,
                                 TBool maskInvert,
                                 TBool ownBitmaps,
                                 TBool ownWindow )
{   
   m_ownWindow = ownWindow;
   if ( m_ownWindow ) {
      // Create a window to live in.
      CreateWindowL(parent);
   } else {
      // Set the container window instead.
      SetContainerWindowL(*parent);
   }   
   m_imgSize = TSize(0,0);
   if (img) {
      m_imgSize = img->SizeInPixels();
   }
   SetRect( TRect(topLeft, m_imgSize) );
   m_image = img;
   m_mask = mask;
   m_maskInvert = maskInvert;
   m_topLeft = topLeft;
   m_ownBitmaps = ownBitmaps;
   m_nightMode = EFalse;
   m_nightModeAlreadyOn = EFalse;
   m_invertRed = EFalse;
   m_buImage = NULL;
}

CBitmapControl::~CBitmapControl()
{
   if ( m_ownBitmaps ) {
      if( m_image ){
         m_image->Reset();
         delete m_image;
         m_image = NULL;
      }
      if( m_mask ){
         m_mask->Reset();
         delete m_mask;
         m_mask = NULL;
      }
      if( m_buImage ) {
         m_buImage->Reset();
         delete m_buImage;
         m_buImage = NULL;
      }
   }
}


void CBitmapControl::SetBitmap(const TPoint& topLeft,
                               CFbsBitmap* img,
                               CFbsBitmap* mask,
                               TBool maskInvert,
                               TBool ownBitmaps )
{
   if ( m_ownBitmaps ) {
      if (m_image) {
         m_image->Reset();
         delete m_image;
         m_image = NULL;
      }
      if (m_mask) {
         m_mask->Reset();
         delete m_mask;
         m_mask = NULL;
      }
   }
   if (!Rect().IsEmpty()) {
      Window().Invalidate( Rect() );
   }
   m_imgSize = TSize(0,0);
   if (img) {
      m_imgSize = img->SizeInPixels();
   }
   SetRect( TRect(topLeft, m_imgSize) );
   m_topLeft = topLeft;
   m_image = img;
   m_mask = mask;
   m_maskInvert = maskInvert;
   m_ownBitmaps = ownBitmaps;
   if( m_nightMode ){
      // re set night mode to make sure correct image is filtered.
      if( m_buImage ) {
         m_buImage->Reset();
         delete m_buImage;
         m_buImage = NULL;
      }
      m_nightModeAlreadyOn = EFalse;
      SetNightModeL( ETrue, m_invertRed );
   }
   // Get a redraw.
   if (img) {
      Window().Invalidate( Rect() );
   }
}

void CBitmapControl::SetNightModeL( TBool aOn, TBool aInvertRed )
{
   // Protect the bitmap from several consecutive
   // SetNightModeL calls.
   m_nightMode = aOn;
   if( m_image /*&& m_nightMode*/ ) {//m_nightMode != aOn ){
      m_invertRed = aInvertRed;
      CFbsBitmap* tmpImage = m_image;
      if( aOn && !m_nightModeAlreadyOn ){
         m_nightModeAlreadyOn = ETrue;
         // Create a copy of the bitmap
#if 1//NAV2_CLIENT_SERIES60_V3
         m_buImage = WFBitmapUtil::CopyBitmapL( m_image );
#else
         m_buImage = new (ELeave) CFbsBitmap();
         WFBitmapUtil::CopyBitmapL( m_buImage, m_image );
#endif         
         // Move the copy to the original
         m_image = m_buImage;
         // Save the original.
         m_buImage = tmpImage;
         // Filter the copy. If the original is filtered all
         // instances of the bitmap from the MBM id is filtered.
         WFBitmapUtil::FilterImage( m_image, aInvertRed );
      } else if (!aOn) {
         m_nightModeAlreadyOn = EFalse;
         if (m_buImage) {
            // we actually have made a copy earlier
            m_image = m_buImage;
            // Release the filtered image.
            tmpImage->Reset();
            delete tmpImage;
            tmpImage = NULL;
            m_buImage = NULL;
         }
      }
      m_imgSize = m_image->SizeInPixels();
   }
}

void CBitmapControl::SetPos(const TPoint& topLeft)
{
   m_topLeft = topLeft;
   SetRect(TRect(m_topLeft, m_imgSize));
   Window().Invalidate(Rect());
}

void CBitmapControl::Draw( const TRect& /*rect*/ ) const
{
   if( m_image &&
       m_imgSize.iHeight > 0 &&
       m_imgSize.iWidth > 0 ){
      // If the window is owned by the control the coordinates are
      // the same for the control as for the window.
      TPoint topLeft = m_topLeft;

      CBitmapContext* gc;
      if( iGc ){
         gc = iGc;
      }
      else{
         // If the window is owned by the control the coordinates are
         // the same for the control as for the window.
         topLeft = m_ownWindow ? TPoint(0,0) : m_topLeft;
         gc = STATIC_CAST( CWindowGc*, &SystemGc() );
      }
      gc->SetBrushStyle(CGraphicsContext::ENullBrush);
      
      if( m_mask &&
          m_mask->SizeInPixels().iHeight > 0 &&
          m_mask->SizeInPixels().iWidth > 0 ){
         gc->BitBltMasked( topLeft,
                           m_image,
                           m_image->SizeInPixels(),
                           m_mask, m_maskInvert );
      }
      else{
         gc->BitBlt( topLeft,
                     m_image,
                     m_image->SizeInPixels() );
      }
   }
}

void CBitmapControl::ChangeGc( CBitmapContext* aGc )
{
   iGc = aGc;
}

#if defined NAV2_CLIENT_SERIES60_V3
void CBitmapControl::SetSizeAndPos(const TRect& aRect, TScaleMode aScaleMode)
{
   if (m_image) {
      AknIconUtils::SetSize(m_image, aRect.Size(), aScaleMode);
      m_topLeft = aRect.iTl;
      SetRect(TRect(m_topLeft, m_image->SizeInPixels()));
   } else {
      m_topLeft = aRect.iTl;
      SetRect(aRect);
   }
}
#endif
