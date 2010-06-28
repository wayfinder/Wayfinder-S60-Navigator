/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MOVEMENTHELPER_H
#define MOVEMENTHELPER_H

/**
 *    A funny little class that helps TileMapHandler
 *    keeping track of how the map is moving.
 */
class MovementHelper {
   public:
      /// Constructor. Assumes that movement is detected self.
      MovementHelper() : 
         m_moving( false ), m_trustMovingMember( false ) {};

      /**
       *    Updates the drawOutlines attribute sent as parameter.
       *    I.e. if the attribute updated (overwritten).
       */
      void updateIfToDrawOutlines( bool& drawOutlines ) {
         if ( m_trustMovingMember ) {
            // Update the attribute.
            drawOutlines = ! m_moving;
         }
      }

      /**
       *    @return If movement of the map should be detected manually.
       */
      bool detectMovementBySelf() {
         return ! m_trustMovingMember;
      }
      
      /**
       *    Set if the map is moving or not.
       */
      void setMoving( bool moving ) {
         m_moving = moving;
      }

      /**
       *    Tell if the map is moving or not.
       */
      bool isMoving() {
         return m_moving;
      }

      /**
       *    Set if movement of the map is detected manually, or by
       *    using the setMoving() method.
       */
      void setDetectMovementBySelf( bool detectSelf ) {
         m_trustMovingMember = !detectSelf;
      }
      
   private:
      
      /**
       *    If the map is moving or not.
       */
      bool m_moving;
      
      /**
       *    Trust moving member, i.e. if not to detect movement by self.
       */
      bool m_trustMovingMember;
};

#endif
