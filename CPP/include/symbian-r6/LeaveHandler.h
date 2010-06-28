/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <e32base.h>
#include <eikenv.h>

             
namespace LeaveHandler {
   
   /**
    * Print out a message in following format.
    * Title = header.
    * Message = Function name (aFunction)
    *           State: aState     
    *           Error code: aErrorCode
    */
   template<typename T>
   void ProcessLeave(CEikonEnv* aEikEnv, const TDesC& aHeader, TInt aErrorCode, 
                     TInt aState, const TDesC& aFunction, T& aCallback)
   {
      _LIT(KFormatParams, "%S%S%d%S%d");
      _LIT(KErrCode, "\nError code: ");
      _LIT(KState, "\nState: ");
      TBuf<128> message;
      message.Format(KFormatParams, &aFunction, &KState(), 
                     aState, &KErrCode(), aErrorCode);
      aEikEnv->InfoWinL(aHeader, message);
      aCallback.LeaveHandled();
   }

   /**
    * Print out a message in following format.
    * Title = header.
    * Message = Function name (aFunction)
    *           Error code: aErrorCode
    */
   template<typename T>
   void ProcessLeave(CEikonEnv* aEikEnv, const TDesC& aHeader, TInt aErrorCode, 
                     const TDesC& aFunction, T& aCallback)
   {
      _LIT(KFormatParams, "%S%S%d");
      _LIT(KErrCode, "\nError code: ");
      TBuf<128> message;
      message.Format(KFormatParams, &aFunction, &KErrCode(), aErrorCode);
      aEikEnv->InfoWinL(aHeader, message);
      aCallback.LeaveHandled();
   }
}
