/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef NPARAM_H
#define NPARAM_H

#include "arch.h"
#include "Log.h"
#include <vector>
#ifdef UNDER_CE
# include <Winsock.h>
#else
# include <netinet/in.h>
#endif

namespace isab {


/**
 * XXX: Copied 20040621 15:00 from MC2. Modified to use Nav2 logging.
 * Added addUint32ToByteVector 20060508 /Dp
 * Class holding a NavServerProt, v10+, param.
 * All complex values (16 bit, 32 bit) are considered to have
 * the most significant byte first. All data is in network
 * order that is.
 *
 */
class NParam {
   public:
      /**
       * Constructor.
       * 
       * @param paramID The parameter ID.
       * @param buff The buffer with the parameter data.
       * @param len The length of the parameter data.
       */
      NParam( uint16 paramID, const uint8* buff, uint16 len );


      /**
       * Constructor from bool.
       */
      NParam( uint16 paramID, bool data );


      /**
       * Constructor from uint8.
       */
      NParam( uint16 paramID, uint8 data );


      /**
       * Constructor from uint16.
       */
      NParam( uint16 paramID, uint16 data );


      /**
       * Constructor from uint32.
       */
      NParam( uint16 paramID, uint32 data );


      /**
       * Constructor from int32.
       */
      NParam( uint16 paramID, int32 data );


      /**
       * Constructor from string.
       */
      NParam( uint16 paramID, const char* data );


      /**
       * Constructor from uint16 array.
       */
      NParam( uint16 paramID, const uint16* data, uint16 len );


      /**
       * Constructor from uint32 array.
       */
      NParam( uint16 paramID, const uint32* data, uint16 len );


      /**
       * Constructor from int32 array.
       */
      NParam( uint16 paramID, const int32* data, uint16 len );


      /**
       * Constructor from Coordinate.
       */
      NParam( uint16 paramID, int32 lat, int32 lon );


      /**
       * Constructor for param without data.
       */
      NParam( uint16 paramID );


      /**
       * Get the paramID.
       */
      uint16 getParamID() const;


      /**
       * Set the paramID.
       */
      void setParamID( uint16 paramID );


      /**
       * Get the buffer length.
       */
      uint16 getLength() const;


      /**
       * Get const referense to buffer.
       */
      const uint8* getBuff() const;


      /**
       * Get buffer as a bool.
       */
      bool getBool() const;


      /**
       * Get buffer as a uint8.
       */
      uint8 getByte( uint16 index = 0 ) const;


      /**
       * Get buffer as a uint16.
       */
      uint16 getUint16( uint16 index = 0 ) const;

      /**
       * Get an uint32 from a byte vector.
       */
      static uint32 getUint32FromByteArray( const byte* b,
                                            uint16 index = 0 );

      /**
       * Get buffer as a uint32.
       */
      uint32 getUint32( uint16 index = 0 ) const;


      /**
       * Get buffer as a int32.
       */
      int32 getInt32( uint16 index = 0 ) const;


      /**
       * Get buffer as a string.
       */
      const char* getString( uint16 index = 0 ) const;


      /**
       * Get buffer as uint8 array. Is getLength bytes long.
       */
      const uint8* getByteArray() const;


      /**
       * Get uint16 at position index.
       */
      uint16 getUint16Array( uint16 index ) const;


      /**
       * Get uint32 at position index.
       */
      uint32 getUint32Array( uint16 index ) const;


      /**
       * Get int32 at position index.
       */
      int32 getInt32Array( uint16 index ) const;


      /**
       * Append a uint8.
       */
      void addByte( uint8 data );


      /**
       * Append an uint16.
       */
      void addUint16( uint16 data );


      /**
       * Append an uint32.
       */
      void addUint32( uint32 data );


      /**
       * Append an int32.
       */
      void addInt32( int32 data );


      /**
       * Append a uint8 array.
       */
      void addByteArray( const uint8* buff, uint16 len );
      void addByteArray( const char* buff, uint16 len );

      /**
       * Append a string.
       */
      void addString( const char* str );


      /**
       * Write to a uintu buffer.
       */
      void writeTo( std::vector< uint8 >& buff ) const;


      /**
       * Comparison operator <.
       */
      bool operator < ( const NParam& b ) const;


      /**
       * Comparison operator >.
       */
      bool operator > ( const NParam& b ) const;


      /**
       * Comparison operator !=.
       */
      bool operator != ( const NParam& b ) const;


      /**
       * Comparison operator ==.
       */
      bool operator == ( const NParam& b ) const;


      /**
       * Copy cont for debugprinting
       */
      NParam( const NParam& other );


      /**
       * Dump param to out.
       */
      void dump( Log* out, bool dumpValue = false ) const;


      /**
       * Set the content.
       */
      void setBuff( const std::vector< uint8 > buff );


      /**
       * Get the buffer as std::vector< uint8 >.
       */
      const std::vector< uint8 >& getVector() const;


      /**
       * Clear the param from any content.
       */
      void clear();

      /**
       * Static method to add an uint32 to a byte array.
       */
      static void addUint32ToByteVector( std::vector<byte>& v, uint32 d );

   private:
      /// The paramID.
      uint16 m_paramID;


      /// The buffer.
      std::vector< uint8 > m_buff;
};


// =======================================================================
//                                     Implementation of inlined methods =


} /* namespace isab */

#endif // NPARAM_H

