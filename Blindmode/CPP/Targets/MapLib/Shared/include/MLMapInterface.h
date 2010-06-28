/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MLMAPINTERFAFACE_H
#define MLMAPINTERFAFACE_H

#include "config.h"

class MC2Coordinate;
class MC2Point;

/**
 *   Interface for movement, map information.
 *   All strings are utf-8.
 */
class MLMapInterface {
public:
   /**
    *    Sets the center of the screen to the
    *    supplied coordinate.
    */
   virtual void setCenter( const MC2Coordinate& center ) = 0;

   /**
    *    Sets the scale of the map to the supplied value.
    *    Unit is meters per pixel on a S60v2 device.
    */
   virtual void setScale( double scale ) = 0;

   /**
    *    Returns the currently active coordinate.
    */
   virtual MC2Coordinate getCoord() const = 0;

   /**
    *    Returns a string to be sent to the server
    *    when more information about the currently
    *    active coordinate is to be requested.
    *    The string is only valid until the next call
    *    to this function.
    *    Empty string means no information.
    */
   virtual const char* getServerString() const = 0;

   /**
    *    Set the GPS-coordinate. To be used in or out
    *    of tracking mode to move the "car".
    */
   virtual void setGPSPosition( const MC2Coordinate& gpsPos ) = 0;

   /**
    *    Adds a bitmap to the map at the specified coordinate.
    *    @param bitmap  Name of the bitmap on the server.
    *    @param coord   Coordinate of the center of the bitmap.
    *    @param info    Optional information.
    *    @return An id to be used e.g. when removing the bitmap from
    *            the map.
    */
   virtual int addUserDefBitmap( const char* bitmap,
                                 const MC2Coordinate& coord,
                                 const char* info = "" ) = 0;

   /**
    *    Removes a user defined bitmap with the supplied id.
    *    @param id ID of the bitmap as returned by addUserDefBitmap.
    *    @return -1 if not found, else the sent-in id.
    */
   virtual int removeUserDefBitmap( int id ) = 0;

   /**
    *    Removes all the user defined bitmaps.
    */
   virtual void clearUserDefBitmaps() = 0;
   
};

#endif
