/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPCOMPONENTWRAPPER_H
#define MAPCOMPONENTWRAPPER_H

class MapMovingInterface;
class MapDrawingInterface;
class SymbianBitmapDrawingInterface;
class CCoeControl;
class MapRectInterface;

/**
 *    Wrapper class for a map component.
 */
class MapComponentWrapper {

   public:
      /**
       *    Virtual destructor.
       */
      virtual ~MapComponentWrapper() {}

      /**
       *    Get the CCoeControl.
       */
      virtual CCoeControl& getControl() const = 0;

      /**
       *    Get the map rect interface.
       */
      virtual const MapRectInterface& getMapRectInterface() const = 0; 
      
      /**
       *    Get the MapMovingInterface.
       */
      virtual MapMovingInterface& getMapMovingInterface() const = 0;

      /**
       *    Get the MapDrawingInterface.
       */
      virtual MapDrawingInterface& getMapDrawingInterface() const = 0;

      /**
       *    Get the BitmapDrawingInterface.
       */
      virtual SymbianBitmapDrawingInterface& getBitmapDrawingInterface() const = 0;

      /**
       *    Get the minimum scale when this map component should be shown.
       */
      virtual int getMinScale() const = 0;
};

/**
 *    Creates the globe component.
 */
class GlobeCreator {
public:
   /**
    *   Create the globe component.
    */
   static MapComponentWrapper* createGlobe( const CCoeControl& container );
   
   /**
    *    Create the globe component with additional parameters.
    * 
    *    @param   container         The parent control.
    *    @param   aPath             The path to the resources.
    *    @param   aRect             The rectangle.
    *    @param   aLocationString   The location string e.g. 
    *                               "I'm currently in".
    *    @param   aLanguageCode     The two character language code.
    */
   static MapComponentWrapper* createGlobe( const CCoeControl& container,
                                            const char* aPath, 
                                            const TRect& aRect,
                                            const char* aLocationString,
                                            const char* aLanguageCode );
};


#endif

