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
#include "nav2util.h"
#include "TimeUtils.h"
static const unsigned KValidityInterval = 3 * 60 * 60; //3 hrs as seconds


enum isab::DNSCache::cacheResult 
isab::DNSCache::cacheHost(const char* host, uint32 ip)
{
   enum cacheResult ret = HOST_TO_LONG;
   if(strlen(host) < HostAndIP::MaxHostLength){
      cache_t::iterator p = findHost(host);
      if(p != m_cache.end()){
         ret = ALREADY_EXISTS;
         p->m_ip = ip;
      } else {
         m_cache.push_back(HostAndIP(host, ip));
         ret = CACHE_SUCCESS;
      }
   }
   return ret;
}

isab::DNSCache::cache_t::iterator isab::DNSCache::findHost(const char* host)
{
   for(cache_t::iterator it = m_cache.begin(); it != m_cache.end(); ++it){
      if(strequ(it->m_name, host)){
         return it;
      }
   }
   return m_cache.end();
}

bool isab::DNSCache::lookupHost(const char* host, uint32& ip) 
{
   const cache_t::iterator p = findHost(host);
   if(p != m_cache.end()){
      if(p->isValid()){
         ip = p->m_ip;
         return true;
      } else {
         m_cache.erase(p);
      }
   } 
   return false;
}

bool isab::DNSCache::removeHost(const char* host)
{
   const cache_t::iterator p = findHost(host);
   if(p != m_cache.end()){
      m_cache.erase(p);
      return true;
   }
   return false;
}

isab::DNSCache::HostAndIP::HostAndIP(const char* name, uint32 ip) :
   m_ip(ip), m_time(TimeUtils::time(NULL))
{
   strncpy(m_name, name, sizeof(m_name) - 1); //works while m_name is an array.
   m_name[sizeof(m_name) - 1] = '\0';
}

isab::DNSCache::HostAndIP::HostAndIP(const HostAndIP& orig) :
   m_ip(orig.m_ip), m_time(orig.m_time)
{
   strcpy(m_name, orig.m_name);
}

bool isab::DNSCache::HostAndIP::isValid() const
{
   time_t now = TimeUtils::time(NULL);
   double diff = difftime(now, m_time);
   return diff < KValidityInterval;
}

bool 
isab::DNSCache::HostAndIP::operator<(const struct isab::DNSCache::HostAndIP& aOther) const
{
   return this->m_name < aOther.m_name;
}

const struct isab::DNSCache::HostAndIP 
isab::DNSCache::HostAndIP::operator=(const struct isab::DNSCache::HostAndIP& from)
{
   strcpy(m_name, from.m_name);
   m_ip = from.m_ip;
   const_cast<time_t&>(m_time) = from.m_time;
   return *this;
}

