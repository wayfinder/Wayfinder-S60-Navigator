/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_VIEW_STACK_H
#define WF_VIEW_STACK_H

#include "arch.h"
#include <list>
#include <set>
#include <map>
#include <vector>

#ifdef __SYMBIAN32__
# define VIEWID TUid
# define GROUPID TUid
// Global less than operator for TUids
inline bool operator < ( const TUid& a, const TUid& b ) {
   return a.iUid < b.iUid;
}
#else
# define VIEWID int
# define GROUPID int
#endif

/**
 * Class that is a view stack.
 *
 * TODO: Add types for main and routeplanner so it is less hardcoded 
 *       to routeplanner but a general thinge that removes all back to
 *       prevoius occurrence. 
 *       Perhaps a clearAll and clearToPrevoius set that views can be added
 *       to.
 */
class WFViewStack  {
   protected:
      typedef VIEWID ViewID;
      typedef GROUPID GroupID;

   private:
      /// The invalid view id
      ViewID m_invVid;
       
      /// The invalid group id
      GroupID m_invUid;

      /// The main view id
      ViewID m_mainViewID;

      /// The route planer view id
      ViewID m_routePlanerViewID;

      /**
       * Class that represents a view, possibly in a group, on the stack.
       */
      class WFViewStackItem {
         public:
            WFViewStackItem( ViewID view, GroupID group )
               : m_view( view ), m_group( group )
               {}

            WFViewStackItem()
               {}

            const ViewID& v() const { return m_view; }
            const GroupID& g() const { return m_group; }
            ViewID& v() { return m_view; }
            GroupID& g() { return m_group; }

            bool operator == ( const WFViewStackItem& o ) const {
               return m_view == o.m_view && m_group == o.m_group;
            }

            bool operator < ( const WFViewStackItem& o ) const {
               if ( m_group != o.m_group ) {
                  return m_group < o.m_group;
               } else {
                  return m_view < o.m_view;
               }
            }

         private:
            ViewID m_view;
            GroupID m_group;
      };

      typedef std::list< WFViewStackItem > wfstack;

      /// The stack of views.
      wfstack m_stack;

      typedef std::vector< WFViewStackItem > wfvector;
      /**
       * The set of groups defined.
       */
      wfvector m_groups;

      typedef std::map< ViewID, GroupID > wfmap;
      /**
       * Map from view to group.
       */
      wfmap m_viewToGroup;

      /**
       * Find a view iterator.
       */
      wfstack::iterator find( WFViewStackItem w );

      /**
       * Reverse find a view iterator.
       */
      wfstack::iterator rfind( WFViewStackItem w );

      /**
       * Erase, first occurrence of, view from stack.
       */
      void erase( WFViewStackItem w );

      /**
       * Erase all after, and including, view.
       */
      void eraseAllAfter( WFViewStackItem w );

      /**
       * Goto a view.
       */
      void internalGoto( WFViewStackItem b, bool isPop );

      /**
       * Helper function.
       */
      wfvector::const_iterator lastWithGroup( 
         wfvector::const_iterator it, GroupID g, 
         wfvector::const_iterator end ) const;

      /**
       * Helper function.
       */
      wfvector::const_iterator firstWithGroup( 
         wfvector::const_iterator it, GroupID g,
         wfvector::const_iterator begin ) const;

   protected:
      /**
       * Removes the last.
       */
      void pop_back();

   public:
      /**
       * Constructor.
       */
      WFViewStack( ViewID invalidViewID, GroupID invalidGroupID,
                   ViewID mainViewID, ViewID routePlanerViewID );

      /**
       * Destructor.
       */
      virtual ~WFViewStack() {}

      /**
       * Add a view in a group.
       */
      void push( ViewID v, GroupID g );

      /**
       * Add a view.
       */
      void push( ViewID v ) { 
         push( v, m_invUid ); 
      }

      /**
       * Set up the navigation view stack. E.g. clear all stack and
       * then add main menu and then route planner and then the 
       * preferred navigation view.
       */
      void setupNavigationViewStack( ViewID v, GroupID g );
   
      void setupNavigationViewStack( ViewID v ) {
         setupNavigationViewStack( v, m_invUid );  
      }

      void setupNavigationViewStack( ) {
         setupNavigationViewStack( m_invVid, m_invUid );  
      }
   
      /**
       * Pop the latest view.
       */
      virtual void pop();

      /**
       * Add a view as being in a group. 
       * ViewStack will recognize view as in group even if GroupID is
       * not set in call to push.
       */
      void addGroupView( ViewID v, GroupID g );

      /**
       * Remove a view from group.
       */
      void removeGroupView( ViewID v, GroupID g );

      /**
       * What is the current view?
       */
      ViewID current() const;

      /**
       * What is the current group?
       */
      GroupID currentGroup() const;

      /**
       * What is the view before current view?
       */
      ViewID peek() const;

      /**
       * What is the group of the view before current view?
       */
      GroupID peekGroup() const;

      /**
       * The view before v in group, invalidViewID if no such view.
       */
      ViewID prevViewInGroup( ViewID v, GroupID g ) const;

      /**
       * The view after v in group, invalidViewID if no such view.
       */
      ViewID sucViewInGroup( ViewID v, GroupID g ) const;

      /**
       * The view before current view in group, invalidViewID if no such 
       * view.
       */
      ViewID prevViewInGroup() const;

      /**
       * The view after current view in group, invalidViewID if no such
       * view.
       */
      ViewID sucViewInGroup() const;

      /**
       * Goto a view, called by pop/push by default.
       */
      virtual void gotoView( ViewID v, bool isPop ) = 0;

      /**
       * Goto a grouped view, called by pop/push by default.
       */
      virtual void gotoGroupView( ViewID v, GroupID g, bool isPop ) = 0;

      /**
       * Error pop called on empty stack. 
       * Default is to gotoView m_mainViewID.
       */
      virtual void errorEmptyPop();

      /**
       * Prints contents, in debug log.
       */
      virtual void trace() const;

      /**
       * The invalid view id.
       */
      ViewID invVid();
       
      /**
       * The invalid group id.
       */
      GroupID invUid();
};


#endif // WF_VIEW_STACK_H

