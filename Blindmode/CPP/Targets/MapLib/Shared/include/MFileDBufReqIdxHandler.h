/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MFILEDBUFREQUESTERIDXHANDLER_H
#define MFILEDBUFREQUESTERIDXHANDLER_H

#include "config.h"
#include "MC2SimpleString.h"
#include "FileHandler.h"
#include <vector>
#include <map>
#include "MemTracker.h"

#include "MultiFileDBufRequester.h"

class BitBuffer;
class SharedBuffer;

/**
 *   Class containing the index information of the file cache.
 */
class IndexMap
{

   /// Less than string functor.
   struct ltstr
   {
     bool operator()(const char* s1, const char* s2) const
     {
       return strcmp(s1, s2) < 0;
     }
   };

   
   public:
      
      /// Constructor.
      IndexMap( MemTracker* memTracker );

      /// Destructor.
      ~IndexMap();
   
      /// Clear.
      void clear();

      /// Get the number of elements.
      uint32 size() const;

      /// Return if empty.
      bool empty() const;
      
      /// Set the index buffer.
      bool setBuffer( BitBuffer* buf );

      /// The map contains the descs as key and the value is offset and size.
      typedef std::map<const char*, std::pair<int, int>, ltstr > map_t;
      /// Iterator to the map.
      typedef map_t::iterator iterator;
      /// Const iterator to the map.
      typedef map_t::const_iterator const_iterator;
      
      /// Find the desc.
      iterator find( const char* desc ); 
      
      /// Erase the desc.
      uint32 erase( const char* desc );
      
      /// Insert (replace) the desc.
      std::pair<int, int>& operator[]( const char* desc );

      /// Write the contents to the buffer.
      void write( BitBuffer& buf ) const;
      
      /// Begin iterator.
      iterator begin();
      
      /// End iterator.
      iterator end();

   private:
      
      /// The index buffer directly from disk.
      BitBuffer* m_indexBuf;

      /// Descs that are not present in m_indexBuf. 
      std::vector<char*> m_singleAllocDescs;
      
      /**
       *    Map containing descs as key and (offset, size) in the
       *    cache as vaule.
       *    The descs can either point directly into the indexbuffer
       *    read from disk (m_indexBuf) or to descs that have been
       *    added afterwards (m_singleAllocDescs).
       */
      map_t m_map;

      /// The mem tracker that keeps track of mem usage.
      MemTracker* m_memTracker;

};


class MFileDBufReqIdxHandler : public FileHandlerListener {
public:
   /**
    *   Creates a new MFileDBufReqIdxHandler with the
    *   supplied indexFiles.
    *   @param memTracker Shared object, that keep track of how much
    *                     memory that may be used for caching index
    *                     files etc.
    */
   MFileDBufReqIdxHandler(MultiFileDBufRequester* listener,
                                      std::vector<FileHandler*>& indexFiles,
                                      int nbrHash,
                                      const SharedBuffer* xorBuffer,
                                      MemTracker* memTracker );

   virtual ~MFileDBufReqIdxHandler();

   /**
    *   Sets new xorBuffer.
    */
   void setXorBuffer( const SharedBuffer* xorBuffer );
   
   /**
    *   Sets the shutdown flag.
    */
   void shutDown();
   
   /**
    *   Tries to find the specified desc in the index and calls the listener
    *   when it is done.
    *   @param desc Desc to find.
    *   @param lastWrittenFileNbr File to start looking at.
    *   @param maxNbrFilesToSearch The maximum number of files to search.
    */
   void find( const MC2SimpleString& desc,
              int lastWrittenFileNbr,
              int maxNbrFilesToSearch = -1 );

   /**
    *   Starts writing a map to the index.
    */
   void updateMapIndex( int fileIdx,
                        const MC2SimpleString& desc,
                        int startOffset,
                        int size );

   /**
    *   Starts removing the map from the index files.
    */
   void remove( const MC2SimpleString& descr );

   /**
    *   Called by the FileHandler when a read is done.
    */
   void readDone(int nbrRead);

   /**
    *   Called by the FileHandler when a write is done.
    */
   void writeDone(int nbrWritten);

   /**
    *   Called by the MultiFileDBufRequester when it clears the index file.
    */
   void clearIndex(int nbr);

   /**
    *    Clear all indeces.
    */
   void clearAllIndeces();
   
   /**
    *   Get the index number (for m_indeces) 
    *   for the given mapdesc and filenumber.
    */
   inline int getIndexNbr( int fileNbr, const MC2SimpleString& desc ) const;
   
   /**
    *  Method that will be called when the MemTracker thinks that too
    *  much memory are used for the cache. Will clean up the
    *  cached index tables.
    */
   void cleanUpMemUsage();

private:
   /// Pair of offset in file and size of map
   typedef std::pair<int, int> offsetAndSize_t;

   /// Returns the offset and size of m_currentDescToFind in the current idx
   offsetAndSize_t findMapInCurrentIndexBuffer();
   
   /**
    *   Moves the the next index or tells the MultiFileDBufRequester that
    *   we are done
    */
   void moveToNextIdxOrStop();

   /**
    *   Calls the requester and tells it that we are done.
    */
   void findComplete( int startOffset, int size );

   /**
    *   Calls the requester and tells it that we are done removing.
    */
   void removeComplete();
   
   /**
    *   Initiates the finding process.
    */
   void startFinding();

   /// Removes the currently looked for map from the index.
   void removeFromIndex();

   void startReadingIndex();
   
   void allocReadBuffer(int size);

   bool updateFindIndex();

   void createIndex();

   /// Writes the current index back to disk.
   void writeIndex();

   /// Called when index is written
   void indexWritten();

   /// Adds the current writing map to the current index.
   void addCurrentMapToCurrentIndex();
   
   /// Removes m_currentDescToFind from m_currentIndexFile. @return If removed
   int removeCurrentMapFromCurrentIndex();
  
   /**
    *    Check if it's necessary to clear the index tables and
    *    do so if it was necessary.
    */
   void checkClearIndex();

   /// Vector of index files to look in.
   std::vector<FileHandler*>& m_indexFiles;

   enum mode_t {
      /// We are looking for a map.
      MODE_FINDING,
      /// We are removing a map - not writing it
      MODE_REMOVING,
      /// We are writing a map.
      MODE_WRITING,
   } m_mode;
   
   enum state_t {
      /// Not doing anything
      IDLE = 0,
      /// Looking for a map - reading index length,
      FINDING_MAP_READING_INDEX_LENGTH = 11,
      /// Looking for a map
      FINDING_MAP_READING_INDEX        = 12,
      /// About to write a map - reading index length
      WRITING_MAP_READING_INDEX_LENGTH = 21,
      /// About to write a map - reading index.
      WRITING_MAP_READING_INDEX        = 22,
      /// Writing index.
      WRITING_INDEX                    = 23,
      /// Reading index length before removing map from it
      REMOVING_MAP_READING_INDEX_LENGTH = 31,
      /// Reading the index before removing map from it
      REMOVING_MAP_READING_INDEX        = 32,
   } m_state;

   void setState( state_t newState, int lineNbr );
   
   /// Index of the current file.
   int m_currentIndexFile;
   /// The number of files left to search.
   int m_nbrFilesLeftToSearch;
   /// Current desc to look for or to write.
   MC2SimpleString m_currentDescToFind;
   /// Type of map to store index data in
//   typedef map<MC2SimpleString, pair<int,int> > indexMap_t;
   typedef IndexMap indexMap_t;

   /// Vector of indeces
   std::vector<indexMap_t*> m_indeces;


   /// The listener to all requests
   MultiFileDBufRequester* m_listener;
   
   /// BitBuffer used when reading
   BitBuffer* m_readBuffer;
   
   /// BitBuffer used when writing
   BitBuffer* m_writeBuffer;

   /// Current write position and size of map.
   offsetAndSize_t m_currentWritePosAndSize;

   /// True if no more request may be served
   bool m_shuttingDown;
 
   /**
    * The number of hashes per file, 
    * i.e. how many parts each index file is divided into.
    */
   int m_nbrHash;

   /**
    * The number of datafiles. 
    * XXX: Note that there are m_nbrHash indexfiles for each datafile!
    */
   int m_nbrFiles;
   
   /**
    *   Buffer to use when "encrypting" the index.
    */
   const SharedBuffer* m_xorBuffer;

   /**
    *    Pointer to the shared memory tracker.
    *    Keeps track of how much memory that is used for
    *    cached index tables etc.
    */
   MemTracker* m_memTracker;

};

/// --- Inlines ---

inline int 
MFileDBufReqIdxHandler::getIndexNbr( 
                     int fileNbr, const MC2SimpleString& desc ) const
{
   uint32 hash = 0;
   int len = desc.length();
   for ( int i = 0; i < len; ++i ) {
      hash = (hash >> 28) | (hash << 4);
      hash ^= desc[i];
   }
   hash %= m_nbrHash;
   return fileNbr * m_nbrHash + hash;
}

#endif

