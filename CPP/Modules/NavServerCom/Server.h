/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SERVER_H
#define SERVER_H
#include <vector>
#include <algorithm>
#include <functional>

namespace isab{
   /** Utlity class to keep track of a server with a port.*/
   class Server{
      char* m_host;
      char* m_url;
      char* m_hostAndPort;
      uint16 m_port;
   public:
      /** If set, indicates that this Server should be kept for the next 
       * session*/
      bool persistent;
   private:
      int m_group;
      bool m_unreachable;
   public:
      bool isReachable() const;
      void setUnreachable();
      void setReachable() { m_unreachable = false;}
      int group() const;
      void setGroup(int newGroup) {m_group = newGroup;}
      bool isGroup(int group) { return m_group == group; }
      /** A fully qualified domain name, or ip address as text. */
      const char* getHost() const;
      /** A port. Anything less than or equal to 1024 is invalid.*/
      uint16 getPort() const;
      /** Constructs a Server object from a string and a port.
       * @param host a fully qualified domain nam or an ip address as text.
       * @param port a port number. Anything less than 1025 is invalid. */
      Server(const char* host, uint16 port);
      /** Constructs a Server object from a FQDN and a port encoded in 
       * a string with a colon as delimiter: host.domain:port 
       * @param hostAndPort a host and port encoded as described above.*/
      Server(const char* hostAndPort);
      //      Server() : host(NULL), port(0), persistent(false) {}
      ~Server();
      static Server* factory(Buffer& buf, int lastGroup, int delim = ',',
                             int groupDelim = ';');
      /** Allocates enough memory and writes a string in it that can be used 
       * to create a new Server object.
       * @return a pointer to a dynamically allocated string that the caller
       *         must delete[] at a later time.*/
      const char * getHostAndPort() const;
      const char * getUrl() const;
      void toString(Buffer& buf) const;
      Server(const Server& server);
      const Server& operator=(const Server& rhs);
      bool operator<(const Server& rhs) const;
      bool operator==(const Server& rhs) const;
   };

   //===================Server inlines ==================================

   inline Server::Server(const Server& server) :
      m_host(NULL), m_url(NULL), m_hostAndPort(NULL), m_port(server.m_port), 
      persistent(server.persistent),
      m_group(server.m_group), m_unreachable(server.m_unreachable)
   {
      m_host = strdup_new(server.m_host);
      m_url  = strdup_new(server.m_url);
      m_hostAndPort = strdup_new(server.m_hostAndPort);
   }
   
   inline const Server& Server::operator=(const Server& rhs)
   {
      if(this != &rhs){
         m_port        = rhs.m_port; 
         persistent    = rhs.persistent;
         m_group       = rhs.m_group; 
         m_unreachable = rhs.m_unreachable;
         m_host        = strdup_new(rhs.m_host);
         m_url         = strdup_new(rhs.m_url);
         m_hostAndPort = strdup_new(rhs.m_hostAndPort);
      }
      return *this;
   }      

   inline Server::~Server(){
      delete[] m_host;
      delete[] m_url;
      delete[] m_hostAndPort;
   }
   
   inline const char * Server::getHostAndPort() const
   {
      return m_hostAndPort;
   }
   inline const char * Server::getUrl() const
   {
      return m_url;
   }

   inline bool Server::isReachable() const
   {
      return !m_unreachable;
   }
   inline int Server::group() const
   {
      return m_group;
   }
   inline void Server::setUnreachable()
   {
      m_unreachable = true;
   }

   inline bool Server::operator<(const Server& rhs) const
   {
      return strcmp(this->m_hostAndPort, rhs.m_hostAndPort) < 0;
   }

   inline bool Server::operator==(const Server& rhs) const
   {
      return 0 == strcmp(this->m_hostAndPort, rhs.m_hostAndPort);
   }
   //============= end of Server inlines ==============================

   class ServerList{
      typedef std::vector<Server*> container;
      typedef std::vector<Server*>::iterator iterator;
      container m_list;
      iterator m_current;
   public:
      ServerList();
      ~ServerList();
      unsigned populate(Buffer& buf, int lastGroup, 
                        bool keepPersistent = false,
                        bool keepNonpersistent = true, int delim = ',', 
                        int groupDelim = ';');


      const Server* getServer(bool first = false);
      const char* getHost() const;
      uint16 getPort() const;
      const char* getHostAndPort() const;

      unsigned size() const;
      bool empty() const;

      void setUnreachable();
      void removeCurrent();
      void setAllReachable();

      bool addServer(const char* host, uint16 port, 
                     bool persistent = false, int group = -1);
      void clear();

      uint32 saveParameter(ParameterProviderPublic& ppp);
   private:
      ServerList(const ServerList& rhs);
      const ServerList& operator=(const ServerList& rhs);
   };

   //================ ServerList inlines =============================

   inline ServerList::ServerList()
   {
      m_current = m_list.end();
   }

   inline ServerList::~ServerList()
   {
      while(!m_list.empty()){
         Server* tmp = m_list.back();
         m_list.pop_back();
         delete tmp;
      }
   }

   inline unsigned ServerList::populate(Buffer& buf, int lastGroup, 
                                        bool /*keepPersistent*/, 
                                        bool /*keepNonpersistent*/,
                                        int /*delim*/, int /*groupDelim*/)
   {
      bool end = m_current == m_list.end();
      unsigned cnt = 0;
      uint8 next = '\0';
      do{
         Server* tmp = Server::factory(buf, lastGroup);
         if(!tmp){
            break;
         }
         m_list.push_back(tmp);
         cnt++;
         next = buf.readNext8bit();
         if(next == ';'){
            next = ',';
            lastGroup++;
         }
      }while(next == ',');
      if(end){
         m_current = m_list.end();
      }
      return cnt;
   }

   inline const Server* ServerList::getServer(bool first)
   {
      Server* ret = NULL;
      if(first){
         //XXX
      } else if(m_current != m_list.end()){
         ret = *m_current;
      }
      return ret;
   }

   inline const char* ServerList::getHost() const
   {
      if(m_current != m_list.end()){
         return (*m_current)->getHost();
      }
      return NULL;
   }

   inline uint16 ServerList::getPort() const
   {
      if(m_current != m_list.end()){
         return (*m_current)->getPort();
      }
      return 0;
   }

   inline const char* ServerList::getHostAndPort() const
   {
      if(m_current != m_list.end()){
         return (*m_current)->getHostAndPort();
      }
      return NULL;
   }


   inline unsigned ServerList::size() const
   {
      return m_list.size();
   }
   
   inline bool ServerList::empty() const
   {
      return m_list.empty();
   }

   inline void ServerList::setUnreachable()
   {
      if(m_current != m_list.end()){
         (*m_current)->setUnreachable();
         //XXX get next
      }
   }
    
   inline void ServerList::removeCurrent()
   {
   }
   
   inline void ServerList::setAllReachable()
   {
      for(iterator q = m_list.begin(); q != m_list.end(); ++q){
         (*q)->setReachable();
      }
      //XXX get next
   }

   inline bool ServerList::addServer(const char* /*host*/, uint16 /*port*/, 
                                     bool /*persistent*/, int /*group*/)
   {
      return false;
   }

   inline void ServerList::clear()
   {
   }
   
   inline uint32 ServerList::saveParameter(ParameterProviderPublic& /*ppp*/)
   {
      return 0;
   }


   //===========end of ServerList inlines =========================
   class ServerCmpLess : 
      public std::binary_function<const Server* const&, const Server* const&, bool>{
   public:
      result_type operator()(first_argument_type a, 
                             second_argument_type b)const
      {
         return strcmp(a->getHostAndPort(), b->getHostAndPort()) < 0;
      }
   };

   class ServerCmpEqual :
      public std::binary_function<const Server* const&, const Server* const&, bool>{
   public:
      result_type operator()(first_argument_type a, 
                             second_argument_type b) const
      {
         return (0 == strcmp(a->getHostAndPort(), b->getHostAndPort()));
      }
   };

   ///Sorts Servers by group and reachability. 
   ///The first sorting criteria is group.
   ///The second sorting criteria is reachability, where reachables
   /// preceedes unreachebles.
   ///If two servers have the same group and reachablity they are
   /// considered equal.
   class ServerGroupLess :
      public std::binary_function<const Server* const&, const Server* const&, bool>{
   public:
      result_type operator()(first_argument_type a, 
                             second_argument_type b) const
      {
         if(a->group() == b->group()){
            if((a->isReachable() && b->isReachable()) ||
               !(a->isReachable() || b->isReachable())){
               return false;
            } else {                    //a is reachable, b is not =>
               return a->isReachable(); //a preceedes b
            }
         } else {
            return a->group() < b->group();
         }
      }
   };

   class ServerGroupLessStrict :
      public std::binary_function<const Server* const&, const Server* const&, bool>{
   public:
      result_type operator()(first_argument_type a, 
                             second_argument_type b) const
      {
         return a->group() < b->group();
      }
   };




   int nextServer(int64& seed, std::vector<Server*>& v, int oldIndex = -1);


   inline int findFirstServer(std::vector<Server*>& v, int64& seed){
      return nextServer(seed, v);
   }

   inline void setReachable(std::vector<Server*>& v){
      std::vector<Server*>::iterator q;
      for(q = v.begin(); q != v.end(); ++q){
         (*q)->setReachable();
      }
   }

   inline void removeServer(std::vector<Server*>& v, int idx){
      std::vector<Server*>::iterator toRemove = v.begin();
      std::advance(toRemove, idx);
      Server* bad = *toRemove;
      v.erase(toRemove);
      delete bad;
   }

}
#endif
