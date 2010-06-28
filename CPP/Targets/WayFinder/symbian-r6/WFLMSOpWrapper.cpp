/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <epos_landmarks.h>
#include <epos_cposlmoperation.h>

#include "WFLMSOpWrapper.h"

CWFLMSOpWrapper::CWFLMSOpWrapper(): 
   CActive(CActive::EPriorityStandard) 
{
   CActiveScheduler::Add(this);
}

// Destructor
CWFLMSOpWrapper::~CWFLMSOpWrapper()
{
   Cancel();
   delete iLmOperation;
   iLmOperation = NULL;
}

void CWFLMSOpWrapper::StartOperation(CPosLmOperation* aLmOperation,
                                     TRequestStatus& aStatus,
                                     TBool aReportProgress)
{
   delete iLmOperation;
   iLmOperation = aLmOperation;
   iCallerStatus = &aStatus;
   iReportProgress = aReportProgress;
   iProgress = 0;

   ExecuteNextStep();
}

CPosLmOperation* CWFLMSOpWrapper::LmOperationPtr()
{
   return iLmOperation;
}

void CWFLMSOpWrapper::ExecuteNextStep(TRequestStatus& aStatus)
{
   iCallerStatus = &aStatus;
   ExecuteNextStep();
}

TInt CWFLMSOpWrapper::Progress()
{
   return (TInt) (iProgress * 100);
}

void CWFLMSOpWrapper::DoCancel()
{
   delete iLmOperation;
   iLmOperation = NULL;
   User::RequestComplete(iCallerStatus, KErrCancel);
}

void CWFLMSOpWrapper::RunL()
{
   if (iStatus == KErrNone) {
      // Operation finished.
      User::RequestComplete(iCallerStatus, KErrNone);
   } else if (iStatus == KPosLmOperationNotComplete) {
      if (iReportProgress) {
         User::RequestComplete(iCallerStatus, KPosLmOperationNotComplete);
      } else {
         ExecuteNextStep();
      }
   }
}

void CWFLMSOpWrapper::ExecuteNextStep()
{
   iStatus = KRequestPending;
   iLmOperation->NextStep(iStatus, iProgress);
   SetActive();
}

void CWFLMSOpWrapper::ResetOperationPtr()
{
   if (!IsActive()) {
      delete iLmOperation;
      iLmOperation = NULL;
   }
}
