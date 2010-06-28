/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ErrorObj.h"
#include "nav2util.h"
#include "Buffer.h"

namespace isab {


ErrorObj::ErrorObj( Nav2Error::ErrorNbr err, const char* extendedErrorString,
                const char* errorMessage )
      : Serializable( NULL/*Buffer*/ )
{
   m_err = err;
   m_errorMessage = strdup_new( errorMessage );
   m_extendedErrorString = strdup_new( extendedErrorString );
}

ErrorObj::ErrorObj( Buffer* buf ) 
      : Serializable( buf )
{
   m_err = Nav2Error::ErrorNbr( buf->readNext32bit() );
   m_errorMessage = buf->getNextCharStringAlloc();
   m_extendedErrorString = buf->getNextCharStringAlloc();
}

ErrorObj::ErrorObj( Nav2Error::ErrorNbr err )
      : Serializable( NULL/*Buffer*/ )
{
   m_err = err;
   m_errorMessage = strdup_new( "" );
   m_extendedErrorString = strdup_new( "" );
}

ErrorObj::ErrorObj( const ErrorObj& o ) {
   m_err = o.m_err;
   m_errorMessage = strdup_new( o.m_errorMessage );
   m_extendedErrorString = strdup_new( o.m_extendedErrorString );
}

ErrorObj::~ErrorObj() {
   delete [] m_errorMessage;
   delete [] m_extendedErrorString;
}


void
ErrorObj::serialize( Buffer* buf ) const {
   buf->writeNext32bit( m_err );
   buf->writeNextCharString( m_errorMessage );
   buf->writeNextCharString( m_extendedErrorString );
}

uint32
ErrorObj::getSize() const {
   return 4 + strlen( m_errorMessage ) + 1 + strlen( m_errorMessage ) + 1;
}

Nav2Error::ErrorNbr
ErrorObj::getErr() const {
   return m_err;
}

void 
ErrorObj::setErr( Nav2Error::ErrorNbr err ) {
   m_err = err;
}

const char*
ErrorObj::getErrorMessage() const {
   return m_errorMessage;
}

const char*
ErrorObj::getExtendedErrorString() const {
   return m_extendedErrorString;
}

} // End namespace isab
