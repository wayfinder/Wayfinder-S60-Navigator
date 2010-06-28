/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#include <uikon.hrh>

#ifdef NAV2_CLIENT_UIQ3
#include <qiksimpledialog.h>

//#define CANCEL_DIAL
#endif

#include "Symbian9CallHandler.h"
#include "WFTextUtil.h"

// #ifndef USE_TRACE
//  #define USE_TRACE
// #endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif


_LIT( KSymbian9CallHandlerPanic, "CSymbian9CallHandler" );

CSymbian9CallHandler* CSymbian9CallHandler::NewL( MLineStateListener* aListener,
                                                  TInt aHangUpDlgRsc )
{
   
   CSymbian9CallHandler* self = new (ELeave) CSymbian9CallHandler( aListener, aHangUpDlgRsc );
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop();
   return self;
}


CSymbian9CallHandler::CSymbian9CallHandler( MLineStateListener* aListener,
                                            TInt aHangUpDlgRsc )
: CActive(EPriorityStandard),
  iPhoneIdV1Pckg(iPhoneIdV1),
  iHasImei( EFalse ),
  iLineStatusPckg( iLineStatus )
{
   iListener = aListener;
   iHangUpDlgRsc = aHangUpDlgRsc;
   iState = EIdle;
   //default constructor
}


void CSymbian9CallHandler::ConstructL()
{
   iTelephony = CTelephony::NewL();
   CActiveScheduler::Add(this);
}


CSymbian9CallHandler::~CSymbian9CallHandler()
{
   Cancel();
   delete iTelephony;
}


void CSymbian9CallHandler::RunL()
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   switch( iState )
   {
   case EDialing:
   {
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EDialing");
    #endif
      if( iStatus == KErrNone ){
       #ifdef USE_TRACE
         TRACE_FUNC1("iStatus == KErrNone");
       #endif
         // The call has been dialled successfully;
         // iCallId contains the call's ID,
         // needed when controlling the call.
         iState = EConnected;
#if defined NAV2_CLIENT_UIQ3 && !defined CANCEL_DIAL
         CQikSimpleDialog* dialog = new (ELeave) CQikSimpleDialog();
         TInt ret = dialog->ExecuteLD( iHangUpDlgRsc );
         if(ret == EEikBidYes){
            HangUp();
         }
#endif
      }
      else{
         iState = EIdle;
         // Removed the call below since there is no meaning to do this,
         // If an error occures, the phone app will notify the user about this.
         //iListener->CallError( MLineStateListener::EErrorDialing );
       #ifdef USE_TRACE
         char *errStr = new char[32];
         sprintf( errStr, "iStatus = %i", iStatus.Int() );
         TRACE_FUNC1(errStr);
         delete errStr;
       #endif
      }
      break;
   }
   case EHangingUp:
   {
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EHangingUp");
    #endif
      iState = EIdle;
      break;
   }
   case EGettingId:
   {
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EGettingId");
    #endif
      iState = EIdle;
      if( iStatus == KErrNone ){
         iHasImei = ETrue;
      }
      break;
   }
   case EListening:
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EListening");
    #endif
   case EBusy:
   {
      iState = EBusy;
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EBusy");
    #endif
      if( iStatus == KErrNone ){
         switch( iLineStatus.iStatus )
         {
         case CTelephony::EStatusUnknown:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusUnknown");
          #endif
            break;
         }
         case CTelephony::EStatusIdle:
         {
            iState = EListening;
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusIdle");
          #endif
            iListener->LineStatusChanged( MLineStateListener::EDisconnected );
            break;
         }
         case CTelephony::EStatusDialling:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusDialling");
          #endif
            iListener->LineStatusChanged( MLineStateListener::EDialing );
            break;
         }
         case CTelephony::EStatusRinging:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusRinging");
          #endif
            iListener->LineStatusChanged( MLineStateListener::ERinging );
            break;
         }
         case CTelephony::EStatusAnswering:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusAnswering");
          #endif
            break;
         }
         case CTelephony::EStatusConnecting:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusConnecting");
          #endif
            iListener->LineStatusChanged( MLineStateListener::EConnecting );
            break;
         }
         case CTelephony::EStatusConnected:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusConnected");
          #endif
            iListener->LineStatusChanged( MLineStateListener::EConnected );
            break;
         }
         case CTelephony::EStatusReconnectPending:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusReconnectPending");
          #endif
            break;
         }
         case CTelephony::EStatusDisconnecting:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusDisconnecting");
          #endif
            iListener->LineStatusChanged( MLineStateListener::EDisconnected );
            break;
         }
         case CTelephony::EStatusHold:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusHold");
          #endif
            break;
         }
         case CTelephony::EStatusTransferring:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusTransferring");
          #endif
            break;
         }
         case CTelephony::EStatusTransferAlerting:
         {
          #ifdef USE_TRACE
            TRACE_FUNC1("Line status:EStatusTransferAlerting");
          #endif
            break;
         }
         }// switch
      }
      if( iStatus != KErrCancel ){
         // Request the next notification
         ListenForVoiceLineStatusChange( iState );
      }
      else{
       #ifdef USE_TRACE
         TRACE_FUNC1("iStatus == KErrCancel");
       #endif
      }
      break;
   }
   case EConnected:
   {
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EConnected");
    #endif
      break;
   }
   case EIdle:
   {
    #ifdef USE_TRACE
      TRACE_FUNC1("iState==EIdle");
    #endif
      //Should never happen
      __ASSERT_ALWAYS( EFalse, User::Panic( KSymbian9CallHandlerPanic, 3 ));
      break;
   }
   }//switch
}


void CSymbian9CallHandler::DoCancel()
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   TInt err;
   if( iState == EGettingId ){
    #ifdef USE_TRACE
      TRACE_FUNC1("iState == EGettingId");
    #endif
      err = iTelephony->CancelAsync( CTelephony::EGetPhoneIdCancel );
   }
   else if( iState == EDialing ){
    #ifdef USE_TRACE
      TRACE_FUNC1("iState == EDialing");
    #endif
      err = iTelephony->CancelAsync( CTelephony::EDialNewCallCancel );
   }
   else if( iState == EHangingUp ){
    #ifdef USE_TRACE
      TRACE_FUNC1("iState == EHangingUp");
    #endif
      err = iTelephony->CancelAsync( CTelephony::EHangupCancel );
   }
   else if( iState == EListening ){
    #ifdef USE_TRACE
      TRACE_FUNC1("iState == EListening");
    #endif
      err = iTelephony->CancelAsync( CTelephony::EVoiceLineStatusChangeCancel );
   }
   else{
    #ifdef USE_TRACE
      char stateStr[32];
      sprintf( stateStr, "iState = %i", iState );
      TRACE_FUNC1( stateStr );
    #endif
      err = iTelephony->CancelAsync( CTelephony::EVoiceLineStatusChangeCancel );
   }
 #ifdef USE_TRACE
   char errStr[48];
   sprintf( errStr, "err = %i, iStatus = %i", err, iStatus.Int() );
   TRACE_FUNC1( errStr );
 #endif
}


void CSymbian9CallHandler::MakeCall( const TDesC& aNumber )
{
   TBuf<32> phoneNumber(_L(""));
   for( TInt i=0; i < aNumber.Length(); i++ ){
      if( IsDialableCharacter( aNumber[i] ) ){
         phoneNumber.Append( aNumber[i] );
      }
   }
   phoneNumber.PtrZ();
   if( phoneNumber.Length() < 3 ){
      iListener->CallError( MLineStateListener::EInvalidPhoneNumber );
      return;
   }

   if( IsActive() ){
      Cancel();
   }
 #ifdef USE_TRACE
   char *num = WFTextUtil::newTDesDupL(phoneNumber);
   TRACE_FUNC1( num );
   delete num;
 #endif

   CTelephony::TTelNumber telNumber = CTelephony::TTelNumber( phoneNumber );
   CTelephony::TCallParamsV1 callParams;
   callParams.iIdRestrict = CTelephony::ESendMyId;
   CTelephony::TCallParamsV1Pckg callParamsPckg( callParams );

   iState = EDialing;
   iTelephony->DialNewCall( iStatus,
                            callParamsPckg,
                            telNumber,
                            iCallId );

   SetActive();
#if defined NAV2_CLIENT_UIQ3 && defined CANCEL_DIAL
   CQikSimpleDialog* dialog = new (ELeave) CQikSimpleDialog();
   TInt ret = dialog->ExecuteLD( iHangUpDlgRsc );
   if(ret == EEikBidYes){
      HangUp();
   }
#endif
}


void CSymbian9CallHandler::HangUp()
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   if( IsActive() ){
    #ifdef USE_TRACE
      TRACE_FUNC1( "IsActive()" );
    #endif
      Cancel();
      CTelephony::TCallStatusV1 callStatusV1;
      CTelephony::TCallStatusV1Pckg callStatusV1Pckg( callStatusV1 );
      CTelephony::TPhoneLine voiceLine = CTelephony::EVoiceLine;
      iTelephony->GetLineStatus( voiceLine, callStatusV1Pckg );
      CTelephony::TCallStatus voiceLineStatus = callStatusV1.iStatus;
      if( voiceLineStatus != CTelephony::EStatusIdle ){
         iListener->CallError( MLineStateListener::ELineBusy );
      }
   }
   else{
      iState = EHangingUp;
      iTelephony->Hangup(iStatus, iCallId);
    #ifdef USE_TRACE
      TRACE_FUNC1("iTelephony->Hangup()");
    #endif
      SetActive();
   }
}


void CSymbian9CallHandler::FindIMEI()
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   iState = EGettingId;
   iTelephony->GetPhoneId(iStatus, iPhoneIdV1Pckg);
   SetActive();
}


char* CSymbian9CallHandler::GetIMEI()
{
   char* retval = NULL;
   if( !iHasImei ){
      if( iState != EGettingId ){
         Cancel();
         FindIMEI();
      }
      TInt counter = 0;
      while( !iHasImei && counter < 10 ){
         User::After(1000);
         counter++;
      }
   }
   if( iHasImei ){
      retval = WFTextUtil::newTDesDupL( iPhoneIdV1.iSerialNumber );
   }
   return retval;
}


void CSymbian9CallHandler::ListenForVoiceLineStatusChange( TCallState aState )
{
 #ifdef USE_TRACE
   TRACE_FUNC();
 #endif
   if( IsActive() ){
      Cancel();
   } 
   // Panic if this object is already performing an asynchronous
   // operation
   //__ASSERT_ALWAYS( !IsActive(), User::Panic( KSymbian9CallHandlerPanic, 1 ));

   iTelephony->NotifyChange( iStatus,
                             CTelephony::EVoiceLineStatusChange,
                             iLineStatusPckg );
   iState = aState;
   SetActive();
}


TBool CSymbian9CallHandler::IsHookOn()
{
   TBool result = EFalse;
   if( iState == EListening || iState == EIdle ){
      result = ETrue;
   }
   return result;
}


TBool CSymbian9CallHandler::IsDialableCharacter( TChar aChar ) const
{
   switch( aChar ){
   case '+':
   case '0':
   case '1':
   case '2':
   case '3':
   case '4':
   case '5':
   case '6':
   case '7':
   case '8':
   case '9':
      return ETrue;
   }
   return EFalse;
}
