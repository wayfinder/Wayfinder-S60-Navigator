/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "arch.h"
#include "Buffer.h"
#include "nav2util.h"
#include "Buffer.h"
#include "MsgBuffer.h"
#include "MsgBufferEnums.h"
#include "Module.h"
#include "ParameterEnums.h"
#include "Parameter.h"
#include "Server.h"
#ifdef __SYMBIAN32__
# include <e32math.h>
#endif

#define SWITCH_GROUP_THRESHOLD 2

using namespace std;

namespace isab {

void Server::toString(Buffer& buf) const
{
   buf.writeNextCharString(getHostAndPort());
   buf.jumpWritePos(-1); // skip nullbyte
   if (getUrl()) {
      buf.writeNextCharString(getUrl());
      buf.jumpWritePos(-1); // skip nullbyte
   }
   buf.nprintf(16, ":%d", m_group);
}

Server* Server::factory(Buffer& buf, int lastGroup, int delim,
                                    int groupDelim)
{
   const char* start = reinterpret_cast<const char*>(buf.accessRawData());
   char delims[] = {delim, groupDelim, 0};
   char* end = strpbrk(start, delims);
   Server* retval = NULL;
   if(end){
      int len = end - start;
      char* tmp = new char[len + 1];
      strncpy(tmp, start, len);
      tmp[len] = '\0';
      retval = new Server(tmp);
      delete[] tmp;
      buf.jumpReadPosTo(*end);
   } else {
      retval = new Server(start);
   }
   retval->setGroup(lastGroup);
   return retval;     
}


Server::Server(const char* HOST, uint16 PORT) :
   m_host(NULL), m_url(NULL), m_hostAndPort(NULL),
   m_port(PORT), persistent(true),
   m_group(0), m_unreachable(false)
{
   if(HOST){
      int len = strlen(HOST) + 1;
      m_host = new char[len];
      strcpy(m_host, HOST);
      m_hostAndPort = new char[len + 10];
      snprintf(m_hostAndPort, len + 9, "%s:%u", m_host, m_port);
   }
};

/** Constructs a Server object from a FQDN and a port encoded in 
 * a string with a colon as delimiter: host.domain[/url]:port[:group] 
 * @param hostAndPort a host and port encoded as described above.*/
Server::Server(const char* hostAndPort) :
   m_host(NULL), m_url(NULL), m_hostAndPort(NULL), m_port(0), persistent(true),
   m_group(-1), m_unreachable(false)
{
   if(hostAndPort){ //not NULL
      m_hostAndPort = strdup_new(hostAndPort); //copy 
      m_host = strdup_new(hostAndPort);        //copy
      char * colon = strchr(m_host, ':');      //find first : 
      if(colon != NULL){                       //found one
         *colon++ = '\0';                      //snip host string
         char* end;                            
         unsigned long lPort = strtoul(colon, &end, 10); //read port
         if(lPort <= MAX_UINT16 && lPort > 1){
            m_port = lPort; //OK port number
         }
         if (end == NULL){ // End of data, return.
            return;
         }
         // else data after port
         if (*end == '/') {
            /* Got an optional URL field. */
            char *slash = end;
            m_hostAndPort[slash - m_host] = '\0'; //snip hostandport at /
            end = strchr(end, ':');
            if (end != NULL) {
               /* Nul terminate */
               *end++ = '\0';    // Will advance to next character
               m_group = strtol(++end, &end, 10); //read group
               if(end == NULL){
                  //m_group = 0;
               }
               end = NULL; // No more data.
            }
            m_url = strdup_new(slash);
            return;
         }
         if(*end == ':') {
            *end = '\0';  // remove the second :, why?
            m_hostAndPort[end - m_host] = '\0'; //snip hostandport at 2nd :
            m_group = strtol(++end, &end, 10); //read group
            if(end == NULL){
               //m_group = 0;
            }
         }
      }
   }
}

#if defined DBG_LOAD_BALANCE && !defined __SYMBIAN32__ && !defined _MSC_VER
# define CERR(args) cerr << __LINE__ << ":" << args << endl
#else
# define CERR(args)
#endif
int nextServer(int64& seed, std::vector<Server*>& v, int oldIndex)
{
   CERR("oldindex: " << oldIndex);
   CERR("v.size(): " << v.size());
   typedef std::vector<Server*>::iterator iter;
   if(!(oldIndex >= 0 && unsigned(oldIndex) < v.size())
      /* || !v[oldIndex]->isReachable()*/){
      CERR("!("<< oldIndex << " >= 0 && " << 
           unsigned(oldIndex) << " < " << v.size() << ") ||");
      if(oldIndex >= 0 && unsigned(oldIndex) < v.size()){
         CERR(" ! (v[" << oldIndex << "]->isReachable() : " << 
              v[oldIndex]->isReachable() << ")");
      }
      //start searching from first group
      CERR("oldindex: " << oldIndex << ", will set to -1");
      oldIndex = -1;
   }
   sort(v.begin(), v.end(), ServerGroupLess());
   Server cmp("dummy", 12); //dummy server
   if(oldIndex >= 0){
      //set dummy's group to be the last used group.
      cmp.setGroup(v[oldIndex]->group());
      CERR("cmp.group: " << cmp.group());
   }
   int lastGroup = v.back()->group(); //group with highest number.
   CERR("lastgroup: " << lastGroup);
   std::pair<iter,iter> range;
   //find group to use
   while(cmp.group() <= lastGroup){
      //find range of reachables with same group as dummy.
      range = equal_range(v.begin(), v.end(), &cmp, ServerGroupLess());
      if(range.first != range.second) { //range is not empty
         CERR("range.first != range.second");
         CERR("more servers in list");
#define REACHABLE(iter) ((iter != v.end()) ? ((*iter)->isReachable()) : false)
#define HOST(iter) ((iter != v.end()) ? ((*iter)->getHostAndPort()) : "END")
#define GROUP(iter) ((iter != v.end()) ? ((*iter)->group()) : -1)
#define SERVER(iter) CERR(#iter << " : " << distance(v.begin(), iter) << " "<<\
                          HOST(iter) << " - " GROUP(iter) << " " <<\
                          REACHABLE(iter))
#define SERVER_RANGE(b,e) for(iter a_ = b; a_ != e; ++a_){SERVER(a_);}
         SERVER(v.begin());
         SERVER(range.first);
         SERVER(range.second);
         SERVER((v.end() - 1));
         SERVER_RANGE(v.begin(), v.end());
         CERR("begin -> first  : " << distance(v.begin(), range.first));
         CERR("first -> second : " << distance(range.first, range.second));
         CERR("second -> end   : " << distance(range.second, v.end()));
         iter nextgroup = find_if(range.second - 1, v.end(), 
                                  not1(bind2nd(mem_fun(&Server::isGroup),
                                               cmp.group())));
         SERVER(nextgroup);
         CERR("second -> nextgroup : "<<distance(range.second, nextgroup));
         if(distance(range.second, nextgroup) < SWITCH_GROUP_THRESHOLD){
            CERR("unreachables has not passed THRESHOLD");
            break;
         }
#undef SERVER_RANGE
#undef SERVER
#undef HOST
#undef REACHABLE
      }
      //range is empty or has too many unreachables, increase group by
      //one and try again.
      cmp.setGroup(cmp.group() + 1);
      CERR("cmp.group: " << cmp.group());      
   }
   //size of range,
   int num = distance(range.first, range.second);
   CERR("num: " << num);
   //index of first server in selected group
   int startIdx = distance(v.begin(), range.first);
   CERR("startIdx: " << startIdx);
   if(cmp.group() > lastGroup){
      //out of servers, sorry
      CERR("returning " << v.size() << ", which is illegal");
      return v.size();
   } else if(oldIndex >= 0 && v[oldIndex]->isReachable()){
      //step one step in this group.
      CERR("returning " << startIdx+(oldIndex+1)%num << ", oldIndex + 1");
      return startIdx + (oldIndex + 1) % num;
   } else {
      //srand is called in NavServerCom. It seems to be important to
      //call srand and rand in the same thread context.
#ifndef __SYMBIAN32__
      //random index in this group. 
      int ret = startIdx + (rand() / (RAND_MAX / num + 1));
#else
      int ret = startIdx + (Math::Rand(seed) / (KMaxTInt / num + 1));
#endif
      CERR("returning " << ret << ", a random value");
      return ret;
   }
}

}
