/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <e32def.h>
#include <e32std.h>
//#include "snprintf.h"
#include <stdio.h>
#include <string.h>
#include "GetIMEI.h"


#if defined __WINS__ || defined __WINSCW__ || defined NAV2_CLIENT_SERIES80 || defined NAV2_CLIENT_SERIES90_V1 || defined NAV2_CLIENT_SERIES60_V3 || defined NAV2_CLIENT_UIQ3

# define IMEIFILENAME "imei.txt"
# define DELIM '\\'
# include "../factored/GetIMEI.impl"

#elif 1

# include <plpvariant.h>

namespace isab{
   char* GetIMEI( const char* dataPath )
   {
      char* retval = NULL;
      CTrapCleanup *tc = CTrapCleanup::New();
      if(tc){
         TPlpVariantMachineId aId;
         TRAPD(result, PlpVariant::GetMachineIdL(aId));
         delete tc;
         if(result == KErrNone){
            int len = aId.Length();
            retval = new char[len + 1];
            for(int i = 0; i < len; ++i){
               retval[i] = aId[i];
            }
            retval[len] = '\0';
            if(strlen(retval) != strspn(retval, "0123456789-")){
               delete[] retval;
               retval = NULL;
            }
         }
      }
      return retval;
   }
}

#else

# include <etel3rdparty.h>

char* isab::GetIMEI( const char* dataPath )
{
   char* retval = NULL;
   class CTrapCleanup *tc = CTrapCleanup::New();
   if(tc){
      class TTrap trap;
      TInt leaveValue = KErrNone;
      if(trap.Trap(leaveValue) == 0){
         CTelephony::TPhoneIdV1 v1;
         CTelephony::TPhoneIdV1Pckg pkg(v1);
         class CTelephony* telephony = CTelephony::NewLC();
         class TRequestStatus imeiStatus;
         telephony->GetPhoneId(imeiStatus, pkg);
         User::WaitForRequest( imeiStatus );
         if(imeiStatus == KErrNone){
            const TDesC& imei = v1.iSerialNumber;
            int len = imei.Length();
            retval = new (ELeave) char[len + 1]; //don't Push
            for(int i = 0; i < len; ++i){
               retval[i] = imei[i];
            }
            retval[len] = '\0';
            if(strlen(retval) != strspn(retval, "0123456789-")){
               delete[] retval;
               retval = NULL;
            }
         }
         CleanupStack::PopAndDestroy(telephony);
         TTrap::UnTrap();
      }
   }
   delete tc;
   return retval;
}
      

#endif
