/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


/*
 * This file contains messages used to present a simple GUI
 * to the user with file selection, saving and other operations.
 * Used from Nav2 to get user-responses.
 */

#ifndef GUI_FILE_OPERATION_H
#define GUI_FILE_OPERATION_H
#include "arch.h"
#include <vector>

namespace isab
{
   enum gui_file_operation_file_command {
      command_ignore             = 0x00,
      command_save               = 0x01,
      command_load               = 0x02,
      command_load_and_simulate  = 0x03,
      command_select_load        = 0x04,
      command_select_save        = 0x05,
      command_select_simulate    = 0x06,
      command_select_delete      = 0x07,
      command_select             = 0x08,
      command_chdir              = 0x09,
      command_cancel             = 0x0a,
      command_delete             = 0x0b,
      command_delete_all         = 0x0c,
      command_confirm_load       = 0x0d,
      command_confirm_write      = 0x0e,
      command_confirm_overwrite  = 0x0f,
      command_confirm_delete     = 0x10,
      command_confirm_delete_all = 0x11,
      command_confirm_yes        = 0x12,
      command_confirm_no         = 0x13,
      command_simulate_start     = 0x14,
      command_simulate_stop      = 0x15,
      command_simulate_pause     = 0x16,
      command_simulate_resume    = 0x17,
      command_simulate_inc_speed = 0x18,
      command_simulate_dec_speed = 0x19,
      command_simulate_rep_off   = 0x1a,
      command_simulate_rep_on    = 0x1b,
   };

   enum gui_file_operation_file_type {
      file_type_ignore           = 0x0,
      file_type_directory        = 0x1,
      file_type_directory_up     = 0x2,
      file_type_plain_file       = 0x3,
      file_type_divider          = 0x4,
      file_type_empty_row        = 0x5,
   };

   enum gui_file_operation {
      OPERATION_SELECT           = 0x0,
      OPERATION_SELECT_REPLY     = 0x1,
      OPERATION_CONFIRM          = 0x2,
      OPERATION_CONFIRM_REPLY    = 0x3,
      OPERATION_COMMAND          = 0x4,
   };

   class FileLocation {
      public:

         FileLocation(const FileLocation& org);
         FileLocation(char* name, char* dir, char* dirname, int32 type)
            : m_name(name), m_dir(dir), m_dirname(dirname), m_type(type) {};
         FileLocation() : m_name(0), m_dir(0), m_dirname(0), m_type(0) {};
         ~FileLocation() { delete[] m_name; delete[] m_dir; delete[] m_dirname; };

         void readFromBuffer(class Buffer* buf);
         void writeToBuffer(class Buffer* buf);

         /**
          * User understandable name for entry.
          * May contain date or other additional info.
          */
         char *m_name;

         /**
          * Directory or file path.
          */
         char *m_dir;

         /**
          * Directory name.
          * User understandable name for directory.
          */
         char *m_dirname;

         /**
          * Type of file.
          */
         int32 m_type;
   };
   typedef std::vector<class FileLocation*> LocationVector;

   /**
    * Base class for file operation messages.
    * Contains common data.
    */
   class GuiFileOperation
   {
      public:
         GuiFileOperation(uint16 seqId = 0) : m_seqId(seqId) {};
         virtual ~GuiFileOperation();

         virtual const uint16 typeId() const = 0;

         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         uint16 seqId() { return m_seqId; };

         static class GuiFileOperation* createFromBuffer(Buffer* buf);
      private:
         uint16 m_seqId;
   };

   /**
    * Select: This message informs the GUI to show a selection
    * notice to the user, and reply with the user's choice.
    */
   class GuiFileOperationSelect : public GuiFileOperation
   {
      public:
         GuiFileOperationSelect();
         GuiFileOperationSelect(uint16 seqId, uint16 lastUsedIndex,
               uint16 command, char *fileName, LocationVector* locations)
            : GuiFileOperation(seqId), m_lastUsedIndex(lastUsedIndex),
              m_command(command), m_fileName(fileName),
              m_locations(locations) { };

         ~GuiFileOperationSelect();
         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         virtual const uint16 typeId() const { return OPERATION_SELECT; }

         const char *fileName() const { return m_fileName; }
         const LocationVector *locations() const { return m_locations; }
         const uint16 command() const { return m_command; }
         const uint16 lastUsedIndex() const { return m_lastUsedIndex; }
      private:

         /**
          * Defines which entry in the list was used last time,
          * for use as default.
          */
         uint16 m_lastUsedIndex;

         /**
          * Command: Save, Select...
          */
         uint16 m_command;

         /**
          * Filename template.
          */
         char *m_fileName;

         /**
          * Vector of different locations to be shown to the user.
          * Each location is defined as a string for directory
          * and one string for description of the directory.
          * A integer type is also included.
          */
         LocationVector* m_locations;
   };

   class GuiFileOperationSelectReply : public GuiFileOperation
   {
      public:
         GuiFileOperationSelectReply();

         GuiFileOperationSelectReply(uint16 seqId, uint16 selected,
               uint16 command, class FileLocation* location, char* fileName)
            : GuiFileOperation(seqId), m_selected(selected),
              m_command(command), m_location(location),
              m_fileName(fileName) {};

         ~GuiFileOperationSelectReply();
         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         virtual const uint16 typeId() const { return OPERATION_SELECT_REPLY; }

         const char *fileName() const { return m_fileName; }
         const class FileLocation *location() const { return m_location; }
         const uint16 command() const { return m_command; }
         const uint16 selected() const { return m_selected; }
      private:

         /**
          * Index of the selected entry.
          * This may be unnecessary.
          */
         uint16 m_selected;

         /**
          * Command: Save, Cancel, Chdir ...
          */
         uint16 m_command;

         /**
          * Selected entry data.
          */
         class FileLocation* m_location;

         /**
          * Additional filename string (user input)
          */
         char *m_fileName;

   };
   class GuiFileOperationConfirm : public GuiFileOperation
   {
      public:
         GuiFileOperationConfirm();

         GuiFileOperationConfirm(uint16 seqId, uint16 type,
               class FileLocation* location, char* additional)
            : GuiFileOperation(seqId), m_type(type),
              m_location(location), m_additional(additional) {};

         ~GuiFileOperationConfirm();
         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         virtual const uint16 typeId() const { return OPERATION_CONFIRM; }

         const char *additional() const { return m_additional; }
         const class FileLocation *location() const { return m_location; }
         const uint16 type() const { return m_type; }
      private:

         /**
          * Confirmation type: overwrite, delete, delete all...
          */
         uint16 m_type;

         /**
          * Location that the operation will act upon.
          */
         class FileLocation* m_location;

         /**
          * Additional information to show to user.
          */
         char *m_additional;
   };
   class GuiFileOperationConfirmReply : public GuiFileOperation
   {
      public:
         GuiFileOperationConfirmReply();
         GuiFileOperationConfirmReply(uint16 seqId, uint16 command,
               class FileLocation* location)
            : GuiFileOperation(seqId),
              m_command(command), m_location(location) {};
         GuiFileOperationConfirmReply(class Buffer* buf);
         ~GuiFileOperationConfirmReply();
         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         virtual const uint16 typeId() const { return OPERATION_CONFIRM_REPLY; }

         const uint16 command() const { return m_command; }
         const class FileLocation *location() const { return m_location; }
      private:
         /**
          * Answer: yes, no
          */
         uint16 m_command;

         /**
          * Location that the operation should have acted upon.
          */
         class FileLocation* m_location;
   };
   class GuiFileOperationCommand : public GuiFileOperation
   {
      public:
         GuiFileOperationCommand()
            : GuiFileOperation(0), m_command(0) {};
         GuiFileOperationCommand(uint16 seqId, uint16 command)
            : GuiFileOperation(seqId), m_command(command) {};
         GuiFileOperationCommand(class Buffer* buf);
         ~GuiFileOperationCommand();
         virtual void writeToBuffer(class Buffer* buf) const;
         virtual void readFromBuffer(class Buffer* buf);

         virtual const uint16 typeId() const { return OPERATION_COMMAND; }

         const uint16 command() const { return m_command; }
      private:
         /**
          * Command: Load, save, delete...
          */
         uint16 m_command;
   };
}

#endif
