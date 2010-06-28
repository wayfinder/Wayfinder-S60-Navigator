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

#ifndef MAPLIBUTIL_H
#define MAPLIBUTIL_H

#include <e32std.h>

#include "MultiFileDBufRequester.h"

class DBufRequester;
class MapLib;
class MemTracker;
class RFs;
class SharedBuffer;

/**
 *   Contains a function for MapLib that is useful in wayfinder.
 */
class MapLibSymbianUtil {
public:

   /**
    *   Searches the disk and inserts caches found into MapLib.
    *   @param serv      A working file server session.
    *   @param maplib    Reference to the MapLib object.
    *   @param userName  The uin of the user.
    *   @param findWarez Find maps that are encrypted with the fixed key.
    *   @return False if username empty when not looking for warez.
    */
   static bool insertPrecachedMapsL( RFs& serv,
                                     MapLib& maplib,
                                     const TDesC& userName,
                                     int findWarez );
   
   /**
    *   Searches the disk and inserts caches found there as parents
    *   to the supplied requester. The old parent of topReq will
    *   become the parent of the newly inserted ones.
    *   @deprecated
    *   @param fsSession      A file server session.
    *   @param topReq         The old (and new) top-level requester.
    *   @param userName       Non-empty username.
    *   @param findWarez      Find the maps that are encrypted with the
    *                         fixed key.
    *   @param xorBuffer      Pointer to a pointer to the xorbuffer,
    *                         which must be saved. Different ones must
    *                         be used for warez and ordinary.
    *   @param memTracker     Memory tracking object for the caches.
    *   @return If username is empty -> return false.
    */
   static bool
      insertPrecachedMapsAsParentsL( RFs& fsSession,
                                     DBufRequester* topReq,
                                     TDesC& userName,
                                     int findWarez,
                                     SharedBuffer** xorBuffer,
                                     MemTracker* memTracker );
   
};

#endif
