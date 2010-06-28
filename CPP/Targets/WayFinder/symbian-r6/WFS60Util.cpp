/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <f32file.h>

#include "WFS60Util.h"

/* For WML browser id (KWmlcHandler) */
#include <documenthandler.h>
/* For TApaTask and TApaTaskList */
#include <apgtask.h>
/* For CEikonEnv */
#include <eikenv.h>
/* For RApaLsSession */
#include <apgcli.h>

#include "WFTextUtil.h"

const TUid KPhoneUidWmlBrowser = { KWmlcHandler };

void
WFS60Util::LaunchWapBrowserL(const TDesC& aAddr,
                                 RApaLsSession& aApaLsSession)
{
   TUid id( KPhoneUidWmlBrowser );
   TApaTaskList taskList( CEikonEnv::Static()->WsSession() );
   TApaTask task = taskList.FindApp( id );

   if ( task.Exists() ) {
      HBufC8* param8 = HBufC8::NewLC( aAddr.Length() );
      param8->Des().Append( aAddr );
      task.SendMessage( TUid::Uid( 0 ), *param8 ); // UID is not used
      CleanupStack::PopAndDestroy( param8 );
   } else {
      if ( !aApaLsSession.Handle() )
      {
         User::LeaveIfError( aApaLsSession.Connect() );
      }

      TThreadId thread;
      User::LeaveIfError( aApaLsSession.StartDocument( aAddr,
         KPhoneUidWmlBrowser, thread ) );
   }
}

void
WFS60Util::TDesReadResourceL(TDes& dst, TInt resource_id)
{ 
   TDesReadResourceL(dst, resource_id, CCoeEnv::Static());
}

void
WFS60Util::TDesReadResourceL(TDes& dst, TInt resource_id,class CCoeEnv* aCoeEnv)
{
   HBufC *tmp;
   tmp = aCoeEnv->AllocReadResourceLC(resource_id);
   WFTextUtil::TDesCopy(dst, *tmp);
   CleanupStack::PopAndDestroy(tmp);
}

