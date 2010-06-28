/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef HISTORY_HANDLER_H
#define HISTORY_HANDLER_H

#include "arch.h"
#include <deque>

typedef std::deque<char*> urlList;

/**
 * Holds a list of URLs that have been visited.
 */
class URLHistoryHandler
{
public:

   URLHistoryHandler(uint32 maxEntries = 30)
    : m_maxEntries(maxEntries), m_enabled(1) {}

   ~URLHistoryHandler();

   /**
    * Add an url to history list.
    * Will push out the last entry if the list contains too many URLs.
    * Will take ownership of url.
    */
   void PushUrl(char* url);

   /**
    * Remove the topmost url from history list.
    * Ownership of url is transferred to caller.
    */
   char *PopUrl();

   /**
    * Get the current history list.
    */
   const urlList& GetUrlList() { return m_urlList; }

   /**
    * Get the n:th history entry.
    * NOTE!!! Ownership is not transferred!
    */
   const char *GetUrl(uint32 num)
   {
      if (num < m_urlList.size()) {
         return m_urlList[num];
      }
      return NULL;
   }

   void Clear();

   void enableHistory() { m_enabled = 1; }
   void disableHistory() { m_enabled = 0; }

private:
   urlList m_urlList;

   uint32 m_maxEntries;

   int32 m_enabled;

};

#endif

