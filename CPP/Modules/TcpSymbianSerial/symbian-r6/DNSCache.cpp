/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DNSCache.h"
#include <in_sock.h>
namespace {
   //A function in order to avoid the stupid ban on static data in
   //Symbian OS before version 9.
   inline class TTimeIntervalHours KValidityInterval()
   {
      return TTimeIntervalHours(3);
   }

   inline class TTime UniversalTime()
   {
      class TTime utc;
      utc.UniversalTime();
      return utc;
   }
}

TInt RDNSCache::CacheHost(const TDesC& aHost, TUint32 aIP)
{
   TInt ret = KErrArgument;
   if(aHost.Length() <= THostAndIP::EMaxHostLength){
      const TLinearOrder<THostAndIP> order(THostAndIP::LinearOrder);
      const struct THostAndIP entry(aHost, aIP);
      ret = iDNSCache.InsertInOrder(entry, order);
      if(ret == KErrAlreadyExists){
         TInt pos = iDNSCache.FindInOrder(entry, order);
         if(pos != KErrNotFound){
            iDNSCache[pos].iAddr = aIP;
         }
      }
   }
   return ret;
}

TInt RDNSCache::CacheHostLookup(const TDesC& aHost, 
                                TNameEntry& aEntry,
                                TInt aPort)
{
   const struct THostAndIP dummy(aHost, 0);
   const TLinearOrder<THostAndIP> order(THostAndIP::LinearOrder);
   TInt pos = iDNSCache.FindInOrder(dummy, order);
   if(pos != KErrNotFound){
      struct THostAndIP& entry = iDNSCache[pos];
      if(entry.IsValid()){
         TInetAddr::Cast(aEntry().iAddr) = 
            TInetAddr(entry.iAddr, aPort);
         pos = KErrNone;
      } else {
         iDNSCache.Remove(pos);
         pos = KErrNotFound;
      }
   }
   return pos;
}

TInt RDNSCache::Remove(const TDesC& aHost)
{
   const struct THostAndIP dummy(aHost, 0);
   const TLinearOrder<THostAndIP> order(THostAndIP::LinearOrder);
   TInt pos = iDNSCache.FindInOrder(dummy, order);
   if(pos != KErrNotFound){
      iDNSCache.Remove(pos);
      pos = KErrNone;
   }
   return pos;
}


void RDNSCache::Reset()
{
   iDNSCache.Reset();
}

void RDNSCache::Close()
{
   Reset();
   iDNSCache.Close();
}

RDNSCache::THostAndIP::THostAndIP(const TDesC& aName, TUint32 aAddr) :
   iName(aName), iAddr(aAddr), iTime(UniversalTime())
{
}

TBool RDNSCache::THostAndIP::IsValid() const
{
   class TTimeIntervalHours age;
   class TTime now = UniversalTime();
   return ((KErrNone == now.HoursFrom(iTime, age)) && 
           (age < KValidityInterval())); 
}

TBool 
RDNSCache::THostAndIP::operator<(const struct THostAndIP& aOther) const
{
   return this->iName < aOther.iName;
}

TInt RDNSCache::THostAndIP::LinearOrder(const struct THostAndIP& aLhs, 
                                       const struct THostAndIP& aRhs)
{
   return ((aLhs < aRhs) ? -1 : ((aRhs < aLhs) ? 1 : 0));
}
