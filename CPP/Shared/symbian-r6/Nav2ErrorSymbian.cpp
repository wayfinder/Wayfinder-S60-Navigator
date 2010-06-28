/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "Nav2ErrorSymbian.h"
#include <barsread.h>
#include <eikenv.h>
#include <utf.h>

#include <vector>
#include "WFTextUtil.h"

namespace isab {
namespace Nav2Error {

   Nav2ErrorTableSymbian::Nav2ErrorTableSymbian()
   {
   }

   Nav2ErrorTableSymbian::Nav2ErrorTableSymbian(TInt aResourceId) : 
      Nav2ErrorTable()
   {
      //XXXX this is obviously wrong, but kept around for backwards
      //compatibility.
      ConstructFromResourceL(aResourceId);
   }

   void Nav2ErrorTableSymbian::ConstructFromResourceL(TInt aResourceId)
   {
      class TResourceReader reader;

      HBufC8* resource =
         CEikonEnv::Static()->AllocReadResourceAsDes8LC(aResourceId);
      reader.SetBuffer(resource);

      m_tableSize = reader.ReadUint16();
      struct Nav2ErrorElement *elem = new(ELeave)Nav2ErrorElement[m_tableSize];
      m_table = elem;

      for(TUint line = 0; line < m_tableSize; ++line) {
         TUint32 errorNum = reader.ReadUint32();
         TPtrC  errorText = reader.ReadTPtrC();
         elem->errorNbr = ErrorNbr(errorNum);
         elem->errorString = WFTextUtil::TDesCToUtf8L(errorText);
         ++elem;
      }

      CleanupStack::PopAndDestroy(resource);
   }

   class Nav2ErrorTableSymbian* Nav2ErrorTableSymbian::NewLC(TInt aResourceId)
   {
      class Nav2ErrorTableSymbian* self = new (ELeave) Nav2ErrorTableSymbian();
      CleanupStack::PushL(self);
      self->ConstructFromResourceL(aResourceId);
      return self;
   }

   class Nav2ErrorTableSymbian* Nav2ErrorTableSymbian::NewL(TInt aResourceId)
   {
      class Nav2ErrorTableSymbian* self = 
         Nav2ErrorTableSymbian::NewLC(aResourceId);
      CleanupStack::Pop(self);
      return self;
   }

   Nav2ErrorTableSymbian::~Nav2ErrorTableSymbian()
   {
      for(uint32 i = 0; i < m_tableSize; ++i){
         delete[] const_cast<char*>(m_table[i].errorString);
         const_cast<Nav2ErrorElement*>(m_table)[i].errorString = NULL;
      }
      delete[] const_cast<Nav2ErrorElement*>(m_table);
   }

} /* namespace Nav2Error */
} /* namespace isab */
