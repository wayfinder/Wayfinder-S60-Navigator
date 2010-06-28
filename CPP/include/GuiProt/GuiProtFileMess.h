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
 * Used from Nav2 to get user-response.
 */

#ifndef GUI_PROT_FILE_MESS_H
#define GUI_PROT_FILE_MESS_H
namespace isab{
   /**
    *   File select message.
    *   Used to let GUI show the user a list of files
    *   which the user then can select from.
    *
    */
   class GuiProtFileMess : public GuiProtMess
   {
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GuiProtFileMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       */
      GuiProtFileMess(class GuiFileOperation *mess);

      /** Destructor. Doesn't free any resources. */
      virtual ~GuiProtFileMess();


      /** GuiProtMess interface methods. */
      //@{ 
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();

      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return File operation object.
       */
      class GuiFileOperation *getFileOperation() const;

   protected:
      /**
       * Member variables.
       */
      //@{
      class GuiFileOperation *m_fileOp;
      //@}


   }; // SelectFileGuiMess


}
#endif
