/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "ActiveLog.h"
#include "Log.h"


CActiveLog::CActiveLog(enum TPriority aPriority, const char* aPrefix) : 
   CActive(aPriority)
{
   SetPrefix(aPrefix);
}

CActiveLog::CActiveLog(const char* aPrefix) :
   CActive(EPriorityNormal)
{
   SetPrefix(aPrefix);
}

CActiveLog::~CActiveLog()
{
   delete iLog;
}

void CActiveLog::UpdateLogMastersL()
{
   if(iLog){
      CArrayPtr<CActiveLog>* logarray = SubLogArrayLC();
      if(logarray){
         for(TInt i = 0; i < logarray->Count(); ++i){
            if(logarray->At(i)){
               logarray->At(i)->SetLogMasterL(iLog->getLogMaster());
            }
         }  
      }
      CleanupStack::PopAndDestroy(logarray);
   }
}

CArrayPtr<CActiveLog>* CActiveLog::SubLogArrayLC()
{
   CArrayPtr<CActiveLog>* ret = NULL;
   CleanupStack::PushL(ret);
   return ret;
}

void CActiveLog::SetPrefix(const char* aPrefix)
{
   strncpy(iPrefix, aPrefix, sizeof(iPrefix));
   iPrefix[sizeof(iPrefix) - 1] = '\0';
}

void CActiveLog::SetLogMasterL(class isab::LogMaster* aLogMaster)
{
   delete iLog;
   iLog = NULL;
   iLog = new (ELeave) isab::Log(iPrefix, isab::Log::LOG_ALL, aLogMaster);
   UpdateLogMastersL();
}
