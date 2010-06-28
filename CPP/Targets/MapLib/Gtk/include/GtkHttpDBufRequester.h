/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GTKHTTPDBUFREQUESTER_H
#define GTKHTTPDBUFREQUESTER_H

#include "config.h"

#include <gtk/gtk.h>


#include <map>
#include <vector>
#include "MC2SimpleString.h"
#include <set>

#include "HttpDBufRequester.h"

class MC2SimpleString;

class GtkHttpConnData {
public:
   /**
    *   Constructor.
    */
   GtkHttpConnData(DBufRequestListener* listener,
                   FILE* file,
                   const MC2SimpleString& parm,
                   guint conn)
         : m_listener(listener), m_file(file),
      m_paramString(parm),m_connection(conn),
      m_done(false) {}
                   
   /**
    *   The listener of the connection.
    */
   DBufRequestListener* m_listener;

   /**
    *   The file of the connection.
    */
   FILE* m_file;

   /**
    *   The parameter string used.
    */
   MC2SimpleString m_paramString;

   /**
    *   Vector of bytes which are read so far.
    */
   vector<byte> m_bytes;
   
   /**
    *   The event id, which signals if there are bytes to read.
    */
   guint m_connection;

   /**
    *   True if nothing more can be read.
    */
   bool m_done;
};

class GtkHttpDBufRequester : public HttpDBufRequester {
public:
   /**
    *   Creates a new GtkHttpDBufRequester which will use the supplied
    *   baseURL to and add the desc-strings to it when requesting stuff.
    */
   GtkHttpDBufRequester(const char* baseURL);

   /**
    *   Deletes the GtkHttpDBufRequester.  
    */
   virtual ~GtkHttpDBufRequester();

   /**      
    *   Requests a TileMap or TileMapDescriptionFormat.
    *   Returns immediately and relies on GTK to wake it
    *   up when there is something to read on the socket.
    */
   void request(const MC2SimpleString& descr,
                DBufRequestListener* caller,
                request_t whereFrom );

   /**
    *   Cancels all the requests that are possible to cancel.
    */
   void cancelAll();

private:
   /**
    *   Reads some data from the file.
    *   @return A DataBuffer or NULL. Done is set in data if done.
    */
   BitBuffer* readSomeFromFile(GtkHttpConnData& data);
   
   /**
    *   Called by GTK when there is something to read on the fd.
    */
   void readCallback(int id);

   static gboolean 
      gdkReadCallback( GIOChannel *channel, 
                       GIOCondition condition, gpointer data );

   /**
    *    Maximum number of simultaneous requests.
    */
   static const unsigned int MAX_NBR_REQUESTS = 40;

   /**
    *   Type of vector for pending requests.
    */
   typedef vector<pair<MC2SimpleString, DBufRequestListener*> > pendingVect_t;
   
   /**
    *   Pending requests.
    */
   pendingVect_t m_pending;
   
   /**
    *    Files about to be read.
    */
   map<int, GtkHttpConnData> m_files;
};

#endif
