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
#ifndef TlsGlobalData_H
#define TlsGlobalData_H
#include <e32std.h>
#include <map>
namespace isab {
   class Thread;
}

void Panic(::isab::Thread* thread);

namespace isab{

   class TlsGlobalData {

      public:
 
      TlsGlobalData(class TlsGlobalData &t, class InnerThread *new_thread);
      TlsGlobalData();

      class InnerThread * m_innerThread;
      class GlobalData * m_global;
      class ArchGlobalData * m_archGlobal;
   };

#ifdef SYMBIAN_9
	class TTlsData{
		std::map<TUint64, isab::TlsGlobalData*> iTlsMap;
		class RCriticalSection iMutex;
	public:
		TTlsData();
		~TTlsData();
		void SetTls(isab::TlsGlobalData*);
		isab::TlsGlobalData* GetTls();
		void DeleteTls(isab::TlsGlobalData*);
	};
#endif


#if defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3
   extern isab::TTlsData globalDataWsd;
#endif

   inline class isab::TlsGlobalData* getTlsGlobalData() 
   {
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
      return reinterpret_cast<isab::TlsGlobalData *>(Dll::Tls());
#else
      return globalDataWsd.GetTls();
#endif
   }

   inline void setTlsGlobalData(void* data)
   {
#if !(defined NAV2_CLIENT_UIQ3 || defined NAV2_CLIENT_SERIES60_V3)
      Dll::SetTls(data);
#else
      isab::TlsGlobalData* threadData = reinterpret_cast<isab::TlsGlobalData *>(data);
      globalDataWsd.SetTls(threadData);
#endif
   }
   
   inline void deleteTlsGlobalData(void* data) {
      isab::TlsGlobalData* threadData = reinterpret_cast<isab::TlsGlobalData *>(data);
#ifndef SYMBIAN_9
      delete threadData;
#else
      globalDataWsd.DeleteTls(threadData);
#endif
   }
}

#endif /* TlsGlobalData_H */
