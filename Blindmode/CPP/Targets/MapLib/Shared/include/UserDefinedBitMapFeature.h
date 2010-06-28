/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef USERDEFINEDBITMAPFEATURE_H
#define USERDEFINEDBITMAPFEATURE_H

#include "config.h"
#include "UserDefinedFeature.h"

class UserDefinedBitMapFeature : public UserDefinedFeature {
public:
   /**
    *   Creates a new UserDefinedBitMapFeature with the center
    *   at <code>center</code> using the bitmap with the
    *   supplied name on the server. The name should be given without
    *   suffix, e.g. "mappin" and must exist on the server.
    *   Avoid other characters than 7 bit-ascii.
    */
   UserDefinedBitMapFeature( const ScreenOrWorldCoordinate& center,
                             const char* serverBitMapName );

   /**
    *   Deletes the bitmap name.
    */
   ~UserDefinedBitMapFeature();

   /// Returns the server name of the bitmap for this feature.
   const char* getBitMapName() const;
   
private:
   /// The name of the bitmap on the server.
   char* m_bitMapName;
   
};

#endif
