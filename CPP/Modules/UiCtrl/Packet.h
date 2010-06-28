/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef PACKET_H
#define PACKET_H

#include "Buffer.h"
#include "PacketEnums.h"
#include "Matches.h"
#include "Log.h"
#define IS_REQUEST(id) (!(id&0x80))

namespace isab{
   using namespace Packet_Enums;

   class Packet : public Buffer{
   public:


      Packet* next;
      Packet* prev;

      uint8 format_type;
      uint8 type;
      uint16 ID;
      const char* string1;
      const char* string2;
      const char* string3;
      uint32 uint32_1;
      uint32 uint32_2;
      uint32 uint32_3;
      uint16 uint16_1;
      uint16 uint16_2;
      uint8 uint8_1;
      uint8 uint8_2;

      /** Constructor that copies the supplied data and decodes the contents
       * into the proper member variables.
       * @param data the serialized Packet data.
       * @param len  the size of the data field.
       */
      Packet(const uint8* data, unsigned len);

      /** Constructor that creates either a <code>only_type</code> packet
       * or a <code>type_and_data</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID the ID of the created Packet.
       * @param is_type_and_data specifies if this packet should have the 
       *                         format_type field set to 
       *                         <code>only_type</code> or 
       *                         <code>type_and_data</code>. The default 
       *                         value is <code>false</code>, which gives a 
       *                         <code>only_type</code> packet. The 
       *                         <code>only_type</code> packet is immediately
       *                         closed with correct encodeing and checksum 
       *                         while <code>type_and_data</code> packets are 
       *                         left open so the data can be written. When 
       *                         all data has been written the packet is 
       *                         closed with a call to 
       *                         <code>closeMessage</code>.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       bool is_type_and_data = false ); 

      /** Constructor that creates a <code>type_and_uint8</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID the ID of the created Packet.
       * @param theuint8 the single uint8 to include in the packet.
       */
      explicit Packet( PacketType theType, uint16 theID,uint8 theuint8 );

      /** Constructor that creates a <code>type_and_uint16</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID the ID of the created Packet.
       * @param theuint16 the single uint16 to inclued in the packet.
       */
      explicit Packet( PacketType theType, uint16 theID,uint16 theuint16 );

      /** Constructor that creates a <code>type_and_uint32</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID the ID of the created Packet.
       * @param theuint32 the single uint32 to include in the packet.
       */
      explicit Packet( PacketType theType, uint16 theID,uint32 theuint32 );

      /** Constructor that creates a <code>type_and_two_uint8</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param first   the first uint8.
       * @param second  the second uint8.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       uint8 first, uint8 second);

      /** Constructor that creates a <code>type_and_two_uint16</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param first   the first uint16.
       * @param second  the second uint16.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       uint16 first, uint16 second);

      /** Constructor that creates a <code>type_and_two_uint32</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param first   the first uint32.
       * @param second  the second uint32.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       uint32 first, uint32 second);

      /** Constructor that creates a <code>type_and_strings</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param first   the first string. 
       * @param second  the second string. Defaults to NULL.
       * @param third   the third string. Default to NULL.
       */
      Packet( PacketType theType, uint16 theID,
              const char* first, const char* second = NULL,
              const char* third = NULL);

      /** Constructor that creates a <code>type_and_uint8_and_strings</code> 
       * packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param u8      the uint8.
       * @param s1      the first string.
       * @param s2      the second string. Defaults to NULL.
       */
      explicit Packet( PacketType theType, uint16 theID, uint8 u8, 
                       const char* s1, const char* s2 = NULL);

      /** Constructor that creates a <code>type_and_uint16_and_strings</code> 
       * packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID the ID of the created Packet.
       * @param u16   the uint16.
       * @param s1    the first string.
       * @param s2    the second string. Defaults to NULL.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       uint16 u16, const char* s1, const char* s2 = NULL);

      /** Constructor that creates a <code>type_and_uint32_and_strings</code> 
       * packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param u32     the uint32.
       * @param s1      the first string.
       * @param s2      the second string. Defaults to NULL.
       */
      explicit Packet( PacketType theType, uint16 theID,
                       uint32 u32, const char* s1, const char* s2 = NULL);

      /** Constructor that creates a
       * <code>type_and_two_uint32_and_strings</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param u32_1   the first uint32.
       * @param u32_2   the second uint32.
       * @param s1      the first string.
       * @param s2      the second string. Defaults to NULL.
       */
      Packet( PacketType theType, uint16 theID, uint32 u32_1, 
              uint32 u32_2, const char* s1, const char* s2 = NULL);

      /** Constructor that creates a 
       * <code>type_and_three_uint32_and_three_strings</code> packet.
       * @param theType a value from the PacketType enum describing the
       *                content of the Packet.
       * @param theID   the ID of the created Packet.
       * @param u1      the first uint32.
       * @param u2      the second uint32.
       * @param u3      the third uint32.
       * @param s1      the first string.
       * @param s2      the second string.
       * @param s3      the third string.
       */
      Packet( PacketType theType, uint16 theID,
              uint32 u1, uint32 u2, uint32 u3,
              const char* s1, const char* s2, const char* s3);

      void encode(bool finish = true);
      void closeMessage();
      size_t length();
      void pad();

      /**
       * Decodes a byte array into Packet(s).
       *
       * @param data The byte array with presumed Packet contents.
       * @param len The length of data.
       * @return A linked list of zero or more Packets.
       */
      static Packet* decodeBytes( const uint8* data, size_t len, Log *m_log );

#ifndef _MSC_VER
      static const uint8 DLE = 0x10;
      static const uint8 SOH = 0x01;
      static const uint8 STX = 0x02;
      static const uint8 ETX = 0x03;
      static const uint8 EOT = 0x04;
#else
      enum { DLE = 0x10,
             SOH = 0x01,
             STX = 0x02,
             ETX = 0x03,
             EOT = 0x04
      };
#endif

      /**
       * Dequeues a Packet p from the linked list list.
       *
       * @param list The list to remove first Packet from.
       */
      static Packet* dequeuePacket( Packet*& list );

      int writeNextMatch(const Match* match);
      int writeNextOverviewMatch(const OverviewMatch* om);

   private:
      enum DLEstate {
         state_normal,
         state_last_was_dle,
         state_no_dle
      };

      static inline uint16 update_csum(uint16 csum, uint8 data);
      static inline uint16 calc_csum(const uint8* data, int len);

      /**
       * Checks if a buffer is a Packet buffer.
       *
       * @param data The presumed Packet buffer.
       * @param len The length of data.
       * @param out_len Set to the number of bytes left to end of data 
       *                array from returned pointer.
       * @return Beginning position of packet data in data or NULL if not
       *         a packet buffer.
       */
      static const uint8* check_buffer( const uint8* data, int len, 
                                        int& out_len );

      /**
       * Inserts a Packet p into the linked list list with the last packet 
       * last. 
       */
      static void insertPacket( Packet*& list, Packet*& last, Packet* p );
   };

#define HEADER_SIZE 10


   // ========================================================================
   //                                  Implementation of the inlined methods =


   inline Packet::Packet( PacketType theType, uint16 theID, 
                          bool is_type_and_data) : 
      Buffer(HEADER_SIZE), next(NULL), prev(NULL),
      format_type(only_type), type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0) 
   {
      if( is_type_and_data ) {
         format_type = type_and_data;
      }
      encode( !is_type_and_data );
   }

   inline Packet::Packet( PacketType theType, uint16 theID, uint8 theuint8) :
      Buffer(HEADER_SIZE + 1), next(NULL), prev(NULL),
      format_type(type_and_uint8), type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(theuint8), uint8_2(0)
   {
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, uint16 theuint16) :
      Buffer(HEADER_SIZE + 2), next(NULL), prev(NULL),
      format_type(type_and_uint16), type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(theuint16), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, uint32 theuint32) :
      Buffer(HEADER_SIZE + 4), next(NULL), prev(NULL),
      format_type(type_and_uint32), type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(theuint32), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      encode();
   }


   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint8 first, uint8 second) :
      Buffer(HEADER_SIZE + 2), next(NULL), prev(NULL),
      format_type(type_and_two_uint8), 
      type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(first), uint8_2(second)
   {
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint16 first, uint16 second) :
      Buffer(HEADER_SIZE + 4), next(NULL), prev(NULL),
      format_type(type_and_two_uint16), 
      type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(first), uint16_2(second), 
      uint8_1(0), uint8_2(0)
   {
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint32 first, uint32 second) :
      Buffer(HEADER_SIZE + 8), next(NULL), prev(NULL),
      format_type(type_and_two_uint32), 
      type(theType), ID(theID), 
      string1(NULL), string2(NULL), string3(NULL), 
      uint32_1(first), uint32_2(second), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      encode();
   }


   inline Packet::Packet( PacketType theType, uint16 theID, 
                          const char* first, const char* second, 
                          const char* third) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_strings), type(theType), ID(theID), 
      string1(first), string2(second), string3(third), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 
                  ((first != NULL) ? strlen(first) : 0) + 1 +
                  ((second != NULL) ? strlen(second) : 0) + 1 +
                  ((third != NULL) ? strlen(third) : 0) + 1);
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, uint8 u8, 
                          const char* s1, const char* s2) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_uint8_and_strings), 
      type(theType), ID(theID), 
      string1(s2), string2(s2), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(u8), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 1 +
                  ((s1 != NULL) ? strlen(s1) : 0) + 1 +
                  ((s2 != NULL) ? strlen(s2) : 0) + 1);
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint16 u16, const  char* s1, const char* s2) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_uint16_and_strings), 
      type(theType), ID(theID), 
      string1(s1), string2(s2), string3(NULL), 
      uint32_1(0), uint32_2(0), uint32_3(0), 
      uint16_1(u16), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 2 +
                  ((s1 != NULL) ? strlen(s1) : 0) + 1 +
                  ((s2 != NULL) ? strlen(s2) : 0) + 1);
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint32 u32, const char* s1, const char* s2) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_uint32_and_strings), 
      type(theType), ID(theID), 
      string1(s1), string2(s2), string3(NULL), 
      uint32_1(u32), uint32_2(0), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 4 +
                  ((s1 != NULL) ? strlen(s1) : 0) + 1 +
                  ((s2 != NULL) ? strlen(s2) : 0) + 1);
      encode();
   }


   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint32 u32_1, uint32 u32_2, const char* s1, const char* s2) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_two_uint32_and_strings), 
      type(theType), ID(theID), 
      string1(s1), string2(s2), string3(NULL), 
      uint32_1(u32_1), uint32_2(u32_2), uint32_3(0), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 8 +
                  ((s1 != NULL) ? strlen(s1) : 0) + 1 +
                  ((s2 != NULL) ? strlen(s2) : 0) + 1);
      encode();
   }

   inline Packet::Packet( PacketType theType, uint16 theID, 
                          uint32 u1, uint32 u2, uint32 u3, 
                          const char* s1, const char* s2, const char* s3) :
      Buffer(0), // init later
      next(NULL), prev(NULL),
      format_type(type_and_three_uint32_and_three_strings), 
      type(theType), ID(theID), 
      string1(s1), string2(s2), string3(s3), 
      uint32_1(u1), uint32_2(u2), uint32_3(u3), 
      uint16_1(0), uint16_2(0), 
      uint8_1(0), uint8_2(0)
   {
      enlargeData(HEADER_SIZE + 3*4 +
                  ((s1 != NULL) ? strlen(s1) : 0) + 1 +
                  ((s2 != NULL) ? strlen(s2) : 0) + 1 +
                  ((s3 != NULL) ? strlen(s3) : 0) + 1);
      encode();
   }


   inline uint16 Packet::update_csum(uint16 csum, uint8 data)
   {
      return csum + data;
   }

   inline uint16 Packet::calc_csum(const uint8* data, int len)
   {
      uint16 csum = 0;
      while (len --) 
         csum = update_csum(csum, *data++);
      return csum;
   }

}

#endif // PACKET_H


