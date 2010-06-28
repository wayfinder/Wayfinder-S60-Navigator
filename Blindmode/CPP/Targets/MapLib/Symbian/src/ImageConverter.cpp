/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ImageConverter.h"
#include <imageconversion.h>

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

class CImageConverter* CImageConverter::NewLC(class CImage& aPicture, 
                                              class RFs aFs, 
                                              const TDesC& aFile,
                                              enum TPriority aPriority)
{
   class CImageConverter* self = new (ELeave) CImageConverter(aPicture, aPriority);
   CleanupStack::PushL(self);
   self->ConstructL(aFs, aFile);
   return self;
}

class CImageConverter* CImageConverter::NewL(class CImage& aPicture, 
                           class RFs aFs, 
                           const TDesC& aFile,
                            enum TPriority aPriority)
{
   class CImageConverter* self = CImageConverter::NewLC(aPicture, aFs, aFile, aPriority);
   CleanupStack::Pop();
   return self;
}


CImageConverter::CImageConverter(class CImage& aPicture, 
                                 enum TPriority aPriority) : 
   CActive(aPriority), iPicture(aPicture), iDecoder(NULL)
{
   CActiveScheduler::Add(this);
}

void CImageConverter::ConstructL(RFs aFs, const TDesC& aFile)
{

   iDecoder = CImageDecoder::FileNewL(aFs, aFile);
   const TFrameInfo& info = iDecoder->FrameInfo();

   iBmpRotator = CBitmapRotator::NewL();
   iBitmap = new (ELeave) CFbsBitmap();
   iBitmap->Create(info.iOverallSizeInPixels, info.iFrameDisplayMode);
   if(info.iFlags & 
      (TFrameInfo::ETransparencyPossible | TFrameInfo::EAlphaChannel)){
      iMask = new (ELeave) CFbsBitmap();
      iMask->Create(info.iOverallSizeInPixels, EGray256);
      iDecoder->Convert(&iStatus, *iBitmap, *iMask);
   } else {
      iDecoder->Convert(&iStatus, *iBitmap);
   }
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();
}

CImageConverter::~CImageConverter()
{
   if(IsActive()){
      Cancel();
      iPicture.DecodeCanceled();
   }
   delete iDecoder;
}

void CImageConverter::RunL()
{
   if(iStatus == KErrNone){
      iPicture.DecodeComplete(iBitmap, iMask);
      delete iDecoder;
      iDecoder = NULL;
   } else if(iStatus == KErrUnderflow){
      iDecoder->ContinueConvert(&iStatus);
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif
      SetActive();
   }
}

void CImageConverter::DoCancel()
{
   iDecoder->Cancel();
}


/****************************************************************************
=============================================================================
* ///////////////////////////////////////////////////////////////////////////
*                                                                           *  
* CImage -- Takes File name as input and call CImageConverter               *  
*                                                                           *           
* ///////////////////////////////////////////////////////////////////////////
=============================================================================
****************************************************************************/

void CImage::DecodeCanceled()
{
}

CImage::CImage() :
   iActive(NULL), iRotateStatus(ERotationNone), iBitmap(NULL), iMask(NULL)   
{
   iIsReady = EFalse;
}

CImage::~CImage()
{
   iFs.Close();
   delete iBitmap;
   delete iMask;
}

void CImage::DecodeComplete(CFbsBitmap* aBitmap, CFbsBitmap* aMask)
{
   delete iBitmap;
   delete iMask;
   iBitmap = aBitmap;
   iMask = aMask;
   
   if(iRotateStatus == ERotationPending)
   {
      RotateImage();
      iRotateStatus = ERotationNone;
   }
   iIsReady = ETrue;
   DrawNow();
}

void CImage::ConstructL()
{
   iFs.Connect();
}

void CImage::OpenL( RFs& /*aFs */, const TDesC &aFile )
{
   iIsReady = EFalse;
   delete iActive;
   iActive = NULL;
   iActive = CImageConverter::NewL(*this, iFs, aFile);
}

// void CImage::SizeChanged()
// {
// }

void CImage::Draw(const TRect& aRect) const
{
   // Draw the parent control
   //CEikBorderedControl::Draw(aRect);
   // Get the standard graphics context 
   CWindowGc& gc = SystemGc();
   // Gets the control's extent - Don't encroach on the border
   TRect rect = Rect();//Border().InnerRect(Rect());
   // set the clipping region
   gc.SetClippingRect(rect);   
   if(iMask == NULL && iBitmap != NULL && rect.Intersects(aRect)){
      gc.BitBlt(rect.iTl, iBitmap);
   } else if(iMask != NULL && iBitmap != NULL && rect.Intersects(aRect)){
      TRect pictRect(TPoint(0,0), iBitmap->SizeInPixels());
      gc.BitBltMasked(rect.iTl, iBitmap, pictRect, iMask, EFalse);
   }
}

void  CImage::RotateImageClockwiseL(TImageRotateAngle aAngle)
{
   iAngle = aAngle;
   iRotateStatus = ERotationPending;
   return;
}

void CImage::RotateImage()
{
   CBitmapRotator::TRotationAngle LastRotation;
   
   switch (iAngle)
   {
   case E90Degrees:
      LastRotation = CBitmapRotator::ERotation90DegreesClockwise;
      break;
   case E180Degrees:
      //Start an asynchronous process to rotate the bitmap
      LastRotation = CBitmapRotator::ERotation180DegreesClockwise;
      break;
   case E270Degrees:
      //Start an asynchronous process to rotate the bitmap
      LastRotation = CBitmapRotator::ERotation270DegreesClockwise;
      break;
   default:
      return ;
   }
//Start an asynchronous process to rotate the bitmap

   delete iBmpRot;
   iBmpRot = NULL;
   
   iBmpRot = CImageRotator::NewL(this, LastRotation);
   iBmpRot->RotateBitmap();

   return ;
}


void CImage::RotationComplete()
{
   iRotateStatus = ERotationDone;
}

TSize CImage::GetSize() const
{
   return iBitmap->SizeInPixels();
}

TBool CImage::IsReady() const
{
   return iIsReady;
}

/*
=============================================================================
* ///////////////////////////////////////////////////////////////////////////
*
* CImage Rotator -- Used to rotate the Bitmap from CImage
*
* ///////////////////////////////////////////////////////////////////////////
=============================================================================
*/

class CImageRotator* CImageRotator::NewLC(class CImage* aPicture, 
                                  CBitmapRotator::TRotationAngle aAngle,
                                  enum TPriority aPriority)
{
   class CImageRotator* self = new (ELeave) CImageRotator(aPicture, aAngle, aPriority);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

class CImageRotator* CImageRotator::NewL(class CImage* aPicture, 
                                 CBitmapRotator::TRotationAngle aAngle,
                                 enum TPriority aPriority)
{
   class CImageRotator* self = CImageRotator::NewLC(aPicture, aAngle, aPriority);
   CleanupStack::Pop();
   return self;
}


CImageRotator::CImageRotator(class CImage* aPicture, 
                             CBitmapRotator::TRotationAngle aAngle,    
                             enum TPriority aPriority) : 
   CActive(aPriority), iPicture(aPicture), iAngle(aAngle)
{
   CActiveScheduler::Add(this);
   RotationNum = 0;
}

void CImageRotator::ConstructL()
{
   iBmpRotator = CBitmapRotator::NewL();
   iBitmap = iPicture->iBitmap;
   iMask = iPicture->iMask;
}

CImageRotator::~CImageRotator()
{
   if(IsActive()){
      Cancel();
   }
}

void CImageRotator::RunL()
{
   if(iStatus == KErrNone){
      if(RotationNum == 1)
      {
         RotationNum = 2;
         iPicture->iBitmap = iBitmap;
         if(iMask != NULL)
         {
            iBmpRotator->Rotate(&iStatus, *iMask, iAngle);
         #ifdef USE_TRACE
            TRACE_FUNC();
         #endif
            SetActive();
         }
         else
         {
            iPicture->RotationComplete();
            DoCancel();
         }
      }
      else if(RotationNum == 2)
      {
         RotationNum = 3;
         iPicture->iMask = iMask;
         iPicture->RotationComplete();
         DoCancel();
      }   
   } 
}

void CImageRotator::DoCancel()
{
}


void CImageRotator::RotateBitmap()
{
   RotationNum =1;
   iBmpRotator->Rotate(&iStatus, *iBitmap, iAngle);
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();
}
