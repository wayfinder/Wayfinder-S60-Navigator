/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef USERBITMAP
#define USERBITMAP

#include <fbs.h>

/* class basically created intentionally which contain the bitmap
which will be shown on to vector map at a given position*/

class TUserBitmap
{
   public:
   /* construct class object for User define bitmap*/
      TUserBitmap();
      
   /* set variable with out mask*/  
      void setUserBitmap(CFbsBitmap* aBitmap, TPoint aPoint,TRect aRect);
      
   /* set variable with mask*/  
      void setUserBitmapWithMask(CFbsBitmap* aBitmap,CFbsBitmap* aBitmapMask,TPoint aPoint,TRect aRect);
      
   /* set bitmap usually required if bitmap changes runtime*/   
      void setBitmap(CFbsBitmap* aBitmap);
      
   /* set point usually required if position of bitmap changes*/   
      void setPoint(TPoint aPoint);

   /* set to show the bitmap else reset*/      
      void setShow(TBool aShow);

   /* if the bitmap is visible or not */
      TBool getShow();
      
   public:
   
   CFbsBitmap* iBitmap;
   CFbsBitmap* iBitmapMask;
   TPoint iBlitPoint;
   TRect iRect;
   TBool iShow;
};

#endif
