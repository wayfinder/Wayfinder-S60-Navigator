/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <e32std.h>
#include "arch.h"
#include "GetProxy.h"
#include <commdb.h>
#include "WFTextUtil.h"

namespace isab {
   bool GetProxy( char*& aHost, uint32& aPort, const int32 aIAP )
   { 
# ifdef NAV2_CLIENT_SERIES60_V2
      CCommsDatabase * comdb = CCommsDatabase::NewL();
# else
      CCommsDatabase * comdb = CCommsDatabase::NewL( EDatabaseTypeUnspecified );
# endif
      CleanupStack::PushL( comdb );
   
      // First get the IAP
      CCommsDbTableView* iaptable = comdb->OpenViewMatchingUintLC( TPtrC( IAP ), 
                                                                TPtrC( COMMDB_ID ), aIAP );
      TInt iapres = iaptable->GotoFirstRecord();
      bool found = false;
      if ( iapres == KErrNone ) {
         HBufC* iap_name = iaptable->ReadLongTextLC( TPtrC( COMMDB_NAME) );
         uint32 iap_service = 0;
         iaptable->ReadUintL( TPtrC( IAP_SERVICE ), iap_service );
            
         // The current IAP exists!
         HBufC* iap_type = iaptable->ReadLongTextLC( TPtrC( IAP_SERVICE_TYPE ) );
      
         // Find Proxy for ISP (and same service type)
         CCommsDbTableView* proxytable = comdb->OpenViewMatchingUintLC( TPtrC( PROXIES ), 
                                                                     TPtrC( PROXY_ISP ), iap_service );
         TInt dretval= proxytable->GotoFirstRecord();
         while ( dretval == KErrNone && !found ) {
            // Check if matching proxy service type
            HBufC* proxy_service_type = proxytable->ReadLongTextLC( TPtrC( PROXY_SERVICE_TYPE ) );
            if ( proxy_service_type != NULL &&
               proxy_service_type->CompareC( *iap_type ) == 0 ) 
            {
               // Match!
               // PROXY_USE_PROXY_SERVER 
               TBool proxy_use_proxy_server = 0;
               proxytable->ReadBoolL( TPtrC( PROXY_USE_PROXY_SERVER ), proxy_use_proxy_server );
# ifdef NAV2_CLIENT_SERIES60_V2
               if ( proxy_use_proxy_server ) {
# endif
               // PROXY_SERVER_NAME - Name of the proxy server
               HBufC* proxy_server_name = proxytable->ReadLongTextLC( 
               TPtrC( PROXY_SERVER_NAME ) );
               if ( proxy_server_name ) {
                  found = true;
                  // Convert to something we can use.
                  aHost = WFTextUtil::TDesCToUtf8L( proxy_server_name->Des() );
                  proxytable->ReadUintL( TPtrC( PROXY_PORT_NUMBER ), aPort );
               
                  // Sanity on port
                  if ( aPort == 9201 ) {
                     // We don't talk wap
                     // XXX: Or no proxy at all?
                     aPort = 8080;
                  } else if ( aPort == 0 ) {
                     // Not valid => no proxy
                     found = false;
                     delete [] aHost;
                     aHost = NULL;
                  }
               } // End if have proxy_server_name
                     
               CleanupStack::PopAndDestroy( proxy_server_name );
# ifdef NAV2_CLIENT_SERIES60_V2
               } // End if proxy_use_proxy_server is true
# endif
            } // End if service type matches
            CleanupStack::PopAndDestroy( proxy_service_type );
            dretval = proxytable->GotoNextRecord(); // next proxy
         } // End while all proxies

         // XXX: Perhaps "IAP_SERVICE_TYPE" table -> [GPRS|ISP]_IP_GATEWAY 
         // especially in s60v1
         CleanupStack::PopAndDestroy( proxytable );
         CleanupStack::PopAndDestroy( iap_type );
         CleanupStack::PopAndDestroy( iap_name );
      } // End if the current IAP is found
      
      CleanupStack::PopAndDestroy( iaptable );
      CleanupStack::PopAndDestroy( comdb );
     
      return found;   
   }
}
