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

#include "Log.h"
#include "isabTime.h"
#include "Output.h"
#include "RecursiveMutex.h"
#include <stdarg.h>
#include <stdio.h>
#include <algorithm>
#include <ctype.h>
//#define MEMORY_LOG

/** Size of output string buffers. No output string will ever be longer than 
    this. */
#define BUF_SIZE 512

//defeat stupid windows
#undef max
#undef min

namespace isab{

      
#if (defined MEMORY_LOG ) && (defined __SYMBIAN32__)
   const char* const Log::m_stampFormat = "%5d : %7s : %s : %d :";
#else
   const char* const Log::m_stampFormat = "%5d : %7s : %s :";
#endif

   const char* const Log::m_error   = "ERROR";
   const char* const Log::m_warning = "WARNING";
   const char* const Log::m_info    = "INFO";
   const char* const Log::m_debug   = "DEBUG";

   bool Log::addOutput(const char* target,enum FileMode mode)
   {
      mode = mode; // quiets gcc when on it's most cranky level
      bool ret = false;
      Output* newOutput = m_master->getOutput(target, this);
      if(newOutput != NULL){
         std::vector<Output*>::iterator p;
         p = std::find(m_outputs.begin(), m_outputs.end(), newOutput);
         if(p == m_outputs.end()){
            m_outputs.push_back(newOutput);
            ret = true;
         }
      }
      return ret;
   }
   
   void Log::removeOutput(const char* target)
   {
      Output* oldOutput = m_master->getOutput(target, this);
      if(oldOutput != NULL){
         std::vector<Output*>::iterator p;
         p = std::find(m_outputs.begin(), m_outputs.end(), oldOutput);
         if(p != m_outputs.end()){
            m_outputs.erase(p);
         }
         m_master->releaseOutput(target, this);
      }
   }

   
   /** 
    * Logs an error message. Checks if the specifed prefix is allowed to
    * log error messages.
    * @param format a printf-style format string that constitutes the log
    *               message. 
    * @param args enough arguments to match the format string.
    * @return the number of characters printed.
    */
   int Log::real_verror(const char* format, va_list args){
      int pos = 0;
      if(testLevels(LOG_ERROR)){
         pos = log(format, m_error, args);

      }
      return pos;
   }

   /** Logs a warning message. Checks if the specifed prefix is allowed
    * to log warning messages.
    * @param format a printf-style format string that constitutes the log
    *               message. 
    * @param args enough arguments to match the format string.
    * @return the number of characters printed.
    */
   int Log::real_vwarning(const char* format, va_list args){
      int pos = 0;
      if(testLevels(LOG_WARNING)){
         pos = log(format, m_warning, args);
      }
      return pos;
   }

   /** Logs an info message. Checks if the specifed prefix is allowed to
    * log info messages.
    * @param format a printf-style format string that constitutes the log
    *               message. 
    * @param args enough arguments to match the format string.
    * @return the number of characters printed.
    */
   int Log::real_vinfo(const char* format, va_list args){
      int pos = 0;
      if(testLevels(LOG_INFO)){
         pos = log(format, m_info, args);
      }
      return pos;
   }

   /** Logs a debug message. Checks if the specifed prefix is allowed to
    * log debug messages.
    * @param format a printf-style format string that constitutes the log
    *               message. 
    * @param args enough arguments to match the format string.
    * @return the number of characters printed.
    */
   int Log::real_vdebug(const char* format, va_list args){
      int pos = 0;
      if(testLevels(LOG_DEBUG)){
         pos = log(format, m_debug, args);
      }
      return pos;
   }

   int Log::errorDump(const char* logMsg, 
                      const uint8* data, size_t dataLen, 
                      int base, int group, int row )
   {
      int pos = 0;
      if(testLevels(LOG_ERROR)){
         pos = dump(logMsg, m_error, data, dataLen, base, group, row);
      }
      return pos;
   }

   int Log::warningDump(const char* logMsg, 
                        const uint8* data, size_t dataLen, 
                        int base, int group, int row )
   {
      int pos = 0;
      if(testLevels(LOG_WARNING)){
         pos = dump(logMsg, m_warning, data, dataLen, base, group, row);
      }
      return pos;
   }

   int Log::infoDump(const char* logMsg, 
                     const uint8* data, size_t dataLen, 
                     int base, int group, int row )
   {
      int pos = 0;
      if(testLevels(LOG_INFO)){
         pos = dump(logMsg, m_info, data, dataLen, base, group, row);
      }
      return pos;
   }

   int Log::debugDump(const char* logMsg, 
                      const uint8* data, size_t dataLen, 
                      int base, int group, int row )
   {
      int pos = 0;
      if(testLevels(LOG_DEBUG)){
         pos = dump(logMsg, m_debug, data, dataLen, base, group, row);
      }
      return pos;
   }
   

   int Log::dump(const char* logMsg, const char* level,
                 const uint8* data, size_t dataLen, 
                 int base, int group, int row )
   {
      int retSum = 0;               //the return value accumulator
      char locfmt[] = "%#0*    \0"; //the format string for each group
      char msg[BUF_SIZE];           //output buffer
      int pos = 4;                  //format string write position
      // matrix of widths for the format string.
      //          group    1 2  4
      int widths[4][3] = {{4,6,10}, //x
                          {3,5,10}, //u
                          {4,7,12}, //o
                          {1,1, 1}};//c

      isabTime stamp; //timestamp to write
      // write the correct number of 'h' in format string
      switch(group){  
         case  2: locfmt[pos++] = 'h'; break;
         case  4: break;
         case  1:
         default: group = 1; locfmt[pos++] = 'h'; locfmt[pos++] = 'h';
      }
      
      //select and write output format - octal, hex, or decimal
      switch(base){
      case  0: pos = 1;   locfmt[pos++] = '*'; //this is a little compilcated
                          locfmt[pos++] = 'c'; 
                          locfmt[pos--] = ' '; break; 
      case 10:            locfmt[pos++] = 'u'; break;
      case  8:            locfmt[pos++] = 'o'; break;
      case 16:
      default: base = 16; locfmt[pos++] = 'x'; break;
      }
      locfmt[++pos] = '\0'; //finish the string, ++pos to leave a space

      
#if (defined MEMORY_LOG ) && (defined __SYMBIAN32__)
      //for memory logging in symbian
      TInt largestBlock;
      pos = snprintf(msg, BUF_SIZE - pos, m_stampFormat, 
                     stamp.millis(), level, m_prefix, User::Available(largestBlock) );
#else
      //write log system prefix (timestamp, level prefix)
      //You'd think that it's necessary to reset pos before this
      //printf call, but unless BUF_SIZE is way to small it doesn't
      //matter.
      pos = snprintf(msg, BUF_SIZE - pos, m_stampFormat, 
                     stamp.millis(), level, m_prefix );
#endif
      snprintf(msg + pos, BUF_SIZE - pos, "%s\n", logMsg);//write log string
      retSum = strlen(msg);     //number of characters written so far
      msg[BUF_SIZE - 1] = '\0'; //Safety '\n' and '\0' in case the printf was
      msg[BUF_SIZE - 2] = '\n'; //truncated
      output(msg);              //output 'header' line
      
      int rows = (dataLen/group)/row; //number of rows needed.
      int colonPos = pos - 1;         //column where the ':' should be written
      const uint8* src = data;        //temp pointer to the data to dump
      uint8* charTmp = NULL;          //pointer for character data
      if(base == 0){ 
         charTmp = new uint8[dataLen];
         memcpy(charTmp, data, dataLen);
         for(unsigned i = 0; i < dataLen; ++i){
            if(!isprint(charTmp[i])){
               charTmp[i] = '\xb7';
            }
         } //must redirect data as well for for loop to work
         data = src = charTmp; 
      }
      //select the width to use based on base and group.
      int basew[] = {16,10,8,0};
      unsigned w1 = 0;
      while((w1 < (sizeof(basew)/sizeof(*basew))) && (basew[w1] != base))
         ++w1;
   //int w1 = ((base == 16) ? 0 : ((base == 10) ? 1 : ((base == 8) ? 2 : 3)));
      int w2 = group / 2;
      int width = widths[w1][w2];
      for(int r = 0; r <= rows; ++r){ //for each row
         //print address and colon.
         pos = sprintf(msg, "%*s0x%0*x : ", colonPos - 11, "", 8, r*row*group);
         char* addrend = msg + pos; //where to startdumping data
         for(int c = 0; (c < row) && (src < data + dataLen); ++c){//foreach col
            //print a column using the produced format string
            uint32 tmp = 0; //Correct alignment for all types.
            uint8* tmpp = (uint8*)(&tmp);
            int numleft = data + dataLen - src;

            switch(std::min(numleft, group)){
            case 4:  *tmpp++ = *src++; 
            case 3:  *tmpp++ = *src++;
            case 2:  *tmpp++ = *src++;
            case 1: 
            default: *tmpp++ = *src++;
            }
            tmpp = (uint8*)&tmp;
            pos += snprintf(msg + pos, BUF_SIZE - pos, locfmt, width, *tmpp);
            //src += group; //jump src pointer the right amount
         }
         pos += snprintf(msg + pos, BUF_SIZE - pos, "\n");//linebreak
         msg[BUF_SIZE - 2] = '\n';//Safety '\n' and '\0' in case the printf 
         msg[BUF_SIZE - 1] = '\0';//truncated the line
         retSum += strlen(msg); //update return value accumulator
         if(base == 16){ //special case for hex output
            //some printf implementations print "0000" on
            //printf("%#4hhx",0) (similar for 16 and 32 bit
            //values). The contents of this if-statement changes any
            //string of the type ' 00\(0+\) ' to ' 0x\1 '.
            char match[] = "0000000000\0"; //able to match a uint32
            match[group*2 + 2] = '\0';     //select the right start
            char* hexnull = NULL;           
            while(NULL != (hexnull = strstr(addrend, match))){
               //if the strstr function finds any match, that match is modified
               hexnull[1] = 'x'; //by substituting the second '0' with 'x'
            }
         }
         output(msg); //output the row.
      }
      delete[] charTmp;
      return retSum; //return the return value accumulator.
   }

   int Log::log(const char* logMsg, const char* level, va_list args){
      char msg[BUF_SIZE];
      int pos = 0;
      isabTime stamp;
#if (defined MEMORY_LOG ) && (defined __SYMBIAN32__)
      int largestBlock; //TInt largestBlock;
      pos = snprintf(msg, BUF_SIZE - pos, m_stampFormat, stamp.millis(), 
                     level, m_prefix, User::Available(largestBlock));
#else
      pos = snprintf(msg, BUF_SIZE - pos, m_stampFormat, stamp.millis(), 
                     level, m_prefix);
#endif

      pos += vsnprintf(msg + pos, BUF_SIZE - pos, logMsg, args);
      msg[BUF_SIZE - 1] = '\0';
      int n = strlen(msg);
      if(n == BUF_SIZE - 1){ 
         msg[BUF_SIZE - 2] = '\n';
         pos = BUF_SIZE - 1;
      } else if(msg[n-1] != '\n'){
         msg[n] = '\n';
         msg[n+1] = '\0';
      }
      
      output(msg);

      return pos;
   }

   void Log::output(const char* msg)
   {
      std::vector<Output*>::iterator p;
      for(p = m_outputs.begin(); p != m_outputs.end(); ++p){
         (*p)->puts(msg);
      }
   }

   class LogMaster* Log::getLogMaster()
   {
      return m_master;
   }

   Log::levels_t Log::setLevels(levels_t newLevels){
      lock();
      levels_t ret = m_levels;
      m_levels = newLevels;
      unlock();
      return ret;
   }
   
   Log::levels_t Log::getLevels(){
      lock();
      levels_t l = m_levels;
      unlock();
      return l;
   }

   Log::Log(const char* prefix, levels_t levels, class LogMaster* master) 
      : m_mutex(NULL), m_master(master)
   {
      m_mutex = new RecursiveMutex();
      if(!m_master){
         m_master = &LogMaster::getLogMaster();
      }
      m_prefix = new char[strlen(prefix) + 1];
      strcpy(m_prefix, prefix);
      m_levels = levels;
      m_outputs.push_back(m_master->getDefaultOutput(this));
      levels_t fileLevels = m_master->registerLog(this);
      m_levels = levels & fileLevels;
   }

   Log::~Log()
   {
      while(! m_outputs.empty()){
         Output* tmp = m_outputs.back();
         m_outputs.pop_back();
         m_master->releaseOutput(tmp, this);
      }
      if(0 == m_master->unregisterLog(this)){
         //LogMaster::noMoreLogMaster();
      }
      delete[] m_prefix;
      delete m_mutex;
   }

   void Log::lock()
   {
      m_mutex->lock();
   }

   void Log::unlock()
   {
      m_mutex->unlock();
   }

}
