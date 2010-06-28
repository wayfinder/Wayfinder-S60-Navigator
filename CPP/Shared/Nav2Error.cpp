/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#include "Nav2Error.h"

namespace isab {

namespace Nav2Error{

   Nav2ErrorTable::Nav2ErrorTable() : 
      m_table(0), m_tableSize(0)
   {}

   Nav2ErrorTable::~Nav2ErrorTable()
   {}


bool Nav2ErrorTable::checkErrorTable() const
{
   bool result = false;

   result = m_table &&  
      ( m_table[m_tableSize-1].errorNbr == 
        INVALID_ERROR_NBR);

   return result;
} // checkErrorTable

const char* Nav2ErrorTable::getErrorString(enum ErrorNbr errorNbr)const
{
   const char* result = NULL;

   //XXX Linear seach. Change to binary or STL!
   bool found = false;
   uint32 i = 0;
   while ( (!found) && (i < m_tableSize) ){
      found = (m_table[i].errorNbr == errorNbr);

      if (!found){
         i++;
      }
   }

   if (found){
      result = m_table[i].errorString;
   }

   return result;
} // getErrorString

bool Nav2ErrorTable::acceptableServerError(uint32 errorNbr)
{
   using namespace isab;
   using namespace Nav2Error;

   uint32 allowedErrors[] = {
      NSC_SERVER_NEW_VERSION,
      NSC_EXPIRED_USER,
      NSC_AUTHORIZATION_FAILED,
      NSC_SERVER_NOT_OK,
      NSC_SERVER_REQUEST_TIMEOUT,
      NSC_SERVER_OUTSIDE_MAP,
      NSC_SERVER_PROTOCOL_ERROR,
      NSC_NO_TRANSACTIONS,
      NSC_UNAUTH_OTHER_HAS_LICENSE
   };
   uint32 size = (sizeof(allowedErrors)/sizeof(*allowedErrors));
   uint32 ok = 0;

   uint32 a;
   for(a = 0; a < size ; ++a) {
      if(allowedErrors[a] == errorNbr) {
         /* Matched an error that indicates that the server was reached. */
         ok = 1;
         break;
      }
   }
   return ok;
}


} // Nav2Error

} // isab
