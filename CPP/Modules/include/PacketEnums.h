/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PACKET_ENUMS_H
#define PACKET_ENUMS_H

namespace isab{
   namespace Packet_Enums {
      /**
       * The types of packets. Only 8 bits may be used.
       */
      enum PacketType{
         // Administration
         ERRORE                       = 0x00,
         TEST_DATA                    = 0x01,
         PDA_TYPE                     = 0x02,
         NAVIGATOR_TYPE               = 0x03,
         RESET_NAVIGATOR              = 0x04,
         GET_DATA_PHONE_NO            = 0x05,
         SET_DATA_PHONE_NO            = 0x06,
         GET_VOICE_PHONE_NO           = 0x07,
         SET_VOICE_PHONE_NO           = 0x08,
         GET_USER_ID                  = 0x09,                
         GET_PASSWORD                 = 0x0a,                
         SET_USER_ID                  = 0x0b,
         SET_PASSWORD                 = 0x0c,
         EXTENDED_ERROR               = 0x0d,
         NAVIGATOR_PARAMETERS         = 0x0e,
         SERVER_PARAMETERS            = 0x0f,
         
         // Administration replies
         ERRORE_REPLY                 = (ERRORE              | 0x80),
         TEST_DATA_REPLY              = (TEST_DATA           | 0x80),
         PDA_TYPE_REPLY               = (PDA_TYPE            | 0x80),
         NAVIGATOR_TYPE_REPLY         = (NAVIGATOR_TYPE      | 0x80),
         RESET_NAVIGATOR_REPLY        = (RESET_NAVIGATOR     | 0x80),
         GET_DATA_PHONE_NO_REPLY      = (GET_DATA_PHONE_NO   | 0x80),
         SET_DATA_PHONE_NO_REPLY      = (SET_DATA_PHONE_NO   | 0x80),
         GET_VOICE_PHONE_NO_REPLY     = (GET_VOICE_PHONE_NO  | 0x80),
         SET_VOICE_PHONE_NO_REPLY     = (SET_VOICE_PHONE_NO  | 0x80),
         GET_USER_ID_REPLY            = (GET_USER_ID         | 0x80),
         GET_PASSWORD_REPLY           = (GET_PASSWORD        | 0x80),
         SET_USER_ID_REPLY            = (SET_USER_ID         | 0x80),
         SET_PASSWORD_REPLY           = (SET_PASSWORD        | 0x80),
         EXTENDED_ERROR_REPLY         = (EXTENDED_ERROR      | 0x80),
         NAVIGATOR_PARAMETERS_REPLY   = (NAVIGATOR_PARAMETERS| 0x80),
         SERVER_PARAMETERS_REPLY      = (SERVER_PARAMETERS   | 0x80),
         
         // Navigation related requests.
         START_ROUTE                  = 0x20,
         STOP_ROUTE                   = 0x21,
         CONTINUE_ROUTE               = 0x22,
         GET_CURRENT_ROUTE            = 0x23,
         GET_DESTINATIONS             = 0x24,
         WHERE_AM_I                   = 0x25,
         SYNC_DESTINATIONS            = 0x26,
         DOWNLOAD_ROUTE               = 0x27,
         DELETE_DEST                  = 0x28,
         ADD_DEST                     = 0x29,
         SEARCH                       = 0x2a,
         CALL_SERVER_FOR_ROUTE        = 0x2b,
         CALL_VOICE_FOR_ROUTE         = 0x2c,
         NEW_ROUTE                    = 0x2d,
         DOWNLOAD_DATA                = 0x30,
         START_TUNNEL                 = 0x31,
         DATA_CHUNK                   = 0x32,

         // Navigation related replies
         START_ROUTE_REPLY            = (START_ROUTE          | 0x80),
         STOP_ROUTE_REPLY             = (STOP_ROUTE           | 0x80),
         CONTINUE_ROUTE_REPLY         = (CONTINUE_ROUTE       | 0x80),
         GET_CURRENT_ROUTE_REPLY      = (GET_CURRENT_ROUTE    | 0x80),
         GET_DESTINATIONS_REPLY       = (GET_DESTINATIONS     | 0x80),
         WHERE_AM_I_REPLY             = (WHERE_AM_I           | 0x80),
         SYNC_DESTINATIONS_REPLY      = (SYNC_DESTINATIONS    | 0x80),
         DOWNLOAD_ROUTE_REPLY         = (DOWNLOAD_ROUTE       | 0x80),
         DELETE_DEST_REPLY            = (DELETE_DEST          | 0x80),
         ADD_DEST_REPLY               = (ADD_DEST             | 0x80),
         SEARCH_REPLY                 = (SEARCH               | 0x80),
         CALL_SERVER_FOR_ROUTE_REPLY  = (CALL_SERVER_FOR_ROUTE| 0x80),
         CALL_VOICE_FOR_ROUTE_REPLY   = (CALL_VOICE_FOR_ROUTE | 0x80),
         NEW_ROUTE_REPLY              = (NEW_ROUTE            | 0x80),
         DOWNLOAD_DATA_REPLY          = (DOWNLOAD_DATA        | 0x80),
         START_TUNNEL_REPLY           = (START_TUNNEL         | 0x80),
         DATA_CHUNK_REPLY             = (DATA_CHUNK           | 0x80),

         // Input data
         SUBSCRIBE                    = 0x40,
         UNSUBSCRIBE                  = 0x41,
         GET_INPUT_DATA               = 0x42,
         INPUT_DATA                   = 0x43,
         
         // Input data replies
         SUBSCRIBE_REPLY              = (SUBSCRIBE            | 0x80),
         UNSUBSCRIBE_REPLY            = (UNSUBSCRIBE          | 0x80),
         GET_INPUT_DATA_REPLY         = (GET_INPUT_DATA       | 0x80),
         INPUT_DATA_REPLY             = (INPUT_DATA           | 0x80),
         
         // Requests without replys.
         ROUTE_INFO                   = 0x70,
         STATUS_PACKET                = 0x71,
      };

      /**
       * Types of packets in regards of contents.
       */
      enum DataType {
         only_type                               = 0x00,
         type_and_uint8                          = 0x01,
         type_and_uint16                         = 0x02,
         type_and_uint32                         = 0x03,
          
         type_and_two_uint8                      = 0x08,
         type_and_two_uint16                     = 0x09,
         type_and_two_uint32                     = 0x0a,
          
         type_and_strings                        = 0x10,
         type_and_uint8_and_strings              = 0x11,  
         type_and_uint16_and_strings             = 0x12,    
         type_and_uint32_and_strings             = 0x13,
          
         type_and_two_uint32_and_strings         = 0x18,
         type_and_three_uint32_and_three_strings = 0x19,
          
         type_and_data                           = 0x20, 
          
      };
   } // namespace Packet_Enums
} // namespace isab
#endif // PACKET_ENUMS_H

