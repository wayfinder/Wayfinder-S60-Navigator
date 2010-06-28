/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* The public interface to the Parameter module. This module
 * stores permanent parameters that needs to be kept over a 
 * system restart. */


#ifndef MODULE_Parameter_H
#define MODULE_Parameter_H

namespace isab {

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * ParameterConsumerInterface. */
class ParameterConsumerPublic : public ModulePublic {
   public:
      ParameterConsumerPublic(Module *m) : 
         ModulePublic(m) { };

      virtual uint32 paramValue(uint32 paramId,
                                const int32 * data,
                                int32 numEntries,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 paramValue(uint32 paramId,
                                const float * data,
                                int32 numEntries,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 paramValue(uint32 paramId,
                                int length, 
                                const char * data,
                                int32 numEntries,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      /** Binary block */
      virtual uint32 paramValue(uint32 paramId, const uint8* data, int size,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 paramNoValue(uint32 paramId,
                                uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
};

/** A module to convert function calls into messages that are 
 * sent to another module. That other module must implement
 * ParameterProviderInterface. */
class ParameterProviderPublic : public ModulePublic {
   public:
      ParameterProviderPublic(Module *m) : 
         ModulePublic(m) { 
            setDefaultDestination(Module::addrFromId(Module::ParameterModuleId));
         };
private:
      virtual uint32 clearParam(uint32 paramId,
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 setParam(uint32 paramId, const int32 * data,
                              int32 numEntries = 1, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 setParam(uint32 paramId, const float * data,
                              int32 numEntries = 1, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 setParam(uint32 paramId, const char * const * data,
                              int32 numEntries = 1, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      /** Binary block */
      virtual uint32 setParam(uint32 paramId, const uint8* data, int size, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      virtual uint32 getParam(uint32 paramId, 
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
public:

   uint32 clearParam(enum ParameterEnums::ParamIds paramId, 
                     uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return clearParam(uint32(paramId), dst);
   }

   uint32 setParam(enum ParameterEnums::ParamIds paramId, const int32* data, 
                   int32 numEntries = 1, uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return setParam(uint32(paramId), data, numEntries, dst);
   }

   uint32 setParam(enum ParameterEnums::ParamIds paramId, const float * data, 
                   int32 numEntries = 1, uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return setParam(uint32(paramId), data, numEntries, dst);
   }
   
   uint32 setParam(enum ParameterEnums::ParamIds paramId, const char * const * data, 
                   int32 numEntries = 1, uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return setParam(uint32(paramId), data, numEntries, dst);
   }
   
   uint32 setParam(enum ParameterEnums::ParamIds paramId, const uint8* data, int size, 
                   uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return setParam(uint32(paramId), data, size, dst);
   }
   
   uint32 getParam(enum ParameterEnums::ParamIds paramId, uint32 dst = MsgBufferEnums::ADDR_DEFAULT)
   {
      return getParam(uint32(paramId), dst);
   }


      static uint16 paramIdToMulticastGroup(enum ParameterEnums::ParamIds paramId)
      {
         return paramIdToMulticastGroup(uint32(paramId)); 
      }

      static bool paramIdIsMulticast(enum ParameterEnums::ParamIds paramId)
      {
         return paramIdIsMulticast(uint32(paramId));
      }



      virtual uint32 rewriteOrigFile(uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

private:
      static uint16 paramIdToMulticastGroup(uint32 paramId) 
      {
         return uint16((paramId & 0x7fff0000) >> 16);
      }

      static bool paramIdIsMulticast(uint32 paramId)
      {
         return ((paramId & 0x80000000) != 0);
      }
};




/* Next are the interfaces. These should be implemented by the 
 * module that wishes to receive the corresponding messages. 
 * The classes below are pure virtual and correspond in function
 * to "interfaces" in java. */

/** An interface that must be implemented to receive messages in a
 * NavTask provider-consumer relationship. This is implemented 
 * in the _upper_ module.
 */
class ParameterConsumerInterface {
   public:
      virtual void decodedParamValue(uint32 paramId,
                                const int32 * data,
                                int32 numEntries,
                                uint32 src,
                                uint32 dst) = 0;
      virtual void decodedParamValue(uint32 paramId,
                                const float * data,
                                int32 numEntries,
                                uint32 src,
                                uint32 dst) = 0;
      virtual void decodedParamValue(uint32 paramId,
                                const char * const * data,
                                int32 numEntries,
                                uint32 src,
                                uint32 dst) = 0;
      /** Delivers a binary block parameter.*/
      virtual void decodedParamValue(uint32 paramId, 
                                const uint8* data, 
                                int size,
                                uint32 src,
                                uint32 dst) = 0;
      virtual void decodedParamNoValue(uint32 paramId,
                                uint32 src,
                                uint32 dst) = 0;
};

/** An interface that must be implemented to receive messages in a
 * NavTask provider-consumer relationship. This is implemented 
 * in the _lower_ module.
 */
class ParameterProviderInterface {
   public:
      virtual void decodedClearParam(uint32 paramId, 
                              uint32 src) = 0;
      virtual void decodedSetParam(uint32 paramId, 
                              const int32 * data,
                              int32 numEntries, 
                              uint32 src) = 0;
      virtual void decodedSetParam(uint32 paramId, 
                              const float * data,
                              int32 numEntries, 
                              uint32 src) = 0;
      virtual void decodedSetParam(uint32 paramId, 
                              int length, 
                              const char * data,
                              int32 numEntries, 
                              uint32 src) = 0;
      /** Sets a binary block parameter.*/
      virtual void decodedSetParam(uint32 paramId,
                              const uint8* data,
                              int size,
                              uint32 src) = 0;
                                
      virtual void decodedGetParam(uint32 paramId, 
                              uint32 src) = 0;

      virtual void decodedRewriteOrigFile(uint32 src) = 0;
};

/** An object of this class is owned by each module that implements 
 * the NavTaskConsumerInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in NavTaskConsumerInterface.
 */
class ParameterConsumerDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, ParameterConsumerInterface *m );
};

/** An object of this class is owned by each module that implements 
 * the NavTaskProviderInterface. This object contains a dispatch()
 * function that does the actual decoding of an incoming buffer.
 * It then call the apropriate methods in NavTaskProviderInterface.
 *
 * @return the buffer if further processing may be done or NULL if 
 *         the buffer was processed and either passed on or 
 *         destroyed and thus no further processing should be done.
 */
class ParameterProviderDecoder {
   public:
      class MsgBuffer * dispatch(class MsgBuffer *buf, ParameterProviderInterface *m );
};

} /* namespace isab */

#endif /* MODULE_Parameter_H */


