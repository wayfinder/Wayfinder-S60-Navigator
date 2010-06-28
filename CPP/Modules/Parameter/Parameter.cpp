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
#include <stdio.h>
#include <ctype.h>
#ifdef __SYMBIAN32__
# include <sys/unistd.h>
# if !(defined(NAV2_CLIENT_UIQ) || defined(NAV2_CLIENT_UIQ3))
#  include <sysutil.h>
# endif
# include <f32file.h>
#elif defined UNDER_CE
# include <winbase.h>
#else
# include <unistd.h>
#endif

#include "MsgBufferEnums.h"
#include "Module.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "ParameterInternal.h"
#include "GlobalData.h"

#include "Buffer.h"
#include "MsgBuffer.h"

#include "LogMacros.h"
#if O
# define DBG    m_log->debug
# define INFO   m_log->info
# define WARN   m_log->warning
# define ERR    m_log->error
#endif

/** Max delay in writing persistant parameters to stable store. 
 * Used to coalesc multiple changed parameters into one write. 
 * Expressed in ms.           */
#define PERSISTANT_PARAMETER_MAX_WRITE_DELAY 10000
#define WAIT_AND_SLEEP_TIME 50


#if defined(_MSC_VER) && !defined(__SYMBIAN32__)
  /** Location of the parameter file */
# define PARAMFILE_LOCATION "paramfile.txt"
  /** Location of the temporary parameter file used while rewriting */
# define PARAMFILE_LOCATION_TEMP "paramnew.txt"
  /** Location of the last (old) parameter file created while rewriting */
# define PARAMFILE_LOCATION_OLD "paramold.txt"
  /** Location of the original parameter file from the server */
# define PARAMFILE_LOCATION_ORIGINAL "paramorig.txt"
  /** Location of the seed parameter file (used by new users only) */
# define PARAMFILE_LOCATION_SEED "paramseed.txt"
  /** Location of parameter override file. */
# define OVERRIDEFILE_LOCATION "param_override.txt"

#elif defined(__SYMBIAN32__)

  /** Location of the parameter file */
# define PARAMFILE_LOCATION "params\\paramfile.txt"
  /** Location of the temporary parameter file used while rewriting */
# define PARAMFILE_LOCATION_TEMP "params\\paramnew.txt"
  /** Location of the last (old) parameter file created while rewriting */
# define PARAMFILE_LOCATION_OLD "params\\paramold.txt"
  /** Location of the original parameter file from the server */
# define PARAMFILE_LOCATION_ORIGINAL "params\\paramorig.txt"
  /** Location of the seed parameter file (used by new users only) */
# define PARAMFILE_LOCATION_SEED "params\\paramseed.txt"
  /** Location of parameter override file. */
# define OVERRIDEFILE_LOCATION "params\\param_override.txt"

#else

  /** Location of the parameter file */
# define PARAMFILE_LOCATION "paramfile.txt"
  /** Location of the temporary parameter file used while rewriting */
# define PARAMFILE_LOCATION_TEMP "paramnew.txt"
  /** Location of the last (old) parameter file created while rewriting */
# define PARAMFILE_LOCATION_OLD "paramold.txt"
  /** Location of the original parameter file from the server */
# define PARAMFILE_LOCATION_ORIGINAL "paramorig.txt"
  /** Location of the original parameter file from the server */
# define PARAMFILE_LOCATION_SEED "paramseed.txt"
  /** Location of parameter override file. */
# define OVERRIDEFILE_LOCATION "param_override.txt"
#endif /* _MSC_VER */

# if defined(UNDER_CE)
#  define BINARY_READ "rb"
#  define BINARY_WRITE "wb"
# else
#  define BINARY_READ "r"
#  define BINARY_WRITE "w"
# endif

using namespace std;

namespace isab {

#ifndef __SYMBIAN32__
   using namespace std;
#endif

   Parameter::Param::~Param()
   {
   }

   void Parameter::Param::hacked_fprintf(filestate savefile, const char *format, ...)
   {
      va_list args;
      va_start(args, format);
#ifdef __SYMBIAN32__
      char data[80];
      int len=vsprintf(data, format, args);
      TPtrC8 ptr((TUint8*)(data), len);
      savefile.Append(ptr);
#else
      vfprintf(savefile, format, args);
#endif

      va_end(args);
   }

   void Parameter::Param::hacked_fwrite(filestate savefile, const void *ptr, size_t size, size_t nmemb)
   {
#ifdef __SYMBIAN32__
      size_t i;
      const TUint8 *tmpptr = (const TUint8*)ptr;
      for (i=0; i<nmemb; ++i) {
         TPtrC8 symbptr;
         symbptr.Set(tmpptr + (i * size), size);
         savefile.Append(symbptr);
      }
#else
         fwrite(ptr, size, nmemb, savefile);
#endif
   }


   Parameter::ParamInt32::ParamInt32(uint32 paramId, const int32 * data, int numEntries)
      : Parameter::Param(paramId), m_numEntries(numEntries)
   {
      int i;
      m_data = new int32[numEntries];
      for (i=0; i<numEntries; i++) {
         m_data[i] = data[i];
      }
   }

   Parameter::ParamInt32::~ParamInt32()
   {
      delete[] m_data;
   }

   void Parameter::ParamInt32::fileOut(filestate savefile)
   {
      int i;
      hacked_fprintf(savefile,"I%d:%08"PRIx32"\n", m_numEntries, m_paramId);
      for (i=0; i<m_numEntries; i++) {
         hacked_fprintf(savefile, "+%"PRIi32"\n", m_data[i]);
      }
   }

   void Parameter::ParamInt32::sendParam(uint32 dest, ParameterConsumerPublic * encoder)
   {
      encoder->paramValue(m_paramId, m_data, m_numEntries, dest);
   }

   unsigned Parameter::ParamInt32::toWrite()
   {
      //21 bytes for the tag and then 13 bytes per entry.
      return 21 + (m_numEntries * 13);
   }

   Parameter::ParamFloat::ParamFloat(uint32 paramId, const float * data, int numEntries)
      : Parameter::Param(paramId), m_numEntries(numEntries)
   {
      int i;
      m_data = new float[numEntries];
      for (i=0; i<numEntries; i++) {
         m_data[i] = data[i];
      }
   }

   Parameter::ParamFloat::~ParamFloat()
   {
      delete[] m_data;
   }

   void Parameter::ParamFloat::fileOut(filestate savefile)
   {
      int i;

      hacked_fprintf(savefile,"F%d:%08"PRIx32"\n", m_numEntries, m_paramId);
      for (i=0; i<m_numEntries; i++) {
         hacked_fprintf(savefile, "+%.12g\n", m_data[i]);
      }
   }

   void Parameter::ParamFloat::sendParam(uint32 dest, ParameterConsumerPublic * encoder)
   {
      encoder->paramValue(m_paramId, m_data, m_numEntries, dest);
   }

   unsigned Parameter::ParamFloat::toWrite()
   {
      //21 bytes tag, 18 bytes per float.
      return 21 + (m_numEntries * 18);
   }

   Parameter::ParamString::ParamString(uint32 paramId, int length, const char * data, int numEntries)
      : Parameter::Param(paramId), m_dataLength(length), m_numEntries(numEntries)
   {
      m_data = new char[length];
      memcpy(m_data, data, length);
   }

   Parameter::ParamString::~ParamString()
   {
      delete[] m_data;
   }

   void Parameter::ParamString::fileOut(filestate savefile)
   {
      char *paramPtr;

      hacked_fprintf(savefile,"S%d:%08"PRIx32"\n+%i:", m_numEntries, m_paramId, 
              m_dataLength);
      paramPtr=m_data;
      hacked_fwrite(savefile, m_data, m_dataLength, 1);
      hacked_fprintf(savefile,"\n");
//      for (i=0; i<m_numEntries; i++) {
//         fprintf(savefile, "+%s%n\n", paramPtr, &length);
//         paramPtr += length;   /* length is strlen(paramPtr) + 1 which is
//                                  right for skipping terminating zero */
//      }
   }

   void Parameter::ParamString::sendParam(uint32 dest, ParameterConsumerPublic * encoder)
   {
      encoder->paramValue(m_paramId, m_dataLength, m_data, m_numEntries, dest);
   }

   unsigned Parameter::ParamString::toWrite()
   {
      //21 bytes tag, 13 bytes fluff around the data.
      return 34 + m_dataLength;
   }

   bool Parameter::ParamString::hasValue(const char *val)
   {
      if (m_numEntries != 1)                              return false;
      if ((size_t)m_dataLength != (strlen(val)+1))        return false;
      if (strncmp(m_data, val, m_dataLength))             return false;
      return true;
   }


   Parameter::ParamBinaryBlock::ParamBinaryBlock(uint32 paramId, 
                                                 const uint8* data, int size) :
      Parameter::Param(paramId), m_size(size), m_data(NULL)
   {
      // XXX +4 makes it possible to load 
      // parameter files that otherwise 
      // results in KERN-SVR -6 on symbian-r6.
      // m_data = new uint8[size+4]; Out commented becuse it probably works anyway.
      m_data = new uint8[size];
      memcpy(m_data, data, size);
   }

   Parameter::ParamBinaryBlock::~ParamBinaryBlock()
   {
      delete[] m_data;
   }

   void Parameter::ParamBinaryBlock::sendParam(uint32 dst,
                                               ParameterConsumerPublic *encoder)
   {
      encoder->paramValue(m_paramId, m_data, m_size, dst);
   }

   void Parameter::ParamBinaryBlock::fileOut(filestate savefile)
   {
      hacked_fprintf(savefile, "B1:%08"PRIx32"\n+%i:", m_paramId, m_size);
      hacked_fwrite(savefile, m_data, m_size, 1);
      hacked_fprintf(savefile, "\n");
   }

   unsigned Parameter::ParamBinaryBlock::toWrite()
   {
      //21 bytes tag, 13 bytes fluff around the data.
      return 34 + m_size;
   }

   Parameter::Parameter(bool release)
      : Module("Parameter"), m_ParameterConsumer(NULL), m_dirty(false), m_okToRewrite(false),
        m_release(release)
   {
      DBG("Parameter::Parameter\n");

      m_parameters = new map <uint32, Param *, less<uint32> >;

      m_ParameterConsumer = new ParameterConsumerPublic(this);

      m_saveTimeoutId = m_queue->defineTimer();
   }


   void Parameter::decodedStartupComplete()
   {
      loadParameters();
   }


   void Parameter::treeIsShutdown()
   {
      /* Delete the public interface that is used to send parameter replies,
       * and delee all the parameters. Noone can send queries anymore. */
      delete m_ParameterConsumer;
      m_ParameterConsumer = NULL;

      if (m_dirty) {
         m_queue->cancelTimer(m_saveTimeoutId);
         saveParameters(false);
      }
      eraseAllParameters();

      delete m_parameters;
      m_parameters = NULL;

      Module::treeIsShutdown();
   }

   MsgBuffer * Parameter::dispatch(MsgBuffer *buf)
   {
      DBG("Parameter::dispatch : %i\n", buf->getMsgType());
      buf=m_ParameterProviderDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=m_ctrlHubAttachedDecoder.dispatch(buf, this);
      if (!buf)
         return buf;
      buf=Module::dispatch(buf);
      return buf;
   }


   CtrlHubAttachedPublic * Parameter::newPublicCtrlHub()
   {
      DBG("newPublicCtrlHub()\n");
      return new CtrlHubAttachedPublic(m_queue);

   }

   void Parameter::maybeNotify(uint32 paramId, Param *p)
   {
      if((m_rawRootPublic != NULL) && (paramId & 0x80000000)) {
         p->sendParam( ( (paramId & 0x7fff0000) >> 16 ) | MsgBufferEnums::ADDR_MULTICAST_UP, 
                       m_ParameterConsumer);
      }
   }

   void Parameter::maybeNotifyEmpty(uint32 paramId)
   {
      if((m_rawRootPublic != NULL) && (paramId & 0x80000000)) {
         m_ParameterConsumer->paramNoValue(paramId, 
               ( (paramId & 0x7fff0000) >> 16 ) | MsgBufferEnums::ADDR_MULTICAST_UP) ;
      }
   }

   void Parameter::maybeDelayedPersistantWrite(uint32 paramId)
   {
      if ( (paramId & 0x00008000) && !m_dirty ) {
         m_dirty = true;
         m_queue->setTimer(m_saveTimeoutId, PERSISTANT_PARAMETER_MAX_WRITE_DELAY);
      }
   }

   void Parameter::decodedClearParam(uint32 paramId, uint32 src)
   {
      DBG("decodedClearParam(0x%08"PRIx32", 0x%08"PRIx32")", paramId, src);
      eraseParameter(paramId);
      maybeNotifyEmpty(paramId);
      maybeDelayedPersistantWrite(paramId);
   }

   void Parameter::decodedSetParam(uint32 paramId, const int32 * data,
         int32 numEntries, uint32 src)
   {
      DBG("decodedSetParam(int32)(0x%08"PRIx32", (%"PRId32"), %"PRId32
          ", 0x%08"PRIx32")", paramId, *data, numEntries, src);
      eraseParameter(paramId);
      if (numEntries > 0) {
         ParamInt32 *p = new ParamInt32(paramId, data, numEntries);
         (*m_parameters)[paramId] = p;
         maybeNotify(paramId, p);
      } else {
         maybeNotifyEmpty(paramId);
      }
      maybeDelayedPersistantWrite(paramId);
   }

   void Parameter::decodedSetParam(uint32 paramId, const float * data,
         int32 numEntries, uint32 src)
   {
      DBG("decodedSetParam(float)(0x%08"PRIx32", (%f), %"PRId32
          ", 0x%08"PRIx32")", paramId, *data, numEntries, src);
      eraseParameter(paramId);
      if (numEntries > 0) {
         ParamFloat *p = new ParamFloat(paramId, data, numEntries);
         (*m_parameters)[paramId] = p;
         maybeNotify(paramId, p);
      } else {
         maybeNotifyEmpty(paramId);
      }
      maybeDelayedPersistantWrite(paramId);
   }

   void Parameter::decodedSetParam(uint32 paramId, int dataLength, const char * data,
         int32 numEntries, uint32 src)
   {
      DBG("decodedSetParam(string)(0x%08"PRIx32", %d, (%s), %"PRId32","
          " 0x%08"PRIx32")", paramId, dataLength, data, numEntries, src);
      eraseParameter(paramId);
      if (numEntries > 0) {
         ParamString *p = new ParamString(paramId, dataLength, data, numEntries);
         (*m_parameters)[paramId] = p;
         maybeNotify(paramId, p);
      } else {
         maybeNotifyEmpty(paramId);
      }
      maybeDelayedPersistantWrite(paramId);
   }

   void Parameter::decodedSetParam(uint32 paramId, const uint8* data, int size,
                                   uint32 src)
   {
      DBG("decodedSetParam(binary block)(0x%08"PRIx32", %p, %d,"
          " 0x%08"PRIx32")", paramId, data, size, src);
      eraseParameter(paramId);
      ParamBinaryBlock *p = new ParamBinaryBlock(paramId, data, size);
      (*m_parameters)[paramId] = p;
      maybeDelayedPersistantWrite(paramId);
      maybeNotify(paramId, p);
   }

   void Parameter::decodedGetParam(uint32 paramId, uint32 src)
   {
      DBG("decodedGetParam(0x%08"PRIx32", 0x%08"PRIx32")", paramId, src);
      map <uint32, Param *, less<uint32> >::iterator theParam = m_parameters->find(paramId);
      if(m_rawRootPublic != NULL){
         if (theParam != m_parameters->end()) {
            // Cannot use -> since Visual C++ is broken
            DBG("Sending parameter 0x%"PRIx32, paramId);
            (*theParam).second -> sendParam(src, m_ParameterConsumer);
         } else {
            DBG("Request for nonexistant parameter 0x%"PRIx32, paramId);
            m_ParameterConsumer->paramNoValue(paramId, src);
         }
      }
   }

   void Parameter::decodedExpiredTimer(uint16 timerid)
   {
      if (timerid == m_saveTimeoutId) {
         if (m_dirty) {
            DBG("Commiting persistant parameters to stable storage");
            saveParameters(false);
            m_dirty = false;
         } else {
            WARN("Got expired timer with m_dirty==false - should not happen");
         }
         return;
      }
      Module::decodedExpiredTimer(timerid);
   }

   void Parameter::decodedRewriteOrigFile(uint32 /*src*/)
   {
      saveParameters(true);
   }

   void Parameter::eraseParameter(uint32 paramId)
   {
      map <uint32, Param *, less<uint32> >::iterator theParam = m_parameters->find(paramId);
      if (theParam != m_parameters->end()) {
         delete (*theParam).second;
         m_parameters->erase(theParam);
      }
   }

   void Parameter::hackRemoveCmmcUserFile()
   {
      map <uint32, Param *, less<uint32> >::iterator theParam = m_parameters->find(uint32(ParameterEnums::NSC_ClientType));
      if (theParam != m_parameters->end()) {
         // Cannot use -> since Visual C++ is broken
         if ( (*theParam).second -> hasValue("wf-s-60-v2-akm-nb") ||
              (*theParam).second -> hasValue("wf-s-60-v2-akm-nb-2") ||
              (*theParam).second -> hasValue("wf-s-60-v2-akm-nb-3") ||
              (*theParam).second -> hasValue("wf-s-60-v2-akm-nb-4")
              ) {
            // Old cmmc parameter file detected. Ignoring.
            eraseAllParameters();
            if (loadParametersFromFile(PARAMFILE_LOCATION_SEED)) {
               // Worked ok - this must be our first time. Nuke everything and
               // hope this really was a new user. Create all the normal parameter
               // files which will _not_ be overwritten by a reinstall
               m_okToRewrite = true;
               saveParameters(true);
               saveParameters(false);
               return;
            } else {
               // Unable to read the seed file. This is bad.
               m_okToRewrite = false;
               unsolicitedError(Nav2Error::PARAM_NO_VALID_PARAM_FILE);
               if (m_release) {
                  unsolicitedError(Nav2Error::PANIC_ABORT);
               }
            }
         }
      }
   }

   void Parameter::readOverrideFile()
   {
      if (loadParametersFromFile(OVERRIDEFILE_LOCATION, 0)) {
         /* We found a file containing override commands. */
         /* Remove it now that we have read it in. */
#if defined(__SYMBIAN32__)
         MLIT(KOverrideFileName, OVERRIDEFILE_LOCATION);
         TBuf<256> overrideFileName;
         TPtrC8 basePath((TUint8*)(getGlobalData().m_commondata_base_path));
         overrideFileName.Copy(basePath);
         overrideFileName.Append(KOverrideFileName);
         RFs session;
         TInt retval;
         retval = session.Connect();
         if (retval == KErrNone) {
            retval = session.Delete(overrideFileName);
            if (retval == KErrNone) {
               session.Close();
            }
         }
#else
         /* Delete file. */
         unlink(OVERRIDEFILE_LOCATION);
#endif
      }
   }

   void Parameter::loadParameters()
   {
      m_okToRewrite = false;
      if (loadParametersFromFile(PARAMFILE_LOCATION)) {
         // Worked ok
         m_okToRewrite = true;

         // This hack removes an old cmmc paramfile in favour of the seed file.
         hackRemoveCmmcUserFile();

         readOverrideFile();
         return;
      }

      // Loading of the ordinary file failed - trying alternate.
      if (loadParametersFromFile(PARAMFILE_LOCATION_OLD)) {
         // Worked ok - notify the user
         readOverrideFile();
         unsolicitedError(Nav2Error::PARAM_TRYING_BACKUP);
         m_okToRewrite = true;
         saveParameters(false);
         return;
      }

      // Loading of the backup file failed - trying original (username/password only).
      if (loadParametersFromFile(PARAMFILE_LOCATION_ORIGINAL)) {
         // Worked ok - notify the user
         readOverrideFile();
         unsolicitedError(Nav2Error::PARAM_TRYING_ORIGINAL);
         m_okToRewrite = true;
         saveParameters(false);
         return;
      }

      // Loading of the backup file failed - trying seed (used only the first time normally).
      if (loadParametersFromFile(PARAMFILE_LOCATION_SEED)) {
         // Worked ok - this must be our first time. Nuke everything and
         // hope this really was a new user. Create all the normal parameter
         // files which will _not_ be overwritten by a reinstall
         readOverrideFile();
         m_okToRewrite = true;
         saveParameters(true);
         saveParameters(false);
         return;
      }

      // Failed to load the parameters. Signal an error in the most appropriate way.
      unsolicitedError(Nav2Error::PARAM_NO_VALID_PARAM_FILE);
      if (m_release) {
         unsolicitedError(Nav2Error::PANIC_ABORT);
      }
   }

   bool Parameter::loadParametersFromFile(const char *param_file_name,
         int complain)
   {
      FILE * loadFile;
      char type;
      int numEntries;
      uint32 paramId;

      char fileName[256];
      strcpy(fileName, getGlobalData().m_commondata_base_path);
      strcat(fileName, param_file_name);

      loadFile = fopen(fileName, BINARY_READ);
      if (!loadFile) {
#if !defined(_MSC_VER) && ! defined(__SYMBIAN32__)
         WARN("Unable to open %s : %s", fileName, strerror(errno));
#else
         WARN("Unable to open %s", fileName);
#endif
         if (!m_release && 1 == complain) {
            unsolicitedError(Nav2Error::PARAM_NO_OPEN_FILE);
         }
         return false;
      } else {
         DBG("%s opened for reading", fileName);
      }
      while (!feof(loadFile)) {
         if (fscanf(loadFile, "%c%i:%"PRIx32"\n", &type, &numEntries, &paramId) != 3){
            if (!feof(loadFile)) {
               fclose(loadFile);
               WARN("loadParameters:Unable to read line");
               if (!m_release) {
                  unsolicitedError(Nav2Error::PARAM_NO_READ_LINE);
               }
               return false;
            }
            fclose(loadFile);
            return true;
         }
         //DBG("Got %i of %c, id 0x%08x", numEntries, type, paramId);
         switch (type) {
         case 'B':
            {
               if (numEntries == 0) {
                  eraseParameter(paramId);
                  break;
               }
               if(numEntries != 1){
                  ERR("Binary Blocks can only contain one entry, "
                      "surplus entries will be ignored.");
               }
               int size = 0;
               uint8* data = NULL;
               if(fscanf(loadFile, "+%i:", &size) != 1){
                  WARN("loadParameters: Missing data in 0x%08"PRIx32, paramId);
               } else {
                  data = new uint8[size];
                  if(size_t(size) != fread(data, 1, size, loadFile)){
                     WARN("Failed to read %d bytes of data to param 0x%08"
                          PRIx32, size, paramId);
                  }
                  if('\n' != getc(loadFile)){
                     ERR("Reading of Binary Block 0x%08"PRIx32" failed.", 
                         paramId);
                     delete[] data;
                     fclose(loadFile);
                     if (!m_release) {
                        unsolicitedError(Nav2Error::PARAM_CORRUPT_BINARYBLOCK);
                     }
                     return false;
                  }
                  eraseParameter(paramId);
                  ParamBinaryBlock *p = new ParamBinaryBlock(paramId, data, 
                                                              size);
                  (*m_parameters)[paramId] = p;
                  delete[] data;
               }
            }
            break;
            case 'I':
               {
                  if (numEntries == 0) {
                     /* erase. */
                     eraseParameter(paramId);
                     break;
                  }
                  int32 *data = new int32[numEntries];
                  int i;

                  for (i=0; i<numEntries; i++) {
                     if (fscanf(loadFile, "+%"PRIi32"\n", &(data[i]) ) != 1) {
                        WARN("loadParameters: Missing data in 0x%08"PRIx32, 
                             paramId);
                     }
                  }
                  eraseParameter(paramId);
                  ParamInt32 *p = new ParamInt32(paramId, data, numEntries);
                  (*m_parameters)[paramId] = p;
                  delete[] data;
               }
               break;
            case 'F':
               {
                  if (numEntries == 0) {
                     /* erase. */
                     eraseParameter(paramId);
                     break;
                  }
                  float * data = new float[numEntries];
                  int i;

                  for (i=0; i<numEntries; i++) {
                     if (fscanf(loadFile, "+%g\n", &(data[i]) ) != 1) {
                        WARN("loadParameters: Missing data in 0x%08"PRIx32,
                             paramId);
                     }
                  }
                  eraseParameter(paramId);
                  ParamFloat *p = new ParamFloat(paramId, data, numEntries);
                  (*m_parameters)[paramId] = p;
                  delete[] data;
               }
               break;
            case 'S':
               {
                  if (numEntries == 0) {
                     /* erase. */
                     eraseParameter(paramId);
                     break;
                  }
                  int dataLength;

                  if (fscanf(loadFile, "+%i:", &dataLength) != 1) {
                     WARN("loadParameters: Unable to read dataLength");
                     fclose(loadFile);
                     if (!m_release) {
                        unsolicitedError(Nav2Error::PARAM_CORRUPT_STRING);
                     }
                     return false;
                  }
                  char *data = new char[dataLength];
                  fread(data, dataLength, 1, loadFile);
                  fscanf(loadFile, "\n");
                  eraseParameter(paramId);
                  ParamString *p = new ParamString(paramId, dataLength, data, numEntries);
                  (*m_parameters)[paramId] = p;
                  delete[] data;
               }
               break;
            case '+':
               WARN("loadParameters: out of sync");
               fclose(loadFile);
               if (!m_release) {
                  unsolicitedError(Nav2Error::PARAM_CORRUPT_SYNC);
               }
               return false;
            default:
               WARN("loadParameters: unknown line");
               fclose(loadFile);
               if (!m_release) {
                  unsolicitedError(Nav2Error::PARAM_CORRUPT_TYPE);
               }
               return false;
         }
      }
      fclose(loadFile);
      return true;
   }

   void Parameter::eraseAllParameters()
   {
      map<uint32, Param*, less<uint32> >::iterator i;
      for(i = m_parameters->begin(); i != m_parameters->end(); ++i){
         delete (*i).second;
         (*i).second = NULL;
      }
      m_parameters->clear();
   }

   int Parameter::calculateFileSize()
   {
      map <uint32, Param *, less<uint32> > :: iterator i;
      unsigned bytes = 0;

      for (i = m_parameters->begin() ; i != m_parameters->end() ; ++i )
      {
         // Cannot use -> since Visual C++ is broken
         if ((*i).first & 0x00008000) {
            DBG("calcSize param : 0x%08"PRIx32, (*i).first);
            bytes += (*i).second->toWrite();
         } else {
            DBG("ignoredParam   : 0x%08"PRIx32, (*i).first);
         }
      }
      return bytes;
   }


   void Parameter::writeParameters(filestate saveFile)
   {
      map <uint32, Param *, less<uint32> > :: iterator i;

      for (i = m_parameters->begin() ; i != m_parameters->end() ; ++i )
      {
         // Cannot use -> since Visual C++ is broken
         if ((*i).first & 0x00008000) {
            DBG("saveParameters : 0x%08"PRIx32, (*i).first);
            (*i).second->fileOut(saveFile);
         } else {
            DBG("ignoredParam   : 0x%08"PRIx32, (*i).first);
         }
      }
   }



#if defined(__SYMBIAN32__)
   void Parameter::saveParameters(bool rewriteOrigInstead)
   {
      // Do not overwrite the file if it was damaged
      if (!m_okToRewrite) {
         return;
      }



      MLIT(KFileNameOrig, PARAMFILE_LOCATION_ORIGINAL);
      MLIT(KFileName,     PARAMFILE_LOCATION         );
      MLIT(KFileNameOld,  PARAMFILE_LOCATION_OLD     );
      MLIT(KFileNameTemp, PARAMFILE_LOCATION_TEMP    );

      TBuf<256> fileName;
      TBuf<256> fileNameTemp;
      TBuf<256> fileNameOldParams;
      TPtrC8 basePath((TUint8*)(getGlobalData().m_commondata_base_path));
      fileName.Copy(basePath);
      if (rewriteOrigInstead) {
         fileName.Append(KFileNameOrig);
      } else {
         fileName.Append(KFileName);
      }

      fileNameOldParams.Copy(basePath);
      fileNameOldParams.Append(KFileNameOld);

      fileNameTemp.Copy(basePath);
      fileNameTemp.Append(KFileNameTemp);

      // The session is used throughout this function
      RFs session;
      TInt retval;
      retval = session.Connect();
      retval = session.Delete(fileNameTemp);
      sleep(WAIT_AND_SLEEP_TIME);


      // Check if there is enough space left to write the file
      int filesize = calculateFileSize();
#if !(defined(NAV2_CLIENT_UIQ) || defined(NAV2_CLIENT_UIQ3))
      {
         int driveLetter = toupper(*(getGlobalData().m_commondata_base_path)); 
         TInt drive = EDriveC;             // Default value since the error case is not yet handled
         if (RFs::CharToDrive(driveLetter, drive) != KErrNone) {
            // Do something here??? FIXME.
         }
         CTrapCleanup *tc = CTrapCleanup::New();
         TBool result = false;
         TRAPD(leaveValue, 
               result = SysUtil::DiskSpaceBelowCriticalLevelL(&session,
                                                              filesize, 
                                                              drive));
         delete tc;
         if(result || leaveValue != KErrNone){
            ERR("Unable to save all parameters. Out of disk space.");
            unsolicitedError(Nav2Error::PARAM_NO_SPACE_ON_DEVICE_2);
            // unsolicitedError(Nav2Error::PANIC_ABORT);
            session.Close();
            return;
         }
      }
#endif


      // Create a buffer with enough space in it to hold the whole parameter file
      HBufC8 * buf = HBufC8::NewL(filesize);
      TPtr8 writebuf = buf->Des();
      
      writeParameters(writebuf);

      RFile saveFile;
      // CAP: dependent
      retval = saveFile.Replace(session, fileNameTemp, EFileWrite | EFileStream | EFileShareExclusive);
      if (retval == KErrNone) {
         // Got an open file, write to it
         sleep(WAIT_AND_SLEEP_TIME);
         retval = saveFile.Write(writebuf);
         if ( retval == KErrNone ) {
            retval = saveFile.Flush();
            saveFile.Close();
            sleep(WAIT_AND_SLEEP_TIME);
            session.Delete(fileNameOldParams);
            sleep(WAIT_AND_SLEEP_TIME);
            session.Rename(fileName, fileNameOldParams);
            sleep(WAIT_AND_SLEEP_TIME);
            session.Rename(fileNameTemp, fileName);
         } else {
            saveFile.Close();
            sleep(WAIT_AND_SLEEP_TIME);
            session.Delete(fileNameTemp);
         }
         delete buf;
      } else {
         ERR("Unable to open parameter file for writing.");
         unsolicitedError(Nav2Error::PARAM_NO_SPACE_ON_DEVICE);  // FIXME - better message?
      }

      session.Close();
   }

#else /* defined(__SYMBIAN32__) */

   /* Not symbian */
   void Parameter::saveParameters(bool rewriteOrigInstead)
   {
      // Do not overwrite the file if it was damaged
      if (!m_okToRewrite) {
         return;
      }

      char fileName[256];
      strcpy(fileName, getGlobalData().m_commondata_base_path);
      strcat(fileName, PARAMFILE_LOCATION_TEMP);

      FILE * saveFile;
      saveFile = fopen(fileName, BINARY_WRITE);
      if (!saveFile) {
# if !defined(_MSC_VER)
         ERR("Unable to open %s : %s", fileName, strerror(errno));
# else
         ERR("Unable to open %s", fileName);
# endif
         return;
      }

      writeParameters(saveFile);
      fclose(saveFile);

# if !defined(_MSC_VER)
      /* Linux */
      rename(PARAMFILE_LOCATION, PARAMFILE_LOCATION_OLD);
      rename(PARAMFILE_LOCATION_TEMP, PARAMFILE_LOCATION);
# else
      /* WindowsCE */
      wchar_t base[256];
      wchar_t wfilename1[256];
      wchar_t wfilename2[256];

      int i = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, getGlobalData().m_commondata_base_path, -1, base, 256);
      wcscpy(wfilename1, base);
      wcscat(wfilename1, TEXT(PARAMFILE_LOCATION_OLD));

      DeleteFile(wfilename1);

      wcscpy(wfilename2, base);
      wcscat(wfilename2, TEXT(PARAMFILE_LOCATION));

      MoveFile(wfilename2, wfilename1);

      wcscpy(wfilename1, base);
      wcscat(wfilename1, TEXT(PARAMFILE_LOCATION_TEMP));

      MoveFile(wfilename1, wfilename2);
# endif
   }
#endif /* defined(__SYMBIAN32__) */



} /* namespace isab */

