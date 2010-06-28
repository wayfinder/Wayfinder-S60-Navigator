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

#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H

#include <e32base.h>               //CBase
#include "GuiProt/HelperEnums.h" //BacklightStrategy

///Mixin for a class that provides data on whether we are currently
///following a route or not.
class MRouteDataProvider {
public:
   ///The return value of this function specifies whether the
   ///backlight should be on or off when the strategy is set to
   ///backlight_on_during_route.
   ///@return ETrue if the application is currently following a route. 
   virtual TBool IsRouteActive() const = 0;
   ///Virtual destructor. 
   virtual ~MRouteDataProvider()
   {};
};

///Class that manages the backlight according to a backlight
///strategy. This is acheived by resetting the inactivity timer every
///5 seconds.
class CBacklightManager : public CBase
{
   enum {
      ///The interval for resetting the incativity timer. In
      ///microseconds.
      EInterval = 5*1000*1000
   };
public:
   ///This static function is called by the CPeridodicTimer held by
   ///this class. The argument TAny pointer will point to the
   ///CBacklightManager object whos backlightstrategy should be used.
   ///The function calls the Strategy function on the
   ///CBacklightManager object and resets the inactivitytimer
   ///accordingly.
   ///@param aPtr a pointer to the CBacklightManager object. 
   ///@return 0 
   static TInt Tick(TAny* aPtr);
private:
   ///@name Constructors and destructor. 
   //@{
   ///Second phase constructor. 
   void ConstructL();
public:
   ///Static constructor.
   ///@param aProvider pointer to a MRouteDataProvider that will be
   ///       queried for the current route following status when the
   ///       strategy is set to backlight_on_during_route.
   static class CBacklightManager* NewL(class MRouteDataProvider* aProvider);
   ///Virtual destructor.
   virtual ~CBacklightManager();
   //@}
private:
   ///@name Functions to set and use the MRouteDataProvider object. 
   //@{
   ///Sets the provider. 
   ///@param aProvider the new provider. 
   void SetProvider(class MRouteDataProvider* aProvider);
public:
   ///Calls the provider to see whether the application is currently
   ///following a route.
   ///@return the value returned by iProvider->HasRoute();
   TBool RouteActive() const;
   //@}

   ///@name Functions regarding the backlight strategy.
   //@{
   ///Returns the current backlight strategy. 
   ///@return the current backlight strategy. 
   enum isab::GuiProtEnums::BacklightStrategy Strategy() const;

   ///Sets the backlight strategy.
   ///@param aStrategy the new backlight strategy. 
   ///@return the previous backlight strategy. 
   enum isab::GuiProtEnums::BacklightStrategy 
   SetStrategy(enum isab::GuiProtEnums::BacklightStrategy aStrategy);
   //@}
private:
   ///@name Data members. 
   //@{
   ///The current backlight strategy.
   enum isab::GuiProtEnums::BacklightStrategy  iStrategy;
   ///The CPeriodic object used to reset the inactivity timer at a set
   ///interval.
   class CPeriodic* iTicker;
   ///The object that is queried for the current route status. 
   class MRouteDataProvider* iProvider;
   //@}
};

#endif
