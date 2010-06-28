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

#ifndef TDESTRACKPIN_H
#define TDESTRACKPIN_H

#include <e32base.h>
#include "WFTextUtil.h"
#include "TrackPIN.h"

namespace isab{

/**
 * A class for manipulating an item from TrackPINList as descriptors.
 */
class CDesTrackPIN : public CBase {
   /**
    * @name Constructors and destructor. 
    */
   //@{
   /**
    * Default constructor. 
    */
   CDesTrackPIN();
   /**
    * Second phase constructor.
    * @param aPin the pin that this object will be a copy of.
    */
   void ConstructL( const class TrackPIN& aPin );
public:
   /**
    * Static constructor.
    * @param aPin The pin that this object will be a copy of.
    * @return A new CDesTrackPIN object on the CleanupStack.
    */
   static class CDesTrackPIN* NewLC( const class TrackPIN& aPin );
   /**
    * Static constructor.
    * @param aPin The pin that this object will be a copy of.
    * @return A new CDesTrackPIN object.
    */
   static class CDesTrackPIN* NewL( const class TrackPIN& aPin );
   /**
    * Virtual destructor. 
    */
   virtual ~CDesTrackPIN();
   //@}

   /**
    * Gets the ID of the PIN.
    * @return The ID.
    */
   TUint32 GetID() const;
   /**
    * Gets the PIN code.
    * @return the PIN code. 
    */
   const TDesC& GetTDesPIN() const;
   /**
    * Gets the comment of the PIN.
    * @return the comment.
    */
   const TDesC& GetTDesComment() const;
   
private:
   /** The ID. */
   TUint32 iId;
   /** The PIN. */
   HBufC* iPin;
   /** The comment. */
   HBufC* iComment;
};

} // End isab namespace


#endif // TDESTRACKPIN_H

