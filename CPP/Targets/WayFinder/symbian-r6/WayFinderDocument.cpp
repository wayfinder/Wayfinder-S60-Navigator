/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES

#include <aknviewappui.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include <msvapi.h>

#include <arch.h>
#include "WayFinderConstants.h"
#include "RouteEnums.h"
#include "TimeOutNotify.h"
#include "GuiProt/GuiProtEnums.h"
#include "WayFinderAppUi.h"

#include "WayFinderDocument.h"
#include "memlog.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CWayFinderDocument::CWayFinderDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
{
}

// destructor
CWayFinderDocument::~CWayFinderDocument()
{
   LOGDEL(iLog);
#ifdef CREATE_LOG_AND_MASTER
   LOGDEL(iLogMaster);
#endif
   LOGDEL(this);
   delete iLog;
#ifdef CREATE_LOG_AND_MASTER
   delete iLogMaster;
#endif
}

// EPOC default constructor can leave.
void CWayFinderDocument::ConstructL()
{
#ifdef CREATE_LOG_AND_MASTER
   iLogMaster = new (ELeave) LogMaster();
   iLogMaster->setDefaultOutput("file:e:\\guilog.txt");
   iLog = new (ELeave) isab::Log("WayFinder", Log::LOG_ALL, iLogMaster);
   LOGNEW(this, CWayFinderDocument);
   LOGNEW(iLogMaster, LogMaster);
   LOGNEW(iLog, isab::Log);
#else
   iLog = NULL;
#endif
}

// Two-phased constructor.
CWayFinderDocument* CWayFinderDocument::NewL( CEikApplication& aApp ) // CWayFinderApp reference
{
   CWayFinderDocument* self = new (ELeave) CWayFinderDocument( aApp );
   CleanupStack::PushL( self );
   self->ConstructL();
   CleanupStack::Pop();

   return self;
}


    
// ----------------------------------------------------
// CWayFinderDocument::CreateAppUiL()
// constructs CWayFinderAppUi
// ----------------------------------------------------
//
CEikAppUi* CWayFinderDocument::CreateAppUiL()
{
   CWayFinderAppUi* tmp =  new (ELeave) CWayFinderAppUi(iLog);
   LOGNEW(tmp, CWayFinderAppUi);
   return tmp;
   
}

// End of File  
