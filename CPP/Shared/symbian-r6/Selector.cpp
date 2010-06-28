/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

 /*
 *    wait_many for sockets.
 */
#include "Selector.h"
#include "Selectable.h"
#include <vector>

using namespace isab;

using std::pair;
using std::make_pair;

/**
 *   Helper class for keeping track of the selectables.
 *   Currently inherited from vector, can be reimplemented if
 *   necessary. Insertion and removal are O(n) which shouldn't
 *   be a problem. Using vector because I want to avoid iterators
 *   in the Selector if the stl cannot be used.
 */
class isab::SelectorSet : private std::vector<pair<Selectable*,
                                                   Selector::selectfor_t> >
{
public:
   /**
    *   Inserts the selectable into the set if it does not exist.
    *   @param s Selectable to insert.
    *   @param forWhat What types of operations to select for.
    *   @return Number of Selectables inserted.
    */
   int insert(Selectable* s, Selector::selectfor_t forWhat);
   
   /**
    *   Removes the selectable from the set if it does exist.
    *   @param s Selectable to remove.
    *   @return Number of Selectables removed.
    */
   int remove(Selectable* s);
   
   /**
    *   Returns the selectable at position <code>idx</code>.
    *   @param idx The index.
    *   @return Selectable at index <code>idx</code>.
    */
   Selectable* operator[] (int idx) const;

   /**
    * Returns the selectfor_t for the selectable at position 
    * <code>idx</code>.
    *
    * @param idx The index.
    * @return For what at index <code>idx</code>.
    */
   Selector::selectfor_t getForWhat( int idx ) const;
   
   /** 
    *   Returns the size of the SelectorSet.
    */
   int getSize() const;   
};
   
int
isab::SelectorSet::insert(Selectable* s, Selector::selectfor_t forWhat)
{
   // Check if there is one already.
   for(int i=0; i < getSize(); ++i ) {
#ifdef _MSC_VER
      // Stupid VC++
      std::vector<pair<Selectable*,Selector::selectfor_t> >* vect = 
         (std::vector<pair<Selectable*,Selector::selectfor_t> >*)this;
      if ( (*vect)[ i ].first == s )
#else
      // Non-stupid g++
      if ( std::vector<pair<Selectable*, Selector::selectfor_t> >::operator[]
           ( i ).first == s )
#endif
      {
         // Found it - RETURN 0
         return 0;
      }
   }
   // Didn't find it.
   push_back(make_pair(s,forWhat));
   return 1;
}

int
isab::SelectorSet::remove(Selectable* s)
{
   // Check if we can find the s
   for( iterator it = begin();
        it != end();
        ++it ) {
      if ( it->first == s ) {
         // Found it - remove it and RETURN 1
         erase(it);
         return 1; 
      }
   }
   // Did not find it.
   return 0;
}

int
isab::SelectorSet::getSize() const
{
   return size();
}

isab::Selectable*
isab::SelectorSet::operator[](int idx) const
{
#ifdef _MSC_VER
   // Stupid VC++
   std::vector<pair<Selectable*,Selector::selectfor_t> >* vect = 
      (std::vector<pair<Selectable*,Selector::selectfor_t> >*)this;
   return (*vect)[idx].first;
#else
   // Non-stupid g++
   return std::vector<pair<Selectable*,Selector::selectfor_t> >::operator[](
      idx).first;
#endif
}

isab::Selector::selectfor_t 
isab::SelectorSet::getForWhat( int idx ) const {
#ifdef _MSC_VER
   // Stupid VC++
   std::vector<pair<Selectable*,Selector::selectfor_t> >* vect = 
      (std::vector<pair<Selectable*,Selector::selectfor_t> >*)this;
   return (*vect)[idx].second;
#else
   // Non-stupid g++
   return std::vector<pair<Selectable*,Selector::selectfor_t> >::operator[](
      idx).second;
#endif
}


/**
 * Helper class for keeping track of the current outstanding ioctl
 * requests.
 * Currently inherited from vector, can be reimplemented if
 * necessary. Insertion and removal are O(n) which shouldn't
 * e a problem. Using vector because I want to avoid iterators
 * in the Selector if the stl cannot be used.
 */
class isab::IoctlSet : private std::vector<pair<Selectable*,
            pair<TRequestStatus, TBuf8<1>*> > >
{
public:
   /**
    * Inserts the selectable into the set.
    *
    * @param s Selectable to insert.
    * @param status The ioctl request status.
    * @param flags The TUint with the flags.
    */
   void insert( Selectable* s, TRequestStatus status, TBuf8<1>* flags );
   
   /**
    * Removes the selectable from the set.
    * Cancels the ioclt request if it's pending.
    *
    * @param s Selectable to remove.
    */
   void remove( Selectable* s );
   
   /**
    * Returns the selectable at position <code>idx</code>.
    * @param idx The index.
    * @return Selectable at index <code>idx</code>.
    */
   Selectable* getSelectable( int idx );

   /**
    * Returns the request status for the selectable at position 
    * <code>idx</code>.
    *
    * @param idx The index.
    * @return For what at index <code>idx</code>.
    */
   TRequestStatus getRequestStatus( int idx );
   
   /** 
    *   Returns the size of the IocltSet.
    */
   int getSize() const;   
};


void
isab::IoctlSet::insert(Selectable* s, TRequestStatus status, 
                       TBuf8<1>* flags )
{
   push_back( make_pair( s, make_pair( status, flags ) ) );
}


void
isab::IoctlSet::remove( Selectable* s ) {
   // Check if we can find the s
   for( iterator it = begin();
        it != end();
        ++it ) {
      if ( it->first == s ) {
         if ( it->second.first == KRequestPending ) {
            // Cancel request
            it->first->getFd().CancelIoctl();
         }
         delete it->second.second;
         // Found it - remove it
         erase(it);
         return; 
      }
   }
   // Did not find it.
   return;
}


int
isab::IoctlSet::getSize() const
{
   return size();
}


isab::Selectable*
isab::IoctlSet::getSelectable(int idx) {
#ifdef _MSC_VER
   // Stupid VC++
   std::vector<pair<Selectable*,pair<TRequestStatus, TBuf8<1>*> > >* vect= 
      (std::vector<pair<Selectable*,pair<TRequestStatus, TBuf8<1>*> > >*)
         this;
   return (*vect)[idx].first;
#else
   // Non-stupid g++
   return std::vector<pair<Selectable*,pair<TRequestStatus, TBuf8<1>*> > >
      ::operator[]( idx ).first;
#endif
}

TRequestStatus
isab::IoctlSet::getRequestStatus( int idx ) {
#ifdef _MSC_VER
   // Stupid VC++
   std::vector<pair<Selectable*,pair<TRequestStatus, TBuf8<1>*> > >* vect= 
      (std::vector<pair<Selectable*, pair<TRequestStatus, TBuf8<1>* > > >*)
         this;
   return (*vect)[ idx ].second.first;
#else
   // Non-stupid g++
   return std::vector<pair<Selectable*, pair<TRequestStatus, TBuf8<1>*> > >
      ::operator[]( idx ).second.first;
#endif
}



// ---------------- Selector --------------------
   
isab::Selector::Selector() :
   m_selected(false), m_terminated(false), m_maxWaitMicros(2*1000*1000)
{
   m_selectable = new SelectorSet();
   m_ioctl = new IoctlSet();
   m_session.Connect(); // Init sort'a, parameter is number of outstanding requests
}

isab::Selector::~Selector()
{
   // If we had a wakeup socket we should set it to NULL
   // in the ModuleQueues now.
   delete m_selectable;
//   delete m_writing;
   m_session.Close();
}

int
isab::Selector::addSelectable(Selectable* target,
                              Selector::selectfor_t forWhat )
{
   int num = 0;
   num += m_selectable->insert( target, forWhat );
   return num;
}

int
isab::Selector::addSelectable(int num,
                              Selectable* target[],
                              Selector::selectfor_t forWhat)
{
   int nbr = 0;
   for(int i=0; i < num; ++i ) {
      nbr += addSelectable(target[i], forWhat);
   }
   return nbr;
}

int
isab::Selector::removeSelectable(Selectable* target,
                           Selector::selectfor_t /*forWhat*/)
{
   int num = 0;
   num += m_selectable->remove( target );  
   return num;
}

int
isab::Selector::removeSelectable(int num,
                                 Selectable* target[],
                                 selectfor_t forWhat)
{
   int nbr = 0;
   for(int i=0; i < num; ++i ) {
      nbr += removeSelectable(target[i], forWhat);      
   }
   return nbr;
}

Selectable*
isab::Selector::findAndClear( IoctlSet* ioctlSet )
{
   for ( int i = 0 ; i < ioctlSet->getSize() ; ++i ) {
      if ( ioctlSet->getRequestStatus( i ) != KRequestPending ) {
         Selectable* curSelectable = ioctlSet->getSelectable( i );
         ioctlSet->remove( curSelectable );
         return curSelectable;
      }
   }
   return NULL;
}


void 
isab::Selector::checkSelectorSet( SelectorSet* selectorSet, 
                                  IoctlSet* ioctlSet )
{
   for ( int i = 0 ; i < selectorSet->getSize() ; ++i ) {
      Selectable* curSelectable = (*selectorSet)[i];
      
      bool found = false;
      for ( int j = 0 ; j < ioctlSet->getSize() ; ++j ) {
         if ( curSelectable == ioctlSet->getSelectable( j ) ) {
            found = true;
            break;
         }
      }
      if ( ! found ) {
         // Add to ioctlSet
         TRequestStatus status;
         TBuf8<1>* flags = new TBuf8<1>(1);
         selectfor_t what = selectorSet->getForWhat( i );
         (*flags)[0] = 0;
         if ( what & WRITING ) {
            (*flags)[0] = (*flags)[0] | KSockSelectWrite;
         }
         if ( what & READING ) {
            (*flags)[0] = (*flags)[0] | KSockSelectRead;
         }
         // XXX: Perhaps KSockSelectExcept so it returns if error
         curSelectable->getFd().Ioctl( KIOctlSelect, status, 
                                       flags, KSOLSocket );
         ioctlSet->insert( curSelectable, status, flags );
      }
   }
}
   

int
isab::Selector::select(int32 micros, Selectable*& ready)
{
   ready = NULL;

   // Only used when waiting with timeout.
   int32 waitTimeLeft = micros;
      
   while ( ! m_terminated ) {
      if ( m_selected ) {
         // Someting was selected last time or this time if we have
         // visited the code below.
         // Check all in m_ioctl for completed requests
         Selectable* selectable = findAndClear( m_ioctl );

         if ( selectable != NULL ) {
            ready = selectable;
            return 1; // EARLY RETURN
         }
         // Nothing found
         m_selected = false;
      }

      // Nothing was found in the selectable sets.

      // Make sure all selectable in m_ioctl is in m_selectable
//      checkIoctlSet( m_ioctl, m_selectable );

      // Add all new selectable in m_selectable to m_ioctl.
      checkSelectorSet( m_selectable, m_ioctl );

      // Set timer using m_maxWaitMicros
      RTimer timer;
      TRequestStatus timerStatus;
      int32 waitTime = MIN( m_maxWaitMicros, waitTimeLeft );
      if ( micros >= 0 ) {
         if ( timer.CreateLocal() != KErrNone ) {
            // Error
            return -1;
         }
         timer.After( timerStatus, waitTime );
      }

      // Wait for any to return
      User::WaitForAnyRequest();


      // Check if timer
      if ( micros >= 0 ) {
         // Decrease the waittime. Not exact
         waitTimeLeft -= waitTime;
         if ( timerStatus != KRequestPending ) {
            // Timer!
            if ( waitTimeLeft <= 0) {
               timer.Close();
               return 0; // RETURN
            }
         } else {
            timer.Cancel();
         }
         timer.Close();
      }

      // Check m_ioctl
      // Check m_ioctl in next loop
      m_selected = true;
   } // while ! terminated
   // Terminated
   return -2;
}



