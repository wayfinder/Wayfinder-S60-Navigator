/*  Copyright (c) 2004-2006, Nokia. */

// INCLUDE FILES

#include "BitmapConverter.h"
#include <imageconversion.h>
#include <bitmaptransforms.h>
#include <eikapp.h> 
#include <eikappui.h> 

// -----------------------------------------------------------------------------
// CBitmapConverter::CBitmapConverter
// C++ constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBitmapConverter::CBitmapConverter(class MImageObserver* aObserver,
                                   class RFs& aFs)
   : CActive(CActive::EPriorityStandard),
     iObserver(aObserver),
     iDecoder(NULL),
     iBitmap(NULL), 
     iMask(NULL),
     iFs(aFs)
{
}

// -----------------------------------------------------------------------------
// CBitmapConverter::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBitmapConverter::ConstructL()
{
   iScaler = CBitmapScaler::NewL();
   iState = EIdle;
   CActiveScheduler::Add(this);
}

// -----------------------------------------------------------------------------
// CBitmapConverter::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBitmapConverter* CBitmapConverter::NewL(class MImageObserver* aObserver,
                                         class RFs& aFs)
{
   CBitmapConverter* self = NewLC(aObserver, aFs);
   CleanupStack::Pop();
   return self;
}

// -----------------------------------------------------------------------------
// CBitmapConverter::NewLC
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBitmapConverter* CBitmapConverter::NewLC(class MImageObserver* aObserver,
                                          class RFs& aFs)
{
   CBitmapConverter* self = new (ELeave) CBitmapConverter(aObserver, aFs);
   CleanupStack::PushL( self );
   self->ConstructL();  
   return self;
}

// Destructor
CBitmapConverter::~CBitmapConverter()
{
   delete iDecoder;
   delete iBitmap;
   delete iMask;
   delete iScaler;
}

void CBitmapConverter::CompleteRequest(TInt aStatus)
{
   if (!IsActive()) {
      iState = EConvertStateReady;
      class TRequestStatus* status = &iStatus; 
      iStatus = KRequestPending;
      SetActive();
      User::RequestComplete(status, aStatus);      
   }
}

// // -----------------------------------------------------------------------------
// // CBitmapConverter::SetBitmapL
// // Set the destination bitmap.
// // -----------------------------------------------------------------------------
// //
// TBool CBitmapConverter::SetBitmapL(class CFbsBitmap*& aBitmap)
// {
//    if (IsActive()) {
//       // If active, fail.
//       return EFalse;
//    }
//    // else
//    iBitmap = aBitmap;
//    return ETrue;
// }

// -----------------------------------------------------------------------------
// CBitmapConverter::LoadFileL
// Loads a selected frame from a named file
// -----------------------------------------------------------------------------
//
TInt CBitmapConverter::ConvertFileL(const TDesC& aFileName, 
                                    TSize aSize, 
                                    TDisplayMode aDisplayMode,
                                    TBool aDoScale, 
                                    TInt aSelectedFrame)
{
   if (IsActive()) {
      return EFalse;
   }
   iState = EConverting;
   if (iDecoder) {
      delete iDecoder;
   }
//    if (!iBitmap) {
      iBitmap = new (ELeave) CFbsBitmap; 
//    }
//    if (!iMask) {
      iMask = new (ELeave) CFbsBitmap; 
//    }
   iSize = aSize;
   iDoScale = aDoScale;
   iDecoder = NULL;
   iDecoder = CImageDecoder::FileNewL(iFs, aFileName);
   // Get image information
   TFrameInfo frameInfo = iDecoder->FrameInfo(aSelectedFrame);
   // Resize to fit.
   TRect bitmapSize = frameInfo.iFrameCoordsInPixels;
   iBitmap->Create(bitmapSize.Size(), aDisplayMode);
   iMask->Create(bitmapSize.Size(), EGray2);
   //   iBitmap->Resize(bitmapSize.Size());
   
   // Decode as bitmap.
   iDecoder->Convert(&iStatus, *iBitmap, *iMask, aSelectedFrame);
   SetActive();
   return 1;
}

// -----------------------------------------------------------------------------
// CBitmapConverter::RunL
// CActive::RunL() implementation. Called on image load success/failure.
// -----------------------------------------------------------------------------
//
void CBitmapConverter::RunL()
{
   // Invoke callback.
   switch (iStatus.Int()) {
   case KErrNone:
      switch (iState) {
      case EIdle:
         // This should never happen
         break;
      case EConverting:
         ScaleImage();
         break;
      case EScaling:
      case EConvertStateReady:
         iState = EConvertStateReady;
         Complete();
         break;
      }
      break;
   default:
      Complete();
   }
}

// -----------------------------------------------------------------------------
// CBitmapConverter::DoCancel
// CActive::Cancel() implementation. Stops decoding.
// -----------------------------------------------------------------------------
//
void CBitmapConverter::DoCancel()
{
   if (iDecoder) {
      iDecoder->Cancel();
      delete iDecoder;
      iDecoder = NULL;
   }  
}

void CBitmapConverter::ScaleImage()
{
   if (iDoScale && (iSize != iBitmap->SizeInPixels())) {
      iState = EScaling;
      if (!iScaler) {
         iScaler = CBitmapScaler::NewL();
      }
      iScaler->Scale(&iStatus, *iBitmap, iSize);
      SetActive();
   } else {
      // Either we are told not to scale the image or 
      // the size is already correct.
      iState = EConvertStateReady;
      Complete();
   }
}

void CBitmapConverter::Complete()
{
   delete iDecoder;
   iDecoder = NULL;
   iObserver->ImageConversionDone(iStatus.Int());
}

TInt CBitmapConverter::GetConvertState()
{
   return iState;
}

const CFbsBitmap* CBitmapConverter::GetImage()
{
   return iBitmap;
}

const CFbsBitmap* CBitmapConverter::GetMask()
{
   return iMask;
}
