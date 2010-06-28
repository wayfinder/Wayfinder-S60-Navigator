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

#include "MultiFileDBufRequester.h"
#include "MFileDBufReqIdxHandler.h"
#include "MemoryDBufRequester.h"
#include "BitBuffer.h"
#include <stdio.h>
#include <algorithm>
#include "TileMapParams.h"
#include "TileMapParamTypes.h"
#include "TileMapUtil.h"

#include "ParamsNotice.h"

using namespace std;

static inline MC2SimpleString removeTrailingPathSep( const char* orig,
                                                     const char* pathSep )
{
   char* newOrig = TileMapUtil::newStrDup( orig );
   char* origOrg = newOrig;

   int pathSepLen = strlen( pathSep );
   char* pathSepRev = TileMapUtil::newStrDup( pathSep );
   
   int origLen = strlen( newOrig );
   // Reverse the strings
   std::reverse( newOrig, newOrig + origLen );
   std::reverse( pathSepRev, pathSepRev + pathSepLen );

   // Skip path separators if the begin the string.
   while ( strstr( newOrig, pathSepRev ) == newOrig ) {
      newOrig += pathSepLen;
   }

   // Reverse back.
   std::reverse( newOrig, newOrig + strlen( newOrig ) );
   memmove( origOrg, newOrig, strlen( newOrig ) + 1 );

   delete [] pathSepRev;
   return MC2SimpleStringNoCopy( origOrg );
}

MultiFileDBufRequester::MultiFileDBufRequester( DBufRequester* parent,
                                                const char* basepath,
                                                MemTracker* memTracker,
                                                uint32 maxSize,
                                                int nbrFiles, 
                                                int nbrHash,
                                                int readOnly )
      : DBufRequester(parent), m_readOnly( readOnly ),
        m_currentlyReading( "", NULL, DBufRequester::cacheOrInternet ),
        m_memTracker( memTracker )
{
   m_containsBitmaps = true;
   m_memTracker->addMemEater( this );
   // Alloc the dumpbuffer already so that we know that we have
   // it when shutting down.
   if ( ! m_readOnly ) {
      m_dumpBuffer = new BitBuffer(10*1024);
      m_requesterType = PERMANENT;
   } else {
      m_dumpBuffer = NULL;
      m_requesterType = PERMANENT | READ_ONLY;
   }
   m_basePath   = basepath;

   // Check the nbrFiles & nbrHash parameters.
   m_nbrFiles   = ( nbrFiles > 0 ) ? nbrFiles : 5;
   m_nbrHash    = ( nbrHash  > 0 ) ? nbrHash  : 23;
   m_nbrFiles   = MAX( m_nbrFiles, 1 );
   m_nbrHash    = MAX( m_nbrHash, 1 );
   // Don't call setMaxSize until the filehandlers are created.
   m_maxSize = maxSize;
   // Make preliminary calculation of the max size per file
   m_maxSizePerFile = maxSize / m_nbrFiles;
   m_state = IDLE;
   m_lastWrittenFileNbr = 0;
   m_indexHandler = NULL;
#ifdef NAV2_CLIENT_SERIES60_V1
   m_writeQueue = new MemoryDBufRequester(NULL, 64*1024);
#else
   m_writeQueue = new MemoryDBufRequester(NULL, 256*1024);
#endif
   //m_writeQueue = new MemoryDBufRequester(NULL, 50*1024*1024);
   m_readBuffer = NULL;
   // Set to 31 so that setMaxSize is called the first time writeDone.
   m_nbrWrites = 31;
   m_shuttingDown    = false;
   m_inStartWriting  = 0;
   m_inStartFinding  = 0;
   m_inStartRemoving = 0;
   m_nbrErrorFiles   = 0;
   m_xorBuffer = NULL;
   m_readOnlyMisses = 0; // Counts number of readonly cache misses.
}

MultiFileDBufRequester::~MultiFileDBufRequester()
{
   m_shuttingDown = true;
   if ( m_indexHandler ) {
      m_indexHandler->shutDown();
   }

   if ( m_currentlyWriting.second != NULL ) {
      m_writeQueue->release( m_currentlyCorrectLang,
                             new BitBuffer( *m_currentlyWriting.second ) );
   }
   
   if ( ! m_readOnly ) {
      writeRemainders();
   } else {

      static const int bufSize = 10*1024;
      uint8* tmpBuf = new uint8[bufSize];
      if ( tmpBuf != NULL ) {
         m_dumpBuffer = new BitBuffer( tmpBuf, bufSize );
         writeRemainders();
         delete tmpBuf;
         delete m_dumpBuffer;
         m_dumpBuffer = NULL;
      } else {
         // No memory
         // Send the writeQueue to the next requester.
         // The last one will have allocated the buffer from the beginning
         while ( ! m_writeQueue->empty() ) {
            m_currentlyWriting = m_writeQueue->front();
            m_writeQueue->pop_front();
            DBufRequester::release( m_currentlyWriting.first,
                                    m_currentlyWriting.second );
         }
      }
      
   }
   
   for ( uint32 i = 0; i < m_dataFiles.size(); ++i ) {
      m_dataFiles[i]->cancel();
      delete m_dataFiles[i];
   }
   {for ( uint32 i = 0; i < m_indexFiles.size(); ++i ) {
      m_indexFiles[i]->cancel();
      delete m_indexFiles[i];
   }}
   delete m_dumpFile;
   delete m_readBuffer;
   delete m_indexHandler;
   delete m_writeQueue;
   delete m_dumpBuffer;
   m_memTracker->removeMemEater( this );
}

inline bool
MultiFileDBufRequester::okToWriteEvenIfItExists( const MC2SimpleString& descr )
{
   // This method knows too much about tilemaps
   // Well. It is ok to save DXXX:s.
   return TileMapParamTypes::getParamType( descr ) ==
      TileMapParamTypes::FORMAT_DESC;
}

inline int
MultiFileDBufRequester::status() const
{
   return m_nbrErrorFiles;
}

void
MultiFileDBufRequester::clearFileAndIndex(int nbr)
{
   for ( int i = 0; i < m_nbrHash; ++i ) {
      m_indexFiles[nbr*m_nbrHash + i]->clearFile();
   }
   m_dataFiles[nbr]->clearFile();
   m_indexHandler->clearIndex(nbr);
}

uint32
MultiFileDBufRequester::getAvailableSpace() const
{
   uint32 tmpVal = m_indexFiles[0]->getAvailableSpace();
   return tmpVal;
}

int32
MultiFileDBufRequester::getTotalCacheSize() const
{
   uint32 sum = 0;
   for ( uint32 i = 0; i < m_dataFiles.size(); ++i ) {
      sum += m_dataFiles[i]->getFileSize();
   }
   {for ( uint32 i = 0; i < m_indexFiles.size(); ++i ) {
      sum += m_indexFiles[i]->getFileSize();
   }}
   return sum;
}

int
MultiFileDBufRequester::decIndex(int oldIndex) const
{
   int fileIndex = oldIndex - 1;
   if ( fileIndex < 0 ) {
      fileIndex = m_nbrFiles - 1;
   }
   return fileIndex;
}

int
MultiFileDBufRequester::incIndex(int oldIndex) const
{
   int fileIndex = oldIndex + 1;
   if ( fileIndex >= m_nbrFiles ) {
      fileIndex = 0;
   }
   return fileIndex;
}

int
MultiFileDBufRequester::isBeingRemoved( const MC2SimpleString& desc )
{
   // Really means if the desc is in the remove queue.
   return ( m_buffersToRemove.find( desc ) != m_buffersToRemove.end() );
}


uint32
MultiFileDBufRequester::clearOldestNotEmptyFile()
{   
   int fileIndex = incIndex(m_lastWrittenFileNbr);
   int maxNbr = m_nbrFiles;
   
   while ( maxNbr-- && ( getFileSize( fileIndex ) == 0  ) ) {
      fileIndex = incIndex(fileIndex);
   }
   
   uint32 deletedAmount = getFileSize( fileIndex );
  
   clearFileAndIndex(fileIndex); 
   return deletedAmount;
}

uint32
MultiFileDBufRequester::calcMaxAllowedCacheSize( uint32 wantedSize ) const
{
   static const int extraRoom = 512*1024;
   int32 alreadyUsed = getTotalCacheSize();
   // Include the space that we have already used in the calculation.
   // Adjust so that it is never negative.
   int32 available   = MAX( int32(getAvailableSpace()) + 
                            alreadyUsed - extraRoom ,
                            0 );
   // Return the minimum of the two.
   return MIN( wantedSize, uint32(available) );
}

void
MultiFileDBufRequester::setMaxSize(uint32 size)
{
   
   m_newSize = size;
   // Wait with setting the size until next time we're IDLE.
   if ( m_state == IDLE && !m_readOnly ) {
      // FIXME: Not detecting if we delete so much that we
      // arrive under the limit for the available disk space.
      // Set the max size to the sent in size. We will only
      // use m_maxSizePerFile to check when to delete a file
      m_maxSize = size;

      // Adjust the size for available space
      int nbrLeftToClear = m_nbrFiles;
      do {
         size = calcMaxAllowedCacheSize( m_maxSize );
         m_maxSizePerFile = size / m_nbrFiles + 1;
         if ( size == 0 || int(size) < getTotalCacheSize() ) {
            clearOldestNotEmptyFile();
         }
      } while ( ( (size == 0 ) || ( int(size) < getTotalCacheSize() ) )
                && (--nbrLeftToClear > 0 ) );
   }
}

void
MultiFileDBufRequester::clearCache()
{
   cancelAll();
   for ( int i = 0; i < m_nbrFiles; ++i ) {
      clearFileAndIndex( i );
   }
}

static inline MC2SimpleString
getIndexFileName(const MC2SimpleString& basePath,
                 int fileNbr,
                 int hashNbr,
                 const char* pathSep)
{
   char hashNbrBuf[16];
   sprintf( hashNbrBuf, "%d", hashNbr );
   char nbrBuf[16];
   sprintf( nbrBuf, "%d", fileNbr );

   // Very strange length calculation.
   int length = basePath.length() +
      strlen( pathSep ) * 2 +
      strlen("fili.") + 1 +
      strlen(nbrBuf) * 2 +
      strlen(hashNbrBuf);

   char* curIndexName = new char[length + 50];
   
   sprintf( curIndexName, "%s%s%s%s%s%s%s%s",
            basePath.c_str(),
            pathSep,
            nbrBuf,
            pathSep,
            "fili.",
            nbrBuf,
            "_",
            hashNbrBuf );
   return MC2SimpleStringNoCopy( curIndexName );
}

bool
MultiFileDBufRequester::init()
{
   // Fixup the path.
   m_basePath = removeTrailingPathSep( m_basePath.c_str(),
                                       getPathSeparator() );
   
   char* tmpPath = new char[ m_basePath.length() + 10 ];
   sprintf(tmpPath, "%s%s%s",
           m_basePath.c_str(), getPathSeparator(), "a" );
   // Will delete the tmpPath
   MC2SimpleStringNoCopy basePath(tmpPath);
   tmpPath = NULL;
   
   // Create the fileHandlers
   m_dataFiles.resize(m_nbrFiles);
   m_indexFiles.resize(m_nbrFiles*m_nbrHash);
   {
      for ( int i = 0; i < m_nbrFiles; ++i ) {
         char nbrBuf[16];
         sprintf(nbrBuf, "%d", i);
         int length = basePath.length() +
            strlen(getPathSeparator()) +
            strlen("fili.") +
            strlen(nbrBuf) + 50;
         char* curDataName = new char[ length ];
         sprintf( curDataName, "%s%s%s%s", basePath.c_str(),
                  getPathSeparator(), "fil.", nbrBuf );
         // Only create the file if it is not read only.
         m_dataFiles[i] = createFileHandler( curDataName,
                                             !m_readOnly );
         
         if ( m_dataFiles[i]->status() != 0 ) {
               if ( m_readOnly ) {
                  if ( i != 0 ) {
                     m_nbrErrorFiles = 0;
                  } else {
                     ++m_nbrErrorFiles;
                  }
                  // We have found how many files that were present.
                  m_nbrFiles = i;
                  // Adjust the nbr of files.
                  m_indexFiles.resize(m_nbrFiles*m_nbrHash);
                  m_dataFiles.resize(m_nbrFiles);
                  break;
               } else {
                  ++m_nbrErrorFiles;
               }
            ++m_nbrErrorFiles;
         }

         for ( int j = 0; j < m_nbrHash; ++j ) {
            MC2SimpleString indexName =
               getIndexFileName( basePath, i, j, getPathSeparator() );
            
            FileHandler* curFile = m_indexFiles[i * m_nbrHash + j ] = 
               // Only create the file if it is not read only.
               createFileHandler( indexName.c_str(), !m_readOnly, 
                                  false ); // Init later.
            if ( curFile->status() != 0 ) {
               ++m_nbrErrorFiles;
            }
         }         
         delete [] curDataName;
      }
   }

   // Create the dump file.
   m_dumpFile = NULL;
   {
      int length = basePath.length() +
         strlen(getPathSeparator()) +
         strlen("fil.dump") + 1;

      char* dumpFileName = new char[ length ];
      sprintf( dumpFileName, "%s%sfil.dump",
               basePath.c_str(),
               getPathSeparator() );
      // Create the dumpfile even if we have read only cache.
      m_dumpFile = createFileHandler( dumpFileName,  true  );
      delete [] dumpFileName;
   }  

   
   // Create file for initial maps
   FileHandler* initialDumpFile = NULL;
   if (! m_readOnly ){
      int length = basePath.length() +
         strlen(getPathSeparator()) +
         strlen("initial.dump") + 1;

      char* dumpFileName = new char[ length ];
      sprintf( dumpFileName, "%s%sinitial.dump", basePath.c_str(),
               getPathSeparator() );
      // Do not create the file if it does not exist.
      initialDumpFile = createFileHandler( dumpFileName, false );
      delete [] dumpFileName;
   }

   if ( m_readOnly ) {
      int length = basePath.length() +
         strlen(getPathSeparator()) +
         strlen("cacheinfo") + 1;

      char* cacheInfoFileName = new char[ length ];
      sprintf( cacheInfoFileName, "%s%scacheinfo", basePath.c_str(),
               getPathSeparator() );
      // Do not create the file if it does not exist.
      FileHandler* cacheInfoFile = 
            createFileHandler( cacheInfoFileName, false );   
      
      readCacheInfoFile( cacheInfoFile );
      delete [] cacheInfoFileName;
      delete cacheInfoFile;
   }
   
   // Create index handler
   m_indexHandler = new MFileDBufReqIdxHandler(this,
                                                           m_indexFiles, 
                                                           m_nbrHash,
                                                           m_xorBuffer,
                                                           m_memTracker );

   // Instead of setMaxSize. setMaxSize will be called first time in
   // writeDone().
   m_newSize = m_maxSize;
//   setMaxSize( m_maxSize );   
   // Find the newest file and keep adding to it
   uint32 newest = 0;
   uint32 modIdx = 0;
   for ( uint32 i = 0; i < m_dataFiles.size(); ++i ) {
      uint32 modTime = m_dataFiles[i]->getModificationDate();
      if ( modTime > newest ) {
         newest = modTime;
         modIdx = i;
      }
   }
   m_lastWrittenFileNbr = modIdx;

   readRemainders(initialDumpFile);
   readRemainders(m_dumpFile);

   delete initialDumpFile;
   
   return true;
}

void
MultiFileDBufRequester::setXorBuffer(const SharedBuffer* xorBuffer)
{
   m_xorBuffer = xorBuffer;
   m_indexHandler->setXorBuffer( xorBuffer );
}

inline void
MultiFileDBufRequester::setState( state_t newState )
{
   m_state = newState;
}

void
MultiFileDBufRequester::release( const MC2SimpleString& desc,
                                 BitBuffer* buffer )
{
   mc2log << "[MultiFileDBufRequester::release("
          << MC2CITE(desc) 
          << ")]" << endl;

   if ( status() != 0 || ( ! maybeInCache( desc ) ) ) {
      mc2log << "[MFDBR]: File not ok - sending to next requester" << endl;
      DBufRequester::release( desc, buffer );
      return;
   }
   
   if ( m_maxSizePerFile > 10000 || m_readOnly ) {
      // If the max file size is big enough, or we are read only
      // we should start writing, or in the case of read-only
      // check if it is already in the cache.
      m_writeQueue->release( desc, buffer );
   } else {
      if ( ( m_nbrWrites++ & 127 ) == 127 ) {
         // Check if there is available space on the disk again.
         setMaxSize(m_maxSize);
      }      
      DBufRequester::release( desc, buffer );
   }
   // Check if it is time to start working.   
   startWorking();
}

BitBuffer*
MultiFileDBufRequester::requestCached(const MC2SimpleString& descr)
{
   return m_writeQueue->requestCached(descr);
}

void
MultiFileDBufRequester::internalRemove( const MC2SimpleString& descr )
{
   if ( status() != 0 ) {
      return;
   }
   mc2log << "[MultiFileDBufRequester::internalRemove]" << endl;
   // Remove it from the write queue at once
   m_writeQueue->removeBuffer( descr );
   // Add it to the set of buffers that should be removed
   m_buffersToRemove.insert( descr );
   // Start working
   startWorking();
}

void
MultiFileDBufRequester::request(const MC2SimpleString& descr,
                                DBufRequestListener* caller,
                                request_t whereFrom )
{
   if ( status() != 0 || ( ! maybeInCache( descr ) ) ) {
      if ( m_parentRequester != NULL ) {
         m_parentRequester->request(descr, caller, whereFrom );
      }
      return;
   }

   
   
   mc2log << "[MultiFileDBufRequester::request("
          << MC2CITE(descr) 
          << ")]" << endl;

   BitBuffer* maybeCached = requestCached(descr);
   if ( maybeCached ) {
      caller->requestReceived( MC2SimpleString(descr), maybeCached,
                               *this );
      return;
   }
   // Check if the buffer is about to be removed
   if ( ! isBeingRemoved(descr) ) {
      
      // Not in memory - check queue.
      readJob_t thePair( descr, caller, whereFrom );

      // FIXME: Check currently reading too?
      if ( std::find( m_readQueue.begin(), m_readQueue.end(), thePair ) ==
           m_readQueue.end() ) {
         // Not in queue - push back
         m_readQueue.push_back( thePair );
         mc2log << "MFDBR: Requested desc: " << descr << endl;
              
      } else {
         mc2log << "[MFDBR]: " << descr << " already in queue" << endl;
      }
   }         
   // Start reading if not reading already.
   startWorking();
}

void
MultiFileDBufRequester::cancelAll()
{
   mc2log << "[MultiFileDBufRequester::cancelAll]" << endl;
   m_readQueue.clear();
   DBufRequester::cancelAll();
}

void
MultiFileDBufRequester::findComplete(const MC2SimpleString& desc,
                                     int fileNbr,
                                     int startOffset,
                                     int size )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == LOOKING_FOR_MAP ||
               m_state == WRITING_LOOKING_FOR_MAP );
   
   if ( m_state == LOOKING_FOR_MAP ) {
      mc2log << "[MFDBR]: Find complete " << endl;
      MC2_ASSERT( desc == m_currentlyReading.first );
      mc2log << "[MFDBR]: m_maxSizePerFile = " << m_maxSizePerFile << endl;

      setState( IDLE ); 
      if ( startOffset < 0 || size < 0 ) {
         // Not found - request from parent
         if ( m_parentRequester ) {
            MC2SimpleString tmp(m_currentlyCorrectLang);
            m_parentRequester->request(
               tmp,
               m_currentlyReading.second,
               m_currentlyReading.whereFrom );
         } else {
            mc2log << "[MFDBR]: No parent" << endl;
         }
         startWorking();
      } else {
         mc2log << "[MFDBR]: Map " << desc
                << " found in cache " << endl;
         // Continue by reading the stuff from the file.
         setState( READING_MAP );
         MC2_ASSERT( m_readBuffer == NULL );
         MC2_ASSERT( size != 0 );
         m_readBuffer = new BitBuffer( size );
         m_dataFiles[fileNbr]->setPos( startOffset );
         m_dataFiles[fileNbr]->read( m_readBuffer->getBufferAddress(),
                                     m_readBuffer->getBufferSize(),
                                     this );
      }
   } else {
      MC2_ASSERT( m_state == WRITING_LOOKING_FOR_MAP );
      // It is ok to write the map if it is not already in cache
      bool writeOK = ! ( ( startOffset >= 0 ) && (size >= 0 ) );

      // But some maps are ok to write anyway.
      if ( ! m_readOnly && 
           okToWriteEvenIfItExists( m_currentlyWriting.first ) ) {
         writeOK = true;
      }
      
      if ( !writeOK ) {
         setState( WRITING_INDEX );
         if ( !m_readOnly ) {
            mc2log << "[MFDBR]: Not writing map - "
                   << MC2CITE( m_currentlyWriting.first)
                   << " it was in one of the last files "
                   << endl;
            // Give the map to the next cache.
            DBufRequester::release( m_currentlyCorrectLang,
                                    m_currentlyWriting.second );
         } else {
            // Read only - delete map instead.
            mc2log << "[MFDBR]: Deleting map - "
                   << MC2CITE( m_currentlyWriting.first)
                   << " already in cache"
                   << endl;
            delete m_currentlyWriting.second;
         }
         m_currentlyWriting.second = NULL;
         indexWritten( m_currentlyWriting.first );
      } else {
         if ( ! m_readOnly ) {
            // The code that used to be in startWriting.
            mc2log << "[MFDBR]: Will start writing " << endl;
            setState( WRITING_MAP_DATA );         
            m_dataFiles[m_lastWrittenFileNbr]->setPos( -1 );
            m_startWritePos = m_dataFiles[m_lastWrittenFileNbr]->tell();
            m_dataFiles[m_lastWrittenFileNbr]->write(
               m_currentlyWriting.second->getBufferAddress(),
               m_currentlyWriting.second->getBufferSize(),
               this);
         } else {
            mc2log << "[MFDBR]: Read only releasing to next cache "
                   << endl;
            setState( WRITING_INDEX );
            // Read only. It was not found in the cache, and must
            // therefore be released to the next cache.
            DBufRequester::release( m_currentlyCorrectLang,
                                    m_currentlyWriting.second );
            m_currentlyWriting.second = NULL;
            indexWritten( m_currentlyWriting.first );
         }
      }                                    
   }
}

void
MultiFileDBufRequester::readDone( int nbrRead )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == READING_MAP );
   setState( IDLE );
   if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
      mc2log << "[MFDBR]: Failed to read map" << endl;
      delete m_readBuffer;
      m_readBuffer = NULL;
      if ( m_parentRequester ) {
         m_parentRequester->request(
            MC2SimpleString( m_currentlyCorrectLang ),
            m_currentlyReading.second,
            m_currentlyReading.whereFrom );
      }
   } else {      
      // All ok - copy all data.
      BitBuffer* tmpBuffer = m_readBuffer;
      m_readBuffer = NULL;
      MC2SimpleString tmpStr( m_currentlyCorrectLang );
      DBufRequestListener* listener = m_currentlyReading.second;
      m_currentlyReading.first  = "";
      m_currentlyCorrectLang  = "";
      m_currentlyReading.second = NULL;
      // And then tell the listener about the wonderful new buffer.
      mc2log << "[MFDBR]: Sending " << MC2CITE( tmpStr ) << " to listener"
             << endl;
      listener->requestReceived( tmpStr,
                                 tmpBuffer,
                                 *this);

   }
   startWorking();
}

void
MultiFileDBufRequester::writeDone( int nbrWritten )
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == WRITING_MAP_DATA );
   MC2_ASSERT( ! m_readOnly );

   mc2log << "[MFDBR]: writeDone " << endl;
   setState( IDLE ); 
   if ( (m_nbrWrites++ & 31) == 31 ) {
      // Check disk space
      setMaxSize( m_newSize );
   }

   // Save values needed.
   int bufSize = m_currentlyWriting.second->getBufferSize();

   
   // Get rid of the buffer.
   DBufRequester::release( m_currentlyCorrectLang,
                           m_currentlyWriting.second );
   m_currentlyWriting.second = NULL;
   
   if ( nbrWritten == bufSize ) {
      // Update the index too.
      setState( WRITING_INDEX );
      m_indexHandler->updateMapIndex( m_lastWrittenFileNbr,
                                      m_currentlyWriting.first,
                                      m_startWritePos,
                                      nbrWritten );
   } else {
      mc2log << "[MFDBR]: Write error" << endl;
      startWorking();
   }
}

void
MultiFileDBufRequester::indexWritten(const MC2SimpleString& desc)
{
   //XXX: Note that the index might not have been written depending on
   // very many circumstances. The method name is somewhat misleading.
   
   if ( m_shuttingDown ) {
      return;
   }
   // Some sanity checks
   MC2_ASSERT( m_state == WRITING_INDEX );
   MC2_ASSERT( desc == m_currentlyWriting.first );
   MC2_ASSERT( m_currentlyWriting.second == NULL );
   setState ( IDLE );
   
   uint32 sizeSum = getFileSize( m_lastWrittenFileNbr );

   if ( ! m_readOnly ) {
      if ( sizeSum >= m_maxSizePerFile ) {
         mc2log << "[MFDBR]: Moving to next file" << endl;
         m_lastWrittenFileNbr = (m_lastWrittenFileNbr + 1) % m_nbrFiles;
         clearFileAndIndex( m_lastWrittenFileNbr );
      }
   }
   
   startWorking();
}

MC2SimpleString
MultiFileDBufRequester::updateParamLang( const MC2SimpleString param ) const
{
   // Check if it's needed to fake the language to suit the
   // one in readonly cache.
   if ( m_readOnly && !m_readOnlyLangs.empty() && 
         ( param[0] == 'T' ) ) { // read only and string.
      TileMapParams tmpParams( param );

      // Check if the requested language is one of the 
      // languages present in the readonly cache.
      if ( std::find( m_readOnlyLangs.begin(), m_readOnlyLangs.end(),
               tmpParams.getLanguageType() ) ==
            m_readOnlyLangs.end() ) {
         // No it was not present.
         // Fake that the we requested one of the available languages.
         mc2log << "MFDBR: Language " 
            << (int) tmpParams.getLanguageType()
            << " not found in read only cache. Using "
            << (int) m_readOnlyLangs.front() << " instead." << endl;
         tmpParams.setLanguageType( m_readOnlyLangs.front() );
         return tmpParams.getAsString();
      }
   }
   return param;
}

void
MultiFileDBufRequester::startFinding()
{
   if ( m_inStartFinding ) {
      return;      
   }
   m_inStartFinding++;
   while ( ( m_state == IDLE ) && ( ! m_readQueue.empty() ) ) {
      // Only start the reading process if we are idle.
      BitBuffer* cached = NULL;
      m_currentlyReading = m_readQueue.front();
      m_readQueue.pop_front();
      m_currentlyCorrectLang = m_currentlyReading.first;
      m_currentlyReading.first = updateParamLang( m_currentlyCorrectLang );
     
      // Check if it is in the write queue first.
      cached = requestCached( m_currentlyCorrectLang );
      if ( cached ) {         
         m_currentlyReading.second->requestReceived(
            MC2SimpleString( m_currentlyCorrectLang ),
            cached, *this );
         continue;
      }
      setState ( LOOKING_FOR_MAP );
      // The following function may call startFinding via startWorking
      // from findComplete
      m_indexHandler->find( m_currentlyReading.first,
                            m_lastWrittenFileNbr );
   }
   m_inStartFinding--;
}

void
MultiFileDBufRequester::startWriting()
{
   // Avoid recursion - m_indexHandler->find can call findComplete
   // and then we will get here again.
   if ( m_inStartWriting ) {
      return;
   }
   m_inStartWriting++;  
   while ( ( m_state == IDLE ) && ( ! m_writeQueue->empty() ) ) {
      
      m_currentlyWriting = m_writeQueue->front();
      m_writeQueue->pop_front();
      m_currentlyCorrectLang = m_currentlyWriting.first;
      m_currentlyWriting.first = updateParamLang( m_currentlyCorrectLang );
      setState ( WRITING_LOOKING_FOR_MAP );
      // Look for the map in the two newest files if not read only
      // If readonly, then look in all the files.
      const int maxNbr = m_readOnly ? m_nbrFiles : MIN( 2, m_nbrFiles );
      m_indexHandler->find( m_currentlyWriting.first,
                            m_lastWrittenFileNbr,
                            maxNbr );
      // If the indexhandler has called findComplete then we are idle
      // again here.      
   }
   m_inStartWriting--;
}

void
MultiFileDBufRequester::removeComplete( const MC2SimpleString& desc)
{
   MC2_ASSERT( desc == m_currentlyRemoving );
   MC2_ASSERT( m_state == REMOVING_MAP );
   mc2log << "[MFDBR]: removeComplete" << endl;
   setState( IDLE );
   m_currentlyRemoving = "";
   startWorking();
}

void
MultiFileDBufRequester::startRemoving()
{
   if ( m_inStartRemoving ) {
      return;
   }
   ++m_inStartRemoving;
   while ( ( m_state == IDLE ) && ( ! m_buffersToRemove.empty() ) ) {
      MC2_ASSERT( m_currentlyRemoving == "" );
      m_currentlyCorrectLang = *(m_buffersToRemove.begin());
      m_currentlyRemoving = updateParamLang( m_currentlyCorrectLang );
      m_buffersToRemove.erase( m_buffersToRemove.begin() );
      setState( REMOVING_MAP );
      m_indexHandler->remove( m_currentlyRemoving );
   }
   --m_inStartRemoving;
}

void
MultiFileDBufRequester::startWorking()
{
   if ( !m_shuttingDown ) {
      if ( m_state == IDLE ) {
         // Safe to do tricks with the files
         if ( m_maxSize != m_newSize ) {
            setMaxSize( m_newSize );
         }
      }
      
      if ( m_state == IDLE ) {
         startRemoving();
      }
      
      if ( m_state == IDLE ) {
         startFinding();
      } 
      // Check if we are still idle.
      if ( m_state == IDLE ) {
         // Start writing etc.
         startWriting();      
      }
   }
}

void 
MultiFileDBufRequester::writeRemainders()
{
   typedef MemoryDBufRequester::stringBufPair_t stringBufPair_t;

   m_dumpBuffer->reset();
  
   /* write the number of maps */
   m_dumpBuffer->writeNextBALong( 0 ); // Fill in later.
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(), 
                      m_dumpBuffer->getCurrentOffset(), 
                      NULL ); // Synch
   m_dumpBuffer->reset();
   
   uint32 i = 0;
   while ( !m_writeQueue->empty() ) {
      //m_dumpBuffer->reset();
      
      int freeDisk = m_dumpFile->getAvailableSpace() - 1024*512;
      if ( freeDisk < 0 ) {
         // No more disk! Jump out.
         mc2log << "writeRemainders: Out of disk at map " << i << endl;
         break;
      }
      
      MemoryDBufRequester::stringBufPair_t& stringBuf = 
         m_writeQueue->front();

      const MC2SimpleString& curDesc = stringBuf.first;  
      BitBuffer* curBuf = stringBuf.second;

      if ( ! isBeingRemoved ( curDesc ) ) {
         m_dumpBuffer->writeNextBALong( curDesc.length() );
         m_dumpBuffer->writeNextByteArray( (const byte*)curDesc.c_str(),
                                           curDesc.length() );
         m_dumpBuffer->writeNextBALong( curBuf->getBufferSize() );
         // Check how much room is left in the m_dumpBuffer->
         uint32 room =
            m_dumpBuffer->getBufferSize() - m_dumpBuffer->getCurrentOffset();
         // Make sure that there is room for the next description and lengths
         // too.
         if ( room > ( curBuf->getBufferSize() + 80 ) ) {
            mc2log << "[MFDBR]: Adding to old buffer room = "
                   << room << " bufsize = " << curBuf->getBufferSize() << endl;
            // Write the buffer into the temp buffer and then all at once
            // to the file.
            m_dumpBuffer->writeNextByteArray( curBuf->getBufferAddress(),
                                              curBuf->getBufferSize() );
         } else {
            mc2log << "[MFDBR]: Writing buffer room = "
                   << room << " bufsize = " << curBuf->getBufferSize() << endl;
            // Write the buffers separately.
            m_dumpFile->write( m_dumpBuffer->getBufferAddress(),
                               m_dumpBuffer->getCurrentOffset(),
                               NULL ); // Synch
            m_dumpFile->write( curBuf->getBufferAddress(),
                               curBuf->getBufferSize(),
                               NULL ); // Synch
            m_dumpBuffer->reset();
         }
      } else {
         // Buffer should be removed so don't write it.
      }
      
      m_writeQueue->pop_front();
#ifndef QUICK_SHUTDOWN      
      delete curBuf;
#endif      
      ++i;
   }
   
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(),
                      m_dumpBuffer->getCurrentOffset(),
                      NULL ); // Sync
   m_dumpBuffer->reset();
   
   // Write nbr of maps.
   m_dumpBuffer->writeNextBALong( i );
   m_dumpFile->setPos( 0 );
   m_dumpFile->write( m_dumpBuffer->getBufferAddress(), 
                      m_dumpBuffer->getCurrentOffset(), 
                      NULL ); // Synch
}

void 
MultiFileDBufRequester::readRemainders(FileHandler* dumpFile)
{
   if ( dumpFile == NULL || dumpFile->status() != 0 ) {
      mc2log << "[MFDBR]: Dumpfile not ok - skipping" << endl;
      return;
   }
   BitBuffer tempBuf( 1024 );

   // Read the number of maps.
   if ( dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ) != 4 ) {
      // The dumpfile was empty.
      return;
   }
   uint32 nbrMaps = tempBuf.readNextBALong();

   for ( uint32 i = 0; i < nbrMaps; ++i ) {

      tempBuf.reset();
     
      // Length of descr.
      dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ); // Synch
      uint32 descSize = tempBuf.readNextBALong();

      tempBuf.reset();

      // Descr.
      dumpFile->read( tempBuf.getBufferAddress(),
                        descSize, NULL ); // Synch
      
      char* desc = new char[ descSize + 1];
      tempBuf.readNextByteArray( (byte*) desc, descSize );
      desc[ descSize ] = '\0';

      MC2SimpleString curDesc = desc;
      delete[] desc;
      tempBuf.reset();
      
      // Buffer size. 
      dumpFile->read( tempBuf.getBufferAddress(), 4, NULL ); // Synch
      uint32 bufSize = tempBuf.readNextBALong();
     
      // The buffer.
      BitBuffer* curBuf = new BitBuffer( bufSize );
      dumpFile->read( curBuf->getBufferAddress(), 
                        bufSize, NULL ); // Synch

      m_writeQueue->release( curDesc, curBuf );   
   }

   // Clear the dumpfile once we are finished.
   dumpFile->clearFile();
}
   
void 
MultiFileDBufRequester::readCacheInfoFile( FileHandler* cacheInfoFile )
{
   if ( cacheInfoFile == NULL || cacheInfoFile->status() != 0 ) {
      mc2log << "[MFDBR]: Infofile not ok - skipping" << endl;
      return;
   }
   BitBuffer lengthBuf( 4 );
   // Read number bytes in file.
   if ( cacheInfoFile->read( lengthBuf.getBufferAddress(), 4, NULL ) != 4 ) {
      // Empty.
      mc2log << "MFDBR: Empty infofile." << endl;
      return;
   }
   int fileSize = (int) lengthBuf.readNextBALong();
   BitBuffer tempBuf( fileSize );

   if ( cacheInfoFile->read( tempBuf.getBufferAddress(), fileSize, NULL ) !=
        fileSize ) {
      // Couldn't read all data in infofile.
      mc2log << "MFDBR: Could not read all data in infofile." << endl;
      return;
   } 
   
   uint32 nbrLangs = tempBuf.readNextBALong();

   m_readOnlyLangs.reserve( nbrLangs );
   
   for ( uint32 i = 0; i < nbrLangs; ++i ) {
      LangTypes::language_t lang = 
         LangTypes::language_t( tempBuf.readNextBALong() );
      mc2log << "Found readonly lang " << (int) lang << endl;
      m_readOnlyLangs.push_back( lang );
   }

   // Now read the paramsnotices.
   // Nbr param notices.
   uint32 nbrNotices = tempBuf.readNextBALong();
   {for ( uint32 i = 0; i < nbrNotices; ++i ) {
      ParamsNotice notice;
      notice.load( tempBuf );
      layerAndDetail_t layerAndDetail( notice.m_layerID,
                                       notice.m_detailLevel );
      m_paramsByLayerAndDetail[ layerAndDetail ] = notice;
   }}

   // Default is that old caches do not contain bitmaps, unless
   // specifically noted.
   m_containsBitmaps = false;
   if ( tempBuf.getNbrBytesLeft() >= 1 ) {
      m_containsBitmaps = tempBuf.readNextBAByte();
   }
}

bool
MultiFileDBufRequester::maybeInCache( const MC2SimpleString& desc ) 
{
   bool retVal = true;
   if ( m_readOnly ) {
      retVal = false;
      // We only have a clue of what is inside the readonly cache.
      TileMapParamTypes::param_t paramType =
         TileMapParamTypes::getParamType( desc.c_str() );
      
      if ( paramType == TileMapParamTypes::TILE ) {
         TileMapParams tmpParam( desc );
         
         map<layerAndDetail_t, ParamsNotice>::const_iterator findit = 
            m_paramsByLayerAndDetail.find( 
               make_pair( tmpParam.getLayer(), 
                          tmpParam.getDetailLevel() ) );
         if ( findit != m_paramsByLayerAndDetail.end() ) {
            retVal = (*findit).second.inside( tmpParam );
         } else {
            retVal = false;
         }
         if ( ! retVal && ( (m_readOnlyMisses++ & 1023) == 1023 ) ) {
            // Not found in the readonly cache.
            // Clear the index tables in the cache to save memory.
            m_indexHandler->clearAllIndeces();
         }
      } else  if ( paramType == TileMapParamTypes::BITMAP ) {
         retVal = m_containsBitmaps;
      } else if ( desc == "DYYY" ) {
         retVal = true;
      }

#if 0
      if ( retVal ) {
         mc2log << "MFDBR: maybeInCache - " << desc 
                << " maybe inside readonly cache."
                << endl;
      } else {
         mc2log << "MFDBR: maybeInCache - " << desc 
                << " not inside readonly cache."
                << endl;
      }
#endif      
   }
   
   return retVal;
}

void
MultiFileDBufRequester::cleanUpMemUsage()
{
   m_indexHandler->cleanUpMemUsage();
}

