/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XORFILEHANDLER_H
#define XORFILEHANDLER_H

#include "config.h"

#include "XorHelper.h"
#include "FileHandler.h"

/**
 *    FileHandler that can handle files that are partly xor "encrypted".
 */
class XorFileHandler : public FileHandler, public FileHandlerListener {
public:

   /**
    *    Constructs a XorFilehandler by passing another filehandler to it.
    *    The supplied filehandler will not be deleted by the 
    *    XorFileHandler's constructor.
    */
   XorFileHandler( FileHandler* fh )
         : FileHandler( fh->getFileName() ),
           m_fh( fh ), 
           m_xorHelper( NULL, 0, 0 ),
           m_pos( 0 ) {}

   /**
    *    Set the xor helper.
    */
   void setXorHelper( const XorHelper& xorHelper ) { 
      m_xorHelper = xorHelper;
   }
                                       
   /**
    *   Destroys the file handler and closes the file.
    */
   virtual ~XorFileHandler() {}

   /**
    *   Returns the original file handler.
    */
   FileHandler* getOriginalFileHandler() const {
      return m_fh;
   }
   
   /**
    *   Clears the file to zero length.
    */
   void clearFile() { m_fh->clearFile(); }

   /**
    *   Returns 0 if everything has been ok so far.
    */
   int status() const { return m_fh->status(); }
   
   /**
    *   Reads maxLength bytes from the file.
    *   Calls the listener when done. If listener is NULL the
    *   call will by synchronous and the number of bytes written
    *   will be returned.
    */
   int read( uint8* bytes,
             int maxLength,
             FileHandlerListener* listener ) {
      m_lastReadBuffer = bytes;
      m_listener = listener;
      return m_fh->read( bytes, maxLength, this );
   }

   /**
    *   Sets the read and write positions of the stream to the sent-in value.
    *   -1 should set the position to end of file.
    */
   void setPos( int pos ) { m_pos = pos; m_fh->setPos( pos ); }

   /**
    *   Sets the size of the file.
    *   @param size  The new size in bytes.
    */
   void setSize( int size ) { m_fh->setSize( size ); }
   
   /**
    *   Returns the read/write position in the stream.
    */
   int tell() { return m_pos; }
   
   /**
    *   Writes bytes to the file of the FileHandler.
    *   Calls the listener when it is done.
    *   If listener is NULL the
    *   call will by synchronous and the number of bytes written
    *   will be returned.
    */
   int write( const uint8* bytes,
              int length,
              FileHandlerListener* listener ) {
      xorBuffer( const_cast<uint8*> (bytes), m_pos, length );
      m_listener = listener;
      return m_fh->write( bytes, length, this ); 
   }

   /**
    *   Returns the modification date of the file.
    *   Currently it is enough if the date is less for
    *   old files and more for new files.
    */
   uint32 getModificationDate() const { return m_fh->getModificationDate(); }

   /**
    *   Returns the amount of available space on the drive
    *   where the file is located.
    */
   uint32 getAvailableSpace() const { return m_fh->getAvailableSpace(); }

   /**
    *   Cancels outstanding reads/writes.
    */
   void cancel() { m_fh->cancel(); }
   
   /**
    *   Returns the size of the file.
    */
   int getFileSize() { return m_fh->getFileSize(); }

   /**
    *   Called when the read is done.
    *   @param nbrRead The number of bytes read. Negative value for failure.
    */
   void readDone( int nbrRead ) {
      xorBuffer( m_lastReadBuffer, m_pos, nbrRead );
      m_pos += nbrRead;
      m_listener->readDone( nbrRead );
   }

   /**
    *   Called when the write is done.
    *   @param nbrWritten The number of bytes written. Negative for failure.
    */
   void writeDone( int nbrWritten ) { 
      m_pos += nbrWritten; 
      m_listener->writeDone( nbrWritten );
   }

private:

   /**
    *   Xor the supplied buffer.
    *   @param buf      The buffer to xor.
    *   @param pos      The position of the buffer in the file.
    *   @param length   The length of the buffer.
    */
   void xorBuffer( uint8* buf, int pos, int length ) {
      m_xorHelper.xorBuffer( buf, pos, length ); 
   }

   /// The filehandler.
   FileHandler* m_fh;
   
   /// The XorHelper.
   XorHelper m_xorHelper;
   
   /// The position of the file.
   int m_pos;

   /// The last readbuffer.
   uint8* m_lastReadBuffer;

   /// The listener.
   FileHandlerListener* m_listener;

};

#endif
