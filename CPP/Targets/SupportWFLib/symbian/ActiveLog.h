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

#include <e32base.h>
#ifndef LOG_ACTIVE_H
#define LOG_ACTIVE_H

namespace isab {
   class Log;
   class LogMaster;
}

class CActiveLog : public CActive
{
   //no copy construction allowed
   CActiveLog(const class CActiveLog&);
   //no assignments allowed.
   class CActiveLog& operator=(const class CActiveLog&);
protected:
   /** @name Constructors and destructor. */
   //@{
   /**
    * Constructor that sets the active object priority.
    * @param aPriority the active object priority.
    * @param aPrefix   the prefix used in Log objects.
    */
   CActiveLog(enum TPriority aPriority, const char* aPrefix);
   /**
    * Constructor that sets the active object priority to standard.
    * @param aPrefix   the prefix used in Log objects.
    */
   CActiveLog(const char* aPrefix);
   /** Virtual destructor. */
   virtual ~CActiveLog();
   //@}
   /**
    * If iLog is not NULL, changes the LogMaster of all CActiveLog
    * objects returned by SubLogArrayLC.
    */
   void UpdateLogMastersL();
   /**
    * Returns an array of pointers to other CActiveLog objects. 
    * The default implementation returns NULL. 
    * @return an array of pointers to other CActiveLog objects. 
    *         It may return NULL, but the NULL-pointer must have been
    *         pushed on the cleanupstack.
    *         The array may contain pointers that are NULL.
    */
   virtual CArrayPtr<CActiveLog>* SubLogArrayLC();
public:
   /**
    * Deletes the current log object and creates a new one using the
    * new LogMaster.
    * @param aLogMaster the new LogMaster.
    */
   void SetLogMasterL(class isab::LogMaster* aLogMaster);
private:
   /**
    * Sets the prefix used by Log. The new Prefix will not have any
    * effect until a new Log is created.
    * @param aPrefix the new prefix. If it is longer than 32
    *                characters it will be truncated.
    */
   void SetPrefix(const char* aPrefix);
   /** @name Member varialbles. */
   //@{
   /** The Log prefix. */
   char iPrefix[32];
protected:
   /** The log object. */
   class isab::Log* iLog;
   //@}
};
#endif
