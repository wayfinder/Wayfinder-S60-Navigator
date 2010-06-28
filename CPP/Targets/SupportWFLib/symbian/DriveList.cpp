/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "DriveList.h"
#include <e32std.h>
#include <eikenv.h>
#include <bautils.h>

CDriveList::CDriveList(RFs& aFsSession) : iFsSession(aFsSession)
{
}

CDriveList::~CDriveList()
{
   if(iDriveArray){
      iDriveArray->Reset();
      delete iDriveArray;
   }
   if(iDriveListArray){
      iDriveListArray->Reset();
      delete iDriveListArray;
   }
}

CDriveList* CDriveList::NewLC()
{
   RFs& fsSession = CEikonEnv::Static()->FsSession();
   CDriveList* self = new (ELeave) CDriveList(fsSession);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CDriveList* CDriveList::NewL()
{
   CDriveList* self = CDriveList::NewLC();
   CleanupStack::Pop(self);
   return self;
}

void CDriveList::ConstructL()
{
   iDriveListArray = new (ELeave) CDesCArrayFlat(2);
   iDriveArray = new (ELeave) CDesCArrayFlat(2);
   iCount = 0;
}

CDesCArray* CDriveList::GenerateDriveListL()
{
   _LIT(KMmc, "MMC");
   _LIT(KPhone, "Phone");
   _LIT(KComma, ", ");
   TDriveList driveList;
   User::LeaveIfError(iFsSession.DriveList(driveList));
   TChar driveLetter; 
   TInt driveNumber=EDriveA; 
   for(;driveNumber<=EDriveZ;driveNumber++) {
      if (driveList[driveNumber]) {
         TDriveInfo driveInfo;
         iFsSession.Drive(driveInfo, driveNumber);
         User::LeaveIfError(iFsSession.DriveToChar(driveNumber,driveLetter));
         switch(driveInfo.iType) {
         case EMediaFlash: {
            if(!(driveInfo.iMediaAtt & KMediaAttWriteProtected)) {
               HBufC* driveAndName = HBufC::NewLC(50);
               TPtr pDriveAndName = driveAndName->Des();
               pDriveAndName.Append(KPhone);
               iDriveArray->AppendL(*driveAndName);
               HBufC* dl = HBufC::NewLC(1);
               TPtr pDl = dl->Des();
               pDl.Append(driveLetter);
               iDriveListArray->AppendL(*dl);
               CleanupStack::PopAndDestroy(dl);
               CleanupStack::PopAndDestroy(driveAndName);
               //iFsSession.Delete(fileName);
            }
            break;
         }
         case EMediaHardDisk: {
            if(!(driveInfo.iMediaAtt & KMediaAttWriteProtected)) {
               HBufC* driveAndName = HBufC::NewLC(50);
               TPtr pDriveAndName = driveAndName->Des();
               pDriveAndName.Append(KMmc);
               TVolumeInfo volumeInfo;
               iFsSession.Volume(volumeInfo, driveNumber);
               if(volumeInfo.iName.Length() > 0) {
                  pDriveAndName.Append(KComma);
                  pDriveAndName.Append(volumeInfo.iName);
               }
               iDriveArray->AppendL(*driveAndName);
               HBufC* dl = HBufC::NewLC(1);
               TPtr pDl = dl->Des();
               pDl.Append(driveLetter);
               iDriveListArray->AppendL(*dl);
               //iFsSession.Delete(fileName);
               CleanupStack::PopAndDestroy(dl);
               CleanupStack::PopAndDestroy(driveAndName);
            }
            break;
         }
         default: 
            break;
         }
      }
   }
   return iDriveArray;
}

TInt CDriveList::Count()
{
   iCount = 0;
   TDriveList driveList;
   User::LeaveIfError(iFsSession.DriveList(driveList));
   TChar driveLetter; 
   TInt driveNumber=EDriveA; 
   for(;driveNumber<=EDriveZ;driveNumber++) {
      if (driveList[driveNumber]) {
         TDriveInfo driveInfo;
         iFsSession.Drive(driveInfo, driveNumber);
         User::LeaveIfError(iFsSession.DriveToChar(driveNumber,driveLetter));
         switch(driveInfo.iType) {
         case EMediaFlash:   
            iCount++;
            break;
         case EMediaHardDisk:
            iCount++;
            break;
         default:
            break;
         }
      }
   }
   return iCount;
}

TPtrC CDriveList::Get(TInt index)
{
   if (iDriveListArray && index < iDriveListArray->Count()) {
      return iDriveListArray->MdcaPoint(index);
   } else {
      return KNullDesC();
   }
}
