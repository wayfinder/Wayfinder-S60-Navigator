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

#ifndef NAV_TASK_GUI_HANDLER_H
#define NAV_TASK_GUI_HANDLER_H

#include "GuiFileOperation.h"

namespace isab {
   class GuiProtFileMess;
}

enum TNTCommandEvent {
   EEditOk,
   EEditCancel,
   EConfirmOk,
   EConfirmCancel,
   EListOk,
   EListCancel,
   EMessageOk,
   EMessageCancel
}; 

class CEditFileNameOperation : public CBase
{
public:
   uint16 iCommand;
   class isab::FileLocation* iFileLocation;
   TInt iSelected;
   HBufC* iEditField;
};

class CConfirmCommandOperation : public CBase
{
public:
   class isab::FileLocation* iFileLocation;
   uint16 iCommand;
   HBufC* iAdditionalInfo;
};

class CNavTaskGuiCommandHandler : public CBase
{
public:
   /* Called by WayfinderAppUi to handle Navtask Gui messages. */
   virtual void HandleGuiFileOperation(isab::GuiProtFileMess* message);
   /* Shorthand for sending command messages to NavTask. */
   virtual void SendCommandL(uint16 command);
   /* Returns an allocated string for the file component of parameter. */
   virtual char* aBaseName(const char *full);

private:
   /** These methods needs to be implemented in GUI. */

   /* Convenience method for sending GuiProtFileMess. */
   virtual void SendFileOperationL(class isab::GuiFileOperation* op) = 0;

   /* Handle select and confirm operations, has to be derived. */
   virtual void HandleGuiFileOperationSelect(isab::GuiFileOperationSelect* op) = 0;
   virtual void HandleGuiFileOperationConfirm(isab::GuiFileOperationConfirm* op) = 0;

   virtual TBool ShowFileNameEditBox(char *defaultName) = 0;

   virtual TBool ShowOverwriteConfirmPopup(char *additional) = 0;

   virtual TBool ShowDeleteConfirmPopup(char *additional) = 0;

   virtual TInt ShowSelectList(const isab::LocationVector* loc,
         TInt lastUsedIndex) = 0;

   virtual void ShowNoFilesErrorDialog() = 0;

public:
   virtual void EditOk(class CEditFileNameOperation* edit);
   virtual void EditCancel(class CEditFileNameOperation* edit);
   virtual void ConfirmOk(class CConfirmCommandOperation* confirm);
   virtual void ConfirmCancel(class CConfirmCommandOperation* confirm);
};

#endif

