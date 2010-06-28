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


#include "arch.h"

/* #include <badesca.h> */
//#include "GuiFileOperation.h"
#include "NavTaskGuiHandler.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFileMess.h"

#include "nav2util.h"

#include "WFTextUtil.h"

using namespace isab;

char *
CNavTaskGuiCommandHandler::aBaseName(const char *full)
{
   HBufC* tmp = WFTextUtil::AllocLC(full);
   TInt i = tmp->LocateReverse('/');
   CleanupStack::PopAndDestroy(tmp);

   char *filename = NULL;

   if (KErrNotFound == i) {
      filename = strdup_new(full);
   } else {
      filename = strdup_new((const char*)&full[i]);
   }
   return filename;
}


void
CNavTaskGuiCommandHandler::SendCommandL(uint16 command)
{
   class GuiFileOperationCommand *op =
      new GuiFileOperationCommand(0, command);

   SendFileOperationL(op);
}

void
CNavTaskGuiCommandHandler::HandleGuiFileOperation(GuiProtFileMess* message)
{
   /* Handle file operation messages. */
   GuiFileOperation* op = message->getFileOperation();
   switch (op->typeId()) {
      case OPERATION_SELECT:
         /* Show user a selection dialogue. */
         HandleGuiFileOperationSelect(static_cast<GuiFileOperationSelect*>(op));
         break;
      case OPERATION_SELECT_REPLY:
         /* Should never get here. */
         break;
      case OPERATION_CONFIRM:
         /* Show user a confirmation dialogue. */
         HandleGuiFileOperationConfirm(static_cast<GuiFileOperationConfirm*>(op));
         break;
      case OPERATION_CONFIRM_REPLY:
         /* Should never get here. */
         break;
      case OPERATION_COMMAND:
         break;
   }
}

void
CNavTaskGuiCommandHandler::EditOk(class CEditFileNameOperation* edit)
{
   char *filename = WFTextUtil::newTDesDupL(*edit->iEditField);
   delete edit->iEditField;
   /* Takes ownership of allocated entries. */
   GuiFileOperationSelectReply* rep =
      new GuiFileOperationSelectReply(
            0,
            edit->iSelected,
            edit->iCommand,
            edit->iFileLocation,
            filename);
   SendFileOperationL(rep);

   delete edit;
}

void
CNavTaskGuiCommandHandler::EditCancel(class CEditFileNameOperation* edit)
{
   char *filename = WFTextUtil::newTDesDupL(*edit->iEditField);
   delete edit->iEditField;

   /* Takes ownership of allocated entries. */
   GuiFileOperationSelectReply* rep =
      new GuiFileOperationSelectReply(
            0,
            edit->iSelected,
            command_cancel,
            edit->iFileLocation,
            filename);
   SendFileOperationL(rep);

   delete edit;
}

void
CNavTaskGuiCommandHandler::ConfirmOk(class CConfirmCommandOperation* confirm)
{
   delete confirm->iAdditionalInfo;

   /* Takes ownership of allocated entries. */
   GuiFileOperationConfirmReply* rep =
      new GuiFileOperationConfirmReply(
            0,
            confirm->iCommand,
            confirm->iFileLocation);
   SendFileOperationL(rep);

   delete confirm;
}

void
CNavTaskGuiCommandHandler::ConfirmCancel(class CConfirmCommandOperation* confirm)
{
   delete confirm->iAdditionalInfo;

   /* Takes ownership of allocated entries. */
   GuiFileOperationConfirmReply* rep =
      new GuiFileOperationConfirmReply(
            0,
            command_confirm_no,
            confirm->iFileLocation);
   SendFileOperationL(rep);

   delete confirm;
}

