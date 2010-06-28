/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDTESTER_H
#define SFDTESTER_H

#include "config.h"

#include "FileHandler.h"

#include "SingleFileDBufRequester.h"
#include "MC2SimpleString.h"
#include "BitBuffer.h"
#include "TileMapParams.h"

class SFDTester : public DBufRequestListener {
public:

   /**
    *   Creates the tester.
    */
   SFDTester( FileHandler* fh ) {
      TileMapParams p(MC2SimpleString("G+ASp6++Y"));
      m_requester = new SingleFileDBufRequester( NULL, fh );
   }

   virtual ~SFDTester() {
      delete m_requester;
   }

   void requestReceived( const MC2SimpleString& str,
                         BitBuffer* buf,
                         const DBufRequester& origin ) {
      cerr << "[SFDTester]: requestReceived "
           << str
           << " buffer = " << buf;
      if ( buf ) {
         cerr << " bufsize = " << buf->getBufferSize();
      }
      cerr << endl;
      m_requester->release( str, buf );
   }
   

   void find( const MC2SimpleString& str ) {
      m_requester->request( str, this, DBufRequester::cacheOrInternet );
   }
   
private:
   /// Requester
   DBufRequester* m_requester;
};

#endif
