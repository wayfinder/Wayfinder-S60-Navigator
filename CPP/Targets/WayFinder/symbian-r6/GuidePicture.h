/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GUIDEPICTURE_H
#define GUIDEPICTURE_H

// INCLUDES
#include <coecntrl.h>  // for coe control
#include <mdaimageconverter.h>

#include "WayFinderConstants.h"
#include "Log.h"

class PictureContainer;


// CLASS DECLARATION

/**
 *  CGuidePicture  container control class.
 *  
 */
class CGuidePicture : public CCoeControl,
                      public MMdaImageUtilObserver
{
public: // Constructors and destructor

   /**
    * Constructor.
    */
   CGuidePicture(isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CGuidePicture();

   void ConstructL( const TRect& aBitMapRect,
                    PictureContainer* aContainer );

public: // New functions
	
   /** Valid angles to rotate image */
   enum TImageRotateAngle
   {
      E0Degrees      = 0,
      E360Degrees    = 0,
      ENoRotation    = 0,
      E90Degrees     = 1,
      E180Degrees    = 2,
      E270Degrees    = 3
   };

   enum TGuidePictureType
   {
      ENone             = 0,
      EGifFile          = 1,
      EGifFileWithMask  = 2,
      EBitmap           = 3,
      EBitmapWithMask   = 4,
      EGifDesc          = 5,
      EGifDescWithMask  = 6,
   };

   void SetPriority(CActive::TPriority aPriority);

   void SetPictureContainer( PictureContainer* aContainer );

   void SetBitmap( CWsBitmap &aBitmap );
   
   void OpenBitmapFile( const TDesC &aBitMapFile );
   void OpenMaskBitmap( const TDesC &aBitMapMaskFile );

   void OpenBitmapFromMbm( const TDesC &aMbmFile, TInt aId );
   void OpenBitmapMaskFromMbm( const TDesC &aMbmFile, TInt aMaskId );

   void OpenGif( const TDesC &aGifFile, TBool aDoScale = ETrue );
   void OpenDesc( HBufC8* aImage );
   void OpenGifMask( const TDesC &aGifMaskFile );

   void CloseImage();

   void SetShow( TBool aShow );

   /**
    * Inherited from CoeControl.
    */
   virtual void MakeVisible(TBool aVisible);

   void SetClear( TBool aClear );

   void SetBitMapRect( TRect aRect );

   void SetScreenOffset( TInt aX, TInt aY );
   
   TBool RotateImageClockwiseL( TImageRotateAngle aAngle );

   TBool IsReady();

public: // Functions from base classes

   //from MMdaImageUtilObserver

   /**
    * @function MiuoOpenComplete
  
    * @discussion Called when the gif file has been opened
    * @param aError an error code or KErrNone if process was successful
    */
   virtual void MiuoOpenComplete(TInt aError);

   /**
    * @function MiuoConvertComplete
  
    * @discussion Called when any conversion process performed on the bitmap is complete
    * @param aError an error code or KErrNone if process was successful
    */
   virtual void MiuoConvertComplete(TInt aError);

   /**
    * @function MiuoCreateComplete
  
    * @discussion Called when the file in which the bitmap is to be saved has been created 
    * @param aError an error code or KErrNone if process was successful
    */
   virtual void MiuoCreateComplete(TInt aError);

   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

   
private: // New functions

   /**
    * @function ConvertingFromGifFinished
  
    * @discussion  Called when the the file has been converted to a GIF
    * @param aError an error code or KErrNone if process was successful  
    */
   void ConvertingFromGifFinished(TInt aError);

   /**
    * @function ScalingFinished
  
    * @discussion  Called when scaling has completed
    * @param aError an error code or KErrNone if process was successful  
    */
   void ScalingFinished(TInt aError);

   /**
    * @function RotationFinished
  
    * @discussion  Called when a rotation has completed
    * @param aError an error code or KErrNone if process was successful  
    */
   void RotationFinished(TInt aError);

private: // Functions from base classes

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

private: //data

   /// Keeps track of which stage the open/conversion/creating process is at
   enum TConvertState
   {
      EConvertStateNull,
      EConvertStateConvertingFromGif,
      EConvertStateScaling,
      EConvertStateRotating,
      EConvertStateReady
   };

   /// Converts the gif file to a bitmap
   CMdaImageFileToBitmapUtility* iConverter;

   /// Converts a descriptor to a bitmap
   CMdaImageDescToBitmapUtility* iDescConverter;
   
   /// Scales the bitmap
   CMdaBitmapScaler* iScaler;

   /// Rotates the bitmap
   CMdaBitmapRotator* iRotator;

   /// The bitmap being displayed
   CFbsBitmap* iBitmap;
   CFbsBitmap* iMaskBitmap;

   /// The format to be loaded (i.e. gif)
   TMdaBmpClipFormat iClipFormat;

   /// The codec used to perform the conversion
   TMda24BppBmpCodec iCodec;

   /// Keeps track of which stage the open/conversion/creating process is at
   TConvertState iConvertState;

   TBuf<KBuf64Length> iCurrentFile;

   /// The pictures rect
   TRect iBitMapRect;

   /// The offset off the screen
   TPoint iOffset;

   /// True if the picture is visible.
   TBool iShow;

   /// True if the bitmap rect should be cleared before drawing
   TBool iClear;
   
   /// The container displaying the picture
   PictureContainer* iContainer;

   TGuidePictureType iImageType;

   CMdaBitmapRotator::TRotationAngle iLastRotation;

   TBool iRotateMask;
   TBool iDoScale;

   isab::Log* iLog;
};

#endif

// End of File
