/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef NAV2_ERROR_H
#define NAV2_ERROR_H

#include "machine.h"

namespace isab {

   namespace Nav2Error{

      enum ErrorNbr {
#define NAV2ERROR_LINE(symbol, id, txt)  symbol=id,
#define NAV2ERROR_LINE_LAST(symbol, id, txt)  symbol=id
#include "Nav2Error.master"
#undef NAV2ERROR_LINE
#undef NAV2ERROR_LINE_LAST
      };

      /**
       * Struct which maps error numbers and error
       * strings together.
       */
      struct Nav2ErrorElement{ 
         enum ErrorNbr errorNbr;
         const char* errorString;
      } ;


      class Nav2ErrorTable {
      public:
         /**
          * This method checks that the last element in the error 
          * list has the INVALID_ERROR_NBR error number. If false
          * is returned something is wrong with the initiation of
          * the error list.
          *
          * @return True if the error list initiation seems ok, 
          *         otherwise false.
          */
         bool checkErrorTable() const;


         /**
          * Get the string associated with a specific error number.
          * 
          * @param  errorNbr The error number the string is returned
          *                  for.
          *
          * @return Returns a constant string or NULL if the error 
          *         number isn't present in the error list or an 
          *         invalid error number has been asked for.
          */
         const char* getErrorString(ErrorNbr errorNbr) const;

         /**
          * Return true if the error is one that indicates that the
          * server has been reached, but failed to process the request.
          *
          * @param errorNbr The error number to check against allowed list.
          *
          * @return Returns true if the error is in the list of errors which
          *         indicates that the server has been reached.
          */
         static bool acceptableServerError(uint32 errorNbr);

      protected:
         const Nav2ErrorElement * m_table;
         uint32 m_tableSize;
      protected:
         Nav2ErrorTable();
      public:
         virtual ~Nav2ErrorTable();
      };


   } // Nav2Error

} // isab






#endif // NAV2_ERRORS_H


