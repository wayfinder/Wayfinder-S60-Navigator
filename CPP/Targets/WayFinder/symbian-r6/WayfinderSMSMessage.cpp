/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef NAV2_CLIENT_SERIES60_V3

#include <smut.h>
#include <txtrich.h>
#include <biouids.h>

#if defined BIOCONTROL && ! defined NO_LOG_OUTPUT
# define NO_LOG_OUTPUT
#endif
#include "WFTextUtil.h"
#include "WayfinderSMSMessage.h"
#include "memlog.h"
#include "BioConfig.h"

#include "GuiProt/Favorite.h"
#include "WFTextUtil.h"
#include "Dialogs.h"
#include "LogMem.h"
#include "TraceMacros.h"
#include <mtclbase.h>
#include <mtclreg.h>

namespace isab {
   class Nav2;
};

const TInt32 KUidWayfinderBIOMessage = { UID_WAYFINDER_BIOMESSAGE };

WayfinderSMSMessage*
WayfinderSMSMessage::NewL(isab::Log* iLog, CBaseMtm* mtm)
{
   // TRACE_DBG( "NewL" );
   WayfinderSMSMessage* self = new(ELeave) WayfinderSMSMessage(mtm);
   self->iLog = iLog;
   LOGNEW(self, WayfinderSMSMessage);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

WayfinderSMSMessage::WayfinderSMSMessage(CBaseMtm* mtm)
{
   // TRACE_DBG( "WayfinderSMSMessage constructor" );
   m_messageType = EWFSMSNoMessage;
   m_smsMtm = mtm;
/*    m_sendAs = NULL; */
}

void
WayfinderSMSMessage::ConstructL()
{
   // TRACE_DBG( "ConstructL" );
   m_messageID = 0;
   if (m_smsMtm == NULL) {
/*       TRACE_FUNC(); */

/*       CSendAs* m_sendAs = 0; */

/*       TRAPD(err, m_sendAs = CSendAs::NewL(*this)); */
/*       TRACE_FUNC(); */
/*       if (err == KErrNone) { */
/*          TRACE_FUNC(); */
/*          m_sendAs->SetMtmL(KUidMsgTypeSMS); */
/*          TRACE_FUNC(); */
/*          if (m_sendAs->AvailableServices().Count() < 1) */
/*          { */
/*             TRACE_FUNC(); */
/*             m_messageType = EWFSMSUnknown; */
/*             m_smsMtm = NULL; */
            // User::Leave(KErrNotFound);
/*          } */
/*          TRACE_FUNC(); */
/*          m_sendAs->SetService(0); */
/*          TRACE_FUNC(); */
/*          m_smsMtm = m_sendAs->ClientRegistry().NewMtmL(KUidMsgTypeSMS); */
/*          TRACE_FUNC(); */
/*       } */
   }
   m_favorite = NULL;
   m_senderDetails = NULL;
   
   WideSmsStringDelimiter = TChar('}');
}

WayfinderSMSMessage::~WayfinderSMSMessage()
{
   // TRACE_DBG( "~WayfinderSMSMessage" );
   // m_smsMtm is only borrowed do not delete
/*    if (m_sendAs != NULL){ */
/*       LOGDEL(m_sendAs); */
/*       delete m_sendAs; // causes KERN-EXEC 3 !?!? */
/*    } */
   delete m_favorite;
   delete m_senderDetails;
}


WayfinderSMSMessage::WFSMSType
WayfinderSMSMessage::getType() 
{
   // TRACE_DBG( "getType" );
   return m_messageType;
}

TInt32
WayfinderSMSMessage::getDestinationLat()
{
   // TRACE_DBG( "getDestinationLat" );
   return m_destinationLat;
}

TInt32
WayfinderSMSMessage::getDestinationLon()
{
   // TRACE_DBG( "getDestinationLon" );
   return m_destinationLon;
}

TInt32
WayfinderSMSMessage::getOriginLat()
{
   // TRACE_DBG( "getOriginLat" );
   return m_originLat;
}

TInt32
WayfinderSMSMessage::getOriginLon()
{
   // TRACE_DBG( "getOriginLon" );
   return m_originLon;
}

TDesC&
WayfinderSMSMessage::getDestinationDescription()
{
   // TRACE_DBG( "getDestinationDescription" );
   return m_destinationDescription;
}

TDesC&
WayfinderSMSMessage::getOriginDescription()
{
   // TRACE_DBG( "getOriginDescription" );
   return m_originDescription;
}

TDesC&
WayfinderSMSMessage::getSignature()
{
   // TRACE_DBG( "getSignature" );
   return m_signature;
}

TChar
WayfinderSMSMessage::getDestinationFlag()
{
   // TRACE_DBG( "getDestinationFlag" );
   return m_destinationFlag;
}

TBool
WayfinderSMSMessage::deleteMessage()
{
   // TRACE_DBG( "deleteMessage" );
  TInt l;
  if (m_messageID == 0)
     return EFalse;

// something like this:
  TRAP(l, m_smsMtm->SwitchCurrentEntryL(KMsvGlobalInBoxIndexEntryId));
  if (l != KErrNone) 
      return EFalse;
  TRAP(l, m_smsMtm->Entry().DeleteL(m_messageID));
  if (l != KErrNone) 
      return EFalse;

  m_messageID = 0;
  return ETrue;
}

TPtrC 
WayfinderSMSMessage::getDebugBody()
{
   // TRACE_DBG( "getDebugBody" );
   return m_messageBody;
}

class isab::Favorite* 
WayfinderSMSMessage::getFavoriteD()
{
   // TRACE_DBG( "getFavoriteD" );
   class isab::Favorite* f = m_favorite;
   m_favorite = NULL;
   return f;
}

const class isab::Favorite* 
WayfinderSMSMessage::peekFavorite() const
{
   // TRACE_DBG( "peekFavorite" );
   return m_favorite;
}

void
WayfinderSMSMessage::parseMessage(TMsvId messageID)
{
   // TRACE_DBG( "parseMessage" );
   m_messageID = messageID;
   if (m_smsMtm == NULL) {
      m_messageType = EWFSMSError;
      // TRACE_DBG( "" );
      return;
   }
   {
      TInt l;
      TRAP(l, m_smsMtm->SwitchCurrentEntryL(m_messageID));
      if (l != KErrNone) {
         m_messageType = EWFSMSError;
         m_destinationDescription = _L("switchentry prob"); //DEBUG
         return;
      } else {
         TRAP(l, m_smsMtm->LoadMessageL());
         if (l != KErrNone) {
            m_messageType = EWFSMSError;
            m_destinationDescription = _L("loadmsg prob"); //DEBUG
            return;
         }
      }
   }
#if ! (defined __WINS__ || defined __WINSCW__)
   {
      class TMsvEntry entry = m_smsMtm->Entry().Entry();
      if(!((entry.iMtm == KUidMsgTypeSMS) || 
           ((entry.iMtm == KUidBIOMessageTypeMtm) && 
            (entry.iBioType == KUidWayfinderBIOMessage)))){
         m_messageType = EWFSMSNotWFMessage;
         return;
      }
   }
#endif
   m_senderDetails = m_smsMtm->Entry().Entry().iDetails.AllocL();
   m_messageBody.Set(m_smsMtm->Body().Read(0, m_smsMtm->Body().DocumentLength())); // perhaps Extract(0) instead?
   // sender could be found in:  smsMtm->Entry().Entry().iDetails

   parseSelf();
}


_LIT(Ksckl, "//SCKL3F6B");
_LIT(Kwayf, "//WAYF");

void
WayfinderSMSMessage::parseSelf()
{
   // TRACE_DBG( "parseSelf" );
   m_messageType = EWFSMSParseError; // pessimistic outlook...

   if (m_messageBody.Length() < (Ksckl().Length()+Kwayf().Length()+1)) {
      // no need to even attempt to parse
      m_destinationDescription = _L("length prob"); //DEBUG
      return;
   }
   TInt commandPos = 0;
   /* Check which type of SMS it is. */
   if (m_messageBody.Find(Ksckl) == KErrNotFound) {
      if (m_messageBody.Find(Kwayf) == KErrNotFound) {
         /* Not ours. */
         m_destinationDescription = _L("unknown cmd"); //DEBUG
         return;
      } else {
         commandPos = Kwayf().Length();
      }
   } else {
      commandPos = Ksckl().Length()+Kwayf().Length()+1;
   }

   switch (m_messageBody[commandPos])
   {
   case 'D':
      parseDestination(commandPos+1);
      break;
   case 'R':
      parseRoute(commandPos+1);
      break;
   case 'F':
      parseFavorite(commandPos+1);
      break;
   default:
      m_messageType = EWFSMSUnknown;
      break;
   }
//   TBuf<256> text;
//   text.Format(_L("parse attempt of char %u, destdescr %S"), m_messageBody[6], m_destinationDescription);
//   CWayFinderAppUi::ShowInformationL(text);
   // TRACE_DBG( "" );
}

bool
WayfinderSMSMessage::readNextVal(TLexExts& lex, TInt32& val, const TPtrC& errMess, bool tryinc)
{
   // TRACE_DBG( "readNextVal" );
   bool result = lex.readNextVal(val, "WayfinderSMSMessage::readNextVal error", tryinc);
   if (!result) {
      m_destinationDescription = errMess;
   }
   return result;
}

bool
WayfinderSMSMessage::readNextCheckNum(TLexExts& lex)
{
   // TRACE_DBG( "readNextCheckNum" );
   bool result = false;
   int checkNumFromSMS;
   if (!lex.readNextVal(checkNumFromSMS, "check num prob", false)) {return result;}
   TInt checkNumCalc = (m_destinationLat & 0xFF) ^ 
                       (m_destinationLon & 0xFF) ^ (TInt)'W';
   if (checkNumCalc != checkNumFromSMS) {
      m_destinationDescription = _L("check num not valid"); //DEBUG
   } else {
      result = true;
   }
   return result;
}

void
WayfinderSMSMessage::parseRoute(TInt startpos)
{
   // TRACE_DBG( "parseRoute" );
   TLexExts lex(m_messageBody.Mid(startpos)); // ignore "//WAYFR"
//   m_destinationFlag = lex.Get();

   if (!readNextVal(lex, m_destinationLat, _L("lat prob"), true)) {return;}
   if (!readNextVal(lex, m_destinationLon, _L("lon prob"), true)) {return;}
   if (!readNextVal(lex, m_originLat, _L("originlat prob"), true)) {return;}
   if (!readNextVal(lex, m_originLon, _L("originlon prob"), true)) {return;}
   if (!readNextCheckNum(lex)) {return;}
   lex.tryInc();
   // origin name, ends with " \\ ".
   m_originDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_originDescription.Append(lex.Get());
   }
   lex.tryInc();
   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   lex.tryInc();
   m_signature = lex.Remainder();

   m_messageType = EWFSMSRoute; // everything went fine
}

void
WayfinderSMSMessage::parseDestination(TInt startpos)
{
   // TRACE_DBG( "parseDestination" );
   TLexExts lex(m_messageBody.Mid(startpos)); // ignore "//WAYFD"
   m_destinationFlag = lex.Get();

   if (!readNextVal(lex, m_destinationLat, _L("lat prob"), true)) {return;}
   if (!readNextVal(lex, m_destinationLon, _L("lon prob"), true)) {return;}
   if (!readNextCheckNum(lex)) {return;}
   lex.tryInc();

   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   lex.tryInc();
   m_signature = lex.Remainder();

   m_messageType = EWFSMSDestination; // everything went fine
}

void
WayfinderSMSMessage::parseFavorite(TInt startpos)
{
   // TRACE_DBG( "parseFavorite" );
   TLexExts lex(m_messageBody.Mid(startpos)); // ignore "//WAYFF"
//   m_destinationFlag = lex.Get();

   if (!readNextVal(lex, m_destinationLat, _L("lat prob"), true)) {return;}
   if (!readNextVal(lex, m_destinationLon, _L("lon prob"), true)) {return;}
   if (!readNextCheckNum(lex)) {return;}
   lex.tryInc();

   delete m_favorite;

   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   char* name = WFTextUtil::newTDesDupL(m_destinationDescription);

   lex.tryInc();
   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   char* shortname = WFTextUtil::newTDesDupL(m_destinationDescription);

   lex.tryInc();
   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   char* description = WFTextUtil::newTDesDupL(m_destinationDescription);

   lex.tryInc();
   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   char* category = WFTextUtil::newTDesDupL(m_destinationDescription);

   lex.tryInc();
   m_destinationDescription.Zero();
   while (!lex.Eos() && (lex.Peek() != WideSmsStringDelimiter)) {
      m_destinationDescription.Append(lex.Get());
   }
   char* mapiconname = WFTextUtil::newTDesDupL(m_destinationDescription);

   // also set the description of this sms.
   WFTextUtil::char2TDes(m_destinationDescription, name);

   lex.tryInc();
   m_signature = lex.Remainder();

#ifdef BIOCONTROL
   m_favorite = NULL;
#else
   m_favorite = new class isab::Favorite(m_destinationLat, m_destinationLon, 
      name, 
      shortname, 
      description, 
      category, 
      mapiconname);
#endif
   
   delete[] name;
   delete[] shortname;
   delete[] description;
   delete[] category;
   delete[] mapiconname;

   m_messageType = EWFSMSFavorite; // everything went fine
}

const HBufC*
WayfinderSMSMessage::getSenderDetails() const
{
   // TRACE_DBG( "getSenderDetails" );
   return m_senderDetails;
}

const uint8
WayfinderSMSMessage::SmsStringDelimiter = '}'; // the yen-sign in the GSM-character table.

#include "WayFinderAppUi.h"

void
RealShowMess(const char* file, const int line, const char* mess, int value)
{
   // TRACE_DBG( "RealShowMess" );
   TBuf<512> text;
   WFTextUtil::char2TDes(text, mess);
   text.Append(_L(", "));
   text.AppendNum(value);
   text.Append(_L(". File: "));
   TBuf<100> posInfo;
   WFTextUtil::char2TDes(posInfo, file);
   text.Append(posInfo);
   text.Append(_L(", line: "));
   text.AppendNum(line);
# if defined(NAV2_CLIENT_SERIES60_V1) || defined(NAV2_CLIENT_SERIES60_V2)
#  if !defined(BIOCONTROL)
   TPtrC header = _L("ShowMess");
   WFDialog::ShowScrollingDialogL(header, text, EFalse);
#  endif
# endif
}

void 
HandleAssertion(const char* file, const int line)
{
   // TRACE_DBG( "HandleAssertion" );
   TBuf<512> text;
   WFTextUtil::char2TDes(text, file);
   text.Append(_L(", line "));
   text.AppendNum(line);
   text.Append(_L(". Assertion failed. Continue?\0"));
#  if !defined(BIOCONTROL)
   char* c = WFTextUtil::newTDesDupL(text); LOGNEWSTRING(c);
   LOG_INFO(c);
   DELETEA_AND_LOG(c);
   if (WFDialog::ShowQueryL(text)) {
   } else {
      // break.
#     ifdef _DEBUG_NO_IT_DOESNT_COMPILE
      _asm int 3;
#     endif
   }
#  endif
}

#endif
