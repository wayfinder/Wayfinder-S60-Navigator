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

#ifdef NAV2_CLIENT_SERIES60_V3
# include <akniconutils.h>
#else 
# include "ScaleMode.h"
#endif

// #ifndef NAV2_CLIENT_SERIES60_V3
// enum TScaleMode {
//    EAspectRatioPreserved,
//    EAspectRatioPreservedAndUnusedSpaceRemoved,
//    EAspectRatioNotPreserved
// };
// #endif

class CAnimatorFrame : public CBase {
public:
   CAnimatorFrame();
   ~CAnimatorFrame();
   TSize GetSize() const;
   CFbsBitmap* GetBitmap() const;
   void SetBitmap(CFbsBitmap *newBitmap);
   CFbsBitmap* GetMask() const;
   void SetMask(CFbsBitmap *newMask);
   void CopyFrameL(const CAnimatorFrame *orgFrame);
   void DisownBitmaps();
   CFbsBitmap *CopyBitmapL(const CFbsBitmap *orgBitmap);
   void LoadBitmapL(TDesC& mbmPath, TInt aId);
   void LoadBitmapL(TDesC& mbmPath, TInt aId, TSize aSize, 
                    TScaleMode aKeepAspectRatio = EAspectRatioPreserved);
   void LoadMaskL(TDesC& mbmPath, TInt aId);
   void LoadMaskL(TDesC& mbmPath, TInt aId, TSize aSize, 
                  TScaleMode aKeepAspectRatio = EAspectRatioPreserved);
   void LoadBitmapAndMaskL(TDesC& mbmPath, TInt aBitmapId, 
                           TInt aMaskId, TSize aSize, 
                           TScaleMode aKeepAspectRatio = EAspectRatioPreserved);

#if defined NAV2_CLIENT_SERIES60_V3
   void SetBitmapSize(TSize aSize, TScaleMode aScaleMode = EAspectRatioPreserved);
#endif

private:
   const CAnimatorFrame& operator=(const CAnimatorFrame&);
   CAnimatorFrame(const CAnimatorFrame&);
   CFbsBitmap *bitmap;
   CFbsBitmap *mask;
};

