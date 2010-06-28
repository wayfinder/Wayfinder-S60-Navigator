/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDERAPP_H
#define WAYFINDERAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS

// UID of the application
extern const TUid KUidWayFinder;

// CLASS DECLARATION

/**
* CWayFinderApp application class.
* Provides factory to create concrete document object.
* 
*/
class CWayFinderApp : public CAknApplication
{
    
public: // Functions from base classes
   /**
    * From CEikApplication, determins the name of the resource file
    * to be loaded. Useful way to override which language resources
    * are loaded.
    * @return The filename to be loaded.
    */
   virtual TFileName ResourceFileName() const;
private:
   
   /**
    * From CApaApplication, creates CWayFinderDocument document object.
    * @return A pointer to the created document object.
    */
   CApaDocument* CreateDocumentL();

   /**
    * From CApaApplication, returns application's UID (KUidWayFinder).
    * @return The value of KUidWayFinder.
    */
   TUid AppDllUid() const;

   // New function - helper for ResourceFileName();
   TFileName AttemptResourceNameL(RFs & fs, const TDesC & startName, const TDesC & sysFileName) const;

};

#endif

// End of File

