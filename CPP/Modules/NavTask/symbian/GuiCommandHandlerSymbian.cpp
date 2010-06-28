/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"

#include <f32file.h>
#include <eikenv.h>

#include "GlobalData.h"
#include "WFTextUtil.h"
#include "nav2util.h"
#include "GuiCommandHandler.h"

using namespace isab;

void GuiCommandHandler::getDriveList(LocationVector* locations)
{
   _LIT(KRouteDir, "routes\\");
   TDriveList driveList; //TDriveList is a typedef for TBuf8<KMaxDrives>
   class RFs fsSession;
   TInt retval = fsSession.Connect();
   if (retval == KErrNone) {
      //get the list of drives.
      retval = fsSession.DriveList(driveList);
      if (retval == KErrNone) {
         //check each drive in turn
         for(TInt driveNumber= EDriveA; driveNumber <= EDriveZ; ++driveNumber){
            if (driveList[driveNumber]) {
               //the drive contains "recognised media".
               struct TDriveInfo driveInfo;
               fsSession.Drive(driveInfo, driveNumber);
               if (driveInfo.iType == EMediaHardDisk ||
                   driveInfo.iType == EMediaFlash
#ifdef SYMBIAN_9
                   || driveInfo.iType == EMediaNANDFlash
#endif
                   ) {
                  //the drive is of a suitable type
                  if (!(driveInfo.iMediaAtt & KMediaAttWriteProtected)) {
                     //the drive is not write protected
                     TChar driveLetter; 
                     retval = fsSession.DriveToChar(driveNumber, driveLetter);
                     if (retval == KErrNone) { 
                        //we have a drive letter
                        HBufC* ptrDirPath = HBufC::NewL(256);
                        WFTextUtil::char2HBufC(ptrDirPath, 
                                               getGlobalData().m_commondata_base_path);
                        TPtr dirPath = ptrDirPath->Des();
                        //dirpath contains the common data path
                        dirPath.Append(KRouteDir); //append route dir
                        //replace the drive letter with the drive
                        //currently being investigated
                        dirPath[0] = driveLetter;
                        //create the dir if it doesn't exist.
                        retval = fsSession.MkDirAll(dirPath);
                        if(retval == KErrNone || retval == KErrAlreadyExists){
                           //we have the dir name and we know it exists. 
                           class TVolumeInfo volumeInfo;
                           fsSession.Volume(volumeInfo, driveNumber);
                           char* sVolumeName = NULL;
                           if (volumeInfo.iName.Length() > 0) {
                              //the volume is named, copy that name. 
                              sVolumeName = WFTextUtil::newTDesDupL(volumeInfo.iName);
                           } else {
                              //no name, use a name from loc file that
                              //says Unnamed something.
                              if (driveInfo.iDriveAtt & KDriveAttInternal) {
                                 sVolumeName = strdup_new("Phone");
                              } 
                              else if(driveInfo.iDriveAtt & KDriveAttRemovable){
                                 sVolumeName = strdup_new("Unnamed MMC");
                              }
                              else {
                                 sVolumeName = strdup_new("Unknown");
                              }
                           }
                           //set up info for this to be a possible directory 
                           char* sDirPath = WFTextUtil::newTDesDupL(dirPath);
                           char* sDirName = strdup_new(sVolumeName);
                           class FileLocation *FileLoc = 
                              new FileLocation(sVolumeName, sDirPath, sDirName, 
                                               file_type_directory);
                           locations->push_back(FileLoc);
                        }
                        delete ptrDirPath;
                     }
                  }
               }
            }
         }
      }
      fsSession.Close();
   }
}
