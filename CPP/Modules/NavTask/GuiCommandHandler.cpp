/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GuiCommandHandler.h"

#ifndef UNDER_CE
# include <sys/stat.h>
# include <dirent.h>
#endif

#include "MsgBufferEnums.h"
#include "Module.h"
#include "Buffer.h"
#include "nav2util.h"
#include "CtrlHub.h"

#include "isabTime.h"
#include "Quality.h"
#include "PositionState.h"
#include "RouteEnums.h"
#include "NavTask.h"

#include "NavPacket.h"
#include "MapEnums.h"
#include "NavServerComEnums.h"
#include "GuiProt/ServerEnums.h"
#include "NavServerCom.h"

#include "ParameterEnums.h"
#include "Parameter.h"
#include "../NavTask/Point.h"
#include "../NavTask/Route.h"

#include "Gps.h"
#include "NavTaskInternal.h"

# define DBG  m_navTask->m_log->debug
# define INFO m_navTask->m_log->info
# define WARN m_navTask->m_log->warning
# define ERR  m_navTask->m_log->error

using namespace isab;

GuiCommandHandler::GuiCommandHandler(class NavTask* nt)
{
   m_navTask = nt;
}

GuiCommandHandler::~GuiCommandHandler()
{
}

void GuiCommandHandler::HandleOperationSelect(class GuiFileOperationSelect* /*op*/)
{
}  

void GuiCommandHandler::HandleOperationSelectReply(class GuiFileOperationSelectReply* op)
{
   const uint16 opCommand = op->command();
   switch(opCommand)
      {
      case command_select_load:
         {
            const class FileLocation *FileLoc = op->location();
            char* sLoadFile = NULL;
            if (FileLoc) {
               sLoadFile = createFullPathName_new(FileLoc->m_dir, FileLoc->m_name);
            }
            //Save selected index to parameter file.
            int32 selectedIndex = op->selected();
            m_navTask->m_parameter->setParam(ParameterEnums::R_lastIndex, &selectedIndex);
            int retval = m_navTask->m_receiveRoute->readRoute(sLoadFile);
            if (retval == 0) {
               m_navTask->m_awaitingRouteReply = true;
               m_navTask->switchToNewRoute();
            }
            else {
               DBG("Could not load route.");
            }
            if (sLoadFile) {
               delete[] sLoadFile;
            }
         }
         break;
      case command_select_simulate:
         {
            const class FileLocation *FileLoc = op->location();
            char* sLoadFile = NULL;
            if (FileLoc) {
               sLoadFile = createFullPathName_new(FileLoc->m_dir, FileLoc->m_name);
            }
            int retval = m_navTask->m_receiveRoute->readRoute(sLoadFile);
            if (retval == 0) {
               m_navTask->m_awaitingRouteReply = true;
               m_navTask->switchToNewRoute();
               m_navTask->setupGpsSimulation();
            }
            else {
               DBG("Could not load route.");
            }
            if (sLoadFile) {
               delete[] sLoadFile;
            }
         }
         break;
      case command_select_save:
         {
            const class FileLocation *FileLoc = op->location();
            char* sSaveFile = NULL;
            char* paramFileName = NULL;
            if (FileLoc) {
               sSaveFile = createFullPathName_new(FileLoc->m_dir, op->fileName());
               //Save filename to paramfile as default filename. 
               paramFileName = addFileExtension_new(op->fileName());
               m_navTask->m_parameter->setParam(ParameterEnums::R_routeName, &paramFileName);
            }
            //Check if file already exists.
            FILE* pSaveFile = fopen(sSaveFile, "r");
            if (pSaveFile != NULL) {
               fclose(pSaveFile);
               char* sFileName = addFileExtension_new(op->fileName());
               class FileLocation *FileLocTmp =
                  new FileLocation(sFileName, 
                                   strdup_new(FileLoc->m_dir), 
                                   strdup_new(FileLoc->m_dirname), 
                                   file_type_plain_file);
               char* sAdditional = strdup_new(sFileName);
               class GuiFileOperationConfirm *opConfirm =
                  new GuiFileOperationConfirm(0, command_confirm_overwrite,
                                              FileLocTmp, sAdditional);
               m_navTask->m_navTaskConsumer->sendFileOperation(opConfirm, 
                                                               m_navTask->m_fileOperationRequester);
            }
            else {
               int retval = m_navTask->m_route->writeRoute(sSaveFile);
               if (retval == 0) {
                  DBG("Saved route to stable media");
               }
               else {
                  DBG("Could not save route.");
               }
            }
            if (sSaveFile) {
               delete[] sSaveFile;
            }
            if (paramFileName) {
               delete[] paramFileName;
            }
         }
         break;
      case command_select_delete:
         {
            //If file exists send confirm operation.
            const class FileLocation *FileLoc = op->location();
            char* sDeleteFile = NULL;
            if (FileLoc) {
               sDeleteFile = createFullPathName_new(FileLoc->m_dir, op->fileName());
            }
            //Check if file exists.
            FILE* pDeleteFile = fopen(sDeleteFile, "r");
            if (pDeleteFile != NULL) {
               fclose(pDeleteFile);
               char* sFileName = addFileExtension_new(op->fileName());
               class FileLocation *FileLocTmp =
                  new FileLocation(sFileName, 
                                   strdup_new(FileLoc->m_dir), 
                                   strdup_new(FileLoc->m_dirname), 
                                   file_type_plain_file);
               char* sAdditional = strdup_new(sFileName);
               class GuiFileOperationConfirm *opConfirm =
                  new GuiFileOperationConfirm(0, command_confirm_delete,
                                              FileLocTmp, sAdditional);
               m_navTask->m_navTaskConsumer->sendFileOperation(opConfirm, 
                                                               m_navTask->m_fileOperationRequester);
            }
            else {
               DBG("GuiCommand: Delete - This should never happen.");
            }
            if (sDeleteFile) {
               delete[] sDeleteFile;
            }
         }
         break;
      case command_cancel:
         {
            //Do nothing!
         }
         break;
      }
}

void GuiCommandHandler::HandleOperationConfirm(class GuiFileOperationConfirm* /*op*/)
{
}

void GuiCommandHandler::HandleOperationConfirmReply(class GuiFileOperationConfirmReply* op)
{
   const uint16 opCommand = op->command();
   switch(opCommand){
      case command_confirm_overwrite:
         {
            const class FileLocation *FileLoc = op->location();
            char* sSaveFile = NULL;
            if (FileLoc) {
               sSaveFile = createFullPathName_new(FileLoc->m_dir, FileLoc->m_name);
            }
            int retval = m_navTask->m_route->writeRoute(sSaveFile);
            if (retval == 0) {
               DBG("Saved route to stable media");
            }
            else {
               DBG("Could not save route.");
            }
            if (sSaveFile) {
               delete[] sSaveFile;
            }
         }
         break;
      case command_confirm_delete:
         {
            //Do the real file deleting here.
            const class FileLocation *FileLoc = op->location();
            char* sDeleteFile = NULL;
            if (FileLoc) {
               sDeleteFile = createFullPathName_new(FileLoc->m_dir, FileLoc->m_name);
               int retval = m_navTask->m_route->deleteRoute(sDeleteFile);
               if (retval == 0) {
                  DBG("Deleted route from stable media");
               } else {
                  DBG("Could not delete route. errcode %d", retval);
               }
               delete[] sDeleteFile;
            }
         }
         break;
      case command_cancel:
         //Do nothing!
         DBG("command_cancel redId: %"PRIx16, op->seqId());
         break;
      case command_confirm_no:
         //Do nothing!
         DBG("command_confirm_no redId: %"PRIx16, op->seqId());
         break;
      }
}

void GuiCommandHandler::HandleOperationCommand(class GuiFileOperationCommand* op)
{
   const uint16 opCommand = op->command();
   switch(opCommand)
      {
      case command_load:
         {
            LocationVector *locations = new LocationVector;
            locations->reserve(4);
            LocationVector *driveList = new LocationVector;
            driveList->reserve(2);
            getDriveList(driveList);
            findSavedRoutes(locations, driveList);
            char* sFileName = NULL;
            if (locations->size() > 0) {
               class FileLocation *FileLocTmp = *locations->begin();
               sFileName = strdup_new(FileLocTmp->m_name);
            }
            //Last used index read from parameter file.
            int32 lastIndex = m_navTask->getRouteLastUsedIndex();
            class GuiFileOperationSelect *opSelect =
               new GuiFileOperationSelect(0, lastIndex, command_select_load, sFileName, locations);
            m_navTask->m_navTaskConsumer->sendFileOperation(opSelect, 
                                                            m_navTask->m_fileOperationRequester);
            if (driveList) {
               LocationVector::iterator it = driveList->begin();
               while (it != driveList->end()) {
                  delete (*it);
                  it++;
               }
               driveList->clear();
            }
            delete driveList;
         }
         break;
      case command_load_and_simulate:
         {
            LocationVector *locations = new LocationVector;
            locations->reserve(4);
            LocationVector *driveList = new LocationVector;
            driveList->reserve(2);
            getDriveList(driveList);
            findSavedRoutes(locations, driveList);
            char* sFileName = NULL;
            if (locations->size() > 0) {
               class FileLocation *FileLocTmp = *locations->begin();
               sFileName = strdup_new(FileLocTmp->m_name);
            }
            //Last used index read from parameter file.
            int32 lastIndex = m_navTask->getRouteLastUsedIndex();
            class GuiFileOperationSelect *opSelect =
               new GuiFileOperationSelect(0, lastIndex, command_select_simulate, sFileName, locations);
            m_navTask->m_navTaskConsumer->sendFileOperation(opSelect, 
                                                            m_navTask->m_fileOperationRequester);
            if (driveList) {
               LocationVector::iterator it = driveList->begin();
               while (it != driveList->end()) {
                  delete (*it);
                  it++;
               }
               driveList->clear();
            }
            delete driveList;
         }
         break;
      case command_save:
         {
            LocationVector *locations = new LocationVector;
            locations->reserve(2);
            getDriveList(locations);
            char* sFileName = NULL;
            //sFileName read from parameter file.
            sFileName = strdup_new(m_navTask->getRouteFileName());
            class GuiFileOperationSelect *opSelect =
               new GuiFileOperationSelect(0, 0, command_select_save, sFileName, locations);
            m_navTask->m_navTaskConsumer->sendFileOperation(opSelect, 
                                                            m_navTask->m_fileOperationRequester);
         }
         break;
      case command_delete:
         {
            //Show list of file to delete from stable media.
            LocationVector *locations = new LocationVector;
            locations->reserve(4);
            LocationVector *driveList = new LocationVector;
            driveList->reserve(2);
            getDriveList(driveList);
            findSavedRoutes(locations, driveList);
            char* sFileName = NULL;
            if (locations->size() > 0) {
               class FileLocation *FileLocTmp = *locations->begin();
               sFileName = strdup_new(FileLocTmp->m_name);
            }
            class GuiFileOperationSelect *opSelect =
               new GuiFileOperationSelect(0, 0, command_select_delete, sFileName, locations);
            m_navTask->m_navTaskConsumer->sendFileOperation(opSelect, 
                                                            m_navTask->m_fileOperationRequester);
            if (driveList) {
               LocationVector::iterator it = driveList->begin();
               while (it != driveList->end()) {
                  delete (*it);
                  it++;
               }
               driveList->clear();
            }
            delete driveList;
         }
         break;
      case command_delete_all:
         {
            //Delete all routes from stable media.
         }
         break;
      case command_simulate_start:
         {
            m_navTask->setupGpsSimulation();
         }
         break;
      case command_simulate_stop:
         {
            m_navTask->setSimStop();
         }
         break;
      case command_simulate_pause:
         {
            m_navTask->setSimPaused();
         }
         break;
      case command_simulate_resume:
         {
            m_navTask->resumeSimPaused();
         }
         break;
      case command_simulate_inc_speed:
         {
            m_navTask->incSimSpeed();
         }
         break;
      case command_simulate_dec_speed:
         {
            m_navTask->decSimSpeed();
         }
         break;
      case command_simulate_rep_on:
         {
            m_navTask->setSimRepeat();
         }
         break;
      case command_simulate_rep_off:
         {
            m_navTask->stopSimRepeat();
         }
         break;
      }
}

void GuiCommandHandler::findSavedRoutes(LocationVector* locations, 
                                        LocationVector* driveList)
{
#ifndef UNDER_CE
   //iterate over drivelists
   LocationVector::iterator it = driveList->begin();
   while (it != driveList->end()) {
      FileLocation* fileloc = *it;
      char sFullPath[256];
      //open dir in drivelist iterator
      DIR *pDir = opendir(fileloc->m_dir);
      struct dirent *pDirentStruct;
      struct stat st;
      if (pDir != NULL) {
         while((pDirentStruct = readdir(pDir))) {
            //skip if find . and ..
            if ((strcmp(pDirentStruct->d_name, ".") == 0 ||
                 strcmp(pDirentStruct->d_name, "..") == 0)) {
               continue;
            }
            //only get the route files (.rut)
            if (strstr(pDirentStruct->d_name, ".rut")) {
               char* sCurrentFile = strdup_new(pDirentStruct->d_name);
               char* sSaveDir = strdup_new(fileloc->m_dir);
               char* sSaveDirName = strdup_new(fileloc->m_dirname);
               strcpy(sFullPath, "");
               strcat(sFullPath, sSaveDir);
               strcat(sFullPath, sCurrentFile);
               int statret = stat(sFullPath, &st);
               if (statret != -1 && S_ISDIR(st.st_mode)) {
                  //This is a directory not a route file.
                  delete[] sCurrentFile;
                  delete[] sSaveDir;
               } else {
                  class FileLocation *FileLoc = 
                     new FileLocation(sCurrentFile, sSaveDir, 
                                      sSaveDirName, file_type_plain_file);
                  locations->push_back(FileLoc);
               }
            }
         }
         closedir(pDir);
      }
      it++; 
   }
#else
   WIN32_FIND_DATA wfd,wffd;
   WCHAR* sFullMask = new WCHAR[MAX_PATH]; 
   LocationVector::iterator it = driveList->begin();
   while (it != driveList->end()) {
      FileLocation* fileloc = *it;
      // convert the UTF8 text to Unicode
      MultiByteToWideChar(CP_ACP,
                          0,
                          fileloc->m_dir,
                          -1,
                          sFullMask,
                          MAX_PATH);

      BOOL bRootDir = FALSE;
      //check if we have root directory
      if (!wcscmp(sFullMask,L"\\")){
          bRootDir = TRUE;
      } else if (sFullMask[wcslen(sFullMask) - 1] == L'\\') {
         //remove slash at the end of the path, if it exists
         sFullMask[wcslen(sFullMask) - 1] = L'\0';
      }
      HANDLE hDir = FindFirstFile(sFullMask,&wfd);
      if (hDir != INVALID_HANDLE_VALUE && 
          (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
         if(bRootDir){
            wcscat(sFullMask,L"*.rut");
         } else {
            wcscat(sFullMask,L"\\*.rut");
         }
         HANDLE hFind = FindFirstFile(sFullMask,&wffd);
         if (hFind != INVALID_HANDLE_VALUE) {
            do {
               if (!(wffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                  //determine size of buffer needed for UTF-8 string
                  int nLength = WideCharToMultiByte(CP_ACP, 0, wffd.cFileName,
                                                    -1, NULL, 0, NULL, NULL);
                  char* sCurrentFile= new char[nLength];
                  /* convert the Unicode text to UTF-8 */
                  WideCharToMultiByte(CP_ACP,
                                      0,
                                      wffd.cFileName,
                                      -1,
                                      sCurrentFile,
                                      nLength,
                                      NULL,
                                      NULL);
                  char* sSaveDir     = strdup_new(fileloc->m_dir);
                  char* sSaveDirName = strdup_new(fileloc->m_dirname);
                  class FileLocation *FileLoc = 
                     new FileLocation(sCurrentFile, 
                                      sSaveDir, 
                                      sSaveDirName, 
                                      file_type_plain_file);
                  locations->push_back(FileLoc);
               }
            } while (FindNextFile(hFind, &wffd));
         }
         FindClose(hFind);
      }
      FindClose(hDir);
      it++;
  }   
  delete[] sFullMask;
#endif
}

char* GuiCommandHandler::createFullPathName_new(const char* sDir, const char* sName)
{
   if (sDir && sName) {
      char* sFullPath = new char[strlen(sDir) + strlen(sName) + 5];
      strcpy(sFullPath, sDir);
      strcat(sFullPath, sName);
      if (! strstr(sName, ".rut")) {
         strcat(sFullPath, ".rut");
      }
      return sFullPath;
   } else {
      return NULL;
   }
}

char* GuiCommandHandler::addFileExtension_new(const char* sFileName)
{
   if (sFileName) {
      char* sFullName = new char[strlen(sFileName) + 5];
      strcpy(sFullName, sFileName);
      if (! strstr(sFileName, ".rut")) {
         strcat(sFullName, ".rut");
      }
      return sFullName;
   } else {
      return NULL;
   }
}
