/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GtkTCPConnectionHandler.h"
#include "DBufRequester.h"
#include "SharedHttpDBufRequester.h"
#include "MapClientApp.h"

#include <gtk/gtk.h>

#include <iostream>
#include <cstdio>

using namespace std;



int main( int   argc, char **argv) {


   // parse arguments
   if ( argc < 3 ) {
      cerr << "[Usage]: " << argv[0] << " [-s session] host port" << endl;
      exit(1);
   }
   // Initialize GTK+
   gtk_init(&argc, &argv);
   
   const char* urlParams = "";

   for( int i=1; i < argc; ++i ) {
      if ( strcmp(argv[i], "-s" ) == 0 ) {
         if ( i+1 < argc ) {
            urlParams = argv[i+1];
            ++i;
         }
      }
   }
   

   const char* host = argv[argc-2];
   const int port = atoi(argv[argc-1]);

   // setup TCP connection
   GtkTCPConnectionHandler connHandler;
   HttpClientConnection clientConn(host, port, 
                                   NULL, // no listener
                                   &connHandler);

   SharedHttpDBufRequester* req2 =
      new SharedHttpDBufRequester(&clientConn, "/TMap", urlParams);

   // setup main window
   MapClientApp app(req2);
   app.loop();


   return 0;
}
