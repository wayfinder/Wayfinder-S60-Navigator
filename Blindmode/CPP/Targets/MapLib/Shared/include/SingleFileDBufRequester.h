/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SINGLEFILEDBUFREQUESTER_HEDGE_FILE_H
#define SINGLEFILEDBUFREQUESTER_HEDGE_FILE_H

#include "config.h"

#include "DBufRequester.h"

// FIXME: Maybe put these in .cpp-file.
#include "SFDLoadableHeaderListener.h"
#include "SFDBufReaderListener.h"
#include "SFDSearcherListener.h"


#include<list>

// Helper classes for the queues.
namespace {
   struct releaseJob_t;
   struct readJob_t;
}

class FileHandler;
class SFDBufReader;
class SFDLoadableHeader;
class SFDSearcher;
class SingleFileDBufRequester;
class TileMapParams;

/**
 *   Callback if you want to know when the header has been loaded
 *   and the cache is ready.
 */
class SFDHeaderLoadedListener {
public:
   /// Called by the SFD when the header is loaded.
   virtual void headerLoaded( SingleFileDBufRequester* requester ) = 0;
   
};

/**
 *   Read only DBufRequester that uses a single file for
 *   its data.
 */
class SingleFileDBufRequester : public DBufRequester,
                                public SFDLoadableHeaderListener,
                                public SFDSearcherListener,
                                public SFDBufReaderListener {
public:
   
   /**
    *   Creates a new SingleFileDBufRequester.
    *   @param parent The parent requester.
    *   @param fh     The file handler to use inside. Will be deleted
    *                 when SingleFileDBufRequester is deleted.
    *   @param listener Object to call when the header has been loaded.
    */
   SingleFileDBufRequester( DBufRequester* parent,
                            FileHandler* fh,
                            SFDHeaderLoadedListener* listener = NULL );

   /// The destructor.
   ~SingleFileDBufRequester();

   /**
    *   Requests a buffer from this requester.
    */
   void request( const MC2SimpleString& descr,
                 DBufRequestListener* caller,
                 request_t whereFrom );

   /**
    *   Releases the buffer. Eats it if it belongs here.
    */
   void release( const MC2SimpleString& desc,
                 BitBuffer* buffer );

   /**
    *   Cancel all requests.
    */
   void cancelAll();

   // -- Callbacks

   /// Implements SFDLoadableHeaderListener
   void loadDone( SFDLoadableHeader* header,
                  FileHandler* fileHandler );

   /// Implements SFDSearcherListener
   void searcherDone( SFDSearcher* searcher,
                      uint32 strIdx );

   /// Implements SFDBufReaderListener
   void bufferRead( BitBuffer* buf );

   /// Implements MapLib::CacheInfo
   const char* getPathUTF8() const;
   /// Implements MapLib::CacheInfo
   const char* getNameUTF8() const;
   /// Implements MapLib::CacheInfo
   bool isValid() const;
   
   
private:
   
   /**
    *   Inner start. Return true if there are more work available.
    */
   bool innerStart();

   /**
    *   Examines the queues and starts working on something.
    */
   void start();

   /**
    *   Starts reading the header.
    */
   void startReadingHeader();

   /**
    *   Returns the offset of a tile multi buffer.
    *   @return -1 if it doesn't exist.
    */
   int getMultiBufferOffset( const TileMapParams& params ) const;

   /**
    *    Remove the first desc in the readqueue that is the same
    *    as the supplied desc.
    */
   void removeFromReadQueue( const MC2SimpleString& desc );

   /**
    *   Handles a multibuffer which contains more than one param
    *   with buffers.
    *   @param buf Current multibuffer.
    */
   void handleMultiBuffer( BitBuffer* buf );
   
   /// FileHandler
   FileHandler* m_fileHandler;
   /// FileHandler to delete
   FileHandler* m_fileHandlerToDelete;

   /// Possible states of this requester.
   enum state_t {
      /// Not initialized at all
      NOT_INITIALIZED,
      /// Reading the header
      READING_HEADER,
      /// Peek fitness condition
      IDLE,
      /// Using the SFDSearcher
      SEARCHING_FOR_READING,
      /// Reading the buffer.
      READING_BUFFER,
      /// Searching for string when releasing
      SEARCHING_FOR_RELEASING,
      /// Reading a multibuffer
      READING_MULTI,
      /// Permanent error has occurred.
      PERMANENT_ERROR,
   } m_state;

   /// Buffers to be released.
   std::list<releaseJob_t> m_releaseQueue;
   /// Buffers to be found.
   std::list<readJob_t> m_readQueue;
   /// Current read job
   readJob_t* m_curReadJob;
   /// Current release job
   releaseJob_t* m_curReleaseJob;

   /// The header
   SFDLoadableHeader* m_header;
   /// The searcher
   SFDSearcher* m_searcher;
   /// The buffer reader
   SFDBufReader* m_bufReader;

   /// Keeps track of how many times start() has been entered.
   int m_inStart;

   /// Listener to inform when header is loaded
   SFDHeaderLoadedListener* m_listener;
};

#endif
