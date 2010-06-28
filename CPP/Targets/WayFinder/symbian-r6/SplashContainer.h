/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SPLASHCONTAINER_H
#define SPLASHCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include "PictureContainer.h"

#include "Log.h"
   
// FORWARD DECLARATIONS
class CSplashView;
class CGuidePicture;

// CLASS DECLARATION

/**
*  CSplashContainer  container control class.
*  
*/

class CSplashContainer : public CCoeControl,
                                MCoeControlObserver
{
public: // Constructors and destructor

   CSplashContainer(isab::Log* aLog) : iLog(aLog) {}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL( const TRect& aRect,
         CGuidePicture* aStartupImage,
         CSplashView* aView );

   void ConstructL( const TRect& aRect,
         class CImageHandler* aStartupImage,
         class CSplashView* aView );

   /**
    * Destructor.
    */
   ~CSplashContainer();

public: // New functions

   /**
    * Timer management function
    */
   void SetTimerTick(TTimeIntervalMicroSeconds32  aTick);
   void StartTimer();
   void StopTimer();
   static TInt Period(TAny* aPtr);
   void DoPeriod();

   void AskForIAPDone();

   void ShowStatusPane(TBool aShow);
   void ReportProgress(TInt aVal, TInt aMax);
public: // Functions from base classes

private: // New Functions
   void InitComponents(const TRect& aRect, CSplashView* aView);
   
private: // Functions from base classes
    
   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
   TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From ?base_class ?member_description
    */
   // event handling section
   // e.g Listbox events
   void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
       
   void HandleResourceChange(TInt aType);

private: //data

   CGuidePicture* iStartupImage;
   class CImageHandler* iImageHandler;
   CSplashView* iView;
   TRgb iBgColor;
   TRect iNewsRect;
   CPeriodic* iPeriodicTimer;
   TTimeIntervalMicroSeconds32  iTick;
   TInt iNewsCount;
   TBool iIsStarted;
   TBool iUseImageHandler;
   TPoint iNormalTopLeft;
   isab::Log* iLog;
};

#endif

// End of File
