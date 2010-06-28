/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Output.h"
#include "TCPSocket.h"

isab::SocketOutput::SocketOutput(const char* threadName,
                                 const char* host, short port) :
   Output(), Thread(threadName),
   m_host(NULL), m_port(port), m_stop(false), m_reconnect(false)
{ 
   m_host = new char[strlen(host) + 1];
   strcpy(m_host, host);
   m_socket = new TCPSocket(5);
//   m_socket->create();
   m_socket->setupConnection(-1);
   m_socket->connect(m_host, m_port);
   start();
}

isab::SocketOutput::~SocketOutput()
{
   terminate();
   join();
   if (m_socket) {
      delete m_socket;
   }
   if(m_host) delete[] m_host;
}

size_t isab::SocketOutput::realPuts(const char* txt)
{
   size_t len = 0;
   if(m_socket){
      len = strlen(txt);
      if(m_socket->getState() == TCPSocket::CONNECTED){
         if (m_socket->write(reinterpret_cast<const uint8*>(txt), len) < 0) {
            /* Failed to write? */
            m_reconnect = true;
         }
      }
   }
   return len;
}

void isab::SocketOutput::run()
{
   lock();
   bool stop = m_stop;
   unlock();
    while(!stop){
      sleep(5*1000);
      lock();
      if (m_reconnect) {
         m_socket->close();
         delete m_socket;
         m_socket = new TCPSocket( 5 );
//         m_socket->create();
         m_socket->setupConnection(-1);
         m_socket->connect(m_host, m_port);
         m_reconnect = false;
      }
      stop = m_stop; 
      unlock();
   }
}

void isab::SocketOutput::terminate()
{
   lock();
   m_stop = true;
   unlock();
}

//============================================================

size_t isab::Output::puts(const char* txt)
{
   lock();
   size_t ret = realPuts(txt);
   unlock();
   return ret;
}

isab::Output::~Output()
{
}

//==============================================================

size_t isab::NullOutput::puts(const char* txt)
{
   txt = txt;
   return 0;
}


size_t isab::NullOutput::realPuts(const char* txt)
{
   txt = txt;
   return 0;
}
