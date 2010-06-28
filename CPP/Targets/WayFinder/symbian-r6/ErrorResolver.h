/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __ERRORRESOLVER_H__
#define __ERRORRESOLVER_H__

const TInt KErrorBufLength = 32;

/** 
 * @class CUtilities
  
 * @discussion 
 */
class CErrorResolver
{
public:

   inline static void GetError( TInt aError, TDes* aText );

};

#endif

void CErrorResolver::GetError( TInt aError, TDes* aText )
{
   switch( aError )
   {
   case KErrNotFound:
      aText->Append( _L("Not Found") );
      break;
   case KErrGeneral:
      aText->Append( _L("General") );
      break;
   case KErrCancel:
      aText->Append( _L("Cancel") );
      break;
   case KErrNoMemory:
      aText->Append( _L("No Memory") );
      break;
   case KErrNotSupported:
      aText->Append( _L("Not Supported") );
      break;
   case KErrArgument:
      aText->Append( _L("Argument") );
      break;
   case KErrTotalLossOfPrecision:
      aText->Append( _L("Total Loss Of Precision") );
      break;
   case KErrBadHandle:
      aText->Append( _L("Bad Handle") );
      break;
   case KErrOverflow:
      aText->Append( _L("Overflow") );
      break;
   case KErrUnderflow:
      aText->Append( _L("Underflow") );
      break;
   case KErrAlreadyExists:
      aText->Append( _L("Already Exists") );
      break;
   case KErrPathNotFound:
      aText->Append( _L("Path Not Found") );
      break;
   case KErrDied:
      aText->Append( _L("Died") );
      break;
   case KErrInUse:
      aText->Append( _L("In Use") );
      break;
   case KErrServerTerminated:
      aText->Append( _L("Server Terminated") );
      break;
   case KErrServerBusy:
      aText->Append( _L("Server Busy") );
      break;
   case KErrCompletion:
      aText->Append( _L("Completion") );
      break;
   case KErrNotReady:
      aText->Append( _L("Not Ready") );
      break;
   case KErrUnknown:
      aText->Append( _L("Unknown") );
      break;
   case KErrCorrupt:
      aText->Append( _L("Corrupt") );
      break;
   case KErrAccessDenied:
      aText->Append( _L("Access Denied") );
      break;
   case KErrLocked:
      aText->Append( _L("Locked") );
      break;
   case KErrWrite:
      aText->Append( _L("Write") );
      break;
   case KErrDisMounted:
      aText->Append( _L("DisMounted") );
      break;
   case KErrEof:
      aText->Append( _L("Eof") );
      break;
   case KErrDiskFull:
      aText->Append( _L("Disk Full") );
      break;
   case KErrBadDriver:
      aText->Append( _L("Bad Driver") );
      break;
   case KErrBadName:
      aText->Append( _L("Bad Name") );
      break;
   case KErrCommsLineFail:
      aText->Append( _L("Comms Line Fail") );
      break;
   case KErrCommsFrame:
      aText->Append( _L("Comms Frame") );
      break;
   case KErrCommsOverrun:
      aText->Append( _L("Comms Overrun") );
      break;
   case KErrCommsParity:
      aText->Append( _L("Comms Parity") );
      break;
   case KErrTimedOut:
      aText->Append( _L("Timed Out") );
      break;
   case KErrCouldNotConnect:
      aText->Append( _L("Could Not Connect") );
      break;
   case KErrCouldNotDisconnect:
      aText->Append( _L("Could Not Disconnect") );
      break;
   case KErrDisconnected:
      aText->Append( _L("Disconnected") );
      break;
   case KErrBadLibraryEntryPoint:
      aText->Append( _L("Bad Library Entry Point") );
      break;
   case KErrBadDescriptor:
      aText->Append( _L("Bad Descriptor") );
      break;
   case KErrAbort:
      aText->Append( _L("Abort") );
      break;
   case KErrTooBig:
      aText->Append( _L("Too Big") );
      break;
   case KErrDivideByZero:
      aText->Append( _L("Divide By Zero") );
      break;
   case KErrBadPower:
      aText->Append( _L("Bad Power") );
      break;
   case KErrDirFull:
      aText->Append( _L("Dir Full") );
      break;
   case KErrHardwareNotAvailable:
      aText->Append( _L("Hardware Not Available") );
      break;
   default:
      aText->Append( _L("Unknown Error Code") );
   }
}
