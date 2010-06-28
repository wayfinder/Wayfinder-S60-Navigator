/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* This module stores permanent parameters that needs to be 
 * kept over a system restart.
 */



#ifndef MODULE_ParameterInternal_H
#define MODULE_ParameterInternal_H

#include <map>
#include "Module.h"
#include "Buffer.h"
#include "CtrlHub.h"
#include "Parameter.h"

class TDes8;

namespace isab {

#ifdef __SYMBIAN32__
   typedef ::TDes8 & filestate;
#else
   typedef FILE * filestate;
#endif

   class Parameter : public Module, 
                     public CtrlHubAttachedInterface, 
                     public ParameterProviderInterface {
   public:
      Parameter(bool release);

      /** Creates a new CtrlHubAttachedPublic object used to connect this
       * module to the CtrlHub.
       * @return a new CtrlHubAttachedPublic object connected to the queue.
       */
      CtrlHubAttachedPublic * newPublicCtrlHub();

      virtual void decodedStartupComplete();
      virtual void decodedClearParam(uint32 paramId,
            uint32 src);
      virtual void decodedSetParam(uint32 paramId,
            const int32 * data,
            int32 numEntries,
            uint32 src);
      virtual void decodedSetParam(uint32 paramId,
            const float * data,
            int32 numEntries,
            uint32 src);
      virtual void decodedSetParam(uint32 paramId,
            int length, 
            const char * data,
            int32 numEntries,
            uint32 src);
      virtual void decodedSetParam(uint32 paramId, const uint8* data,
                                   int size, uint32 src);
      virtual void decodedGetParam(uint32 paramId,
            uint32 src);

      virtual void decodedRewriteOrigFile(uint32 src);

      virtual void decodedExpiredTimer(uint16 timerid);

      
   private:
      class Param {
         public:
            uint32 m_paramId;
            Param(uint32 paramId) : m_paramId(paramId) {}
            void hacked_fprintf(filestate savefile, const char *format, ...)
#if defined(__GNUC__)
                     __attribute__ ((format (printf, 3, 4)));
#endif
                     ;
            void hacked_fwrite(filestate savefile, const void *ptr, size_t size, size_t nmemb);
            virtual ~Param();
            virtual void sendParam(uint32 dest, ParameterConsumerPublic *encoder) = 0;
            virtual void fileOut(filestate savefile) = 0;
            virtual unsigned toWrite() = 0;
            virtual bool hasValue(const char* /*val*/) { return false; }
      };

      class ParamInt32 : public Param {
         public:
            ParamInt32(uint32 paramId, const int32 * data, int numEntries);
            virtual ~ParamInt32();
            virtual void sendParam(uint32 dest, ParameterConsumerPublic *encoder);
            virtual void fileOut(filestate savefile);
            virtual unsigned toWrite();
         private:
            int32 * m_data;
            int m_numEntries;
      };

      class ParamFloat : public Param {
         public:
            ParamFloat(uint32 paramId, const float * data, int numEntries);
            virtual ~ParamFloat();
            virtual void sendParam(uint32 dest, ParameterConsumerPublic *encoder);
            virtual void fileOut(filestate savefile);
            virtual unsigned toWrite();
         private:
            float * m_data;
            int m_numEntries;
      };

      class ParamString : public Param {
         public:
            ParamString(uint32 paramId, int length, const char * data, int numEntries);
            virtual ~ParamString();
            virtual void sendParam(uint32 dest, ParameterConsumerPublic *encoder);
            virtual void fileOut(filestate savefile);
            virtual unsigned toWrite();
            virtual bool hasValue(const char *val);
         private:
            char * m_data;
            int m_dataLength;
            int m_numEntries;
      };

      class ParamBinaryBlock : public Param {
      public:
         ParamBinaryBlock(uint32 paramId, const uint8* data, int size);
         virtual ~ParamBinaryBlock();
         virtual void sendParam(uint32 dst, ParameterConsumerPublic * encoder);
         virtual void fileOut(filestate saveFile);
         virtual unsigned toWrite();
      private:
         int m_size;
         uint8* m_data;
      };

      virtual MsgBuffer * dispatch(MsgBuffer *buf);

      int calculateFileSize();
      void writeParameters(filestate);

      /** Decoder for CtrlHub-messages */
      CtrlHubAttachedDecoder m_ctrlHubAttachedDecoder;
      /** Decoder for Parameter-messages */
      ParameterProviderDecoder m_ParameterProviderDecoder;

      /** Encoder for Parameter-messages */
      ParameterConsumerPublic * m_ParameterConsumer;

      std::map <uint32, Param *, std::less<uint32> > *m_parameters;

      /** Id of timeout used in coalescing commits to persistant storage */
      uint16 m_saveTimeoutId;

      /** Set to true if unsaved changes to persistent parameters exist in the 
       * in-memory store */
      bool m_dirty;

      /* Set to true if the parameter file was valid and it is resonable to
       * overwrite it. Set to false if the parameter file was unreadable. An 
       * unreadable file usually means the system has to be reinstalled. */
      bool m_okToRewrite;

      /* Set to true if the module is loaded on a release system. 
       * (Affects debug outputs etc). */
      bool m_release;

      /** Load the parameters from non-volatile store.
       *
       * Return true on success and false on failure.
       * Sends error to GUI if complain is nonzero.
       *
       * This is a kludge for now.
       */
      bool loadParametersFromFile(const char *filename, int complain=1);

      void readOverrideFile();
      void loadParameters();

      void saveParameters(bool rewriteOriginal);

      void eraseAllParameters();

      void eraseParameter(uint32 paramId);

      void maybeNotify(uint32 paramId, Param *p);
      void maybeNotifyEmpty(uint32 paramId);

      void maybeDelayedPersistantWrite(uint32 paramId);

      virtual void treeIsShutdown();

   protected:
      void hackRemoveCmmcUserFile();

   };

} /* namespace isab */

#endif /* MODULE_ParameterInternal_H */
