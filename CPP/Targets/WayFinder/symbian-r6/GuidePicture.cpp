/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include <eikenv.h>

#include "PictureContainer.h"
#include "GuidePicture.h"
#include "memlog.h"
static const TDisplayMode KDeviceColourDepth = EColor4K;
//gif file frame index (in this case there is only one frame)
static const TInt KGifFrameIndex = 0;

//new size to scale image
static const TInt KNewImageWidth = 50;
static const TInt KNewImageHeight = 50;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CGuidePicture::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//

// Constructor
CGuidePicture::CGuidePicture(isab::Log* aLog) : 
   iConvertState(EConvertStateNull), iLog(aLog)
{
}

// ---------------------------------------------------------
// CGuidePicture::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CGuidePicture::ConstructL( const TRect& aBitMapRect, 
                                PictureContainer* aContainer )
{
   iBitmap = new (ELeave) CFbsBitmap();
   LOGNEW(iBitmap, CFbsBitmap);

   // Null the iMaskBitmap
   iMaskBitmap = NULL;

   iScaler = CMdaBitmapScaler::NewL();
   LOGNEW(iScaler, CMdaBitmapScaler);

   iConverter = CMdaImageFileToBitmapUtility::NewL(*this);
   LOGNEW(iConverter, CMdaImageFileToBitmapUtility);
   iConverter->SetPriority(CActive::EPriorityStandard);

   iDescConverter = CMdaImageDescToBitmapUtility::NewL(*this);
   LOGNEW(iDescConverter, CMdaImageDescToBitmapUtility);
   iDescConverter->SetPriority(CActive::EPriorityStandard);

   iRotator = CMdaBitmapRotator::NewL();
   LOGNEW(iRotator, CMdaBitmapRotator);

   iBitMapRect = aBitMapRect;

   iContainer = aContainer;

   iShow = ETrue;

   iClear = ETrue;

   iOffset = TPoint(0,0);

   iRotateMask = EFalse;

   iDoScale = ETrue;
   
   SetRect(aBitMapRect);
}

// Destructor
CGuidePicture::~CGuidePicture()
{
   iConverter->Close();
   iDescConverter->Close();

   LOGDEL(iScaler);
   delete iScaler;
   iScaler = NULL;

   LOGDEL(iConverter);
   delete iConverter;
   iConverter = NULL;

   LOGDEL(iDescConverter);
   delete iDescConverter;
   iDescConverter = NULL;

   LOGDEL(iRotator);
   delete iRotator;
   iRotator = NULL;

   LOGDEL(iBitmap);
   delete iBitmap;
   iBitmap = NULL;

   LOGDEL(iMaskBitmap);
   delete iMaskBitmap;
   iMaskBitmap = NULL;
}

void CGuidePicture::SetPictureContainer( PictureContainer* aContainer )
{
   iContainer = aContainer;
}

void
CGuidePicture::SetPriority(CActive::TPriority aPriority)
{
   if (iConverter) {
      iConverter->SetPriority(aPriority);
   }
   if (iDescConverter) {
      iDescConverter->SetPriority(aPriority);
   }
}

void
CGuidePicture::SetBitmap( CWsBitmap &/*aBitmap*/ )
{
}
void
CGuidePicture::OpenBitmapFile( const TDesC &/*aBitMapFile*/ )
{
}
void
CGuidePicture::OpenMaskBitmap( const TDesC &/*aBitMapMaskFile*/ )
{
}
void
CGuidePicture::OpenBitmapFromMbm( const TDesC &aMbmFile, TInt aId )
{
   /* Load the image from the MBM file. */
   LOGDEL(iBitmap);
   delete iBitmap;
   iBitmap = CEikonEnv::Static()->CreateBitmapL(aMbmFile, aId);
   LOGNEW(iBitmap, CFbsBitmap);
   iImageType = EBitmap;
   ConvertingFromGifFinished( KErrNone );
}
void
CGuidePicture::OpenBitmapMaskFromMbm( const TDesC &aMbmFile, TInt aMaskId )
{
   /* Load the image from the MBM file. */
   LOGDEL( iMaskBitmap );
   delete iMaskBitmap;
   iMaskBitmap = CEikonEnv::Static()->CreateBitmapL(aMbmFile, aMaskId);
   LOGNEW(iMaskBitmap, CFbsBitmap);
   iImageType = EBitmapWithMask;
   ConvertingFromGifFinished( KErrNone );
}
void
CGuidePicture::OpenGif( const TDesC &aGifFile, TBool aDoScale )
{
   iImageType = EGifFile;
   iDoScale = aDoScale;
   iConverter->Close();
   iConverter->OpenL( aGifFile );
}
void
CGuidePicture::OpenDesc( HBufC8* aImage )
{
   iImageType = EGifDesc;

   iDescConverter->Close();
   TRAPD( convertErr, iDescConverter->OpenL( *aImage ));

   //Trap error as this function cannot leave
   if( convertErr != KErrNone ){
      if( iContainer ) {
         iContainer->PictureError( convertErr );
      }
   }
}

void
CGuidePicture::OpenGifMask( const TDesC &/*aGifMaskFile*/ )
{
}


void CGuidePicture::CloseImage()
{
   iConverter->Close();
}

void CGuidePicture::SetShow( TBool aShow )
{
   iShow = aShow;
}

void CGuidePicture::MakeVisible(TBool aVisible)
{
   iShow = aVisible;
}

void CGuidePicture::SetClear( TBool aClear )
{
   iClear = aClear;
}

void CGuidePicture::SetBitMapRect( TRect aRect )
{
   iBitMapRect = aRect;
}

void CGuidePicture::SetScreenOffset( TInt aX, TInt aY )
{
   iOffset.iX = aX;
   iOffset.iY = aY;
}


TBool CGuidePicture::RotateImageClockwiseL( TImageRotateAngle aAngle )
{
   if (iConvertState != EConvertStateReady){
      //Inform the UI that the gif file is still being loaded
      return EFalse;
   }
   
   switch (aAngle)
   {
   case E90Degrees:
      //Start an asynchronous process to rotate the bitmap
      iLastRotation = CMdaBitmapRotator::ERotation90DegreesClockwise;
      break;
   case E180Degrees:
      //Start an asynchronous process to rotate the bitmap
      iLastRotation = CMdaBitmapRotator::ERotation180DegreesClockwise;
      break;
   case E270Degrees:
      //Start an asynchronous process to rotate the bitmap
      iLastRotation = CMdaBitmapRotator::ERotation270DegreesClockwise;
      break;
   default:
      return EFalse;
   }
   iRotator->RotateL(*this,*iBitmap,iLastRotation);
   iConvertState = EConvertStateRotating;
   iRotateMask = EFalse;
   return ETrue;
}

TBool CGuidePicture::IsReady()
{
   return iConvertState == EConvertStateReady;
}

// This function is called when the gif file has been opened
// or an error has occured in the process
void CGuidePicture::MiuoOpenComplete(TInt aError)
{
   if (aError == KErrNone){
      iConvertState = EConvertStateConvertingFromGif;

      if (iImageType == EGifFile || iImageType == EGifFileWithMask) {

         TFrameInfo frameInfo;
         //Get the frame info
         iConverter->FrameInfo( KGifFrameIndex, frameInfo );

         // Free the bitmap from the Fb-server.
         iBitmap->Reset();
         // Create a bitmap based on the size of the gif
         TInt err = iBitmap->Create( frameInfo.iOverallSizeInPixels, 
               CCoeEnv::Static()->ScreenDevice()->DisplayMode());
         if( err == KErrCouldNotConnect || err == KErrArgument ){
            if( iContainer )
               iContainer->PictureError( err );
            return;
         }

         //Convert the gif into a bitmap
         TRAPD( convertErr, iConverter->ConvertL(*iBitmap,KGifFrameIndex) );

         //Trap error as this function cannot leave
         if( convertErr != KErrNone ){
            if( iContainer ) {
               iContainer->PictureError( convertErr );
            }
         }
      } else if (iImageType == EGifDesc || iImageType == EGifDescWithMask) {
         TFrameInfo frameInfo;
         //Get the frame info
         iDescConverter->FrameInfo( KGifFrameIndex, frameInfo );

         //Create a bitmap based on the size of the gif
         iBitmap->Reset();
         TInt err = iBitmap->Create( frameInfo.iOverallSizeInPixels,
               CCoeEnv::Static()->ScreenDevice()->DisplayMode() );
         if( err == KErrCouldNotConnect || err == KErrArgument ){
            if( iContainer )
               iContainer->PictureError( err );
            return;
         }

         //Convert the gif into a bitmap
         TRAPD( convertErr, iDescConverter->ConvertL(*iBitmap,KGifFrameIndex) );

         //Trap error as this function cannot leave
         if( convertErr != KErrNone ){
            if( iContainer ) {
               iContainer->PictureError( convertErr );
            }
         }
      }
   } else {
      if( iContainer )
         iContainer->PictureError( aError );
   }
}

//This function is called whenever a conversion process has finished
void CGuidePicture::MiuoConvertComplete(TInt aError)
{
   switch (iConvertState)
   {
   //Finished converting gif file to bitmap
   case EConvertStateConvertingFromGif:
      ConvertingFromGifFinished( aError );
      break;
   //Finished Scaling file
   case EConvertStateScaling:
      ScalingFinished( aError );
      break;
   //Finished Rotating file
   case EConvertStateRotating:
      RotationFinished( aError );
      break;
   case EConvertStateNull:
   default:
      ASSERT(FALSE);
   }
}

//This function is called when the bitmap (output) file has been created
void CGuidePicture::MiuoCreateComplete(TInt aError)
{
   if (aError != KErrNone){
      //Reset state so that other operations can still be performed
      iConvertState = EConvertStateReady;
      if( iContainer )
         iContainer->PictureError( aError );
   }
}

//This function is called when conversion has finished
void CGuidePicture::ConvertingFromGifFinished(TInt aError)
{
   if (aError == KErrNone){
      if (iDoScale && iBitMapRect.Size() != iBitmap->SizeInPixels()) {
         TRAPD( err,iScaler->ScaleL( *this, *iBitmap, TSize( iBitMapRect.Width(), iBitMapRect.Height() ) ) );
         if (err == KErrNone){
            iConvertState = EConvertStateScaling;
         } else {
            if( iContainer ) {
               iContainer->PictureError( err );
            }
         }
      } else {
         iConvertState = EConvertStateScaling;
         /* Shortcut the scaling. */
         MiuoConvertComplete(aError);
      }
   } else {
      if( iContainer )
         iContainer->PictureError( aError );
   }
}

//This function is called when scaling has finished
void CGuidePicture::ScalingFinished(TInt aError)
{
   if (aError == KErrNone){
      iConvertState = EConvertStateReady;
      if( iContainer )
         iContainer->ScalingDone();
   }
   else{
      if( iContainer )
         iContainer->PictureError( aError );
   }
}

//This function is called when rotation has finished
void CGuidePicture::RotationFinished(TInt aError)
{
   if (aError == KErrNone){
      if (!iRotateMask &&
            (iImageType == EBitmapWithMask ||
             iImageType == EGifFileWithMask)) {
         /* Rotate mask the same amount. */
         iRotator->RotateL(*this,*iMaskBitmap,iLastRotation);
         iRotateMask = ETrue;
      } else {
         iConvertState = EConvertStateReady;
         if( iContainer )
            iContainer->ScalingDone();
      }
   }
   else{
      if( iContainer )
         iContainer->PictureError( aError );
   }
}

// ---------------------------------------------------------
// CGuidePicture::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CGuidePicture::SizeChanged()
{
}

// ---------------------------------------------------------
// CGuidePicture::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CGuidePicture::Draw(const TRect& /*aRect*/) const
{
   if( iShow ){
      CWindowGc& gc = SystemGc();

      // Calculate where the bitmap really is positioned.
      TRect drawRect = iBitMapRect;
      drawRect.Move( iOffset );

      if( iClear ){
         // clear the screen
         gc.Clear( drawRect );
      }

      TPoint topLeft = drawRect.iTl;;
      if (!iDoScale) {
         // If we doesnt want to scale the image to fit the screen,
         // we center it and paints the background.
         if (drawRect.Width() > iBitmap->SizeInPixels().iWidth) {
            topLeft.iX = (drawRect.Width() - iBitmap->SizeInPixels().iWidth) >> 1;
         }
         if (drawRect.Height() > iBitmap->SizeInPixels().iHeight) {
            topLeft.iY = (drawRect.Height() - iBitmap->SizeInPixels().iHeight) >> 1;
         }
      }

      //if the bitmap is not currently being processed in any way
      if (iConvertState == EConvertStateReady ){
         gc.SetBrushStyle(CGraphicsContext::ENullBrush);
         switch (iImageType) {
            case EGifDesc:
            case EGifFile:
            case EBitmap:
               gc.BitBlt( topLeft, iBitmap );
               break;
            case EGifFileWithMask:
            case EGifDescWithMask:
            case EBitmapWithMask:              
               gc.BitBltMasked( topLeft, iBitmap,
                                //iBitMapRect.Size(),
                                iBitmap->SizeInPixels(),
                                iMaskBitmap, ETrue );
               break;
            default:
               gc.BitBlt( topLeft, iBitmap );
               break;
         }
      } else {
         gc.SetPenStyle(CGraphicsContext::ENullPen);
         
#ifdef DEBUG_GUIDE_PICTURE
         switch (iConvertState) {
            case EConvertStateReady:
               gc.SetBrushColor( TRgb( 0xff, 0x0, 0x0) );
               break;
            case EConvertStateNull:
               gc.SetBrushColor( TRgb( 0x0, 0x0, 0x0) );
               break;
            case EConvertStateConvertingFromGif:
               gc.SetBrushColor( TRgb( 0x0, 0xff, 0x0) );
               break;
            case EConvertStateScaling:
               gc.SetBrushColor( TRgb( 0xff, 0x0, 0xff) );
               break;
            case EConvertStateRotating:
               gc.SetBrushColor( TRgb( 0xff, 0x0, 0xff) );
               break;
            default:
               gc.SetBrushColor( TRgb (0xff, 0xff, 0xff));

         }
#else
//          gc.SetBrushColor(
//             TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
// XXX We should not need to set any color here but use the calling
// view or containers background color as is. XXX
//         gc.SetBrushColor(iAppUi->GetNewsBgColor());
#endif
         
         gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
         gc.DrawRect(drawRect);
      }
   }
}

// End of File  
