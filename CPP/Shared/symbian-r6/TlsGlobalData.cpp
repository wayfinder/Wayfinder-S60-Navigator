/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "TlsGlobalData.h"

#ifdef SYMBIAN_9
namespace {	
	TUint64 getThreadHandle() {
   		class RThread thisThread;
   		class TThreadId id = thisThread.Id();
        TUint64 uId = id.Id();		
		return uId;
	}
}
#endif

namespace isab{

   TlsGlobalData::TlsGlobalData(class TlsGlobalData &t, class InnerThread *new_thread)
   : m_innerThread(new_thread), 
     m_global(t.m_global),
     m_archGlobal(t.m_archGlobal)
   {
   }

   TlsGlobalData::TlsGlobalData()
   :m_innerThread(NULL), 
    m_global(NULL),
    m_archGlobal(NULL)
   {
   }

#if defined SYMBIAN_9
   isab::TTlsData globalDataWsd;

	TTlsData::TTlsData(){
	   iMutex.CreateLocal();
	}
	TTlsData::~TTlsData() {
		iMutex.Close();
	}

   void TTlsData::SetTls(isab::TlsGlobalData* aData) {
        TUint64 uId = getThreadHandle();
        iMutex.Wait();
        iTlsMap[uId] = aData;
        iMutex.Signal();
   }
   
   isab::TlsGlobalData* TTlsData::GetTls(){
   		TUint64 uId = getThreadHandle();
   		iMutex.Wait();
   		isab::TlsGlobalData* ret = iTlsMap[uId];
   		iMutex.Signal();
   		return ret;
   }
   
   void TTlsData::DeleteTls(isab::TlsGlobalData* aData){
      TUint64 uId = getThreadHandle();
      iMutex.Wait();
   	  std::map<TUint64, isab::TlsGlobalData*>::iterator it = iTlsMap.find(uId);
   	  if(it != iTlsMap.end()){
   	     isab::TlsGlobalData* data = it->second;
         if(aData != NULL && data != aData){
            User::Panic(_L("TlsData"), 0);
         }
         iTlsMap.erase(it);
         delete data;
      } else {
         User::Panic(_L("TlsData"), 1);
      }
      iMutex.Signal();
   }
#endif
	
}
