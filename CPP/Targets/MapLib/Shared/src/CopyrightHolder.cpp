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

#include "CopyrightHolder.h"

bool 
CopyrightNotice::load( BitBuffer& buf )
{
   // Size of data.
   int startPos = buf.getCurrentOffset();
   int nbrBytes = buf.readNextBALong();

   // Copyright ID.
   m_copyrightID = buf.readNextBALong();
   
   // Bounding box.
   int32 maxLat = buf.readNextBALong();
   int32 minLon = buf.readNextBALong();
   int32 minLat = buf.readNextBALong();
   int32 maxLon = buf.readNextBALong();

   m_box.reset();
   m_box.update( maxLat, minLon );
   m_box.update( minLat, maxLon );

   // Skip bytes at the end.
   int bytesLeft = nbrBytes - ( buf.getCurrentOffset() - startPos );
   buf.readPastBytes( bytesLeft );
   return true;
}

bool 
CopyrightNotice::save( BitBuffer& buf ) const 
{
   // Size of data. Fill in later. 
   int startPos = buf.getCurrentOffset();
   buf.writeNextBALong( 0 );

   // Copyright ID.
   buf.writeNextBALong( m_copyrightID );
   
   // Bounding box.
   buf.writeNextBALong( m_box.getMaxLat() );
   buf.writeNextBALong( m_box.getMinLon() );
   buf.writeNextBALong( m_box.getMinLat() );
   buf.writeNextBALong( m_box.getMaxLon() );
  
   // This should be last. Add new stuff above here.
   
   // Write the size of data.
   
   // Store end pos.
   int endPos = buf.getCurrentOffset();
   
   // Rewind and go to start pos.
   buf.reset();
   buf.readPastBytes( startPos );
   // Write size of data.
   buf.writeNextBALong( endPos - startPos );

   // Set the old offset.
   buf.reset();
   buf.readPastBytes( endPos );

   return true;
}

int
CopyrightNotice::getCopyrightID() const
{
   return m_copyrightID;
}

bool
CopyrightNotice::isOverlappingBox() const
{
   return getCopyrightID() == MAX_INT32;
}

const MC2BoundingBox&
CopyrightNotice::getBox() const
{
   return m_box;
}

void 
CopyrightNotice::dump() const 
{
   mc2dbg << "[CN]: m_copyrightID " << m_copyrightID << endl;

   mc2dbg << "[CN]: m_box " << m_box << endl;
}

void 
CopyrightHolder::dump() const 
{
  
   mc2dbg << "[CH]: Nbr boxes " << m_boxes.size() << endl;
   
   {for ( uint32 i = 0; i < m_boxes.size(); ++i ) {
      m_boxes[ i ].dump();

   }}
   
   MC2_ASSERT( m_boxes.size() == m_boxesByParent.size() );
   {for ( uint32 i = 0; i < m_boxes.size(); ++i ) {
      mc2dbg << "[CH]: m_boxesByParent( " << m_boxesByParent[ i ].first << ", "
             << m_boxesByParent[ i ].second << " ) " << endl;
   }}
                                                     
                                                     
   mc2dbg << "[CH]: Nbr strings " << m_copyrightStrings.size() << endl;
   
   {for ( uint32 i = 0; i < m_copyrightStrings.size(); ++i ) {
      mc2dbg << "[CH]: " << m_copyrightStrings[ i ] << endl;
   }}

   mc2dbg << "[CH]: m_minCovPercent " << m_minCovPercent << endl;

   mc2dbg << "[CH]: m_copyrightHead " << m_copyrightHead << endl;
}

bool
CopyrightHolder::load( BitBuffer& buf ) 
{
   // Size of data.
   int startPos = buf.getCurrentOffset();
   int nbrBytes = buf.readNextBALong();
   
   // Number of boxes.
   uint32 nbrBoxes = buf.readNextBALong();
   
   m_boxes.resize( nbrBoxes );
   {for ( uint32 i = 0; i < nbrBoxes; ++i ) {
      bool res = m_boxes[ i ].load( buf );

      // If loading went bad, clean up and return false.
      if ( ! res ) {
         m_boxes.clear();
         m_boxesByParent.clear();
         m_copyrightStrings.clear();
         return false;
      }
   }}
   
   // Load the boxes by parent table.
   {for ( uint32 i = 0; i < m_boxes.size(); ++i ) {
      // Parent.
      int parent = buf.readNextBALong();
      // Box nbr.
      int boxNbr = buf.readNextBALong();
     
      m_boxesByParent.push_back(
            std::make_pair( parent, boxNbr ) );
   }}

   // Number of copyright ids and  strings.
   uint32 nbrCopyrightStrings = buf.readNextBALong();
   m_copyrightStrings.reserve( nbrCopyrightStrings );
   
   {for ( uint32 i = 0; i < nbrCopyrightStrings; ++i ) {
      m_copyrightStrings.push_back( buf.readNextString() );
   }}

   // The minimum coverage in percent for a copyright ID to be included.
   m_minCovPercent = buf.readNextBALong();
   
   // The copyright head, i.e. "(c) Wayfinder"
   m_copyrightHead = buf.readNextString();

   // Skip bytes at the end.
   int bytesLeft = nbrBytes - ( buf.getCurrentOffset() - startPos );
   buf.readPastBytes( bytesLeft );

   dump();

   return true;
}

bool
CopyrightHolder::save( BitBuffer& buf ) const
{
   // Size of data. Fill in later. 
   int startPos = buf.getCurrentOffset();
   buf.writeNextBALong( 0 );

   // Number of boxes.
   buf.writeNextBALong( m_boxes.size() );
   
   {for ( uint32 i = 0; i < m_boxes.size(); ++i ) {
      bool res = m_boxes[ i ].save( buf );

      // If saving went bad, return false.
      if ( ! res ) {
         return false;
      }
   }}

   MC2_ASSERT( m_boxes.size() == m_boxesByParent.size() );
   // Save the boxes by parent table.
   {for ( uint32 i = 0; i < m_boxes.size(); ++i ) {
      // Parent.
      buf.writeNextBALong( m_boxesByParent[ i ].first );
      // Box nbr.
      buf.writeNextBALong( m_boxesByParent[ i ].second );
   }}

   // Number of copyright ids and  strings.
   buf.writeNextBALong( m_copyrightStrings.size() );
   
   {for ( uint32 i = 0; i < m_copyrightStrings.size(); ++i ) {
      buf.writeNextString( m_copyrightStrings[ i ].c_str() );
   }}

   // The minimum coverage in percent for a copyright ID to be included.
   buf.writeNextBALong( m_minCovPercent );
   
   // The copyright head, i.e. "(c) Wayfinder"
   buf.writeNextString( m_copyrightHead );

   // This should be last. Add new stuff above here.

   // Write the size of data.
   
   // Store end pos.
   int endPos = buf.getCurrentOffset();
   
   // Rewind and go to start pos.
   buf.reset();
   buf.readPastBytes( startPos );
   // Write size of data.
   buf.writeNextBALong( endPos - startPos );

   // Set the old offset.
   buf.reset();
   buf.readPastBytes( endPos );
   
   dump();

   return true;
}

void
CopyrightHolder::initWithHardcodedData()
{
   // AND notice - 0.
   CopyrightNotice notice;
   notice.m_copyrightID = 0;
   notice.m_box.setMaxLat( MAX_INT32 / 2 );
   notice.m_box.setMinLat( MIN_INT32 / 2);
   notice.m_box.setMaxLon( MAX_INT32 );
   notice.m_box.setMinLon( MIN_INT32 );
   notice.m_box.setMinLon(  0 );
   m_boxes.push_back( notice );
   m_copyrightStrings.push_back( "AND" );
   
   // TeleAtlas notice - 1.
   notice.m_copyrightID = 1;
   notice.m_box = MC2BoundingBox( MC2Coordinate( 839440783,-179076280 ),
                                  MC2Coordinate( 427215211,399551259 ) );
   m_boxes.push_back( notice );
   m_copyrightStrings.push_back( "TeleAtlas" );
   
   // Monolit notice - 2.
   notice.m_copyrightID = 2;
   notice.m_box = MC2BoundingBox( MC2Coordinate( 559253233,157064563 ),
                                  MC2Coordinate( 538960363,195838573 ) );
   m_boxes.push_back( notice );
   m_copyrightStrings.push_back( "Monolit" );

   // TopMap notice - 3.
   notice.m_copyrightID = 3;
   notice.m_box = MC2BoundingBox( MC2Coordinate( 572967465,197628143 ),
                                  MC2Coordinate( 548148698,276070949 ) );
   m_boxes.push_back( notice );
   m_copyrightStrings.push_back( "TopMap" );

   m_boxesByParent.push_back( std::make_pair( 0, 1 ) );
   m_boxesByParent.push_back( std::make_pair( 1, 2 ) );
   m_boxesByParent.push_back( std::make_pair( 1, 3 ) );
   
   m_boxesByParent.push_back( std::make_pair( MAX_INT32, 0 ) );


   m_copyrightHead = "© Wayfinder";
   m_minCovPercent = 10;
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


