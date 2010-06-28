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

#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFileMess.h"
#include "GuiFileOperation.h"

namespace isab{
   // GuiProtFileMess ///////////////////////////////////
   //
   GuiProtFileMess::GuiProtFileMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_fileOp = GuiFileOperation::createFromBuffer(buf);
   }

   GuiProtFileMess::~GuiProtFileMess()
   {
   }

   void
   GuiProtFileMess::deleteMembers()
   {
      delete m_fileOp;
      m_fileOp = NULL;
   }

   void 
   GuiProtFileMess::serializeMessData(Buffer* buf) const
   {
      m_fileOp->writeToBuffer(buf);
   } // serializeMessData 

   GuiProtFileMess::GuiProtFileMess(GuiFileOperation* fileOp) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::FILEOP_GUI_MESSAGE)
   {
      m_fileOp = fileOp;
   }

   GuiFileOperation*
   GuiProtFileMess::getFileOperation() const
   {
      return m_fileOp;
   }


}

