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
#include "LaunchWapBrowserUtil.h"
/* For WML browser id (KWmlcHandler) */
#include <documenthandler.h>
/* For TApaTask and TApaTaskList */
#include <apgtask.h>
/* For CEikonEnv */
#include <eikenv.h>
/* For RApaLsSession */
#include <apgcli.h>

const TUid KPhoneUidWmlBrowser = { KWmlcHandler };

_LIT(KHttpString, "http://");
_LIT(KFourHttpString, "4 http://");
_LIT(KFourString, "4 ");

TBool
LaunchWapBrowserUtil::LaunchWapBrowser(const TDesC& aUrl)
{
#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V3
   TInt urlLen = aUrl.Length();
   HBufC* fixedUrl;

   if (KFourHttpString().Compare(aUrl.Left(KFourHttpString().Length()))) {
      /* Not 4 http:// at the beginning. */

      if (KHttpString().Compare(aUrl.Left(KHttpString().Length()))) {
         /* Not http:// at the beginning. */
         urlLen += KFourHttpString().Length() + 1;
         fixedUrl = HBufC::NewLC(urlLen);
         fixedUrl->Des().Copy(KFourHttpString);
      } else {
         urlLen += KFourString().Length() + 1;
         fixedUrl = HBufC::NewLC(urlLen);
         fixedUrl->Des().Copy(KFourString);
      }
      fixedUrl->Des().Append(aUrl);
   } else {
      fixedUrl = aUrl.AllocLC();
   }

   RApaLsSession aApaLsSession;

   TUid id( KPhoneUidWmlBrowser );
   TApaTaskList taskList( CEikonEnv::Static()->WsSession() );
   TApaTask task = taskList.FindApp( id );

   if ( task.Exists() ) {
      HBufC8* param8 = HBufC8::NewLC( fixedUrl->Length() );
      param8->Des().Append( *fixedUrl );
      task.SendMessage( TUid::Uid( 0 ), *param8 ); // UID is not used
      CleanupStack::PopAndDestroy( param8 );
   } else {
      if ( !aApaLsSession.Handle() )
      {
         User::LeaveIfError( aApaLsSession.Connect() );
      }
      TThreadId thread;
      User::LeaveIfError( aApaLsSession.StartDocument( *fixedUrl,
               KPhoneUidWmlBrowser, thread ) );
   }
   CleanupStack::PopAndDestroy(fixedUrl);

   aApaLsSession.Close();

#elif defined NAV2_CLIENT_SERIES60_V1
   return EFalse;
#else
# error This code not implemented!
#endif
   return ETrue;
}
