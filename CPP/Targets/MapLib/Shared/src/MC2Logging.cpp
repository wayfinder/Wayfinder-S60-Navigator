/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2Logging.h"

#ifndef __SYMBIAN32__
struct nullstream : std::ostream {
   struct nullbuf: std::streambuf {
      int overflow(int c) { return traits_type::not_eof(c); }
   } m_sbuf;
   nullstream(): std::ios(&m_sbuf), std::ostream(&m_sbuf) {}
};
#endif


// ---- FOR ALL OTHER PLATFORMS THAN WIN32: MC2Logging.cpp version 1.8 ----

   #ifdef NO_REAL_OSTREAM
   ostream mc2log_real;
   #endif

   #ifdef __unix__

   #include "MC2Logging.h"

   #include <time.h>
   #include <stdio.h>
   #include <string.h>
   #ifndef _MSC_VER
   #include <unistd.h>
   #endif

   ostream&
   MC2Logging::createLogStream(ostream& outStream)
   {
      ostream* newStream;

      newStream = new ostream(new LogBuffer(outStream.rdbuf()));
      *newStream << info << "*** MC2 logging starting";
   #ifdef HAVE_ARCH
      *newStream << " for pid: " << getpid();
   #endif
      *newStream << endl;
      return *newStream;   
   }

   ostream&
   MC2Logging::createLogStream(int outFD)
   {
      ostream* newStream;

      newStream = new ostream(new LogBuffer(outFD));
      *newStream << info << "*** MC2 logging starting";
   #ifdef HAVE_ARCH
      *newStream << " for pid: " << getpid();
   #endif
      *newStream << endl;
      return *newStream;   
   }

   void
   MC2Logging::deleteHandlers(ostream& logStream)
   {
      if (dynamic_cast<LogBuffer*>(logStream.rdbuf()) != NULL)
         static_cast<LogBuffer*>(logStream.rdbuf())->deleteHandlers();
   }

   void
   MC2Logging::debugOutput(bool debug)
   {
      dynamic_cast<LogBuffer*>(mc2log_ostream.rdbuf())->setDebug(debug);
   }

   LogBuffer::LogBuffer(streambuf *buf) : m_buf(buf)
   {
        // we want no buffering here:
        // put area set to zero
        setp(0, 0);
        // get area set to zero
        setg(0, 0, 0);

        m_newline = true;
        m_level = LOGLEVEL_DEBUG;
        m_buffer = new char[32768]; // the size is hard coded below as well!
        m_bufPos = 0;
        m_debug = true;
        m_fd = -1;
        m_prefix = NULL;
        m_magic = false;

        m_handlersDebug = NULL;
        m_handlersInfo  = NULL;
        m_handlersWarn  = NULL;
        m_handlersError = NULL;
        m_handlersFatal = NULL;
   }

   LogBuffer::LogBuffer(int outFD)
   {
        // we want no buffering here:
        // put area set to zero
        setp(0, 0);
        // get area set to zero
        setg(0, 0, 0);

        m_newline = true;
        m_level = LOGLEVEL_DEBUG;
        m_buffer = new char[32768]; // the size is hard coded below as well!
        m_bufPos = 0;
        m_debug = true;
        m_buf = NULL;
        m_fd = outFD;
        m_prefix = NULL;
        m_magic = false;

        m_handlersDebug = NULL;
        m_handlersInfo  = NULL;
        m_handlersWarn  = NULL;
        m_handlersError = NULL;
        m_handlersFatal = NULL;
        m_externalBuffer = NULL;
   }

   LogBuffer::LogBuffer(LogBuffer* outBuf)
   {
        // we want no buffering here:
        // put area set to zero
        setp(0, 0);
        // get area set to zero
        setg(0, 0, 0);

        m_newline = true;
        m_level = LOGLEVEL_DEBUG;
        m_buffer = new char[32768]; // the size is hard coded below as well!
        m_bufPos = 0;
        m_debug = true;
        m_buf = NULL;
        m_fd = -1;
        m_prefix = NULL;
        m_magic = false;

        m_handlersDebug = NULL;
        m_handlersInfo  = NULL;
        m_handlersWarn  = NULL;
        m_handlersError = NULL;
        m_handlersFatal = NULL;
        m_externalBuffer = outBuf;
   }

   void
   LogBuffer::removeList(LogHandlerItem*& list)
   {
      LogHandlerItem* i;
      while (list != NULL) {
         i = list;
         list = list->m_next;
         delete i;
      }
   }

   LogBuffer::~LogBuffer()
   {
      // we don't handle the destruction of the streambuf
     
      deleteHandlers();
       
      delete[] m_buffer;
   }

   void
   LogBuffer::setPrefix(char* prefix)
   {
      if (m_prefix != NULL && (m_bufPos == (int)strlen(m_prefix))) {
         m_bufPos = 0;
      } 
      if (m_bufPos == 0 && prefix != NULL) {
         strcpy(m_buffer, prefix);
         m_bufPos = strlen(prefix);
      }

      m_prefix = prefix;
   }

   void
   LogBuffer::deleteHandlers()
   {
      removeList(m_handlersDebug);
      removeList(m_handlersInfo);
      removeList(m_handlersWarn);
      removeList(m_handlersError);
      removeList(m_handlersFatal);
   }

   bool
   LogBuffer::addHandler(int level, LogHandler* handler)
   {
      if ((level & LOGLEVEL_DEBUG) == LOGLEVEL_DEBUG)
         new LogHandlerItem(handler, &m_handlersDebug);

      if ((level & LOGLEVEL_INFO) == LOGLEVEL_INFO)
         new LogHandlerItem(handler, &m_handlersInfo);

      if ((level & LOGLEVEL_WARN) == LOGLEVEL_WARN)
         new LogHandlerItem(handler, &m_handlersWarn);

      if ((level & LOGLEVEL_ERROR) == LOGLEVEL_ERROR)
         new LogHandlerItem(handler, &m_handlersError);

      if ((level & LOGLEVEL_FATAL) == LOGLEVEL_FATAL)
         new LogHandlerItem(handler, &m_handlersFatal);

      return true;
   }

   void
   LogBuffer::setDebug(bool debug)
   {
      m_debug = debug;
   }

   int
   LogBuffer::output(const char* buf, int len)
   {
   #ifdef _MSC_VER
      return len;
   #else
      if (m_fd != -1)
         return  write(m_fd, buf, len);

      if (m_buf != NULL)
         return m_buf->sputn(buf, len);

      return -1;
   #endif
   }

   void
   LogBuffer::notify()
   {
      m_buffer[m_bufPos] = '\0';
      if (NULL == m_externalBuffer) {
         LogHandlerItem* list = NULL;

         m_buffer[m_bufPos] = '\n';
         output(m_buffer, m_bufPos + 1);
         m_buffer[m_bufPos] = '\0';
         
         if (LOGLEVEL_DEBUG == m_level)
            list = m_handlersDebug;
         else if (LOGLEVEL_INFO == m_level)
            list = m_handlersInfo;
         else if (LOGLEVEL_WARN == m_level)
            list = m_handlersWarn;
         else if (LOGLEVEL_ERROR == m_level)
            list = m_handlersError;
         else if (LOGLEVEL_FATAL == m_level)
            list = m_handlersFatal;

         while (list != NULL) {
            list->m_handler->handleMessage(m_level, m_buffer, m_bufPos,
                                           m_levelBuf, m_timeBuf + 2);
            list = list->m_next;
         }
      } else {
         m_externalBuffer->notifyExternal(m_level, m_buffer, m_bufPos,
                                          m_levelBuf, m_timeBuf + 2);
      }
   }

   void
   LogBuffer::notifyExternal(int level, const char* msg, int msgLen,
                             const char* levelStr, const char* timeStamp)
   {
      LogHandlerItem* list = NULL;


      char* buf = new char[msgLen + 20 + 20];
      strcpy(buf, timeStamp);
      strcat(buf, levelStr);
      strcat(buf, msg);
      strcat(buf, "\n");

      output(buf, strlen(buf));
      delete [] buf;

      if (LOGLEVEL_DEBUG == m_level)
         list = m_handlersDebug;
      else if (LOGLEVEL_INFO == m_level)
         list = m_handlersInfo;
      else if (LOGLEVEL_WARN == m_level)
         list = m_handlersWarn;
      else if (LOGLEVEL_ERROR == m_level)
         list = m_handlersError;
      else if (LOGLEVEL_FATAL == m_level)
         list = m_handlersFatal;

      while (list != NULL) {
         list->m_handler->handleMessage(level, msg, msgLen,
                                        levelStr, timeStamp);
         list = list->m_next;
      }
   }

   int      
   LogBuffer::putTime()
   { 
      time_t tt;
      struct tm* tm;

      ::time(&tt);
      tm = localtime(&tt);

      sprintf(m_timeBuf, "%04d-%02d-%02d %02d:%02d:%02d ",
              tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
              tm->tm_hour, tm->tm_min, tm->tm_sec);

      if (m_externalBuffer == NULL) {
         if (output(m_timeBuf + 2, 20-2) != 20 - 2)
            return EOF;
         else
            return 0;
      } else
         return 0;
      
   }

   int      
   LogBuffer::putLevel()
   { 
      m_levelBuf[0] = '\0';
      switch (m_level) {
         case LOGLEVEL_DEBUG:
            if (m_debug)
               strcpy(m_levelBuf, "DEBUG: ");
         break;
         case LOGLEVEL_INFO:
            strcpy(m_levelBuf, "INFO : ");
         break;
         case LOGLEVEL_WARN:
            strcpy(m_levelBuf, "WARN : ");
         break;
         case LOGLEVEL_ERROR:
            strcpy(m_levelBuf, "ERROR: ");
         break;
         case LOGLEVEL_FATAL:
            strcpy(m_levelBuf, "FATAL: ");
         break;
         default:
            strcpy(m_levelBuf, "FOO!?: ");
         break;
      }

      if (m_externalBuffer == NULL) {
         if (m_levelBuf[0] != '\0' && output(m_levelBuf, 7) != 7)
            return EOF;
         else
            return 0;
      } else
         return 0;
   }


   int      
   LogBuffer::overflow(int c)
   {
      switch(c) {
         case EOF:
            // NOP
         break;
         case '\n':
   /*         if (m_level != LOGLEVEL_DEBUG || m_debug)
               m_buf->overflow(c); */
            if (m_bufPos > 0)
               notify();
            // set the flag
            m_newline = true;
            m_level = LOGLEVEL_DEBUG;
            m_bufPos = 0;
            if (m_prefix != NULL) {
               strcpy(m_buffer, m_prefix);
               m_bufPos = strlen(m_buffer);
            }
         break;
         case '\0':
            // handle the next character as a magic one ?
            if (m_newline)
               m_magic = true;
         break;
         default:
            if (m_magic) {
               m_magic = false;
               if ('I' == c)
                  m_level = LOGLEVEL_INFO;
               else if ('W' == c)
                  m_level = LOGLEVEL_WARN;
               else if ('E' == c)
                  m_level = LOGLEVEL_ERROR;
               else if ('F' == c)
                  m_level = LOGLEVEL_FATAL;
            } else if (m_level != LOGLEVEL_DEBUG || m_debug) {
               // should we output the time and log level now?
               if (m_newline) {
                  m_newline = false;
                  if (putTime() == EOF)
                     return EOF;
                  if (putLevel() == EOF)
                     return EOF;
               }
               // put it in our buffer
               if (m_bufPos < 32766)
                  m_buffer[m_bufPos++] = (char)c;
            }
         break;
      }

      return 0;
   }

   int      
   LogBuffer::underflow()
   {
      // we don't handle input
      return EOF;
   }

   int      
   LogBuffer::uflow()
   {
      // we don't handle input
      return EOF;
   }

   int      
   LogBuffer::sync()
   {
      return 0;
   //   return m_buf->sync();
   }

   LogBuffer::LogHandlerItem::LogHandlerItem(LogHandler* handler,
                                             LogHandlerItem **list)
   {
      LogHandlerItem* item;

      m_handler = handler;
      m_next = NULL;

      if (NULL == *list)
         *list = this;
      else {
         item = *list;
         while (item->m_next != NULL)
            item = item->m_next;
         item->m_next = this;
      }
   }

   nullstream ns;

   // ostream& mc2log = MC2Logging::createLogStream(cerr);
   ostream& mc2log_ostream = MC2Logging::createLogStream(ns);

   // struct mallinfo mi;

   ostream & info (ostream & os)
   {
          os << '\0';
          return (os << 'I');
   }

   ostream & warn (ostream & os)
   {
          os << '\0';
          return (os << 'W');
   }

   ostream & error (ostream & os)
   {
          os << '\0';
          return (os << 'E');
   }

   ostream & fatal (ostream & os)
   {
          os << '\0';
          return (os << 'F');
   }

#elif !defined( __SYMBIAN32__ )
   ostream & info (ostream & os)
   {
          os << '\0';
          return (os << 'I');
   }

   ostream & warn (ostream & os)
   {
          os << '\0';
          return (os << 'W');
   }

   ostream & error (ostream & os)
   {
          os << '\0';
          return (os << 'E');
   }

   ostream & fatal (ostream & os)
   {
          os << '\0';
          return (os << 'F');
   }
#endif


