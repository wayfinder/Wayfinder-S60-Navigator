/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDERSMSMESSAGE_H
#define WAYFINDERSMSMESSAGE_H

#ifndef NAV2_CLIENT_SERIES60_V3

/* #include <sendas.h> */
#include "machine.h"
#include <msvapi.h>

namespace isab{
   class Log;
   class Favorite;
}

/**
 *  Encapsulates a Wayfinder SMS message containing for instance
 *  a destination. Handles retrieving the message from the SMS Mtm, 
 *  parsing it, etc
 */
class WayfinderSMSMessage : public CBase /*MMsvSessionObserver*/
{
   public:

   /// The different types of SMSes including errors
   enum WFSMSType
   {
      EWFSMSFavorite = 0,              // Wayfinder favorite
      EWFSMSRoute,                     // Wayfinder route
      EWFSMSDestination,               // Wayfinder destination
      EWFSMSParseError,                // parse error, checksum error, etc
      EWFSMSNoMessage,                 // no message parsed yet
      EWFSMSUnknown,                   // unknown type of Wayfinder message
      EWFSMSNotWFMessage,              // not a Wayfinder message
      EWFSMSError                      // error, eg problem interfacing with OS
   };

   TChar WideSmsStringDelimiter;
   static const uint8 SmsStringDelimiter;

   /**
    *  Two-phased constructor
    *  @param mtm SMS Mtm instance may not be NULL.
    */
   static WayfinderSMSMessage* NewL( isab::Log* log, CBaseMtm* mtm );

   /**
    *  Destructor
    */
   virtual ~WayfinderSMSMessage();

   /**
    *   Parse the SMS with the given messageID. Use getType() for information
    *   about the result.
    */
   void parseMessage(TMsvId messageID);

   /**
    *   Get the type of the parsed SMS or a failure code, see WFSMSType.
    *   @return WFSMSType, the type of the SMS
    */
   WFSMSType getType();

   /**
    *   Get the latitude for a destination
    *   @return TInt32, the latitude
    */
   TInt32 getDestinationLat();

   /**
    *   Get the longitude for a destination
    *   @return TInt32, the longitude
    */
   TInt32 getDestinationLon();

   /**
    *   Get the latitude for an origin
    *   @return TInt32, the latitude
    */
   TInt32 getOriginLat();

   /**
    *   Get the longitude for an origin
    *   @return TInt32, the longitude
    */
   TInt32 getOriginLon();

   /**
    *   Get the description for a destination
    *   @return TDesC&, the description
    */
   TDesC& getDestinationDescription();

   /**
    *   Get the description for an origin
    *   @return TDesC&, the description
    */
   TDesC& getOriginDescription();

   /**
    *   Get the signature of the sender.
    *   @return TDesC&, the description
    */
   TDesC& getSignature();

   /**
    *   Get the favorite, transfer ownership.
    *   @return The favorite, or null if none.
    */
   class isab::Favorite* getFavoriteD();

   /**
    *   Gets the sender details.
    *   @return The sender details.
    */
   const HBufC* getSenderDetails() const;

   /**
    *   Get a pointer to the favorite.
    *   @return Pointer to the favorite.
    */
   const class isab::Favorite* peekFavorite() const;

   /**
    *   Get the flag for a destination, currently not used
    *   @return TChar, the destination flag
    */
   TChar getDestinationFlag();

   /**
    *   Delete the message that was last parsed
    *   @return TBool: ETrue if deleted OK, otherwise EFalse
    */
   TBool deleteMessage();

   /**
    *   Get the body of the message that was last parsed, should only
    *   be used for debugging purposes.
    *   @return TPtrC Returns pointer to the SMS body
    */
   TPtrC getDebugBody();

   private:

   /**
    *   Two-phase constructor
    */
   void ConstructL();

   /**
    *   The hidden constructor
    *   @param mtm SMS Mtm instance may not be NULL.
    */
   WayfinderSMSMessage( CBaseMtm* mtm );

   /**
    *   Private function that handles the parsing of a destination
    */
   void parseDestination(TInt startpos);

   /**
    *   Private function that handles the parsing of a route
    */
   void parseRoute(TInt startpos);

   /**
    *   Private function that handles the parsing of a favorite.
    */
   void parseFavorite(TInt startpos);

   /**
    *   Parses what is stored in the message body.
    */
   void parseSelf();

   /**
    * Reads the next value from the lexer.
    * @param lex The lexer.
    * @param val The variable that will hold the new value.
    * @param errMess The errormessage to put into destinationdescription if anything fails.
    * @return True if ok, false if not.
    */
   bool readNextVal(TLexExts& lex, TInt32& val, const TPtrC& errMess, bool tryinc);

   /**
    * Reads the next checknum and verifies agains object attributes.
    * @param lex The lexer.
    * @return True if ok, false if not.
    */
   bool readNextCheckNum(TLexExts& lex);

   /// The ID of the last SMS parsed
   TMsvId m_messageID;

   /// The type of the last SMS parsed
   WFSMSType  m_messageType;

   /// Message body of the last SMS parsed
   TPtrC m_messageBody;

   /// A string describing the sender.
   HBufC* m_senderDetails;

   /// The latitude for a destination
   TInt32 m_destinationLat;

   /// The longitude for a destination
   TInt32 m_destinationLon;

   /// The latitude for an origin
   TInt32 m_originLat;

   /// The longitude for an origin
   TInt32 m_originLon;

   /// The description for a destination
   TBuf<255> m_destinationDescription;

   /// The description for an origin.
   TBuf<255> m_originDescription;

   /// The signature of another user.
   TBuf<255> m_signature;
   
   /// The flag for a destination, currently not used
   TChar m_destinationFlag;

   class isab::Favorite* m_favorite;

   /// Our instance of the SMS Mtm
   CBaseMtm* m_smsMtm;

   //for memory debugging (and other debugging)
   isab::Log* iLog;
   
};

#endif

#endif
