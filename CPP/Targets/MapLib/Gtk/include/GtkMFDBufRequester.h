/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GTKMFDBUFREQUESTER_H
#define GTKMFDBUFREQUESTER_H

#include "config.h"
#include "UnixMFDBufRequester.h"

#include <gtk/gtk.h>

// -- GtkFileHandler

class GtkFileHandler : public UnixFileHandler {
public:
   /**
    *   Creates a filehandler that handles the supplied file.
    *   Should create the file if it is not there already.
    */
   GtkFileHandler( const char* fileName,
                   bool createFiles );

   ~GtkFileHandler();

   /**
    *   Reads maxLength bytes from the file.
    *   Calls the listener when done.
    */
   int read( uint8* bytes,
             int maxLength,
             FileHandlerListener* listener );

   /**
    *   Writes bytes to the file of the FileHandler.
    *   Calls the listener when it is done.
    */
   int write( const uint8* bytes,
              int length,
              FileHandlerListener* listener );
   
   /**
    *   Cancels outstanding reads/writes.
    */
   void cancel() {}
   
private:
   /**
    * The ID of the read idle event source.
    */
   guint m_readIdle;
   /**
    * The ID of the read idle event source.
    */
   guint m_writeIdle;

   /// True if an idle operation is active.
   int m_active;
   
   /// Called when read is done.
   static gboolean on_read_idle( gpointer data );
   /// Called when read is done.
   static gboolean on_write_idle( gpointer data );
};

// -- GtkMFDBufRequester

class GtkMFDBufRequester : public UnixMFDBufRequester {
public:
   GtkMFDBufRequester(DBufRequester* parent,
                      const char* path,
                      MemTracker* memTracker,
                      uint32 maxSize,
                      bool readOnly = false ) :
      UnixMFDBufRequester(parent,
                          path,
                          memTracker,
                          maxSize,
                          readOnly,
                          // And no init in the superclass.
                          false ) {
      init();
   }

   /// Create a FileHandler. initNow parameter is ignored.
   FileHandler* createFileHandler(const char* fileName,
                                  bool createIfNotExists,
                                  bool initNow ) {
      // Create new files only if told to.     
      return new GtkFileHandler( fileName, createIfNotExists );
   }

   const char* getPathSeparator() const {
      return "/";
   }
   
};

#endif

