/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SFDSEARCHER_HEDGE_FILE_H
#define SFDSEARCHER_HEDGE_FILE_H

#include "config.h"

class FileHandler;
class SFDHeader;
class SFDIndexor;
class SFDFileIndexor;
class SFDSearcherListener;
class MC2SimpleString;

#include "SFDFileIndexorListener.h"
#include "MC2SimpleString.h"

class SFDSearcher : public SFDFileIndexorListener {
public:
   /**
    *   Creates an SFDSearcher using the supplied header
    *   and the first and last indexors.
    *   @param header      The header of the file.
    *   @param fileHandler The file handler for the file.
    *   @param listener    Listener to call when done.
    */
   SFDSearcher( const SFDHeader& header,
                FileHandler& fileHandler,
                SFDSearcherListener& listener );

   /**
    *   Initializes the SFDSearcher.
    */
   void init();
   
   /// Destructor
   virtual ~SFDSearcher();

   void searchFor( const MC2SimpleString& str );

   /// Implements SFDFileIndexorListener
   void indexorDone( SFDFileIndexor* which_one, int status );   
   
private:

   /**
    *   Calls the listener and tells it that the searching is done.
    *   @param strNbr The string number of the string found. MAX_UINT32 ==
    *                 not found.
    */
   void searcherDone( uint32 strNbr = MAX_UINT32 );
   
   /// Initializes the searching
   void initSearchStep();
   
   /// Handles one step in the search process.
   void handleSearchStep();

   /// Sets the middle and starts loading the string.
   void setMiddleAndStart();
   
   enum state_t {
      /// The searcher is not initialized. First and last must be read
      NOT_INITIALIZED = 0,
      /// Reading the first entry of the file.
      READING_FIRST   = 1,
      /// Reading the last entry of the file.
      READING_LAST    = 2,
      /// Ready to search.
      READY           = 3,
      /// Searching
      SEARCHING       = 4,
      /// Permanent error has occured
      PERMANENT_ERROR = 1000,
   } m_state;

   /// File handler (file) to use
   FileHandler& m_file;
   /// Header to use
   const SFDHeader& m_header;
   /// Listener
   SFDSearcherListener* m_listener;
   
   /// Indexor pointing to the middle position to search.
   SFDFileIndexor* m_fileIndexor;
   /// Indexor pointing to the first position to search.
   SFDIndexor* m_first;
   /// Indexor position to the last position to search.
   SFDIndexor* m_last;

   // -- Used while searching
   /// Lower bound
   SFDIndexor* m_left;
   /// Higher bound
   SFDIndexor* m_right;      
   /// String to look for when searching.
   MC2SimpleString m_toSearchfor;
   
   
};

#endif
