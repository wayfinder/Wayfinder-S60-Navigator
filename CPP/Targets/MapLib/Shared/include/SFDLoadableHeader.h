/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDHEADERLOADABLE_H
#define SFDHEADERLOADABLE_H 

#include "config.h"

#include "SFDHeader.h"
#include "FileHandler.h"

class SharedBuffer;
class XorFileHandler;
class SFDLoadableHeaderListener;

/**
 *    SFDHeader that can be loaded from file.
 */
class SFDLoadableHeader : public SFDHeader, public FileHandlerListener
{
public:
   
   /**
    *    Constructor.
    *
    *    Xor buffers used for encryption should be passed as parameters.
    *    The correct encryption buffer will be selected in the load method.
    *    
    *    @param   uidXorbuffer   Xor buffer encrypted using uid.
    *    @param   warezXorbuffer Xor buffer encrypted using warez encryption.
    */ 
   SFDLoadableHeader( const SharedBuffer* uidXorbuffer,
                      const SharedBuffer* warezXorbuffer );
   
   /**
    *    Destructor.
    */
   virtual ~SFDLoadableHeader();

   /**
    *    Loads the SFDLoadableHeader.
    *    @param   file     The file handler pointing to the
    *                      single file cache to load.
    *    @param   listener Listener that will be notified when the header 
    *                      has been loaded.
    */
   void load( FileHandler* file, SFDLoadableHeaderListener* listener );

private:

   /**
    *    Load the initial part of the header.
    */
   void loadInitialHeader( SharedBuffer& buf );
   
   /**
    *    Load the remaining part of the header.
    */
   void loadRemainingHeader( SharedBuffer& buf );

   /**
    *    Get number of bytes to read next.
    *    Returns zero when there is nothing left to read.
    */
   uint32 getNbrBytesToRead() const;
  
   /**
    *    Load more data if necessary or 
    *    notify the listener that the header has been loaded.
    */
   void innerLoad();

   /**
    *    Handle that the filehandler has finished reading.
    */
   void readDone( int nbrRead );

   /// Should not be the case.
   void writeDone( int ) {
      MC2_ASSERT( false );
   }
   
   /**
    *    Returns zero if ok. 
    */
   int status() const;

   /// Encryption type.
   enum encryption_t {
      no_encryption        = 0,
      uid_encryption       = 1,
      warez_encryption     = 2,
   } m_encryptionType;

   /// The state.
   enum state_t {
      nothing_loaded        = 0,
      loaded_initial_header = 1,
      loaded_all_header     = 2,
      failed_to_load        = 404
   } m_state;

   /// The number of bytes to read. Becomes zero when all is read.
   uint32 m_nbrBytesToRead;

   /**
    *    The xor buffer to use for encryption. 
    *    Can be NULL, m_uidXorBuffer or m_warezXorBuffer.
    */
   const SharedBuffer* m_xorBuffer;

   /// Xor buffer based on uid.
   const SharedBuffer* m_uidXorBuffer;

   /// Xor buffer based on warez encoding.
   const SharedBuffer* m_warezXorBuffer;

   /// The listener that should be informed when the header has been loaded.
   SFDLoadableHeaderListener* m_listener;

   /// The read buffer.
   uint8* m_readBuffer;
   
   /// The file handler with correct xor encryption / decryption.
   XorFileHandler* m_fileHandler;
   
};


// --- Inlines ---

#endif
