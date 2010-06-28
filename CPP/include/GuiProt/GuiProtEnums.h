/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_ENUMS_H
#define GUI_PROT_ENUMS_H


namespace isab {

   namespace GuiProtEnums{

      /**
       * The types of gui messages. Only 8 bits may be used.
       *
       * The numbers work like this:
       * The numbers below 0x80 are divided with  repect to what 
       * they are used for. Each use is further divided with 
       * respect to the direction that the messages are sent in. 
       * All numbers above 0x80 are replies. Not all messages 
       * have replies.
       * 
       * E.g. parameter messages use the number space
       * from 0x10 to 0x1f plus 0x90 to 0x9f for replies. 
       * Further division has the result that numbers from 0x10 
       * to 0x17 are used for messages from Nav2 to the GUI and 
       * messages from 0x18 to 0x1f are used for messages that 
       * can be sent in both directions.
       *
       *
       * NB! Numbers marked with ThinClientGuiProt reserved. 
       *     cannot be used before the ThinClient GUI prot 
       *     has been removed from Nav2.
       *
       *     Numbers marked with "Reserved for future use" are
       *     messages in the ThinClienGuiProt that is currently 
       *     used.
       */
      enum MessageType {
         // Messages marked with type_and_data below have classes
         // in that are subclasses to GuiProtMess, which defines 
         // the shape of the data.
         //
         // The other messages use GenericGuiMess.
         

     // Non-specific messages.

         /* From Nav2 to GUI. */

         // type_and_data.
         // GuiProtMess class = ErrorMess.
        
         // changed from ERROR to MESSAGETYPE_ERROR (ERROR is already defined in WCE)
         MESSAGETYPE_ERROR            = 0x01,

         // type_and_data.
         // GuiProtMess class = RequestFailedMess.
         //
         // This message actually works as a reply to
         // all messages that does not have specific 
         // replys.
         REQUEST_FAILED               = 0x02,
 

         // only_type
         PANIC_ABORT                  = 0x03,
            
         // NB! ThinClientGuiProt reserved. Do not use ERROR          | 0x80  == 0x81.
         // NB! ThinClientGuiProt reserved. Do not use REQUEST_FAILED | 0x80  == 0x82.
         // NB! ThinClientGuiProt reserved. Do not use 0x05           | 0x80  == 0x85.






      // Parameter messages.
 

         /* From GUI to Nav2. */

         // only_type.
         GET_TOP_REGION_LIST          = 0x10,


         // NB! ThinClientGuiProt reserved. Do not use 0x12 | 0x80 == 0x92.
         
         // type_and_uint16
         // uint16 parameterId 0 to 0xFFFF. Tells what parameter to get. 
         //                      The is taken from the Parameter module's
         //                      Param Ids.
         //
         // Replied to with SET_GENERAL_PARAMETER.
         GET_GENERAL_PARAMETER        = 0x13,
         

         /* Both from GUI to Nav2 and from Nav2 to GUI. */


         // type_and_data.
         // GuiProtMess class = GeneralParameterMess.
         //
         // This message works both as a reply to GET_GENERAL_PARAMETER (1)
         // and as an unsolicited message (2). 
         //
         // 1. It works as a reply back to the GUI when it has sent
         //    GET_GENERAL_PARAMETER to Nav2.
         //
         // 2. It works as an unsolicited message from the GUI to Nav2 
         //    when the GUI wants Nav2 to update its parameter with the
         //    value supplied in the message.
         //
         //
         SET_GENERAL_PARAMETER       = 0x1a,

         /* From GUI to Nav2 replys.*/

         // type_and_data.
         // GuiProtMess class = GetTopRegionReplyMess.
         GET_TOP_REGION_LIST_REPLY     = (GET_TOP_REGION_LIST     | 0x80),

         // NB! ThinClientGuiProt reserved. Do not use 0x12 | 0x80 == 0x92.
              
         // From GUI to Nav2
         // type_and_data
         // GuiProtMess class SendMessageMess
         SEND_MESSAGE                 = 0x1e,
         // From Nav2 to GUI
         // type_and_data
         // GuiProtMess class MessageSentMess
         SEND_MESSAGE_REPLY           = (SEND_MESSAGE            | 0x80),
         /* From Nav2 to GUI*/

         // type_and_uint16
         // uint16 parameterType 0 to 0xFFFF. Tells what parameter
         //                      that changed.
         PARAMETER_CHANGED            = 0x1f,
 
         // GUI to Nav2
         // only_type
         PARAMETERS_SYNC              = 0x20,
         // Nav2 to GUI
         // type_and_uint32
         PARAMETERS_SYNC_REPLY        = (PARAMETERS_SYNC         | 0x80),

         // File operation messages from and to NavTask
         FILEOP_GUI_MESSAGE           = 0x21,

      // Favorite messages.

         /* From GUI to Nav2. */

         // type_and_two_uint16
         // uint16 startIndex ( 0 ==> all from beginnig)
         // uint16 endIndex   ( MAX_UINT16 ==> all to end)
         GET_FAVORITES                = 0x30,
         // type_and_data.
         // GuiProtMess class = GetFavoritesReplyMess.
         GET_FAVORITES_REPLY          = (GET_FAVORITES          | 0x80),

         // type_and_two_uint16
         // uint16 startIndex ( 0 ==> all from beginnig)
         // uint16 endIndex   ( MAX_UINT16 ==> all to end)
         GET_FAVORITES_ALL_DATA       = 0x31,
         // type_and_data.
         // GuiProtMess class = GetFavoritesAllDataReplyMess.
         GET_FAVORITES_ALL_DATA_REPLY = (GET_FAVORITES_ALL_DATA | 0x80),

         // type_and_uint16.
         // uint16 sortingType (GuiProtMess::SortingType).
         SORT_FAVORITES               = 0x32,

         // only_type.
         SYNC_FAVORITES               = 0x33,
         SYNC_FAVORITES_REPLY         = (SYNC_FAVORITES         | 0x80),

         // type_and_uint32.
         // uint32 destinationId. 
         GET_FAVORITE_INFO            = 0x34,
         // type_and_data.
         // GuiProtMess class = GetFavoriteInfoReplyMess.
         GET_FAVORITE_INFO_REPLY      = (GET_FAVORITE_INFO      | 0x80),



         // type_and_data.
         // GuiProtMess class = AddFavoriteMess.
         //
         // Favorite* favorite. This favorite should not have an id
         //                     assigned to it.
         ADD_FAVORITE                 = 0x35,

         // type_and_uint32.
         // uint32 searchMatchId. This is the match id used by the GUI.
         //                       It can only be mapped to a server id
         //                       using a VanillaMatchTable.
         ADD_FAVORITE_FROM_SEARCH     = 0x36,

         // type_and_uint32
         // uint32 favoriteId.
         REMOVE_FAVORITE              = 0x37,

         // type_and_data.
         // GuiProtMess class = ChangeFavoriteMess.
         //
         // Favorite* favorite. This favorite's id tells which favoite to
         //                     change, and it's data tells what to change
         //´                    the favorite to.
         CHANGE_FAVORITE              = 0x38,

         // type_and_uint32
         // uint32 favoriteId.
         ROUTE_TO_FAVORITE            = 0x39,

         // only_type
         ROUTE_TO_HOT_DEST            = 0x3a,

         // type_and_data
         //
         // This is the replacement for all of the above route messages
         // which includes the origin.
         ROUTE_MESSAGE                = 0x3b,


         /* From Nav2 to GUI. */

         // only_type.
         FAVORITES_CHANGED            = 0x40, 

         // NB! ThinClientGuiProt reserved. Do not use 0x41.

         // NB! ThinClientGuiProt reserved. Do not use 0x41 | 0x80 == 0xa1.
         // NB! ThinClientGuiProt reserved. Do not use 0x42 | 0x80 == 0xa2.

      // Mixed messages.         
         
         /* From GUI to Nav2. */

         // only_type
         CONNECT_GPS                      = 0x50,

         // only_type
         DISCONNECT_GPS                   = 0x51,

         /* From Nav2 to GUI */
         // type_and_bool
         // bool indicator on
         PROGRESS_INDICATOR = 0x52,
         
         // type_and_data
         // GuiProtMess class = PrepareSoundsMess
         PREPARE_SOUNDS = 0x53,

         /* From GUI to Nav2 */
         // type_and_uint32
         PREPARE_SOUNDS_REPLY = 0x80 | PREPARE_SOUNDS,

         // only_type
         PLAY_SOUNDS = 0x54,
         //only_type
         PLAY_SOUNDS_REPLY = 0x80 | PLAY_SOUNDS,

         // type_and_data
         // GuiProtMess class = SoundFileListMess
         SOUND_FILE_LIST = 0x55,
         // type_and_string
         LOAD_AUDIO_SYNTAX = 0x56,

         // for changing the UIN in Nav2 (also known as UserLogin)
         CHANGE_UIN = 0x57,

         /* From Nav2 to GUI */

         // type_and_data
         // GuiProtMess class = UpdatePositionMess
         UPDATE_POSITION_INFO = 0x58,

         /* From GUI to Nav2 */
         // type_and_uint16
         REQUEST_CROSSING_SOUND = 0x5a,

         // From GUI to Nav2
         // type_and_data
         // GuiProtMess class = LicenseKeyMess
         REQUEST_LICENSE_UPGRADE = 0x5b,
         //from Nav2 to GUI
         //type_and_three_bool
         LICENSE_UPGRADE_REPLY = 0x80 | REQUEST_LICENSE_UPGRADE,

         //from GUI to Nav2
         //type_and_length_and_data
         //GuiProtMess class = 
         CELL_INFO_TO_SERVER = 0x5c,
         //from Nav2 to GUI
         //type_and_length_and_data
         //GuiProtMess class = 
         CELL_INFO_FROM_SERVER = 0x80 | CELL_INFO_TO_SERVER,
      // Route Messages

         //type_and_length_and_data
         GUI_TUNNEL_DATA                             = 0x5d,
         //type_and_length_and_data
         GUI_TUNNEL_DATA_REPLY                       = GUI_TUNNEL_DATA | 0x80,


         //type_and_length_and_data
         GUI_TO_NGP_DATA                              = 0x5e,
         //type_and_length_and_data
         GUI_TO_NGP_DATA_REPLY                        = GUI_TO_NGP_DATA | 0x80,

         /* From GUI to Nav2 */
      
         // type_and_data
         // GuiProtMess class = GET_FILTERED_ROUTE_LIST
         GET_FILTERED_ROUTE_LIST = 0x59,

         // type_and_data
         // GuiProtMess class = RouteListMess
         ROUTE_LIST = 0x80 | GET_FILTERED_ROUTE_LIST,
 

         // type_and_data
         // GuiProtMess class = RouteToPositionMess
         ROUTE_TO_POSITION           = 0x60,
         
         //type_and_string
         // string search item id.
         ROUTE_TO_SEARCH_ITEM = 0x61,

         /* From Nav2 to GUI */
         // type_and_uint64
         // int64 routeid
         // Route id when new route is available or zero when
         // route following stopped.
         STARTED_NEW_ROUTE = 0x62,

         //only type
         REROUTE = 0x63,

         //only_type
         // Sent from GUI with routeid zero means "stop route following".
         INVALIDATE_ROUTE  = 0x64,

         ROUTE_TO_POSITION_CANCEL = 0x65,
         // Reserved for future use.
         //
         // ROUTE_DOWNLOADED    (nRouteDownloadedMessage in ver 0)    = 0x69,

         
         // type_and_data
         // GuiProtMess class == UpdateRouteInfoMess
         UPDATE_ROUTE_INFO                                             = 0x6a,

         // type_and_data
         // GuiProtMess class == SatelliteInfoMess
         SATELLITE_INFO                                                = 0x6b,

      // Search Messages

         /* From GUI to Nav2 */

         // type_and_data
         // GuiProtMess class = SearchMess
         SEARCH = 0x70,

         // type_and_two_bool
         SEARCH_RESULT_CHANGED = 0x71,
         
         //only_type
         GET_SEARCH_AREAS = 0x72,

         //type_and_data
         // GuiProtMess class = SearchAreaReplyMess
         GET_SEARCH_AREAS_REPLY = GET_SEARCH_AREAS | 0x80,

         // only_type
         GET_SEARCH_ITEMS = 0x73,
         
         // type_and_data
         // GuiProtMess class = SearchItemsReplyMess
         GET_SEARCH_ITEMS_REPLY = GET_SEARCH_ITEMS | 0x80,

         // type_and_two_uint16
         GET_FULL_SEARCH_DATA = 0x74,
         
         // type_and_data
         // GuiProtMess class = FullSearchDataMess
         GET_FULL_SEARCH_DATA_REPLY = GET_FULL_SEARCH_DATA | 0x80,

         //type_and_data
         // GuiProtMess class = GetMapMess
         GET_MAP = 0x75,
         //type_and_data
         // GuiProtMess class = MapReplyMess
         GET_MAP_REPLY = GET_MAP | 0x80,

         //type_and_data
         /// GuiProtMess class GetMoreDataMess
         GET_MORE_SEARCH_DATA         = 0x76,

         //type_and_string
         GET_VECTOR_MAP = 0x77,
         //type_and_length_and_data
         GET_VECTOR_MAP_REPLY = GET_VECTOR_MAP | 0x80,

         //type_and_length_and_data
         GET_MULTI_VECTOR_MAP = 0x78,
         //type_and_length_and_data
         GET_MULTI_VECTOR_MAP_REPLY = GET_MULTI_VECTOR_MAP | 0x80,
         
         // type_and_two_strings
         GET_FULL_SEARCH_DATA_FROM_ITEMID = 0x79,
         
         // type_and_data
         // GuiProtMess class = FullSearchDataMess
         GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY = GET_FULL_SEARCH_DATA_FROM_ITEMID | 0x80,

         // type_and_length_and_data. There is no such request but for
         // making the numbering work it put it here anyway
         FORCEFEED_MULTI_VECTOR_MAP = 0x7a,
         //type_and_length_and_data
         FORCEFEED_MULTI_VECTOR_MAP_REPLY = (FORCEFEED_MULTI_VECTOR_MAP | 
                                             0x80),


      }; // enum MessageType

      /**
       * The data types of gui messages. Only 8 bits may be used.
       */
      enum DataType {
         only_type                               = 0x00,
         type_and_uint8                          = 0x01,
         type_and_uint16                         = 0x02,
         type_and_uint32                         = 0x03,
         type_and_bool                           = 0x04,
         type_and_string                         = 0x05,
         type_and_int64                          = 0x06,

         type_and_two_uint8                      = 0x08,
         type_and_two_uint16                     = 0x09,
         type_and_two_uint32                     = 0x0a,
         type_and_two_bool                       = 0x0b,
         type_and_two_strings                    = 0x0c,
         
         type_and_three_bool                     = 0x13,

         type_and_data                           = 0x20, 
         type_and_length_and_data                = 0x21,
      }; // enum DataType


   } // namespace GuiProtEnums.
} // namespace isab

#endif // GUI_PROT_ENUMS_H

