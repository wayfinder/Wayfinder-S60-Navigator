/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MC2LOGGING_H
#define MC2LOGGING_H
   
#include "mcostream.h"

#ifdef __SYMBIAN32__

   #define NO_REAL_OSTREAM

   extern ostream mc2log_real;

#if 0
   #define mc2dbg mc2log
   #define mc2dbg1 mc2log
   #define mc2dbg2 mc2log
   #define mc2dbg4 mc2log
   #define mc2dbg8 mc2log
#endif
   #undef DEBUG_LEVEL_1
   #undef DEBUG_LEVEL_2
   #undef DEBUG_LEVEL_4
   #undef DEBUG_LEVEL_8
   // the debug stream
   #ifdef DEBUG_LEVEL_1
   #   define mc2dbg mc2log
   #   define mc2dbg1 mc2log
   #else
   #   define mc2dbg if (false) mc2log
   #   define mc2dbg1 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_2
   #   define mc2dbg2 mc2log
   #else
   #   define mc2dbg2 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_4
   #   define mc2dbg4 mc2log
   #else
   #   define mc2dbg4 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_8
   #   define mc2dbg8 mc2log
   #else
   #   define mc2dbg8 if (false) mc2log
   #endif

   #   define mc2log if (false) mc2log_real

   // manipulator that chooses info level logging
   inline ostream & info (ostream & os) {return os;}

   // manipulator that chooses warning level logging
   inline ostream & warn (ostream & os) {return os;}

   // manipulator that chooses error level logging
   inline ostream & error (ostream & os) { return os;}

   // manipulator that chooses fatal level logging
   inline ostream & fatal (ostream & os) {return os;}

   // small macro that outputs the filename and line of the current line of code
   // when used on an ostream.
   #define here __FILE__ ":" << __LINE__

   // macro to output an IP numerically
   #define prettyPrintIP(a) int((a & 0xff000000) >> 24) << "." \
                         << int((a & 0x00ff0000) >> 16) << "." \
                         << int((a & 0x0000ff00) >>  8) << "." \
                         << int (a & 0x000000ff)

   // macro to print citation marks before and after a
   #define MC2CITE(a) '"' << (a) << '"'

   // empty debug macros defines for Symbian
   #define   DEBUG8(a)
   #define   DEBUG4(a)
   #define   DEBUG2(a)
   #define   DEBUG1(a)
   #define   DEBUG_DB(a)
   #define   DEBUG_DEL(a)
   #define   DEBUG_CM(a)
   #define   DEBUG_GDF(a)

   // empty macro definitions
   #define hex 'H'
   #define dec 'D'

#elif !defined( __unix__ )

   // #include <malloc.h>
   #include <iostream>
   using namespace std;

   #undef DEBUG_LEVEL_1
   #undef DEBUG_LEVEL_2
   #undef DEBUG_LEVEL_4
   #undef DEBUG_LEVEL_8
   extern ostream& mc2log;

   #define MINFO(a) 

   // the debug stream
   #ifdef DEBUG_LEVEL_1
   #   define mc2dbg mc2log
   #   define mc2dbg1 mc2log
   #else
   #   define mc2dbg if (false) mc2log
   #   define mc2dbg1 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_2
   #   define mc2dbg2 mc2log
   #else
   #   define mc2dbg2 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_4
   #   define mc2dbg4 mc2log
   #else
   #   define mc2dbg4 if (false) mc2log
   #endif
   #ifdef DEBUG_LEVEL_8
   #   define mc2dbg8 mc2log
   #else
   #   define mc2dbg8 if (false) mc2log
   #endif

   // manipulator that chooses info level logging
   ostream & info (ostream & os);

   // manipulator that chooses warning level logging
   ostream & warn (ostream & os);

   // manipulator that chooses error level logging
   ostream & error (ostream & os);

   // manipulator that chooses fatal level logging
   ostream & fatal (ostream & os);

   // small macro that outputs the filename and line of the current line of code
   // when used on an ostream.
   #define here __FILE__ ":" << __LINE__

   // macro to output an IP numerically
   #define prettyPrintIP(a) int((a & 0xff000000) >> 24) << "." \
                         << int((a & 0x00ff0000) >> 16) << "." \
                         << int((a & 0x0000ff00) >>  8) << "." \
                         << int (a & 0x000000ff)

   // macro to output an map id in hex
   #define prettyMapIDFill(a) "0x" << setw(8) << setfill('0') << hex << a << dec
   #define prettyMapID(a) "0x" << hex << a << dec

   // macro to print citation marks before and after a
   #define MC2CITE(a) '"' << (a) << '"'

 #else

 // Unix
// #include <malloc.h>
#include <iostream>
#include <iomanip>
using namespace std;

/**
 *   MC2 logging helper class
 *
 */
class MC2Logging
{
   public:

      /**
        *   Creates a log ostream connected to another ostream (eg cerr)
        *   via LogBuffer
        *   @param outStream The ostream to chain the output to
        *   @return reference to the created ostream
        */
      static ostream& createLogStream(ostream& outStream);

      /**
        *   Creates a log ostream connected to a file descriptor
        *   via LogBuffer
        *   @param outFD The file descriptor to send the output to
        *   @return reference to the created ostream
        */
      static ostream& createLogStream(int outFD);

      /**
        *   Deletes all LogHandlers attached to a logging ostream
        *   @param logStream The stream to delete LogHandlers from
        */
      static void deleteHandlers(ostream& logStream);

      /**
        *   Turn debugging on/off
        *   @param debug If true debugging is turned on, if false all debugging
        *                output is sent to /dev/null
        */
      static void debugOutput(bool debug);
};

/**
 *   Abstract superclass for a LogHandler that's derived to build
 *   external logging features (eg syslog, SNMP traps).
 *
 */
class LogHandler
{
   public:

   virtual ~LogHandler() {}
   
      /**
        *   handleMessage is called with a new log entry (level and text)
        *   @param level The loglevel, see LogBuffer
        *   @param msg   Pointer to the log message
        *   @param msgLen Length of the log message
        *   @param levelStr String representation of the log level
        *   @param timeStamp String representation of the time stamp
        */
      virtual void handleMessage(int level, const char* msg, int msgLen,
                                 const char* levelStr, 
                                 const char* timeStamp) = 0;
};

/**
 *   An iostream streambuf that prefixes each new line with the
 *   date and time and handles other things related to MC2 logging.
 *
 */
class LogBuffer: public streambuf
{
   public:
      /**
        *   Initializes the LogBuffer, selects where output is sent.
        *   The streambuf passed is not deleted by the destructor!
        *   @param buf The streambuf to output to
        */
      LogBuffer(streambuf *buf);

      /**
        *   Initializes the LogBuffer, selects where output is sent.
        *   @param outFD The file descriptor to output to
        */
      LogBuffer(int outFD);

      /**   Initializes the LogBuffer, selects where output is sent.
        *   @param outBuf The LogBuffer (from ::mc2log usually)
        *                 to use for daisy-chaining of loghandlers.
        */
      LogBuffer(LogBuffer* outBuf);

      /**
        *   Cleans up
        */
      virtual ~LogBuffer();

      /**
        *   Set an extra logging prefix
        *   @param prefix Pointer to prefix string
        */
      void setPrefix(char* prefix);

      /**
        *   Deletes all LogHandlers attached
        */
      void deleteHandlers();

      /**
        *   Add a handler
        *   @param level The log levels we want to be notified at
        *   @param handler Pointer to a LogHandler instance
        *   @return true if success
        */
      bool addHandler(int level, LogHandler* handler);

      /**
        *   Turn debugging on/off
        *   @param debug If true debugging is turned on, if false all debugging
        *                output is sent to /dev/null
        */
      void setDebug(bool debug);
      
      /**
        *   Used to output and notify any registered handlers from the outside
        *   @param level The loglevel
        *   @param msg   Pointer to the log message
        *   @param msgLen Length of the log message
        *   @param levelStr String representation of the log level
        *   @param timeStamp String representation of the time stamp
        */
      void notifyExternal(int level, const char* msg, int msgLen,
                          const char* levelStr, const char* timeStamp);

      /**
        *   The log levels
        */
      enum LOGLEVELS {
         LOGLEVEL_DEBUG = 1,
         LOGLEVEL_INFO  = 2,
         LOGLEVEL_WARN  = 4,
         LOGLEVEL_ERROR = 8,
         LOGLEVEL_FATAL = 16
      };

   protected:
      /**
        *  Notifies any registered handlers.
        */
      void notify();

      /**  
        *  Output to either the streambuf or the file descriptor
        *  associated.
        *  @param buf Pointer to the buffer
        *  @param len Size of the data to output
        *  @return Size of the data output (same as len if OK), otherwise
        *          it depends on whether streambuf::sputn() or write() was
        *          used internally.
        */
      int output(const char* buf, int len);

      /**
        *  Outputs the first half of the (date, time)
        *  @return 0 if OK, EOF if error
        */
      int putTime();

      /**
        *  Outputs the second half of the prefix (the level)
        *  @return 0 if OK, EOF if error
        */
      int putLevel();

      /**
        *  Called when there's a new character to output.
        *  @return ???
        */
      int overflow(int c);

      /**
        *  Called to read a character from the input, doesn't advance to
        *  the next character.
        *  @return ???
        */
      int underflow();

      /**
        *  Called to read a character from the input, advances to the
        *  next character.
        *  @return ???
        */
      int uflow();

      /**
        *  Called to sync internal state with external representation
        *  @return ???
        */
      int sync();

   private:

      /**
        * The streambuf we send our output to
        */
      streambuf* m_buf;

      /**
        * The file descriptor we send output to.
        */
      int m_fd;

      /**
        * Flag that's set to true when encounter a newline
        */
      bool m_newline;

      /**
        * Flag that's set when we're doing magic (handling our special
        * manipulators (info, warn, err)
        */
      bool m_magic;

      /**
        * The log level we're using
        */
      int m_level;

      /**
        * Our character buffer, needed for the Notifiers.
        */
      char* m_buffer;

      /**
        * Last timestamp
        */
      char m_timeBuf[32];

      /**
        * Last level string
        */
      char m_levelBuf[32];

      /**
        * Current position in the buffer.
        */
      int m_bufPos;

      /**
        * Output debug text or not?
        */
      bool m_debug;

      /**
        * The prefix string
        */
      char* m_prefix;

      /**
        * Container for a LogHandler used with the simple linked lists
        * that the handlers are placed in.
        */
      class LogHandlerItem {
         public:
           
            /**
              *   Create and add to the list specified
              *   @param handler Pointer to the LogHandler instance
              *   @param list    Pointer to pointer to list
              */
            LogHandlerItem(LogHandler* handler, LogHandlerItem** list);
            
            LogHandler*     m_handler;
            LogHandlerItem* m_next;
      };

      /**
        *  Remove a loghandler list
        */
      void removeList(LogHandlerItem*& list);

      /**
        * Simple linked list for LogHandlers for level LOGLEVEL_DEBUG
        */
      LogHandlerItem* m_handlersDebug;

      /**
        * Simple linked list for LogHandlers for level LOGLEVEL_INFO
        */
      LogHandlerItem* m_handlersInfo;

      /**
        * Simple linked list for LogHandlers for level LOGLEVEL_WARN
        */
      LogHandlerItem* m_handlersWarn;

      /**
        * Simple linked list for LogHandlers for level LOGLEVEL_ERROR
        */
      LogHandlerItem* m_handlersError;

      /**
        * Simple linked list for LogHandlers for level LOGLEVEL_FATAL
        */
      LogHandlerItem* m_handlersFatal;

      /**
        * LogBuffer to use for daisy-chaining.
        */
      LogBuffer* m_externalBuffer;
};

// the logging stream
extern ostream& mc2log;

/*
extern struct mallinfo mi;

#define MINFO(a) mi = mallinfo(); mc2dbg << a << " minfo: " << mi.uordblks << endl;
*/

#define MINFO(a) 

// the debug stream
#ifdef DEBUG_LEVEL_1
#   define mc2dbg mc2log
#   define mc2dbg1 mc2log
#else
#   define mc2dbg if (false) mc2log
#   define mc2dbg1 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_2
#   define mc2dbg2 mc2log
#else
#   define mc2dbg2 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_4
#   define mc2dbg4 mc2log
#else
#   define mc2dbg4 if (false) mc2log
#endif
#ifdef DEBUG_LEVEL_8
#   define mc2dbg8 mc2log
#else
#   define mc2dbg8 if (false) mc2log
#endif

// manipulator that chooses info level logging
ostream & info (ostream & os);

// manipulator that chooses warning level logging
ostream & warn (ostream & os);

// manipulator that chooses error level logging
ostream & error (ostream & os);

// manipulator that chooses fatal level logging
ostream & fatal (ostream & os);

// small macro that outputs the filename and line of the current line of code
// when used on an ostream.
#define here __FILE__ ":" << __LINE__

// macro to output an IP numerically
#define prettyPrintIP(a) int((a & 0xff000000) >> 24) << "." \
                      << int((a & 0x00ff0000) >> 16) << "." \
                      << int((a & 0x0000ff00) >>  8) << "." \
                      << int (a & 0x000000ff)

// macro to output an map id in hex
#define prettyMapIDFill(a) "0x" << setw(8) << setfill('0') << hex << a << dec
#define prettyMapID(a) "0x" << hex << a << dec

// macro to print citation marks before and after a
#define MC2CITE(a) '"' << (a) << '"'

#endif

/// macro to print a hex number prefixed by 0x and then switch to dec again
#define MC2HEX(a) "0x" << hex << (a) << dec

#endif // MC2LOGGING_H
