/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef __SYMBIAN32__
# include <e32std.h>
# include <cctype>
#endif
#include <string.h>

#include "Log.h"
#include "GlobalData.h"
#include "Output.h"
#include "nav2util.h"
#include "RecursiveMutex.h"

#include "LogMacros.h" //define NO_LOG_OUTPUT
#ifndef NO_LOG_OUTPUT
//If we have no logging, don't write loglevels.txt.
# define LOGLEVELS
#endif

#ifdef __linux
# define DEFAULTOUTPUT "cerr"
#elif defined __SYMBIAN32__
// WARNING!!! DO NOT ACTIVATE ANY OTHER OUTPUT HERE, DO IT IN
// PARAMETERS OR AFTER THE LOGMASTER OBJECT HAS BEEN CREATED ___AND___
// THE TLS VALUE IS SET TO A CORRECT VALUE!!! VERY IMPORTANT!!!
# define DEFAULTOUTPUT "null"
#elif defined _MSC_VER
# define DEFAULTOUTPUT "trace"
#else
# define DEFAULTOUTPUT "null"
#endif


namespace isab {
   LogMaster& LogMaster::getLogMaster()
   {
      return * (getGlobalData().m_logmaster);
   }
 
   LogMaster::OutputRecord* LogMaster::findRecord(const char* target)
   {
      OutputRecord* ret = NULL;
      std::vector<OutputRecord*>::iterator p;
      for(p = m_records.begin(); p != m_records.end(); ++p){
         if(0 == strcmp((*p)->target, target)){
            ret = *p;
            break;
         }
      }
      return ret;
   }

   void LogMaster::OutputRecord::addUser(Log* user)
   {
      std::vector<Log*>::iterator User = findUser(user);
      if(User == users.end()){
         users.push_back(user);
      }
   }

   void LogMaster::OutputRecord::removeUser(Log* user)
   {
      std::vector<Log*>::iterator User = findUser(user);
      if(User != users.end()){
         users.erase(User);
      }
   }

   std::vector<Log*>::iterator LogMaster::OutputRecord::findUser(Log* user)
   {
      return std::find(users.begin(), users.end(), user);
   }

   Output* LogMaster::getOutput(const char* target, Log* user)
   {
      Output* ret = NULL;
      lock();
      OutputRecord* rec = findRecord(target);
      if(rec != NULL){
         if(user != NULL){
            rec->addUser(user);
         } else {
            rec->isDefault = true;
         }
         ret = rec->output;
      } else {
         if(NULL != (ret = OutputFactory::createOutput(target))){
            rec = new OutputRecord(target, ret, user);
            m_records.push_back(rec);
         }
      }
      unlock();
      return ret;
   }

   void LogMaster::releaseOutput(const char* target, Log* user)
   {
      lock();
      OutputRecord* rec = findRecord(target);
      if(rec != NULL){
         if(user != NULL){
            rec->removeUser(user);
         } else {
            rec->isDefault = false;
         }
         if((rec->users.size() == 0) && !(rec->isDefault)){
            std::vector<OutputRecord*>::iterator q;
            for(q = m_records.begin(); q != m_records.end() && *q != rec; ++q) {}
            if(q != m_records.end()){
               m_records.erase(q);
               Output* op = rec->output;
               op->puts("Shut down by LogMaster\n");
               delete rec;
               delete op;
            }
         }
      }
      unlock();
   }

   void LogMaster::releaseOutput(Output* op, Log* user)
   {
      lock();
      std::vector<OutputRecord*>::iterator p = m_records.begin();
      for(; p != m_records.end() && (*p)->output != op; ++p) {}
      if(p != m_records.end()){
         releaseOutput((*p)->target, user);
      }
      unlock();
   }

   Output* LogMaster::setDefaultOutput(const char* target)
   {
      lock();
      Output* newOutput = getOutput(target, NULL);
      if(newOutput){
         releaseOutput(m_defaultOutput, NULL);
         m_defaultOutput = newOutput;
      }
      unlock();
      return m_defaultOutput;
   }

   Output* LogMaster::getDefaultOutput(char* target, size_t n)
   {
      lock();
      Output* ret = m_defaultOutput;
      if(n > 0 && target != NULL){
         int size = m_records.size();
         for(int i = 0; i < size; ++i){
            if(m_records[i]->output == ret){
               strncpy(target, m_records[i]->target, n);
               target[n - 1] = '\0';
               break;
            }
         }
      }
      unlock();
      return ret;
   }

   Output* LogMaster::getDefaultOutput(Log* user)
   {
      lock();
      Output* ret = m_defaultOutput;
      if(user){
         int size = m_records.size();
         for(int i = 0; i < size; ++i){
            if(m_records[i]->output == ret){
               m_records[i]->addUser(user);
            }
         }
      }
      unlock();
      return ret;
   }

   /** Will only be called once. */
   LogMaster::~LogMaster()
   {
      while(!m_records.empty()){
         releaseOutput((*m_records.begin())->target, NULL);
      }
#ifdef LOGLEVELS
      appendLogLevels();
#endif
      delete[] m_logLevelsFilename;
      delete[] m_defaultPath;
      delete m_nullOutput;
      delete m_mutex;
   }

   Log::levels_t LogMaster::setSuperLevels(Log::levels_t level)
   {
      lock();
      Log::levels_t ret = m_superLevels;
      m_superLevels = level;
      unlock();
      return ret;
   }

   Log::levels_t LogMaster::getSuperLevels()
   {
      lock();
      Log::levels_t ret = m_superLevels;
      unlock();
      return ret;
   }

   Log::levels_t LogMaster::registerLog(Log* log)
   {
      Log::levels_t retval = Log::LOG_ALL;
      lock();
      std::vector<Log*>::iterator p;
      p = std::find(m_registeredLogs.begin(), m_registeredLogs.end(), log);
      if(p == m_registeredLogs.end()){
         std::vector<PrefixLevels>::iterator q;
         PrefixLevels prospect(log->getPrefix(), retval, true);
         q = std::find(m_prefixLevelsV.begin(), m_prefixLevelsV.end(), 
                       prospect);
         if(q != m_prefixLevelsV.end()){
            retval &= (*q).levels;
         } else {
            // append all unknown prefixes to the list. 
            m_prefixLevelsV.push_back(prospect);
         }
         m_registeredLogs.push_back(log);
      }
      unlock();
      return retval;
   }

   int LogMaster::unregisterLog(Log* log)
   {
      lock();
      unsigned ret = 0;
      std::vector<Log*>::iterator p;
      p = std::find(m_registeredLogs.begin(), m_registeredLogs.end(), log);
      if(p != m_registeredLogs.end()){
         m_registeredLogs.erase(p);
      }
      ret = m_registeredLogs.size();
      unlock();
      return ret;
   }

   int LogMaster::PrefixLevels::compare(const void* pl1, const void* pl2)
   {
      typedef const PrefixLevels* const cPLpc;
      cPLpc p1 = *static_cast<cPLpc*>(pl1);
      cPLpc p2 = *static_cast<cPLpc*>(pl2);
      return  strcmp(p1->prefix, p2->prefix);
   }

   LogMaster::LogMaster(const char *default_path) : 
      m_mutex(NULL), m_nullOutput(NULL), 
      m_defaultOutput(NULL), m_superLevels(Log::LOG_ALL), 
      m_logLevelsFilename(NULL), m_defaultPath(NULL)
   {
      m_mutex = new RecursiveMutex();
      m_defaultPath = strdup_new(default_path);

      m_nullOutput = new NullOutput;
      m_defaultOutput = m_nullOutput;

      setDefaultOutput(DEFAULTOUTPUT);
      const char* ll_filename = "loglevels.txt";
      const size_t filename_len = 
         strlen(m_defaultPath) + strlen(ll_filename) + 4;
      m_logLevelsFilename = new char[filename_len];
      if(m_logLevelsFilename){
         strcpy(m_logLevelsFilename, m_defaultPath);
         strcat(m_logLevelsFilename, ll_filename);
      }
      readLogLevels();
   }

   void LogMaster::readLogLevels()
   {
#ifdef LOGLEVELS
# ifdef LOG_LOGLEVELS
      FILE* mf = NULL;
#  ifdef __SYMBIAN32__
      char fileName[256];
      char *base = m_defaultPath;
      strcpy(fileName, base);
      strcat(fileName, "lm.txt");

      mf = fopen(fileName, "w");
#  else
      mf = fopen("lm.txt","w");
#  endif
# endif
# if !defined(_MSC_VER) && !defined(__MWERKS__)
#  ifdef LOG_LOGLEVELS
#   define LM(format, args...) if(mf)fprintf(mf, format ,##args)
#  else
#   define LM(format, args...) 
#  endif
# else
#  define LM (void)
# endif
      FILE* logfile = NULL;
      if(m_logLevelsFilename != NULL){
         logfile = fopen(m_logLevelsFilename, "r");
      }
      if(logfile != NULL){
         LM("loglevels opened\n");
         char line[512];
         while(NULL != fgets(line, 512, logfile)){
            LM("line: %s\n", line);
            if(line[0] == '#' || line[0] == '\n'){
               continue;
            }
            char* p = line;
            while(!isgraph(*p)) p++;
            char* prefix = p;
            while(isgraph(*p)) p++;
            *p++ = '\0';
            Log::levels_t lvl = 0;
            if(NULL == strstr(p, "NONE")){
               LM("no NONE found\n");
               if(NULL != strstr(p, "ALL")){
                  LM("ALL found\n");
                  lvl = Log::LOG_ALL;
               } else {
                  lvl |= (strstr(p, "ERROR"))   ? Log::LOG_ERROR   : 0;
                  lvl |= (strstr(p, "DEBUG"))   ? Log::LOG_DEBUG   : 0;
                  lvl |= (strstr(p, "WARNING")) ? Log::LOG_WARNING : 0;
                  lvl |= (strstr(p, "INFO"))    ? Log::LOG_INFO    : 0;
               }
            }
            m_prefixLevelsV.push_back(PrefixLevels(prefix, lvl));
            LM("prefix: %s, levels: %0#"PRIx8"\n", prefix, lvl);
         }
         fclose(logfile);
      } else if(m_logLevelsFilename != NULL){
         if(NULL != (logfile = fopen(m_logLevelsFilename, "w"))) {
            fclose(logfile);
         }
      }
      //just so the list isn't empty.
      if(m_prefixLevelsV.empty()){
         m_prefixLevelsV.push_back(PrefixLevels("LogMaster", Log::LOG_ALL));
      }
# ifdef LOG_LOGLEVELS
      if(mf) fclose(mf);
# endif
#endif
   }


   void LogMaster::lock()
   {
      m_mutex->lock();
   }

   void LogMaster::unlock()
   {
      m_mutex->unlock();
   }


   void LogMaster::appendLogLevels()
   {
      if(m_logLevelsFilename){
         FILE* lvlsfile = fopen(m_logLevelsFilename, "at");
         if(lvlsfile){
            bool commentPrinted = false;
            int width = 0;
            while(!m_prefixLevelsV.empty()){
               PrefixLevels& back = m_prefixLevelsV.back();
               if(back.unknown){
                  if(!commentPrinted){
                     fputs("# Added by LogMaster\n", lvlsfile);
                     commentPrinted = true;
                  }
                  int len = strlen(back.prefix);
                  if(width < len) width = len;
                  fprintf(lvlsfile, "%-*s %s %s %s %s\n", width, back.prefix,
                          back.levels & Log::LOG_ERROR ? "ERROR" : "",
                          back.levels & Log::LOG_WARNING ? "WARNING" : "",
                          back.levels & Log::LOG_INFO ? "INFO" : "",
                          back.levels & Log::LOG_DEBUG ? "DEBUG" : "");
               }
               m_prefixLevelsV.pop_back();
            }
            fclose(lvlsfile);
         }
      }
   }

   inline bool LogMaster::testSuperLevels(Log::levels_t levels)
   {
      return (getSuperLevels() & levels) != 0;
   }
      
   LogMaster::OutputRecord::OutputRecord(const char* Target, 
                                                Output* theOutput, 
                                                Log* first) :
      output(theOutput), isDefault(false)
   {
      if(first){
         users.push_back(first);
      } else {
         isDefault = true;
      }
      target = new char[strlen(Target) + 1];
      strcpy(target, Target);
   }


   LogMaster::OutputRecord::~OutputRecord()
   {
      delete[] target;
      while(!users.empty()){
         users.pop_back();
      }
   }

   LogMaster::PrefixLevels::PrefixLevels(const char* prf, 
                                                Log::levels_t lvl,
                                                bool unknown_p) : 
      prefix(NULL), levels(lvl), unknown(unknown_p)
   {
      prefix = new char[strlen(prf) + 1];
      strcpy(prefix, prf);
   }

   LogMaster::PrefixLevels::PrefixLevels(const PrefixLevels& from) :
      prefix(NULL), levels(from.levels), unknown(from.unknown)
   {
      if(from.prefix){
         prefix = strdup_new(from.prefix);
      }
   }

   LogMaster::PrefixLevels& 
   LogMaster::PrefixLevels::operator=(const LogMaster::PrefixLevels& rhs)
   {
      if(this != &rhs){ 
         replaceString(prefix, rhs.prefix);
         levels = rhs.levels;
      }
      return *this;
   }

   inline 
   bool LogMaster::PrefixLevels::operator==(const LogMaster::PrefixLevels& rhs) const
   {
      return 0 == strcmp(prefix, rhs.prefix);
   }

   inline 
   bool LogMaster::PrefixLevels::operator<(const LogMaster::PrefixLevels& rhs) const
   {
      return strcmp(prefix, rhs.prefix) < 0;
   }


}

