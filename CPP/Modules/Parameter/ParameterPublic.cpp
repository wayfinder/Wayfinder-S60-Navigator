/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Buffer.h"
#include "MsgBuffer.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "ParameterEnums.h"
#include "Parameter.h"

namespace isab {

   MsgBuffer * ParameterProviderDecoder::dispatch(MsgBuffer *buf, 
         ParameterProviderInterface *m )
   {
      int length;

      length=buf->getLength();
      switch (buf->getMsgType()) {
         case MsgBufferEnums::PARAM_CLEAR:
            {
               uint32 paramId=buf->readNext32bit();
               m->decodedClearParam(paramId, buf->getSource());
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_SET_INT32:
            {
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               int32 *data = new int32[numEntries];
               int32 *tmpptr = data;
               int i;
               for (i=numEntries; i>0 ; --i) {
                  *tmpptr++ = buf->readNext32bit();
               }
               m->decodedSetParam(paramId, data, numEntries, buf->getSource());
               delete[] data;
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_SET_FLOAT:
            {
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               float *data = new float[numEntries];
               float *tmpptr = data;
               int i;
               for (i=numEntries; i>0 ; --i) {
                  *tmpptr++ = buf->readNextFloat();
               }
               m->decodedSetParam(paramId, data, numEntries, buf->getSource());
               delete[] data;
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_SET_STRING:
            {
               /* Read out the data and store both the strings themselvs
                * and an array of pointers to the strings */
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               int readPos = buf->getReadPos();
               int remainingChars = buf->getLength() - readPos;
               m->decodedSetParam(paramId, remainingChars, 
                     reinterpret_cast<const char *>(buf->accessRawData(readPos)), 
                     numEntries, buf->getSource());
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_SET_BINARY_BLOCK:
            {
               uint32 paramId = buf->readNext32bit();
               int size = buf->readNext32bit();
               const uint8* data = buf->accessRawData();
               m->decodedSetParam(paramId, data, size, buf->getSource());
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_GET:
            {
               uint32 paramId=buf->readNext32bit();
               m->decodedGetParam(paramId, buf->getSource());
               delete(buf);
               return NULL;
            }
         case MsgBufferEnums::PARAM_REWRITE_ORIG_FILE:
            {
               m->decodedRewriteOrigFile(buf->getSource());
               delete(buf);
               return NULL;
            }
         default:
            return buf;
      }

   }

   MsgBuffer * ParameterConsumerDecoder::dispatch(MsgBuffer *buf, ParameterConsumerInterface *m )
   {
      int length;

      length=buf->getLength();
      switch (buf->getMsgType()) {
         case MsgBufferEnums::PARAM_VAL_NONE:
            {
               uint32 paramId=buf->readNext32bit();
               m->decodedParamNoValue(paramId, 
                     buf->getSource(), buf->getDestination());
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_VAL_INT32:
            {
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               int32 *data = new int32[numEntries];
               int32 *tmpptr = data;
               int i;
               for (i=numEntries; i>0 ; --i) {
                  *tmpptr++ = buf->readNext32bit();
               }
               m->decodedParamValue(paramId, data, numEntries, 
                     buf->getSource(), buf->getDestination());
               delete[] data;
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_VAL_FLOAT:
            {
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               float *data = new float[numEntries];
               float *tmpptr = data;
               int i;
               for (i=numEntries; i>0 ; --i) {
                  *tmpptr++ = buf->readNextFloat();
               }
               m->decodedParamValue(paramId, data, numEntries, 
                     buf->getSource(), buf->getDestination());
               delete[] data;
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_VAL_STRING:
            {
               /* Read out the data and stor both the strings themselvs
                * and an array of pointers to the strings */
               uint32 paramId=buf->readNext32bit();
               int32  numEntries=buf->readNext32bit();
               const char **indexarray = new const char *[numEntries];
               const char **indexptr = indexarray;
               const char *rawdata = reinterpret_cast<const char *>(buf->accessRawData(buf->getReadPos()));
               int i;
               for (i=numEntries; i>0 ; --i) {
                  *indexptr++ = rawdata;
                  rawdata += strlen(rawdata) + 1;
               }
               m->decodedParamValue(paramId, indexarray, numEntries, 
                     buf->getSource(), buf->getDestination());
               delete[] indexarray;
               delete buf;
               return NULL;
            }
         case MsgBufferEnums::PARAM_VAL_BINARY_BLOCK:
            {
               uint32 paramId = buf->readNext32bit();
               int size = buf->readNext32bit();
               const uint8* data = buf->accessRawData();
               m->decodedParamValue(paramId, data, size, buf->getSource(), 
                     buf->getDestination());
               delete buf;
               return NULL;
            }
         default:
            return buf;
      }

   }

   uint32 ParameterConsumerPublic::paramNoValue(uint32 paramId,
         uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_VAL_NONE, 4 );
      buf->writeNext32bit(paramId);
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterConsumerPublic::paramValue(uint32 paramId,
         const int32 * data, int32 numEntries, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_VAL_INT32, 8 + numEntries*4 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      while (numEntries) {
         buf->writeNext32bit(*data++);
         --numEntries;
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterConsumerPublic::paramValue(uint32 paramId,
         const float * data, int32 numEntries, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_VAL_FLOAT, 8 + numEntries*4 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      while (numEntries) {
         buf->writeNextFloat(*data++);
         --numEntries;
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterConsumerPublic::paramValue(uint32 paramId,
         int length, const char * data, int32 numEntries, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_VAL_STRING, 8 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      buf->writeNextByteArray(reinterpret_cast<const uint8 *>(data), length);
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterConsumerPublic::paramValue(uint32 paramId, 
                                              const uint8* data, int size,
                                              uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::PARAM_VAL_BINARY_BLOCK, 
                                     size + 8);
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(size);
      buf->writeNextByteArray(data, size);
      m_queue->insert(buf);
      return src;
   }


   uint32 ParameterProviderPublic::clearParam(uint32 paramId, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_CLEAR, 4 );
      buf->writeNext32bit(paramId);
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::setParam(uint32 paramId, const int32 * data,
         int32 numEntries, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_SET_INT32, 8 + numEntries*4 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      while (numEntries) {
         buf->writeNext32bit(*data++);
         --numEntries;
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::setParam(uint32 paramId, const float * data,
         int32 numEntries, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_SET_FLOAT, 8 + numEntries*4 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      while (numEntries) {
         buf->writeNextFloat(*data++);
         --numEntries;
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::setParam(uint32 paramId, const char * const * data,
         int32 numEntries, uint32 dst) 
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_SET_STRING, 8 );
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(numEntries);
      while (numEntries) {
         buf->writeNextCharString(*data++);
         --numEntries;
      }
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::setParam(uint32 paramId, const uint8* data, 
                                            int size, uint32 dst)
   { 
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf = new MsgBuffer(dst, src, 
                                     MsgBufferEnums::PARAM_SET_BINARY_BLOCK, 
                                     size + 8);
      buf->writeNext32bit(paramId);
      buf->writeNext32bit(size);
      buf->writeNextByteArray(data, size);
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::getParam(uint32 paramId, uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_GET,4);
      buf->writeNext32bit(paramId);
      m_queue->insert(buf);
      return src;
   }

   uint32 ParameterProviderPublic::rewriteOrigFile(uint32 dst)
   {
      if (dst == MsgBufferEnums::ADDR_DEFAULT) {
         dst = m_defaultDst;
      }
      uint32 src = m_owner->getRequestId();
      MsgBuffer *buf=new MsgBuffer(dst, src, MsgBufferEnums::PARAM_REWRITE_ORIG_FILE,0);
      m_queue->insert(buf);
      return src;
   }


} /* namespace isab */
