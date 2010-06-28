/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef SHARED_LOG_H
#define SHARED_LOG_H
#include "arch.h"
#include <vector>
#include <algorithm>
#include <stdarg.h>

// #define MEM_LOG_FILTER

namespace isab{

   class Output;

   /** An instance of the Log class allows the logging of messages
    * with a certain prefix. Log messages have four levels: error,
    * warning, debug, and info. Messages to these levels can be turned
    * on and off individually. Messages logged with an instance of the
    * Log class can be directed to any number of ostreams. 
    */
   class Log 
   {
   public:
      /** ORable flags for permission settings. */
      enum levels{
         LOG_ERROR   = 1, /** The flag associated with error log messages.*/
         LOG_WARNING = 2, /** The flag associated with warning log messages.*/
         LOG_INFO    = 4, /** The flag associated with info log messages.*/ 
         LOG_DEBUG   = 8, /** The flag associated with debug log messages.*/ 
         LOG_ALL     = (LOG_ERROR | LOG_WARNING | 
                        LOG_INFO | LOG_DEBUG), /** All flags ORed together.*/
      };
      
      /** Type used for all calls takeing ORed levels as
          arguments. Can change to a larger type when needed. */
      typedef uint8 levels_t;

      /** Constuctor.
       * @param prefix for this Log object.
       * @param levels any number of the values from enum levels ORed 
       *               together. Default value LOG_ALL.
       */
      Log(const char* prefix, levels_t levels = LOG_ALL, 
          class LogMaster* master = NULL);

      /** Virtual destructor. */
      virtual ~Log();

      class LogMaster* getLogMaster();
      
      /** Gets the prefix. *
       * @return this Log objects prefix string.
       */
      const char* getPrefix() const;

      /** Gets the current level permissions for a prefix.
       * @return the current value of the level permissions for the
       *         specified prefix. The unsigned integer consists of zero or
       *         more of the constants LOG_ERROR, LOG_WARNING, LOG_INFO, 
       *         and LOG_DEBUG ORed
       *         together.  
       */
      levels_t getLevels();

      /** Sets new level permissions for this Log object.
       * @param levels the levels to allow from now on. OR together the
       *               constants LOG_ERROR, LOG_WARNING, LOG_INFO, and 
       *               LOG_DEBUG that corresponds to the levels you want. 
       *               Note that any levels not present in the call are 
       *               turned off. The default value of this argument is 
       *               <code>LOG_ALL</code>.
       * @return the old levels value.
       */
      levels_t setLevels(levels_t newLevels = LOG_ALL);

      /** Tests if a level is allowed to output.
       * @param level the level to check.
       * @return true if the output of a logging message is allowed for the
       *         specified level, false otherwise.
       */
      bool testLevels(levels_t levels);
      
      /** Logs an error message. Checks if the specifed prefix is allowed to
       * log error messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param ... enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int error(const char* format, ...) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
         __attribute__ ((format (printf, 2, 3)));;
#endif
      ;

      /** Logs an error message. Checks if the specifed prefix is allowed to
       * log error messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int verror(const char* format, va_list args);

      /** Logs an error message. Checks if the specifed prefix is allowed to
       * log error messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int real_verror(const char* format, va_list args);

      /** Logs a warning message. Checks if the specifed prefix is allowed
       * to log warning messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param ... enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int warning(const char* format, ...) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
         __attribute__ ((format (printf, 2, 3)));
#endif
      ;

      /** Logs a warning message. Checks if the specifed prefix is allowed
       * to log warning messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int vwarning(const char* format, va_list args); 

      /** Logs a warning message. Checks if the specifed prefix is allowed
       * to log warning messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int real_vwarning(const char* format, va_list args); 

      /** Logs an info message. Checks if the specifed prefix is allowed to
       * log info messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param ... enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int info(const char* format, ...) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
         __attribute__ ((format (printf, 2, 3)));
#endif
      ;

      /** Logs an info message. Checks if the specifed prefix is allowed to
       * log info messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int vinfo(const char* format, va_list args); 

      /** Logs an info message. Checks if the specifed prefix is allowed to
       * log info messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int real_vinfo(const char* format, va_list args); 

      /** Logs a debug message. Checks if the specifed prefix is allowed to
       * log debug messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param ... enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int debug(const char* format, ...) 
#if !defined(_MSC_VER) && !defined(__MWERKS__)
         __attribute__ ((format (printf, 2, 3)));
#endif
      ;

      /** Logs a debug message. Checks if the specifed prefix is allowed to
       * log debug messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int vdebug(const char* format, va_list args);

      /** Logs a debug message. Checks if the specifed prefix is allowed to
       * log debug messages.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param args enough arguments to match the format string.
       * @return the number of characters printed.
       */
      int real_vdebug(const char* format, va_list args);

      /** Drops a message.
       * @param prefix the prefix to log this message under.
       * @param format a printf-style format string that constitutes the log
       *               message. 
       * @param ... enough arguments to match the format string.
       * @return the number of characters printed.
       */
      static int dropIt(const char* format, ...);

      /** Dumps formatted data as an error message.
       * @param logMsg  the header message for the dump.
       * @param data    a pointer to the data to be dumped.
       * @param dataLen the number of bytes to dump.
       * @param base    the base to print data in. Defaults to 16.
       * @param group   how many bytes to print in a group. Defaults to 1.
       * @param row     how many groups to print on each row. Defaults to 8.
       * @return the number of characters written, not counting terminating
       *         zero bytes.
       */
      int errorDump(const char* logMsg, 
                    const uint8* data, size_t dataLen, 
                    int base = 16, int group = 1, int row = 8 );

      /** Dumps formatted data as a warnig message.
       * @param logMsg  the header message for the dump.
       * @param data    a pointer to the data to be dumped.
       * @param dataLen the number of bytes to dump.
       * @param base    the base to print data in. Defaults to 16.
       * @param group   how many bytes to print in a group. Defaults to 1.
       * @param row     how many groups to print on each row. Defaults to 8.
       * @return the number of characters written, not counting terminating
       *         zero bytes.
       */
      int warningDump(const char* logMsg,
                      const uint8* data, size_t dataLen, 
                      int base = 16, int group = 1, int row = 8 );

      /** Dumps formatted data as a info message.
       * @param logMsg  the header message for the dump.
       * @param data    a pointer to the data to be dumped.
       * @param dataLen the number of bytes to dump.
       * @param base    the base to print data in. Defaults to 16.
       * @param group   how many bytes to print in a group. Defaults to 1.
       * @param row     how many groups to print on each row. Defaults to 8.
       * @return the number of characters written, not counting terminating
       *         zero bytes.
       */
      int infoDump(const char* logMsg, 
                   const uint8* data, size_t dataLen, 
                   int base = 16, int group = 1, int row = 8 );


      /** Dumps formatted data as a debug message.
       * @param logMsg  the header message for the dump.
       * @param data    a pointer to the data to be dumped.
       * @param dataLen the number of bytes to dump.
       * @param base    the base to print data in. Defaults to 16.
       * @param group   how many bytes to print in a group. Defaults to 1.
       * @param row     how many groups to print on each row. Defaults to 8.
       * @return the number of characters written, not counting terminating
       *         zero bytes.
       */
      int debugDump(const char* logMsg, 
                    const uint8* data, size_t dataLen, 
                    int base = 16, int group = 1, int row = 8 );


      /** Modes to open file in. */
      enum FileMode {
         APPEND, /** Append log messages at the end of file. */
         REPLACE, /** Replace file. */
      };

      /** Add an output channel to this Log object.
       * @param target the output channel specified as a string.
       *               The following channel are valid for all platforms:
       *               "tcp:<hostname>:<port>" connects to a host.
       *               "tcp:<port>" listens for connections on a port.
       *               "file:<filename>" uses a file.
       *               i386-redhat-linux handles
       *               "cerr","cout", and "clog" adds the specified stream as
       *               an output channel.
       *               pocketpc handles
       *               "trace" adds the debug trace as an outout channel.
       * @param mode   a filemode used if files are specified. 
       *               Currently ignored.
       * @param return true if a channel was added.
       */
      bool addOutput(const char* target, enum FileMode mode = APPEND);
      
      /** Removes an output channel from this log object.
       * @param target removes the specified channel. The channel must have 
       *               been added with the exact same string.
       */
      void removeOutput(const char* target);


   private:
      /** Lock. */
      class RecursiveMutex* m_mutex;

      /** Permission levels for this Log instance. */
      levels_t m_levels;

      /** Prefix for this object. Shall be a local copy. */
      char* m_prefix;

      /** List of outputs.*/
      std::vector<class Output*> m_outputs;

      /** Pointer to LogMaster.*/
      class LogMaster* m_master;

      /** The stamp containing time, level, and prefix. Prepended to each
          output. If this is changed, make sure the conversion specifiers
          come in the right order: {%d, %s, %s}.*/
      static const char* const m_stampFormat;
      /** Holds the string "ERROR". */
      static const char* const m_error;
      /** Holds the string "WARNING". */
      static const char* const m_warning;
      /** Holds the string "INFO". */
      static const char* const m_info;
      /** Holds the string "DEBUG". */
      static const char* const m_debug;

      /** Internal log function called by error, warning, info, and
       * debug. Does the formatting and outputting of the log message.
       * @param logMsg the format string inputted as an argument to error,
       *               warning, info, or debug. Follows the printf format
       *               specifier specification. 
       * @param level  the level as a c-string.
       * @param args   a va_list containing all the arguments from error,
       *               warning, info, or debug. va_start must already have
       *               been called on args. 
       * @return       the number of characters printed.
       */
      int log(const char* logMsg, const char* level, va_list args);
      
      /** Internal function for printing data dumps. 
       * 
       * @param logMsg  the header message for the dump.
       * @param level   the error level as a c-string.
       * @param data    a pointer to the data to be dumped.
       * @param dataLen the number of bytes to dump.
       * @param base    the base to print data in. Defaults to 16.
       * @param group   how many bytes to print in a group. Defaults to 1.
       * @param row     how many groups to print on each row. Defaults to 8.
       * @return the number of characters written, not counting terminating
       *         zero bytes.
       */
      int dump(const char* logMsg, const char* level, 
               const uint8* data, size_t dataLen, 
               int base = 16, int group = 1, int row = 8 );

      /** Prints a string to all registered outputs.
       * @param msg the string to print.
       */
      void output(const char* msg);

      /** Locks the mutex. Blocks the calling thread if another
       * process has already locked this lock.*/
      void lock();
      /** Unlocks th mutex.*/
      void unlock();
   };


   /** Singleton class used to hold data all Log objects need to
       share. Also allows some operations on all Log objects. The
       LogMaster object administers all file access, allowing more
       than one Log object to write to the same file. */
   class LogMaster
   {
   public:
      /** Since all constructors are private, this is the only way to get 
       * to the singleton LogMaster object. 
       * Not true anymore... 
       * @return a reference to the LogMaster object.
       */
      static LogMaster& getLogMaster();

      /** Gets the default output, and returns the target string in the 
       * arguments.
       * @param target a pointer to a memory area the target string will
       *               be written into.
       * @param n      the max number of bytes allowed to write into target.
       * @return the target string. */
      class Output* getDefaultOutput(char* target, size_t n);

      /** Gets the default output, and registers a log object to it 
       * if necessary.
       * @param user the Log object that will use this defaultOutput.
       *             It is allowed to call getDefaultOutput with user
       *             set to NULL, but it's important to use a correct 
       *             argument to avoid premature resource deallocation.
       *             Therefore, no default argument is given.
       * @return the current value of defaultOutput. */
      class Output* getDefaultOutput(Log* user);

      /** Sets a file as the new default output. Only prefixes that
       * haven't made any logging calls yet are affected. All prefixes
       * that alread outputs to the old defaultOutput will continue to
       * do so. Note that this is the only way to make a file the
       * default output if not used by some Log object already.
       * @param filename the name of the file to use as default output.
       * @return the previous value of defaultOutput.
       */
      class Output* setDefaultOutput(const char* target);

      /** Finds or creates a Output object for the specified target.
       * See Log::addOutput for what target strings are legal.
       * @param target the kind of output wanted.
       * @param user   the Log object that wants to use the Output.
       * @return an Output object associated with the target string.
       */
      class Output* getOutput(const char* target, Log* user);

      /** Signals to the LogMaster that a Log object wants to release the
       * Output object specified by the target string.
       * @param target the target string associated with the Output object.
       * @param user   the Log object that wants to release th Output.
       */
      void releaseOutput(const char* target, Log* user);
      /** Signale to LogMaster that a Log object wants to release an output.
       * @param op the Output to release.
       * @param user the releasing Log object.
       */
      void releaseOutput(class Output* op, Log* user);

      /** Tests if the global permission levels allows logging of
       * specified levels. 
       * @param level the ORed levels to test against. 
       * @return true if all specified levels are allowed, false
       *         otherwise. 
       */
      inline bool testSuperLevels(Log::levels_t level);

      /** Sets the global levels. 
       * @param level ORed permission flags from Log specifying what
       *              levels will be allowed from now on. Default
       *              value LOG_ALL.
       * @return the old permissions. 
       */
      Log::levels_t setSuperLevels(Log::levels_t level = Log::LOG_ALL);

      /** Gets the current global permission levels. 
       * @return the currently allowed levels.
       */
      Log::levels_t getSuperLevels();

      /** Registers a Log object with the LogMaster.
       * @param log the log object to register.
       * @return the allowed levels from LogMaster.
       */
      Log::levels_t registerLog(Log* log);

      /** Unregisters a Log object.  
       * @param log the Log object to deregister.  
       * @return the number of log objects that still are registered.*/
      int unregisterLog(Log* log);

      /** The default constructor. */
      LogMaster(const char *default_path = "");

      /** Virtual destructor. */
      virtual ~LogMaster();
      
   private:
      void readLogLevels();
      void appendLogLevels();
      /** Locks the mutex. Blocks the calling thread if another
       * process has already locked this lock.*/
      void lock();
      /** Unlocks th mutex.*/
      void unlock();
      

      /** Ties together one Output object, one target string and any
       * number of Log objects.*/
      class OutputRecord{
      public:
         /** Constructor. 
          * @param Target    the string used to identify this Record.
          * @param theOutput the Output object matching the target string.
          * @param first     the first Log object to use the output. If NULL
          *                  the Record is marked as default output.
          */
         OutputRecord(const char* Target, class Output* theOutput, Log* first);

         /** Destructor*/
         ~OutputRecord();

         /** Adds a new user to this OutputRecord. If the Log object is
          * already listed, nothing happens.
          * @param user the Log to add. 
          */
         void addUser(Log* user);

         /** Removes a user from this OutputRecord. If the Log object wasn't
          * found, nothing happens.
          * @param user the Log to remove.
          */
         void removeUser(Log* user);

         /** The string that uniquely identifies this OutputRecord. */
         char* target;

         /** The Output object.*/
         class Output* output;

         /** The Log objects that use the Output.*/
         std::vector<Log*> users;

         /** Specifies if thie Record is the default record.  If so,
          * it shall not be removed even if no Logs use it at the
          * moment.*/
         bool isDefault;
      private:
         /** Finds a Log in the list.
          * @param user the user to find.
          * @param an iterator that refers to the Log pointer.
          */
         std::vector<Log*>::iterator findUser(Log* user);
      };

      /** Finds a record in the records list.
       * @param target the string that uniquely identifies an OutputRecord.
       * @retrun a pointer to the OutputRecord.
       */
      OutputRecord* findRecord(const char* target);

      ///Lock.
      class RecursiveMutex* m_mutex;
      /** A ostream that just ignores any data written to it. Since it
          is created with a NULL argument to the constructor, it's
          always in bad state. */
      class NullOutput* m_nullOutput;

      /** The default output stream, used as a starting point for all
          the Log objects. */
      class Output* m_defaultOutput;

      /** Global permissions. If any level is turned off here, no Log
          object will be able to Log messages of that level. */
      Log::levels_t m_superLevels;

      /** A list (vector) of OutputRecords. */
      std::vector<OutputRecord*> m_records;
      
      /** A list of Log objects. Used to determine when the LogMaster
       * object should be deallocated. Meant to be used for a Log
       * shell to control all Log output.*/
      std::vector<Log*> m_registeredLogs;

      /***/
      class PrefixLevels {
      public:
         PrefixLevels(const char* prf, Log::levels_t lvl = Log::LOG_ALL,
                      bool unknown = false);
         PrefixLevels(const PrefixLevels& from);
         PrefixLevels& operator=(const PrefixLevels& rhs);
         ~PrefixLevels() { delete[] prefix; }
         char*   prefix;
         Log::levels_t levels;
         bool unknown;
         static int compare(const void*, const void*);
         bool operator==(const PrefixLevels& rhs) const;
         bool operator<(const PrefixLevels& rhs) const;
      };
     
      std::vector<PrefixLevels> m_prefixLevelsV;
      char* m_logLevelsFilename;

      char* m_defaultPath;
   };




   //=============================================================
   //======= Inline for Log  =====================================

   inline const char* Log::getPrefix() const
   {
      return m_prefix;
   }

   inline bool Log::testLevels(levels_t levels){
      levels_t perms = getLevels() & levels;
      return perms != 0;
   }

   inline int Log::error(const char* format, ...) 
   {
#ifndef NO_LOG_OUTPUT
      int ret;
      va_list args;
      va_start(args, format);
      ret=real_verror(format, args);
      va_end(args);
      return ret;
#else
      format = format; // To remove warnings
      return 0;
#endif
   }

   inline int Log::verror(const char* format, va_list args) 
   {
#ifndef NO_LOG_OUTPUT
      return real_verror(format, args);
#else
      format = format; // To remove warnings
      args = args;
      return 0;
#endif
   }

   inline int Log::warning(const char* format, ...) 
   {
#ifndef NO_LOG_OUTPUT
      int ret;
      va_list args;
      va_start(args, format);
      ret=real_vwarning(format, args);
      va_end(args);
      return ret;
#else
      format = format; // To remove warnings
      return 0;
#endif
   }

   inline int Log::vwarning(const char* format, va_list args) 
   {
#ifndef NO_LOG_OUTPUT
      return real_vwarning(format, args);
#else
      format = format; // To remove warnings
      args = args;
      return 0;
#endif
   }

   inline int Log::info(const char* format, ...) 
   {
#ifndef NO_LOG_OUTPUT
      int ret;
      va_list args;
      va_start(args, format);
      ret=real_vinfo(format, args);
      va_end(args);
      return ret;
#else
      format = format; // To remove warnings
      return 0;
#endif
   }

   inline int Log::vinfo(const char* format, va_list args) 
   {
#ifndef NO_LOG_OUTPUT
      return real_vinfo(format, args);
#else
      format = format; // To remove warnings
      args = args;
      return 0;
#endif
   }

   inline int Log::debug(const char* format, ...) 
   {
#ifndef NO_LOG_OUTPUT
      int ret;
      va_list args;
      va_start(args, format);
      ret=real_vdebug(format, args);
      va_end(args);
      return ret;
#else
      format = format; // To remove warnings
      return 0;
#endif
   }

   inline int Log::vdebug(const char* format, va_list args) 
   {
#ifndef NO_LOG_OUTPUT
      return real_vdebug(format, args);
#else
      format = format; // To remove warnings
      args = args;
      return 0;
#endif
   }

   inline int Log::dropIt(const char* format, ...) 
   {
      format = format;
      return 0;
   }

}
#endif
