/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_SERVER_ENUMHELPER_H
#define GUI_PROT_SERVER_ENUMHELPER_H

#include "arch.h"
#include "ServerEnums.h"
#include "TraceMacros.h"
#include "nav2util.h"

namespace isab {

      /**
       * Class for holding User's rights and using them.
       */
      class UserRights {
         public:
            /**
             * Constructor.
             */
            UserRights() : m_userRights( NULL ), m_userRightsLen( 0 ) {}


            /**
             * Destructor.
             */
            ~UserRights() { delete [] m_userRights; }


            /**
             * Set User's rights.
             */
            void setUserRights( const int32* data, int len ) {
               delete [] m_userRights;
               m_userRightsLen = len;
               m_userRights = new int32[ m_userRightsLen ];
               memcpy( m_userRights, data, m_userRightsLen*4 );
            }


            /**
             * Checks if a userRightService is in the user right param.
             * 
             * @param s The userRightService to check if set.
             */
            bool hasUserRight( GuiProtEnums::userRightService s ) const {
               if ( m_userRights == NULL ) {
                  return false;
               }
               bool res = false;
               if ( nbrBits( s ) == 1 ) {
                  res = (m_userRights[ 0 ] & s) != 0;
               } else {
                  // Find 
                  for ( int32 i = 1 ; i < m_userRightsLen ; ++i ) {
                     if ( m_userRights[ i ] == s ) {
                        res = true;
                        break;
                     }
                  }
               }
               return res;
            }


         private:
            /// The user's rights, may be NULL
            int32* m_userRights;


            /// The size if m_userRights.
            int32 m_userRightsLen;
      };


} // End isab namespace

#endif //GUI_PROT_SERVER_ENUMHELPER_H
