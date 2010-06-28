/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "WFViewStack.h"

#include <algorithm>

WFViewStack::WFViewStack( ViewID invalidViewID, GroupID invalidGroupID,
                          ViewID mainViewID, ViewID routePlanerViewID )
      : m_invVid( invalidViewID ), m_invUid( invalidGroupID ),
        m_mainViewID( mainViewID ), 
        m_routePlanerViewID( routePlanerViewID ) 
{
   m_stack.push_back( WFViewStackItem( m_mainViewID, m_invUid ) );
}

WFViewStack::wfstack::iterator 
WFViewStack::find( WFViewStackItem w ) {
   for ( wfstack::iterator it = m_stack.begin() ; 
         it != m_stack.end() ; ++it )
   {
      if ( (*it) == w ) {
         return it;
      }
   }
   return m_stack.end();
}

WFViewStack::wfstack::iterator
WFViewStack::rfind( WFViewStackItem w ) {
   if ( !m_stack.empty() ) {
      if ( (*m_stack.begin()) == w ) {
         return m_stack.begin();
      }
      for ( wfstack::iterator it = --m_stack.end() ; 
            it != m_stack.begin() ; --it )
      {
         if ( (*it) == w ) {
            return it;
         }
      }
   }

   return m_stack.end();
}

void
WFViewStack::erase( WFViewStackItem w ) {
   wfstack::iterator findIt = find( w );
   if ( findIt != m_stack.end() ) {
      m_stack.erase( findIt );
   }
}

 
void
WFViewStack::eraseAllAfter( WFViewStackItem w ) {
   wfstack::iterator findIt = rfind( w );
   if ( findIt != m_stack.end() ) {
      m_stack.erase( findIt, m_stack.end() );
   }
}

void
WFViewStack::internalGoto( WFViewStackItem b, bool isPop ) {
   if ( b.g() != m_invUid ) {
      gotoGroupView( b.v(), b.g(), isPop );
   } else {
      gotoView( b.v(), isPop );
   }
}

void
WFViewStack::pop_back() {
   if ( !m_stack.empty() ) {
      m_stack.pop_back();
   }
}
 
void
WFViewStack::push( ViewID v, GroupID g ) {
   // Check if g is invalid and v in a group
   if ( g == m_invUid ) {
      wfmap::const_iterator findIt = m_viewToGroup.find( v );
      if ( findIt != m_viewToGroup.end() ) {
         g = (*findIt).second;
      }
   }

   WFViewStackItem w( v, g );
   if ( w.v() == m_mainViewID ) {
      // Clear all when back to Main
      m_stack.clear(); 
   } else if ( w.v() == m_routePlanerViewID ) {
      // Special remove all back to prevoius occurrence.
      eraseAllAfter( w );
   } else if ( g != m_invUid && g == m_stack.back().g() ) {
      // Remove last in same group and add latest groupViewID.
      m_stack.pop_back();
      // Erase all (should not be any) occurences of group in stack?
   } else {
      // Remove earlier instance of view
      erase( w );
   }

   m_stack.push_back( w );
   internalGoto( m_stack.back(), false/*isPop*/ );
}

void WFViewStack::setupNavigationViewStack( ViewID v, GroupID g ) {
   WFViewStackItem w( v, g );
   m_stack.clear();
   m_stack.push_back( WFViewStackItem( m_mainViewID, m_invUid ) );
   m_stack.push_back( WFViewStackItem( m_routePlanerViewID, m_invUid ) );
   if ( v != m_invVid ) {
      push( v, g );
   } 
}

void
WFViewStack::pop() {
   if ( !m_stack.empty() ) {
      m_stack.pop_back();
      if ( !m_stack.empty() ) {
         WFViewStackItem b = m_stack.back();
         internalGoto( b, true/*isPop*/ );
      } else {
         errorEmptyPop();
      }
   } else {
      errorEmptyPop();
   }
}

void
WFViewStack::addGroupView( ViewID v, GroupID g ) {
   if ( g != m_invUid ) {
      m_groups.push_back( WFViewStackItem( v, g ) ); //insert( WFViewStackItem( v, g ) );
      m_viewToGroup.insert( std::make_pair( v, g ) );
   } else {
      // Not allowed to add invalid group
   }
}

void
WFViewStack::removeGroupView( ViewID v, GroupID g ) {
   m_groups.erase( std::find( m_groups.begin(), m_groups.end(), WFViewStackItem( v, g ) ) ); //WFViewStackItem( v, g ) );
   m_viewToGroup.erase( v );
}

WFViewStack::ViewID
WFViewStack::current() const {
   if ( !m_stack.empty() ) {
      return m_stack.back().v();
   } else {
      return m_mainViewID;
   }
}

WFViewStack::GroupID
WFViewStack::currentGroup() const {
   if ( !m_stack.empty() ) {
      return m_stack.back().g();
   } else {
      return m_invUid;
   }
}

WFViewStack::ViewID
WFViewStack::peek() const {
   if ( !m_stack.empty() && m_stack.size() >= 2) {
      // The one before current
      wfstack::const_reverse_iterator it = ++m_stack.rbegin();
      return (*it).v();
   } else {
      return m_mainViewID;
   }
}

WFViewStack::GroupID
WFViewStack::peekGroup() const {
   if ( !m_stack.empty() && m_stack.size() >= 2) {
      // The one before current
      wfstack::const_reverse_iterator it = ++m_stack.rbegin();
      return (*it).g();
   } else {
      return m_invUid;
   }
}

WFViewStack::wfvector::const_iterator 
WFViewStack::lastWithGroup( 
   WFViewStack::wfvector::const_iterator it, GroupID g,
   wfvector::const_iterator end ) const
{
   while ( it != end && (*it).g() == g ) { 
      ++it; 
   }
   it--;
   return it; // Last with group
}

WFViewStack::wfvector::const_iterator 
WFViewStack::firstWithGroup( 
   WFViewStack::wfvector::const_iterator it, GroupID g,
   wfvector::const_iterator begin ) const
{
   while ( it != begin && (*it).g() == g ) { 
      --it; 
   }
   if ( it != begin ) {
      it++;
   }
   return it; // First with group
}

WFViewStack::ViewID
WFViewStack::prevViewInGroup( ViewID v, GroupID g ) const {
   WFViewStackItem w( v, g );
   wfvector::const_iterator findIt = std::find(m_groups.begin(), m_groups.end(), w); //m_groups.find( w );

   if ( findIt != m_groups.end() ) {
      if ( findIt != m_groups.begin() ) {
         --findIt;
         if ( (*findIt).g() == g ) {
            return (*findIt).v();
         } else {
            return (*lastWithGroup( ++findIt, g, m_groups.end() )).v();
         }
      } else {
         return (*lastWithGroup( findIt, g, m_groups.end() )).v();
      }
   } else {
      return m_invVid;
   }
}

WFViewStack::ViewID
WFViewStack::prevViewInGroup() const {
   return prevViewInGroup( current(), currentGroup() );
}

WFViewStack::ViewID
WFViewStack::sucViewInGroup( ViewID v, GroupID g ) const {
   WFViewStackItem w( v, g );
   wfvector::const_iterator findIt = std::find(m_groups.begin(), m_groups.end(), w); //m_groups.find( w );

   if ( findIt != m_groups.end() ) {
      if ( (++findIt) != m_groups.end() ) {
         if ( (*findIt).g() == g ) {
            return (*findIt).v();
         } else {
            return (*firstWithGroup( --findIt, g, m_groups.begin() )).v();
         }
      } else {
         return (*firstWithGroup( --findIt, g, m_groups.begin() )).v();
      }
   } else {
      return m_invVid;
   }
}

WFViewStack::ViewID
WFViewStack::sucViewInGroup() const {
   return sucViewInGroup( current(), currentGroup() );
}

void
WFViewStack::errorEmptyPop() {
   gotoView( m_mainViewID, true/*isPop*/ );
}

void
WFViewStack::trace() const {
   char* t = new char[ 40*m_stack.size() +1 ];
   t[ 0 ] = '\0';
   char tmp[40];
   for ( wfstack::const_iterator it = m_stack.begin() ; 
         it != m_stack.end() ; ++it )
   {
      sprintf( tmp, " %d", 
#ifdef __SYMBIAN32__
               int((*it).v().iUid)
#else
               (*it).v()
#endif
               );
      if ( (*it).g() != m_invUid ) {
         sprintf( tmp + strlen( tmp ), "(%d)",
#ifdef __SYMBIAN32__
                  int((*it).g().iUid)
#else
                  (*it).g()
#endif
                  );
      }
      strcat( t, tmp );
   }
   //TRACE_FUNC2( "", t );
   delete [] t;
}

WFViewStack::ViewID
WFViewStack::invVid() {
   return m_invVid;
}

WFViewStack::GroupID
WFViewStack::invUid() {
   return m_invUid;
}

