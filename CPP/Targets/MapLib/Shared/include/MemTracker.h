/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MEMTRACKER
#define MEMTRACKER 

#include "config.h"
#include <map>
#include "TileMapToolkit.h"

/// Abstract class representing a memory consuming object.
class MemEater 
{
   public:
      friend class MemTracker;
 
      virtual void cleanUpMemUsage() = 0;
      
   private:
      
      uint32 m_memTrackerID;
};

/**
 *    Adaptor for a memory consuming object.
 *    Used so that all objects used by the MemTracker doesn't have to
 *    inherit from MemEater.
 */
template<class T> class MemEaterAdaptor : public MemEater
{
  
   MemEaterAdaptor( T* memEater ) : m_memEater( memEater ) {}
   
   void cleanUpMemUsage() {
      m_memEater->cleanUpMemUsage();
   }
   
   T* m_memEater;
};

/**
 *    Class that keeps track of how much memory that is used
 *    for certain things. This class should be shared between different
 *    memory eating objects. 
 *    XXX: Is not thread safe!
 */
class MemTracker {
public:
   
   /**
    *   Constructor. Initializes the MemTracker the available amount
    *   of memory in the memory pool, as well as the minimum amount
    *   of free memory that must be present.
    *   
    *   @param availMemInPool The amount of available memory in bytes that
    *                         can be used for the memory pool.
    *                         
    */
   MemTracker( int availMemInPool,
               uint32 minFreeMem = 1024 * 1024 
               ) : m_availMemInPool( availMemInPool ),
                   m_toolkit( NULL ),
                   m_minFreeMem( minFreeMem ) { }
 
   /**
    *    Set the toolkit.
    */
   void setToolkit( const TileMapToolkit* toolkit ) 
   {
      m_toolkit = toolkit;
   }
  
   /**
    *   Release the number of bytes to the memory pool. 
    */
   void releaseToPool( int bytes )
   {
      m_availMemInPool += bytes;
      mc2log << "[MemTracker]: releaseToPool( " << bytes 
             << "), avail in pool = " 
             << m_availMemInPool << endl;
   }

   /**
    *   Allocate the number of bytes from the memory pool.
    */
   void allocateFromPool( int bytes )
   {
      m_availMemInPool -= bytes;
      mc2log << "[MemTracker]: allocateFromPool( " << bytes 
             << "), avail in pool = " 
             << m_availMemInPool << endl;
   }
     
   /**
    *    @return The ID of the mem eater.
    */
   void addMemEater( MemEater* memEater ) 
   {
      uint32 id = m_memEaters.size();
      m_memEaters[ id ] = memEater;
      memEater->m_memTrackerID = id;
   }

   /**
    *    Remove the mem eater with the specified ID.
    */
   void removeMemEater( MemEater* memEater )
   {
      m_memEaters.erase( memEater->m_memTrackerID );
   }
    
   /**
    *    Remove all mem eaters.
    */
   void removeAllMemEaters()
   {
      m_memEaters.clear();
   }
 
   /**
    *
    */
   void cleanUpMemUsage() 
   {
      mc2log << "[MemTracker]: before cleanUpMemUsage, avail in pool = " 
             << m_availMemInPool << endl;
      for ( std::map<uint32, MemEater*>::iterator it = m_memEaters.begin();
            it != m_memEaters.end(); ++it ) {
         (*it).second->cleanUpMemUsage();
      }
      mc2log << "[MemTracker]: after cleanUpMemUsage, avail in pool = " 
             << m_availMemInPool << endl;
   }

   /**
    *
    */
   void checkCleanMemUsage() 
   {
      mc2log << "[MemTracker]: checkCleanMemUsage, avail in pool = " 
             << m_availMemInPool << endl;
      uint32 biggestBlock = 0;
      if ( ( m_availMemInPool < 0 ) || 
           ( m_toolkit != NULL &&
             m_toolkit->trustAvailableMemory() && 
             m_toolkit->availableMemory( biggestBlock ) < m_minFreeMem ) ) {
         cleanUpMemUsage();
      }
   }
   
   
private:

   /// The amount of available memory.
   int m_availMemInPool;

   /// The toolkit.
   const TileMapToolkit* m_toolkit;
   
   /// Minimum amount of total free memory allowed.
   uint32 m_minFreeMem;
   
   /// The mem eaters.
   std::map<uint32, MemEater*> m_memEaters;
   
};


#endif
