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

#include "SFDSearcher.h"

#include "SFDIndexor.h"
#include "SFDFileIndexor.h"
#include "SFDHeader.h"
#include "SFDSearcherListener.h"

SFDSearcher::SFDSearcher( const SFDHeader& header,
                          FileHandler& fileHandler,
                          SFDSearcherListener& listener )
                          
      : m_file( fileHandler ),
        m_header( header )
{
   m_state = NOT_INITIALIZED;
   m_fileIndexor = NULL;
   m_first       = NULL;
   m_last        = NULL;
   m_listener    = &listener;
   init();
}

void
SFDSearcher::init()
{
   m_fileIndexor = new SFDFileIndexor( &m_file,
                                       this,
                                       m_header );
   m_first = new SFDIndexor;
   m_last  = new SFDIndexor;
   m_left  = new SFDIndexor;
   m_right = new SFDIndexor;
}

SFDSearcher::~SFDSearcher()
{
   delete m_fileIndexor;
   delete m_first;
   delete m_last;
   delete m_left;
   delete m_right;
}

void
SFDSearcher::indexorDone( SFDFileIndexor* which_one, int status )
{
   if ( status < 0 ) {
      // Permanent error
      m_state = PERMANENT_ERROR;
      searcherDone();
      return;
   }
   if ( m_state == READING_FIRST ) {
      m_state = READING_LAST;
      // Swap the data into our resident first entry.
      m_first->swap( *m_fileIndexor );
      m_fileIndexor->setStrNbr( m_header.getNbrStrings() - 1 );
      m_fileIndexor->start();
   } else if ( m_state == READING_LAST ) {
      // Swap the data into our resident last entry.
      m_last->swap( *m_fileIndexor );
      // Ready to process.
      m_state = SEARCHING;
      initSearchStep();
   } else if ( m_state == SEARCHING ) {
      handleSearchStep();
   }
}

void
SFDSearcher::initSearchStep()
{
   // Check if inside range at all...
   if ( m_toSearchfor < *m_first || *m_last < m_toSearchfor ) {
      // Not found
      searcherDone();
      return;
   }
   
   // Set the boundaries. Must copy here.
   *m_left  = *m_first;
   *m_right = *m_last;
   // Set m_fileIndexor to middle
   // Make function
   setMiddleAndStart();
}

void
SFDSearcher::searcherDone( uint32 strNbr )
{
   mc2dbg << "[SFDSearcher]: " << m_toSearchfor << "," << strNbr << endl;
   m_state = READY;
   m_listener->searcherDone( this, strNbr );
}

void
SFDSearcher::setMiddleAndStart()
{

   // If right and left are one step apart, we are almost done.
   if ( m_left->getStrNbr() + 1 == m_right->getStrNbr() ) {
      // Check the two strings
      if ( m_toSearchfor == m_right->getStr() ) {
         searcherDone( m_right->getStrNbr() );
      } else if ( m_toSearchfor == m_left->getStr() ) {
         searcherDone( m_left->getStrNbr() );
      } else {
         // Not equal - not found
         searcherDone();
      }
      return;
   } else {
      // Set new middle.
      m_fileIndexor->setStrNbr(
         ( m_left->getStrNbr() + m_right->getStrNbr() ) / 2 );
      // Read the next one
      m_fileIndexor->start();
   }
}

void
SFDSearcher::handleSearchStep()
{
   MC2_ASSERT( m_left->getStrNbr() <= m_right->getStrNbr() );
   
   if ( m_toSearchfor < *m_fileIndexor ) {
      // Swap the non-file part of the indexor.
      m_right->swap( *m_fileIndexor );
      setMiddleAndStart();
   } else if ( *m_fileIndexor < m_toSearchfor ) {
      // Swap the non-file part of the indexor.
      m_left->swap( *m_fileIndexor );      
      setMiddleAndStart();
   } else {
      // Found and done
      searcherDone( m_fileIndexor->getStrNbr() );
   }
}

void
SFDSearcher::searchFor( const MC2SimpleString& str )
{
   // Check so that we are in the correct state.   
   MC2_ASSERT( m_state == NOT_INITIALIZED ||
               m_state == READY ||
               m_state == PERMANENT_ERROR );

   if ( m_state == PERMANENT_ERROR ) {
      searcherDone();
      m_state = PERMANENT_ERROR;
      return;
   }
   
   // Save the search string.
   m_toSearchfor = str;
   
   // Check if the first string is read. If not we need to init.
   if ( m_state == NOT_INITIALIZED ) {
      // Start reading the first entry.
      m_state = READING_FIRST;
      m_fileIndexor->setStrNbr( 0 );
      m_fileIndexor->start();      
   } else {
      // State == READY
      m_state = SEARCHING;
      initSearchStep();
   }
   
}

