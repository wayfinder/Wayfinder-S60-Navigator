/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XORHELPER_H
#define XORHELPER_H 

#include "config.h"

/**
 *    Helper class for xor encrypted buffers.
 *    Handles that the buffer is encrypted first at a certain offset.
 */
class XorHelper {
public:

   /**
    *    Constructor.
    *    @param   xorBuf      The xor buffer.
    *    @param   xorLength   The length of the xor buffer.
    *    @param   xorOffset   The start offset of the xor buffer.
    *                         I.e. a file encrypted with this xor buffer
    *                         will first be xor:ed beginning at this offset.
    */
   XorHelper( const byte* xorBuf,
              uint32 xorLength,
              uint32 xorOffset ) : m_xorBuf( xorBuf ),
                                   m_xorLength( xorLength ),
                                   m_xorOffset( xorOffset ) {}

   /**
    *   Perfrom the xor:ing of the supplied buffer.
    *   @param buf   The buffer to xor.
    *   @param pos   The position of the start of the buffer.
    *   @param len   The length of the buffer.
    */
   void xorBuffer( byte* buf, uint32 pos, uint32 len ) const
   {
      if ( m_xorBuf == NULL || m_xorLength == 0 ) {
         return;
      }
      if ( pos + len < m_xorOffset ) {
         return;
      }
      if ( pos < m_xorOffset ) {
         pos = m_xorOffset;
         buf += m_xorOffset - pos;
         len -= m_xorOffset - pos;
      }

      for ( uint32 i = 0; i < len; ++i ) {
         buf[ i ] ^= m_xorBuf[ (pos + i) % m_xorLength ];
         ++pos;
      }
   }
    
private:

   /// The xor buffer.
   const byte* m_xorBuf;

   /// The length of the xor buffer.
   uint32 m_xorLength;

   /** 
    *    The start offset of the xor buffer.
    *    I.e. a file encrypted with this xor buffer
    *    will first be xor:ed beginning at this offset.
    */
   uint32 m_xorOffset;

};


#endif
