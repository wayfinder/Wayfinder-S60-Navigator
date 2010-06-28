/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef FILEDATABUFFERREQUESTER_H
#define FILEDATABUFFERREQUESTER_H

#include "config.h"
#include "DBufRequester.h"
#include "MC2SimpleString.h"
#include "BitBuffer.h"

#include <e32std.h>
#include <e32base.h>
#include <d32dbms.h>
#include <f32file.h>
#include <s32file.h>

#include <map>
#include <list>
#include <set>

class FileDBufRequester;

/**
 *   Class to write and read left-over maps to a RFile.
 */
class CMapDumper : public CActive
{
   private:
      /* state of the AO */
      enum
      {
         EIdle,
         EReading
      };
      /// Read buffer to keep the map in
      TPtr8 m_mapDataReadBuffer;
      
      /* state of the AO */
      int m_state;

      /* the file handle */
      RFileBuf m_file;

      /* current file position for restoring */
      TStreamPos m_curPos;

      /* number of maps */
      uint32 m_numberOfMaps;

      /* is true, if all maps have been read; else false */
      bool m_allMapsRead;

      /* variables for reading */
      BitBuffer* m_bitBuffer;
      MC2SimpleString m_mapDescr;

      /* the parent FileDBufRequester */
      FileDBufRequester& m_requester;

      /* constructor */
      CMapDumper(FileDBufRequester& req);

      /* second-phase constructor */
      int Construct();

      /* from CActive */
      void RunL();
      void DoCancel();

   public:
    
      
      /* allocator */
      static CMapDumper* New(FileDBufRequester& req);

      /* destructor */
      ~CMapDumper();

      /* 
       * Asynchronous read, it reads the map and then informs the 
       * parent requester. It keeps repeating the request for the total maps.
       */
      void readAsync();

      /* 
       * returns true, if all maps have been read; else false.
       */
      bool allMapsRead() const
      {
         return(m_allMapsRead);
      }
};

/**
 *   Class requesting databuffers from file and a parent
 *   requester.
 */
class FileDBufRequester : public DBufRequester, public CActive {
public:
   
   /// Sets maximum storage size.
   void setMaxSize(uint32 size);
   /// Clears cache. Not implemented.
   void clearCache();
   
   
   /* friend classes */
   friend class CMapDumper;

   /**
    *   Creates a new FileDBufRequester.
    *   @param parent Who to ask if the buffer isn't in file.
    *   @param path   The path to the directory 
    *   @param maxMem Approx max mem in bytes. TODO Implement.
    */
   FileDBufRequester(DBufRequester* parent,
                     const char* path,
                     uint32 maxMem);

   /**
    *   Deletes all stuff.
    */
   virtual ~FileDBufRequester();
   
   /**
    *   Makes it ok for the Requester to delete the BitBuffer
    *   or to put it in the cache. Requesters which use other
    *   requesters should hand the objects back to their parents
    *   and requesters which are top-requesters should delete them.
    *   It is important that the descr is the correct one.
    */ 
   void release(const MC2SimpleString& descr,
                BitBuffer* obj);

   /**
    *   If the DBufRequester already has the map in cache it
    *   should return it here or NULL. The BitBuffer should be
    *   returned in release as usual.
    *   @param descr Key for databuffer.
    *   @return Cached BitBuffer or NULL.
    */
   BitBuffer* requestCached(const MC2SimpleString& descr);

   /**
    *   Requests a BitBuffer from the DBufRequester. When the
    *   buffer is received caller->requestReceived(desc, buffer)
    *   will be called.
    *   @nb Wherefrom does not work here.
    */
   void request(const MC2SimpleString& descr,
                DBufRequestListener* caller,
                request_t whereFrom );

   /**
    *   Cancels all pending requests to the server (or database).
    */
   void cancelAll();


   /* compacts the DB Cache */
   void compressCache() {
      m_database.Compact();
      return;
   }

   /* writes leftover maps to DB */
   void writeRemaindersToDB();

   // callback for the timestamp timer 
   static TInt UpdateCallback(TAny* thisPtr);

   /* state enums */
   enum
   {
      EIdle,
      EReading,
      EWriting,
      EUpdating,
      ECleaning
   };

protected:

   /* from CActive */
   void RunL();
   void DoCancel();
   
private:

   /// Writes the buffer to file.
   bool writeToFile(const MC2SimpleString& descr,
                    BitBuffer* buffer);
   
   /// The path to the dir of the files
   MC2SimpleString m_path;

   /// Stores the maximum size of the cache after which it needs cleanup
   uint32 m_maxCacheSize;

   /// Stores the threshold value for the cache after cleanup stops
   uint32 m_cleanupThresholdSize;

   /// Stores the current size of the cache
   uint32 m_cacheSize;

private:

   // initializes the databse
   TInt StartDb(TDesC& aFullFilePath);

   // creates the table format for the DB
   void CreateMapTableL();

   // checks if the specified map is present in the cache
   bool IsRecordPresent(const MC2SimpleString& descr);

   // counts the number of bytes in cached maps during DB Initialization
   void createInitialByteCount();

   // checks if cache size is over the specified limit. if it is,
   // returns true, else false.
   bool cacheNeedsCleanup() const;

   // checks if cache size is under the specified cleanup threshold.
   // if it is, returns true, else false.
   bool cacheUnderThreshold() const;

   // deletes the oldest maps until the cache size is less than
   // 75% of the max size.
   // Returns true if more cleanup is required, false otherwise.
   bool initiateCleanup();

   // initiates reading for one map from the m_readQueue 
   bool initiateRead();

   // initiates async timestamp update of one map from the m_updateQueue 
   bool initiateUpdate();

   // writes one map and description to the DB 
   void writeOneMap(const MC2SimpleString& descr, BitBuffer* mapData);

   // creates an SQL query from the specified text after appending the
   // provided suffix to it. A single quote is also appended after the suffix.
   void createSQLQuery(TDes& sqlQuery, const char* queryText, const char* suffix);

   // creates an SQL UPDATE query for the specified map.
   // The time is taken from the current system time.
   void createUPDATEQuery(TDes& sqlQuery, const char* mapName);

private:

   RDbNamedDatabase m_database;
   RDbView m_dbView;

   // if DB is connected, then this is true, else false
   bool m_dbConnected;

   // timestamp queue
   typedef std::list<MC2SimpleString> mapUpdateQueue_t;
   mapUpdateQueue_t m_updateQueue;

   // queue for the maps to write
   typedef std::map<MC2SimpleString,BitBuffer*> mapWriteQueue_t;
   mapWriteQueue_t m_writeQueue;

   // queue for the maps to be read
   typedef std::pair<MC2SimpleString,DBufRequestListener*> readInfo_t;
   typedef std::list< readInfo_t > mapReadQueue_t;
   mapReadQueue_t m_readQueue;

   // timer for updating the DB
   CIdle* m_timeUpdateTimer;

   // set to true if doing cleanup, false otherwise
   bool m_doingCleanup;

   // string buffer for the SQL queries
   TBuf<256> m_sqlQuery;

   // stores the state that the FileDbuf is in.
   int m_State;
   
   // the listener to call when the read is complete
   DBufRequestListener* m_dbufListener;

   // the map that is being currently processed
   MC2SimpleString m_curMapDescr;

   // the BitBuffer data for the map that is 
   // being currently processed in the AO.
   BitBuffer* m_curBitBuffer;

   /* reads the dumped maps from a disk file */
   CMapDumper* m_dumpReader;
};


#endif
