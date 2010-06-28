/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SFD_FILE_INDEXOR_H
#define SFD_FILE_INDEXOR_H

#include "config.h"

#include "SFDIndexor.h"

#include "BitBuffer.h"

#include "FileHandlerListener.h"

class FileHandler;
class SFDFileIndexorListener;
class SFDHeader;

/**
 *   SFDIndexor that can be read from the SFD-cache.
 */
class SFDFileIndexor : public SFDIndexor, public FileHandlerListener {
public:
   SFDFileIndexor( FileHandler* fh,
                   SFDFileIndexorListener* listener,
                   const SFDHeader& header );

   /// Has to be virtual or else the compiler complains.
   virtual ~SFDFileIndexor();
   
   /**
    *    Starts reading if necessary.
    *    Calls indexorDone on the listener if it is already done.
    */
   bool start();

   /**
    *    Sets new index to read.
    */
   void setStrNbr( uint32 idx ) {
      if ( m_strNbr != idx ) {
         m_strNbr = idx;
         m_strOffset = MAX_UINT32;
         m_stringRead = false;
      }
   }

   /**
    *    Converts a string number to an offset in the file.
    */
   uint32 strNbrToOffset( uint32 nbr ) const;

   /// Called by FileHandler when read is done
   void readDone( int nbrRead );

   /// Called by FileHandler when write is done
   void writeDone( int nbrWritten ) {
      // Unpossible
      MC2_ASSERT( false );
   }
   
private:


   /// File handler, i.e. a file.
   FileHandler* m_fileHandler;
   
   /// Object to call when done.
   SFDFileIndexorListener* m_listener;
      
   /// The File Header.   
   const SFDHeader* m_header;

   /// The state of this object.
   enum state_t {
      /// Nothing is to be done
      IDLE               = 0,
      /// A read of the offset has been requested
      READING_STR_OFFSET = 1,
      /// A read of the string itself has been requested.
      READING_STR        = 2,
      /// Permanent error has occured
      PERMANENT_ERROR    = 400,
   } m_state;

   /// The maximum string size in the cache.
   uint32 m_minBufSize;
   
   // -- Used when reading index and thus valid only in certain states.
   
   /// Buffer to read the string offset into.
   BitBuffer m_strOffsetBuf;
   /// Current string length
   uint32 m_curStrLength;

   // -- End Used when reading index.
   
};   

#endif // SFD_FILE_INDEXOR_H
