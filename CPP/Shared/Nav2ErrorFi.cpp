/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#define LANGUAGE_FI
#include "master.loc"
#include "Nav2Error.h"
#include "Nav2ErrorXX.h"


namespace isab {
namespace Nav2Error {

   static const Nav2ErrorElement nav2ErrorVector[] = {
#define NAV2ERROR_LINE(symbol, id, txt)  {ErrorNbr(id), txt},
#define NAV2ERROR_LINE_LAST(symbol, id, txt)  {ErrorNbr(id), txt}
#include "Nav2Error.master"
#undef NAV2ERROR_LINE
#undef NAV2ERROR_LINE_LAST
   }; 


   Nav2ErrorTableFi::Nav2ErrorTableFi() : Nav2ErrorTable()
   {
      int32 elementSize = (uint8*)&nav2ErrorVector[1] - 
         (uint8*)&nav2ErrorVector[0];

      m_table = nav2ErrorVector;
      m_tableSize = sizeof(nav2ErrorVector) / elementSize;
   }

} /* namespace Nav2Error */
} /* namespace isab */
