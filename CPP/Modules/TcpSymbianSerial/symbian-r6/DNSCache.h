/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DNSCACHE_SYMBIAN_H
#define DNSCACHE_SYMBIAN_H
#include <e32std.h>
#include <es_sock.h>
class RDNSCache
{
public:
   /**
    * Add a hostname and its IP number to the cache.
    * @param aHost The hostname. 
    * @param aIP   The IP number.
    * @return KErrArgument if the length of the aHost argument is
    *         longer than <code>RDNSCache::THostAndIP::EMaxHostLength. 
    *         KErrAlreadyExists if the host is already listed in the cache.
    *         KErrNone if all is well. 
    *         Otherwise any value that
    *         <code>RArray<T>::InsertInOrder</code> may return.
    */
   TInt CacheHost(const TDesC& aHost, TUint32 aIP);
   /**
    * Searches the cache for a host. 
    * @param aHost The hostname to search for.
    * @param aEntry Where the IP-address will be stored if it is
    *               found. Note that the port number is also stored
    *               here.
    * @param aPort The port number that will be used when connecting
    *              to the host. Will be stored in the aEntry variable
    *              together with the IP-address.
    * @return KErrNone if the hostname was found in the cache,
    *         KErrNotFound if it was not found.
    */
   TInt CacheHostLookup(const TDesC& aHost, 
                        TNameEntry& aEntry, TInt aPort);

   /**
    * Removes a hostname from the cache.
    * @param aHost The hostname to remove. 
    * @return KErrNone if the cache entry was removed. KErrNotFound if
    *         no entry in the cache could be found.
    */
   TInt Remove(const TDesC& aHost);

   /**
    * Clear the cache.
    */
   void Reset();
   /**
    * Clear the cache and close all associated resources. A closed
    * cache can not be reopened.
    */
   void Close();
   
private:

   /** Stored in RDNSCache. sizeof(THostAndIP) must be less than 640.*/
   struct THostAndIP
   {
      /** 
       * Constructor.
       * @param aName The hostname. Must be shorter than 97 characters.
       * @param aAddr The IP address.
       */
      THostAndIP(const TDesC& aName, TUint32 aAddr);

      /**
       * Check whether the entry is still valid. Entries are valid for
       * three hours after being created.
       * @return ETrue if the object is less than three hours old. 
       */
      TBool IsValid() const;

      enum {
         /** Max length of cached hostnames */
         EMaxHostLength = 96
      };
      /** Hostname */
      TBuf<EMaxHostLength> iName; //sizeof(iName) == 96 * 2 = 192
      /** IP address */
      TUint32  iAddr; //sizeof(iAddr) == 4
      /** Time of creation. */
      const class TTime iTime;

      /** 
       * Forms a complete ordering of THostAndIP objects.
       * Objects are ordered by name first and then by IP address.
       * @param aOther The other object to compare with.
       * @return ETrue if this THostAndIP object should be ordered
       *         before aOther.
       */
      TBool operator<(const struct THostAndIP& aOther) const;
      /**
       * Function that may be used by a TLinearOrder<THostAndIP> object.
       * Uses operator<.
       * @param aFirst  The first object.
       * @param aSecond The second object.
       * @return -1 if aFirst < aSecond, 
       *          1 of aSecond < aFirst,
       *          0 otherwise.
       */
      static TInt LinearOrder(const struct THostAndIP& aLhs, 
                              const struct THostAndIP& aRhs);
   };

   /** Cache of looked-up hosts. */
   RArray<THostAndIP> iDNSCache;


};
#endif
