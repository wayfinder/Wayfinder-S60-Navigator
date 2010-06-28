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
#include "MFileDBufReqIdxHandler.h"
#include "MultiFileDBufRequester.h"
#include "BitBuffer.h"
#include "TileMapUtil.h"

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

#define SET_STATE(s) setState(s, __LINE__)

using namespace std;


IndexMap::IndexMap( MemTracker* memTracker )
{
   m_memTracker = memTracker;
   m_indexBuf = NULL;
}

void 
IndexMap::clear()
{
   if ( m_indexBuf != NULL ) {
      m_memTracker->releaseToPool( m_indexBuf->getBufferSize() );
   }
   
   m_map.clear();
   delete m_indexBuf;
   m_indexBuf = NULL;

   for ( uint32 i = 0; i < m_singleAllocDescs.size(); ++i ) {
      delete[] m_singleAllocDescs[ i ];
   }
   
   m_singleAllocDescs.clear();
}

IndexMap::~IndexMap()
{
   clear();
}

uint32 
IndexMap::size() const
{
   return m_map.size();
}

bool 
IndexMap::empty() const
{
   return m_map.empty();
}
      
bool
IndexMap::setBuffer( BitBuffer* buf )
{
   clear();
   m_indexBuf = buf;
   m_indexBuf->reset();

   uint32 bufSize = m_indexBuf->readNextBALong() + 4;
   if ( bufSize != m_indexBuf->getBufferSize() ) {
      delete m_indexBuf;
      m_indexBuf = NULL;
      return false;
   }   

   int nbrEntries = m_indexBuf->readNextBALong();
   for ( int i = 0; i < nbrEntries; ++i ) {
      const char* descr = m_indexBuf->readNextString();
      int startOffset = m_indexBuf->readNextBALong();
      int size = m_indexBuf->readNextBALong();
      // Insert the read index entry last in the map since it is
      // already sorted.
      m_map.insert(
         m_map.end(),
         make_pair(descr, make_pair(startOffset,
                                    size ) ) );
   }   
   m_memTracker->allocateFromPool( m_indexBuf->getBufferSize() ); 

   return true;   
}

IndexMap::iterator 
IndexMap::find( const char* desc ) 
{
   return m_map.find( desc );
}

uint32
IndexMap::erase( const char* desc )
{
   return m_map.erase( desc );
}

pair<int, int>& 
IndexMap::operator[]( const char* desc )
{
   char* newDesc = TileMapUtil::newStrDup( desc );
   m_singleAllocDescs.push_back( newDesc );
   return m_map[ newDesc ];
}
   
void 
IndexMap::write( BitBuffer& buf ) const
{
   buf.writeNextBALong( buf.getBufferSize() - 4 );
   buf.writeNextBALong( m_map.size() );
   for ( const_iterator it = m_map.begin();
         it != m_map.end();
         ++it ) {
      buf.writeNextString( it->first );
      buf.writeNextBALong( it->second.first );
      buf.writeNextBALong( it->second.second );
   }

}

IndexMap::iterator 
IndexMap::begin()
{
   return m_map.begin();
}

IndexMap::iterator 
IndexMap::end()
{
   return m_map.end();
}

MFileDBufReqIdxHandler::
MFileDBufReqIdxHandler(MultiFileDBufRequester* listener,
                                   vector <FileHandler*>& indexFiles,
                                   int nbrHash,
                                   const SharedBuffer* xorBuffer,
                                   MemTracker* memTracker )
      : m_indexFiles(indexFiles),
        m_currentIndexFile(0),
        m_nbrHash( nbrHash ),
        m_memTracker( memTracker )
   
{
   setXorBuffer( xorBuffer );
   m_nbrFiles = indexFiles.size() / nbrHash;
   m_listener = listener;
   m_readBuffer = NULL;
   m_writeBuffer = NULL;
   m_indeces.resize( m_indexFiles.size() );
   // VC++ does not handle these being non-pointers.
   for ( uint32 i = 0; i < m_indeces.size(); ++i ) {
      m_indeces[i] = new indexMap_t( m_memTracker );
   }
   m_state = IDLE;
   m_shuttingDown = false;
}

MFileDBufReqIdxHandler::
~MFileDBufReqIdxHandler()
{
   m_shuttingDown = true;
   delete m_writeBuffer;
   delete m_readBuffer;
   for ( uint32 i = 0; i < m_indeces.size(); ++i ) {
      delete m_indeces[i];
   }
}

void
MFileDBufReqIdxHandler::setXorBuffer( const SharedBuffer* xorBuf)
{
   m_xorBuffer = xorBuf;
}

void
MFileDBufReqIdxHandler::clearIndex(int nbr)
{
   for ( int i = 0; i < m_nbrHash; ++i ) {
      int idx = nbr * m_nbrHash + i;
      m_indeces[ idx ]->clear();
   }
}

void
MFileDBufReqIdxHandler::clearAllIndeces()
{
   int length = m_nbrHash * m_nbrFiles;
   for ( int i = 0; i < length; ++i ) {
      m_indeces[ i ]->clear();
   }
}

void
MFileDBufReqIdxHandler::shutDown()
{
   m_shuttingDown = true;
}

void
MFileDBufReqIdxHandler::setState(state_t newState,
                                             int line)
{
   mc2log << "[MFDBRIH]: " << __FILE__ << ":" << line
          << " State change "
          << m_state << " -> " << newState << endl;
   m_state = newState;
}

void
MFileDBufReqIdxHandler::find( const MC2SimpleString& desc,
                                          int lastWrittenFileNbr,
                                          int maxNbrFiles )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_FINDING;
   m_nbrFilesLeftToSearch = m_nbrFiles;
   if ( maxNbrFiles > 0 ) {
      m_nbrFilesLeftToSearch = maxNbrFiles;
   }
   m_currentIndexFile     = lastWrittenFileNbr;
   m_currentDescToFind    = desc;
   startFinding();
}

void
MFileDBufReqIdxHandler::
updateMapIndex( int fileIdx,
                const MC2SimpleString& desc,
                int startOffset,
                int size )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_WRITING;
   m_currentIndexFile  = fileIdx;
   m_currentDescToFind = desc;
   m_currentWritePosAndSize = make_pair(startOffset, size);
   
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );

   // Start working
   // Get the size of the index.
   if ( ! m_indeces[ idx ]->empty() ) {
      addCurrentMapToCurrentIndex();
      writeIndex();
      return;
   }
   // If we get here, the index table is already empty.
//   m_indeces[ idx ]->clear();
   SET_STATE( WRITING_MAP_READING_INDEX_LENGTH );
   allocReadBuffer(4);

#if 1
   // Trick
   int len = m_indexFiles[ idx ]->getFileSize();
   mc2log << "[MFDBRIH]: Length of index " << idx << " = "
          << len << endl;
   if ( len > 4 ) {
      m_readBuffer->writeNextBALong(len - 4);
      m_readBuffer->reset();
      readDone( m_readBuffer->getBufferSize() );   
   } else {
      readDone( -1 );
   }
   return;
#endif
   
   m_indexFiles[ idx ]->setPos(0);
   m_indexFiles[ idx ]->read(m_readBuffer->getBufferAddress(),
                             m_readBuffer->getBufferSize(),
                             this);
}

void
MFileDBufReqIdxHandler::remove( const MC2SimpleString& desc )
{
   MC2_ASSERT( m_state == IDLE );
   m_mode = MODE_REMOVING;
   m_currentIndexFile = 0;
   m_currentDescToFind = desc;
   m_nbrFilesLeftToSearch = m_nbrFiles;
   startFinding();
}

void
MFileDBufReqIdxHandler::removeComplete()
{
   SET_STATE( IDLE );
   m_listener->removeComplete( m_currentDescToFind );
}

void
MFileDBufReqIdxHandler::allocReadBuffer(int size)
{
   delete m_readBuffer;
   m_readBuffer = new BitBuffer( size );
}

void
MFileDBufReqIdxHandler::startFinding()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   if ( ! m_indeces[ idx ]->empty() ) {
      if ( m_mode == MODE_FINDING ) {
         offsetAndSize_t offsetAndSize = findMapInCurrentIndexBuffer();
         bool found = offsetAndSize.first != -1;
         if ( found ) {
            // Found the desc.
            findComplete(offsetAndSize.first, offsetAndSize.second);
         } else {
            // Move to next index.
            moveToNextIdxOrStop();
         }
      } else {
         MC2_ASSERT( m_mode == MODE_REMOVING );
         // Will do the moving to next file etc.
         removeFromIndex();
      }
      return;
   }
   // If we get here, the index table is already empty.
//   m_indeces[ idx ]->clear();
   // Get the size of the index.
   if ( m_mode == MODE_FINDING ) {
      SET_STATE( FINDING_MAP_READING_INDEX_LENGTH );
   } else {
      SET_STATE( REMOVING_MAP_READING_INDEX_LENGTH );
   }
   allocReadBuffer(4);

#if 1
   // Trick
   int len = m_indexFiles[ idx ]->getFileSize();
   mc2log << "[MFDBRIH]: Length of index " << idx << " = "
          << len << endl;
   if ( len > 4 ) {
      m_readBuffer->writeNextBALong(len - 4);
      m_readBuffer->reset();
      readDone( m_readBuffer->getBufferSize() );   
   } else {
      readDone( -1 );
   }
   return;
#endif
   
   // Rewind the file
   m_indexFiles[ idx ]->setPos(0);
   // Request the read.
   m_indexFiles[ idx ]->read(m_readBuffer->getBufferAddress(),
                             m_readBuffer->getBufferSize(),
                             this);
}

void
MFileDBufReqIdxHandler::startReadingIndex()
{
   MC2_ASSERT( m_state == FINDING_MAP_READING_INDEX_LENGTH ||
               m_state == WRITING_MAP_READING_INDEX_LENGTH ||
               m_state == REMOVING_MAP_READING_INDEX_LENGTH );
   
   m_readBuffer->reset();
   int indexLength = m_readBuffer->readNextBALong();
   // Get the index
   if ( m_state == FINDING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE( FINDING_MAP_READING_INDEX );
   } else if ( m_state == WRITING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE ( WRITING_MAP_READING_INDEX );
   } else if ( m_state == REMOVING_MAP_READING_INDEX_LENGTH ) {
      SET_STATE( REMOVING_MAP_READING_INDEX );
   }
   
   allocReadBuffer( indexLength + 4 );
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   
   // Rewind the file to the correct position.
   m_indexFiles[ idx ]->setPos(0);
   m_indexFiles[ idx ]->read( m_readBuffer->getBufferAddress(),
                              m_readBuffer->getBufferSize(),
                              this );
}

void
MFileDBufReqIdxHandler::createIndex()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   if ( ! m_indeces[ idx ]->empty() ) {
      return;
   }

   // If we get here, the index table is already empty.
//   m_indeces[ idx ]->clear();
   
   BitBuffer& indexBuf = *m_readBuffer;
   indexBuf.reset();
   if ( m_xorBuffer ) {
      indexBuf.xorBuffer( *m_xorBuffer );
   }

   if ( ! m_indeces[ idx ]->setBuffer( new BitBuffer( indexBuf ) ) ) {
      if ( m_xorBuffer ) {
         mc2log << "[MFDBRIH]: Wrong length of buffer after xor " << endl;
      } else {
         mc2log << "[MFDBRIH]: Wrong length of buffer without xor " << endl;
      }
      m_indexFiles[ idx ]->clearFile();
      return;
   }
}

MFileDBufReqIdxHandler::offsetAndSize_t
MFileDBufReqIdxHandler::findMapInCurrentIndexBuffer()
{
#if 0
   BitBuffer& indexBuf = *m_readBuffer;   
   indexBuf.reset();

   int nbrEntries = indexBuf.readNextBALong();
   for ( int i = 0; i < nbrEntries; ++i ) {
      const char* descr = indexBuf.readNextString();
      if ( m_currentDescToFind < descr ) {
         return offsetAndSize_t(-1, -1);
      }
      int startOffset = indexBuf.readNextBALong();
      int size = indexBuf.readNextBALong();
      // Insert the read index entry last in the map since it is
      // already sorted.
      if ( m_currentDescToFind == descr ) {
         return offsetAndSize_t(startOffset, size);
      }
   }
   return offsetAndSize_t(-1, -1);
#else 
   createIndex();
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   indexMap_t::iterator findit = 
      m_indeces[ idx ]->find( m_currentDescToFind.c_str() );
   
   offsetAndSize_t offsetAndSize(-1, -1);
   if ( findit != m_indeces[ idx ]->end() ) {
      // Found.
      offsetAndSize = (*findit).second;
   }
   checkClearIndex();
   return offsetAndSize;
#endif
}

void
MFileDBufReqIdxHandler::writeIndex()
{
   MC2_ASSERT( m_writeBuffer == NULL );
   SET_STATE( WRITING_INDEX );
   int stringSum = 0;
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   int indexSize = m_indeces[ idx ]->size();
   {
      for ( indexMap_t::const_iterator it =
               m_indeces[ idx ]->begin();
            it != m_indeces[ idx ]->end();
            ++it ) {
         stringSum += strlen( it->first ) + 1;
      }
   }
   m_writeBuffer = new BitBuffer( 4 + 4 + stringSum + ( indexSize * 8 ) );
   m_indeces[ idx ]->write( *m_writeBuffer );
   // Check if to clear the index.
   checkClearIndex();

   if ( m_xorBuffer ) {
      m_writeBuffer->xorBuffer( *m_xorBuffer );
   }
   // Activate the writing.
   m_indexFiles[ idx ]->setPos(0);
   // XXX: This is only necessary if we have removed something from
   // the index!!
   m_indexFiles[ idx ]->setSize( m_writeBuffer->getCurrentOffset() );
   m_indexFiles[ idx ]->write( m_writeBuffer->getBufferAddress(),
                               m_writeBuffer->getCurrentOffset(),
                               this );
}

void
MFileDBufReqIdxHandler::findComplete(int startOffet, int size)
{
   SET_STATE ( IDLE );
   m_listener->findComplete(m_currentDescToFind,
                            m_currentIndexFile,
                            startOffet, size);
}

bool
MFileDBufReqIdxHandler::updateFindIndex()
{
   m_nbrFilesLeftToSearch--;
   if ( m_nbrFilesLeftToSearch == 0 ) {
      return false;
   } else {
      m_currentIndexFile--;
      if ( m_currentIndexFile < 0 ) {
         m_currentIndexFile = m_nbrFiles - 1;
      }
      return true;
   }
}

void
MFileDBufReqIdxHandler::moveToNextIdxOrStop()
{
   if ( updateFindIndex() ) {
      startFinding();
   } else {
      if ( m_mode == MODE_FINDING ) {
         findComplete(-1, -1);
      } else {
         MC2_ASSERT( m_mode == MODE_REMOVING );
         removeComplete();
      }
   }
}

void
MFileDBufReqIdxHandler::addCurrentMapToCurrentIndex()
{
   mc2log << "[MFDBRIH]: Adding map "
          << m_currentDescToFind << " to index "
          << m_currentIndexFile << endl;
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
  
   (*m_indeces[idx])[ m_currentDescToFind.c_str() ] =
      m_currentWritePosAndSize;
}

int
MFileDBufReqIdxHandler::removeCurrentMapFromCurrentIndex()
{
   int idx = getIndexNbr( m_currentIndexFile, m_currentDescToFind );
   
   uint32 nbrErased = 
      m_indeces[ idx ]->erase( m_currentDescToFind.c_str() );
   if ( nbrErased > 0 ) {
      mc2log << "[MFDBRIH]: Removed map "
             << m_currentDescToFind << " from index "
             << m_currentIndexFile
             << endl;
   } else {
      mc2log << "[MFDBRIH]: Desc " << m_currentDescToFind
             << " not found in index " << m_currentIndexFile
             << endl;
   }
   return nbrErased > 0;
}

void
MFileDBufReqIdxHandler::indexWritten()
{
   if ( m_mode == MODE_WRITING ) {
      SET_STATE( IDLE );
      m_listener->indexWritten(m_currentDescToFind);
   } else {
      moveToNextIdxOrStop();
   }
}

void
MFileDBufReqIdxHandler::removeFromIndex()
{
   if ( removeCurrentMapFromCurrentIndex() ) {
      writeIndex();
   } else {
      moveToNextIdxOrStop();
   }

}

void
MFileDBufReqIdxHandler::readDone(int nbrRead)
{
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   if ( m_shuttingDown ) {
      return;
   }
   switch ( m_state ) {
      case FINDING_MAP_READING_INDEX_LENGTH:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            /// Read error
            mc2log << "[MFDBRIH]: Read returned " << nbrRead
                   << " for file number " << m_currentIndexFile << endl;
            moveToNextIdxOrStop();
         } else {
            startReadingIndex();
         }
         break;

      case FINDING_MAP_READING_INDEX:     
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Read error
            mc2log << "[MFDBRIH]: Read returned " << nbrRead
                   << " for file number " << m_currentIndexFile << endl;
            moveToNextIdxOrStop();
         } else {
            offsetAndSize_t offsetAndSize = findMapInCurrentIndexBuffer();
            bool found = offsetAndSize.first != -1;
            if ( found ) {
               mc2log << "[MFDBRIH]: Found " << m_currentDescToFind
                      << " in index " << endl;
               findComplete(offsetAndSize.first, offsetAndSize.second);
            } else {
               // Try the next one or give up.
               moveToNextIdxOrStop();
            }
         }
         break;

      case WRITING_MAP_READING_INDEX_LENGTH:
      case REMOVING_MAP_READING_INDEX_LENGTH:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Probably empty.
            mc2log << warn << "[MFDBRIH]: Could not read index "
                   << endl;
            if ( m_mode == MODE_REMOVING ) {
               removeFromIndex();
            } else {
               addCurrentMapToCurrentIndex();
               writeIndex();
            }
         } else {
            // Index found and ok
            startReadingIndex();
         }
         break;

      case WRITING_MAP_READING_INDEX:
      case REMOVING_MAP_READING_INDEX:
         if ( nbrRead != (int)m_readBuffer->getBufferSize() ) {
            // Something went wrong
            indexWritten();
         } else {
            // Add the map to the index.
            createIndex();
            if ( m_mode == MODE_REMOVING ) {
               removeFromIndex();
            } else {
               addCurrentMapToCurrentIndex();
               writeIndex();
            }
         }
         break;

      case IDLE:
      case WRITING_INDEX:
         mc2log << error << "[Should not be idle now]" << endl;
         break;
   }
}

void
MFileDBufReqIdxHandler::writeDone(int nbrWritten)
{
   if ( m_shuttingDown ) {
      return;
   }
   MC2_ASSERT( m_state == WRITING_INDEX );
   delete m_writeBuffer;
   m_writeBuffer = NULL;
   mc2log << "[MFDBRIH]: nbrWritten = " << nbrWritten << endl;
   if ( m_mode == MODE_WRITING ) {
      indexWritten();
   } else {
      // Means that we are removing from all indeces
      moveToNextIdxOrStop();
   }
}

void
MFileDBufReqIdxHandler::checkClearIndex()
{
   m_memTracker->checkCleanMemUsage();
}

void
MFileDBufReqIdxHandler::cleanUpMemUsage()
{
   if ( m_state == IDLE ) {
      // Does not seem like we're doing anything right now.
      // Clean up.
      mc2log << "[MFDBReq:] cleanUpMemUsage IDLE" << endl;
      clearAllIndeces();
   }
}

