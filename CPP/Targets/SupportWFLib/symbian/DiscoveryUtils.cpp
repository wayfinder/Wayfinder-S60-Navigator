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

#include "DiscoveryUtils.h"
#ifdef SYMBIAN_CKON
# include <cknenv.h>
#endif
#include <coeutils.h>

TBool IfEmulator(TBool aIfEmulator){
#ifdef __WINS__
   return aIfEmulator;
#else
   return EFalse;
#endif
}

_LIT(KSeries60v20File, "z:\\system\\install\\Series60v2.0.sis");
_LIT(KSeries60v21File, "z:\\system\\install\\Series60v2.1.sis");
_LIT(KSeries60v26File, "z:\\system\\install\\Series60v2.6.sis");
_LIT(KSeries60v28File, "z:\\system\\install\\Series60v2.8.sis");

TBool IsSymbian7s()
{
#if (!defined SYMBIAN_7S) || defined SYMBIAN_9
   return EFalse;
#elif defined NAV2_CLIENT_SERIES60_V2
   return ConeUtils::FileExists(KSeries60v20File) && 
      !ConeUtils::FileExists(KSeries60v26File);
#else
   return ETrue;
#endif
}

TBool IsS60Symbian7s()
{
#ifdef SYMBIAN_AVKON
   return IsSymbian7s();
#else
   return EFalse;
#endif
}

TBool IsCommunicator()
{
#ifdef SYMBIAN_CKON
   return CknEnv::Skin().Constant(ESkinConstDialogButtonsOnCBA);
#else
   return EFalse;
#endif
}

MCommunicatorTest::MCommunicatorTest() : 
   iIsCommunicator(::IsCommunicator()) 
{}

TBool MCommunicatorTest::IsCommunicator() const
{
   return iIsCommunicator;
}
