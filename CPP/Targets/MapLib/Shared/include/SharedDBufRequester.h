/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SHARED_BDUBF_REQUYESTE_ER_H
#define SHARED_BDUBF_REQUYESTE_ER_H


#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"

#include <list>
#include <map>
#include <set>

class TCPConnectionHandler;
class DBufConnection;
class BitBuffer;
class SharedDBufRequesterListener;
class TileMapToolkit;
template<class T> class TileMapTimerListenerTemplate;

class SharedDBufRequesterRequestFilter {
public:
   /**
    *   Returns true if the requester is allowed to send for
    *   the buffer.
    */ 
   virtual bool isAllowed( const MC2SimpleString& desc ) = 0;
   
};

class SharedDBufRequester : public DBufRequester {
public: 
   /**
    *   Creates a new SharedHttpDBufRequest.
    *   @param clientConnetion The connection handler
    *   @param host            Hostname.
    *   @param port            The port.
    *   @param prefix          What to prefix all uri:s with.
    */
   SharedDBufRequester(DBufConnection* clientConnetion,
                       TileMapToolkit* toolkit = NULL );

   /**
    *   Destructor.
    */
   virtual ~SharedDBufRequester();

   /**
    *   Will decode the maps from the buffer and send them to
    *   the listener even if it does not want them. The buffer
    *   should be in the same format as when received from the
    *   internet.
    *   @param buf      Buffer to decode.
    *   @param length   Length of the buffer.
    *   @param listener Listener to send the maps to.
    */
   static void sendToListener( const uint8* buf,
                               int length,
                               DBufRequestListener* listener,
                               const DBufRequester& origin );
   
   /**
    *   The Requester must keep the 
    *   release is called.
    *   @param descr      The description.
    *   @param caller     The RequestListener that wants an update
    *                     if a delayed answer comes back.
    *   @param onlyCached True if only cached replies should be considered.
    */ 
   void request(const MC2SimpleString& descr,
                DBufRequestListener* caller,
                request_t whereFrom );
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer.
    *   Default implementation deletes the BitBuffer and removes
    *   it from the set.
    */ 
   void release(const MC2SimpleString& descr,
                BitBuffer* obj);

   /**
    *   Tries to cancel all requests in the requester.
    *   Note that the requests e.g. already sent to the server
    *   cannot be canceled.
    */
   void cancelAll();

   /**
    *   Called by the listener when a buffer is complete.
    */
   void bufferReceived(int id, BitBuffer* buffer);

   /**
    *   Called by the listener when a (POST:ed) buffer is
    *   completed.
    *   @param listener The request listener.
    *   @param descr    The description of the buffer.
    *   @param buffer   The buffer itself.
    *   @param sendMore True if more requests are supposed to be sent.
    */
   void bufferReceived(DBufRequestListener* listener,
                       const MC2SimpleString& descr,
                       BitBuffer* buffer,
                       bool sendMore);

   /**
    *   Called if the timer expires.
    */
   void timerExpired( uint32 timerID );

   /**
    *   Sets the toolkit for timer operations to work.
    */
   void setToolkit( TileMapToolkit* toolkit );

   /**
    *   Sets the filter. If the filter is not NULL
    *   then all requests will be checked for allowance
    *   with the filter before they are sent to the server.
    *   @param filter Filter to use or NULL.
    */
   void setFilter( SharedDBufRequesterRequestFilter* filter );
   
protected:   
   /**
    *   The client connection.
    */
   DBufConnection* m_clientConnection;
   
private:
   /**
    *   Handles that a buffer containing multiple requests
    *   has been received.
    */
   void postBufferReceived(int postID,                    
                           BitBuffer* buffer);
                      
   /**
    *   Checks if we are already waiting for the
    *   descr and caller.
    *   @return True if already sent.
    */
   bool alreadySentOrPending(
      const std::pair<DBufRequestListener*, MC2SimpleString>& thePair );

   /**
    *   Requests using post after a timer delay.
    */
   void requestUsingPostTimer();
   
   /**
    *   Request a number of maps using POST.
    */
   void requestUsingPost();
   
   /**
    *   The listener to the DBufConnection.
    */
   SharedDBufRequesterListener* m_listener;

   /**
    *   Type of list of pending requests.
    */
   typedef std::list<std::pair<DBufRequestListener*, MC2SimpleString> > pendingList_t;

   /**
    *   The pending requests.
    */
   pendingList_t m_pending;
   
   /**
    *   Type of request we are waiting for.
    */
   enum typeOfRequest_t {
      /// The request is sent for using GET (one map in the answer)
      GET  = 1,
      /// The request is sent for using POST (many maps in the answer)
      POST = 2,
   };

   /// Number of outstanding requests to the server.
   int m_nbrOutstanding;

   class ReqData {
     public:
      /**
       *   Creates a new ReqData.
       */
      ReqData(DBufRequestListener* listener,
              const MC2SimpleString& descr,
              typeOfRequest_t typeOfRequest) :
         m_listener(listener),
         m_descr(descr),
         m_typeOfRequest(typeOfRequest) {}

      /**
       *
       */
      ReqData(const std::pair<DBufRequestListener*, MC2SimpleString>& listenerPair,
              typeOfRequest_t typeOfRequest) :
         m_listener(listenerPair.first),
         m_descr(listenerPair.second),
         m_typeOfRequest(typeOfRequest) {}

      /// The listener to the request
      DBufRequestListener* m_listener;
      /// The description of the map
      MC2SimpleString m_descr;
      /// The type of request.
      typeOfRequest_t m_typeOfRequest;
      
   };
   
   /// Type of map with waiting requests.
   typedef std::multimap<int, ReqData> waitingMap_t;
   
   /**
    *   Map of waiting requests.
    */
   waitingMap_t m_waiting;

   /**
    *   Set of sent requests.
    */
   std::map<MC2SimpleString, typeOfRequest_t> m_sentRequests;

#define SHAREDDBUFREQUESTER_NBR_RECENT 10
   
   enum visual_c_plus_plus_etc_etc_dont_get_me_started {
      /// Size of array containing last received descrs
      m_nbrRecentRecv = SHAREDDBUFREQUESTER_NBR_RECENT,
   };

   /**
    *   Current position in m_lastRecv
    */
   int m_curRecentRecv;
   
   /**
    *   Array containing the last received maps to
    *   avoid re-requesting them.
    */
   std::pair<MC2SimpleString,uint32> m_recentRecv[SHAREDDBUFREQUESTER_NBR_RECENT];

#undef SHAREDDBUFREQUESTER_NBR_RECENT

   /**
    *   The total number of bytes sent.
    */
   int m_nbrBytesSent;

   /// Last time the m_nbrBytesSent were written to file
   uint32 m_lastWrittenTime;

   /// Timer listener
   TileMapTimerListenerTemplate<SharedDBufRequester>* m_timerListener;
   /// Timer id
   uint32 m_timerID;
   /// Toolkit
   TileMapToolkit* m_toolkit;
   /// Filter which checks which maps are allowed if not NULL.
   SharedDBufRequesterRequestFilter* m_filter;
};

#endif
