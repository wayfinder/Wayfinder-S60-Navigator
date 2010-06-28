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

#include "BitBuffer.h"
#include "SharedDBufRequester.h"
#include "HttpClientConnection.h"
#include "HttpClientConnectionListener.h"
#include "DBufConnection.h"
#include "TileMapUtil.h"

#include "TileMapToolkit.h"

#include <algorithm>
#include <vector>

#include <stdio.h>

#define NBR_POST_REQUESTS 10
// Number of requests to collect before sending without waiting
#define MIN_COLLECT       10
// Wait time in milliseconds to wait for more requests before sending.
#define COLLECT_WAIT_TIME 250

// Logging
#ifdef __WINS__
 #define BYTELOG_NAME "c:\\Media files\\other\\bytelog.txt"
#elif defined NAV2_CLIENT_UIQ3
 #define BYTELOG_NAME "d:\\other\\bytelog.txt"
#else
 #define BYTELOG_NAME "E:\\bytelog.txt"
#endif
#define WRITE_INTERVAL_MS ( 60 * 1000 )
// Undef BYTELOG_NAME to avoid the bytelog.
#undef  BYTELOG_NAME

using namespace std;

class SharedDBufRequesterListener : public HttpClientConnectionListener {
public:

   /**
    *
    */
   SharedDBufRequesterListener(SharedDBufRequester* requester);

   /**
    *   Dr. Destructo.
    */
   virtual ~SharedDBufRequesterListener();
   
   /**
    *   Tells the listener that there are bytes of the
    *   body
    *   @param requestNbr     The number of the request.
    *   @param httpStatusCode The http status code for the request.
    *   @param bytes          The bytes if any.
    *   @param startOffset    The start offset in the body, not in bytes.
    *   @param nbrBytes       The number of bytes in bytes.
    *   @param contentLength  The content length if known or 0 if unknown.
    *                         -1 if error. Can be used the first time to
    *                         initialize the buffer to store stuff in.
    */
   void bytesReceived(int requestNbr,
                      int httpStatusCode,
                      const byte* bytes,
                      int startOffset,
                      int nbrBytes,
                      int contentLength);
private:
   /// The requetser
   SharedDBufRequester* m_requester;
   
   /// Pointer to current buffer to write in.
   byte* m_curBuffer;
   /// Number of current request.
   int m_curRequest;
   /// Number requests sent
   int m_nbrReq;
#ifdef BYTELOG_NAME
   /// Number of bytes received
   int m_nbrBytesRecv;
   /// Last time the number of bytes were written
   uint32 m_lastWrittenTime;
public:
   int getNbrBytesRecv() const {
      return m_nbrBytesRecv;
   }
#endif
};


SharedDBufRequesterListener::
SharedDBufRequesterListener(SharedDBufRequester* requester)
{
   m_requester  = requester;
   m_curBuffer  = NULL;
   m_curRequest = -1;
   m_nbrReq = 0;
#ifdef BYTELOG_NAME
   m_nbrBytesRecv = 0;
   m_lastWrittenTime = 0;
#endif
}

SharedDBufRequesterListener::
~SharedDBufRequesterListener()
{
   delete [] m_curBuffer;
}

void
SharedDBufRequesterListener::bytesReceived(int requestNbr,
                                               int httpStatusCode,
                                               const byte* bytes,
                                               int startOffset,
                                               int nbrBytes,
                                               int contentLength)
{
#ifdef BYTELOG_NAME
   if ( nbrBytes > 0 ) {
      m_nbrBytesRecv += nbrBytes;
      const uint32 timeDiff =
         TileMapUtil::currentTimeMillis() - m_lastWrittenTime;
      if ( timeDiff > ( WRITE_INTERVAL_MS ) ) {
         FILE* fil = fopen(BYTELOG_NAME, "a");
         if ( fil ) {
            fprintf(fil, "[SHTTP]: tot rec = %d\n", m_nbrBytesRecv);
            fclose(fil);
         }
         m_lastWrittenTime = TileMapUtil::currentTimeMillis();
      }
   }
#endif

   mc2log << "[SHTTP]::bytesReceived" << endl;
#undef FAIL_SOME
#ifdef FAIL_SOME
   if ( startOffset + nbrBytes == contentLength ) {
      if ( ++m_nbrReq % 10 == 0 ) {
         mc2log << "[SHTTP]: Faking failed request" << endl;
         startOffset = -1;
      }
   }
#endif
   
   if ( requestNbr != m_curRequest ) {
      if ( m_curRequest > 0 ) {
         // Wrong request number. Fail!
         m_requester->bufferReceived(m_curRequest, NULL);
         delete [] m_curBuffer;
         m_curBuffer = NULL;
      } else {
         // No current request. Good.
      }
      m_curRequest = requestNbr;
   }
   if ( ( startOffset < 0 ) ||
        ( nbrBytes < 0 ) ||
        ( contentLength < 0 ) ) {
      // I guess we will have to fail again
      m_requester->bufferReceived(m_curRequest, NULL);
      m_curRequest = -1;
      delete [] m_curBuffer;
      m_curBuffer = NULL;
      return;
   }

   // Seems ok
   if ( m_curBuffer == NULL ) {
      mc2dbg << "[SHTTPDBUF]: Creating new buffer for "
             << m_curRequest << " with size " << contentLength << endl;
      m_curBuffer = new byte[contentLength];
   }
   // FIXME: Check that all bytes have been received.
   memcpy(m_curBuffer + startOffset, bytes, nbrBytes);
   
   // Check for completion
   if ( startOffset+nbrBytes == contentLength ) {
      mc2dbg << "[SHTTPDBUF]: Completed one buffer " << requestNbr
             << " at " << startOffset << " nbr = "
             << nbrBytes << " with length "
             << contentLength << endl;
      BitBuffer* bitBuffer = new BitBuffer(m_curBuffer, contentLength);
      bitBuffer->setSelfAlloc(true);
      m_curBuffer = NULL;
      // Quick fix for strange status codes.
      if ( ( httpStatusCode == 200 ) && ( contentLength > 0 ) ) {
         m_requester->bufferReceived(m_curRequest, bitBuffer);
         m_curRequest = -1;            
      } else {
         mc2dbg << "[SHTTPDBUF]: Status code is " << httpStatusCode
                << " sending NULL to listener" << endl;
         m_requester->bufferReceived(m_curRequest, NULL);
         m_curRequest = -1;
         delete bitBuffer;
      }
   } else if ( (startOffset+nbrBytes) > contentLength ) {
      mc2log << error << "[SHTTPDBUF]: Too much data for number "
             << m_curRequest << " contentlength = "
             << contentLength << " datasize = " << (startOffset+nbrBytes)
             << endl;
      m_requester->bufferReceived(m_curRequest, NULL);
      m_curRequest = -1;
      delete [] m_curBuffer;
      m_curBuffer = NULL;
   }   
}

// -----------------------------------------------

SharedDBufRequester::
SharedDBufRequester(DBufConnection* clientConnection,
                    TileMapToolkit* toolkit )
{
   m_requesterType = EXTERNAL;
   m_filter = NULL;
   m_toolkit = toolkit;
   m_timerID = 0;
#ifdef BYTELOG_NAME
   FILE* fil = fopen(BYTELOG_NAME, "a");
   if ( fil ) {
      fprintf(fil, "[SHTTP]: START\n");
      fclose(fil);
   }
#endif
   m_nbrOutstanding = 0;
   m_clientConnection = clientConnection;   
   //
   m_listener = new SharedDBufRequesterListener(this);
   m_clientConnection->setListener(m_listener);
   m_curRecentRecv = 0;
   m_nbrBytesSent = 0;
   m_timerListener =
      new TileMapTimerListenerTemplate<SharedDBufRequester>(this);
}

SharedDBufRequester::~SharedDBufRequester()
{
   if ( m_toolkit && ( m_timerID != 0 ) ) {
      m_toolkit->cancelTimer( m_timerListener, m_timerID );
      delete m_timerListener;
   }
#ifdef BYTELOG_NAME
   FILE* fil = fopen( BYTELOG_NAME, "a");
   if ( fil ) {
      fprintf(fil, "[SHTTP]: tot sent final = %d\n", m_nbrBytesSent);
      if ( m_listener ) {
         fprintf(fil, "[SHTTP]: tot rec  final = %d\n",
                 m_listener->getNbrBytesRecv() );
      }
      fprintf(fil, "[SHTTP]: STOP\n");
      fclose(fil);
   }
#endif
   if ( m_clientConnection ) {
      m_clientConnection->setListener(NULL);
   }
   //delete m_listener;
}

class StringTimeFinder {
public:
   StringTimeFinder(const MC2SimpleString& toLookFor)
         : m_toLookFor(toLookFor) {}
   
   int operator()(const pair<MC2SimpleString, uint32>& curpair) {
      return curpair.first == m_toLookFor;
   }

   const MC2SimpleString& m_toLookFor;
   
};

bool
SharedDBufRequester::alreadySentOrPending(
   const pair<DBufRequestListener*, MC2SimpleString>& thePair )
{
   {
      pair<MC2SimpleString, uint32>* recentBegin = m_recentRecv;
      pair<MC2SimpleString, uint32>* recentEnd =
         m_recentRecv + m_nbrRecentRecv;
      pair<MC2SimpleString, uint32>* found =
         std::find_if( recentBegin,
                       recentEnd,
                       StringTimeFinder(thePair.second) );
      if ( found != recentEnd ) {
         // One of the m_nbrRecentRecv most recently received.
         int timeDiff = TileMapUtil::currentTimeMillis() - found->second;
         if ( timeDiff < 10000 ) {
            mc2log << "[SHTTP]: Too soon to re-request that map called "
                   << found->first << endl;
            return true;
         } else {
            mc2log << "[SHTTP]: Now it is ok to request that map called "
                   << found->first << endl;
            return false;
         }
      }
   }
   
    if ( m_sentRequests.find(thePair.second) != m_sentRequests.end() ) {
      // Already requested and sent.
      return true;
    }
    
    if ( std::find(m_pending.begin(), m_pending.end(),
                   thePair) != m_pending.end() ) {
       // Already requested and not sent.
       return true;
    }

    // Not sent already.
    return false;
}

void
SharedDBufRequester::setToolkit( TileMapToolkit* toolkit )
{
   m_toolkit = toolkit;
}

void
SharedDBufRequester::requestUsingPostTimer()
{
   if ( m_toolkit == NULL || m_pending.size() >= MIN_COLLECT ) {
      requestUsingPost();
      return;
   }
   if ( m_timerID == 0 ) {
      // Request a timer that we will use for requests.
      m_timerID = m_toolkit->requestTimer( m_timerListener,
                                           COLLECT_WAIT_TIME );
   }
}

void
SharedDBufRequester::timerExpired( uint32 /*id*/ )
{
   m_timerID = 0;
   requestUsingPost();
}

void
SharedDBufRequester::setFilter( SharedDBufRequesterRequestFilter* filter )
{
   m_filter = filter;
}

void
SharedDBufRequester::requestUsingPost( )
{
   int nbrMaps = 0;
   const int maxPost = NBR_POST_REQUESTS; // FIXME: Move.
   // We have a 4 down in request() too, not really needed here.
   // Just add all four(4) in m_pending

   int allStringsLen = 0;

   pendingList_t postList;
   // Count the string sizes.
   {for( pendingList_t::iterator it = m_pending.begin();
        it != m_pending.end();
        /* */ ) {
      if ( nbrMaps++ >= maxPost ) {
         break;
      }
      allStringsLen += (*it).second.length() + 1;
      // Move the entry to our temporary list.
      postList.push_back(*it);
      it = m_pending.erase(it);
   }}

   if ( nbrMaps == 0 ) {
      return;
   }
   
   // Prepare the buffer for sending to the server.
   // First comes two longs with start and end offsets.
   // We only use the end offset for now.
   BitBuffer* tempBuf = new BitBuffer(8 + allStringsLen);
   tempBuf->writeNextBALong(   0);
   // FIXME: Move the max size to another better place.
   tempBuf->writeNextBALong(1024*10);
#ifdef BYTELOG_NAME
   FILE* fil = fopen(BYTELOG_NAME, "a");
   int mapNbr = 0;
#endif
   for( pendingList_t::const_iterator it = postList.begin();
        it != postList.end();
        ++it ) {      
      tempBuf->writeNextString( (*it).second.c_str() );
#ifdef BYTELOG_NAME
      ++mapNbr;
      if ( fil ) {
         fprintf( fil, "[SHTTP]: Sending for \"%s\" %d/%d\n",
                  (*it).second.c_str(),
                  mapNbr, nbrMaps );
      }
#endif
   }
#ifdef BYTELOG_NAME
   if ( fil ) {
      fclose( fil );
   }
#endif

   // POST
   mc2log << "[SHTTPDBUF]: POST:ing for " << nbrMaps << " maps" << endl;
   const int bufSize = tempBuf->getCurrentOffset();
   int postID = m_clientConnection->requestMany(tempBuf->getBufferAddress(),
                                                tempBuf->getCurrentOffset());
   delete tempBuf;
   if ( postID >= 0 ) {
      m_nbrBytesSent += bufSize;
#ifdef BYTELOG_NAME
      const uint32 timeDiff =
         TileMapUtil::currentTimeMillis() - m_lastWrittenTime;
      if ( timeDiff > ( WRITE_INTERVAL_MS ) ) {
         FILE* fil = fopen(BYTELOG_NAME, "a");
         if ( fil ) {
            fprintf(fil, "[SHTTP]: tot sent = %d\n", m_nbrBytesSent);
            fclose(fil);
         }
         m_lastWrittenTime = TileMapUtil::currentTimeMillis();
      }
#endif
      ++m_nbrOutstanding;

      // Insert the stuff into the waiting for list.
      for( pendingList_t::const_iterator it = postList.begin();
        it != postList.end();
        ++it ) {
         m_waiting.insert( make_pair ( postID, ReqData(*it, POST) ) );
         m_sentRequests.insert( make_pair( (*it).second , POST ));
      }
   } else {
      // Put back the stuff into the pending list.
      for( pendingList_t::const_iterator it = postList.begin();
        it != postList.end();
        ++it ) {
         m_pending.push_front( *it );
      }
   }
                         
}

void
SharedDBufRequester::sendToListener( const uint8* buf,
                                     int length,
                                     DBufRequestListener* listener,
                                     const DBufRequester& origin )
{
#ifdef BYTELOG_NAME
   FILE* fil = fopen(BYTELOG_NAME, "a");
#endif
   // Same code as below, approx
   BitBuffer* buffer = new BitBuffer( (uint8*)buf, length );
   buffer->reset();
   while ( buffer->getCurrentOffset() < buffer->getBufferSize() ) {
      MC2SimpleString descr( buffer->readNextString() );
      uint32 size = buffer->readNextBALong();
      BitBuffer* retBuf = NULL;
      if ( size != 0 ) {
         // Copy the data into a new buffer.
         retBuf = new BitBuffer(size);
         buffer->readNextByteArray(retBuf->getBufferAddress(), size);
      } else {
         retBuf = NULL;
      }
#ifdef BYTELOG_NAME
      if ( fil ) {
         fprintf(fil, "[SHTTP]: sending extra map = %s\n", descr.c_str());
      }
#endif
      listener->requestReceived( descr, retBuf, origin );
   }
   delete buffer;
#ifdef BYTELOG_NAME
   if ( fil ) {
      fprintf(fil, "[SHTTP]: Totally %u bytes of extra maps\n", length);
      fclose(fil);
   }
#endif
}

void
SharedDBufRequester::request(const MC2SimpleString& descr,
                             DBufRequestListener* caller,
                             request_t whereFrom )
{
   if ( whereFrom == onlyCache ) {
      mc2log << "[SDBR]: Not allowed to request "
             << MC2CITE(descr) << " from internet " << endl;
      // Only cached maps are allowed and this requester has none.
      caller->requestReceived( descr, NULL, *this );
      return;
   }
   
   // FIXME: Will this lead to weird loops?
   if ( m_filter ) {
      if ( !m_filter->isAllowed( descr ) ) {
         // Not allowed by the current filter. Fail immediately.
         caller->requestReceived( descr, NULL, *this );
         return;
      }
   }
   
   mc2dbg << "[SHTTP]: Request - m_waiting.size() = "
          << m_waiting.size() << ", m_pending.size() = "
          << m_pending.size() << endl;
   pair<DBufRequestListener*, MC2SimpleString>
      thePair(make_pair(caller, descr));
   
   if ( alreadySentOrPending( thePair ) ) {
      // FIXME: Check for other stuff to send.
      if ( ! m_pending.empty() ) {
         requestUsingPostTimer();
      }
      return;
   }
   
   // Ok then. Only one request to request, use get.
   
   // TESTING
   m_pending.push_front( make_pair( caller, descr ) );
   
   if ( m_nbrOutstanding >= /*8*/NBR_POST_REQUESTS ) {
      // To much outstanding.
      return;
   }
   
   // Try NBR_POST_REQUESTS request at a time to work around
   // NavServerCom legacy misfeature.
   if ( m_pending.size() < NBR_POST_REQUESTS && !m_waiting.empty() ) {
      // Do nothing
   } else {
      requestUsingPostTimer();
   }
   return;
   // END TESTING
   int getID = m_clientConnection->requestOne(descr.c_str());
   if ( getID >= 0 ) {
      ++m_nbrOutstanding;
      m_waiting.insert( make_pair(getID, ReqData(thePair, GET) ) );
      m_sentRequests.insert(make_pair(descr, GET));
   } else {
      m_pending.push_back(thePair);
   }
}

void
SharedDBufRequester::bufferReceived(DBufRequestListener* listener,
                                    const MC2SimpleString& descr,
                                    BitBuffer* buffer,
                                    bool sendMoreRequests )
{  
   mc2log << warn << "[SHTTP]: Received "
          << descr << " with pointer 0x"
          << hex << buffer << dec
          << endl;
   
   listener->requestReceived(descr, buffer, *this);
   if ( buffer != NULL ) {
      // Insert it into the array of recent received buffers
      m_recentRecv[ m_curRecentRecv ] =
         make_pair(descr, TileMapUtil::currentTimeMillis() );
      m_curRecentRecv = ( m_curRecentRecv + 1 ) % m_nbrRecentRecv;
   }
   m_sentRequests.erase( descr );
   // Request next thing
   if ( ! m_pending.empty() ) {
      if ( buffer != NULL ) {
         // Start by removing the duplicates
         pair<DBufRequestListener*, MC2SimpleString>
            thePair(make_pair(listener, descr));
         pendingList_t::iterator found = std::find(m_pending.begin(),
                                                   m_pending.end(),
                                                   thePair);
         while ( found != m_pending.end() ) {
            mc2dbg << "[SHDB]: Removing one pending since it was received "
                   << "now" << endl;
            m_pending.erase(found);
            found = std::find(m_pending.begin(),
                              m_pending.end(),
                              thePair);
         }
      }
      if ( ! m_pending.empty() && sendMoreRequests ) {
         pair<DBufRequestListener*, MC2SimpleString> oldReq =
            m_pending.front();
         m_pending.pop_front();
         mc2dbg << "[SHTTPDBUF]: Requesting pending" << endl;
         request(oldReq.second, oldReq.first, cacheOrInternet);
      }
   } 
}

void
SharedDBufRequester::postBufferReceived(int postID,                    
                                        BitBuffer* buffer)
{
   mc2dbg << "[SHTTP]: postBufferReceived " << endl;
   if ( buffer == NULL ) {
      // All failed!
      pair<waitingMap_t::iterator, waitingMap_t::iterator> range =
         m_waiting.equal_range(postID);

      // Tell the listener that it failed.
      // Put the data into a temporary vector so that the data can
      // be resent.
      vector<ReqData> tmpVec;
      {for ( waitingMap_t::iterator it = range.first;
            it != range.second;
            ++it ) {
         tmpVec.push_back(it->second);
      }}
      // tmpVec.insert(tmpVec.end(), range.first, range.second);
      // Erase the range.
      m_waiting.erase(range.first, range.second);
      
      for( vector<ReqData>::const_iterator it = tmpVec.begin();
           it != tmpVec.end();
           ++it ) {
         bufferReceived( it->m_listener,
                         it->m_descr,
                         NULL,
                         false);
         
      }
      mc2dbg << "[SHTTP]: RequestRecv all failed - m_waiting.size() = "
             << m_waiting.size() << ", m_pending.size() = "
             << m_pending.size() << endl;
      // FIXME: This should probably be added to the CVS!
      if ( m_waiting.empty() && ( ! m_pending.empty() ) ) {
         pair<DBufRequestListener*, MC2SimpleString> oldReq =
            m_pending.front();
         m_pending.pop_front();
         mc2log << "[SHTTPDBUF]: Requesting pending (POST) after failure"
                << endl;
         request(oldReq.second, oldReq.first, cacheOrInternet);
      }
      return;
   }

   // Success!
   // Read the buffer and get the sub-buffers.
   buffer->reset();
   while ( buffer->getCurrentOffset() < buffer->getBufferSize() ) {
      MC2SimpleString descr( buffer->readNextString() );
      uint32 size = buffer->readNextBALong();
      BitBuffer* retBuf = NULL;
      if ( size != 0 ) {
         // Copy the data into a new buffer.
         retBuf = new BitBuffer(size);
         buffer->readNextByteArray(retBuf->getBufferAddress(), size);
      }
      pair<waitingMap_t::iterator, waitingMap_t::iterator> range =
         m_waiting.equal_range(postID);
      for ( waitingMap_t::iterator it = range.first;
            it != range.second;
            ++it ) {
         // Copy the data.
         ReqData data( it->second );
         if ( data.m_descr == descr ) {
            mc2dbg << "[SHTTP]: Recv " << descr << " from POST" << endl;
            m_waiting.erase(it);
            bufferReceived( data.m_listener,
                            data.m_descr,
                            retBuf,
                            false);
            retBuf = NULL;
            break;
         }
      }
      if ( retBuf != NULL ) {
         // WARNING! Ugglehack. Will not work with multiple requesters.
         // Purpose is to let DXXX through if the server decides
         // to send a new one to us.
         mc2log << warn << "[SHTTP]: Could not find POST data for "
                << descr << endl;
         if ( ! m_waiting.empty() ) {
            mc2log << warn << "[SHTTP]: Sending it up anyway" << endl;
           
            ReqData data( m_waiting.begin()->second );
            
            m_waiting.erase( m_waiting.begin() );
            bufferReceived( data.m_listener,
                            descr,
                            retBuf,
                            false);
                        
         } else {
            delete retBuf;
         }
      }
   }
   // Delete the big buffer
   delete buffer;

   // Put back the not received requests in the pending list
   pair<waitingMap_t::iterator, waitingMap_t::iterator> range =
         m_waiting.equal_range(postID);
   for( waitingMap_t::const_iterator it = range.first;
        it != range.second;
        ++it ) {
      mc2dbg << "[SHTTP]: Moving back one not recv POST" << endl;
      m_sentRequests.erase( it->second.m_descr );
      m_pending.push_front( make_pair( it->second.m_listener,
                                       it->second.m_descr ) );
   }
   // And erase everything about this post.
   
   m_waiting.erase(range.first, range.second);

   if ( ! m_pending.empty() ) {
      pair<DBufRequestListener*, MC2SimpleString> oldReq =
         m_pending.front();
      m_pending.pop_front();
      mc2dbg << "[SHTTPDBUF]: Requesting pending (POST)" << endl;
      request(oldReq.second, oldReq.first, cacheOrInternet);
   }
   
}

void
SharedDBufRequester::bufferReceived(int id, BitBuffer* buffer)
{
   --m_nbrOutstanding;
   
   pair<waitingMap_t::iterator, waitingMap_t::iterator> range =
      m_waiting.equal_range(id);
   
   if ( range.first != m_waiting.end() ) {
      // There must be only one if it is a GET.
      if ( range.first->second.m_typeOfRequest == GET ) {
         bufferReceived( range.first->second.m_listener,
                         range.first->second.m_descr,
                         buffer,
                         true); // Send another request
         m_waiting.erase(range.first);
      } else {
         postBufferReceived(id, buffer);
      }
   } else {
      mc2log << error << "[SHTTPDBUF]: Could not find buffer in map" << endl;
   }   

}
   
void
SharedDBufRequester::release(const MC2SimpleString& /*descr*/,
                                 BitBuffer* obj)
{
   delete obj;
}

void
SharedDBufRequester::cancelAll()
{
#if 0
   // This is an experiment to tell the client that some stuff
   // couldn't be requested.
   pendingList_t tempList;
   tempList.swap( m_pending );
   for( pendingList_t::const_iterator it = tempList.begin();
        it != tempList.end();
        ++it ) {
      (*it).first->requestReceived( (*it).second, NULL, *this);
   }
#else
   mc2log << warn << "[SHAREDHTTPDBUFREQUESTER]: cancelAll" << endl;
   m_pending.clear();

   // Also stop the timer if it is present.
   if ( m_toolkit ) {
      m_toolkit->cancelTimer( m_timerListener, m_timerID );
      m_timerID = 0;
   }
   //m_sentRequests.clear();
#endif
}
