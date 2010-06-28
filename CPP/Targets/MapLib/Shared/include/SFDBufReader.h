/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDBUFREADER_H
#define SFDBUFREADER_H

#include "config.h"

#include "BitBuffer.h"

#include "FileHandlerListener.h"

class FileHandler;
class SFDHeader;
class SFDBufReaderListener;

/**
 *   This class reads the buffer offset and size and then
 *   it reads the buffer itself. It has some similarities
 *   with SFDFileIndexor, so they should really share some
 *   code....
 */
class SFDBufReader : public FileHandlerListener {
public:
   /**
    *   Creates a new SFDBufReader.
    *   @param fh FileHandler to use. Can be shared, but should
    *             not be used simultaneously with in other places.
    */
   SFDBufReader( const SFDHeader& header,
                 FileHandler& fh,
                 SFDBufReaderListener& listener );

   /// Deletes internal data. Compiler insists that it is virtual
   virtual ~SFDBufReader();

   /// Starts the SFDBufReader using a strIdx
   void start( uint32 strIdx );

   /// Starts the SFDBufReader using an absolute offset to the lengths
   void startAbsolute( uint32 offset );
   
   /// Called by fileListener when read is done.
   void readDone( int nbrRead );
   
   /// Called by fileListener when write is done.
   void writeDone( int nbrWritten ) {
      // Should not happen.
      MC2_ASSERT( false );
   }
   
private:

   /// Converts string number to buffer index offset.
   uint32 strNbrToBufIdxOffset( uint32 strNbr ) const;
   
   /// The file handler to use.
   FileHandler* m_fileHandler;

   /// Listener to inform when buffer reading is done or has failed.
   SFDBufReaderListener* m_listener;

   /// Reference to file header
   const SFDHeader& m_header;
   
   /// The state of this object.
   enum state_t {
      /// Nothing is to be done
      IDLE               = 0,
      /// A read of the offset has been requested
      READING_BUF_OFFSET = 1,
      /// A read of the string itself has been requested.
      READING_BUF        = 2,
      /// Permanent error has occured
      PERMANENT_ERROR    = 400,
   } m_state;

   // - Temporary things, used in different states.
   
   /// Buffer to read two buffer sizes into.
   BitBuffer m_bufOffsetBuf;
   /// Buffer which is used to hold the map
   BitBuffer* m_readBuffer;

   // - End temporary things.
   
};

#endif // SFDBUFREADER_H
