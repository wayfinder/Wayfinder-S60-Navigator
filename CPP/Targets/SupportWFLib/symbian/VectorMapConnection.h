/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VECTOR_MAP_CONNECTION_H
#define VECTOR_MAP_CONNECTION_H
#include <e32base.h>
#include "DBufConnection.h"

//#include "GuiProt/GuiProtMess.h"
namespace isab {
   class GuiProtMess;
   class DataGuiMess;
}

class CVectorMapConnection : public CBase, public DBufConnection{
public:
   ///Construction
   CVectorMapConnection(class MMessageSender* aSender);

   void SingleMapReply(const class isab::DataGuiMess& aReply);
   void MultiMapReply(const class isab::DataGuiMess& aReply);
   void MapErrorReply(uint16 aRequestId);
public:
   /** Inherited from <code>DBufConnection</code>. */
   //@{
   /**
    *    N.B! This function may Leave!
    *    Requests one databuffer. The value returned here should
    *    also be used when sending replies to the listeners via
    *    the HttpClientConnectionListener interface.
    *    Two requests must never return the same id and they should
    *    be unique for both requestOne an requestMany, i.e. if
    *    requestOne returns 1 then requestMany may not return that.
    *    @param descr Buffer to request.
    *    @return -1 if too many requests have been sent already.
    */
   virtual int requestOne(const char* descr);

   /**
    *    N.B! This function may Leave!
    *    Requests many databuffers. (e.g. using post in http).
    *    The value returned here should
    *    also be used when sending replies to the listeners via the
    *    HttpClientConnectionListener interface.
    *    @param body Body of data as described in SharedDBufRequester
    *                and ParserThread. (For multiple databufs).
    *    @return -1 if too many requests have been sent already.
    */
   virtual int requestMany(const uint8* buf, int nbrBytes);

   /**
    *    Sets listener.
    */
   virtual void setListener(class HttpClientConnectionListener* listener);
   //@}
private:

   class MMessageSender* iSender;
   class HttpClientConnectionListener* iListener;
};

#endif
