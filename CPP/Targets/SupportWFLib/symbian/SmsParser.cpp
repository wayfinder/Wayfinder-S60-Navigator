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

#include <txtrich.h>

#include "SmsParser.h"
#include "WFTextUtil.h"
#include "GuiProt/Favorite.h"

#include "TraceMacros.h"

using namespace isab;

_LIT( KWayfinderSmsTag, "//WAYF" );
 
CSmsParser* CSmsParser::NewL()
{
   CSmsParser* self = CSmsParser::NewLC();
   CleanupStack::Pop(self);
   return self;
}

CSmsParser* CSmsParser::NewLC()
{
   CSmsParser* self = new (ELeave) CSmsParser();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CSmsParser::CSmsParser()
{
   WideSmsStringDelimiter = TChar('}');
}

void CSmsParser::ConstructL() 
{
}
  
CSmsParser::~CSmsParser()
{
}


_LIT(KSckl, "//SCKL3F6B");
_LIT(KWayf, "//WAYF");
TBool CSmsParser::IsWayfinderSms( const CRichText& aBody )
{
   TPtrC head(aBody.Read( 0, KSckl().Length()+KWayf().Length()) );
   if(head.Find(KSckl) == KErrNotFound) {
      if (head.Find(KWayf) == KErrNotFound) {
         return EFalse;
      }
      else {
         return ETrue;
      }
   }
   else {
      return ETrue;
   }
}


TBool CSmsParser::ParseSmsL( const TDesC& aMessageBody,
                             TDes& aError )
{
   //TRACE_FUNC();
   TBool result = EFalse;
   iMessageType = EWFSMSParseError;

   if( aMessageBody.Length() < (KSckl().Length()+KWayf().Length()+1) ){
      aError.Copy(_L("length prob")); //DEBUG
      return result;
   }

   TInt commandPos = 0;
   // Check which type of SMS it is.
   if( aMessageBody.Find(KSckl) == KErrNotFound) {
      if( aMessageBody.Find(KWayf) == KErrNotFound ){
         // Not ours.
         aError.Copy(_L("unknown cmd")); //DEBUG
         return result;
      }
      else{
         commandPos = KWayf().Length();
      }
   }
   else{
      commandPos = KSckl().Length()+KWayf().Length()+1;         
   }  

   // we know that the message starts with "//WAYF", check 7th char for type:
   switch (aMessageBody[commandPos])
   {
      case 'D':
      {
         result = ParseDestination( aMessageBody,
                                    commandPos+1,
                                    iDestinationDescription,
                                    iDestinationLat,
                                    iDestinationLon,
                                    iSignature,
                                    aError );
         break;
      }
      case 'R':
      {
         result = ParseRoute( aMessageBody,
                              commandPos+1,
                              iOriginDescription,
                              iOriginLat,
                              iOriginLon,
                              iDestinationDescription,
                              iDestinationLat,
                              iDestinationLon,
                              iSignature,
                              aError );
         break;
      }
      case 'F':
      {
         result = ParseFavoriteL( aMessageBody,
                                  commandPos+1,
                                  iDestinationDescription,
                                  iDestinationLat,
                                  iDestinationLon,
                                  iSignature,
                                  aError );
         break;
      }
      default:
      {
         iMessageType = EWFSMSUnknown;
         break;
      }
   }
   return result;
}


CSmsParser::WFSMSType CSmsParser::GetSmsType()
{
   return iMessageType;
}


const TDesC& CSmsParser::GetDestinationDescription()
{
   return iDestinationDescription;
}

TPoint CSmsParser::GetDestinationCoordinate()
{
   return TPoint( iDestinationLon, iDestinationLat );
}

const TDesC& CSmsParser::GetOriginDescription()
{
   return iOriginDescription;
}

TPoint CSmsParser::GetOriginCoordinate()
{
   return TPoint( iOriginLon, iOriginLat );
}

const TDesC& CSmsParser::GetSignature()
{
   return iSignature;
}

isab::Favorite* CSmsParser::GetFavoriteD()
{
   if( iMessageType == EWFSMSFavorite ){
      return iFavorite;
   }
   else{
      return NULL;
   }
}

TBool CSmsParser::ParseDestination( const TDesC& aMessageBody,
                                    TInt aCommandPos,
                                    TDes& aDescription,
                                    TInt32& aLat, TInt32& aLon,
                                    TDes& aSignature,
                                    TDes& aError )
{
   //TRACE_FUNC();
   TLex lex(aMessageBody.Mid(aCommandPos)); // ignore "//WAYFD"
   TChar destinationFlag = lex.Get();
   destinationFlag = destinationFlag;

   if( !ReadNextVal(lex, aLat) ){
      aError.Copy( _L("lat prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextVal(lex, aLon) ){
      aError.Copy( _L("lon prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextCheckNum(lex, aLat, aLon, aError) ){
      return EFalse;
   }
   TryInc(lex);
   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   TryInc(lex);
   aSignature = lex.Remainder();

   iMessageType = EWFSMSDestination; // everything went fine

   return ETrue;
}


TBool CSmsParser::ParseRoute( const TDesC& aMessageBody,
                              TInt aCommandPos,
                              TDes& aOrgDescription,
                              TInt32& aOrgLat, TInt32& aOrgLon,
                              TDes& aDestDescription,
                              TInt32& aDestLat, TInt32& aDestLon,
                              TDes& aSignature,
                              TDes& aError )
{
   //TRACE_FUNC();
   TLex lex(aMessageBody.Mid(aCommandPos)); // ignore "//WAYFR"

   if( !ReadNextVal(lex, aDestLat) ){
      aError.Copy( _L("lat prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextVal(lex, aDestLon) ){
      aError.Copy( _L("lon prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextVal(lex, aOrgLat) ){
      aError.Copy( _L("originlat prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextVal(lex, aOrgLon) ){
      aError.Copy( _L("originlon prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextCheckNum(lex, aDestLat, aDestLon, aError) ){
      return EFalse;
   }
   TryInc(lex);
   // origin name, ends with " \\ ".
   aOrgDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aOrgDescription.Append(lex.Get());
   }

   TryInc(lex);
   aDestDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDestDescription.Append(lex.Get());
   }

   TryInc(lex);
   aSignature = lex.Remainder();

   iMessageType = EWFSMSRoute; // everything went fine

   return ETrue;
}


TBool CSmsParser::ParseFavoriteL( const TDesC& aMessageBody,
                                  TInt aCommandPos,
                                  TDes& aDescription,
                                  TInt32& aLat, TInt32& aLon,
                                  TDes& aSignatur,
                                  TDes& aError  )
{
   //TRACE_FUNC();
   TLex lex(aMessageBody.Mid(aCommandPos)); // ignore "//WAYFF"

   if( !ReadNextVal(lex, aLat) ){
      aError.Copy( _L("lat prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextVal(lex, aLon) ){
      aError.Copy( _L("lon prob") );
      return EFalse;
   }
   TryInc(lex);
   if( !ReadNextCheckNum(lex, aLat, aLon, aError) ){
      return EFalse;
   }
   TryInc(lex);

   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   char* name = WFTextUtil::newTDesDupL(aDescription);

   TryInc(lex);
   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   char* shortname = WFTextUtil::newTDesDupL(aDescription);

   TryInc(lex);
   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   char* description = WFTextUtil::newTDesDupL(aDescription);

   TryInc(lex);
   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   char* category = WFTextUtil::newTDesDupL(aDescription);

   TryInc(lex);
   aDescription.Copy( _L("") );
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)){
      aDescription.Append(lex.Get());
   }
   char* mapiconname = WFTextUtil::newTDesDupL(aDescription);

   // also set the description of this sms.
   WFTextUtil::char2TDes(aDescription, name);

   TryInc(lex);
   aSignatur = lex.Remainder();

   iFavorite = NULL;
   iFavorite = new Favorite( iDestinationLat,
                             iDestinationLon,
                             name,
                             shortname,
                             description,
                             category,
                             mapiconname );
   delete[] name;
   delete[] shortname;
   delete[] description;
   delete[] category;
   delete[] mapiconname;

   iMessageType = EWFSMSFavorite; // everything went fine

   return ETrue;
}


TBool CSmsParser::ReadNextVal( TLex& lex, TInt32& val )
{
   TBool result = EFalse;
   lex.Mark();
   lex.SkipCharacters();
   if (lex.TokenLength() > 0) {
      TBuf<100> b(lex.MarkedToken());
      TLex l(b);
      if( l.Val(val) == KErrNone ){
         result = ETrue;
      }
   }
   return result;
}


_LIT( KNumErr, "Expected checknum: %i received: %i" );
TBool CSmsParser::ReadNextCheckNum( TLex& lex, TInt32 aLat,
                                    TInt32 aLon, TDes& aError )
{
   TBool result = EFalse;
   TInt32 checkNumFromSMS;
   if( ReadNextVal(lex, checkNumFromSMS) ){
      TInt checkNumCalc = (aLat & 0xFF) ^ 
                          (aLon & 0xFF) ^ (TInt)'W';
      if( checkNumCalc != checkNumFromSMS ){
         aError.Format( KNumErr, checkNumCalc, checkNumFromSMS ); //DEBUG
      }
      else{
         result = ETrue;
      }
   }
   else{
      aError.Copy( _L("check num prob") );
   }
   return result;
}


void CSmsParser::TryInc(TLex& lex)
{
   if (!lex.Eos()) {
      lex.Inc();
   }
}
