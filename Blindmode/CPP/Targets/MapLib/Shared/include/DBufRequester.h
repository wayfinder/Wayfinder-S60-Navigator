/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef DATABUFFERREQUESTER_H
#define DATABUFFERREQUESTER_H

// Includes config.h if mc2
#include "TileMapConfig.h"
#include "LangTypes.h"

// The header file for external use.
#include "MapLib.h"

class MC2SimpleString;
class BitBuffer;
class DBufRequester;

/**
 *   Class waiting for a request.
 */
class DBufRequestListener {
public:

   virtual ~DBufRequestListener() {}
   
   /**
    *   Called by the Requester when a request is done.
    *   @param descr          The descr used to get the object.
    *   @param receivedObject The received object.
    *   @param origin         The origin requester.
    */
   virtual void requestReceived(const MC2SimpleString& descr,
                                BitBuffer* dataBuffer,
                                const DBufRequester& origin ) = 0;
};

/**
 *   Class that is asked if it is ok to release the buffer to the
 *   next requester. Used for the traffic information maps that are
 *   not allowed to be stored anywhere else than in the
 *   MemoryDBufRequester.
 */
class DBufReleaseChecker {
public:

   virtual ~DBufReleaseChecker() {}

   /**
    *   Returns true if the buffer is allowed to be released
    *   to the next cache. Returns false if it should be destroyed
    *   instead.
    */   
   virtual bool releaseToNextAllowed( const MC2SimpleString& descr,
                                      const DBufRequester& req ) = 0;
};

/**
 *   Class describing something that can request BitBuffers.
 *   <br />
 *   If you are making a DBufRequester which request stuff
 *   from an external source, you will have to override
 *   cancelAll and requestFromExternal and set parent to NULL.
 *   <br />
 *   If you are making a caching DBufRequester you will have to
 *   override requestCached and release and set parent == real parent.
 */
class DBufRequester : public MapLib::CacheInfo {
public:
   /// Describes from where the maps are allowed to be requested.
   enum request_t {
      /// It is allowed to request the map from internet or the cache
      cacheOrInternet = 0,
      /// It is allowed to request the map from caches only.
      onlyCache       = 1,
   };

   /// Type of requester to allow different behaviour
   enum requester_t {
      /// Memory requester
      MEMORY    = 1,
      /// File requester
      PERMANENT = 2,
      /// Read only (to be or:ed together with file )
      READ_ONLY = 4,
      /// External (internet)
      EXTERNAL  = 8,
      /// Unkown
      UNKNOWN   = 16,
   };

   // - Implements MapLib::CacheInfo
   virtual const char* getPathUTF8() const {
      return "";
   }

   virtual const char* getNameUTF8() const {
      return "";
   }
   
   virtual bool isValid() const {
      return true;
   }

   // - End of implementation of MapLib::CacheInfo
   
   /**
    *   Creates a new DBufRequester with the supplied parent
    *   requester.
    */
   DBufRequester(DBufRequester* parentRequester = NULL);

   /**
    *   Deletes the parent.
    */
   virtual ~DBufRequester() {
      delete m_parentRequester;
   }

   /// Sets the preferred language
   void setPreferredLang( LangTypes::language_t preflang ) {
      m_preferredLang = preflang;
      if ( m_parentRequester ) {
         m_parentRequester->setPreferredLang( preflang );
      }
   }

   /**
    *    Puts all the requesters beginning with this one into
    *    the vector or similar.
    */
   template<class VECTOR> void getAllRequesters( VECTOR& dest ) {
      dest.push_back( this );
      if ( m_parentRequester ) {
         m_parentRequester->getAllRequesters( dest );
      }
   }
   
   /**
    *   Sets new parent. Returns old parent.
    */
   DBufRequester* setParent( DBufRequester* parent ) {
      DBufRequester* tmpParent = m_parentRequester;
      m_parentRequester = parent;
      return tmpParent;
   }

   /**
    *   Sets new DBufReleaseChecker.
    *   Should only be called by TileMapHandler.
    */
   void setReleaseChecker( DBufReleaseChecker* rel ) {
      m_releaseChecker = rel;
      if ( m_parentRequester ) {
         m_parentRequester->setReleaseChecker( rel );
      }
   }
   
   /**
    *   Returns the parent requester.
    */
   DBufRequester* getParent() const {
      return m_parentRequester;
   }

   /**
    *   @return Type of requester.
    */
   requester_t getType() const {
      return requester_t( m_requesterType );
   }

   /**
    *   Sets the type of the requester.
    */
   void setType(  requester_t type ) {
      m_requesterType = type;
   }
   
   /**
    *   Requests a BitBuffer from the DBufRequester. When the
    *   buffer is received caller->requestReceived(desc, buffer)
    *   will be called.
    *   <br />
    *   Default implementation calls requestCached and if it
    *   returns a buffer, caller->requestReceived will be called
    *   immideately, otherwise, if there is a parent, it will
    *   be called and if there is no parent requestFromExternal
    *   will be called.
    *   @param descr      The description.
    *   @param caller     The RequestListener that wants an update
    *                     if a delayed answer comes back.
    *   @param whereFrom  Where to request the map from.
    */ 
   virtual void request(const MC2SimpleString& descr,
                        DBufRequestListener* caller,
                        request_t whereFrom );

   /**
    *   Deletes the cached version of the buffer.
    */
   void removeBuffer( const MC2SimpleString& descr );

   /**
    *   If the DBufRequester already has the map in cache it
    *   should return it here or NULL. The BitBuffer should be
    *   returned in release as usual.
    *   <br />Default implementation returns
    *   <code>m_parentRequester->requestCached</code>
    *   or <b>NULL</b> if <code>m_parentRequester</code> is
    *   <b>NULL</b>.
    *   @param descr Key for databuffer.
    *   @return Cached BitBuffer or NULL.
    */
   virtual BitBuffer* requestCached(const MC2SimpleString& descr);
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer
    *   or to put it in the cache. Requesters which use other
    *   requesters should hand the objects back to their parents
    *   and requesters which are top-requesters should delete them.
    *   It is important that the descr is the correct one.
    *   <br />
    *   This is where the caching requesters should save their buffers.
    *   <br />
    *   Default implementaion calls release in the parent if there
    *   is one or else deletes the buffer.
    */ 
   virtual void release(const MC2SimpleString& descr,
                        BitBuffer* obj);
   /**
    *   Tries to cancel all requests in the requester.
    *   Note that the requests e.g. already sent to the server
    *   cannot be canceled.
    *   Default implementation calls the cancelAll in the parent
    *   requester if there is one.
    */
   virtual void cancelAll();

protected:
   /**
    *   Does the real requesting.
    *   Default implementation does nothing.
    *   This is the one to override if doing a requester which
    *   requests buffers from e.g. internet.
    */
   virtual void requestFromExternal(const MC2SimpleString& descr,
                                    DBufRequestListener* caller);

   /**
    *   Should be overridden by the subclasses if they are caches.
    *   Should delete all occurances of the buffer associated
    *   with the descr.
    */
   virtual void internalRemove(const MC2SimpleString& /*descr*/ ) {}

   /**
    *   The parent requester.
    */
   DBufRequester* m_parentRequester;

   /// Type of requester
   uint32 m_requesterType;

   /// Release checker to ask whether to release or delete a buffer
   DBufReleaseChecker* m_releaseChecker;

   /// Preferred language of the requestor
   LangTypes::language_t m_preferredLang;
   
};

#endif
