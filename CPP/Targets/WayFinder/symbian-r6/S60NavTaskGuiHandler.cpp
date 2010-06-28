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
#include <badesca.h>
#include "GuiFileOperation.h"
#include "EventGenerator.h"
#include "S60NavTaskGuiHandler.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFileMess.h"
#include "PopUpList.h"

#include "WFTextUtil.h"
#include "RsgInclude.h"
#include "WayFinderAppUi.h"

#include <coemain.h>
#include <aknquerydialog.h>
#include <aknmessagequerydialog.h> 

#include "nav2util.h"

#include "Dialogs.h"
#include "CallBackDialog.h"

using namespace isab;

void
CS60NavTaskGuiCommandHandler::ConstructL()
{
   iEventGenerator = CTNTEventGenerator::NewL(*this);
}

class CS60NavTaskGuiCommandHandler*
CS60NavTaskGuiCommandHandler::NewL(class CWayFinderAppUi* appUi)
{
   CS60NavTaskGuiCommandHandler* self =
      new (ELeave) CS60NavTaskGuiCommandHandler(appUi);
   CleanupStack::PushL(self);
   self->ConstructL();
   CleanupStack::Pop(self);
   return self;
}

CS60NavTaskGuiCommandHandler::~CS60NavTaskGuiCommandHandler()
{
   delete iEventGenerator;
   if (iEditCommand) {
      delete iEditCommand->iFileLocation;
      delete iEditCommand->iEditField;
   }
   delete iEditCommand;
   if (iConfirmCommand) {
      delete iConfirmCommand->iFileLocation;
      delete iConfirmCommand->iAdditionalInfo;
   }
   delete iConfirmCommand;
}

void
CS60NavTaskGuiCommandHandler::SendFileOperationL(class GuiFileOperation* op)
{
   GuiProtFileMess mess(op);
   iAppUi->SendMessageL(&mess);
   mess.deleteMembers();
}

void
CS60NavTaskGuiCommandHandler::ShowNoFilesErrorDialog()
{
   WFDialog::ShowScrollingWarningDialogL(R_WF_NO_FILES_AVAILABLE);
}

void
CS60NavTaskGuiCommandHandler::InitArrayFromLocationVector(CDesCArrayFlat* aArray,
      const LocationVector* loc)
{
   LocationVector::const_iterator it = loc->begin();

   while (it != loc->end()) {
      FileLocation* f = *it;
      switch (f->m_type) {
         case file_type_directory:
            {
            HBufC* tmp1 = WFTextUtil::AllocLC(f->m_dirname);
            HBufC* tmp2 = WFTextUtil::AllocLC(f->m_dir);
            HBufC* tmp3 = HBufC::NewLC(tmp1->Length()+tmp2->Length()+16);
            tmp3->Des().Copy(*tmp1);
            tmp3->Des().Append(KTab);
            tmp3->Des().Append(*tmp2);
            aArray->AppendL(*tmp3);
            CleanupStack::PopAndDestroy(tmp3);
            CleanupStack::PopAndDestroy(tmp2);
            CleanupStack::PopAndDestroy(tmp1);
            break;
         }
         case file_type_plain_file:
         {
            HBufC* tmp1 = WFTextUtil::AllocLC(f->m_name);
            HBufC* tmp2 = WFTextUtil::AllocLC(f->m_dirname);
            HBufC* tmp3 = HBufC::NewLC(tmp1->Length()+tmp2->Length()+16);
            tmp3->Des().Copy(*tmp1);
            tmp3->Des().Append(KTab);
            tmp3->Des().Append(*tmp2);
            aArray->AppendL(*tmp3);
            CleanupStack::PopAndDestroy(tmp3);
            CleanupStack::PopAndDestroy(tmp2);
            CleanupStack::PopAndDestroy(tmp1);
            break;
         }
         default:
            break;
      }
      it++;
   }
}

TInt
CS60NavTaskGuiCommandHandler::ShowSelectList(const LocationVector* loc,
      TInt lastUsedIndex)
{
   CDesCArrayFlat* descArray = new (ELeave) CDesCArrayFlat(10);
   CleanupStack::PushL( descArray );

   InitArrayFromLocationVector(descArray, loc);

   HBufC* title = CCoeEnv::Static()->AllocReadResourceLC(R_CONNECT_SELECT_ITEM);

   TBool okChosen = EFalse;
   if ((lastUsedIndex < 0) || (descArray->Count() <= lastUsedIndex)) {
      lastUsedIndex = 0;
   }

   TInt selection = PopUpList::ShowPopupListL(
         R_WAYFINDER_SOFTKEYS_OK_CANCEL,
         *descArray, okChosen, lastUsedIndex,
         ETrue,
         &(*title),
         0);
   CleanupStack::PopAndDestroy(title);
   CleanupStack::PopAndDestroy(descArray);

   if (okChosen) {
      /* Selected one of the choices. */
      return selection;
   } else {
      return -1;
   }
}

TBool
CS60NavTaskGuiCommandHandler::ShowFileNameEditBox(char *defaultName)
{
   HBufC* editField = HBufC::NewLC(256);

   if (defaultName) {
      WFTextUtil::char2HBufC(editField, defaultName);
   } else {
      editField->Des().Zero();
   }
   TPtr ptr = editField->Des();
   CleanupStack::Pop(editField);
   iEditCommand->iEditField = editField;

   typedef TCallBackEvent<CS60NavTaskGuiCommandHandler, TNTCommandEvent> cb_t;
   typedef CCallBackEditDialog<cb_t> cbd_t;
   cbd_t::RunDlgLD(cb_t(this,
            EEditOk,
            EEditCancel),
         R_WF_ENTER_NAME,
         ptr,
         R_WF_BLOCKING_DATA_QUERY);
   return ETrue;
}

TBool
CS60NavTaskGuiCommandHandler::ShowOverwriteConfirmPopup(char *additional)
{
   HBufC* additionalInfo = WFTextUtil::AllocLC(additional);
   iConfirmCommand->iAdditionalInfo = WFTextUtil::AllocLC(additional);
   HBufC* confirmText = CCoeEnv::Static()->AllocReadResourceLC(R_WF_CONFIRM_OVERWRITE_BODY);
   HBufC* tmp1 = HBufC::NewLC(additionalInfo->Length()+confirmText->Length()+16);
   tmp1->Des().Copy(*additionalInfo);
   tmp1->Des().Append(_L(" "));
   tmp1->Des().Append(*confirmText);
   
   CleanupStack::Pop(tmp1);
   CleanupStack::PopAndDestroy(confirmText);
   CleanupStack::Pop(iConfirmCommand->iAdditionalInfo);
   CleanupStack::PopAndDestroy(additionalInfo);

   typedef TCallBackEvent<CS60NavTaskGuiCommandHandler, TNTCommandEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   
   //Dialog takes ownership of HBufC that is sent to it.
   cbd_t::RunDlgLD(cb_t(this, EConfirmOk, EConfirmCancel),
                   R_WF_CONFIRM_OVERWRITE_HEADER,
                   tmp1,
                   R_NON_BLOCKING_QUERY_MESSAGE);
   return ETrue; 
}

TBool
CS60NavTaskGuiCommandHandler::ShowDeleteConfirmPopup(char *additional)
{
   HBufC* additionalInfo = WFTextUtil::AllocLC(additional);
   iConfirmCommand->iAdditionalInfo = WFTextUtil::AllocLC(additional);
   HBufC* confirmText = CCoeEnv::Static()->AllocReadResourceLC(R_WF_CONFIRM_DELETE_BODY);
   HBufC* tmp1 = HBufC::NewLC(additionalInfo->Length()+confirmText->Length()+16);
   tmp1->Des().Copy(*confirmText);
   tmp1->Des().Append(_L(" "));
   tmp1->Des().Append(*additionalInfo);
   tmp1->Des().Append(_L("?"));
   
   CleanupStack::Pop(tmp1);
   CleanupStack::PopAndDestroy(confirmText);
   CleanupStack::Pop(iConfirmCommand->iAdditionalInfo);
   CleanupStack::PopAndDestroy(additionalInfo);

   typedef TCallBackEvent<CS60NavTaskGuiCommandHandler, TNTCommandEvent> cb_t;
   typedef CCallBackDialog<cb_t> cbd_t;
   
   //Dialog takes ownership of HBufC that is sent to it.
   cbd_t::RunDlgLD(cb_t(this, EConfirmOk, EConfirmCancel),
                   R_WF_CONFIRM_DELETE_HEADER,
                   tmp1,
                   R_NON_BLOCKING_QUERY_MESSAGE);
   return ETrue;
}

void
CS60NavTaskGuiCommandHandler::HandleGuiFileOperationSelect(
      GuiFileOperationSelect* op)
{
   switch (op->command()) {
      case command_select_load:
      {
         uint32 numLocations = op->locations()->size();
         if (numLocations == 0) {
            /* No locations available... */
            WFDialog::ShowScrollingWarningDialogL(R_WF_NO_FILES_AVAILABLE);
            return;
         }
         /* Show choice to user with the entries in locations. */
         TInt selected = ShowSelectList(op->locations(), op->lastUsedIndex());
         GuiFileOperationSelectReply* rep;

         if (selected >= 0) {
            /* Selected one of the choices. */
            /* Create reply. */
            const FileLocation* org = (*(op->locations()))[selected];
            //char* filename = aBaseName(op->fileName());
            char* filename = aBaseName(org->m_name);
            FileLocation* f = new FileLocation(*org);

            //Set the destination field in RouteView(DestView).
            HBufC* symFileName = WFTextUtil::AllocLC(filename);
            TParsePtrC parser(*symFileName);
            iAppUi->SetCurrentRouteDestinationName(parser.Name());
            CleanupStack::PopAndDestroy(symFileName);

            /* Takes ownership of allocated entries. */
            rep = new GuiFileOperationSelectReply(
                  0, selected, op->command(), f, filename);
         } else {
            /* Didn't select anything. */
            rep = new GuiFileOperationSelectReply(
               0, 0, command_cancel, new FileLocation(), 0);
         }

         SendFileOperationL(rep);

         break;
      }
      case command_select_simulate:
      {
         uint32 numLocations = op->locations()->size();
         if (numLocations == 0) {
            /* No locations available... */
            WFDialog::ShowScrollingWarningDialogL(R_WF_NO_FILES_AVAILABLE);
            return;
         }
         /* Show choice to user with the entries in locations. */
         TInt selected = ShowSelectList(op->locations(), op->lastUsedIndex());
         GuiFileOperationSelectReply* rep;

         if (selected >= 0) {
            /* Selected one of the choices. */
            /* Create reply. */
            const FileLocation* org = (*(op->locations()))[selected];
            //char* filename = aBaseName(op->fileName());
            char* filename = aBaseName(org->m_name);
            FileLocation* f = new FileLocation(*org);
            /* Takes ownership of allocated entries. */
            rep = new GuiFileOperationSelectReply(
                  0, selected, op->command(), f, filename);
         } else {
            /* Didn't select anything. */
            rep = new GuiFileOperationSelectReply(
               0, 0, command_cancel, new FileLocation(), 0);
         }

         SendFileOperationL(rep);

         break;
      }
      case command_select_save:
      {
         FileLocation* f = NULL;
         char *filename = NULL;
         TInt selected = -1;

         uint32 numLocations = op->locations()->size();
         if (numLocations == 0) {
            /* No locations available... Strange... */
            WFDialog::ShowScrollingWarningDialogL(R_WF_NO_FILES_AVAILABLE);
            return;
         }
         if (numLocations == 1) {
            /* Only one location, use that. */
            const FileLocation* org = (op->locations())->front();
            filename = strdup_new(op->fileName());
            f = new FileLocation(*org);
         } else {
            /* numLocations > 1 */
            /* Show choice to user with the entries in locations. */
            selected = ShowSelectList(op->locations(), op->lastUsedIndex());
            if (selected >= 0) {
               /* Selected one of the choices. */
               /* Create reply. */
               const FileLocation* org = (*(op->locations()))[selected];
               filename = strdup_new(op->fileName());
               f = new FileLocation(*org);
            } else {
               /* Didn't select anything. */
               GuiFileOperationSelectReply* rep =
                  new GuiFileOperationSelectReply(
                        0, 0, command_cancel, new FileLocation(), 0);
               SendFileOperationL(rep);

               return;
            }

         }

         /* Save data for callback. */
         if (!iEditCommand) {
            iEditCommand = new CEditFileNameOperation();
            iEditCommand->iCommand = op->command();
            iEditCommand->iFileLocation = f;
            iEditCommand->iSelected = selected;
         }

         /* Show editbox for filename. Return value is given in callback. */
         if (! ShowFileNameEditBox(filename) ) {
            /* Cancelled. */
            delete iEditCommand;
            /* Use f and filename, but only to make sure that */
            /* they are deleted. */
            GuiFileOperationSelectReply* rep =
               /* TInt res = dlg->ExecuteLD(R_WF_BLOCKING_DATA_QUERY); */
               new GuiFileOperationSelectReply(
                  0, 0, command_cancel, f, filename);
            SendFileOperationL(rep);
         } else {
            delete[] filename;
         }
         break;
      }
      case command_select_delete:
      {
         uint32 numLocations = op->locations()->size();
         if (numLocations == 0) {
            /* No locations available... */
            WFDialog::ShowScrollingWarningDialogL(R_WF_NO_FILES_AVAILABLE);
            return;
         }
         /* Show choice to user with the entries in locations. */
         TInt selected = ShowSelectList(op->locations(), op->lastUsedIndex());
         GuiFileOperationSelectReply* rep;

         if (selected >= 0) {
            /* Selected one of the choices. */
            /* Create reply. */
            const FileLocation* org = (*(op->locations()))[selected];
            //char* filename = aBaseName(op->fileName());
            char* filename = aBaseName(org->m_name);
            FileLocation* f = new FileLocation(*org);
            /* Takes ownership of allocated entries. */
            rep = new GuiFileOperationSelectReply(
                  0, selected, op->command(), f, filename);
         } else {
            /* Didn't select anything. */
            rep = new GuiFileOperationSelectReply(
               0, 0, command_cancel, new FileLocation(), 0);
         }

         SendFileOperationL(rep);

         break;  
      }
      default:
         break;
   }
}

void
CS60NavTaskGuiCommandHandler::HandleGuiFileOperationConfirm(
      GuiFileOperationConfirm* op)
{
   switch (op->type()) 
      {
      case command_confirm_overwrite:
         {
            FileLocation* f = NULL;
            char *additional = NULL;
            additional = strdup_new(op->additional());

            const FileLocation* org = op->location();
            f = new FileLocation(*org);

            if (!iConfirmCommand) {
               iConfirmCommand = new CConfirmCommandOperation();
               iConfirmCommand->iCommand = op->type();
               iConfirmCommand->iFileLocation = f;
            } 

            /* Show editbox for filename. Return value is given in callback. */
            if (!ShowOverwriteConfirmPopup(additional)) {
               /* Cancelled. */
               delete iConfirmCommand;
               /* Use f and filename, but only to make sure that */
               /* they are deleted. */
               GuiFileOperationConfirmReply* rep =
                  /* TInt res = dlg->ExecuteLD(R_WF_BLOCKING_DATA_QUERY); */
                  new GuiFileOperationConfirmReply(
                          0, command_confirm_no, f);
            SendFileOperationL(rep);
            } else {
               delete[] additional;
            }
            break;
         }
      case command_confirm_delete:
         {
            char *additional = strdup_new(op->additional());
            const class FileLocation* org = op->location();
            class FileLocation* f = new FileLocation(*org);

            if (!iConfirmCommand) {
               iConfirmCommand = new CConfirmCommandOperation();
               iConfirmCommand->iCommand = op->type(); //command_confirm_del..
               iConfirmCommand->iFileLocation = f;
            } 

            /* Show editbox for filename. Return value is given in callback. */
            if (!ShowDeleteConfirmPopup(additional)) {
               /* Cancelled. */
               delete iConfirmCommand;
               /* Use f and filename, but only to make sure that */
               /* they are deleted. */
               GuiFileOperationConfirmReply* rep =
                  /* TInt res = dlg->ExecuteLD(R_WF_BLOCKING_DATA_QUERY); */
                  new GuiFileOperationConfirmReply(
                          0, command_confirm_no, f);
               SendFileOperationL(rep);
            } else {
               delete[] additional;
            }
            break;
         }
      default:
         break;
      }
}

void
CS60NavTaskGuiCommandHandler::GenerateEvent(enum TNTCommandEvent aEvent)
{
   iEventGenerator->SendEventL(aEvent);
}

void
CS60NavTaskGuiCommandHandler::HandleGeneratedEventL(enum TNTCommandEvent aEvent)
{

   switch (aEvent) {
      case EEditOk:
         {
            EditOk(iEditCommand);
            iEditCommand = 0;
            break;
         }
      case EEditCancel:
         {
            EditCancel(iEditCommand);
            iEditCommand = 0;
            break;
         }
   case EConfirmOk:
         {
            ConfirmOk(iConfirmCommand);
            iConfirmCommand = 0;
            break;
         }
      case EConfirmCancel:
         {
            ConfirmCancel(iConfirmCommand);
            iConfirmCommand = 0;
            break;
         }
      default:
         break;
   }
}

