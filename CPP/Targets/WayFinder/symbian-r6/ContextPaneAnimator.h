/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONTEXT_PANE_ANIMATOR_H
#define CONTEXT_PANE_ANIMATOR_H

#ifdef NAV2_CLIENT_SERIES60_V3
   // When building the mbm on symbian9 all masks
   // are automatically named to name_mask
   // FIXME Hfasth, should use svg icons here
#define BUSY_ICON_LIST        \
      EMbmWficonsBusy1, \
      EMbmWficonsBusy2, \
      EMbmWficonsBusy3, \
      EMbmWficonsBusy4, \
      EMbmWficonsBusy5, \
      EMbmWficonsBusy6, \
      EMbmWficonsBusy7, \
      EMbmWficonsBusy8, \
      EMbmWficonsBusy1_mask, \
      EMbmWficonsBusy2_mask, \
      EMbmWficonsBusy3_mask, \
      EMbmWficonsBusy4_mask, \
      EMbmWficonsBusy5_mask, \
      EMbmWficonsBusy6_mask, \
      EMbmWficonsBusy7_mask, \
      EMbmWficonsBusy8_mask
#else
#define BUSY_ICON_LIST        \
      EMbmWficons32x30_busy1, \
      EMbmWficons32x30_busy2, \
      EMbmWficons32x30_busy3, \
      EMbmWficons32x30_busy4, \
      EMbmWficons32x30_busy1_m, \
      EMbmWficons32x30_busy2_m, \
      EMbmWficons32x30_busy3_m, \
      EMbmWficons32x30_busy4_m
#endif

#define BUSY_ICON_NUM_FRAMES  8

/**
 * CContextPaneAnimator
 * Loads and caches the context pane icon.
 * It also combines the images for GPS quality and animation.
 */
class CContextPaneAnimator : public CBase {
public:
   /**
    * Constructor
    */
   CContextPaneAnimator(const TDesC &mbmPath);
   /**
    * Destructor
    */
   ~CContextPaneAnimator();

   /**
    * GetCurrentFrameL
    * @return the current frame (bitmap and mask)
    */
   CAnimatorFrame *GetCurrentFrameL();
   /**
    * IncCurrentFrame
    * Increment the current frame index
    * @param count      Number of frames to increment index
    * @return the old frame index
    */
   TInt IncCurrentFrame(TInt count = 1);
   /**
    * DecCurrentFrame
    * Decrement the current frame index
    * @param count      Number of frames to decrement index
    * @return the old frame index
    */
   TInt DecCurrentFrame(TInt count = 1);
   /**
    * SetCurrentFrame
    * Set current frame number index
    * @param newFrame   New frame index
    * @returns the old frame index
    */
   TInt SetCurrentFrame(TInt newFrame);
   /**
    * SetCurrentGpsStatus
    * Set current GPS status index
    * @param newQuality New GPS quality index
    * @returns the old GPS quality index
    */
   TInt SetCurrentGpsStatus(TInt newQuality);

   /**
    * CurrentOk
    * @return ETrue if current frame has been loaded
    */
   TBool CurrentOk();
   /**
    * GetAnimationFrameL
    * @return the current animation frame
    */
   CAnimatorFrame *GetAnimationFrameL();
   /**
    * GetAnimationFrameL
    * @param index      Index of frame to return
    * @return the animation frame with specified index.
    */
   CAnimatorFrame *GetAnimationFrameL(TInt index);
#if defined NAV2_CLIENT_SERIES60_V3
   /**
    * GetSvgAnimationFrameL
    * @return the current SVG animation frame
    */
   CAnimatorFrame *GetSvgAnimationFrameL();
   /**
    * GetSvgAnimationFrameL
    * @param index      Index of SVG frame to return
    * @return the SVG animation frame with specified index.
    */
   CAnimatorFrame *GetSvgAnimationFrameL(TInt index);
   /**
    * RecalcLargeGpsFrameSize
    * Recalculates the large GPS frame size.
    */
   void RecalcLargeGpsFrameSize();
   /**
    * RecalcContextAnimSizes
    * Recalculates the context pane animation sizes.
    */
   void RecalcContextAnimSizes();

#endif
   /**
    * GetGpsFrameL
    * @return the current GPS frame
    */
   CAnimatorFrame *GetGpsFrameL();
   /**
    * GetGpsFrameL
    * @param index      Index of GPS frame to return
    * @return the GPS frame with specified index
    */
   CAnimatorFrame *GetGpsFrameL(TInt index);
   /**
    * GetLargeGpsFrameL
    * @return the current large GPS frame
    */
   CAnimatorFrame *GetLargeGpsFrameL();
   /**
    * GetLargeGpsFrameL
    * @param index      Index of large GPS frame to return
    * @return the large GPS frame with specified index
    */
   CAnimatorFrame *GetLargeGpsFrameL(TInt index);
   /**
    * MergeToContextPaneL
    * Merge two CFbsBitmaps by copying them to a bitmap that is
    * the height of both bitmaps together. The anim bitmap will
    * be positioned at the top, and the gps bitmap will be positioned
    * at the bottom of the new bitmap.
    * @param anim       Bitmap to put at the top
    * @param gps        Bitmap to put at the bottom
    * @return a bitmap containing both images.
    */
   CFbsBitmap *MergeToContextPaneL(CFbsBitmap* anim, CFbsBitmap* gps);
   /**
    * MergeToContextPaneL
    * Merge two animator frames by copying them to a single frame of
    * the combined height of the two frames.
    * Uses the CFbsBitmap version above to merge both bitmaps and masks.
    * @param anim       Animation frame to put at the top
    * @param gps        Animation frame to put at the bottom
    * @return a CAnimatorFrame containing both animation frames.
    */
   CAnimatorFrame *MergeToContextPaneL(CAnimatorFrame* anim, CAnimatorFrame* gps);

private:
   /**
    * Assignment operator.
    * Copies the animation frame.
    */
   const CContextPaneAnimator& operator=(const CContextPaneAnimator&);
   /**
    * Copy constructor
    */
   CContextPaneAnimator(const CContextPaneAnimator&);

   /**
    * CalculateSize
    * Internal method, calculates size based on rect.
    */
   void CalculateSize(const TRect& aRect,
                      TSize& aNewSize, 
                      const TSize aOldSize,
                      TInt aSceenWidth, 
                      TInt aScreenHeight);

private:
   TBuf<256> iMbmPath;
   TBool iCurrentOk;
   TInt iCurrentFrameIndex;
   TInt iCurrentGpsStatus;
   CAnimatorFrame *iCurrentFrame;
   TInt numAnimationFrames;
   CAnimatorFrame **iAnimationFrames;
   CAnimatorFrame **iSvgAnimationFrames;
   TInt numGpsQualityFrames;
   CAnimatorFrame **iGpsQualityFrames;
   CAnimatorFrame **iLargeGpsQualityFrames;
};
#endif
