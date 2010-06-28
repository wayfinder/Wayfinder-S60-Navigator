/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SETTINGSLISTBOX_H
#define SETTINGSLISTBOX_H

// INCLUDES
#include <aknsettingitemlist.h>

// FORWARD DECLARATIONS
class CSettingsData;
namespace isab{
   class Log;
}
// CLASS DECLARATION

/**
* CSettingsContainer  container control class.
*/
class CSettingsListBox : public CAknSettingItemList 
{
public:
   CSettingsListBox(TBool ReleaseVersion, TBool OneLanguageVersion,
                    isab::Log* aLog, CSettingsData *aData) : 
      iData(aData),
      iRelease(ReleaseVersion), 
      iOneLanguage(OneLanguageVersion), 
      iLog(aLog) {}
   
   CAknSettingItem* CreateSettingItemL( TInt identifier );
   
   TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );
   
private:
   
   void SizeChanged();

private:
   CSettingsData* iData;
   TBool iRelease;
   TBool iOneLanguage;
   isab::Log* iLog;
};


#endif

// End of File
