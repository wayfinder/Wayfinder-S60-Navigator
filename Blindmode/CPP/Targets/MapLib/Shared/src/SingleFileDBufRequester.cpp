/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "SingleFileDBufRequester.h"

#include "FileHandler.h"
#include "MC2SimpleString.h"
#include "SFDLoadableHeader.h"
#include "SFDSearcher.h"
#include "SFDBufReader.h"
#include "SFDMultiBufferReader.h"
#include "TileMapParams.h"
#include "TileMapParamTypes.h"
#include <algorithm>

using namespace std;

namespace {
   struct releaseJob_t {
      releaseJob_t( const MC2SimpleString& str,
                    BitBuffer* buf ) : m_str(str),
                                       m_buf(buf) {}

      /// Needed by STLPort 4.5.1
      releaseJob_t() {}

      MC2SimpleString m_str;
      BitBuffer*      m_buf;      
   };

   struct readJob_t {
      readJob_t( const MC2SimpleString& descr,
                 DBufRequestListener* caller,
                 DBufRequester::request_t whereFrom )
            : m_str( descr ),
              m_caller( caller ),
              m_whereFrom( whereFrom ) {}

      /// Needed by STLPort 4.5.1
      readJob_t() {}

      /// Equals operator, du.
      int operator==( const readJob_t& other ) const {
         return m_caller == other.m_caller &&
            m_whereFrom == other.m_whereFrom &&
            m_str == other.m_str;
      }

      void callListener( BitBuffer* buf, DBufRequester& origin ) {
         // Make temporary string since this can be deleted while
         // sending. Scary.
         MC2SimpleString tmp(m_str);         
         m_caller->requestReceived( tmp, buf, origin );
      }

      /// Special version where we send some stuff not yet requested.
      void callListener( const MC2SimpleString& str,
                         BitBuffer* buf,
                         DBufRequester& origin ) {
         m_caller->requestReceived( str, buf, origin );
      }
      
      MC2SimpleString m_str;
      DBufRequestListener* m_caller;
      DBufRequester::request_t m_whereFrom;
   };
}

SingleFileDBufRequester::SingleFileDBufRequester( DBufRequester* parent,
                                                  FileHandler* fh,
                                                  SFDHeaderLoadedListener* l )
      : DBufRequester( parent )
{
   m_listener = l;
   /// We made Amiga, they f*cked it up.
   m_state = NOT_INITIALIZED;
   m_requesterType = PERMANENT | READ_ONLY;
   m_fileHandler = fh;
   m_fileHandlerToDelete = fh;
   // FIXME: Add the XorBuffers here.
   m_header = new SFDLoadableHeader(NULL, NULL);
   m_curReadJob = new readJob_t( "", NULL, cacheOrInternet );
   m_curReleaseJob = new releaseJob_t( "", NULL );
   m_searcher = NULL;
   m_bufReader = NULL;
   m_inStart = 0;

   // Read header
   startReadingHeader();
}

SingleFileDBufRequester::~SingleFileDBufRequester()
{
   delete m_fileHandlerToDelete;
   delete m_header;
   delete m_searcher;
   delete m_bufReader;
   delete m_curReadJob;
   delete m_curReleaseJob;
}

bool
SingleFileDBufRequester::isValid() const
{
   return m_state != PERMANENT_ERROR;
}

void
SingleFileDBufRequester::cancelAll()
{
   // Lägg märke till!
   m_readQueue.clear();
   // The parent too!
   DBufRequester::cancelAll();
}

void
SingleFileDBufRequester::request( const MC2SimpleString& descr,
                                  DBufRequestListener* caller,
                                  request_t whereFrom )
{
   if ( m_state == PERMANENT_ERROR ) {
      // We cannot recover
      DBufRequester::request( descr, caller, whereFrom );
      return;
   }
   if ( m_state != NOT_INITIALIZED &&
        m_state != READING_HEADER ) {
      if ( ! m_header->maybeInCache( descr ) ) {
         // This means that it cannot be in this cache so send it on.
         if ( m_parentRequester != NULL ) {
            m_parentRequester->request(descr, caller, whereFrom );
         }
         return;
      }
   }
   
   // Make job.
   readJob_t readJob( descr, caller, whereFrom );
   
   // Check if already there
   if ( std::find( m_readQueue.begin(), m_readQueue.end(), readJob ) ==
        m_readQueue.end() ) {
      m_readQueue.push_back( readJob );
   } else {
      // Will come later.     
   }
   if ( m_state == NOT_INITIALIZED ) {
      // Read header
      startReadingHeader();
      // Don't start working
      return;
   } else if ( m_state != READING_HEADER ) {
      // Will start working if not working already.
      start();
   }
}

void
SingleFileDBufRequester::release( const MC2SimpleString& desc,
                                  BitBuffer* buffer )
{
   if ( buffer == NULL ) {
      // Nothing to do.
      return;
   }
 
   if ( m_state == PERMANENT_ERROR ) {
      // Error - send on
      DBufRequester::release( desc, buffer );
      return;
   }
   
   if ( m_state == NOT_INITIALIZED ||
        m_state == READING_HEADER ) {
      // It cannot have come from here. Send to next.
      DBufRequester::release( desc, buffer );
   } else {
      // Check the offset and don't put it in the queue if it is
      // mine!!
      int multiBufferOffset = -1;
      if ( TileMapParamTypes::isMap( desc.c_str() ) ) {
         // Only do this for maps!
         multiBufferOffset = getMultiBufferOffset( desc );
      }
      if ( multiBufferOffset >= 0 ) {
         // It is mine!
         mc2dbg << "[SFDBR]: IT IS DEFINITELY MINE (1) !!!!" << endl;
         delete buffer;
      } else {
         if ( m_header->maybeInBinaryCache( desc ) ) {
            mc2dbg << "[SFDBR]: Could be mine : " << MC2CITE(desc) << endl;
            m_releaseQueue.push_back( releaseJob_t( desc, buffer ) );
         } else {
            mc2dbg << "[SFDBR]: Definitely not mine : " << MC2CITE(desc)
                   << endl;
            DBufRequester::release( desc, buffer );
         }
      }
   }
   
   start();
}

void
SingleFileDBufRequester::searcherDone( SFDSearcher*,
                                       uint32 strIdx )
{
   if ( m_state == SEARCHING_FOR_READING ) {
      if ( strIdx == MAX_UINT32 ) {
         // Not found.
         m_state = IDLE;
         if ( m_parentRequester ) {
            m_parentRequester->request( m_curReadJob->m_str,
                                        m_curReadJob->m_caller,
                                        m_curReadJob->m_whereFrom );
         } else {
            m_curReadJob->callListener( NULL, *this );
         }
         start();
         return;
      } else {
         // Found
         m_state = READING_BUFFER;
         m_bufReader->start( strIdx );
         return;
      }
   } else if ( m_state == SEARCHING_FOR_RELEASING ) {
      if ( strIdx == MAX_UINT32 ) {
         mc2dbg8 << "[SFDBR]: Released map not found in my cache" << endl;
         // Not found -> release to next
         DBufRequester::release( m_curReleaseJob->m_str,
                                 m_curReleaseJob->m_buf );
      } else {
         mc2dbg8 << "[SFDBR]: IT IS MINE" << endl;
         // Found here -> delete and forget
         delete m_curReleaseJob->m_buf;
      }
      m_curReleaseJob->m_buf = NULL;
      m_state = IDLE;
      // Start again if necessary.
      start();
   }
}

void
SingleFileDBufRequester::removeFromReadQueue( const MC2SimpleString& desc )
{
   list<readJob_t>::iterator it = m_readQueue.begin();
   while ( it != m_readQueue.end() ) {
      if ( (*it).m_str == desc ) {
         // Remove
         m_readQueue.erase( it );
         return;
      } 
      ++it;
   }
}


void
SingleFileDBufRequester::handleMultiBuffer( BitBuffer* buf )
{
   // read will delete the big buffer when it is done
   // Should maybe be called decoder.
   TileMapParams curParam( m_curReadJob->m_str );
   SFDMultiBufferReader reader( buf, curParam, m_header );

   // Now let's send all the wanted/unwanted stuff to the listener.
   // The small buffer will not be deleted by the reader so it is
   // ok to send it on.

   /* Well well, we need to see if this big multi buffer actually contains
    * the requested buffer. If not, we need to take action! */
   
   bool handledOriginalRequest = false;
   
   while ( reader.hasNext() ) {
      SFDMultiBufferReader::bufPair_t& curPair = 
         reader.readNext( m_preferredLang );
      // Ok since we have the reader on the stack.

      if(m_curReadJob->m_str == curPair.first) {
         handledOriginalRequest = true;
      }
      
      m_curReadJob->callListener( curPair.first, curPair.second, *this );
      // Also remove the incoming jobs that correspond to this
      // param.
      removeFromReadQueue( curPair.first ); 
   }

   /* The requested map was not found. It is probably an "empty" map,
    * which are appearantly not stored in the WFD file. Our only reasonable
    * option is to pass the request on to the parent requester. */
   
   if(!handledOriginalRequest) {
      m_parentRequester->request( m_curReadJob->m_str,
                                  m_curReadJob->m_caller,
                                  m_curReadJob->m_whereFrom );

   }
}

void
SingleFileDBufRequester::bufferRead( BitBuffer* buf )
{
   // These are the allowed states.
   MC2_ASSERT( m_state == READING_BUFFER ||
               m_state == READING_MULTI );
   
   // Call the listener
   state_t prevState = m_state;
   m_state = IDLE;
   if ( prevState == READING_BUFFER ) {
      m_curReadJob->callListener( buf, *this );
      // Check if we want to start.
   } else if ( prevState == READING_MULTI ) {
      // We got a buffer with lots of importances.
      handleMultiBuffer( buf );
   }

   // Start again if necessary.
   start();
}

int
SingleFileDBufRequester::
getMultiBufferOffset( const TileMapParams& params ) const
{
   return m_header->getMultiBufferOffsetOffset( params );
}

const char*
SingleFileDBufRequester::getNameUTF8() const
{
   if ( ( m_state == NOT_INITIALIZED ) || ( m_state == READING_HEADER ) ) {
      return "Cache not yet loaded";
   } else {
      return m_header->getName().c_str();
   }
}

const char*
SingleFileDBufRequester::getPathUTF8() const
{
   return m_fileHandler->getFileName();
}
void
SingleFileDBufRequester::start()
{
   if ( m_inStart ) {
      return;
   }
   ++m_inStart;
   while( innerStart() ) {}
   --m_inStart;
}

bool
SingleFileDBufRequester::innerStart()
{
   if ( m_state != IDLE ) {
      // Already doing something.
      return false;
   }
   // FIXME: Check the sizes of the queues and take the longest?
   if ( ! m_readQueue.empty() ) {
      // Read wanted - start the searcher.
      // Switch to swap when that is implemented properly
      //std::swap( *m_curReadJob, m_readQueue.front() );
      *m_curReadJob = m_readQueue.front();
      m_readQueue.pop_front();
      int multiBufferOffset = -1;
      if ( TileMapParamTypes::isMap( m_curReadJob->m_str.c_str() ) ) {
         // Only do this for maps!
         multiBufferOffset = getMultiBufferOffset( m_curReadJob->m_str );
      }
      if ( multiBufferOffset < 0 ) {
         // Not there - search for it using string search.
         // First letter already checked in ::request.
         m_state = SEARCHING_FOR_READING;
         m_searcher->searchFor( m_curReadJob->m_str );
      } else {
         m_state = READING_MULTI;
         m_bufReader->startAbsolute( multiBufferOffset );
      }
   } else if ( ! m_releaseQueue.empty() ) {
      // Switch to swap when that is implemented properly
      //std::swap( *m_curReleaseJob, m_releaseQueue.front() );
      *m_curReleaseJob = m_releaseQueue.front();
      m_releaseQueue.pop_front();
      
      // The ones in the queue should not be in the tile collection.
      
      m_state = SEARCHING_FOR_RELEASING;
      m_searcher->searchFor( m_curReleaseJob->m_str );
   }

   // Return if there are anything more to do. 
   // Next time innerStart is called it will check the state for idleness.
   return !m_readQueue.empty() || !m_releaseQueue.empty();
}

void
SingleFileDBufRequester::loadDone( SFDLoadableHeader* header,
                                   FileHandler* fileHandler )
{
   // Check if header loaded ok
   if ( ! header->isValid() ) {
      m_fileHandler = fileHandler;
      m_state = PERMANENT_ERROR;
   } else {
     // Change file handler
     m_fileHandler = fileHandler;
     m_state = IDLE;

     // Initialize the Searcher and buffer readers.
     m_searcher  = new SFDSearcher( *m_header, *m_fileHandler, *this );
     m_bufReader = new SFDBufReader( *m_header, *m_fileHandler, *this );

     // If we have a listener - inform it
     if ( m_listener ) {
        m_listener->headerLoaded( this );
     }
   }
    
   if ( m_state != PERMANENT_ERROR ) { 
      // Start if we have something to do.   
      start();
   }
}

void
SingleFileDBufRequester::startReadingHeader()
{
   m_state = READING_HEADER;
   m_header->load( m_fileHandler, this );   
}
