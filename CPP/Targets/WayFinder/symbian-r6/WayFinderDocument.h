/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDERDOCUMENT_H
#define WAYFINDERDOCUMENT_H

// INCLUDES
#include <akndoc.h>
#include "Log.h"   
#include "memlog.h"
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  CWayFinderDocument application class.
*/
class CWayFinderDocument : public CAknDocument
{

public: // Constructors and destructor

   /**
    * Two-phased constructor.
    */
   static CWayFinderDocument* NewL(CEikApplication& aApp);

   /**
    * Destructor.
    */
   virtual ~CWayFinderDocument();

public: // New functions

protected:  // New functions

protected:  // Functions from base classes

private:

   /**
    * EPOC default constructor.
    */
   CWayFinderDocument(CEikApplication& aApp);
   
   void ConstructL();

   /**
    * From CEikDocument, create CWayFinderAppUi "App UI" object.
    */
   CEikAppUi* CreateAppUiL();

#ifdef CREATE_LOG_AND_MASTER
   isab::LogMaster *iLogMaster;
#endif
   isab::Log* iLog;

};

#endif

// End of File

