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

#include "GuiFileOperation.h"

#include "Buffer.h"
#include "nav2util.h"

namespace isab
{

/** FileLocation **/
void
FileLocation::readFromBuffer(class Buffer* buf)
{
   m_type = buf->readNextUnaligned32bit();
   m_name = buf->getNextCharStringAlloc();
   m_dir = buf->getNextCharStringAlloc();
   m_dirname = buf->getNextCharStringAlloc();
}
void
FileLocation::writeToBuffer(class Buffer* buf)
{
   buf->writeNextUnaligned32bit(m_type);
   buf->writeNextCharString(m_name);
   buf->writeNextCharString(m_dir);
   buf->writeNextCharString(m_dirname);
}
FileLocation::FileLocation(const FileLocation& org)
{
   m_type = org.m_type;
   m_name = strdup_new(org.m_name);
   m_dir = strdup_new(org.m_dir);
   m_dirname = strdup_new(org.m_dirname);
}

/** GuiFileOperation **/
void
GuiFileOperation::readFromBuffer(class Buffer* buf)
{
   m_seqId = buf->readNextUnaligned16bit();
}
void
GuiFileOperation::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(m_seqId);
}
GuiFileOperation::~GuiFileOperation()
{
}

GuiFileOperation*
GuiFileOperation::createFromBuffer(Buffer* buf)
{
   class GuiFileOperation* fileOp = 0;

   size_t readPos = buf->getReadPos();

   uint16 type = buf->readNextUnaligned16bit();
   switch (type) {
      case OPERATION_SELECT:
         fileOp = new GuiFileOperationSelect();
         break;
      case OPERATION_SELECT_REPLY:
         fileOp = new GuiFileOperationSelectReply();
         break;
      case OPERATION_CONFIRM:
         fileOp = new GuiFileOperationConfirm();
         break;
      case OPERATION_CONFIRM_REPLY:
         fileOp = new GuiFileOperationConfirmReply();
         break;
      case OPERATION_COMMAND:
         fileOp = new GuiFileOperationCommand();
         break;
   }
   buf->setReadPos(readPos);

/*    if (fileOp) { */
      fileOp->readFromBuffer(buf);
      return fileOp;
/*    } else { */
/*       return 0; */
/*    } */
}

/** GuiFileOperationSelect **/
GuiFileOperationSelect::GuiFileOperationSelect()
   : GuiFileOperation(0),
     m_lastUsedIndex(0), m_command(0),
     m_fileName(0), m_locations(0)
{
   m_locations = new LocationVector();
   m_locations->reserve(4);
}
GuiFileOperationSelect::~GuiFileOperationSelect()
{
   if (m_locations) {
      LocationVector::iterator it = m_locations->begin();
      while (it != m_locations->end()) {
         delete (*it);
         it++;
      }
      m_locations->clear();
   }
   delete m_locations;
   delete[] m_fileName;
}
void
GuiFileOperationSelect::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(OPERATION_SELECT);

   GuiFileOperation::writeToBuffer(buf);

   buf->writeNextUnaligned16bit(m_lastUsedIndex);
   buf->writeNextUnaligned16bit(m_command);
   if (m_fileName) {
      buf->writeNextCharString(m_fileName);
   } else {
      buf->writeNextCharString("");
   }

   buf->writeNextUnaligned32bit(m_locations->size());

   LocationVector::iterator it = m_locations->begin();

   while (it != m_locations->end()) {
      FileLocation* item = *it;
      item->writeToBuffer(buf);
      it++; 
   }
}
void
GuiFileOperationSelect::readFromBuffer(class Buffer* buf)
{
   int16 unused = buf->readNextUnaligned16bit();
   if (unused != OPERATION_SELECT) {
      /* Error. */
   }
   GuiFileOperation::readFromBuffer(buf);

   m_lastUsedIndex = buf->readNextUnaligned16bit();
   m_command = buf->readNextUnaligned16bit();

   m_fileName = buf->getNextCharStringAlloc();

   uint32 num_items = buf->readNextUnaligned32bit();

   while (num_items > 0) {
      FileLocation* item = new FileLocation();
      item->readFromBuffer(buf);
      m_locations->push_back(item);
      num_items--; 
   }
}

/** GuiFileOperationSelectReply **/
GuiFileOperationSelectReply::GuiFileOperationSelectReply()
   : GuiFileOperation(0),
      m_selected(0), m_command(0), m_fileName(0)
{
   m_location = new FileLocation();
}
GuiFileOperationSelectReply::~GuiFileOperationSelectReply()
{
   delete m_location;
   delete[] m_fileName;
}
void
GuiFileOperationSelectReply::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(OPERATION_SELECT_REPLY);

   GuiFileOperation::writeToBuffer(buf);

   buf->writeNextUnaligned16bit(m_selected);
   buf->writeNextUnaligned16bit(m_command);
   m_location->writeToBuffer(buf);

   buf->writeNextCharString(m_fileName);
}
void
GuiFileOperationSelectReply::readFromBuffer(class Buffer* buf)
{
   int16 unused = buf->readNextUnaligned16bit();
   if (unused != OPERATION_SELECT_REPLY) {
      /* Error. */
   }

   GuiFileOperation::readFromBuffer(buf);

   m_selected = buf->readNextUnaligned16bit();
   m_command = buf->readNextUnaligned16bit();
   m_location->readFromBuffer(buf);

   m_fileName = buf->getNextCharStringAlloc();
}

/** GuiFileOperationConfirm **/
GuiFileOperationConfirm::GuiFileOperationConfirm()
   : GuiFileOperation(0),
      m_type(0), m_additional(0)
{
   m_location = new FileLocation();
}
GuiFileOperationConfirm::~GuiFileOperationConfirm()
{
   delete m_location;
   delete[] m_additional;
}
void
GuiFileOperationConfirm::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(OPERATION_CONFIRM);

   GuiFileOperation::writeToBuffer(buf);

   buf->writeNextUnaligned16bit(m_type);
   m_location->writeToBuffer(buf);

   buf->writeNextCharString(m_additional);
}
void
GuiFileOperationConfirm::readFromBuffer(class Buffer* buf)
{
   int16 unused = buf->readNextUnaligned16bit();
   if (unused != OPERATION_CONFIRM) {
      /* Error. */
   }

   GuiFileOperation::readFromBuffer(buf);

   m_type = buf->readNextUnaligned16bit();
   m_location->readFromBuffer(buf);

   m_additional = buf->getNextCharStringAlloc();
}

/** GuiFileOperationConfirmReply **/
GuiFileOperationConfirmReply::GuiFileOperationConfirmReply()
   : GuiFileOperation(0),
      m_command(0)
{
   m_location = new FileLocation();
}
GuiFileOperationConfirmReply::~GuiFileOperationConfirmReply()
{
   delete m_location;
}
void
GuiFileOperationConfirmReply::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(OPERATION_CONFIRM_REPLY);

   GuiFileOperation::writeToBuffer(buf);

   buf->writeNextUnaligned16bit(m_command);
   m_location->writeToBuffer(buf);
}
void
GuiFileOperationConfirmReply::readFromBuffer(class Buffer* buf)
{
   int16 unused = buf->readNextUnaligned16bit();
   if (unused != OPERATION_CONFIRM_REPLY) {
      /* Error. */
   }

   GuiFileOperation::readFromBuffer(buf);

   m_command = buf->readNextUnaligned16bit();
   m_location->readFromBuffer(buf);
}
/** GuiFileOperationCommand **/
GuiFileOperationCommand::~GuiFileOperationCommand()
{
}
void
GuiFileOperationCommand::writeToBuffer(class Buffer* buf) const
{
   buf->writeNextUnaligned16bit(OPERATION_COMMAND);

   GuiFileOperation::writeToBuffer(buf);

   buf->writeNextUnaligned16bit(m_command);
}
void
GuiFileOperationCommand::readFromBuffer(class Buffer* buf)
{
   int16 unused = buf->readNextUnaligned16bit();
   if (unused != OPERATION_COMMAND) {
      /* Error. */
   }

   GuiFileOperation::readFromBuffer(buf);

   m_command = buf->readNextUnaligned16bit();
}

}

