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
#include "wayfinder.hrh"
/* #include "WayFinderConstants.h" */
#include "LanguageSettingItem.h"
#include "RsgInclude.h"
#include <barsread.h>
#include <commdb.h>

#include "SettingsData.h"

#include "LanguageList.hrh"
#include "LanguageList.h"

CLanguageSettingItem::CLanguageSettingItem(TInt aResourceId, class CSettingsData* aData, TInt& aValue) : 
   CAknEnumeratedTextPopupSettingItem(aResourceId, aValue),
   iData(aData)
{   
}


void AddEnumeratedLanguageL(CArrayPtr<CAknEnumeratedText>& aArray,
                            TInt aEnum, HBufC* aText)
{
   CAknEnumeratedText* text = new (ELeave) CAknEnumeratedText(aEnum, aText);
   CleanupStack::PushL(text);
   aArray.AppendL(text);
   CleanupStack::Pop(text);
}


// ================= MEMBER FUNCTIONS =========================================

// ----------------------------------------------------------------------------
// CLanguageSettingItem::CompleteConstructionL()
// Update the Language list
// ----------------------------------------------------------------------------
void CLanguageSettingItem::CompleteConstructionL()
{
   CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

   class CLanguageList * langList = 
      CLanguageList::NewLC(R_WAYFINDER_LANGUAGE_LIST_NEW,
                           *(iData->m_resourceNoExt),
                           *(iData->m_audioBasePath),
                           *(iData->m_audioFormatPrefix),
                           iData->iAllowedLangs
                           );

   CArrayPtr<CAknEnumeratedText> *langs = 
      new (ELeave) CArrayPtrSeg<CAknEnumeratedText>(10);
   CleanupStack::PushL(langs);

   // Left empty to use texts from langs.
   CArrayPtr<HBufC> *poppedUp = new(ELeave) CArrayPtrSeg<HBufC>(10);
   CleanupStack::PushL(poppedUp);

   class TParse * parser = new (ELeave) TParse();
   CleanupStack::PushL(parser);

   class RFs fs;
   fs.Connect();
   CleanupClosePushL(fs);

   // Remember the first shown value
   TInt symbian_code_for_first_element = -1;

   // Only show languages for which we have an .rXX file.
   // Optionally only show those with audio as well.
   for (TInt i = 0; i < langList->Count(); ++i) {
      struct CLanguageList::TWfLanguageList & elem = langList->Get(i);
      
      if (! elem.isAllowed) {
         elem.handle = -1;
         continue;
      }

      if (!iData->m_showLangsWithoutResource) {
         if (!elem.hasRXX) {
            elem.handle = -1;
            continue;
         }
      }

      if ( ! elem.hasAudio ) {
         // Not found, ignore or mark this language
         if (iData->m_showLangsWithoutVoice) {
            _LIT(KStar, "*");
            HBufC * name = HBufC::NewLC(elem.fullname.Length()+1);
            TPtr name_ptr = name->Des();
            name_ptr.Copy(KStar);
            name_ptr.Append(elem.fullname);  
            AddEnumeratedLanguageL(*langs, elem.symbianCode, name);
            CleanupStack::Pop(name); 
            name = NULL;
            if (symbian_code_for_first_element < 0) {
               symbian_code_for_first_element = elem.symbianCode;
            }
         } else {
            elem.handle = -1;
         }
         continue; //continue for loop
      }

      HBufC * name = elem.fullname.AllocLC();
      AddEnumeratedLanguageL(*langs, elem.symbianCode, name);
      CleanupStack::Pop(name);  // FIXME - Is this right? Does
      name = NULL;              // AppendL take over ownership of
                                // the name?
      if (symbian_code_for_first_element < 0) {
         symbian_code_for_first_element = elem.symbianCode;
      }
   }

   fs.Close();
   CleanupStack::PopAndDestroy(/*fs*/); 

   SetEnumeratedTextArrays(langs, poppedUp);

   CleanupStack::PopAndDestroy(parser);

   CleanupStack::Pop(poppedUp);
   CleanupStack::Pop(langs);

   // Does the (old) specified value still exist? Default to 
   // the first element which is "system default" unless prohibited.
   if (IndexFromValue(ExternalValue()) < 0) {
      SetInternalValue(symbian_code_for_first_element);
      SetExternalValue(symbian_code_for_first_element);
   }
   CleanupStack::PopAndDestroy(langList);
}

// End of File
