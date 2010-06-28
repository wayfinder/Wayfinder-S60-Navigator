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

#ifndef POSITIONDISPLAY_H
#define POSITIONDISPLAY_H
#include <coecntrl.h> //ccoecontrol

///This class is meant to display one line of either latitude or
///longitude data.
class CCoordinateLineDisplay: public CCoeControl
{
   ///The current value as rad*1e8.
   TInt iVal;
   ///This TBuf stores the letters used for direction (N,S,E,W)
   TBuf<5> iDirections;
   ///This constant holds the maximum absolute value that will be
   ///displayed by this instance of CCoordinateLineDisplay. If iVal is
   ///set to anything larger, the control will only display zeros.
   const TInt iMaxVal;
   ///The background color of the control.
   class TRgb iBgClr;
   ///The text color of the control. 
   class TRgb iTxtClr;
   ///The currently displayed string. Can be used to compare with new values. 
   TBuf<16> iLastString;
   ///The new string to display. Will be written to iLastString once drawn. 
   TBuf<16> iNewString;
   ///Constructor. 
   ///@param aMaxVal the value that iMaxVal will be set to. 
   ///@param aBackGround the controls background color. 
   ///@param aText the text color to use. Defaults to KRgbBlack. 
   CCoordinateLineDisplay(TInt aMaxVal, class TRgb aBackground, 
                          class TRgb aText = KRgbBlack);
   ///Second phase constructor. 
   ///@param aParent the holding container. Used to set the window. 
   ///@param aPosId the resource id of the one-character string that will 
   ///              be used as the prefix letter for positive values (N,E).
   ///@param aPosId the resource id of the one-character string that will
   ///              be used as the prefix letter for negative values (S,W)
   void ConstructL(class CCoeControl& aParent, TInt aPosId, TInt aNegId);
   ///Writes a formatted position string to iNewVal based on the value.
   ///TODO: Why a argument.
   ///@param aVal the value to format.
   void Format(TInt aVal) ;

   ///Used to hold the values needed for a Format.
   class TValueParts {
   public:
      ///The only legal values are 1 and -1, depending on whether the
      ///value was positive or negative.
      TInt iSign;
      ///The degrees part. Should be between 0 and 360.
      TInt iDeg;
      ///The minutes part. Should be between 0 and 60.
      TInt iMin;
      ///The seconds part. Should be between 0 and 60. 
      TInt iSec;
      ///The hundreds of a second part. Should be between 0 and 100.
      TInt iParts;
      ///Default constructor that sets all member values to zero.
      TValueParts() : iSign(0), iDeg(0), iMin(0), iSec(0), iParts(0) {}
   };
   ///Sets the contents of aParts to represent the value of aCoord.
   ///@param aCoord a nav2 coordinate.
   ///@param aParts target parameter.
   void SetCoordinate(TInt32 aCoord, class TValueParts& aParts) const;
public:
   ///Static constructor. 
   ///@param aParent the containing control. Used to find a window. 
   ///@param aPosId the resource id of the one-character string that will 
   ///              be used as the prefix letter for positive values (N,E).
   ///@param aPosId the resource id of the one-character string that will
   ///              be used as the prefix letter for negative values (S,W)
   ///@param aMaxVal the value that iMaxVal will be set to. 
   ///@param aBackGround the controls background color. 
   ///@param aText the text color to use. Defaults to KRgbBlack.
   ///@return a new CCoordinateLineDisplay object that has been pushed to 
   ///        the CleanupStack.
   static class CCoordinateLineDisplay* 
   NewLC(class CCoeControl& aParent, TInt aPosId, TInt aNegId, TInt aMaxVal, 
         class TRgb aBackground, class TRgb aText = KRgbBlack);
   ///Static constructor. 
   ///@param aParent the containing control. Used to find a window. 
   ///@param aPosId the resource id of the one-character string that will 
   ///              be used as the prefix letter for positive values (N,E).
   ///@param aPosId the resource id of the one-character string that will
   ///              be used as the prefix letter for negative values (S,W)
   ///@param aMaxVal the value that iMaxVal will be set to. 
   ///@param aBackGround the controls background color. 
   ///@param aText the text color to use. Defaults to KRgbBlack.
   ///@return a new CCoordinateLineDisplay object..
   static class CCoordinateLineDisplay* 
   NewL(class CCoeControl& aParent, TInt aPosId, TInt aNegId, TInt aMaxVal, 
        class TRgb aBackground, class TRgb aText = KRgbBlack);
   ///Virtual destructor.
   virtual ~CCoordinateLineDisplay();

   ///Draw function. See CCoeControl.
   virtual void Draw(const TRect& aRect) const;

   ///Gets the current value as a nav2 coordinate.
   ///@return the current value. 
   TInt GetVal() const;
   ///Sets the value to display. 
   ///@param aVal a nav2 coordinate value. 
   void SetVal(TInt aVal);
};

///This control displays a header and a coordinate as lat and lon. 
///The coordinates will be written as monospace, so they line up really nice. 
///It will draw a "bubble" around the coordinates, but not the header. 
///Note that this control will look weird if it not gets enough space. 
class CPositionDisplay : public CCoeControl
{
   ///The indentation in pixels from the header to the bubble. 
   TInt iIndent;
   ///The header label.
   class CEikLabel* iHeader;
   ///The top line of coordinate data, showing latitude. 
   class CCoordinateLineDisplay* iLatControl;
   ///The bottom line of coordinate data, showing longitude. 
   class CCoordinateLineDisplay* iLonControl;
   ///The background color. 
   class TRgb iBgClr;
   ///The color of the bubble outline.
   class TRgb iOutlineClr;
   ///The color of the bubble. 
   class TRgb iFldClr;
   ///The color of the text. 
   class TRgb iTxtClr;
   ///Constructor. 
   ///@param aBackground the background color. 
   ///@param aOutline the bubble outline color. 
   ///@param aField the bubble color.
   ///@param aText the text color. Defaults to KRgbBlack. 
   CPositionDisplay(class TRgb aBackground, class TRgb aOutline, 
                    class TRgb aField, class TRgb aText = KRgbBlack);
   ///Second phase constructor. 
   ///@param aParent    parent control. Used to set the window.
   ///@param aHeaderId  resource id of the header string. 
   ///@param aLatPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (north) latitude 
   ///@param aLatMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (south) latitude 
   ///@param aLonPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (east) longitude.
   ///@param aLonMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (west) longitude. 
   void ConstructL(class CCoeControl& aParent, TInt aHeaderId,
                   TInt aLatPlusId, TInt aLatMinusId, 
                   TInt aLonPlusId, TInt aLonMinusId);
public:
   ///Static constructor. 
   ///@param aParent    parent control. Used to set the window.
   ///@param aHeaderId  resource id of the header string. 
   ///@param aLatPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (north) latitude 
   ///@param aLatMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (south) latitude 
   ///@param aLonPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (east) longitude.
   ///@param aLonMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (west) longitude. 
   ///@param aBackground the background color. 
   ///@param aOutline the bubble outline color. 
   ///@param aField the bubble color.
   ///@param aText the text color. 
   ///@return a new CPositionDisplay object, that is on top of the 
   ///        CleanupStack.
   static class CPositionDisplay* NewLC(class CCoeControl& aParent,
                                        TInt aHeader, 
                                        TInt aLatPlusId, TInt aLatMinusId, 
                                        TInt aLonPlusId, TInt aLonMinusId,
                                        class TRgb aBackground, 
                                        class TRgb aOutline, 
                                        class TRgb aField, class TRgb aText);
   ///Static constructor. 
   ///@param aParent    parent control. Used to set the window.
   ///@param aHeaderId  resource id of the header string. 
   ///@param aLatPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (north) latitude 
   ///@param aLatMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (south) latitude 
   ///@param aLonPlusId resource id if the one-character string used as a 
   ///                  prefix to a positive (east) longitude.
   ///@param aLonMinusId resource id if the one-character string used as a 
   ///                   prefix to a negative (west) longitude. 
   ///@param aBackground the background color. 
   ///@param aOutline the bubble outline color. 
   ///@param aField the bubble color.
   ///@param aText the text color. 
   ///@return a new CPositionDisplay object.
   static class CPositionDisplay* NewL(class CCoeControl& aParent,
                                       TInt aHeader, 
                                       TInt aLatPlusId, TInt aLatMinusId, 
                                       TInt aLonPlusId, TInt aLonMinusId,
                                       class TRgb aBackground, 
                                       class TRgb aOutline, 
                                       class TRgb aField, class TRgb aText);
   ///Virtual destructor. 
   virtual ~CPositionDisplay();
   ///Gets the current bubble indentation. 
   ///@return the current indentation in pixels.
   TInt Indent() const;
   ///Sets the bubble indentation.
   ///@param aIndent the new bubble indentation in pixels. 
   void SetIndent(TInt aIndent);

   ///@name Functions derived from CCoeControl.
   //@{
   virtual void Draw(const TRect& aRect) const;
   virtual TInt CountComponentControls() const;
   virtual class CCoeControl* ComponentControl(TInt aIndex) const;
   virtual void SizeChanged();
   virtual void MakeVisible(TBool aVisible);
   //@}

   ///Gets the current latitude.
   ///@return the current latitude, nav2 style.
   TInt GetLat() const;
   ///Gets the current longitude.
   ///@return the current longitude, nav2 style.
   TInt GetLon() const;
   ///Gets the current position.
   ///@return the current position, nav2 style.
   TPoint GetPos() const;
   ///Gets the current position
   ///@param return paremeter for the latitude, nav2 style.
   ///@param return paremeter for the longitude, nav2 style.
   void GetPos(TInt& aLat, TInt& aLon) const;
   ///Sets the current latitude. 
   ///@param aLat the new latitude, nav2 style
   void SetLat(TInt aLat);
   ///Sets the current longitude. 
   ///@param aLon the new longitude, nav2 style
   void SetLon(TInt aLon);
   ///Sets the current position. 
   ///@param aLat the new latitude, nav2 style. 
   ///@param aLon the new longitude, nav2 style.
   void SetPos(TInt aLat, TInt aLon);
   ///Sets the current position. 
   ///@param aPos the new position, nav2 style.
   void SetPos(TPoint aPos);
};
#endif
