/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "UnixMFDBufRequester.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <stdio.h>

UnixFileHandler::UnixFileHandler( const char* fileName, bool createFile ) :
      FileHandler( fileName ),
      m_file( NULL )
{
   if ( createFile ) {
      // Create the directory which should contain the file
      char* tmpName = new char[ strlen( fileName ) + 1 ];
      
      strcpy( tmpName, getFileName() );
      char* lastSlashPos = strrchr( tmpName, '/' );
      *lastSlashPos = '\0';
      
      char cmdStr[1024];
      sprintf(cmdStr, "mkdir -p %s", tmpName);
      
      FILE* filen = popen(cmdStr, "r");
      if ( filen ) {
         byte buf;
         fread( &buf, 1, 1, filen);
         pclose(filen);
      }
      
      delete [] tmpName;
      
      // Create the file
      m_file = fopen( fileName, "a+b");
      fclose(m_file);
   }
   // Open file for writing and reading.
   m_file = fopen( fileName, "r+b");
   m_working = 0;
}

int
UnixFileHandler::status() const
{
   if ( m_file == NULL ) {
      return -1;
   } else {
      return 0;
   }
}

UnixFileHandler::~UnixFileHandler()
{
   if ( m_file ) {
      fclose(m_file);
   }
}

void
UnixFileHandler::clearFile()
{
   fclose( m_file );
   mc2dbg << "[UFH]: Removing file " << getFileName() << endl;
   // Clears the file.
   remove( getFileName() );
   // Create a new file.
   m_file = fopen( getFileName(), "a+b");
   fclose(m_file);
   // Open file for writing and reading.
   m_file = fopen( getFileName(), "r+b"); 
}

int
UnixFileHandler::read( uint8* bytes,
                       int maxLength,
                       FileHandlerListener* listener )
{
   MC2_ASSERT( m_working == 0 );
   m_working++;
   
   clearerr(m_file);
   fseek( m_file, 0L, SEEK_CUR );
   
   mc2dbg << "[UFH]: read: filepos = " << tell() << endl;
   
   /*mc2dbg << "[UFH]: read("
          << getFileName() << ","
          << bytes << "," << maxLength
          << "," << listener << ");" << endl; */
   int res = fread( bytes, maxLength, 1, m_file );
   /*mc2dbg << "[UFH]: res = " << res << endl; */
   if ( res == 1 ) {
      res = maxLength;
   }
   m_working--;
   if ( listener ) {
      listener->readDone( res );
      return 0;
   } else {
      return res;
   }
}

int
UnixFileHandler::write( const uint8* bytes,
                        int length,
                        FileHandlerListener* listener )
{
   MC2_ASSERT( m_working == 0);

   clearerr(m_file);
   fseek( m_file, 0L, SEEK_CUR );
   
   mc2dbg << "[UFH]: write: filepos = " << tell() << endl;
   
   m_working++;
   int res = fwrite( bytes, length, 1, m_file );
   if ( res == 1 ) {
      res = length;
   }
   fflush( m_file );
   m_working--;
   if ( listener ) {
      listener->writeDone( res );
      return 0;
   } else {
      return res;
   }
}

void
UnixFileHandler::setPos( int pos )
{
   if ( pos >= 0 ) {
      fseek( m_file, pos, SEEK_SET);
   } else {
      fseek( m_file, 0, SEEK_END );
   }
}

void
UnixFileHandler::setSize( int size )
{
   // Get the fd for the file
   int fd = fileno( m_file );
   MC2_ASSERT( fd >= 0 );
   // Flush the file
   int fl = fflush( m_file );
   MC2_ASSERT( fl >= 0 );
   // Then truncate it.
   int tr = ftruncate( fd, size );
   MC2_ASSERT( tr >= 0 );   
}

int
UnixFileHandler::tell()
{
   return ftell(m_file);
}

uint32
UnixFileHandler::getModificationDate() const
{
   struct stat myStat;
   int res = stat( getFileName(), &myStat );
   if ( res == 0 ) {
      return uint32(myStat.st_mtime);
   } else {
      return MAX_UINT32;
   }
}

uint32
UnixFileHandler::getAvailableSpace() const
{
   return uint32(MAX_UINT32) >> 4;
}
