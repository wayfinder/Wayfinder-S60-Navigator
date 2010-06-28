/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _WFID_h_
#define _WFID_h_

#include "arch.h"
#include <algorithm>

/**
 * Struct for storing hardware id:s
 *
 * Usage:
 * Store hardvare id:s such as imsi and imei in this struct.
 * The value for the Id in the key value and the type in hardwaretype.
 */

namespace isab 
{
   // Class used for storing hardware id:s
   class HWID
   {
   public:  
      /**
       * Standard constructor that default inits member variables to NULL.
       */
      HWID() : 
         m_key(NULL), m_hardWareType(NULL) {}

      /**
       * Copy constructor.
       * @param hwId   The HWID object to copy.
       */
      HWID(const HWID& hwId);

      /**
       * Constructor that creates an object with the members variables set.
       * @param key       The key that this instance will keep.
       * @param thetype   The key type that this instance will be.
       */
      HWID(const char* key, const char* thetype);
      
      /**
       * Standard destructor.
       */
      ~HWID();

   public:
      // The key to the ID
      char* m_key;
      
      // What type is it, to start with imei or imsi
      char* m_hardWareType;
   };   

struct HWType: public std::binary_function<HWID*, const char*, bool>
{
   bool operator()(const HWID* hwid, const char* hwtype) const {
      return strcmp(hwid->m_hardWareType, hwtype) == 0;
   }
   bool operator()(const char* hwtype, const HWID* hwid) const {
      return strcmp(hwtype, hwid->m_hardWareType) == 0;
   }
};

// class HWIDTypeCmp
// {
// public:
//    bool operator()(const HWID& a, const HWID& b) {
//       return strcmp(a.m_hardWareType, b.m_hardWareType) == 0;
//    }
// };

} // End namespace isab
#endif
