/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include <eikenv.h>
#include <etel3rdparty.h>

#include "GuiProt/ServerEnums.h"
#include "DebugSms.h"

HBufC*
DebugSms::FormatLC(
         isab::GuiProtEnums::WayfinderType aWfType,
         const TDesC &aClientType, const TDesC &aClientOption,
         const TDesC &uid,
         uint32 errNbr,
         int32 lat, int32 lon, int32 speed, int32 angle,
         const TDesC &viewStr,
         TUint high, TUint low,
         TInt aMajor, TInt aMinor, TInt aBuild,
         const TDesC &errTxt,
         const HBufC* aImeiNum)
{

   HBufC* realSms = HBufC::NewLC(180);

   HBufC* sms = HBufC::NewLC(1024);
#if defined __WINS__ || defined __WINSCW__
   sms->Des().Copy( _L("//WAYFE") );
   sms->Des().Append(_L("wins"));
#else
   sms->Des().Copy( _L(""));
   if (aImeiNum) {
      sms->Des().Append( *aImeiNum );
   }
#endif

   sms->Des().Append( ';' );
   sms->Des().AppendNum(static_cast<int32>(aWfType), EDecimal);
   sms->Des().Append( ';' );
   sms->Des().Append(uid);
   sms->Des().Append( ';' );
   sms->Des().AppendNum(static_cast<int32>(errNbr), EHex);
   sms->Des().Append( ';' );
   if (lat == MAX_INT32) {
      lat=0;
   }
   if (lon == MAX_INT32) {
      lon=0;
   }
   sms->Des().AppendNum((TInt)lat);
   sms->Des().Append( ';' );
   sms->Des().AppendNum((TInt)lon);
   sms->Des().Append( ';' );
   sms->Des().AppendNum(speed, EDecimal);
   sms->Des().Append( ';' );
   sms->Des().AppendNum((TInt)angle);
   sms->Des().Append( ';' );
   sms->Des().Append(viewStr);
   sms->Des().Append( ';' );
   sms->Des().AppendNum(high, EDecimal);
   sms->Des().Append( ';' );
   sms->Des().AppendNum(low, EDecimal);
   sms->Des().Append( ';' );
   sms->Des().AppendNum(aMajor, EDecimal);
   sms->Des().Append( '.' );
   sms->Des().AppendNum(aMinor, EDecimal);
   sms->Des().Append( '.' );
   sms->Des().AppendNum(aBuild, EDecimal);
   sms->Des().Append( ';' );
   sms->Des().Append(aClientType);
   sms->Des().Append( ';' );
   sms->Des().Append(aClientOption);
   sms->Des().Append( ';' );
   sms->Des().Append(errTxt);

   if (sms->Length() > 160) {
      realSms->Des().Copy(sms->Left(160));
   } else {
      realSms->Des().Copy(*sms);
   }
   CleanupStack::PopAndDestroy(sms);
   return realSms;
}

