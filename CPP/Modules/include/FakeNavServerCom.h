/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef FAKENAVSERVERCOM_H
#define FAKENAVSERVERCOM_H
namespace isab{

   using namespace NavServerComEnums;
   using namespace Nav2Error;
   /** This class handles all server communication. It needs a lower
       serial layer that handles the transfer of serial data to and
       from the server. */
   class FakeNavServerCom : public Module,
                            public CtrlHubAttachedInterface,
                            public ParameterConsumerInterface,
                            public NavServerComReplyInterface
   {
   public:
      /** The constructor. 
       * @param spp pointer to the SerialProviderPublic object providing
       *            serial data from the NavigatorServer.  
       * @param server the default server.
       * @param port the default server port.
       */
      FakeNavServerCom();

      /** Destructor.*/
      virtual ~FakeNavServerCom();

      /** Allocates a new CtrlHubAttachedPublic object connected to the
       * queue of this object.
       * @return a pointer to a new CtrlHubAttachedPublic object.
       */
      CtrlHubAttachedPublic * newPublicCtrlHub();   

      /** Sends a message to NavTask to tell it a new route is incoming.
       * @param req_id      the server id for this request.
       * @param packlen     the size of the route, whithout header.
       * @param FIXMEheader the route header. This is supposed to be removed 
       *                    once we change the format of the route sent to 
       *                    the NavClient.
       */
      void newRoute(uint8 req_id, int packlen, uint8* FIXMEheader);

      /** Send a chunk of the route to NavTask.
       * @param req_id the server id of the request resulting in this route.
       * @param chunk  a Buffer holding a chunk of route data
       * @param last   a boolean indicating if this chunk is the last one 
       *               received from the server. Default value: false.
       */
      void sendChunk(uint8 req_id, Buffer* chunk, bool last = false);


      virtual void decodedStartupComplete();
      virtual void decodedExpiredTimer(uint16 timerID);

      virtual void decodedParamNoValue(uint32 param, 
                                       uint32 src, uint32 dst);
      virtual void decodedParamValue(uint32 /*param*/, const int32* /*data*/, 
                                     int32 /*numEntries*/, uint32 /*src*/, 
                                     uint32 /*dst*/) {}
      virtual void decodedParamValue(uint32 /*param*/, const float* /*data*/, 
                                     int32 /*numEntries*/, uint32 /*src*/, 
                                     uint32 /*dst*/) {}
      virtual void decodedParamValue(uint32 /*paramId*/, const uint8* /*data*/, 
                                     int /*size*/, uint32 /*src*/, uint32 /*dst*/) {}
      virtual void decodedParamValue(uint32 /*param*/, const char*const* /*data*/, 
                                     int32 /*numEntries*/, uint32 /*src*/, 
                                     uint32 /*dst*/) {}
      virtual uint16 setTimer(int32 timeout, 
                              class NavServerComTimerCallBack* callback);
      
      virtual void getCoords(int32& lat, int32&lon) const;
   public: //The functions from NavServerComReplyInterface
      virtual 
      class NavServerComConsumerPublic& NavServerComConsumerPublicRef()
      {
         return *m_nscConsumer;
      }
      virtual class NavTaskProviderPublic& NavTaskProviderPublicRef()
      {
         return *m_navTaskProvider;
      }
      virtual class ParameterProviderPublic& ParameterProviderPublicRef()
      {
         return *m_paramProvider;
      }
   protected:
      


      /** The message sorting function. 
       * @param buf the MsgBuffer containing the message.  */
      virtual MsgBuffer* dispatch(MsgBuffer* buf);

      /** Sends an error message to a requester.
       * @param request the data from the sent-list.
       */
      void sendError( const ErrorObj& err, uint32 src = 0 ) const;
   private:

      /** Interface to the NavTask module. Usefull for sending
          downloaded routes and whatnot. */
      class NavTaskProviderPublic* m_navTaskProvider;

      /** Interface to all our consumers. Used to answer any requests. */
      class NavServerComConsumerPublic* m_nscConsumer;

      /** Decoder for all messages concerned with infrastructure matters. */
      class CtrlHubAttachedDecoder m_ctrlHubAttachedDecoder;

      /** Decoder for messages directed to this Module.  */
      class NavServerComProviderDecoder m_comDecoder;

      /** Interface to the Parameter Module. */
      class ParameterProviderPublic* m_paramProvider;

      class ParameterConsumerDecoder m_paramDecoder;

      class Faker* m_faker;
      ///Map of timers handled by this class for any classes using the
      ///NavServerComReplyInterface.
      std::map<uint16, class NavServerComTimerCallBack*> m_externalTimers;

   };
   //===============================================================
   //====== inlines for FakeNavServerCom ===============================


   inline void FakeNavServerCom::sendChunk(uint8 /*req_id*/, Buffer* chunk, 
                                           bool /*last*/)
   {
      m_navTaskProvider->routeChunk(false, chunk->getLength(), 
                                    chunk->accessRawData(0));
   }

   inline CtrlHubAttachedPublic * FakeNavServerCom::newPublicCtrlHub()
   {
      return new CtrlHubAttachedPublic(m_queue);
   }

   inline void FakeNavServerCom::getCoords(int32& /*lat*/, int32&/*lon*/) const
   {
      //we don't know what the coords are.
   }

}


#endif
