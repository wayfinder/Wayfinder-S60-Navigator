/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FileDBufRequester.h"

#include "MC2SimpleString.h"
#include "BitBuffer.h"

#include <stdio.h>

#ifndef __SYMBIAN32__
#define PATH_SEPARATOR "/"
#define PATH_SEPARATOR_LENGTH 1
#else
#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_LENGTH 1
#endif

FileDBufRequester::FileDBufRequester(DBufRequester* parent,
                                     const char* path,
                                     uint32 maxMem)
      : DBufRequester(parent),
        m_path(path)
{

}


FileDBufRequester::~FileDBufRequester()
{
}

// Some kind of unix

MC2SimpleString
FileDBufRequester::getFileName(const MC2SimpleString& descr)
{
   char* fullFileName = new char[m_path.length() + descr.length() +
                                PATH_SEPARATOR_LENGTH + 3];
   strcpy(fullFileName, m_path.c_str());
   strcat(fullFileName, PATH_SEPARATOR);
   strcat(fullFileName, descr.c_str());
   MC2SimpleString retVal(fullFileName);
   delete [] fullFileName;
   return retVal;
}

BitBuffer*
FileDBufRequester::readFromFile(const MC2SimpleString& descr)
{
   MC2SimpleString fullFileName(getFileName(descr));
   FILE* file = fopen(fullFileName.c_str(), "r");

   BitBuffer* retBuf = NULL;
   
   if ( file ) {
      // Seek to the end of file
      int seekRes = fseek(file, 0, SEEK_END);
      if ( seekRes < 0 ) {
         fclose(file);
      }
      long fileSize = ftell(file);
      // Set back the position.
      seekRes = fseek(file, 0, SEEK_SET);
      retBuf = new BitBuffer(fileSize);
      // FIXME: How about some error checks?
      fread(retBuf->getBufferAddress(), fileSize, 1, file);
      fclose(file);
   } else {
      // No file yet.
   }
   
   return retBuf;
}


bool
FileDBufRequester::writeToFile(const MC2SimpleString& descr,
                               BitBuffer* buffer)
{
   MC2SimpleString fullFileName(getFileName(descr));

   {
      FILE* readFile = fopen(fullFileName.c_str(), "r");
      if ( readFile != NULL ) {
         // Already there
         mc2dbg8 << "[FDBR]: " << descr << " already on disk" << endl;
         fclose(readFile);
         return true;
      }
   }
#if defined (__unix__) || defined (__MACH__)
#define USE_TEMP_FILE
#endif
   
#ifdef USE_TEMP_FILE
   // Make temporary file in the same dir. (For e.g. server
   // to avoid two threads writing to the same file).
   char tempTemplate[1024];   
   sprintf(tempTemplate, "%stilecachXXXXXX", m_path.c_str());
   int tmpDesc = mkstemp(tempTemplate);
   MC2SimpleString tempName(tempTemplate);
   FILE* file = fdopen(tmpDesc, "w");
#else
   // Write directly to the file
   FILE* file = fopen(fullFileName.c_str(), "w");
#endif
      
   if ( file && (fwrite(buffer->getBufferAddress(),
                        buffer->getBufferSize(), 1, file) == 1 ) ) {
      mc2dbg8 << "[FDBR]: Wrote " << descr << " to disk" << endl;
#ifdef USE_TEMP_FILE
      // Rename the file to the correct name and hope it works.
      rename(tempName.c_str(), fullFileName.c_str());
#endif
      fclose(file);            
      file = NULL;
      return true;
   }
   if ( file ) {
      fclose(file);
   }
   return false;
}

BitBuffer*
FileDBufRequester::requestCached(const MC2SimpleString& descr)
{
   return readFromFile(descr);
}


void
FileDBufRequester::release(const MC2SimpleString& descr,
                             BitBuffer* buffer)
{
   if ( buffer != NULL ) {
      writeToFile(descr, buffer);
   }  
   // Since we create all our buffers from file we can
   // send up the stuff to the parent.
   DBufRequester::release(descr, buffer);
}

