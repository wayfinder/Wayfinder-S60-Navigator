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

#ifndef TRECTTOOLS_H
#define TRECTTOOLS_H
#include <e32std.h>
namespace {
   enum TRectPanic {
      ERectBadPercent = 0,
      ERectNot100Percent = 1,
   };
   void RectPanic(enum TRectPanic aPanic)
   {
      _LIT(KRectPanic, "RectTools");
      User::Panic(KRectPanic, aPanic);
   }

///Sets the width to half of the original width. The top left corner
///of the rect stays in the same place.
///@param aRect original TRect to work with.
///@return a Rect with half the width of aRect.
inline class TRect HalfWidth(class TRect aRect)
{
   aRect.Normalize();
   aRect.SetWidth(aRect.Width() / 2);
   return aRect;
}
   
///Sets the height to half of the original height. The topleft corner
///of the rect stays in the same place.
///@param aRect original TRect to work with.
///@return a TRect with half the height of aRect.
inline class TRect HalfHeight(class TRect aRect)
{
   aRect.Normalize();
   aRect.SetHeight(aRect.Height() / 2);
   return aRect;
}

///Flips the rect around it's right edge. This is the same as moving
///the rect the rects width pixels right.
///@param aRect the original TRect.
///@return a Rect with the same size as aRect, but with it's top left
///        corner in the same spot as aRect's top right corner.
inline class TRect FlipRight(class TRect aRect)
{
   aRect.Normalize();
   aRect.Move(aRect.Width(), 0);
   return aRect;
}


///Sets the width to half of the original width. The bottom right corner
///of the rect stays in the same place.
///@param aRect original TRect to work with.
///@return a Rect with half the width of aRect.
inline class TRect RightHalf(const class TRect& aRect)
{
   return FlipRight(HalfWidth(aRect));
}


///Flips the rect around it's lower edge. This is the same as moving
///the rect the rects height pixels down.
///@param aRect the original TRect.
///@return a Rect with the same size as aRect, but with it's top left
///        corner in the same spot as aRect's lower left corner.
inline class TRect FlipDown(class TRect aRect)
{
   aRect.Normalize();
   aRect.Move(0, aRect.Height());
   return aRect;
}

///Moves the right edge of the rect inwards by the specified number of
///pixels.
///@param aRect   the original TRect.
///@param aPixels the number of pixels to move the right edge. If
///               aPixels is negative the edge will be moved outward
///               instead.
///@return a TRect with the same position as aRect, but aPixels thinner.
inline class TRect ShrinkRight(class TRect aRect, TInt aPixels)
{
   aRect.Normalize();
   aRect.iBr.iX -= aPixels;
   return aRect;
}

///Moves the left edge of the rect inwards by the specified number of
///pixels.
///@param aRect   the original TRect.
///@param aPixels the number of pixels to move the left edge. If
///               aPixels is negative the edge will be moved outward
///               instead.
///@return a TRect with the same bottom right corner as aRect, but
///        aPixels thinner.
inline class TRect ShrinkLeft(class TRect aRect, TInt aPixels)
{
   aRect.Normalize();
   aRect.iTl.iX += aPixels;
   return aRect;
}

///Moves the top edge of the rect inwards by the specified number of
///pixels.
///@param aRect   the original TRect.
///@param aPixels the number of pixels to move the top edge. If
///               aPixels is negative the edge will be moved outward
///               instead.
///@return a TRect with the same bottom right corner as aRect, but
///        aPixels lower.
inline class TRect ShrinkTop(class TRect aRect, TInt aPixels)
{
   aRect.Normalize();
   aRect.iTl.iY += aPixels;
   return aRect;
}

///Moves the bottom edge of the rect inwards by the specified number of
///pixels.
///@param aRect   the original TRect.
///@param aPixels the number of pixels to move the bottom edge. If
///               aPixels is negative the edge will be moved outward
///               instead.
///@return a TRect with the same top left corner as aRect, but
///        aPixels smaller height.
inline class TRect ShrinkBottom(class TRect aRect, TInt aPixels)
{
   aRect.Normalize();
   aRect.iBr.iY -= aPixels;
   return aRect;
}

///Moves the top and left edges inwards by the number of pixels
///specified in aPixelPair.iY and aPixelPair.iX respectively.
///@param aRect      the original TRect.
///@param aPixelPair the value to subtract from aRect.iTl. If any of
///                  the values in aPixelPair is negative, that edge
///                  will be moved outwards instead.
///@return a TRect with the same bottom right corner as aRect, but
///        with the top left corner moved aPixelPair pixels. 
inline class TRect ShrinkTopLeft(class TRect aRect, TPoint aPixelPair)
{
   aRect.Normalize();
   aRect.iTl -= aPixelPair;
   return aRect;
}

///Moves the top and left edges inwards by the number of pixels
///specified in aPixels
///@param aRect      the original TRect.
///@param aPixelPair the value to move the top left corner inwards.
///                  If the aPixels value is negative, the edges
///                  will be moved outwards instead.
///@return a TRect with the same bottom right corner as aRect, but
///        with the top left corner moved aPixels pixels inwards. 
inline class TRect ShrinkTopLeft(class TRect aRect, TInt aPixels)
{
   return ShrinkTopLeft(aRect, TPoint(aPixels, aPixels));
}

///Shrinks a rectangle using a specified TSize offset.  The rectangle
///shrinks by twice the value of the height and width specified in the
///TSize. The co-ordinates of the centre of the rectangle remain
///unchanged. If either value is negative, the rectangle expands in
///the corresponding direction.
///@param aRect the orignal TRect that will be shrunk.
///@param aSize the shrink offset.
///@return the shrunk rectangle.
inline class TRect Shrink(class TRect aRect, class TSize aSize)
{
   aRect.Normalize();
   aRect.Shrink(aSize);
   return aRect;
}

///Shrinks a rectangle using a specified offset.  The rectangles
///height and width shrinks by twice the offset. The co-ordinates of
///the centre of the rectangle remain unchanged. If the value is
///negative, the rectangle expands with this offset.
///@param aRect the orignal TRect that will be shrunk.
///@param aSize the shrink offset.
///@return the shrunk rectangle.
inline class TRect Shrink(const class TRect& aRect, TInt aSize)
{
   return Shrink(aRect, TSize(aSize, aSize));
}

///Divides both the iHeight and iWidth members of a TSize object by
///the specified divisor.
///@param aTop the original TSize object.
///@param aDivisor the divisor.
///@return the resulting TSize object.
inline const class TSize operator/(class TSize aTop, TInt aDivisor)
{
   aTop.iWidth  /= aDivisor;
   aTop.iHeight /= aDivisor;
   return aTop;
}

///Finds the center point of a rectangle.
///@param aRect the rectangle which's center point we want to find.
///@return a TPoint object representing the center point of aRect.
inline class TPoint GetCenter(class TRect aRect)
{
   aRect.Normalize();
   return aRect.iTl + (aRect.Size() / 2);
}

///Sets the center point of a rectngle.
///@param aRect the rectangle to move.
///@param aNewCenter the new center point.
///@return the moved rectangle.
inline class TRect SetCenter(class TRect aRect, class TPoint aNewCenter)
{
   aRect.Normalize();
   TPoint offset = aNewCenter - GetCenter(aRect);
   aRect.Move(offset);
   return aRect;
}

///Sets the Height of a rectangle without moving it's top left corner.
///@param aRect   the original rectangle.
///@param aHeight the new height.
///@return        the modified rectangle.
inline class TRect SetHeight(class TRect aRect, TInt aHeight)
{
   aRect.Normalize();
   aRect.SetHeight(aHeight);
   return aRect;
}

///Sets the width of a rectangle without moving it's top left corner.
///@param aRect  the original rectangle.
///@param aWidth the new width.
///@return       the modified rectangle.
inline class TRect SetWidth(class TRect aRect, TInt aWidth)
{
   aRect.Normalize();
   aRect.SetWidth(aWidth);
   return aRect;
}

///Move one rectangle so that its center position is the same as that
///of another rectangle
///@param aCenterIn the reference rectangle.
///@param aOrigin the rectangle that should be moved.
///@return the moved rectangle.
inline class TRect Center(const class TRect& aCenterIn, class TRect aOrigin)
{
   return SetCenter(aOrigin, GetCenter(aCenterIn));
//    TSize offset = (aCenterIn.Size() - aOrigin.Size()) / 2;
//    aOrigin.SetRect(aCenterIn.iTl, aOrigin.Size());
//    aOrigin.Move(offset.iWidth, offset.iHeight);
//    return aOrigin;
}

///Reduces the size of a TRect object by the amount contained in a
///TSize object, whitout changing the top left position of the TRect.
///@param aRect a reference to the object to be changed. 
///@param aSize the amount to change the size with.
///@return a copy of the changed TRect object.
inline const class TRect operator-=(class TRect& aRect, TSize aSize)
{
   aRect.Normalize();
   aRect.iBr -= aSize;
   return aRect;
}

///Creates a new TRect object with the same top left corner as aRect,
///and its size being aRect.Size() - aSize.
///@param aRect the orignal TRect object.
///@param aSize the size to reduce with.
///@return a new TRect object with reduced size.
inline const class TRect operator-(class TRect aRect, class TSize aSize)
{
   aRect.Normalize();
   return aRect -= aSize;
}

///Splits a TRect horizontally so that the top half will be a
///specified percentage of the height. The original TRect that is fed
///as an argument to this funtion will be modified to be the lower
///half while the upper part will be retuned as a normal return value.
///@param aRect   the orignal rectangle. Also used as the return value
///               for the lower TRect.
///@param percent specifies how many percent of aRect's original
///       height that will make up the top rectangle. Values less than
///       0 or greater than 100 will cause a RectTools panic 0.
///@return the top rectangle that will be the top aPercent percent of
///        the original aRect rectangle
inline class TRect SplitHeight(class TRect& aRect, TInt aPercent)
{
   if(aPercent > 100 || aPercent < 0){
      RectPanic(ERectBadPercent);
   }
   TRect ret = aRect - TSize(0, (aRect.Height() * (100 - aPercent)) / 100);
   aRect.iTl.iY = ret.iBr.iY + 1;
   return ret;
}

///Splits a TRect vertically so that the left half will be a specified
///percentage of the original width. The original TRect that is fed as
///an argument to this funtion will be modified to be the right half
///while the left part will be retuned as a normal return value.
///@param aRect the orignal rectangle. Also used as the return value
///             for the right TRect.
///@param percent specifies how many percent of ARect's original
///       width that will make up the left rectangle. Values less than
///       0 or greater than 100 will cause a RectTools 0 panic.
///@return the left rectangle that will be the left aPercent percent of
///        the original aRect rectangle
inline class TRect SplitWidth(class TRect& aRect, TInt percent)
{
   if(percent > 100 || percent < 0){
      RectPanic(ERectBadPercent);
   }
   TRect ret = aRect - TSize((aRect.Width() * (100 - percent)) / 100, 0);
   aRect.iTl.iX = ret.iBr.iX + 1;
   return ret;
}

///Support template class for SplitHeight. Used as return type.
template<TInt N>
struct TRectArray{
   ///An array of N TRects.
   TRect iRects[N];
   ///Allows indexing in const TRectArray objects.
   ///@param aIndex index into array. Note that no check is made
   ///              whether the index is in range.
   const TRect& operator[](TInt aIndex) const {
      return iRects[aIndex];
   }
   ///Allows indexing in non-const TRectArray objects.
   ///@param aIndex index into array. Note that no check is made
   ///              whether the index is in range.
   TRect& operator[](TInt aIndex){
      return iRects[aIndex];
   }
   void fill(TRect aRect)
   {
      for(TInt n = 0; n < N; ++n){
         iRects[n] = aRect;
      }
   }
};

///Support template class for SplitHeight. Used as parameter type.
template<TInt N>
struct TIntArray{
   ///An array of TInts.
   TInt iInts[N];
   ///Allows indexing in const TIntArray objects.
   ///@param aIndex index into array. Note that no check is made
   ///              whether the index is in range.
   const TInt& operator[](TInt aIndex) const {
      return iInts[aIndex];
   }
   ///Allows indexing in non-const TIntArray objects.
   ///@param aIndex index into array. Note that no check is made
   ///              whether the index is in range.
   TInt& operator[](TInt aIndex) {
      return iInts[aIndex];
   }
};

template<typename T, TInt N>
struct TTypeArray{
   T iArray[N];
   const T& operator[](TInt aIndex) const {
      return iArray[aIndex];
   }
   T& operator[](TInt aIndex){
      return iArray[aIndex];
   }
   void Fill(const T& aValue){
      for(TInt n = 0; n < N; ++n){
         iArray[n] = aValue;
      }
   }
};


///This template function splits a TRect N-ways. All parts have equal
///width, but the height is split according to the aParts TIntArray
///object. If the sum of all values in the aParts array isn't 100, a
///ERectNot100percent panic is raised.
///@param aRect the original TRect.
///@param aParts a TIntArray object holding the percent values.
///@return a TRectArray object with each held TRect matching a percent
///        value in aParts.
template<TInt N>
TRectArray<N> SplitHeight(const class TRect& aRect, const TIntArray<N>& aParts)
{
   TInt sumOfParts = 0;
   TIntArray<N> heights;
   TInt sum = 0;
   for(TInt a = 0; a < N; ++a){
      sumOfParts += aParts[a];
      heights[a] = (aRect.Height() * aParts[a])/100;
      sum += heights[a];
   }
   if(sumOfParts != 100){
      RectPanic(ERectNot100Percent);
   }

   TInt leftover = aRect.Height() - sum;
   for(TInt j = 0; j < leftover; ++j){
      heights[(leftover*j)%N] += 1;
   }

   TRectArray<N> results;
   results.fill(aRect);
   results[0].iBr.iY = aRect.iTl.iY + heights[0];
   for(TInt k = 1; k < N; ++k){
      results[k].iTl.iY = results[k-1].iBr.iY + 1;
      results[k].iBr.iY = results[k].iTl.iY + heights[k];
   }
   return results;
}

///Returns a squale rectangle with width and height equal to the
///height of aRect. The returned TRect will have the same center
///position as aRect.
///@param aRect the original TRect.
///@return a Square.
inline class TRect SquareOnHeight(const class TRect& aRect)
{
   TRect ret = aRect;
   ret.SetWidth(aRect.Height());
   return Center(aRect, ret);
}

///Returns a TRect with size aSize and the same top right corner as aRect.
///@param aRect the TRect from which the top right corner shall be taken.
///@param aSize the size of the returned rectangle.
///@return the new rectangle
inline class TRect TopRight(const class TRect& aRect, const class TSize& aSize)
{
   return TRect(TPoint(aRect.iBr.iX - aSize.iWidth, aRect.iTl.iY), aSize);
}

///Returns a TRect with size aSize and the same lower right corner as aRect.
///@param aRect the TRect from which the lower right corner shall be taken.
///@param aSize the size of the returned rectangle.
///@return the new rectangle
inline class TRect LowerRight(const class TRect& aRect, 
                              const class TSize& aSize)
{
   return TRect(TPoint(aRect.iBr.iX - aSize.iWidth - 1, 
                       aRect.iBr.iY - aSize.iHeight - 1), aSize);
}

///Returns a TRect with size aSize and the same bottom left corner as aRect.
///@param aRect the TRect from which the bottom left corner shall be taken.
///@param aSize the size of the returned rectangle.
///@return the new rectangle
inline class TRect LowerLeft(const class TRect& aRect,const class TSize& aSize)
{
   return TRect(TPoint(aRect.iTl.iX, aRect.iBr.iY - aSize.iHeight - 1), aSize);
}


///Returns aRect moved by the specified amount.
///@param aRect the original rectangle.
///@param aDx the amount to move sideways. A positive amount moves
///           right, a negative left.
///@param aDy the amount to move up or down. A positive amount moves
///           down, a negative up.
///@return the moved rect.
inline class TRect Move(class TRect aRect, TInt aDx, TInt aDy)
{
   aRect.Move(aDx, aDy);
   return aRect;
}

///Returns aRect moved horizontally by the specified amount.
///@param aRect the original rectangle.
///@param aDx the amount to move sideways. A positive amount moves
///           right, a negative left.
///@return the moved rect.
inline class TRect MoveX(const class TRect& aRect, TInt aDx)
{
   return Move(aRect, aDx, 0);
}

///Returns aRect moved vertically by the specified amount.
///@param aRect the original rectangle.
///@param aDy the amount to move. A positive amount moves down, a
///           negative up.
///@return the moved rect.
inline class TRect MoveY(const class TRect& aRect, TInt aDy)
{
   return Move(aRect, 0, aDy);
}


///Tests whether one rectangle is totally contained whithin another rectangle.
///@param aContainer the containing TRect.
///@param aContainee the contained TRect.
///@param ETrue if both the bottom right corner and upper left corner
///             of aContainee are contained whithin the aContainer
///             rectangle.
inline TBool Contains(const class TRect& aContainer, 
                      const class TRect& aContainee)
{
   return aContainer.Contains(aContainee.iTl) && 
      aContainer.Contains(aContainee.iBr);
}

}//namespace

///Tests if the control pointer is NULL. If not it will call the
///SetRect function with the TRect argument.
///@param aControl the control that will be affectef. If the pointer
///                is NULL the function is a no-op.
///@param aRect    the new extent of the control.
inline void SetRect(class CCoeControl* aControl, const class TRect& aRect)
{
   if(aControl){ 
      aControl->SetRect(aRect);
   }
}



#endif
