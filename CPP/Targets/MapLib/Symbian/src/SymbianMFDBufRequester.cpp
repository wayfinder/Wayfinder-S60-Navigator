/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SymbianMFDBufRequester.h"
#include "MapUtility.h"
#include <utf.h>
#include <f32file.h>

void
SymbianFileHandler::init()
{
   // Open the file for reading. FIXME: Error checking.
   openFile();
   m_initialized = true;
   m_state = IDLE;
   if ( m_status == KErrNone ) {
      setPos( 0 );
      CActiveScheduler::Add(this);
   }
}

SymbianFileHandler::SymbianFileHandler( RFs& fileServer,
                                        const char* fileName,
                                        int createFile,
                                        int initNow,
                                        int readOnly )
    : FileHandler( fileName ),
      CActive( EPriorityNormal ),      
      //CActive( EPriorityHigh ),
      m_fileServer( fileServer ),
      m_dataPtr( NULL, 0 ),
      m_createFile( createFile ),
      m_readOnly( readOnly )
{
   m_status = KErrNone;

   // Convert the filename to unicode. Needed for lots of operations.
   m_fileName16 = MapUtility::utf8ToUnicode( fileName );

   m_initialized = false;
   if ( initNow ) {
      init();
   }
}

SymbianFileHandler::~SymbianFileHandler()
{
   if ( IsActive() ) {
      Cancel();
   }
   if ( status() == 0 && m_initialized ) {
      m_file.Close();
   }
   // Delete the HBufC
   delete m_fileName16;   
}

int
SymbianFileHandler::status() const
{
   if ( m_status == KErrNone ) {
      return 0;
   } else {
      return -1;
   }
}

bool
SymbianFileHandler::openFile()
{
   m_status = KErrNone;
   if ( m_createFile ) {
      // Create the directory where the file should be if it does not
      // exist.
      TParsePtrC parser( *m_fileName16 );
      // And make the dir
      m_status = m_fileServer.MkDirAll( parser.DriveAndPath() );
   }
   
   if( m_status == KErrNone || m_status == KErrAlreadyExists )
   // Try to open the file.
   {      
      m_status = m_file.Open( m_fileServer, *m_fileName16,
                              m_readOnly ? EFileRead | EFileShareReadersOnly : EFileWrite );
      if ( m_status != KErrNone ) {
         // Seems like we must close the file now.
         m_file.Close();
      } else {
         return true;
      }
   }
   
   // Try creating it if we should
   if ( m_createFile ) {
      // Now we will have read/write file even if we should open it
      // read only.
      m_status = m_file.Create( m_fileServer, *m_fileName16, EFileWrite );
   }
   
   return m_status == KErrNone;
}

void
SymbianFileHandler::clearFile()
{
   if ( ! m_initialized ) {
      init();
   }
   m_file.Close();
   // Delete file.
   m_fileServer.Delete( *m_fileName16 );
   // And create empty file.
   m_status = m_file.Create( m_fileServer, *m_fileName16, EFileWrite );
}

int
SymbianFileHandler::read( uint8* bytes,
                          int maxLength,
                          FileHandlerListener* listener )
{
   if ( ! m_initialized ) {
      init();
   }
   
   m_state = READING;
   m_dataPtr.Set( bytes, maxLength, maxLength );
   m_currentListener = listener;
   
   
   if ( listener ) {
      m_file.Read( m_dataPtr, iStatus );
      SetActive();
      return 0;
   } else {
      TRequestStatus status;
      m_file.Read( m_dataPtr, status );
      User::WaitForRequest( status );
      m_state = IDLE;
      if ( status == KErrNone ) {
         return m_dataPtr.Length();
      } else {
         return -1;
      }
   }
}

int
SymbianFileHandler::write( const uint8* bytes,
                           int length,
                           FileHandlerListener* listener )
{
   if ( ! m_initialized ) {
      init();
   }
   m_state = WRITING;
   m_dataPtr.Set( const_cast<unsigned char*>(bytes), length, length );
   m_currentListener = listener;
   if ( listener ) {
      m_file.Write( m_dataPtr, iStatus );
      SetActive();
      return 0;
   } else {
      TRequestStatus status;
      m_file.Write( m_dataPtr, status );
      User::WaitForRequest( status );
      m_state = IDLE;
      if ( status == KErrNone ) {
         return length;
      } else {
         return -1;
      }
   }
}

void
SymbianFileHandler::cancel()
{
   if ( IsActive() ) {
      Cancel();
   }
}


void
SymbianFileHandler::setPos( int pos )
{   
   if ( ! m_initialized ) {
      init();
   }
   if ( pos >= 0 ) {
      int tmpPos = pos;
      m_file.Seek( ESeekStart , tmpPos );
   } else {
      int tmpPos = 0;
      m_file.Seek( ESeekEnd, tmpPos );
   }
}
   
void 
SymbianFileHandler::setSize( int size )
{
   if ( ! m_initialized ) {
      init();
   }

   m_file.SetSize( size );
}

int
SymbianFileHandler::tell()
{
   if ( ! m_initialized ) {
      init();
   }
   int tmpPos = 0;
   m_file.Seek( ESeekCurrent, tmpPos );
   return tmpPos;
}

uint32
SymbianFileHandler::getModificationDate() const
{
   // The day the unix time starts. Used to get the time down
   // to a reasonable value.
   TDateTime epoch( 1970, EJanuary, 01, 0 , 0 , 0 , 0);
   TTime aTime( epoch );
   TInt res = m_fileServer.Modified( *m_fileName16, aTime );
   if ( res != KErrNone ) {
      return MAX_UINT32;
   }
   TTimeIntervalMicroSeconds interval = aTime.MicroSecondsFrom( epoch );   
   return LOW( interval.Int64() / 1000000 );
}

uint32
SymbianFileHandler::getAvailableSpace() const
{
   // Sometimes Symbian makes life tricky.
   TChar driveChar = (*m_fileName16)[0];
   TInt currentDrive;
   int ctdRes = m_fileServer.CharToDrive( driveChar, currentDrive);
   if ( ctdRes != KErrNone ) {
      // Something went wrong. Use the default drive.
      currentDrive = KDefaultDrive;
   }
   TVolumeInfo volume;
   TInt res = m_fileServer.Volume( volume, currentDrive );
   if ( res != KErrNone ) {
      return MAX_UINT32;
   }
   // Check if we have _a lot_ of space.
   if ( volume.iFree > TInt64(TInt(MAX_INT32)) ) {
      return MAX_INT32;
   } else {
      return LOW(volume.iFree);
   }
}

int 
SymbianFileHandler::getFileSize() 
{
   if ( m_initialized ) {
      // It seems to be faster to use FileHandler::getFileSize()
      // (using Seek) if the file is already opened.
      return FileHandler::getFileSize();
   } else {
      // Get the filesize without opening the file to reduce the
      // startup delay.
      TEntry fileEntry;
      if ( m_fileServer.Entry( *m_fileName16, fileEntry ) != KErrNone ) {
         // Ouch. Didn't work.
         return 0;
      }
      return fileEntry.iSize;
   }
}

void
SymbianFileHandler::RunL()
{
   if ( iStatus != KErrNone ) {
   }
   switch ( m_state ) {
      case IDLE:
         break;
      case READING:
         m_state = IDLE;
         if ( iStatus == KErrNone ) {
            m_currentListener->readDone( m_dataPtr.Length() );
         } else {
            m_currentListener->readDone( -1 );
         }
         break;
      case WRITING:
         m_state = IDLE;
         if ( iStatus == KErrNone ) {
            m_currentListener->writeDone( m_dataPtr.Length() );            
         } else {
            m_currentListener->writeDone( -1 );
         }
   }
}

void
SymbianFileHandler::DoCancel()
{
}

// -- SymbianMFDBufRequester

const char*
SymbianMFDBufRequester::createUTF8Path( const TDesC& unicodePath )
{
   // Paths may be max 256 in Symbian.
   TBuf<256> unicodePathBuf;
   unicodePathBuf.Copy( unicodePath );  
   TBuf8<256> aUtf8;
   CnvUtfConverter::ConvertFromUnicodeToUtf8(aUtf8,
                                             unicodePathBuf);
   const char* tmpPtr = reinterpret_cast<const char*>(aUtf8.PtrZ());
   
   m_utf8Path = new char[ strlen(tmpPtr) + 1 ];
   strcpy(m_utf8Path, tmpPtr);
   // Remove trailing slashes.
   while ( m_utf8Path[ strlen( m_utf8Path ) - 1 ]  == '\\' ) {
      m_utf8Path[ strlen( m_utf8Path ) - 1 ] = '\0';
   }
   return m_utf8Path;
}
