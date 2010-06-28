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

#ifndef SMS_PARSER_H_
#define SMS_PARSER_H_

#include <smsclnt.h>
#include <bamdesca.h>


namespace isab
{
   class Favorite;
}

class CSmsParser
{

public:

   static CSmsParser* NewL();
   static CSmsParser* NewLC();

protected:

   CSmsParser();

   void ConstructL();

public:

   virtual ~CSmsParser();

   enum WFSMSType{
      EWFSMSFavorite = 0,              // Wayfinder favorite
      EWFSMSRoute,                     // Wayfinder route
      EWFSMSDestination,               // Wayfinder destination
      EWFSMSParseError,                // parse error, checksum error, etc
      EWFSMSNoMessage,                 // no message parsed yet
      EWFSMSUnknown,                   // unknown type of Wayfinder message
      EWFSMSNotWFMessage,              // not a Wayfinder message
      EWFSMSError                      // error, eg problem interfacing with OS
   };

   /**
    * Checks whether incomming message is intended for Wayfinder.
    */
   TBool IsWayfinderSms( const CRichText& body );

   /**
    * Message parsing starts from here  
    * Identifies message for Destination, Route, Favorite
    */
   TBool ParseSmsL( const TDesC& aMessageBody,
                    TDes& aError );

   WFSMSType GetSmsType();

   const TDesC& GetDestinationDescription();

   TPoint GetDestinationCoordinate();

   const TDesC& GetOriginDescription();

   TPoint GetOriginCoordinate();

   const TDesC& GetSignature();

   isab::Favorite* GetFavoriteD();

private:

   /**
    *   Private function that handles the parsing of a destination
    */
   TBool ParseDestination( const TDesC& aMessageBody,
                           TInt aCommandPos,
                           TDes& aDescription,
                           TInt32& aLat, TInt32& aLon,
                           TDes& aSignature,
                           TDes& aError  );

   /**
    *   Private function that handles the parsing of a route
    */
   TBool ParseRoute( const TDesC& aMessageBody,
                     TInt aCommandPos,
                     TDes& aOrgDescription,
                     TInt32& aOrgLat, TInt32& aOrgLon,
                     TDes& aDestDescription,
                     TInt32& aDestLat, TInt32& aDestLon,
                     TDes& aSignature,
                     TDes& aError  );

   /**
    *   Private function that handles the parsing of a favorite.
    */
   TBool ParseFavoriteL( const TDesC& aMessageBody,
                         TInt aCommandPos,
                         TDes& aDescription,
                         TInt32& aLat, TInt32& aLon,
                         TDes& aSignature,
                         TDes& aError  );

   /**
    * Reads the next value from the lexer.
    * @param lex The lexer.
    * @param val The variable that will hold the new value.
    * @param errMess The errormessage to put into destinationdescription if anything fails.
    * @return True if ok, false if not.
    */
   TBool ReadNextVal( TLex& lex, TInt32& val );

   /**
    * Reads the next checknum and verifies agains object attributes.
    * @param lex The lexer.
    * @return True if ok, false if not.
    */
   TBool ReadNextCheckNum( TLex& lex, 
                           TInt32 aLat, TInt32 aLon,
                           TDes& aError );

   /**
    * Calls Inc on the tlex unless at end of string already.
    * Avoids possible panics.
    * @param lex The lexer.
    */
   void TryInc(TLex& lex);

private:

   /// The rype of received wayfinder SMS
   WFSMSType iMessageType;

   /// The description for a destination
   TBuf<255> iDestinationDescription;

   /// The latitude for a destination
   TInt32 iDestinationLat;

   /// The longitude for a destination
   TInt32 iDestinationLon;

   /// The description for an origin.
   TBuf<255> iOriginDescription;

   /// The latitude for an origin
   TInt32 iOriginLat;

   /// The longitude for an origin
   TInt32 iOriginLon;

   /// The signature of another user.
   TBuf<255> iSignature;

   isab::Favorite* iFavorite;

   TChar WideSmsStringDelimiter;

};

#endif
