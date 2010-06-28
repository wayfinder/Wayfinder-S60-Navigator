/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MULTIFILEDBUFREQUESTER_H
#define MULTIFILEDBUFREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"
#include "FileHandler.h"
#include "MemTracker.h"

#include "LangTypes.h"

#include <list>
#include <vector>
#include <set>

class MemoryDBufRequester;
class MFileDBufReqIdxHandler;
class BitBuffer;
class SharedBuffer;
struct ParamsNotice;

// -- ReadOnly requester is further down.

class MultiFileDBufRequester : public DBufRequester,
                               public FileHandlerListener,
                               public MemEater {
public:
   /**
    *   Creates a new MultiFileDBufRequester that saves files
    *   in the specified path and uses the specified number of
    *   files.
    *   @param parent   The usual parent parameter.
    *   @param basepath The directory to put the cache into.
    *   @param memTracker Shared object, that keep track of how much
    *                     memory that may be used for caching index
    *                     files etc.
    *   @param maxSize  Maximum size of the cache before deleting old files.
    *   @param nbrFiles The number of wanted data files.
    *   @param nbrHash  The number of index files per data file.
    *   @param readOnly True if the cache should not write anything.
    */
   MultiFileDBufRequester( DBufRequester* parent,
                           const char* basepath,
                           MemTracker* memTracker,
                           uint32 maxSize,
                           int nbrFiles = -1,
                           int nbrHash  = -1,
                           int readOnly = false
                           );

   /// Deletes the requester
   virtual ~MultiFileDBufRequester();

   /**
    *    Sets a buffer to use when xor:ing.
    *    Call this function before starting to use the
    *    MultiFileDBufRequester, because otherwise it may
    *    not recognize the files.
    */
   void setXorBuffer( const SharedBuffer* xorBuffer );
   
   /**
    *    Returns zero if ok. Currently returns the number of
    *    needed files that were not possible to open.
    */
   int status() const;

   
   /// Does the initialization.
   bool init();

   // -- DBufRequester
   
   void request( const MC2SimpleString& desc,
                 DBufRequestListener* client,
                 request_t whereFrom);
   
   BitBuffer* requestCached( const MC2SimpleString& desc);

   void release( const MC2SimpleString& desc, BitBuffer* buffer );

   void cancelAll();
   
   // -- FileHandlerListener
   void readDone( int nbrRead );
   void writeDone( int nbrWritten );
   
   /// Sets maximum storage size.
   void setMaxSize(uint32 size);
   /// Clears cache
   void clearCache();
   
   /// Called by the MFileDBufReqIdxHandler when done searching
   void findComplete( const MC2SimpleString& desc,
                      int fileNo,
                      int startOffset, int size );

   /// Called by the MFileDBufReqIdxHandler when index is written.
   void indexWritten( const MC2SimpleString& desc );

   /// Called by the MFileDBufReqIdxHandler when it has removed
   void removeComplete( const MC2SimpleString& desc );

   /**
    *  Method that will be called when the MemTracker thinks that too
    *  much memory are used for the cache. Will clean up the
    *  cached index tables.
    */
   void cleanUpMemUsage();

protected:
   
   /**
    *    Creates a filehandler siutable for the current platform.
    *    @param   fileName          The file name.
    *    @param   createIfNotExists If to create the file.
    *    @param   initNow           If the file will be opened immediately
    *                               or only when needed. May not be
    *                               supported for all subclasses.
    */
   virtual FileHandler* createFileHandler(const char* fileName,
                                          bool createIfNotExists,
                                          bool initNow = true ) = 0;
   
   /**
    *   Returns the path separator for the current platform.
    */
   virtual const char* getPathSeparator() const = 0;

   /**
    *   Returns true if the buffer is ok to write even if it already
    *   exists.
    */
   bool okToWriteEvenIfItExists( const MC2SimpleString& descr );
   
private:
   /// Enum of states
   enum state_t {
      /// The MultiFileDBufRequester does not have anything to do.
      IDLE,
      /// We are looking for a map.
      LOOKING_FOR_MAP,
      /// We are reading a map from a file
      READING_MAP,
      /// We are checking where the map is
      WRITING_LOOKING_FOR_MAP,
      /// We are adding the map to a data file
      WRITING_MAP_DATA,
      /// We are adding the map to an index file.
      WRITING_INDEX,
      /// We are removing a map from the index.
      REMOVING_MAP,
   } m_state;

   MC2SimpleString updateParamLang( const MC2SimpleString param ) const;

   void startFinding();
   void startWorking();
   void startWriting();
   void startRemoving();
   void setState( state_t newState );

   /// Decreases index. Wraps if necessary
   int decIndex(int oldIndex) const;
   
   /// Increases index. Wraps if necessary
   int incIndex(int oldIndex) const;

   /// Clears the oldest not empty file. @return The amount of deleted mem
   uint32 clearOldestNotEmptyFile();

   /// Returns the amount of avaliable space on the drive of the first index.
   uint32 getAvailableSpace() const;

   /// Returns the total cache size
   int32 getTotalCacheSize() const;

   /// Calculates max size of the cache
   uint32 calcMaxAllowedCacheSize( uint32 wantedSize ) const;
   
   /// Write remainding maps to a dump file.
   void writeRemainders();

   /// Read remainding maps from dump file.
   void readRemainders(FileHandler* dumpFile);

   /// Clears a file and its index.
   void clearFileAndIndex( int fileNbr );

   /// Internal remove call from DBufRequester
   void internalRemove( const MC2SimpleString& desc );

   /// Returns true if the desc is currently being removed.
   int isBeingRemoved( const MC2SimpleString& desc );
   
   inline uint32 getFileSize( int index );

   /// Read the read only cache info file.
   void readCacheInfoFile( FileHandler* cacheInfoFile );
  
   /// True if the desc may be in the cache. False if definitely not. 
   bool maybeInCache( const MC2SimpleString& desc );
 
   /// True if the cache is read only
   const int m_readOnly;
   /// Maximum size of the files
   uint32 m_maxSize;
   /// Max size for one file.
   uint32 m_maxSizePerFile;
   /// Number of files to use.
   int m_nbrFiles;
   /// Nbr hashes for each index file.
   int m_nbrHash;
   /// Base path
   MC2SimpleString m_basePath;
   /// Map of fileHandlers for the map files.
   std::vector<FileHandler*> m_dataFiles;
   /// Map of fileHandlers for the index files.
   std::vector<FileHandler*> m_indexFiles;
   /// Set of maps to be removed from the index.
   std::set<MC2SimpleString> m_buffersToRemove;
   
   /// Maps that are waiting to be written do disk.
   MemoryDBufRequester* m_writeQueue;

   /// Currently removing descr
   MC2SimpleString m_currentlyRemoving;
   
   /// Buffer about to be written
   std::pair<MC2SimpleString, BitBuffer*> m_currentlyWriting;
   int m_startWritePos;

   struct readJob_t {

      /// Default constructor seems to be needed by the stlport
      readJob_t() {
         second = NULL;
         whereFrom = DBufRequester::cacheOrInternet;
      }
      
      readJob_t( const MC2SimpleString& descr,
                 DBufRequestListener* listener,
                 request_t pWhereFrom ) {
         first      = descr;
         second     = listener;
         whereFrom  = pWhereFrom;
      }
      
      int operator==( const readJob_t& other ) const {
         return other.first  == first &&
                other.second == second &&
            other.whereFrom == other.whereFrom;
      }
      /// The requested databuffer. Called first since this was a pair
      MC2SimpleString first;
      /// The request listener. Called second since this was a pair.
      DBufRequestListener* second;
      /// True if it is only allowed to request from cache.
      request_t whereFrom;
   };
   
   //typedef pair<MC2SimpleString, DBufRequestListener*> readJob_t;

   /// Queue of jobs to read.
   std::list<readJob_t> m_readQueue;
   
   /// Pair of map description and listener that we currently are reading.
   readJob_t m_currentlyReading;
   MC2SimpleString m_currentlyCorrectLang;

   /// Last written fil
   int m_lastWrittenFileNbr;
   
   /// The index handler
   MFileDBufReqIdxHandler* m_indexHandler;

   /// The current read buffer.
   BitBuffer* m_readBuffer;
   
   /// BitBuffer used when dumping (10k constantly allocated)
   BitBuffer* m_dumpBuffer;
   
   /// Counts the number of writes to disk.
   uint32 m_nbrWrites;
   
   /// Counts number of readonly cache misses.
   uint32 m_readOnlyMisses; 

   /// The dump file.
   FileHandler* m_dumpFile;

   /// Size of cache to set next time the handles is idle
   uint32 m_newSize;
   
   /// True if we are shutting down
   bool m_shuttingDown;

   /// Count if we are in startWriting
   int m_inStartWriting;
   /// Count if we are in startFinding.
   int m_inStartFinding;
   /// Count if we are in startRemoving.
   int m_inStartRemoving;   
   /// The number of needed files with errors.
   int m_nbrErrorFiles;
   /// Pointer to the buffer we will use for the xor:ing.
   const SharedBuffer* m_xorBuffer;
  
   /// The languages present available in the readonly cache.
   std::vector<LangTypes::language_t> m_readOnlyLangs;
   
   /// first is layerid and second is detaillevel.
   typedef std::pair<int,int> layerAndDetail_t;

   /**
    * Table used for the readonly cache to quickly check if inside
    * the available tiles.
    */
   std::map<layerAndDetail_t, ParamsNotice> m_paramsByLayerAndDetail;

   /// The memory tracker.
   MemTracker* m_memTracker;

   /// If the cache contains bitmaps.
   bool m_containsBitmaps;
};

/// Same same.
typedef MultiFileDBufRequester MFDBufReq;

// --- Inlines ---

inline uint32
MultiFileDBufRequester::getFileSize( int index ) 
{
   uint32 size = m_dataFiles[ index ]->getFileSize();
   for ( int i = 0; i < m_nbrHash; ++i ) {
      size += m_indexFiles[ index * m_nbrHash + i ]->getFileSize();
   }
   return size;
}   

#endif
