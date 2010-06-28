/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef TRACKPIN_H
#define TRACKPIN_H


#include "arch.h"
#include "Buffer.h"
#include <list>


namespace isab{



/**
 * Class representing a tracking PIN.
 *
 */
class TrackPIN {
   public:
      /**
       * Constructor.
       */
      TrackPIN( uint32 id, const char* pin, const char* comment ) 
         : m_id( id ), m_pin( strlen( pin ) ),
           m_comment( strlen( comment ) ) 
      {
         m_pin.writeNextCharString( pin );
         m_comment.writeNextCharString( comment );
      }


      uint32 getID() const { return m_id ; }
      void setID( uint32 id ) { m_id = id ; }

      const char* getPIN() const { 
         return reinterpret_cast<const char*>( m_pin.accessRawData( 0 ) );
      }

      const char* getComment() const { 
         return reinterpret_cast<const char*>( 
            m_comment.accessRawData( 0 ) );
      }



   private:
      uint32 m_id;
      Buffer m_pin;
      Buffer m_comment;
};


/**
 * Class for holding TrackPINs.
 *
 */
class TrackPINList : public std::list< TrackPIN* > {
   public:
      /**
       * Constructor.
       */
      TrackPINList( const uint8* data, int size );


      /**
       * Empty constructor.
       */
      TrackPINList();


      /**
       * Destructor.
       */
      ~TrackPINList();


      /**
       * Add a PIN.
       */
      void addPIN( const char* pin, const char* comment, uint32 id = 0 );


      /**
       * Delete a PIN identified by id, id 0 is not recommended.
       * Sets the PIN as deleted.
       */
      void deletePIN( uint32 id );


      /**
       * Delete a PIN object.
       */
      void deletePIN( const TrackPIN* pin );


      /**
       * Get the number of PINs.
       */
      //uint32 size() const;


      /**
       * Pack into a buffer.
       */
      void packInto( Buffer* buff ) const;


      /**
       * Get the crc.
       */
      uint32 getCRC() const { return m_crc ; }


      /**
       * Set the crc.
       */
      void setCRC( uint32 crc ) { m_crc = crc ; }


      /**
       * Removes a PIN identified by id, id 0 is not recommended.
       */
      void removePIN( uint32 id );



      /**
       * Removes a specific PIN.
       */
      void removePIN( const TrackPIN* pin );


      /**
       * Trace TrackPINList.
       */
      void TRACE();

   private:
      /**
       * CRC.
       */
      uint32 m_crc;
};



} // End isab namespace

#endif // TRACKPIN_H

