/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __LANDMARKS_LM_OP_WRAPPER_H__
#define __LANDMARKS_LM_OP_WRAPPER_H__

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CPosLmOperation;

// CLASS DECLARATION

/**
*  Active class that wraps an instance of CPosLmOperation.
*  The wrapped CPosLmOperation instance is executed incrementally. 
*  Execution progress can be reported.
*  This class is designed to be utilized by active objects.
*/
class CWFLMSOpWrapper: public CActive
{
public: // Constructors and destructor

        /**
         * C++ default constructor.
         * 
         */
   CWFLMSOpWrapper();

   /**
    * Destructor.
    */
   ~CWFLMSOpWrapper();

public: // New Functions
   
   /**
    * Starts execution of the supplied operation.
    *
    * @param aLmOperation operation to execute. Ownership of the operation
    * is transferred to this instance.
    * @param aStatus the request status to complete when execution of the 
    * supplied LmOperation is finished.
    * @param aReportProgress ETrue if progress should be reported 
    * (incremental execution), EFalse otherwise.
    */
   void StartOperation(CPosLmOperation* aLmOperation,
                       TRequestStatus& aStatus,
                       TBool aReportProgress = EFalse);

   /**
    * Returns a pointer to the wrapped CPosLmOperation object. 
    * This object keeps ownership of the operation.
    *
    * @return a pointer to a CPosLmOperation instance
    */
   CPosLmOperation* LmOperationPtr();

   /**
    * Executes the next step of the incremental operation. Typically 
    * called when progress of the CPosLmOperaiotion is reported.
    *
    * @param aStatus the request status to complete when execution of the 
    * next step is finished.
    */
   void ExecuteNextStep(TRequestStatus& aStatus);

   /**
    * Return the current progress of the CPosLmnOperation.
    *
    * @return a number between 0-100. 100 means that the operation is 
    * completed
    */
   TInt Progress();

   /**
    * Deletes iLmOperation if we dont have an outstanding request.
    */
   void ResetOperationPtr();

protected: // from CActive

   /**
    * Implements cancellation of an outstanding request.
    */
   void DoCancel();

   /**
    * Handles an active objectÂ?s request completion event.
    */
   void RunL();

private:

   /**
    * Executes the next step of the incremental operation.
    */
   void ExecuteNextStep();

private: // Data

   /// the wrapped CPosLmOperation that is executed
   CPosLmOperation* iLmOperation;
   /// contains a value between 0-1 and indicates the progress
   TReal32 iProgress;
   /// the req. status of the caller executing the operation
   TRequestStatus* iCallerStatus;
   /// indicates if progress should be reported or not
   TBool iReportProgress;
};


#endif // __LANDMARKS_LM_OP_WRAPPER_H__

// End of File
