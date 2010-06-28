/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef S60_NAV_TASK_GUI_COMMAND_HANDLER_H
#define S60_NAV_TASK_GUI_COMMAND_HANDLER_H

#include "NavTaskGuiHandler.h"

class CS60NavTaskGuiCommandHandler : public CNavTaskGuiCommandHandler
{
   CS60NavTaskGuiCommandHandler(class CWayFinderAppUi* appUi)
      : iAppUi(appUi), iEditCommand(0), iConfirmCommand(0) {}
   void ConstructL();
public:
   static class CS60NavTaskGuiCommandHandler* NewL(class CWayFinderAppUi* appUi);

   ~CS60NavTaskGuiCommandHandler();

private:
   /* Convenience method for sending GuiProtFileMess. */
   virtual void SendFileOperationL(class isab::GuiFileOperation* op);

   /* Show no files error popup */
   virtual void ShowNoFilesErrorDialog();

   /* Convenience method for creating an array from a location vector */
   void InitArrayFromLocationVector(CDesCArrayFlat* aArray,
         const isab::LocationVector* loc);

   /* Show select file list */
   TInt ShowSelectList(const isab::LocationVector* loc,
         TInt lastUsedIndex);

   /* Show save file edit box popup */
   TBool ShowFileNameEditBox(char *defaultName);

   /* Show confirm overwrite popup */
   TBool ShowOverwriteConfirmPopup(char *additional);

   /* Show confirm delte popup */
   TBool ShowDeleteConfirmPopup(char *additional);

   /* Handle select operation */
   virtual void HandleGuiFileOperationSelect(isab::GuiFileOperationSelect* op);

   /* Handle confirm operation */
   virtual void HandleGuiFileOperationConfirm(isab::GuiFileOperationConfirm* op);

public:
   /* Handle callbacks. */
   void GenerateEvent(enum TNTCommandEvent aEvent);
   void HandleGeneratedEventL(enum TNTCommandEvent aEvent);

private:

   /* Pointer to AppUi for sending messages. */
   class CWayFinderAppUi* iAppUi;

   /* Saved data for editbox callback. */
   class CEditFileNameOperation* iEditCommand;

   /* Saved data for confirmbox callback. */
   class CConfirmCommandOperation* iConfirmCommand;

   /* Event generator. */
   typedef CEventGenerator<CS60NavTaskGuiCommandHandler, enum TNTCommandEvent>
      CTNTEventGenerator;
   CTNTEventGenerator* iEventGenerator;

};

#endif

