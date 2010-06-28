/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DNSCACHE_H
#define DNSCACHE_H

#include "arch.h"
#include <deque>

namespace isab{
   class DNSCache
   {
   public:

      /** Return values for DNSCache::cacheHost. */
      enum cacheResult {
         /** The hostname was longer than HostAndIP::MaxHostLength. */
         HOST_TO_LONG   = -2,
         /** The cache already contains an entry for this hostname. */
         ALREADY_EXISTS = -1,
         /** The cache insert operation was successfull. */
         CACHE_SUCCESS  =  0,
      };

      /**
       * Add a hostname and its IP number to the cache.
       * @param host The hostname. 
       * @param ip   The IP number.
       * @return HOST_TO_LONG if the length of the host argument is
       *         longer than <code>DNSCache::HostAndIP::MaxHostLength</code>.
       *         ALREADY_EXISTS if the host was already in the cache. 
       *         Otherwise CACHE_SUCCESS. 
       */
      enum cacheResult cacheHost(const char* host, uint32 ip);

      /**
       * Look up a host in the cache. 
       * @param host The name of the host.
       * @param ip   Output parameter for the IP address.
       * @return true if the host was found in the cache, false
       *         otherwise. If the host was found the ip out-parameter
       *         is set. Otherwise it is not touched.
       */
      bool lookupHost(const char* host, uint32& ip);

      /**
       * Remove a host entry from the cache. 
       * @param host The name of the host that should be removed. 
       * @return true if the host was present and then removed, false
       *         otherwise.
       */
      bool removeHost(const char* host);

   private:
      /** Stored in DNSCache. */
      struct HostAndIP
      {
         /** 
          * Constructor.
          * @param aName The hostname. Must be shorter than 97 characters.
          * @param aAddr The IP address.
          */
         HostAndIP(const char* aName, uint32 ip);
         /** 
          * Copy constructor.
          * @param orig The HostAndIP object to copy. 
          */
         HostAndIP(const HostAndIP& orig);
         /**
          * Test whether this HostAndIP object is still valid for use.
          * @return true if this object is less than three hours old,
          *         false otherwise.
          */
         bool isValid() const;

         enum {
            /** Max length of cached hostnames */
            MaxHostLength = 96
         };
         /** Hostname */
         char m_name[MaxHostLength]; 
         /** IP address */
         uint32  m_ip; 
         /** Time of creation. */
         const time_t m_time;

         /** 
          * Forms a complete ordering of THostAndIP objects.
          * Objects are ordered by name first and then by IP address.
          * @param aOther The other object to compare with.
          * @return ETrue if this THostAndIP object should be ordered
          *         before aOther.
          */
         bool operator<(const struct HostAndIP& aOther) const;
         /**
          * Assignment operator.
          * @param from Object to copy. 
          * @return Const reference to this. 
          */
         const struct HostAndIP operator=(const struct HostAndIP& from);
      };

      /** The collection type used to hold the HostAndIP objects. */
      typedef std::deque<HostAndIP> cache_t;
      /** 
       * Find a cache entry for a specified host.
       * @param host The name of the host to find. 
       * @return Iterator into m_cache where HostAndIP::m_name is
       *         equal to host. If no such hos is found, m_cache.end()
       *         is returned.
       */
      cache_t::iterator findHost(const char* host);

      /** Cache of looked-up hosts. */
      cache_t m_cache;
   };
}
#endif
