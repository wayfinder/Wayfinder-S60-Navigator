/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "GtkHttpDBufRequester.h"

#include "MC2SimpleString.h"

#include "BitBuffer.h" 

GtkHttpDBufRequester::GtkHttpDBufRequester(const char* baseURL)
      : HttpDBufRequester(baseURL)
{
}

GtkHttpDBufRequester::~GtkHttpDBufRequester()
{

}

BitBuffer*
GtkHttpDBufRequester::readSomeFromFile(GtkHttpConnData& data)
{
   BitBuffer* returnBuf = NULL;
   int maxRead = 1024; // Maximum number of bytes to read in each session.
   FILE* dataStream = data.m_file;
   while ( !feof(dataStream) && !ferror(dataStream) && ( maxRead-- ) ) {
      unsigned char dataByte;
      int res = fread(&dataByte, 1, 1, dataStream);
      if ( res != 1 ) {
         break;
      }
      if ( !feof(dataStream) && !ferror(dataStream) ) {
         data.m_bytes.push_back(dataByte);
      }
   }     

   if ( feof( data.m_file) || ferror( data.m_file ) && ( errno != EAGAIN )  ) {
      returnBuf = getBitBuffer(dataStream, data.m_bytes);
      data.m_done = true;
      if ( returnBuf ) {
         m_buffers[data.m_paramString] = returnBuf;
      }
      data.m_bytes.clear();
      if ( returnBuf ) {
         // Also remove the buffer from pending
         for( pendingVect_t::iterator it = m_pending.begin();
              it != m_pending.end();
              /**/ ) {
            if ( it->first == data.m_paramString ) {
               mc2dbg << "[GtkHttp]: Remove one pending" << endl;
               it = m_pending.erase(it);
            } else {
               ++it;
            }                      
         }
      }
   } else if ( errno == EAGAIN ) {
      clearerr(dataStream);
   }
   
   return returnBuf;
}

gboolean 
GtkHttpDBufRequester::
gdkReadCallback( GIOChannel *channel, 
                 GIOCondition condition, gpointer data ) {
   mc2dbg << "[" << __FUNCTION__ << "] "<<endl;
   if (condition & G_IO_IN) {    
      GtkHttpDBufRequester *req = static_cast<GtkHttpDBufRequester*>( data );
      mc2dbg << "channel fd = "<<g_io_channel_unix_get_fd(channel) <<endl;
      req->readCallback( g_io_channel_unix_get_fd(channel) );
   }

   if (condition & G_IO_ERR) {
      mc2dbg << "oops error ey?"<<endl;
   }

   return true;
}

void
GtkHttpDBufRequester::readCallback(int id)
{
   // Use the id instead of the fd.
   map<int, GtkHttpConnData>::iterator it = m_files.find(id);

   if ( it != m_files.end() ) {
      // Get information out of the data
      DBufRequestListener* listener = it->second.m_listener;      
      
      BitBuffer* buf = readSomeFromFile(it->second);

      if ( it->second.m_done ) {
         if ( buf == NULL ) {
            mc2dbg8 << "[GTKHTTP]: DONE Reading error" << endl;
         } else if ( buf != NULL ) {
            mc2dbg8 << "[GTKHTTP]: DONE Reading " << buf->getBufferSize()
                    << " bytes from socket" << endl;
         }
         listener->requestReceived(it->second.m_paramString, buf, *this);
         // Disconnect the connection.
         g_source_remove(it->second.m_connection);
         m_files.erase(it);
      } else {
         // There is more to read on the file.
      }
   } else {
      mc2dbg << "[readCallBack]: Cannot find " << id << endl;
   }
   
   // Check if there are requests pending and insert them as requests.
   while ( m_files.size() < MAX_NBR_REQUESTS && !m_pending.empty() ) {

      mc2dbg << "[readCallBack]: FÖRE " << m_pending.size() << " reqs pending"
             << endl;

      DBufRequestListener* caller = m_pending.front().second;
      MC2SimpleString params ( m_pending.front().first );
      m_pending.erase(m_pending.begin());

      mc2dbg << "[readCallBack]: EFTER "
             << m_pending.size() << " reqs pending"
             << endl;

      request(params, caller, cacheOrInternet );
   }
}

void
GtkHttpDBufRequester::cancelAll()
{
   m_pending.clear();
}
void
GtkHttpDBufRequester::request(const MC2SimpleString& paramString,
                              DBufRequestListener* caller,
                              request_t whereFrom )
{
   // FIXME: Remove this. This is a memory cache!!
   map<MC2SimpleString, BitBuffer*>::iterator it =
      m_buffers.find( paramString );

   if ( it != m_buffers.end() ) {
      caller->requestReceived(it->first, it->second, *this);
      return;
   }
   
   if ( m_files.size() >= MAX_NBR_REQUESTS ) {
      m_pending.push_back(make_pair(paramString, caller) );
      return;
   }

   if ( whereFrom == onlyCache ) {
      caller->requestReceived( paramString, NULL, *this );
      return;
   }
   
   FILE* curFile = openFile( paramString.c_str() );
   if ( curFile == NULL ) {
      caller->requestReceived(it->first, NULL, *this);
      return;
   }
   //!! TODO !! following code ...check with old
   return;

   int fd = fileno(curFile);

   // Set nonblocking non portable etiher
   fcntl(fd, F_SETFL, O_NONBLOCK);



   // Signal when there is something on the fd

   // attach a watch on IO channel
   GIOChannel *channel = g_io_channel_unix_new(fd);
   mc2dbg << "channel fd = "<<g_io_channel_unix_get_fd(channel) <<endl;
   mc2dbg << "real fd = "<< fd <<endl;
   guint event_id = g_io_add_watch( channel,
                                    GIOCondition(G_IO_IN | G_IO_ERR),
                                    GtkHttpDBufRequester::gdkReadCallback,
                                    static_cast<gpointer>(this));
                                    
   /*
   SigC::Slot3<void,gint,GdkInputCondition,int> tmp_slot =
      SigC::slot(*this,
                 &GtkHttpDBufRequester::readCallBack);



   // Pass the counter in the last parameter of the callback function.
   Gtk::InputSig::SlotType my_slot = SigC::bind( tmp_slot, counter);

   SigC::Connection conn =
      Gtk::Main::input.connect(my_slot,
                               fd,
                               GdkInputCondition(GDK_INPUT_READ|
                                                 GDK_INPUT_EXCEPTION));
   */
   m_files.insert(make_pair(fd,
                            GtkHttpConnData(caller, curFile, paramString,
                                            event_id) ) );
   

}
