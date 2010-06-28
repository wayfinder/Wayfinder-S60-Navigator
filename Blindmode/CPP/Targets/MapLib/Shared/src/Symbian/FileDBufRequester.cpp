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
#include "TileMapUtil.h"
#include <stdio.h>
#include <eikenv.h>
#include <string.h>
#include <f32file.h>
#include <s32file.h>
#include <coeutils.h>

#include <eikenv.h>
#include <bautils.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikappui.h>

#include "LogFile.h"

using namespace std;

#ifndef USE_TRACE
 #define USE_TRACE
#endif

#undef USE_TRACE

#ifdef USE_TRACE
 #include "TraceMacros.h"
#endif

/* macros to display above messages */
#define SHOWMSGWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericMsgTitle, x)
#define SHOWMSG(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 1000000)
#define SHOWERRWIN(x)   CEikonEnv::Static()->InfoWinL(KMsgGenericErrorTitle, x)
#define SHOWERR(x)      CEikonEnv::Static()->InfoMsgWithDuration(x, 2000000)
#define SHOWBUSY(x)     CEikonEnv::Static()->BusyMsgL(x)
#define CANCELBUSY()    CEikonEnv::Static()->BusyMsgCancel()

/* converts a c-string to a unicode Symbian descriptor */
static int char2TDes(TDes &dest, const char* src)
{
   dest.Zero();
   if (!src) {
      dest.PtrZ();
      return 0;
   }
   int length = strlen(src);
   int i = 0;

   // Crashes if we try to write more then maxlength in the buffer
   if( length >= dest.MaxLength() )
      length = dest.MaxLength()-1;

   while (i < length) {
      dest.Append((unsigned int)(src[i]));
      i++;
   }
   dest.PtrZ();
   return length;
}

/* returns the complete path to a file located in the application's folder */
static int GetCompletePath(const TDesC& fileName, TDes& fullPath) {
   /* zero out the destination descriptor */
   fullPath.Zero();
   /* get our application's path using the CEikApplication */
   /* the trailing slash is present after the folder name */
   fullPath.Copy(BaflUtils::DriveAndPathFromFullName(CEikonEnv::Static()->EikAppUi()->Application()->AppFullName()));
   /* append the filename to the path */
   fullPath.Append(fileName);
   /* success */
   return(0);
}


//********* CMAPDUMPER *********

/* name of the dump file */
_LIT(KDumpFile, "MapFile.dump");

/* messages */
_LIT(KDumpReadBegin, "Reading Dumped Map...");
_LIT(KDumpRead, "Read Dumped Map!");
_LIT(KDumpReadError, "Dump Read Error!");

/* constructor */
CMapDumper::CMapDumper(FileDBufRequester& req)
   : CActive(EPriorityHigh),
     m_mapDataReadBuffer(NULL, 0, 0),
     m_state(EIdle),
     m_numberOfMaps(0),
     m_allMapsRead(false),
     m_requester(req)
{
}

/* second-phase constructor */
int CMapDumper::Construct()
{
   /* add ourselves to the scheduler */
   CActiveScheduler::Add(this);

   /* get the complete path */
   TBuf<128> fullPath;
#ifdef __WINS__
      GetCompletePath(KDumpFile, fullPath);
      fullPath.Copy(_L("c:\\system\\data\\"));      
      fullPath.Append(KDumpFile);
#else
      GetCompletePath(KDumpFile, fullPath);
#endif

   /* check if the file exists */
   if(!ConeUtils::FileExists(fullPath)) {
      /* no maps to be read */
      m_allMapsRead = true;
      m_numberOfMaps = 0;
      return(KErrNone);
   }

   /* the file exists; open it */
   int errCode = m_file.Open(CEikonEnv::Static()->FsSession(), 
                             fullPath,
                             EFileWrite);
   /* if error on opening file, return */
   if(errCode != KErrNone) {
      return(errCode);
   }

   /* read the number of maps */
   TPtr8 numMaps((TUint8*)&m_numberOfMaps, sizeof(uint32), sizeof(uint32));
   numMaps.Set( (TUint8*)&m_numberOfMaps, sizeof(uint32), sizeof(uint32) );
   m_file.Read(numMaps, sizeof(uint32), iStatus);
   User::WaitForRequest(iStatus);
   /* if no maps, set all maps read to true */
   if(m_numberOfMaps == 0) {
      m_allMapsRead = true;
   }

   /* file is ready, return success */
   return(KErrNone);
}

/* destructor */
CMapDumper::~CMapDumper()
{
   Cancel();
   m_file.Close();
   Deque();
}

/* allocator */
CMapDumper* CMapDumper::New(FileDBufRequester& req)
{
   /* create a new object */
   CMapDumper* newObj = new CMapDumper(req);
   if(newObj == NULL) {
      return(NULL);
   }
   /* do second-phase construction */
   if(newObj->Construct() != KErrNone) {
      /* file could not be opened, delete the object and return NULL */
      delete newObj;
      return(NULL);
   }

   return(newObj);
}

static uint32 convertFromBE(uint32 invalue)
{
   uint32 tmpVal = BigEndian::Get32((TUint8*)&invalue);
   return tmpVal;
}

/* 
 * Asynchronous read, it reads the map and then informs the 
 * parent requester.
 */
void CMapDumper::readAsync()
{
   TPtr8 readBuffer(NULL,0,0);

   /* maintain the current file position to use in case of error */
   m_curPos = m_file.TellL(MStreamBuf::ERead);

   /* read the map name length synchronously */
   uint32 descrLen = 0;
   readBuffer.Set((TUint8*)&descrLen, sizeof(uint32), sizeof(uint32));
   m_file.Read(readBuffer, sizeof(uint32), iStatus);
   User::WaitForRequest(iStatus);
   
   // Swap byte order if necessary
   bool bigendian = false;
   if ( descrLen > 0xffff ) {
      // It is not likely that the description is bigger than 0xffff.
      descrLen = convertFromBE( descrLen );
      bigendian = true;
   }

   /* read the map name */
   TBuf8<128> descr;
   m_file.Read(descr, descrLen, iStatus);
   User::WaitForRequest(iStatus);
   m_mapDescr = (const char*)descr.PtrZ();

   /* read the map data length */
   uint32 dataLen = 0;
   readBuffer.Set((TUint8*)&dataLen, sizeof(uint32), sizeof(uint32)); 
   m_file.Read(readBuffer, sizeof(uint32), iStatus);
   User::WaitForRequest(iStatus);
   
   // Swap byte order if necessary
   if ( bigendian ) {
      dataLen = convertFromBE( dataLen );
   }

   /* create the BitBuffer */
   m_bitBuffer = new BitBuffer(dataLen);
   if(m_bitBuffer == NULL) {
      /* oops, no memory */
      /* restore read stream position */
      m_file.SeekL(MStreamBuf::ERead, m_curPos);
      return;
   }

   /* start a read */
   m_state = EReading;
   m_mapDataReadBuffer.Set(m_bitBuffer->getBufferAddress(), 
                           m_bitBuffer->getBufferSize(),
                           m_bitBuffer->getBufferSize());
   
   m_file.Read(m_mapDataReadBuffer, m_bitBuffer->getBufferSize(), iStatus);
   /* make ourselves active */
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();

   SHOWMSG(KDumpReadBegin);

   return;
}

/* from CActive */
void CMapDumper::RunL()
{
   switch(m_state)
   {
      case(EReading):
      {
         /* check for success */
         if(iStatus == KErrNone) {
            /* map was read successfully, inform the parent requester */
            m_requester.writeToFile(m_mapDescr, m_bitBuffer);
            m_state = EIdle;
            /* decrement map count */
            m_numberOfMaps--;
            /* check if it zero */
            if(m_numberOfMaps == 0) {
               /* all maps are read, set the flag to true */
               m_allMapsRead = true;
            }
            /* check if any more reads are required */
            if(m_allMapsRead == false) {
               /* initiate another read */
               readAsync();
            }

            /* log the writes */
            LogFile::Print("Dump Read : %s [%u], %u bytes\n", 
                  m_mapDescr.c_str(), m_mapDescr.length(),
                  m_bitBuffer->getBufferSize() );

            SHOWMSG(KDumpRead);
         }
         else {
            /* error during read, restore read position */
            m_file.SeekL(MStreamBuf::ERead, m_curPos);
            SHOWMSG(KDumpReadError);
         }
         break;
      }
      default:
      {
         break;
      }
   }

   return;
}

void CMapDumper::DoCancel()
{
   if(m_state == EReading) {
      /* restore the read position */
      m_file.SeekL(MStreamBuf::ERead, m_curPos);
   }
   return;
}


//********* FILEDBUFREQUESTER *********

/* error messages */
_LIT(KMsgGenericMsgTitle, "TMap : ");
_LIT(KMsgGenericErrorTitle, "TMap : Error :");

/* info messages */
_LIT(KMsgCacheCleaned, "Cache Cleaned!");
_LIT(KMsgTimeWritten, "Asynchronous TimeStamp Written!");
_LIT(KMsgCacheWritten, "Cache Written!");
_LIT(KMsgCacheRead, "Asynchronous Cache Read!");
_LIT(KBusyMsgRecovery, "DB Recovered!");

#define PATH_SEPARATOR "\\"
#define PATH_SEPARATOR_LENGTH 1
#define MAX_DESCR_LENGTH 40
#define MAX_PATH_LENGTH 255
_LIT(KDbFileName,"MapCache.db");
_LIT(KSeparator, "\\");

FileDBufRequester::FileDBufRequester(DBufRequester* parent,
                                     const char* path,
                                     uint32 maxMem)
: DBufRequester(parent),
  CActive(EPriorityNormal),
   m_path(path),
   m_maxCacheSize(maxMem),
   m_cacheSize(0),
   m_dbConnected(false),
   m_doingCleanup(false),
   m_State(EIdle),
   m_dbufListener(NULL),
   m_curBitBuffer(NULL)
{
   // start up the database
   TBuf<MAX_PATH_LENGTH> fullPath;
   char2TDes(fullPath, path);
   fullPath.Append(KSeparator);
   fullPath.Append(KDbFileName);

   /* trap any possible error, as constructors can't Leave.
      If error occurs during initializing the DB, then we set
      the m_dbConnected variable to false */
   TInt errCode;
   errCode = StartDb(fullPath);
   if(errCode == KErrNone) {
      /* DB is working */
      m_dbConnected = true;
   } else {
      /* could not connect to DB */
      m_dbConnected = false;
   }

   // calculate cleanup threshold .. currently 75% of maximum size
   m_cleanupThresholdSize = (uint32)((float)m_maxCacheSize * 0.75f);

   // get the current cache size from DB
   createInitialByteCount();

   // check if cache is too large
   if(cacheNeedsCleanup()) {
      m_doingCleanup = true;
   }

   /* start the map reading AO */
   m_dumpReader = CMapDumper::New(*this);
   if(m_dumpReader != NULL) {
      /* start reading the maps from the file, if there are any maps */
      if(m_dumpReader->allMapsRead() == false) {
         m_dumpReader->readAsync();
      }
   }

   // start the idle timer for updating timestamps
   m_timeUpdateTimer = CIdle::New(CActive::EPriorityIdle);
   
   /* add to active scheduler */
   CActiveScheduler::Add(this);
}

FileDBufRequester::~FileDBufRequester()
{
   /* delete the map reader */
   if(m_dumpReader) {
      m_dumpReader->Cancel();
      delete m_dumpReader;
   }

   // shutdown the timer
   m_timeUpdateTimer->Cancel();
   delete m_timeUpdateTimer;

   // shutdown the AO
   Cancel();

   // write leftover maps to DB */
   writeRemaindersToDB();

   // clear the read cache 
   m_readQueue.clear();

   // clear the update queue
   m_updateQueue.clear();

   // compress and close the DB
   compressCache();

   // close the database
   m_database.Close();

   /* remove from active scheduler */
   Deque();
}

void
FileDBufRequester::setMaxSize(uint32 size)
{
   m_maxCacheSize = size;
   // calculate cleanup threshold .. currently 75% of maximum size
   m_cleanupThresholdSize = (uint32)((float)m_maxCacheSize * 0.75f);
}

void
FileDBufRequester::clearCache()
{
   // How?
}

bool
FileDBufRequester::writeToFile(const MC2SimpleString& descr,
                               BitBuffer* buffer)
{
   // check if the map is present in the cache
   mapWriteQueue_t::iterator it = m_writeQueue.find(descr);
   if(it != m_writeQueue.end()) {
      /* its present, return true */
      /* it's in queue to be written */
      // MEMORY LEAK!
      return(true);
   }

   // map is not in writing queue, add it
   m_writeQueue.insert( make_pair(descr,buffer) );
   // start the timer
   if ( m_timeUpdateTimer ) {
      if( !m_timeUpdateTimer->IsActive() ) {
         m_timeUpdateTimer->Start(TCallBack(UpdateCallback,this));
      }
   }

   return true;
}

void FileDBufRequester::request(const MC2SimpleString& descr,
                                DBufRequestListener* caller,
                                request_t )
{
   BitBuffer* cachedBuffer = requestCached( descr );
   // Check cache first.
   if ( cachedBuffer != NULL ) {
      caller->requestReceived(descr, cachedBuffer, *this );
      return;
   }
   mapReadQueue_t::value_type thePair = make_pair( descr, caller );
   // Check if the pair is already in the queue.
   if ( std::find( m_readQueue.begin(), m_readQueue.end(), thePair ) !=
        m_readQueue.end() ) {
      // Already in queue
   } else {
      /* insert the currently required map onto the queue */
      m_readQueue.push_back( thePair );
   }
   
   /* The queue has entries since we just pushed back an entry,
      initiate the evaluation */
   
   /* check the state of the read queue */
   if( m_State == EIdle ) {
      if ( m_readQueue.empty() ) {
         // Nothing to do (should not happen since something is enqueued
         // or already found above )
         return;
      } else {
         // Start the reading AO.
         initiateRead();
      }
   }
}

void
FileDBufRequester::cancelAll()
{
   m_timeUpdateTimer->Cancel();
   Cancel();
   m_readQueue.clear();
}

BitBuffer*
FileDBufRequester::requestCached(const MC2SimpleString& descr)
{
   /* check if BitBuffer exists in the write queue */
   mapWriteQueue_t::iterator it = m_writeQueue.find(descr);
   if(it != m_writeQueue.end()) {
      /* found the map, do the needful */
      BitBuffer* buf = it->second;
      buf->reset();
      m_writeQueue.erase(it);
      return(buf);
   }
   else {
      return(NULL);
   }
}

void
FileDBufRequester::release(const MC2SimpleString& descr,
                             BitBuffer* buffer)
{
   if ( buffer != NULL ) 
   {
      writeToFile(descr, buffer);
   }  
   return;
}

TInt 
FileDBufRequester::StartDb(TDesC& aFullFilePath)
{
   TInt err;

   if(ConeUtils::FileExists(aFullFilePath))
   {
      err = m_database.Open(CEikonEnv::Static()->FsSession(),aFullFilePath);
      if(err == KErrNone) {
         /* check if DB is damaged, try and recover it */
         if(m_database.IsDamaged()) {
            /* DB is damamged, recover */
            /* start recovery */
            TInt errCode = m_database.Recover();
            if(errCode != KErrNone) {
               /* error during recovery */
               m_database.Close();
               err = errCode;
            }
            SHOWMSGWIN(KBusyMsgRecovery);
         }
      }
   }
   else
   {
      err = m_database.Create(CEikonEnv::Static()->FsSession(),aFullFilePath);
      if(err == KErrNone) {
	      TRAP(err, CreateMapTableL());
         if(err != KErrNone) {
            m_database.Close();
         }
      }
   }

   return err;
}

void FileDBufRequester::CreateMapTableL()
{
   // Create the table with columns
   CDbColSet* columns= CDbColSet::NewLC();
   // Create text column for the description of the tilemap

   // create text format for the map description
   _LIT(KTileMapDescr,  "tilemapdesc");
   TDbCol tileMapDescr(KTileMapDescr, EDbColText8, MAX_DESCR_LENGTH);
   tileMapDescr.iAttributes = TDbCol::ENotNull;
   columns->AddL(tileMapDescr);

   // Create binary for tile map data.
   _LIT(KTileMapData,   "tilemapdata");
   TDbCol tileMapData(KTileMapData, EDbColLongBinary);
   tileMapData.iAttributes = TDbCol::ENotNull;
   columns->AddL(tileMapData);

   // Create time stamp for tile map data.
   _LIT(KTileMapTime,   "lastaccesstime");
   TDbCol tileMapTime(KTileMapTime, EDbColDateTime);
   tileMapTime.iAttributes = TDbCol::ENotNull;
   columns->AddL(tileMapTime);

   // Create the table.
   _LIT(KTable,         "maps");
   User::LeaveIfError (m_database.CreateTable (KTable, *columns));
   CleanupStack::PopAndDestroy(); // columns

   // START :: Creating Table Index 
   _LIT(KName,  "tilemapdesc");
   _LIT(KIndex, "mc2_index");
   CDbKey* key = CDbKey::NewLC();

   TDbKeyCol keyCol(KName);
   key->AddL(keyCol);
   key->MakeUnique();
   User::LeaveIfError(m_database.CreateIndex(KIndex,KTable,*key));
   CleanupStack::PopAndDestroy(1); // key
   //	 END
}

bool FileDBufRequester::IsRecordPresent(const MC2SimpleString& descr)
{
   // create the query
   createSQLQuery(m_sqlQuery, 
                  "SELECT tilemapdesc FROM maps WHERE tilemapdesc = '", 
                  descr.c_str());

   // Start doing the DB-stuff
   m_dbView.Prepare(m_database, TDbQuery(m_sqlQuery),RDbView::EReadOnly);
   m_dbView.EvaluateAll();

   bool isPresent;
   if(m_dbView.IsEmptyL())   
   {
      isPresent = false;
   }
   else
   {
      isPresent = true;
   }
   m_dbView.Close();

   return isPresent;
}

// counts the number of bytes in cached maps during DB Initialization
void FileDBufRequester::createInitialByteCount() {

   _LIT(KSQLQuery, "SELECT tilemapdata,tilemapdesc FROM maps");
   
   m_dbView.Prepare(m_database, TDbQuery(KSQLQuery));
   m_dbView.EvaluateAll();
   if(m_dbView.IsEmptyL()) {
      return;
   }

   uint32 curMapSize = 0;
   while (m_dbView.NextL())
   {
      m_dbView.GetL();
      // get size of the map
      curMapSize = (uint32)m_dbView.ColLength(1); 
      // add it to current byte count
      m_cacheSize += curMapSize;
   }

   m_dbView.Close();

   return;
}

// initiates reading for one map from the m_readQueue 
bool FileDBufRequester::initiateRead()
{
   /* get the map to be read from the start of the queue */
   mapReadQueue_t::iterator it = m_readQueue.begin();
   m_curMapDescr = it->first;
   m_dbufListener = it->second;

   // create the query
   createSQLQuery(m_sqlQuery, 
                  "SELECT tilemapdata FROM maps WHERE tilemapdesc = '", 
                  m_curMapDescr.c_str());

   // Start doing the DB-stuff
   m_dbView.Prepare(m_database, TDbQuery(m_sqlQuery),RDbView::EReadOnly);

   // set the flag to specify that we are reading
   m_State = EReading;

   // activate!
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();

   // request the async evaluation
   m_dbView.Evaluate(iStatus);

   /* check if any more maps need reading */
   if(m_readQueue.empty()) {
      return(false);
   }
   else {
      return(true);
   }
}

// updates the timestamp of one map from the m_updateQueue 
bool FileDBufRequester::initiateUpdate() {

   /* get the map to be updated from the start of the queue */
   mapUpdateQueue_t::iterator it = m_updateQueue.begin();

   if(it != m_updateQueue.end()) {
      m_curMapDescr = *it;
      m_State = EUpdating;

      /* create the query */
      createSQLQuery(m_sqlQuery, 
                     "SELECT lastaccesstime FROM maps WHERE tilemapdesc = '",
                     m_curMapDescr.c_str());
   
      /* prepare the DB and initiate the Asynch evaluation */
      int err = m_dbView.Prepare(m_database, TDbQuery(m_sqlQuery),
                                 RDbView::EUpdatable);
      if(err != KErrNone) {
         User::Panic(_L("Error on Update!"), err);
      }

      /* set ourselves to active */
   #ifdef USE_TRACE
      TRACE_FUNC();
   #endif
      SetActive();

      /* GO ASYNC! */
      m_dbView.Evaluate(iStatus);

      /* remove the map from the update queue */
      m_updateQueue.pop_front();
   }

   /* if no more timestamps need updating, return false */
   if(m_updateQueue.empty()) {
      return(false);
   }
   else {
      return(false);
   }
}

// checks if cache size is over the specified limit. if it is,
// returns true, else false.
bool FileDBufRequester::cacheNeedsCleanup() const {
   /* check if the current cache size is greater than the allowed max size */
   if(m_cacheSize > m_maxCacheSize) {
      return(true);
   } 
   else {
      return(false);
   }
}

// checks if cache size is under the specified cleanup threshold.
// if it is, returns true, else false.
bool FileDBufRequester::cacheUnderThreshold() const {
   if(m_cacheSize < m_cleanupThresholdSize) {
      return(true);
   } 
   else {
      return(false);
   }
}

// deletes the oldest maps until the cache size is less than
// 75% of the max size.
// returns True, if more cleanup is required, False otherwise
bool FileDBufRequester::initiateCleanup() {

   /* Cache size greater than the allowed maximum. Chuck the oldest maps 
    * until the cache size is lesser than m_maxCacheSize.
    */

   // if DB is not connected, return quietly
   if(!m_dbConnected) return(false);

   // gets all rows from the DB sorted in oldest-first order
   _LIT(KSQLQuery, "SELECT tilemapdata,tilemapdesc,lastaccesstime FROM maps ORDER BY lastaccesstime");
  
   /* prepare the DB */
   m_dbView.Prepare(m_database, TDbQuery(KSQLQuery));

   /* set state to cleanup */
   m_State = ECleaning;

   /* set ourselves to active */
#ifdef USE_TRACE
   TRACE_FUNC();
#endif
   SetActive();

   /* start the evaluation */
   m_dbView.Evaluate(iStatus);

   /* check if more cleanup is required */
   if(cacheUnderThreshold()) {
      return(false);
   }
   else {
      return(true);
   }
}

// writes one map and description to the DB 
void FileDBufRequester::writeOneMap(const MC2SimpleString& descr, 
                                    BitBuffer* mapData) {

   // if DB is not connected, return quietly
   if(!m_dbConnected) return;

   /* create the TPtrC8 for the data */
   TPtrC8 descrPtr((TUint8*)descr.c_str());
   TPtrC8 bufPtr(mapData->getBufferAddress(),
                 mapData->getBufferSize());

   // Petersv says that the constructors of TPtrC8 do not work.
   descrPtr.Set( (TUint8*)descr.c_str() );
   bufPtr.Set( mapData->getBufferAddress(),
               mapData->getBufferSize() );

   /* create the update SQL Query */
   TBuf<256> sqlQuery;
   createSQLQuery(sqlQuery, 
     "SELECT tilemapdesc,tilemapdata,lastaccesstime FROM maps WHERE tilemapdesc = '",
     descr.c_str());
                  
   /* prepare the view for transactions */
   m_dbView.Prepare(m_database, TDbQuery(sqlQuery),
                    RDbView::EUpdatable);

   /* do the evaluation */
   m_dbView.EvaluateAll();

   /* if the rowset is empty, insert the map */
   if( m_dbView.IsEmptyL() ) {
      /* insert a new empty row */
      m_dbView.InsertL();
   
      /* First Column insertion */
      m_dbView.SetColL(1,descrPtr);

      /* Second Column insertion */
      RDbColWriteStream writeStream;
      writeStream.OpenL(m_dbView, 2);
      writeStream.WriteL(bufPtr, bufPtr.Length());
      writeStream.Close();

      /* Time stamp insertion */
      TTime CurrTime;
      CurrTime.HomeTime();
      m_dbView.SetColL(3,CurrTime);

      /* commit the stuff to the DB */
      m_dbView.PutL();
      m_dbView.Close();

      /* add the bitbuffer size to the byte count */
      m_cacheSize += mapData->getBufferSize();
   }
   else {
      /* the map is present..update the map */
      m_dbView.FirstL();
      m_dbView.GetL();

      /* get the size of the current map */
      uint32 curMapSize = m_dbView.ColLength(2);

      /* update the map */
      m_dbView.UpdateL();

      /* First Column update */
      m_dbView.SetColL(1,descrPtr);

      /* Second Column update */
      RDbColWriteStream writeStream;
      writeStream.OpenL(m_dbView, 2);
      writeStream.WriteL(bufPtr, bufPtr.Length());
      writeStream.Close();

      /* Time stamp insertion */
      TTime CurrTime;
      CurrTime.HomeTime();
      m_dbView.SetColL(3,CurrTime);

      /* commit the stuff to the DB */
      m_dbView.PutL();
      m_dbView.Close();

      /* subtract previous map size from byte count */
      m_cacheSize -= curMapSize;
      /* add the bitbuffer size to the byte count */
      m_cacheSize += mapData->getBufferSize();
   }

   // check if cleanup is required
   if(cacheNeedsCleanup()) {
      // set cleanup flag
      m_doingCleanup = true;
   }

   /* delete the BitBuffer using the parent release() */
   DBufRequester::release(descr, mapData);

   SHOWMSG(KMsgCacheWritten);

   return;
}

// callback for the timestamp timer 
TInt FileDBufRequester::UpdateCallback(TAny* thisPtr) {

   /* this is set to 1 if the callback needs to be called again, else 0 */
   TInt repeatFlag = 0;

   /* get the pointer */
   FileDBufRequester* req = (FileDBufRequester*)thisPtr;

   /* check if any DB activity is on */
   if(req->m_State != EIdle) {
      /* schedule it to be called again */
      repeatFlag = 1;
   }
   /* check cache size */
   else if(req->m_doingCleanup == true) {
      /* cleanup required, do it! */
      if(req->initiateCleanup()) {
         /* more cleanup is required */
         repeatFlag = 1;
      }
      else {
         /* set cleanup flag to false */
         req->m_doingCleanup = false;
      }
   }
#if 0
   /* check if there any timestamp updates required */
   else if(req->m_updateQueue.empty() == false) {
      /* ask for the update */
      if(req->initiateUpdate()) {
         repeatFlag = 1;
      }
   }
#endif
   /* check if any maps need to get written */
   else if(req->m_writeQueue.size() != 0) {
      
      /* write a map and then check if the timer should continue */
      mapWriteQueue_t::iterator it = req->m_writeQueue.begin();

      /* write the map */
      req->writeOneMap(it->first, it->second);
      
      /* erase the map from the array */
      req->m_writeQueue.erase(it);

      /* check if any more maps need to be written */
      if(req->m_writeQueue.size() != 0) {
         /* more writing needed, continue the timer */
         repeatFlag = 1;
      }
   }

   /* return with the repeat flag */
   return(repeatFlag);
}

// creates an SQL query from the specified text after appending the
// provided suffix to it. A single quote is also appended after the suffix.
void FileDBufRequester::createSQLQuery(TDes& sqlQuery, 
                                       const char* queryText, 
                                       const char* suffix)
{
   char finalResult[256];
   /* do the appropriate concatenating */
   strcpy(finalResult, queryText);
   strcat(finalResult, suffix);
   strcat(finalResult, "'");
   /* copy the ASCIIZ string to the Unicode Query descriptor */
   char2TDes(sqlQuery, finalResult);
   return;
}

// creates an SQL UPDATE query for the specified map.
// The time is taken from the current system time.
void FileDBufRequester::createUPDATEQuery(TDes& sqlQuery, 
                                          const char* mapName)
{
   _LIT(KUpdateQueryOne, "UPDATE maps SET lastaccesstime = '");
   _LIT(KUpdateQueryTwo, " WHERE tilemapdesc = '");
   _LIT(KUpdateQuerySuffix, "'");
   _LIT(KTimeFormat, "%i-%i-%i %i:%i:%i");

   /* get the map name as a Unicode descriptor */
   TBuf<64> mapDesc;
   char2TDes(mapDesc, mapName);

   /* get the current time as a string */
   TTime curTime;
   TDateTime dateTime;
   TBuf<128> curTimeStr;
   curTime.HomeTime();
   dateTime = curTime.DateTime();
   curTimeStr.Format(KTimeFormat, dateTime.Day(),
                                  dateTime.Month(),
                                  dateTime.Year(),
                                  dateTime.Hour(),
                                  dateTime.Minute(),
                                  dateTime.Second() );

   /* do the appropriate concatenating */
   sqlQuery.Copy(KUpdateQueryOne);
   sqlQuery.Append(curTimeStr);
   sqlQuery.Append(KUpdateQuerySuffix);
   sqlQuery.Append(KUpdateQueryTwo);
   sqlQuery.Append(mapDesc);
   sqlQuery.Append(KUpdateQuerySuffix);

   return;
}

/* from CActive */
void FileDBufRequester::RunL() {

   /* act according to the state we are in */
   switch(m_State)
   {
      case(EReading):
         {
            /* check if any more evaluation is required */
            if(iStatus == 0) {
               /* evaluation is done, read the map if present */
               if(m_dbView.IsEmptyL()) {
                  m_dbView.Close();
                  /* call the parent requester to request the map */
                  if ( m_parentRequester ) {
                     m_parentRequester->request(m_curMapDescr, m_dbufListener,
                                                cacheOrInternet);
                  }

                  /* remove the map from the read queue */
                  if(!m_readQueue.empty()) {
                     m_readQueue.pop_front();
                  }

                  /* nullify the variables */
                  m_curMapDescr = "";
                  m_dbufListener = NULL;
                  m_State = EIdle;
               }
               else {
                  /* read the map */

                  /* retrieve the row from the rowset */
                  m_dbView.NextL();
                  m_dbView.GetL();

                  /* create a read stream */
                  RDbColReadStream readStr;
                  readStr.OpenL(m_dbView, 1);
   
                  /* read the map into a BitBuffer */
                  BitBuffer* retBuf = new BitBuffer(m_dbView.ColLength(1));
                  TPtr8 mapdatabuf(retBuf->getBufferAddress(),
                                   retBuf->getBufferSize());
                  // Explicitly set the stuff too.
                  mapdatabuf.Set( (TUint8*)retBuf->getBufferAddress(),
                                  retBuf->getBufferSize(),
                                  retBuf->getBufferSize() );
                  readStr.ReadL(mapdatabuf, m_dbView.ColLength(1)); 
                  readStr.Close();

                  /* clean up */
                  m_dbView.Close();

                  /* call the listener with the buffer */
                  m_dbufListener->requestReceived(m_curMapDescr, retBuf, *this);

                  /* remove the map from the read queue */
                  if(m_readQueue.empty() == false) {
                     m_readQueue.pop_front();
                  }


                  /* add this map to the timestamp update list */
                  m_updateQueue.push_back(m_curMapDescr);

                  /* nullify the variables */
                  m_curMapDescr = "";
                  m_dbufListener = NULL;

                  /* set the state to Idle */
                  m_State = EIdle;

                  /* show debug message */
                  SHOWMSG(KMsgCacheRead);
               }
            }
            else if(iStatus > 0) {
               /* set the state to Reading */
               m_State = EReading;
               /* set ourselves to active */
            #ifdef USE_TRACE
               TRACE_FUNC();
            #endif
               SetActive();
               /* more evaluation needs to be done */
               m_dbView.Evaluate(iStatus);
            }
            else {
               /* an error occurred .. close all and return quietly */
               /* remove the map from the read queue */
               if(m_readQueue.empty() == false) {
                  m_readQueue.pop_front();
               }
               /* nullify the variables */
               m_curMapDescr = "";
               m_dbufListener = NULL;
               /* close the DB View */
               m_dbView.Close();
               /* set the state to Idle */
               m_State = EIdle;
            }
            break;
         }
      case(EWriting):
         {
            /* not implemented yet */
            break;
         }
      case(EUpdating):
         {
            /* check for status of the evaluation */
            if(iStatus == 0) {
               /* evaluation done */
               /* check if the record is present */
               if(m_dbView.IsEmptyL()) {
                  /* Map is not present .. must have been removed during
                     cache-cleanup. */
                  /* nullify the variables */
                  m_curMapDescr = "";
                  m_State = EIdle;
                  m_dbView.Close();
               }
               else {
                  /* map is present, do the update */
                  m_dbView.NextL();
                  m_dbView.UpdateL();

                  // update time stamp
                  TTime CurrTime;
                  CurrTime.HomeTime();
                  m_dbView.SetColL(1,CurrTime);
                  m_dbView.PutL(); 
                  
                  SHOWMSG(KMsgTimeWritten);

                  /* close the RDBView */
                  m_dbView.Close();

                  /* nullify the variables */
                  m_curMapDescr = "";
                  m_State = EIdle;
               }
            }
            /* more evaluation needed */
            else if(iStatus > 0) {
               /* set the state to Updating */
               m_State = EUpdating;
               /* set ourselves to active */
            #ifdef USE_TRACE
               TRACE_FUNC();
            #endif
               SetActive();
               /* more evaluation needs to be done */
               m_dbView.Evaluate(iStatus);
            }
            else {
               /* an error occurred .. close all and return quietly */
               /* nullify the variables */
               m_curMapDescr = "";
               /* set the state to Idle */
               m_State = EIdle;
               /* close the DB View */
               m_dbView.Close();
            }
            break;
         }
      case(ECleaning):
         {
            /* check for status of the evaluation */
            if(iStatus >= 0) {
               /* evaluation done */
               /* this should not happen, but check anyways */
               if(m_dbView.IsEmptyL()) {
                  /* rowset is empty, return quietly */
                  m_dbView.Close();
                  /* reset state of AO */
                  m_State = EIdle;
                  return;
               }

               /* success, do cleanup */
               uint32 curMapSize;
               TInt errCode;
   
               /* get the next row */
               m_dbView.FirstL();
               m_dbView.GetL();

               /* get the size of the map */
               curMapSize = m_dbView.ColLength(2);

               /* delete the map */
               TRAP(errCode, m_dbView.DeleteL());
               if(errCode != KErrNone) {
                  /* error during deletion, close everything and return */
                  m_dbView.Close();
                  return;
               }

               /* update the cache size */
               m_cacheSize -= curMapSize;

               /* all done, close the view */
               m_dbView.Close();

               /* show message */
               SHOWMSG(KMsgCacheCleaned);

               /* reset variables */
               m_State = EIdle;
            }
            else {
               /* error occurred, return quietly */
               m_dbView.Close();
               /* reset variables */
               m_State = EIdle;
               return;
            }

            break;
         }
      default: break;
   }

   /* if AO is Idle, check if any requests need processing */
   if( m_State == EIdle ) {
      /* any more read requests pending, initiate them */
      if(m_readQueue.empty() == false) {
         initiateRead();
      }
      /* second priority is updating timestamps; check if any are pending */
      else if(m_updateQueue.empty() == false) {
         /* timestamp updates pending */
         initiateUpdate();
      }
      /* third prio is cleanup, check if any is needed */
      else if(cacheNeedsCleanup()) {
         /* initiate another cleanup */
         initiateCleanup();
      }
   }

   return;
}

void FileDBufRequester::DoCancel() {

   /* cleanup resources */
   m_State = EIdle;
   /* nullify the variables */
   m_curMapDescr = "";
   m_dbufListener = NULL;
   /* close the DB View */
   m_dbView.Close();

   return;
}

/* writes leftover maps to DB */
void FileDBufRequester::writeRemaindersToDB() {

   /* open the file */
   RFs& rfs = CEikonEnv::Static()->FsSession();
   RFileBuf outFile;

   TBuf<128> fullPath;
#ifdef __WINS__
   GetCompletePath(KDumpFile, fullPath);
   fullPath.Copy(_L("c:\\system\\data\\"));      
   fullPath.Append(KDumpFile);
#else
   GetCompletePath(KDumpFile, fullPath);
#endif

   if(outFile.Create(rfs, fullPath, EFileWrite) != KErrNone) {
      /* file cannot be created, replace it */
      if(outFile.Replace(rfs, fullPath, EFileWrite) != KErrNone) {
         /* file cannot be opened, all maps shall be lost */
         /* FIXME : initiate DB Writing if file writing fails */
         /* iterate thru the map to write leftovers */
         if(m_writeQueue.size()) {
            for(mapWriteQueue_t::iterator it = m_writeQueue.begin();
                        it != m_writeQueue.end();
                        ++it) {
               /* write one map at a time */
               writeOneMap(it->first, it->second);
            }
            /* clear the array */
            m_writeQueue.clear();
         }

         /* return */
         return;
      }
   }

   /* write the number of maps */
   uint32 numMaps;
   numMaps = m_writeQueue.size();
   outFile.WriteL( &numMaps, sizeof(uint32) );         

   /* check if any maps are left over */
   if(m_writeQueue.size()) {

      /* file opened */
      // Create buffer to use when dumping. This should have enough room
      // for descriptions and lengths. The map will be added if it fits.
      BitBuffer tempBuf(1024);
      
      _LIT(KDumpBusy, "Dumping...");
      SHOWBUSY(KDumpBusy);
      /* iterate thru the map to write leftovers */      
      for(mapWriteQueue_t::iterator it = m_writeQueue.begin();
          it != m_writeQueue.end();
          ++it) {

         const MC2SimpleString& curDesc = it->first;
         const BitBuffer* curBuf        = it->second;
         
         tempBuf.reset();
         
         tempBuf.writeNextBALong( curDesc.length() );
         tempBuf.writeNextByteArray( (const byte*)curDesc.c_str(),
                                     curDesc.length() );
         tempBuf.writeNextBALong( curBuf->getBufferSize() );
         // Check how much room is left in the tempBuf.
         uint32 room = tempBuf.getBufferSize() - tempBuf.getCurrentOffset();
         if ( room > curBuf->getBufferSize() ) {
            // Write the buffer into the temp buffer and then all at once
            // to the file.
            tempBuf.writeNextByteArray( curBuf->getBufferAddress(),
                                        curBuf->getBufferSize() );
            outFile.WriteL( tempBuf.getBufferAddress(),
                            tempBuf.getCurrentOffset() );
         } else {
            // Write the buffers separately.
            outFile.WriteL( tempBuf.getBufferAddress(),
                            tempBuf.getCurrentOffset() );
            outFile.WriteL( curBuf->getBufferAddress(),
                            curBuf->getBufferSize() );
         }
#if 0
         /* write the length of the map description */
         uint32 descrLen = it->first.length();
         outFile.WriteL( &descrLen, sizeof(uint32) );         
         /* write the map description */
         outFile.WriteL( it->first.c_str(), descrLen );
         /* write the length of the map data */
         uint32 mapLen = it->second->getBufferSize();
         outFile.WriteL( &mapLen, sizeof(uint32) );         
         /* write one map at a time */
         outFile.WriteL( it->second->getBufferAddress(), 
                         mapLen );
         /* log the writes */
         LogFile::Print("Dumping : %s [%u], %u bytes\n", 
                        it->first.c_str(), descrLen, mapLen);
#endif
      
         /* delete the BitBuffer using the parent release() */
         DBufRequester::release(it->first, it->second);

      }
      CANCELBUSY();
   }

   /* close all handles */
   outFile.Close();

   /* clear the array */
   m_writeQueue.clear();

   return;
}

