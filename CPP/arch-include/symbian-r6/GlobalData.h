/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
 *    Global data stored on the heap and referenced through 
 *    the Tls (Thread Local Store).
 */
#ifndef GlobalData_H
#define GlobalData_H

#include "arch.h"
#include "../factored/GlobalData.incl"
#include "TlsGlobalData.h"

namespace isab {

   class ArchGlobalData {
      public:
      ArchGlobalData();
      class TimerThread * m_timerThread;
      class RSocketServ * m_socketServ;
      bool m_use_exception_handler;
      TExceptionHandlerPtr m_exceptionHandler;
   };

   inline ArchGlobalData::ArchGlobalData() : 
      m_timerThread(NULL), m_socketServ(NULL), 
      m_use_exception_handler(true), m_exceptionHandler(NULL)
   {}


   inline GlobalData& getGlobalData() 
   {
      return *(getTlsGlobalData()->m_global);
   }

   inline ArchGlobalData& getArchGlobalData() 
   {
      return *(getTlsGlobalData()->m_archGlobal);
   }

}

#endif /* GlobalData_H */
