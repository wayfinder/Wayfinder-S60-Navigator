/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTPDBUFREQUESTER_H
#define HTTPDBUFREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include <map>

#include <vector>
#include <map>

#include <stdio.h>

class TileMapParams;
class TileMap;
class BitBuffer;
class DBufRequestListener;

class HttpDBufRequester : public DBufRequester {
public:
   /**
    *   Creates a new HttpDBufRequester which will use the supplied
    *   baseURL to and add the desc-strings to it when requesting stuff.
    */
   HttpDBufRequester(const char* baseURL);

   /**
    *   Deletes the HttpDBufRequester.  
    */
   virtual ~HttpDBufRequester();
   
   /**      
    *   Requests a TileMap or TileMapDescriptionFormat and
    *   calls caller->dataBufferReceived directly.
    */
   virtual void request(const MC2SimpleString& descr,
                        DBufRequestListener* caller,
                        request_t whereFrom );

   /**
    *   Releases the BitBuffer.
    */
   virtual void release(const MC2SimpleString& descr,
                        BitBuffer* dataBuffer);
   
   /**
    *   Does nothing.
    */
   virtual void cancelAll();

protected:
   /**
    *   Returns a file where the map can be read or NULL.
    */
   FILE* openFile(const char* paramString);

   /**
    *   Reads all data from the file and closes it.
    *   @return NULL if errro.
    */
   BitBuffer* readAllFromFileAndClose(FILE* file, const char* paramString);
   
   /**
    *   Converts the vector of bytes into a databuffer.
    *   @return NULL if error on the file.
    */
   BitBuffer* getBitBuffer(FILE* file, vector<byte>& bytes);

   /**
    *   BitBuffers owned by this requester.
    */
   map<MC2SimpleString, BitBuffer*> m_buffers;

private:

   /**
    *   The base url.
    */
   char* m_baseURL;

   /**
    *   The total number of bytes loaded through this requester.
    */
   uint32 m_totalBytes;

};
#endif
