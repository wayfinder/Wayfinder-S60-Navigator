/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef NAV2_CLIENT_SERIES60_V3
#include <eikstart.h>
#endif

#include <bautils.h>

// INCLUDE FILES
#include    "WayFinderApp.h"
#include    "WayFinderDocument.h"
#include    "WayFinderConstants.h"
#include    "WayFinderSettings.h"
#include "ResourceFileName.h"

const TUid KUidWayFinder = { WayFinder_uid3 };

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CWayFinderApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CWayFinderApp::AppDllUid() const
{
   return KUidWayFinder;
}

   
// ---------------------------------------------------------
// CWayFinderApp::CreateDocumentL()
// Creates CWayFinderDocument object
// ---------------------------------------------------------
//
CApaDocument* CWayFinderApp::CreateDocumentL()
{
   return CWayFinderDocument::NewL( *this );
}

// ---------------------------------------------------------
// CWayFinderApp::ResourceFileName()
// Determins the name of the resource file to be loaded
// by PreDocConstructL().
// ---------------------------------------------------------
//

// This is the default for symbian applications.
#undef USE_NO_SPECIAL_LANG
#ifdef ONLY_PHONE_LANG_ROM_BUILD
# undef IF_SPECIAL_USE_PREF_FILE
#else
# if defined (USE_ONLY_ONE_LANGUAGE)
// This means that the else of the ifdef below is hit,
// and the .rsc file is used instead of any other.
#  undef IF_SPECIAL_USE_PREF_FILE
# else
// This means that the application tries to read the value
// in the lang.txt file and use that to determine which
// language resource file to use (.r01 .r02 etc)
#  define IF_SPECIAL_USE_PREF_FILE
# endif
#endif

TFileName CWayFinderApp::ResourceFileName() const
{
#ifdef USE_NO_SPECIAL_LANG
   return CAknApplication::ResourceFileName();
#elif defined IF_SPECIAL_USE_PREF_FILE
   return FindResourceFileName(CAknApplication::ResourceFileName());
#elif defined ONLY_PHONE_LANG_ROM_BUILD
   class RFs fs;
   if(KErrNone != fs.Connect()){   
      // Not good at all, nothing to do. The program will most probably crash like hell!
      return KNullDesC();
   }
   TFileName rscFileName = CAknApplication::ResourceFileName();
   TLanguage lang;
   BaflUtils::NearestLanguageFile(fs, rscFileName, lang);
   fs.Close();
   return rscFileName;
#else
   return RscResourceFileName(CAknApplication::ResourceFileName());
#endif
}

// ================= OTHER EXPORTED FUNCTIONS ==============
//
// ---------------------------------------------------------
// NewApplication() 
// Constructs CWayFinderApp
// Returns: created application object
// ---------------------------------------------------------
//
EXPORT_C CApaApplication* NewApplication()
{
   return new CWayFinderApp;
}

#ifdef NAV2_CLIENT_SERIES60_V3

/*
The function is called by the framework immediately after it has loaded the 
application's EXE. E32Main() contains the program's start up code, 
the entry point for an EXE.
*/
GLDEF_C TInt E32Main()
{
   return EikStart::RunApplication(NewApplication);
}

#else


// ---------------------------------------------------------
// E32Dll(TDllReason) 
// Entry point function for EPOC Apps
// Returns: KErrNone: No error
// ---------------------------------------------------------
//
GLDEF_C TInt E32Dll(TDllReason)
{
   return KErrNone;
}

#endif

// End of File  
