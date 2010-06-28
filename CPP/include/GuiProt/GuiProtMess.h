/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_MESSAGES_H
#define GUI_PROT_MESSAGES_H

#include "GuiProt/Serializable.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/Favorite.h"
#include "TopRegion.h"
#include "Nav2Error.h"
#include "RouteInfo.h"
#include "GuiProt/ServerEnums.h"
#include "HintNode.h"
/* #include "GpsSatelliteInfo.h" */
#include <vector>

//More includes at end of file.

#ifndef __SYMBIAN32__
   using namespace std;
#endif

namespace isab {


   namespace GuiProtEnums {
      class ParameterTypeHolder;
   }
   

/**
  *   GuiProtMess is superclass to all GUI protocol message
  *   classes. Gui protocol message classes are used for 
  *   GuiProtEnums::MessageType:s with GuiProtEnums::DataType
  *   type_and_data.
  *
  */
class GuiProtMess : public Serializable
{

   public:

      /**
       * This id is assigned to m_id by default as 
       * place holder. Until sync has been obtained,
       * this id is unchanged in the messages.
       */
      static const uint16 NO_SYNC_MESS_ID;
      
      /**
       * The GUI protocol used by the PDA ThinClient.
       */
      static const uint8 THIN_CLIENT_GUI_PROT_NBR;

      /**
       * The GUI protocol used now.
       */
      static const uint8 CURRENT_GUI_PROT_NBR;

      
      /** Constructor.*/
      GuiProtMess(class Buffer* buf);

      GuiProtMess(GuiProtEnums::DataType dataType, GuiProtEnums::MessageType messType);

      GuiProtMess(GuiProtEnums::MessageType messType);

      /** Destructor. */
      virtual ~GuiProtMess();

      static GuiProtMess* createMsgFromBuf(class Buffer* buf);

      /** Serializable interface methods. */
      //@{
         /**
          * Writes this message to a buffer, which can be used
          * when sending the message. (Make sure the id is set).
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serialize(class Buffer* buf) const;
      //@}

      /** Abstract methods. */
      //@{
         /**
          * This method is called in order to serialize the 
          * data of the message.
          *
          * @param buf The buffer to write the data to.
          */
         virtual void serializeMessData(class Buffer* buf) const = 0;

         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers() = 0;
      //@}

      /** Get methods. */
      //@{
         GuiProtEnums::DataType    getMessageDataType() const;
         GuiProtEnums::MessageType getMessageType() const;
         uint16                    getMessageID() const;
      //@}

      /**
       * Use this method to set the id of the message, which is 
       * used for sync. Set the id right before sending the 
       * message to make sure that the number serie is not broken.
       *
       * @param id The id of this message. When sync has been 
       *           reached, this id should be incremented with
       *           1 for each message sent.
       */
      void setMessageId(uint16 id);

   protected:
      /**
       * Member variables.
       */
      //@{
         GuiProtEnums::DataType m_dataType;
         GuiProtEnums::MessageType m_messageType;
         uint16 m_ID;
      //@}

      /**
       * Protected methods.
       */
      //@{

      //@}

}; // GuiProtMess


/**
  *   A class for messages with simple data, i.e. all messages
  *   that have a data type other than type_and_data.
  *
  */
class GenericGuiMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GenericGuiMess(class Buffer* buf);

      /**
       * Copy constructor.
       */
      explicit GenericGuiMess( const GenericGuiMess* o );

      /**
       * Different generic messages.
       *
       * Only the data given in the constructor is set in the 
       * message. Use this class for messages with simple data.
       */
      explicit GenericGuiMess(GuiProtEnums::MessageType messType);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     bool firstBool);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     bool firstBool, bool secondBool);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     bool firstBool, bool secondBool, bool thirdBool);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType,
                     const char* firstString);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType,
                     const char* firstString, const char* secondString);
      
      explicit GenericGuiMess(GuiProtEnums::MessageType messType,
                     const char* firstString, const char* secondString,
                              const char* thirdString);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint8 firstUint8);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint8 firstUint8, uint8 secondUint8);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint16 firstUint16);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint16 firstUint16, uint16 secondUint16);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     int64 firstInt64);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint32 firstUint32);

      explicit GenericGuiMess(GuiProtEnums::MessageType messType, 
                     uint32 firstUint32, uint32 secondUint32);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
      GenericGuiMess(data);
       */

      /** Destructor. */
      virtual ~GenericGuiMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();

          /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}


      /**
        * Get methods for different members. Check the getMessageType 
        * to determine which members that are valid.
        */
      //@{  
         uint8 getFirstUint8() const;
         uint8 getSecondUint8() const;
         uint16 getFirstUint16() const;
         uint16 getSecondUint16() const;
         uint32 getFirstUint32() const;
         uint32 getSecondUint32() const;
         char* getFirstString() const;
         char* getSecondString() const;
         char* getThirdString() const;
         bool getFirstBool() const;
         bool getSecondBool() const;
         bool getThirdBool() const;
         int64 getFirstInt64() const;
      //@}
 
   protected:
      /**
       * Member variables.
       */
      //@{
         /**
          * m_firstUint8 stores both uint8 values
          * and bool values.
          */
         uint8 m_firstUint8;
         uint8 m_secondUint8;
         uint8 m_thirdUint8;
         uint16 m_firstUint16;
         uint16 m_secondUint16;
         uint32 m_firstUint32;
         uint32 m_secondUint32;
         int64 m_firstInt64;
         char* m_firstString;
         char* m_secondString;
      //@}

      /**
       * Protected methods.
       */
      //@{
         void initMembers();
      //@}

}; // GenericGuiMess



/**
  *   ErrorMess description.
  *
  */
class ErrorMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      ErrorMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param errorNbr The error number of the error.
       * @param errorString The error string of the error.
       * @param errorURL The URL for the error, to be opened in content
       *                 window if not empty and we have content window.
       */
      ErrorMess( Nav2Error::ErrorNbr errorNbr, 
                 const char* errorString,
                 const char* errorURL );
       

      /** Destructor. */
      virtual ~ErrorMess();

      /** GuiProtMess interface methods. */
      //@{ 
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();

         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the error string explaining this error in the 
       * choosen language. This string must be deleted 
       * by the caller.
       *
       * @return A string, which describes the error in the 
       *         choosen language.
       */
      char* getErrorString() const;

      /**
       * Returns the error URL. May be empty if so no need to open the
       * URL.
       *
       * @return An URL which to be opened in content window.
       */
      const char* getErrorURL() const;

      /**
       * @return Returns a number uniquely identifying the 
       *         error usign Nav2 error numbers.
       */
      Nav2Error::ErrorNbr getErrorNumber() const;
       
 
   protected:
      /**
       * Member variables.
       */
      //@{
         /**
          * Identyfies the error uniquely using Nav2 error 
          * codes.
          */
         Nav2Error::ErrorNbr m_errorNbr;

         /**
          * Describes the error in the used language.
          */
         char* m_errorString;

         /**
          * The error URl, mey be empty.
          */
         char* m_errorURL;
      //@}

}; // ErrorMess

/**
  *   RequestFailedMess description.
  *
  */
class RequestFailedMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      RequestFailedMess(class Buffer* buf);

      /**
       * Use one of these constructors to create a message to send.
       *
       * @param errorNbr The error number of the error causing the 
       *                 request to fail.
       *
       * @param errorString The error string of the error causing 
       *                    the request to fail.
       *
       * @param errorURL The URL for the error, to be opened in content
       *                 window if not empty and we have content window.
       * @param failedRequestSeqId The sequence id got from the GUI
       *                           on the request that failed.
       * @param failedRequestMessageNbr The message number on the 
       *                                 request that failed.
       * @param failedRequestString The "request failed" message,
       *                            telling the user what happened.
       */
      //@{
         RequestFailedMess(Nav2Error::ErrorNbr errorNbr,
                           const char* errorString,
                           const char* errorURL,
                           uint16 failedRequestSeqId,
                           GuiProtEnums::MessageType failedRequestMessageNbr,
                           const char* failedRequestString);

      //@}
       

      /** Destructor. */
      virtual ~RequestFailedMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       */
      //@{
         /**
          * Information on the error causing the request to 
          * fail.
          */
         //@{
            uint32 getErrorNbr() const;
            char*  getErrorString() const;

            /**
             * Returns the error URL. May be empty if so no need to open 
             * the URL.
             *
             * @return An URL which to be opened in content window.
             */
            const char* getErrorURL() const;
         //@}

         /**
          * Information on the request that failed.
          */
         //@{
            /**
             * @return The message sequence id for the message 
             *         that the request arrived to Nav2 in.
             *   
             *         NB! This one is always 0 for 
             *             ThinClient GUI protocol messages.
             */ 
            uint32 getFailedRequestSeqId() const;

            /**
             * @return Either a GuiProtEnums::MessageType or
             *         NavCtrlGuiProt::PacketNumber_t depending
             *         on what GUI protocol that was used when
             *         initiating the request.
             *
             *         These numbers never mix.
             *
             *         NavCtrlGuiProt::PacketNumber_t is the same
             *         as ThinClientGuiProtEnums::PacketNumber_t
             */
            uint8  getFailedRequestMessageNbr() const;
            /**
             * @return The "request failed" message, telling the user
             *         what happened.
             */
            char*  getFailedRequestString() const;
         //@}
      //@}
      

   protected:
      /**
       * Member variables.
       */
      //@{
         Nav2Error::ErrorNbr m_errorNbr;
         char*               m_errorString;
         char*               m_errorURL;

         uint16 m_failedRequestSeqId;
         uint8  m_failedRequestMessageNbr;
         char*  m_failedRequestString;

      //@}

      /**
       * Protected methods.
       */
      //@{
         /**
          * Used by the constructor.
          */
         void initMembers(Nav2Error::ErrorNbr errorNbr,
                          const char* errorString,
                          const char* errorURL,
                          uint16 failedRequestSeqId,
                          uint8 failedRequestMessageNbr,
                          const char* failedRequestString);
      //@}

}; // RequestFailedMess


class UpdatePositionMess : public GuiProtMess
{
public:
   UpdatePositionMess(class Buffer* buf);
   UpdatePositionMess(int32 lat, int32 lon,
                      int32 routelat, int32 routelon,
                      uint8 positionQuality,
                      uint8 heading, uint8 routeheading,
                      uint8 headingQuality,
                      uint16 speed, uint8 speedQuality,
                      int32 alt,
                      uint32 timeStampMillis,
                      const std::vector<HintNode>& hintNodes );
   virtual ~UpdatePositionMess();
   virtual void deleteMembers();
   virtual void serializeMessData(class Buffer* buf) const;

   int32 getLat() const;
   int32 getLon() const;
   // Contains the best positon on the route for the snap to route function.
   int32 getRouteLat() const;
   int32 getRouteLon() const;
   uint8 getRouteHeading() const;
   uint8 positionQuality() const;
   uint8 getHeading() const;
   uint8 headingQuality() const;
   uint16 getSpeed() const;
   uint8 speedQuality() const;
   int32 getAlt() const;
   uint32 getTimeStampMillis() const;
   const std::vector<HintNode>& getInterpolationHints() const;
private:
   int32 m_lat;
   int32 m_lon;

   // Contains the best positon on the route for the snap to route function.
   int32 m_routelat;
   int32 m_routelon;

   uint8 m_positionQuality; //enum?
   uint8 m_heading;
   uint8 m_routeheading;
   uint8 m_headingQuality; //enum?
   uint16 m_speed;
   uint8 m_speedQuality;
   int32 m_alt;
   uint32 m_timeStampMillis;
   std::vector<HintNode> m_positionHints;
};

/**
  *   GetTopRegionReplyMess description.
  *
  */
class GetTopRegionReplyMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GetTopRegionReplyMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param topRegionList The data is not deleted in the message 
       *                      destructor. This top region list is not
       *                      deleted by the message destructor.
       *
       */
      GetTopRegionReplyMess(TopRegionList* topRegionList);
       

      /** Destructor. */
      virtual ~GetTopRegionReplyMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();

         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return A top region list. The checksum member of it 
       *         should not be used by the GUI. This list may
       *         be empty.
       */
      TopRegionList* getTopRegionList() const;
       
 
   protected:
      /**
       * Member variables.
       */
      //@{
         TopRegionList* m_topRegionList;
      //@}

}; // GetTopRegionReplyMess


/**
  *   GeneralParameterMess description.
  *
  */
class GeneralParameterMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      explicit GeneralParameterMess(class Buffer* buf);

      /**
       * Use one of these constructors to create a message to send.
       *
       *        The data is not deleted in the message 
       *        destructor.
       */
      /* Constructor used to create a "get" message. */
      explicit GeneralParameterMess(uint16 paramId);
      /* Convenience constructors with only one value as input. */
      /** NOTE! These methods will allocate space for and copy data, */
      /** so it is necessary to perform deleteMembers()!!! */
      GeneralParameterMess(uint16 paramId, int32 data);
      GeneralParameterMess(uint16 paramId, float data);
      GeneralParameterMess(uint16 paramId, char* data);
      /* More general constructors that take any number of values */
      /* as input. */
      GeneralParameterMess(uint16 paramId, int32* data, int32 numEntries);
      GeneralParameterMess(uint16 paramId, float* data, int32 numEntries);
      GeneralParameterMess(uint16 paramId, uint8* data, int32 numEntries);
      GeneralParameterMess(uint16 paramId, char** data, int32 numEntries);
       

      /** Destructor. */
      virtual ~GeneralParameterMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return     Address of the data.
       */
       char**   getStringData() const;
       uint8*  getBinaryData() const;
       float*  getFloatData() const;
       int32* getIntegerData() const;

       uint16 getParamId() const;
       int32 getNumEntries() const;
       uint16 getParamType() const;
 
   protected:
       void initData();
      /**
       * Member variables.
       */
      //@{
          char** m_stringData;
          uint8* m_binaryData;
          float* m_floatData;
          int32* m_integerData;

          uint16 m_type;
          uint16 m_paramId;
          int32 m_numEntries;
      //@}

}; // GeneralParameterMess



class UpdateRouteInfoMess : public GuiProtMess
{
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      UpdateRouteInfoMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      UpdateRouteInfoMess(const RouteInfo* info);
   

      /** Destructor. */
      virtual ~UpdateRouteInfoMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   const RouteInfo* getRouteInfo() const;
   protected:
      /**
       * Member variables.
       */
      //@{
   /// A routeinfo object containing everything a guideview could ever
   /// hope for..
   const RouteInfo* m_routeInfo;
      //@}

}; // UpdateRouteInfoMess


class SatelliteInfoMess : public GuiProtMess
{
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      SatelliteInfoMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      SatelliteInfoMess(class GpsSatInfoHolder* info);


      /** Destructor. */
      virtual ~SatelliteInfoMess();

      /** GuiProtMess interface methods. */
      //@{  
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();  
      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
      class GpsSatInfoHolder* getSatInfo();
   protected:
      /**
       * Member variables.
       */
      //@{
      /// A routeinfo object containing everything a guideview could ever
      /// hope for..
      class GpsSatInfoHolder* m_satInfo;
      //@}

}; // SatelliteInfoMess



class RouteListMess : public GuiProtMess
{
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      RouteListMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      RouteListMess(const RouteList* list);
   

      /** Destructor. */
      virtual ~RouteListMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   const RouteList* getRouteList() const;
   protected:
      /**
       * Member variables.
       */
      //@{
   /// A routeinfo object containing everything a guideview could ever
   /// hope for..
   const RouteList* m_routeList;
      //@}

}; // RouteListMess


class GetFilteredRouteListMess : public GuiProtMess
{
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GetFilteredRouteListMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      GetFilteredRouteListMess(int16 startWpt, int16 numWpts);
   

      /** Destructor. */
      virtual ~GetFilteredRouteListMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   const int16 getStartWpt() const { return m_startWpt; }
   const int16 getNumWpts() const  { return m_numWpts;  }
   protected:
      /**
       * Member variables.
       */
      //@{
      int16 m_startWpt;
      int16 m_numWpts;
      //@}

}; // UpdateRouteInfoMess


class PrepareSoundsMess : public GuiProtMess
{
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      PrepareSoundsMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      PrepareSoundsMess(int numSounds, const int sounds[]);
   

      /** Destructor. */
      virtual ~PrepareSoundsMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   const int * getSounds() const;
   int getNumSounds() const;
   protected:
      /**
       * Member variables.
       */
      //@{
   const int *m_soundList;
   int m_numSounds;
      //@}

}; // PrepareSoundsMess

/**
  *   SoundFileListMess description.
  *
  */
class SoundFileListMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      explicit SoundFileListMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       *        The data is not deleted in the message 
       *        destructor.
       */
      SoundFileListMess(char** data, int32 numEntries);
       

      /** Destructor. */
      virtual ~SoundFileListMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return     Address of the data.
       */
       char**   getStringData() const;

       int32 getNumEntries() const;
 
   protected:
       void initData();
      /**
       * Member variables.
       */
      //@{
          char** m_stringData;
          int32 m_numEntries;
      //@}

}; // SoundFileListMess


class SendMessageMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      SendMessageMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      SendMessageMess(enum GuiProtEnums::UserMessageType messT, 
                      enum GuiProtEnums::ObjectType objT, char* id, char* from, 
                      char* to, char* sig);
       

      /** Destructor. */
      virtual ~SendMessageMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   GuiProtEnums::UserMessageType getMedia() const ;
   GuiProtEnums::ObjectType getObject() const;
   char* getId() const;
   char* getFrom() const;
   char* getTo() const;
   char* getSignature() const;
 
   protected:
      /**
       * Member variables.
       */
      //@{
   GuiProtEnums::UserMessageType m_media;
   GuiProtEnums::ObjectType m_object;
   char* m_id;
   char* m_from;
   char* m_to;
   char* m_sig;
      //@}

}; // SendMessageMess

class MessageSentMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      MessageSentMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       */
      MessageSentMess(uint8* data, unsigned len);


      /** Destructor. */
      virtual ~MessageSentMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       */
   unsigned size() const;
   uint8* getData(unsigned*size=NULL) const;
   
 
   protected:
      /**
       * Member variables.
       */
      //@{
   uint8* m_data;
   unsigned m_len;
      //@}

}; // MessageSentMess


class LicenseKeyMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      LicenseKeyMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      LicenseKeyMess(const char* key, const char* phone, uint32 region);

      LicenseKeyMess(const char* key, const char* phone, 
                     const char* name, const char* email, 
                     const char* optional,
                     uint32 region);


      /** Destructor. */
      virtual ~LicenseKeyMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       */
   const char* getKey() const;
   const char* getPhone() const;
   const char* getName() const;
   const char* getEmail() const;
   const char* getOptional() const;
   uint32 getRegion() const;
 
   protected:
      /**
       * Member variables.
       */
      //@{
   char* m_key;
   char* m_phone;
   char* m_name;
   char* m_email;
   char* m_optional;
   uint32 m_region;
      //@}

}; // LicenseKeyMess

/**
  *   LicenseReplyMess description.
  *
  */
class LicenseReplyMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      LicenseReplyMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      LicenseReplyMess(bool keyOk, bool phoneOk, bool regionOk, 
                       bool nameOk, bool emailOk, 
                       GuiProtEnums::WayfinderType type);

      /** Destructor. */
      virtual ~LicenseReplyMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       */
   bool isKeyOk() const;
   bool isPhoneOk() const;
   bool isRegionOk() const;
   bool isNameOk() const;
   bool isEmailOk() const;
   bool allOk() const;
   GuiProtEnums::WayfinderType getWayfinderType() const;
 
   protected:
      /**
       * Member variables.
       */
      //@{
         
   bool m_keyOk;
   bool m_phoneOk;
   bool m_regionOk;
   bool m_nameOk;
   bool m_emailOk;
   GuiProtEnums::WayfinderType m_type;
      //@}

}; // LicenseReplyMess


/**
  *   DataGuiMess is a representation of any message that consists of a 
  *   32 length field followed by binary data.
  *   The message content type is always set to type_and_length_and_data.
  *
  */
class DataGuiMess : public GuiProtMess
{
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      DataGuiMess(class Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       * @param messType The message type. 
       * @param size the size of the data field.
       * @param data the binary data block to include in the message. 
       *             The data is copied into the object.
       *             The data is not deleted in the message 
       *             destructor, use deleteMembers instead.
       * @param ad_size the size of the additional data field.
       * @param ad_data the additional binary data block. 
       *
       */
      DataGuiMess(GuiProtEnums::MessageType messType, 
                  uint32 size, const uint8* data,
                  uint32 ad_size = 0, const uint8* ad_data = NULL,
                  uint8 requestVer = 1 );

      /** Destructor. */
      virtual ~DataGuiMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(class Buffer* buf) const;
      //@}
   /** Returns the size of the data block, size field is not included 
    * in this size.
    * @return the data block size.
    */
   uint32 getSize() const;

   /** Returns the size of the additional data block, 
    * @return the additional data block size.
    */
   uint32 getAdditionalSize() const;

   /**
    * Returns the data of message. This data is not deleted by the
    * message destructor, use deleteMembers if you want that to
    * happen.
    *
    * @param length pointer to a length variable. If the pointer is
    *               non-null the size of the data block will be written 
    *               through the pointer.
    * @return a pointer to the start of the data block.
    */
   const uint8* getData(uint32* length = NULL) const;

   const uint8* getAdditionalData(uint32* length = NULL) const;

   uint8 getRequestVersion() const;
 
   protected:
      /**
       * Protected member variables.
       */
      //@{
   /** Holds the size of the data block.*/
   uint32 m_size;
   /** Pointer to the data block.*/ 
   uint8* m_data;
   /** Holds the size of the additional data block.*/
   uint32 m_ad_size;
   /** Pointer to the additional data block.*/ 
   uint8* m_ad_data;
   /** The protocol version of the request.*/
   uint8 m_requestVer;
      //@}

}; // DataGuiMess

} // namespace isab


#endif // GUI_PROT_MESSAGES_H
