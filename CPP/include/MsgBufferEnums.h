/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef MSG_BUFFER_ENUMS_H
#define MSG_BUFFER_ENUMS_H

namespace isab{

   namespace MsgBufferEnums {

      enum MsgTypes {
         TIMER = 1,
         MODULE_CONNECT_LOWLEVEL,
         MODULE_NEGOTIATE_ADDRESSSPACE,
         MODULE_MANAGE_MULTICAST,
         MODULE_STARTUP_COMPLETE,
         MODULE_SHUTDOWN_PREPARE,
         MODULE_SHUTDOWN_PREPARE_COMPLETE,
         MODULE_SHUTDOWN_NOW,
         MODULE_SHUTDOWN_COMPLETE,
         INTERNAL_MODULE_MSG,
         CTRLHUB_START_SHUTDOWN_PROCEDURE,
         SERIAL_SEND_DATA,
         SERIAL_RECEIVE_DATA,
         GPS_POSITION_VELOCITY,
         GPS_STATUS,
         GPS_SATELLITE_INFO,
         GPS_TIME,
         GPS_RESET_RECEIVER,
         CONNECTION_CTRL,
         CONNECTION_NOTIFY,
         PARAM_CLEAR,
         PARAM_SET_INT32,
         PARAM_SET_FLOAT,
         PARAM_SET_STRING,
         PARAM_SET_BINARY_BLOCK,
         PARAM_REWRITE_ORIG_FILE,
         PARAM_VAL_INT32,
         PARAM_VAL_FLOAT,
         PARAM_VAL_STRING,
         PARAM_VAL_BINARY_BLOCK,
         PARAM_VAL_NONE,
         PARAM_GET,

         //NavTask
         GET_OLD_STYLE_ROUTE,
         POSITION_STATE,
         OLD_STYLE_ROUTE_INFO,
         OLD_STYLE_ROUTE_DATA,
         NAV_TASK_COMMAND,
         NEW_ROUTE,
         ROUTE_CHUNK,
         KEEP_ROUTE,
         INVALIDATE_ROUTE,
         ROUTE_INFO,
         ROUTE_LIST,
         GET_FILTERED_ROUTE_LIST,
         NT_ROUTE_REQUEST,
         NT_ROUTE_REPLY,
         NT_GPS_SATELLITE_INFO,
         NAV_TASK_FILE_OPERATION,
         NEW_ROUTE_COORD,

         //NavServerCom uses these
         SEARCH_REQUEST,
         SEARCH_REPLY,
         WHERE_AM_I_REQUEST,
         WHERE_AM_I_REPLY,
         ROUTE_REQUEST,
         ROUTE_REPLY,
         BINARY_UPLOAD,
         BINARY_UPLOAD_REPLY, 
         PROGRESS_MESSAGE,
         CANCEL_NSC_REQUEST,

         SET_SERVER_PARAMS,
         NAV_SERVER_COM_ERROR,
         BINARY_DOWNLOAD,

         REQUEST_REFLASH,

         VOICE_REPLY,
         DATA_REPLY,
         CALL_VOICE,
         CALL_DATA,

         MAP_REQUEST,
         MAP_REPLY,
         VECTOR_MAP_REQUEST,
         VECTOR_MAP_REPLY,
         MULTI_VECTOR_MAP_REQUEST,
         MULTI_VECTOR_MAP_REPLY,

         FORCEFEED_MULTI_VECTOR_MAP_REQUEST,// This should not be used
         FORCEFEED_MULTI_VECTOR_MAP_REPLY,

         ADDITIONAL_INFO_REQ,
         ADDITIONAL_INFO_REPLY,

         MESSAGE_REQ,
         MESSAGE_REPLY,

         LATEST_NEWS,

         LICENSE_KEY,
         LICENSE_KEY_REPLY,

         PARAM_SYNC,
         PARAM_SYNC_REPLY,

         CELL_REPORT,
         CELL_CONFIRM,

         TUNNEL_DATA_REQUEST,
         TUNNEL_DATA_REPLY,
         // End of NavServerCom messages.

         //DisplaySerial
         NEW_ROUTE_MESSAGE, //stupid name, make something better up.

         
         // Destinations Module
         GET_FAVORITES_REQ,
         GET_FAVORITES_REPLY,

         GET_FAVORITES_ALL_DATA_REQ,
         GET_FAVORITES_ALL_DATA_REPLY,

         GET_FAVORITE_INFO_REQ,
         GET_FAVORITE_INFO_REPLY,

         SORT_FAVORITES_REQ,
         SYNC_FAVORITES_REQ,
         ADD_FAVORITE_REQ,
         ADD_FAVORITE_FROM_SEARCH_REQ,
         REMOVE_FAVORITE_REQ,
         CHANGE_FAVORITE_REQ,

         DEST_REQUEST_OK_REPLY,


         FAVORITES_CHANGED_MSG,

         // Error Module
         UNSOLICITED_ERROR_MSG,

         // Module super class.
         SOLICITED_ERROR_MSG,

         // BlueTooth
         BT_SEARCH_DEVICE,
         BT_FOUND_DEVICE,
         BT_REQUEST_SERIAL,
         BT_SERIAL_RESULT,

         // NGP Packet
         NAV_REQUEST,
         NAV_REPLY,
      };
      /* Broken Microsoft compiler rejects valid C++ idioms */
      enum {
         ADDR_MULTICAST_DOWN  = 0xfc000000,
         ADDR_MULTICAST_UP    = 0xfc010000,
         ADDR_DEFAULT         = 0xfb000000,
         ADDR_LINK_LOCAL      = 0xfb000001,
      };
   }

}

#endif
