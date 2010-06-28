/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ERROROBJ_H
#define ERROROBJ_H

#include "arch.h"
#include "GuiProt/Serializable.h"
#include "Nav2Error.h"

namespace isab {


/** 
 * Holds error data.
 *
 */
class ErrorObj : public Serializable {
   public:
      /**
       * Constructor with explicit parameters.
       */
      ErrorObj( Nav2Error::ErrorNbr err, const char* extendedErrorString,
                const char* errorMessage );

      /**
       * Constructor from serialized object in buffer.
       */
      ErrorObj( Buffer* buf );

      /**
       * Constructor from only Nav2Error.
       */
      ErrorObj( Nav2Error::ErrorNbr err );

      /**
       * Copy constructor.
       */
      ErrorObj( const ErrorObj& o );

      /**
       * Destructor.
       */
      ~ErrorObj();

      /**
       * Writes this object to a buffer.
       *
       * @param buf The buffer to write this object to.
       */
      virtual void serialize( Buffer* buf ) const;

      /**
       * The size packed into a buffer.
       */
      virtual uint32 getSize() const;

      /**
       * Get the error code.
       */
      Nav2Error::ErrorNbr getErr() const;

      /**
       * Set the error code.
       */
      void setErr( Nav2Error::ErrorNbr err );

      /**
       * Get the error message.
       */
      const char* getErrorMessage() const;

      /**
       * Get the extended error string.
       */
      const char* getExtendedErrorString() const;

   private:
      /// The error code.
      Nav2Error::ErrorNbr m_err;

      /// The error string.
      char* m_errorMessage;

      /// The extened error string.
      char* m_extendedErrorString;

      /// Private to avoid use.
      const ErrorObj& operator = ( const ErrorObj& o );
};


} // End namespace

#endif // ERROROBJ_H
