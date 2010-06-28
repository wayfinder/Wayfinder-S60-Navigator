/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/


#include <e32base.h>
#include <fbs.h>
#ifdef NAV2_CLIENT_SERIES60_V3
#include <akniconutils.h>
#endif
#include "AnimatorFrame.h"
#include <bitstd.h>
#include <bitdev.h>
#include "WFSymbianUtil.h"

CAnimatorFrame::CAnimatorFrame() :
   bitmap(NULL),
   mask(NULL)
{
}

CAnimatorFrame::~CAnimatorFrame()
{
   delete bitmap;
   delete mask;
}

CFbsBitmap*
CAnimatorFrame::GetBitmap() const
{
   return bitmap;
}
CFbsBitmap*
CAnimatorFrame::GetMask() const
{
   return mask;
}

TSize
CAnimatorFrame::GetSize() const
{
   return bitmap->SizeInPixels();
}
void
CAnimatorFrame::SetBitmap(CFbsBitmap *newBitmap)
{
   if (bitmap) {
      delete bitmap;
   }
   bitmap = newBitmap;
}
void
CAnimatorFrame::SetMask(CFbsBitmap *newMask)
{
   if (mask) {
      delete mask;
   }
   mask = newMask;
}

void
CAnimatorFrame::LoadBitmapL(TDesC& mbmPath, TInt aId)
{
   if (bitmap) {
      delete bitmap;
   }
   bitmap = new (ELeave) CFbsBitmap();
   bitmap->Load(mbmPath, aId);
}

void 
CAnimatorFrame::LoadBitmapL(TDesC& mbmPath, TInt aId, TSize aSize, 
                            TScaleMode aKeepAspectRatio)
{
   if (bitmap) {
      delete bitmap;
   }
#ifdef NAV2_CLIENT_SERIES60_V3
   bitmap = AknIconUtils::CreateIconL(mbmPath, aId);
   AknIconUtils::SetSize(bitmap, aSize, aKeepAspectRatio);
#else
   LoadBitmapL(mbmPath, aId);
#endif
}

void
CAnimatorFrame::LoadMaskL(TDesC& mbmPath, TInt aId)
{
   if (mask) {
      delete mask;
   }
   mask = new (ELeave) CFbsBitmap();
   mask->Load(mbmPath, aId);
}

void
CAnimatorFrame::LoadMaskL(TDesC& mbmPath, TInt aId, TSize aSize, 
                          TScaleMode aKeepAspectRatio)
{
   if (mask) {
      delete mask;
   }
#ifdef NAV2_CLIENT_SERIES60_V3
   mask = AknIconUtils::CreateIconL(mbmPath, aId);
   AknIconUtils::SetSize(mask, aSize, aKeepAspectRatio);
#else
   LoadMaskL(mbmPath, aId);
#endif
}

void
CAnimatorFrame::LoadBitmapAndMaskL(TDesC& mbmPath, TInt aBitmapId, 
                                   TInt aMaskId, TSize aSize, TScaleMode aKeepAspectRatio)
{
   if (bitmap) {
      delete bitmap;
   }
   if (mask) {
      delete mask;
   }
#ifdef NAV2_CLIENT_SERIES60_V3
   AknIconUtils::CreateIconL(bitmap, mask, mbmPath, 
                             aBitmapId, aMaskId);
   // Sets the size on both bitmap and mask.
   AknIconUtils::SetSize(bitmap, aSize, aKeepAspectRatio);
#else
   LoadBitmapL(mbmPath, aBitmapId);
   LoadMaskL(mbmPath, aMaskId);
#endif
}

CFbsBitmap*
CAnimatorFrame::CopyBitmapL(const CFbsBitmap *orgBitmap)
{
   if (!orgBitmap) {
      return NULL;
   }
   CFbsBitmap* tmpBitmap = new (ELeave) CFbsBitmap();
   CleanupStack::PushL(tmpBitmap);
#if defined NAV2_CLIENT_SERIES60_V3
   User::LeaveIfError(tmpBitmap->Create(orgBitmap->SizeInPixels(),
            orgBitmap->DisplayMode()));
#else
   User::LeaveIfError(tmpBitmap->Create(orgBitmap->SizeInPixels(),
            EColor256));
#endif
   CFbsBitmapDevice* bitmapDevice = CFbsBitmapDevice::NewL(tmpBitmap);
   CleanupStack::PushL(bitmapDevice);

   CFbsBitGc* bitmapContext=NULL;
   User::LeaveIfError(bitmapDevice->CreateContext(bitmapContext));
   CleanupStack::PushL(bitmapContext);

   bitmapContext->BitBlt(TPoint(0,0), orgBitmap);
   CleanupStack::PopAndDestroy(2, bitmapDevice);

   CleanupStack::Pop(tmpBitmap);
   return tmpBitmap;
}

void
CAnimatorFrame::CopyFrameL(const CAnimatorFrame *orgFrame)
{
   /* Copy bitmaps to current object. */
   delete bitmap;
   bitmap = NULL;
   delete mask;
   mask = NULL;
   CFbsBitmap *orgBitmap = orgFrame->GetBitmap();
   CFbsBitmap *orgMask = orgFrame->GetMask();

   if (orgBitmap) {
      bitmap = CopyBitmapL(orgBitmap);
   }
   if (orgMask) {
      mask = CopyBitmapL(orgMask);
   }
}
void
CAnimatorFrame::DisownBitmaps()
{
   bitmap = NULL;
   mask = NULL;
}

#if defined NAV2_CLIENT_SERIES60_V3
void
CAnimatorFrame::SetBitmapSize(TSize aSize, TScaleMode aScaleMode)
{
   if (bitmap) {
      AknIconUtils::SetSize(bitmap, aSize, aScaleMode);
   }
}
#endif
