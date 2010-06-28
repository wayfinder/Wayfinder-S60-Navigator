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

#include "CopyrightHandler.h"

#include <algorithm>
#include "TileMapUtil.h"
#include <stdio.h>

CopyrightHandler::CopyrightHandler() : m_holder( NULL )
{
   
}

void 
CopyrightHandler::setCopyrightHolder( const CopyrightHolder* holder ) 
{
   m_holder = holder;
   if ( m_holder != NULL && m_holder->m_boxes.empty() ) {
//      initWithHardcodedData();
   }
}

void 
CopyrightHandler::initWithHardcodedData()
{
   if ( m_holder == NULL ) {
      return;
   }
   (const_cast<CopyrightHolder*> (m_holder))->initWithHardcodedData();
}

struct ltPair_first
{
   bool operator() (const CopyrightHolder::pair_t& p1, int p2 ) const 
   {
      return p1.first < p2;
   }
   bool operator() (int p1, const CopyrightHolder::pair_t& p2 ) const 
   {
      return p1 < p2.first;
   }
};

int64
CopyrightHandler::recursiveCheckCoverage( const MC2BoundingBox& screenBox, 
                                          int id, 
                  std::vector<std::pair<int64,int> >& covByCopyrightId ) 
{
   const CopyrightNotice& curNotice = m_holder->m_boxes[ id ];
   
   MC2BoundingBox intersection;
   if ( curNotice.getBox().getInterSection( screenBox, intersection ) ) {

      // Calculate the coverage as area.
      int64 coverage = (int64) intersection.getLonDiff() * 
                       (int64) intersection.getHeight();
      mc2dbg << "coverage (copyright): " << coverage << endl;
      
      int64 covForChildren = 0;
      range_t range = std::equal_range( m_holder->m_boxesByParent.begin(),
                                        m_holder->m_boxesByParent.end(), 
                                        id, 
                                        ltPair_first() );
      for ( mapIt_t it = range.first; it != range.second; ++it ) {
         int childID = it->second;
         covForChildren += recursiveCheckCoverage( intersection, 
                                                   childID, 
                                                   covByCopyrightId );
         if ( covForChildren == coverage ) {
            mc2dbg << "Break (copyright)" << endl;
            break;
         }
         MC2_ASSERT( covForChildren <= coverage );
      }
      
      if ( curNotice.isOverlappingBox() ) {
         // Box is only used as bounding box for children and
         // does not contain any copyright info of it's own.
         // Return the copyright coverage for the children.
         return covForChildren;
      }
      
      // Calculate the current coverage, 
      // by subtracting any coverage already covered by a child.
      int64 curCov = coverage - covForChildren;
      
      mc2dbg << "curCov (copyright): " << curCov << endl;
      mc2dbg << "covForChildren (copyright): " << covForChildren << endl;
      
      // Update coverage.
      covByCopyrightId[ curNotice.getCopyrightID() ].first += curCov;

      return coverage;
   }
   // else, no overlapping
   return 0;
}

struct gtFirst {

   bool operator() (const std::pair<int64,int>& p1, 
                    const std::pair<int64,int>& p2 ) const
   {
      return p1.first > p2.first;
   }
};

void
CopyrightHandler::getCopyrightString( char* copyrightString,
                                      int maxLength )
{
   if ( m_curCopyrightString.empty() ) {
      m_curCopyrightString = m_staticCopyrightString;
   }

   int len = MIN( maxLength, (int) m_curCopyrightString.length() );
   strncpy( copyrightString, m_curCopyrightString.c_str(), len );
   // Terminator.
   copyrightString[ len ] = '\0';
}

void
CopyrightHandler::getCopyrightString( 
            const std::vector<MC2BoundingBox>& screenBoxes,
                                      char* copyrightString,
                                      int maxLength )
{
   TileMapClock clock;

   // Let's make sure we always have room for the terminating zero.
   maxLength -= 1;

   copyrightString[ 0 ] = '\0';
   
   if ( m_holder == NULL ) {
      m_curCopyrightString = m_staticCopyrightString;
      int len = MIN( maxLength, (int) m_curCopyrightString.length() );
      strncpy( copyrightString, m_curCopyrightString.c_str(), len );
      // Terminator.
      copyrightString[ len ] = '\0';
      return;
   }
   
   // Get the root boxes. (Parent is MAX_INT32)
  
   range_t range = std::equal_range( m_holder->m_boxesByParent.begin(),
                                     m_holder->m_boxesByParent.end(), 
                                     MAX_INT32, 
                                     ltPair_first() );

   // First is coverage area, second is copyright id.
   std::vector<std::pair<int64,int> > covByCopyrightId;
   covByCopyrightId.resize( m_holder->m_copyrightStrings.size() );
   for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      covByCopyrightId[ i ] = std::make_pair( 0, i );
   }
   
   {for ( uint32 i = 0; i < screenBoxes.size(); ++i ) {
      for ( mapIt_t it = range.first; it != range.second; ++it ) {
         int childID = it->second;
         recursiveCheckCoverage( screenBoxes[ i ], childID, covByCopyrightId );
      }
   }} 
 
   // Will contain the coverage area in first and copyright id in second.
   // Will be sorted in coverage order.
   std::sort( covByCopyrightId.begin(), covByCopyrightId.end(), gtFirst() );
   
   
   // The result should be in covByCopyrightId.
   float32 totalArea = 0;
   {for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      totalArea += (float32) covByCopyrightId[ i ].first;
   }}
  
   if ( totalArea < 0.000001 ) {
      // No copyright area found. Return to avoid divide by zero.
      return;
   }
   mc2dbg << "------------------- Copyright" << endl;
   char* curStr = copyrightString;
   int nbrAdded = 0;

   
   // First add the copyright head.
   // Moved to separate namespace in order to support platforms with no
   // sprintf support
   int nbrChars = snprintf( curStr, maxLength, "%s", 
                            m_holder->m_copyrightHead.c_str() );
   curStr += nbrChars;
   maxLength -= nbrChars;

   {for ( uint32 i = 0; i < covByCopyrightId.size(); ++i ) {
      mc2dbg << "Copyright Id: " << i << ", " 
             << m_holder->m_copyrightStrings[ covByCopyrightId[ i ].second ]
             << ", area: " << covByCopyrightId[ i ].first 
             << ", percent: " << (float32)covByCopyrightId[ i ].first / 
                                 totalArea * 100 
             << ", total area: " << totalArea << endl;

      int percent = int( (float32)covByCopyrightId[ i ].first / 
                         totalArea * 100 );
      if ( percent >= m_holder->m_minCovPercent ) {
         nbrChars = snprintf( curStr, maxLength, "%s%s", 
                              ",",
                              m_holder->m_copyrightStrings[ 
                                 covByCopyrightId[ i ].second ].c_str() );
         curStr[ nbrChars ] = '\0';
         curStr += nbrChars;
         maxLength -= nbrChars;
         ++nbrAdded;
      }
   }}

   // Store this copyright string.
   m_curCopyrightString = MC2SimpleString( copyrightString );
}

void
CopyrightHandler::setStaticCopyrightString( const MC2SimpleString& crString )
{
   m_staticCopyrightString = crString;
   // Need to NULL out the const pointer to the copyright holder,
   // since it may be deleted.
   // No risk of memory leak, since someone else owns the 
   // copyright holder.
   m_holder = NULL;
}

