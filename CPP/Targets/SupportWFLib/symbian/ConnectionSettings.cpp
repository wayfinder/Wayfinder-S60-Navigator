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

#include "ConnectionSettings.h"
#include "nav2util.h"

CConnectionSettings::CConnectionSettings() :
   iHost(0)
{
}

void
CConnectionSettings::ConstructL()
{
}
void
CConnectionSettings::ConstructL(const class CConnectionSettings& c)
{
   iIAP = c.GetIAP();
   iProxyUsed = c.IsProxyUsed();
   iHost = isab::strdup_new(c.GetProxyHost());
   iPort = c.GetProxyPort();

   if (c.GetUAString()) {
      iUAString = c.GetUAString()->AllocL();
   }
   if (c.IsUAStringSet()) {
      iUAStringSet = c.IsUAStringSet();
   }

   if (c.GetIAPName()) {
      iIAPName = c.GetIAPName()->AllocL();
   }
   if (CDesCArray* tmp = c.GetPreferredIAPs()) {
      iPreferredIAPs = new (ELeave) CDesCArrayFlat(tmp->Count());
      for (TInt i = 0; i < tmp->Count(); i++) {
         iPreferredIAPs->AppendL(tmp->MdcaPoint(i));
      }
   }
}

class CConnectionSettings*
CConnectionSettings::NewLC()
{
   class CConnectionSettings* self = new (ELeave) CConnectionSettings();
   CleanupStack::PushL( self );
   self->ConstructL();
   return self;
}

class CConnectionSettings*
CConnectionSettings::NewL()
{
   class CConnectionSettings* self = CConnectionSettings::NewLC();
   CleanupStack::Pop( self );
   return self;
}

class CConnectionSettings*
CConnectionSettings::NewLC(const class CConnectionSettings& c)
{
   class CConnectionSettings* self = new (ELeave) CConnectionSettings();
   CleanupStack::PushL( self );
   self->ConstructL(c);
   return self;
}
class CConnectionSettings*
CConnectionSettings::NewL(const class CConnectionSettings& c)
{
   class CConnectionSettings* self = CConnectionSettings::NewLC(c);
   CleanupStack::Pop( self );
   return self;
}

CConnectionSettings::~CConnectionSettings()
{
   delete iHost;
   delete iUAString;
   delete iIAPName;
   if(iPreferredIAPs) {
      iPreferredIAPs->Reset();
      delete iPreferredIAPs;
   }
}

void CConnectionSettings::SetIAP( TInt32 aIAP )
{
   iIAP = aIAP;
}

TInt32 CConnectionSettings::GetIAP() const
{
   return iIAP;
}

void CConnectionSettings::SetProxy(TBool aProxyUsed, const char* aHost, 
                                   TUint32 aPort )
{
   iProxyUsed = aProxyUsed;
   iHost = isab::strdup_new( aHost );
   iPort = aPort;
}

TBool CConnectionSettings::IsProxyUsed() const
{
   return iProxyUsed;
}

char* CConnectionSettings::GetProxyHost() const
{
   return iHost;
}

TUint32 CConnectionSettings::GetProxyPort() const
{
   return iPort;
}

void CConnectionSettings::SetUAString(const TDesC& aName, 
                                      const TDesC& aVersion)
{
   iUAString = HBufC::NewL( 1 + aName.Length() + aVersion.Length() );
   iUAString->Des().Copy( aName );
   _LIT( KIAPAndProxySettingsSpace, " " );
   if (aVersion.Compare(KNullDesC)) {
      iUAString->Des().Append( KIAPAndProxySettingsSpace );
      iUAString->Des().Append( aVersion );
   }
   iUAStringSet = true;
}

TBool CConnectionSettings::IsUAStringSet() const
{
   return iUAStringSet;
}

HBufC* CConnectionSettings::GetUAString() const
{
   return iUAString;
}

void
CConnectionSettings::SetIAPName(const TDesC& aName)
{
   iIAPName = aName.AllocL();
}

HBufC*
CConnectionSettings::GetIAPName() const
{
   return iIAPName;
}

void 
CConnectionSettings::SetPreferredIAPsL(const TDesC& aPreferredIAPs) 
{
   if(aPreferredIAPs.Length() > 0) {
      if(iPreferredIAPs) {
         iPreferredIAPs->Reset();
         delete iPreferredIAPs;
      }
      iPreferredIAPs = new (ELeave) CDesCArrayFlat(2);
      TLex lexer(aPreferredIAPs);
      while (!lexer.Eos()) {
         HBufC *tmp = lexer.NextToken().AllocLC();
         TChar ch = '_';
         TInt pos = tmp->Locate(ch);
         while (pos != KErrNotFound) {
            tmp->Des().Replace(pos, 1, _L(" "));
            pos = tmp->Locate(ch);
         }
         iPreferredIAPs->AppendL(*tmp);
         CleanupStack::PopAndDestroy(tmp);
      }
   }
}

CDesCArray*
CConnectionSettings::GetPreferredIAPs() const
{
   return iPreferredIAPs;
}
