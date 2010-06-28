/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "arch.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtRouteMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/GuiProtFileMess.h"
#include "GuiProt/GuiParameterEnums.h"

#include "GpsSatelliteInfo.h"

#include "nav2util.h"

namespace isab {


// GuiProtMess ////////////////////////////////////
const uint16 GuiProtMess::NO_SYNC_MESS_ID = 0;


const uint8 GuiProtMess::THIN_CLIENT_GUI_PROT_NBR = 0;
// We have jumped from 0 to 3 to distinguish the difference.
const uint8 GuiProtMess::CURRENT_GUI_PROT_NBR = 3;

GuiProtMess::GuiProtMess(Buffer* buf) :
   Serializable(buf)
{
   m_dataType = static_cast<GuiProtEnums::DataType >
      (uint8(buf->readNext8bit()));

   m_messageType = static_cast<GuiProtEnums::MessageType>
      (uint8(buf->readNext8bit()));

   m_ID = buf->readNextUnaligned16bit();
}

GuiProtMess::GuiProtMess(GuiProtEnums::DataType dataType, 
                         GuiProtEnums::MessageType messType)
{
   m_dataType = dataType;
   m_messageType = messType;
   m_ID = NO_SYNC_MESS_ID; // Reserve room for message id.
}

GuiProtMess::GuiProtMess(GuiProtEnums::MessageType messType)
{
   m_dataType = GuiProtEnums::only_type;
   m_messageType = messType;
   m_ID = NO_SYNC_MESS_ID; // Reserve room for message id.
}

GuiProtMess::~GuiProtMess()
{
}

GuiProtMess*
GuiProtMess::createMsgFromBuf(Buffer* buf)
{
   GuiProtMess* result = NULL;
   
   const uint8* msgData = buf->accessRawData();
   const GuiProtEnums::DataType  msgDataType = GuiProtEnums::DataType (msgData[0]);

   if (msgDataType == GuiProtEnums::type_and_data){
      // This is a message with a class associated.

      const GuiProtEnums::MessageType msgType = 
         GuiProtEnums::MessageType(msgData[1]);

      switch (msgType){
      case GuiProtEnums::MESSAGETYPE_ERROR:
         {
            result = new ErrorMess(buf);
         } break;
      case GuiProtEnums::REQUEST_FAILED:
         {
            result = new RequestFailedMess(buf);
         } break;
      case GuiProtEnums::GET_TOP_REGION_LIST_REPLY:
         {
            result = new GetTopRegionReplyMess(buf);
         } break;
      case GuiProtEnums::SET_GENERAL_PARAMETER:
         {
            result = new GeneralParameterMess(buf);
         } break;
      case GuiProtEnums::GET_GENERAL_PARAMETER:
         {
            result = new GeneralParameterMess(buf);
         } break;
      case GuiProtEnums::ADD_FAVORITE:
         {
            result = new AddFavoriteMess(buf);
         } break;
      case GuiProtEnums::CHANGE_FAVORITE:
         {
            result = new ChangeFavoriteMess(buf);
         } break;
      case GuiProtEnums::GET_FAVORITES_REPLY:
         {
            result = new GetFavoritesReplyMess(buf);
         } break;
      case GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY:
         {
            result = new GetFavoritesAllDataReplyMess(buf);
         } break;
      case GuiProtEnums::GET_FAVORITE_INFO_REPLY:
         {
            result = new GetFavoriteInfoReplyMess(buf);
         } break;
      case GuiProtEnums::UPDATE_ROUTE_INFO:
         result = new UpdateRouteInfoMess(buf);
         break;
      case GuiProtEnums::SATELLITE_INFO:
         result = new SatelliteInfoMess(buf);
         break;
      case GuiProtEnums::ROUTE_LIST:
         result = new RouteListMess(buf);
         break;
      case GuiProtEnums::GET_FILTERED_ROUTE_LIST:
         result = new GetFilteredRouteListMess(buf);
         break;
      case GuiProtEnums::PREPARE_SOUNDS:
         result = new PrepareSoundsMess(buf);
         break;
      case GuiProtEnums::SOUND_FILE_LIST:
         result = new SoundFileListMess(buf);
         break;
      case GuiProtEnums::UPDATE_POSITION_INFO:
         result = new UpdatePositionMess(buf);
         break;
      case GuiProtEnums::ROUTE_TO_POSITION:
         result = new RouteToPositionMess(buf);
         break;
      case GuiProtEnums::ROUTE_MESSAGE:
         result = new RouteMess(buf);
         break;
      case GuiProtEnums::SEARCH:
         result = new SearchMess(buf);
         break;
      case GuiProtEnums::GET_SEARCH_AREAS_REPLY:
         result = new SearchAreaReplyMess(buf);
         break;
      case GuiProtEnums::GET_SEARCH_ITEMS_REPLY:
         result = new SearchItemReplyMess(buf);
         break;
      case GuiProtEnums::GET_FULL_SEARCH_DATA_REPLY:
         result = new FullSearchDataReplyMess(buf);
         break;
      case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID_REPLY:
         result = new FullSearchDataFromItemIdReplyMess(buf);
         break;
      case GuiProtEnums::GET_MAP:
         result = new GetMapMess(buf);
         break;
      case GuiProtEnums::GET_MAP_REPLY:
         result = new MapReplyMess(buf);
         break;
      case GuiProtEnums::STARTED_NEW_ROUTE:
         result = new StartedNewRouteMess(buf);
         break;
      case GuiProtEnums::SEND_MESSAGE_REPLY:
         result = new MessageSentMess(buf);
         break;
      case GuiProtEnums::GET_MORE_SEARCH_DATA:
         result = new GetMoreDataMess(buf);
         break;
      case GuiProtEnums::SEND_MESSAGE:
         result = new SendMessageMess(buf);
         break;
      case GuiProtEnums::REQUEST_LICENSE_UPGRADE:
         result = new LicenseKeyMess(buf);
         break;
      case GuiProtEnums::LICENSE_UPGRADE_REPLY:
         result = new LicenseReplyMess(buf);
         break;
      case GuiProtEnums::FILEOP_GUI_MESSAGE:
         result = new GuiProtFileMess(buf);
         break;
      case GuiProtEnums::ADD_FAVORITE_FROM_SEARCH:
      case GuiProtEnums::CONNECT_GPS:
      case GuiProtEnums::DISCONNECT_GPS:
      case GuiProtEnums::FAVORITES_CHANGED:
      case GuiProtEnums::GET_FAVORITES:
      case GuiProtEnums::GET_FAVORITES_ALL_DATA:
      case GuiProtEnums::GET_FAVORITE_INFO:
      case GuiProtEnums::GET_TOP_REGION_LIST:
      case GuiProtEnums::PANIC_ABORT:
      case GuiProtEnums::PARAMETER_CHANGED:
      case GuiProtEnums::REMOVE_FAVORITE:
      case GuiProtEnums::ROUTE_TO_FAVORITE:
      case GuiProtEnums::ROUTE_TO_HOT_DEST:
      case GuiProtEnums::ROUTE_TO_SEARCH_ITEM:
      case GuiProtEnums::REROUTE:
      case GuiProtEnums::SORT_FAVORITES:
      case GuiProtEnums::SYNC_FAVORITES:
      case GuiProtEnums::SYNC_FAVORITES_REPLY:
      case GuiProtEnums::PROGRESS_INDICATOR:
      case GuiProtEnums::PREPARE_SOUNDS_REPLY:
      case GuiProtEnums::PLAY_SOUNDS:
      case GuiProtEnums::PLAY_SOUNDS_REPLY:
      case GuiProtEnums::LOAD_AUDIO_SYNTAX:
      case GuiProtEnums::CHANGE_UIN:
      case GuiProtEnums::SEARCH_RESULT_CHANGED:
      case GuiProtEnums::GET_SEARCH_AREAS:
      case GuiProtEnums::GET_SEARCH_ITEMS:
      case GuiProtEnums::GET_FULL_SEARCH_DATA:
      case GuiProtEnums::GET_FULL_SEARCH_DATA_FROM_ITEMID:
      case GuiProtEnums::INVALIDATE_ROUTE:
      case GuiProtEnums::ROUTE_TO_POSITION_CANCEL:
      case GuiProtEnums::REQUEST_CROSSING_SOUND:
      case GuiProtEnums::PARAMETERS_SYNC:
      case GuiProtEnums::PARAMETERS_SYNC_REPLY:
      case GuiProtEnums::CELL_INFO_TO_SERVER:
      case GuiProtEnums::CELL_INFO_FROM_SERVER:
      case GuiProtEnums::GET_VECTOR_MAP:
      case GuiProtEnums::GET_VECTOR_MAP_REPLY:
      case GuiProtEnums::GET_MULTI_VECTOR_MAP:
      case GuiProtEnums::GET_MULTI_VECTOR_MAP_REPLY:
      case GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP:
      case GuiProtEnums::FORCEFEED_MULTI_VECTOR_MAP_REPLY:
      case GuiProtEnums::GUI_TUNNEL_DATA:
      case GuiProtEnums::GUI_TUNNEL_DATA_REPLY:
      case GuiProtEnums::GUI_TO_NGP_DATA:
      case GuiProtEnums::GUI_TO_NGP_DATA_REPLY:
         //do nothing
         break;
#ifdef __VC32__
      default:
				  // ERR...
		break;
#endif
		}; // switch
   } else if (msgDataType == GuiProtEnums::type_and_length_and_data){
      result = new DataGuiMess(buf);
   } else{
      // This is a generic message or a generic
      // message subclass.
      result = new GenericGuiMess(buf);
   }

   return result;
}

void 
GuiProtMess::serialize(Buffer* buf) const
{
   uint32 messStartPos = buf->getWritePos();


   /* Write the header */

   // Reserve room for message length.
   buf->writeNextUnaligned32bit(0);

   buf->writeNext8bit(
      static_cast<uint8>(m_dataType & 0xff));
   buf->writeNext8bit(
      static_cast<uint8>(m_messageType & 0xff));
   buf->writeNextUnaligned16bit(m_ID);


   /* Write message data */ 
   serializeMessData(buf);


   /* Write protocol version and length first in header */
   uint32 messAfterEndPos = buf->getWritePos();
   buf->setWritePos(messStartPos);
   uint32 length = messAfterEndPos - messStartPos;
   buf->writeNext8bit(CURRENT_GUI_PROT_NBR); // current protocol version.
   buf->writeNext8bit((length >> 16) & 0xff); // length
   buf->writeNext8bit((length >> 8)  & 0xff);
   buf->writeNext8bit(length         & 0xff);
   buf->setWritePos(messAfterEndPos);

} // serialize

GuiProtEnums::DataType     
GuiProtMess::getMessageDataType() const
{
   return m_dataType;
}
         
GuiProtEnums::MessageType 
GuiProtMess::getMessageType() const
{
   return m_messageType;
}

uint16                   
GuiProtMess::getMessageID() const
{
   return m_ID;
}

void
GuiProtMess::setMessageId(uint16 id){
   m_ID = id;
}





// GenericGuiMess///////////////////////////////////
GenericGuiMess::GenericGuiMess(Buffer* buf) :
   GuiProtMess(buf)
{
   initMembers();
   
   switch( m_dataType ){
   case GuiProtEnums::only_type:
      {
         // Nothing more to do.
      } break;
   case GuiProtEnums::type_and_bool:
   case GuiProtEnums::type_and_uint8:
      m_firstUint8 = buf->readNext8bit();
      break;
   case GuiProtEnums::type_and_two_bool:
   case GuiProtEnums::type_and_two_uint8:
      m_firstUint8 = buf->readNext8bit();
      m_secondUint8 = buf->readNext8bit();
      break;      
   case GuiProtEnums::type_and_three_bool:
      m_firstUint8 = buf->readNext8bit();
      m_secondUint8 = buf->readNext8bit();
      m_thirdUint8 = buf->readNext8bit();
      break;
   case GuiProtEnums::type_and_uint16:
   case GuiProtEnums::type_and_two_uint16:
      {
         m_firstUint16 = buf->readNextUnaligned16bit();
         
         if (m_dataType == GuiProtEnums::type_and_two_uint16){
            m_secondUint16 = buf->readNextUnaligned16bit();
         }
      } break;
   case GuiProtEnums::type_and_uint32:
   case GuiProtEnums::type_and_two_uint32:
      {
         m_firstUint32 = buf->readNextUnaligned32bit();
         
         if (m_dataType == GuiProtEnums::type_and_two_uint32){
            m_secondUint32 = buf->readNextUnaligned32bit();
         } 
      } break;
   case GuiProtEnums::type_and_int64:
      {
         m_firstInt64 = buf->readNextUnaligned64bit();
      } break;
   case GuiProtEnums::type_and_string:
      {
         m_firstString = buf->getNextCharStringAlloc();
      } break;
   case GuiProtEnums::type_and_two_strings:
      {
         m_firstString = buf->getNextCharStringAlloc();
         m_secondString = buf->getNextCharStringAlloc();
      } break;
   case GuiProtEnums::type_and_data:
      {
#ifdef _LINUX
         cerr << "GenericGuiMess::GenericGuiMess. "
              << "Tried to create GenericGuiMess with type_and_data type. "
              << "These messages are not allowed to be generic ."
              << endl;
#endif
      } break;
   case GuiProtEnums::type_and_length_and_data:
      {
#ifdef _LINUX
         cerr << "GenericGuiMess::GenericGuiMess. Tried to create "
              << "GenericGuiMess with type_and_length_and_data type. "
              << "These messages have their own generic class DataGuiMess."
              << endl;
#endif
      } break;
   //no default!
   } // switch
} // GenericGuiMess(Buffer* buf)


GenericGuiMess::GenericGuiMess( const GenericGuiMess* o )
      : GuiProtMess( o->getMessageDataType(), o->getMessageType() )
{
   initMembers();
   
   m_firstUint8   = o->m_firstUint8;
   m_secondUint8  = o->m_secondUint8;
   m_firstUint16  = o->m_firstUint16;
   m_secondUint16 = o->m_secondUint16;
   m_firstUint32  = o->m_firstUint32;
   m_secondUint32 = o->m_secondUint32;
   m_firstInt64   = o->m_firstInt64;
   m_firstString  = strdup_new( o->m_firstString );
   m_secondString = strdup_new( o->m_secondString );
}


void 
GenericGuiMess::initMembers()
{
   m_firstUint8   = MAX_UINT8;
   m_secondUint8  = MAX_UINT8;
   m_firstUint16  = MAX_UINT16;
   m_secondUint16 = MAX_UINT16;
   m_firstUint32  = MAX_UINT32;
   m_secondUint32 = MAX_UINT32;
   m_firstInt64   = MAX_INT64;
   m_firstString  = NULL;
   m_secondString  = NULL;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType) :
   GuiProtMess( GuiProtEnums::only_type, 
                messType)
{
   initMembers();
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                               bool firstBool) :
   GuiProtMess( GuiProtEnums::type_and_bool, 
                messType)
{
   initMembers();
   m_firstUint8 = firstBool;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                               bool firstBool, bool secondBool) :
   GuiProtMess( GuiProtEnums::type_and_two_bool, 
                messType)
{
   initMembers();
   m_firstUint8 = firstBool;
   m_secondUint8 = secondBool;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                               bool firstBool, bool secondBool, 
                               bool thirdBool) :
   GuiProtMess( GuiProtEnums::type_and_three_bool, messType)
{
   initMembers();
   m_firstUint8 = firstBool;
   m_secondUint8 = secondBool;
   m_thirdUint8 = thirdBool;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType,
                     const char* firstString) :
   GuiProtMess( GuiProtEnums::type_and_string, 
                messType)
{
   initMembers();
   m_firstString = strdup_new(firstString);
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType,
                     const char* firstString,
                     const char* secondString) :
   GuiProtMess( GuiProtEnums::type_and_two_strings, 
                messType)
{
   initMembers();
   m_firstString = strdup_new(firstString);
   m_secondString = strdup_new(secondString);
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint8 firstUint8) :
   GuiProtMess( GuiProtEnums::type_and_uint8, 
                messType)
{
   initMembers();
   m_firstUint8 = firstUint8;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint8 firstUint8, uint8 secondUint8) :
   GuiProtMess( GuiProtEnums::type_and_two_uint8, 
                messType)
{
   initMembers();
   m_firstUint8  = firstUint8;
   m_secondUint8 = secondUint8;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint16 firstUint16) :
   GuiProtMess( GuiProtEnums::type_and_uint16, 
                messType)
{
   initMembers();
   m_firstUint16 = firstUint16;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint16 firstUint16, uint16 secondUint16) :
   GuiProtMess( GuiProtEnums::type_and_two_uint16, 
                messType)
{
   initMembers();
   m_firstUint16  = firstUint16;
   m_secondUint16 = secondUint16;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint32 firstUint32) :
   GuiProtMess( GuiProtEnums::type_and_uint32, 
                messType)
{
   initMembers();
   m_firstUint32  = firstUint32;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     int64 firstInt64) :
   GuiProtMess( GuiProtEnums::type_and_int64, 
                messType)
{
   initMembers();
   m_firstInt64  = firstInt64;
}

GenericGuiMess::GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint32 firstUint32, uint32 secondUint32) :
   GuiProtMess( GuiProtEnums::type_and_two_uint32, 
                messType)
{
   initMembers();
   m_firstUint32  = firstUint32;
   m_secondUint32 = secondUint32;
}

GenericGuiMess::~GenericGuiMess()
{
   // Do not delete m_firstString.
}

void
GenericGuiMess::deleteMembers()
{
   delete[] m_firstString;
   m_firstString = NULL;
   delete[] m_secondString;
   m_secondString = NULL;
}


void 
GenericGuiMess::serializeMessData(Buffer* buf) const
{
   switch (m_dataType){
   case GuiProtEnums::only_type:
      {
         // Nothing more to do.
      } break;
   case GuiProtEnums::type_and_bool:
   case GuiProtEnums::type_and_two_bool:
   case GuiProtEnums::type_and_uint8:
   case GuiProtEnums::type_and_two_uint8:
      {
         buf->writeNext8bit(m_firstUint8);
         
         if (m_dataType == GuiProtEnums::type_and_two_uint8 ||
             m_dataType == GuiProtEnums::type_and_two_bool){
            buf->writeNext8bit(m_secondUint8);
         }
      } break;
   case GuiProtEnums::type_and_three_bool:
      buf->writeNext8bit(m_firstUint8);
      buf->writeNext8bit(m_secondUint8);
      buf->writeNext8bit(m_thirdUint8);
      break;
   case GuiProtEnums::type_and_uint16:
   case GuiProtEnums::type_and_two_uint16:
      {
         buf->writeNextUnaligned16bit(m_firstUint16);
         
         if (m_dataType == GuiProtEnums::type_and_two_uint16){
            buf->writeNextUnaligned16bit(m_secondUint16);
         }
      } break;
   case GuiProtEnums::type_and_uint32:
   case GuiProtEnums::type_and_two_uint32:
      {
         buf->writeNextUnaligned32bit(m_firstUint32);
         
         if (m_dataType == GuiProtEnums::type_and_two_uint32){
            buf->writeNextUnaligned32bit(m_secondUint32);
         } 
      } break;
   case GuiProtEnums::type_and_int64:
      {
         buf->writeNextUnaligned64bit(m_firstInt64);
      } break;
   case GuiProtEnums::type_and_string:
      {
         buf->writeNextCharString(m_firstString);
         break;
      }
   case GuiProtEnums::type_and_two_strings:
      {
         buf->writeNextCharString(m_firstString);
         buf->writeNextCharString(m_secondString);
         break;
      }
   case GuiProtEnums::type_and_data:
      {
#ifdef _LINUX
         cerr << "GenericGuiMess::serializeMessData. "
              << "Tried to serialize GenericGuiMess with type_and_data type. "
              << "These messages are not allowed to be generic."
              << endl;
#endif
      } break;
   case GuiProtEnums::type_and_length_and_data:
      {
#ifdef _LINUX
         cerr << "GenericGuiMess::GenericGuiMess. Tried to create "
              << "GenericGuiMess with type_and_length_and_data type. "
              << "These messages have their own generic class DataGuiMess."
              << endl;
#endif
      } break;
   //no default!
   }
} // serializeMessData 


char*
GenericGuiMess::getFirstString() const
{
   return m_firstString;
}
char*
GenericGuiMess::getSecondString() const
{
   return m_secondString;
}

bool
GenericGuiMess::getFirstBool() const
{
   return !!m_firstUint8;
}

bool GenericGuiMess::getSecondBool() const
{
   return !!m_secondUint8;
}

bool GenericGuiMess::getThirdBool() const
{
   return !!m_thirdUint8;
}

uint8 
GenericGuiMess::getFirstUint8() const
{
   return m_firstUint8;
}

uint8 
GenericGuiMess::getSecondUint8() const
{
   return m_secondUint8;
}

uint16 
GenericGuiMess::getFirstUint16() const
{
   return m_firstUint16;
}

uint16 
GenericGuiMess::getSecondUint16() const
{
   return m_secondUint16;
}

uint32 
GenericGuiMess::getFirstUint32() const
{
   return m_firstUint32;
}

uint32 
GenericGuiMess::getSecondUint32() const
{
   return m_secondUint32;
}

int64 
GenericGuiMess::getFirstInt64() const
{
   return m_firstInt64;
}


// ErrorMess///////////////////////////////////
ErrorMess::ErrorMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_errorNbr = Nav2Error::ErrorNbr(buf->readNextUnaligned32bit());
   m_errorString = buf->getNextCharStringAlloc();
   m_errorURL = buf->getNextCharStringAlloc();
}

ErrorMess::~ErrorMess()
{
   // Do not delete m_errorString.
}

void
ErrorMess::deleteMembers()
{
   delete[] m_errorString;
   m_errorString = NULL;
   delete [] m_errorURL;
   m_errorURL = NULL;
}

void 
ErrorMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_errorNbr);
   buf->writeNextCharString(m_errorString);
   buf->writeNextCharString( m_errorURL );
} // serializeMessData 


ErrorMess::ErrorMess( Nav2Error::ErrorNbr errorNbr, 
                      const char* errorString,
                      const char* errorURL )
      : GuiProtMess( GuiProtEnums::type_and_data, 
                     GuiProtEnums::MESSAGETYPE_ERROR )
{
   m_errorNbr = errorNbr;
   m_errorString = strdup_new(errorString);
   m_errorURL = strdup_new( errorURL );
}


char* 
ErrorMess::getErrorString() const
{
   return m_errorString;
}

const char*
ErrorMess::getErrorURL() const {
   return m_errorURL;
}

Nav2Error::ErrorNbr
ErrorMess::getErrorNumber() const
{
   return m_errorNbr;
}


// RequestFailedMess///////////////////////////////////
RequestFailedMess::RequestFailedMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_errorNbr = Nav2Error::ErrorNbr(buf->readNextUnaligned32bit());
   m_errorString = buf->getNextCharStringAlloc();
   m_errorURL    = buf->getNextCharStringAlloc();
   m_failedRequestSeqId = buf->readNextUnaligned16bit();
   m_failedRequestMessageNbr = buf->readNext8bit();
   m_failedRequestString = buf->getNextCharStringAlloc();
}

RequestFailedMess::~RequestFailedMess()
{
   // Do not delete m_errorString or m_failedRequestString
}

void
RequestFailedMess::deleteMembers()
{
   delete[] m_errorString;
   delete[] m_failedRequestString;
   delete [] m_errorURL;
   m_errorURL = NULL;
}

void 
RequestFailedMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_errorNbr);
   buf->writeNextCharString(m_errorString);
   buf->writeNextCharString( m_errorURL );
   buf->writeNextUnaligned16bit(m_failedRequestSeqId);
   buf->writeNext8bit(m_failedRequestMessageNbr);
   buf->writeNextCharString(m_failedRequestString);
} // serializeMessData 


RequestFailedMess::RequestFailedMess(Nav2Error::ErrorNbr errorNbr,
                                     const char* errorString,
                                     const char* errorURL,
                                     uint16 failedRequestSeqId,
                                     GuiProtEnums::MessageType failedRequestMessageNbr,
                                     const char* failedRequestString) :
   GuiProtMess( GuiProtEnums::type_and_data, 
                GuiProtEnums::REQUEST_FAILED)
{
   initMembers( errorNbr,
                errorString,
                errorURL,
                failedRequestSeqId,
                static_cast<uint8>(failedRequestMessageNbr),
                failedRequestString );
}


void
RequestFailedMess::initMembers(Nav2Error::ErrorNbr errorNbr,
                               const char* errorString,
                               const char* errorURL,
                               uint16 failedRequestSeqId,
                               uint8 failedRequestMessageNbr,
                               const char* failedRequestString)
{
   m_errorNbr = errorNbr;
   m_errorString = strdup_new(errorString);
   m_errorURL = strdup_new( errorURL );
   m_failedRequestSeqId = failedRequestSeqId;
   m_failedRequestMessageNbr = failedRequestMessageNbr;
   m_failedRequestString = strdup_new(failedRequestString);  

} // initMembers


uint32
RequestFailedMess::getErrorNbr() const
{
   return m_errorNbr;
}

char*
RequestFailedMess::getErrorString() const
{
   return m_errorString;
}

const char* 
RequestFailedMess::getErrorURL() const {
   return m_errorURL;
}

uint32 
RequestFailedMess::getFailedRequestSeqId() const
{
   return m_failedRequestSeqId;
}

uint8 
RequestFailedMess::getFailedRequestMessageNbr() const
{
   return m_failedRequestMessageNbr;
}

char* 
RequestFailedMess::getFailedRequestString() const
{
   return m_failedRequestString;
}

//UpdatePositionMess ///////////////////////////////////////////////
UpdatePositionMess::UpdatePositionMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_lat             = buf->readNextUnaligned32bit();
   m_lon             = buf->readNextUnaligned32bit();
   m_routelat        = buf->readNextUnaligned32bit();
   m_routelon        = buf->readNextUnaligned32bit();
   m_positionQuality = buf->readNext8bit();
   m_heading         = buf->readNext8bit();
   m_routeheading    = buf->readNext8bit();
   m_headingQuality  = buf->readNext8bit();
   m_speed           = buf->readNextUnaligned16bit();
   m_speedQuality    = buf->readNext8bit();
   m_alt             = buf->readNextUnaligned32bit();
   m_timeStampMillis = buf->readNext32bit();
   uint32 numHints   = buf->readNext32bit();
   
   for (unsigned int i = 0; i < numHints; i++) {
      HintNode curHint;
      curHint.lat = buf->readNext32bit();
      curHint.lon = buf->readNext32bit();
      curHint.velocityCmPS = buf->readNext32bit();
      m_positionHints.push_back(curHint);
   }

}

UpdatePositionMess::UpdatePositionMess(int32 lat, int32 lon, 
                                       int32 routelat, int32 routelon,
                                       uint8 positionQuality,
                                       uint8 heading, uint8 routeheading,
                                       uint8 headingQuality,
                                       uint16 speed, uint8 speedQuality,
                                       int32 alt,
                                       uint32 timeStampMillis,
                                       const std::vector<HintNode>& hintNodes) :
   GuiProtMess(GuiProtEnums::type_and_data,GuiProtEnums::UPDATE_POSITION_INFO),
   m_lat(lat), m_lon(lon), m_routelat(routelat), m_routelon(routelon),
   m_positionQuality(positionQuality),
   m_heading(heading), m_routeheading(routeheading),
   m_headingQuality(headingQuality),
   m_speed(speed), m_speedQuality(speedQuality), m_alt(alt),
   m_timeStampMillis( timeStampMillis ),
   m_positionHints( hintNodes )
{
}

UpdatePositionMess::~UpdatePositionMess()
{
}

void UpdatePositionMess::deleteMembers()
{
}

void UpdatePositionMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_lat);
   buf->writeNextUnaligned32bit(m_lon);
   buf->writeNextUnaligned32bit(m_routelat);
   buf->writeNextUnaligned32bit(m_routelon);
   buf->writeNext8bit(m_positionQuality);
   buf->writeNext8bit(m_heading);
   buf->writeNext8bit(m_routeheading);
   buf->writeNext8bit(m_headingQuality);
   buf->writeNextUnaligned16bit(m_speed);
   buf->writeNext8bit(m_speedQuality);
   buf->writeNextUnaligned32bit(m_alt);
   buf->writeNext32bit(m_timeStampMillis);
   buf->writeNext32bit(m_positionHints.size());
      
   for(unsigned int i = 0; i < m_positionHints.size(); i++) {
      buf->writeNext32bit(m_positionHints[i].lat);
      buf->writeNext32bit(m_positionHints[i].lon);
      buf->writeNext32bit(m_positionHints[i].velocityCmPS);
   }
}

int32 UpdatePositionMess::getLat() const
{
   return m_lat;
}

int32 UpdatePositionMess::getLon() const
{
   return m_lon;
}

int32 UpdatePositionMess::getRouteLat() const
{
   return m_routelat;
}

int32 UpdatePositionMess::getRouteLon() const
{
   return m_routelon;
}

uint8 UpdatePositionMess::getRouteHeading() const
{
   return m_routeheading;
}

uint8 UpdatePositionMess::positionQuality() const
{
   return m_positionQuality;
}

uint8 UpdatePositionMess::getHeading() const
{
   return m_heading;
}

uint8 UpdatePositionMess::headingQuality() const
{
   return m_headingQuality;
}

uint16 UpdatePositionMess::getSpeed() const
{
   return m_speed;
}

uint8 UpdatePositionMess::speedQuality() const
{
   return m_speedQuality;
}
int32 UpdatePositionMess::getAlt() const
{
   return m_alt;
}

uint32 isab::UpdatePositionMess::getTimeStampMillis() const
{
   return m_timeStampMillis;
}

const std::vector<HintNode>&
isab::UpdatePositionMess::getInterpolationHints() const
{
   return m_positionHints;
}


// GetTopRegionReplyMess///////////////////////////////////
GetTopRegionReplyMess::GetTopRegionReplyMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_topRegionList = new TopRegionList(*buf);
}

GetTopRegionReplyMess::~GetTopRegionReplyMess()
{
   // Do not delete m_topRegionList.
}

void
GetTopRegionReplyMess::deleteMembers()
{
   delete m_topRegionList;
   m_topRegionList = NULL;
}

void 
GetTopRegionReplyMess::serializeMessData(Buffer* buf) const
{
   m_topRegionList->serialize(*buf);
} // serializeMessData 


GetTopRegionReplyMess::GetTopRegionReplyMess(TopRegionList* topRegionList):
   GuiProtMess( GuiProtEnums::type_and_data, 
                GuiProtEnums::GET_TOP_REGION_LIST_REPLY)
{
   m_topRegionList = topRegionList;
}


TopRegionList*
GetTopRegionReplyMess::getTopRegionList() const
{
   return m_topRegionList;
}


// GeneralParameterMess ///////////////////////////////////
GeneralParameterMess::GeneralParameterMess(Buffer* buf) :
   GuiProtMess(buf)
{
   /* Initialize data pointers to NULL. */
   initData();

   /* Get type of parameter. */
   m_type = buf->readNextUnaligned16bit();
   /* Get parameter id. */
   m_paramId = buf->readNextUnaligned16bit();

   /* Get number of elements in message. */
   m_numEntries = buf->readNextUnaligned32bit();

   /* Depending on type, read rest of data. */
   switch (m_type) {
      case GuiProtEnums::paramTypeInt32:
         {
            /* Int32 data. */
            m_integerData = new int32[m_numEntries];
            for (int32 i = 0; i < m_numEntries; i++) {
               m_integerData[i] = buf->readNextUnaligned32bit();
            }
         }
         break;
      case GuiProtEnums::paramTypeFloat:
         {
            /* Float data. */
            m_floatData = new float[m_numEntries];
            for (int32 i = 0; i < m_numEntries; i++) {
               m_floatData[i] = buf->readNextFloat();
            }
         }
         break;
      case GuiProtEnums::paramTypeString:
         {
            m_stringData = new char*[m_numEntries];
            for (int32 i = 0; i < m_numEntries; i++) {
               m_stringData[i] = buf->getNextCharStringAlloc();
               // Empty strings are replaced with NULL.
               if (strcmp(m_stringData[i], "") == 0){
                  delete[] m_stringData[i];
                  m_stringData[i] = NULL;
               }
            }
         }
         break;
      case GuiProtEnums::paramTypeBinary:
         {
            m_binaryData = new uint8[m_numEntries];
            buf->readNextByteArray(m_binaryData, m_numEntries);
         }
         break;
      default:
         /* No such type. Used for "Get". */
         break;
   }
}

GeneralParameterMess::~GeneralParameterMess()
{
}

void
GeneralParameterMess::deleteMembers()
{
   if (m_type == GuiProtEnums::paramTypeString) {
      for (int32 i = 0; i < m_numEntries; i++){
         delete[] m_stringData[i];
      }
   }
   delete[] m_stringData;
   delete[] m_binaryData;
   delete[] m_floatData;
   delete[] m_integerData;
   initData();
   m_type = GuiProtEnums::paramTypeInvalid;
}

void 
GeneralParameterMess::serializeMessData(Buffer* buf) const
{
   /* Type of parameter. */
   buf->writeNextUnaligned16bit(m_type);
   /* Parameter id. */
   buf->writeNextUnaligned16bit(m_paramId);

   /* Number of elements in message. */
   buf->writeNextUnaligned32bit(m_numEntries);

   /* Depending on type, read rest of data. */
   switch (m_type) {
      case GuiProtEnums::paramTypeInt32:
         {
            /* Int32 data. */
            for (int32 i = 0; i < m_numEntries; i++) {
               buf->writeNextUnaligned32bit(m_integerData[i]);
            }
         }
         break;
      case GuiProtEnums::paramTypeFloat:
         {
            /* Float data. */
            for (int32 i = 0; i < m_numEntries; i++) {
               buf->writeNextFloat(m_floatData[i]);
            }
         }
         break;
      case GuiProtEnums::paramTypeString:
         {
            for (int32 i = 0; i < m_numEntries; i++) {
               // (Writes "" if NULL)
               buf->writeNextCharString(m_stringData[i]);
            }
         }
         break;
      case GuiProtEnums::paramTypeBinary:
         {
            buf->writeNextByteArray(m_binaryData, m_numEntries);
         }
         break;
      default:
         /* No such type. Used for "Get". */
         break;
   }
} // serializeMessData 

void
GeneralParameterMess::initData()
{
   m_stringData = NULL;
   m_binaryData = NULL;
   m_floatData = NULL;
   m_integerData = NULL;
   m_numEntries = 0;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::GET_GENERAL_PARAMETER)
{
   initData();
   m_type = GuiProtEnums::paramTypeInvalid;
   m_paramId = paramId;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, char* string) :
   GuiProtMess( GuiProtEnums::type_and_data, 
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeString;
   m_numEntries = 1;
   m_stringData = new char*[1];
   m_stringData[0] = string;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, char** strings,
      int32 numEntries) :
   GuiProtMess( GuiProtEnums::type_and_data, 
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeString;
   m_numEntries = numEntries;
   m_stringData = strings;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, int32 data) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeInt32;
   m_numEntries = 1;
   m_integerData = new int32[1];
   m_integerData[0] = data;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, int32 *data,
      int32 numEntries) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeInt32;
   m_numEntries = numEntries;
   m_integerData = data;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, float data) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeFloat;
   m_numEntries = 1;
   m_floatData = new float[1];
   m_floatData[0] = data;
}
GeneralParameterMess::GeneralParameterMess(uint16 paramId, float *data,
      int32 numEntries) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeFloat;
   m_numEntries = numEntries;
   m_floatData = data;
}

GeneralParameterMess::GeneralParameterMess(uint16 paramId, uint8 *data,
      int32 numEntries) :
   GuiProtMess( GuiProtEnums::type_and_data,
                GuiProtEnums::SET_GENERAL_PARAMETER)
{
   initData();
   m_paramId = paramId;
   m_type = GuiProtEnums::paramTypeBinary;
   m_numEntries = numEntries;
   m_binaryData = data;
}

char**
GeneralParameterMess::getStringData() const
{
   return m_stringData;
}
uint8*
GeneralParameterMess::getBinaryData() const
{
   return m_binaryData;
}
float*
GeneralParameterMess::getFloatData() const
{
   return m_floatData;
}
int32*
GeneralParameterMess::getIntegerData() const
{
   return m_integerData;
}

uint16
GeneralParameterMess::getParamId() const
{
   return m_paramId;
}
int32
GeneralParameterMess::getNumEntries() const
{
   return m_numEntries;
}
uint16 
GeneralParameterMess::getParamType() const
{
   return m_type;
}

// UpdateRouteInfoMess ////////////////////////////////
UpdateRouteInfoMess::UpdateRouteInfoMess(Buffer* buf) :
   GuiProtMess(buf)
{
   RouteInfo* tmp = new RouteInfo();
   tmp->readFromBuf(buf);
   m_routeInfo = tmp;
}


UpdateRouteInfoMess::UpdateRouteInfoMess(const RouteInfo* info) : 
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::UPDATE_ROUTE_INFO),
   m_routeInfo(info)
{
}

UpdateRouteInfoMess::~UpdateRouteInfoMess()
{
}

void UpdateRouteInfoMess::deleteMembers()
{
   //stupid compiler....
   delete const_cast<RouteInfo*>(m_routeInfo);
   m_routeInfo = NULL;
}

void UpdateRouteInfoMess::serializeMessData(Buffer* buf) const
{
   if(m_routeInfo != NULL){
      m_routeInfo->writeToBuf(buf);
   }
}

const RouteInfo* 
UpdateRouteInfoMess::getRouteInfo() const
{
   return m_routeInfo;
}


// SatelliteInfoMess ////////////////////////////////
SatelliteInfoMess::SatelliteInfoMess(Buffer* buf) :
   GuiProtMess(buf)
{
   GpsSatInfoHolder* satInfo = new GpsSatInfoHolder();

   satInfo->readFromBuffer(buf);

   m_satInfo = satInfo;
}


SatelliteInfoMess::SatelliteInfoMess(GpsSatInfoHolder* info) : 
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::SATELLITE_INFO),
   m_satInfo(info)
{
}

SatelliteInfoMess::~SatelliteInfoMess()
{
}

void SatelliteInfoMess::deleteMembers()
{
   delete const_cast<GpsSatInfoHolder*>(m_satInfo);
   m_satInfo = NULL;
}

void SatelliteInfoMess::serializeMessData(Buffer* buf) const
{
   if(m_satInfo != NULL){
      m_satInfo->writeToBuffer(buf);
   } else {
      /* Add nonsense zero entries. */
      buf->writeNextUnaligned32bit(0);
   }
}

GpsSatInfoHolder* 
SatelliteInfoMess::getSatInfo()
{
   return m_satInfo;
}

// RouteListMess ////////////////////////////////
RouteListMess::RouteListMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_routeList = new RouteList(buf);
}


RouteListMess::RouteListMess(const RouteList* list) :
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::ROUTE_LIST),
   m_routeList(list)
{
}

RouteListMess::~RouteListMess()
{
}

void RouteListMess::deleteMembers()
{
   //stupid compiler....
   delete const_cast<RouteList*>(m_routeList);
   m_routeList = NULL;
}

void RouteListMess::serializeMessData(Buffer* buf) const
{
   if(m_routeList != NULL){
      m_routeList->writeToBuf(buf);
   }
}

const RouteList* 
RouteListMess::getRouteList() const
{
   return m_routeList;
}

// GetFilteredRouteListMess ////////////////////////////////
GetFilteredRouteListMess::GetFilteredRouteListMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_startWpt = buf->readNextUnaligned16bit();
   m_numWpts  = buf->readNextUnaligned16bit();
}


GetFilteredRouteListMess::GetFilteredRouteListMess(int16 startWpt, int16 numWpts) :
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::GET_FILTERED_ROUTE_LIST),
   m_startWpt(startWpt), m_numWpts(numWpts)
{
}

GetFilteredRouteListMess::~GetFilteredRouteListMess()
{
}

void GetFilteredRouteListMess::deleteMembers()
{
}

void GetFilteredRouteListMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned16bit(m_startWpt);
   buf->writeNextUnaligned16bit(m_numWpts);
}

// PrepareSoundsMess ////////////////////////////////
PrepareSoundsMess::PrepareSoundsMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_numSounds = buf->readNextUnaligned16bit();
   int * sounds = new int[m_numSounds];
   for (int i = 0; i < m_numSounds; ++i) {
      sounds[i] = buf->readNextUnaligned16bit();
   }
   m_soundList = sounds;
}

PrepareSoundsMess::PrepareSoundsMess(int numSounds, const int *sounds) :
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::PREPARE_SOUNDS),
   m_soundList(sounds), m_numSounds(numSounds)
{
}


PrepareSoundsMess::~PrepareSoundsMess()
{
}

void PrepareSoundsMess::deleteMembers()
{
   //this is ugly and due to games played with the allocation of variables etc.
   //Needs to be fixed properly. FIXME.
   delete[] const_cast<int*>(m_soundList);
   m_soundList = NULL;
   m_numSounds = 0;
}

void PrepareSoundsMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned16bit(m_numSounds);
   for (int i = 0; i < m_numSounds; ++i) {
      buf->writeNextUnaligned16bit(m_soundList[i]);
   }
}

const int * 
PrepareSoundsMess::getSounds() const
{
   return m_soundList;
}

int PrepareSoundsMess::getNumSounds() const
{ 
   return m_numSounds;
}

// SoundFileListMess ///////////////////////////////////
SoundFileListMess::SoundFileListMess(Buffer* buf) :
   GuiProtMess(buf)
{
   /* Initialize data pointers to NULL. */
   initData();

   /* Get number of elements in message. */
   m_numEntries = buf->readNextUnaligned32bit();

   m_stringData = new char*[m_numEntries];
   for (int32 i = 0; i < m_numEntries; i++) {
      m_stringData[i] = buf->getNextCharStringAlloc();
      // Empty strings are replaced with NULL.
      if (strcmp(m_stringData[i], "") == 0){
         delete[] m_stringData[i];
         m_stringData[i] = NULL;
      }
   }
}

SoundFileListMess::~SoundFileListMess()
{
}

void
SoundFileListMess::deleteMembers()
{
   for (int32 i = 0; i < m_numEntries; i++){
      delete[] m_stringData[i];
   }
   delete[] m_stringData;
   initData();
}

void 
SoundFileListMess::serializeMessData(Buffer* buf) const
{
   /* Number of elements in message. */
   buf->writeNextUnaligned32bit(m_numEntries);

   for (int32 i = 0; i < m_numEntries; i++) {
      // (Writes "" if NULL)
      buf->writeNextCharString(m_stringData[i]);
   }
} // serializeMessData 

void
SoundFileListMess::initData()
{
   m_stringData = NULL;
   m_numEntries = 0;
}

SoundFileListMess::SoundFileListMess(char** strings, int32 numEntries) :
   GuiProtMess( GuiProtEnums::type_and_data, 
                GuiProtEnums::SOUND_FILE_LIST)
{
   initData();
   m_numEntries = numEntries;
   m_stringData = strings;
}

char**
SoundFileListMess::getStringData() const
{
   return m_stringData;
}
int32
SoundFileListMess::getNumEntries() const
{
   return m_numEntries;
}


// SendMessageMess
SendMessageMess::SendMessageMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_media = GuiProtEnums::UserMessageType(buf->readNextUnaligned16bit());
   m_object = GuiProtEnums::ObjectType(buf->readNextUnaligned16bit());
   m_id = strdup_new(buf->getNextCharString());
   m_from = strdup_new(buf->getNextCharString());
   m_to = strdup_new(buf->getNextCharString());
   m_sig = strdup_new(buf->getNextCharString());
}

SendMessageMess::SendMessageMess(GuiProtEnums::UserMessageType messT,
      GuiProtEnums::ObjectType objT, char* id, char* from,
      char* to, char* sig) : 
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::SEND_MESSAGE),
   m_media(messT), m_object(objT), m_id(id), m_from(from), m_to(to), m_sig(sig)
{
}


SendMessageMess::~SendMessageMess()
{
}

void SendMessageMess::deleteMembers()
{
   delete[] m_id;
   delete[] m_from;
   delete[] m_to;
   delete[] m_sig;
   m_id = NULL;
   m_from = NULL;
   m_to = NULL;
   m_sig = NULL;
}

void SendMessageMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned16bit(m_media);
   buf->writeNextUnaligned16bit(m_object);
   buf->writeNextCharString(m_id);
   buf->writeNextCharString(m_from);
   buf->writeNextCharString(m_to);
   buf->writeNextCharString(m_sig);
}

GuiProtEnums::UserMessageType SendMessageMess::getMedia() const 
{
   return m_media;
}

GuiProtEnums::ObjectType SendMessageMess::getObject() const
{
   return m_object;
}

char* SendMessageMess::getId() const
{
   return m_id;
}

char* SendMessageMess::getFrom() const
{
   return m_from;
}

char* SendMessageMess::getTo() const
{
   return m_to;
}

char* SendMessageMess::getSignature() const
{
   return m_sig;
}

//=========================MessageSentMess =====================

MessageSentMess::MessageSentMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_len = buf->readNextUnaligned32bit();
   if (m_len > 0) {
      m_data = new uint8[m_len];
      buf->readNextByteArray(m_data, m_len);
   } else {
      m_data = NULL;
   }
}

MessageSentMess::MessageSentMess(uint8* data, unsigned len) :
   GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::SEND_MESSAGE_REPLY)
{
   m_data = data;
   m_len = len;
}

MessageSentMess::~MessageSentMess()
{
}

void MessageSentMess::deleteMembers()
{
   delete[] m_data;
   m_data = NULL;
   m_len = 0;
}

void MessageSentMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_len);
   buf->writeNextByteArray(m_data, m_len);
}

unsigned MessageSentMess::size() const
{ 
   return m_len;
}

uint8* MessageSentMess::getData(unsigned* size) const
{
   if(size){
      *size = m_len;
   }
   return m_data;
}

///=================LicenseKeyMess=======================

LicenseKeyMess::LicenseKeyMess(Buffer* buf) :
   GuiProtMess(buf), m_key(NULL), m_phone(NULL), 
   m_name(NULL), m_email(NULL), m_region(MAX_UINT32)
{
   m_region = buf->readNextUnaligned32bit();
   m_key    = strdup_new(buf->getNextCharString());
   m_phone  = strdup_new(buf->getNextCharString());
   m_name   = strdup_new(buf->getNextCharString());
   m_email  = strdup_new(buf->getNextCharString());
   m_optional  = strdup_new(buf->getNextCharString());
}

LicenseKeyMess::LicenseKeyMess(const char* key, const char* phone, 
                               uint32 region) : 
   GuiProtMess(GuiProtEnums::type_and_data, 
               GuiProtEnums::REQUEST_LICENSE_UPGRADE), m_key(NULL), 
   m_phone(NULL), m_name(NULL), m_email(NULL), m_optional(NULL),
   m_region(region)
{
   m_key = strdup_new(key);
   m_phone = strdup_new(phone);
}

LicenseKeyMess::LicenseKeyMess(const char* key, const char* phone, 
                               const char* name, const char* email, 
                               const char* optional,
                               uint32 region) : 
   GuiProtMess(GuiProtEnums::type_and_data, 
               GuiProtEnums::REQUEST_LICENSE_UPGRADE), m_key(NULL), 
   m_phone(NULL), m_name(NULL), m_email(NULL), m_optional(NULL),
   m_region(region)
{
   m_key   = strdup_new(key);
   m_phone = strdup_new(phone);
   m_name  = strdup_new(name);
   m_email = strdup_new(email);
   m_optional = strdup_new(optional);
}

LicenseKeyMess::~LicenseKeyMess()
{
}

void LicenseKeyMess::deleteMembers()
{
   delete[] m_key;
   delete[] m_phone;
   delete[] m_name;
   delete[] m_email;
   delete[] m_optional;
}
  
void LicenseKeyMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_region);
   buf->writeNextCharString(m_key);
   buf->writeNextCharString(m_phone);
   buf->writeNextCharString(m_name);
   buf->writeNextCharString(m_email);
   buf->writeNextCharString(m_optional);
}

const char* LicenseKeyMess::getKey() const
{
   return m_key;
}

const char* LicenseKeyMess::getPhone() const
{
   return m_phone;
}

const char* LicenseKeyMess::getName() const
{
   return m_name;
}

const char* LicenseKeyMess::getEmail() const
{
   return m_email;
}

const char* LicenseKeyMess::getOptional() const
{
   return m_optional;
}

uint32 LicenseKeyMess::getRegion() const
{
   return m_region;
}

LicenseReplyMess::LicenseReplyMess(Buffer* buf) :
   GuiProtMess(buf)
{
   m_keyOk    = buf->readNext8bit();
   m_phoneOk  = buf->readNext8bit();
   m_regionOk = buf->readNext8bit();
   m_nameOk   = buf->readNext8bit();   
   m_emailOk  = buf->readNext8bit();
   m_type     = GuiProtEnums::WayfinderType(buf->readNext8bit());
}      

LicenseReplyMess::LicenseReplyMess(bool keyOk, bool phoneOk, bool regionOk, 
                                   bool nameOk, bool emailOk, 
                                   GuiProtEnums::WayfinderType type) :
   GuiProtMess(GuiProtEnums::type_and_data,
               GuiProtEnums::LICENSE_UPGRADE_REPLY),
   m_keyOk(keyOk), m_phoneOk(phoneOk), m_regionOk(regionOk), m_nameOk(nameOk),
   m_emailOk(emailOk), m_type(type)
{
}

LicenseReplyMess::~LicenseReplyMess()
{
}

void LicenseReplyMess::deleteMembers()
{
}           

void LicenseReplyMess::serializeMessData(Buffer* buf) const
{
   buf->writeNext8bit(m_keyOk);
   buf->writeNext8bit(m_phoneOk);
   buf->writeNext8bit(m_regionOk);
   buf->writeNext8bit(m_nameOk);
   buf->writeNext8bit(m_emailOk);
   buf->writeNext8bit(m_type);
}

bool LicenseReplyMess::isKeyOk() const
{
   return m_keyOk;
}

bool LicenseReplyMess::isPhoneOk() const
{
   return m_phoneOk;
}

bool LicenseReplyMess::isRegionOk() const
{
   return m_regionOk;
}

bool LicenseReplyMess::isNameOk() const
{
   return m_nameOk;
}

bool LicenseReplyMess::isEmailOk() const
{
   return m_emailOk;
}

bool LicenseReplyMess::allOk() const
{
   return m_keyOk && m_phoneOk && m_regionOk && m_nameOk && m_emailOk;
}

GuiProtEnums::WayfinderType LicenseReplyMess::getWayfinderType() const
{
   return m_type;
}

DataGuiMess::DataGuiMess(Buffer* buf)
 : GuiProtMess(buf),
   m_size(0),
   m_data(0),
   m_ad_size(0),
   m_ad_data(0)
{
   m_size = buf->readNextUnaligned32bit();
   m_ad_size = buf->readNextUnaligned32bit();
   m_data = new uint8[m_size];
   buf->readNextByteArray(m_data, m_size);
   if (m_ad_size) {
      m_ad_data = new uint8[m_ad_size];
      buf->readNextByteArray(m_ad_data, m_ad_size);
   }
   m_requestVer = buf->readNext8bit();
}

DataGuiMess::DataGuiMess(GuiProtEnums::MessageType messType, 
                         uint32 size, const uint8* data,
                         uint32 ad_size, const uint8* ad_data,
                         uint8 requestVer
                         ) :
   GuiProtMess(GuiProtEnums::type_and_length_and_data, messType), 
   m_size(size), m_data(0),
   m_ad_size(ad_size), m_ad_data(0)
{
   m_data = new uint8[size];
   memcpy(m_data, data, size);

   if (ad_size) {
      m_ad_data = new uint8[ad_size];
      memcpy(m_ad_data, ad_data, ad_size);
   }
   m_requestVer = requestVer;
}

DataGuiMess::~DataGuiMess()
{
}

void DataGuiMess::deleteMembers()
{
   delete[] m_data;
   m_data = NULL;
   m_size = 0;

   delete[] m_ad_data;
   m_ad_data = NULL;
   m_ad_size = 0;
}
  
void DataGuiMess::serializeMessData(Buffer* buf) const
{
   buf->writeNextUnaligned32bit(m_size);
   buf->writeNextUnaligned32bit(m_ad_size);
   buf->writeNextByteArray(m_data, m_size);
   if (m_ad_size) {
      buf->writeNextByteArray(m_ad_data, m_ad_size);
   }
   buf->writeNext8bit(m_requestVer);
}

uint32 DataGuiMess::getSize() const
{
   return m_size;
}

const uint8* DataGuiMess::getData(uint32* length) const
{
   if(length){
      *length = m_size;
   }
   return m_data;
}

uint32 DataGuiMess::getAdditionalSize() const
{
   return m_ad_size;
}

const uint8* DataGuiMess::getAdditionalData(uint32* length) const
{
   if(length){
      *length = m_ad_size;
   }
   return m_ad_data;
}

uint8 DataGuiMess::getRequestVersion() const
{
   return m_requestVer;
}


} // namespace isab

