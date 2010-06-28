/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CONNECTION_SETTINGS_H
#define CONNECTION_SETTINGS_H

#include <e32base.h> //CBase
#include <badesca.h> //CDesCArray

class CConnectionSettings : public CBase
{
   CConnectionSettings();
   void ConstructL();
   void ConstructL(const class CConnectionSettings& c);
public:
   static class CConnectionSettings* NewLC();
   static class CConnectionSettings* NewL();

   static class CConnectionSettings* NewLC(const class CConnectionSettings& c);
   static class CConnectionSettings* NewL(const class CConnectionSettings& c);
   
   virtual ~CConnectionSettings();
public:
   /* IAP settings */
   void SetIAP( TInt32 aIAP );
   TInt32 GetIAP() const;

   /**
    * Initalizes a list of iaps to search for, 
    * all matches found should have highest 
    * priority when testing for iap.
    * The different iaps in aPreferredIAPs 
    * have to be space separated.
    */
   void SetPreferredIAPsL(const TDesC& aPreferredIAPs);
   CDesCArray* GetPreferredIAPs() const;
   
   /* Proxy settings */
   void SetProxy( TBool aProxyUsed, const char* aHost, TUint32 aPort );
   TBool IsProxyUsed() const;
   char* GetProxyHost() const; // does not transfer ownership
   TUint32 GetProxyPort() const;
   
   /* User Agent information */
   void SetUAString( const TDesC& aName, const TDesC& aVersion );
   TBool IsUAStringSet() const;
   HBufC* GetUAString() const; // does not transfer ownership
   void SetIAPName(const TDesC& aName);
   HBufC* GetIAPName() const;

private:
   /* IAP */
   TInt32   iIAP;
   
   /* Proxy */
   TBool    iProxyUsed;
   char*    iHost;   // Proxy host
   TUint32  iPort;   // Proxy port

   /* UA */
   HBufC*   iUAString;
   TBool    iUAStringSet;

   /* IAP Name. */
   HBufC*   iIAPName;

   /* List of preferred iaps */
   CDesCArray* iPreferredIAPs;
};

#endif
