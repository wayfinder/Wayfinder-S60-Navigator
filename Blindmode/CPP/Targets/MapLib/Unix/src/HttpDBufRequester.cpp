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

#include "HttpDBufRequester.h"

#include "TileMapParams.h"
#include "TileMapFormatDesc.h"
#include "TileMap.h"
#include "BitBuffer.h"

#include "MC2SimpleString.h"

#include <stdio.h>

HttpDBufRequester::HttpDBufRequester(const char* baseURL)
{
   int memlen = strlen(baseURL) + 1;
   m_baseURL    = new char[memlen];
   memcpy(m_baseURL, baseURL, memlen);
   m_totalBytes = 0;
}

HttpDBufRequester::~HttpDBufRequester()
{
   delete [] m_baseURL;
   // FIXME: Do not cache in this class.
   for( map<MC2SimpleString, BitBuffer*>::iterator it = m_buffers.begin();
        it != m_buffers.end();
        ++it ) {
      delete it->second;
   }
}

FILE*
HttpDBufRequester::openFile(const char* paramString)
{
   char* cmdBuf = new char[1024];
   const char* userAgent = "-U HttpDBufRequester";
   // Put in the wget-command
   sprintf(cmdBuf, "wget -q %s -S -O - %s%s", userAgent,
           m_baseURL, paramString);

   // Try to run it
   FILE* file = popen(cmdBuf, "r");
   delete [] cmdBuf;
   return file;
}

BitBuffer*
HttpDBufRequester::getBitBuffer(FILE* dataStream,
                                 vector<byte>& bytes)
{
   BitBuffer* returnBuf = NULL;
   
   if ( !ferror( dataStream ) && feof( dataStream ) &&
        bytes.size() != 0 ) {
      int pcloseVal = pclose(dataStream);
      if ( pcloseVal == 0 ) {
         mc2dbg << "[HttpDBufRequester]: Read "
                << bytes.size() << " bytes" << endl;
         m_totalBytes += bytes.size();
         mc2dbg << "[HttpDBufRequester]: Read total :"
                << m_totalBytes << endl;
         // Copy the contents into a DataBuffer.
         BitBuffer* saveBuf = new BitBuffer(bytes.size());
         for( vector<byte>::const_iterator it = bytes.begin();
              it != bytes.end();
              ++it ) {
            saveBuf->writeNextByte(*it);
         }
         saveBuf->reset();
         saveBuf->setSelfAlloc(false);
         returnBuf = saveBuf;
      } else {
         mc2log << error << "[HttpDBufRequester]: pclose returned "
                << pcloseVal << endl;
      }
   } else {
      pclose(dataStream);
      mc2log << error << "[HttpDBufRequester]: Error while loading" << endl;
      mc2log << error << "[HttpDBufRequester]: " << bytes.size()
             << " bytes read" << endl;
   }

   return returnBuf;
}

BitBuffer*
HttpDBufRequester::readAllFromFileAndClose(FILE* dataStream,
                                           const char* paramString)
{
   vector<byte> bytes;
   
   while ( !feof(dataStream) && !ferror(dataStream) ) {
      unsigned char dataByte;
      fread(&dataByte, 1, 1, dataStream);
      if ( !feof(dataStream) && !ferror(dataStream) ) {
         bytes.push_back(dataByte);
      }
   }     
      
   BitBuffer* returnBuf = getBitBuffer(dataStream, bytes);
   
   if ( returnBuf ) {
      m_buffers[paramString] = returnBuf;
   }

   return returnBuf;
}

void
HttpDBufRequester::request(const MC2SimpleString& paramString,
                           DBufRequestListener* caller,
                           request_t whereFrom )
{   
   // FIXME: Remove this. This is a memory cache!!
   map<MC2SimpleString, BitBuffer*>::iterator it = m_buffers.find( paramString );
   if ( it != m_buffers.end() ) {
      caller->requestReceived(it->first.c_str(), it->second, *this);
      return;
   }

   if ( whereFrom == onlyCache ) {
      caller->requestReceived( paramString, NULL, *this );
      return;
   }
      
   FILE* dataStream = openFile(paramString.c_str());
   
   if ( dataStream == NULL ) {
      return;
   }

   BitBuffer* returnBuf = readAllFromFileAndClose(dataStream,
                                                   paramString.c_str());
   
   if ( returnBuf ) {
      map<MC2SimpleString, BitBuffer*>::iterator it = 
        m_buffers.find( paramString );
      caller->requestReceived(it->first, it->second, *this);
      m_buffers.erase(it);
   } else {
      map<MC2SimpleString, BitBuffer*>::iterator it =
         m_buffers.find( paramString );
      m_buffers.erase(it);
      caller->requestReceived(it->first, NULL, *this);
   }
}

void
HttpDBufRequester::cancelAll()
{
   // We only have one request. Cannot cancel it.
}

void
HttpDBufRequester::release( const MC2SimpleString& descr, BitBuffer* buf )
{
   mc2dbg8 << "[HttpDBufRequester]: Releasing buf 0x"
           << hex  << buf << dec << endl;
   buf->reset();
}
