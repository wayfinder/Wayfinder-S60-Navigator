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
#include "WayFinderConstants.h"
#include "VoiceSyntaxSettingItem.h"
#include "WayFinder.rsg"
#include <barsread.h>
#include <commdb.h>

#include "LanguageList.hrh"
#include "LanguageList.h"



// ================= MEMBER FUNCTIONS =========================================

CVoiceSyntaxSettingItem::~CVoiceSyntaxSettingItem()
{
   delete iVoicesFullName;
}

TPtrC CVoiceSyntaxSettingItem::ExtractVoice(TDesC & fullname)
{
   _LIT(KWfAudioPrefix,"WFAudioEN_"); 
   TParsePtrC parser(fullname);
   TPtrC tmp = parser.Path();
   TParsePtrC p2(tmp.Left(tmp.Length()-1));  // Strip trailing backslash
   TPtrC dirname = p2.Name();
   if (dirname.Length() > KWfAudioPrefix().Length()) {
      return dirname.Right(dirname.Length()-KWfAudioPrefix().Length());
   }
   return TPtrC(dirname);
}

TPtrC CVoiceSyntaxSettingItem::ExtractSyntax(TDesC & fullname)
{
   TParsePtrC parser(fullname);
   return parser.Name();
}

// ----------------------------------------------------------------------------
// CVoiceSyntaxSettingItem::CompleteConstructionL()
// Update the Language list
// ----------------------------------------------------------------------------

void CVoiceSyntaxSettingItem::CompleteConstructionL()
{
   CAknEnumeratedTextPopupSettingItem::CompleteConstructionL();

   CArrayPtr<CAknEnumeratedText> * voices = new(ELeave) CArrayPtrFlat<CAknEnumeratedText>(10);
   CleanupStack::PushL(voices);
   CArrayPtr<HBufC> * poppedUp = new(ELeave) CArrayPtrFlat<HBufC>(1); // Left empty to use texts from langs.
   CleanupStack::PushL(poppedUp);

   CLanguageList * langList = CLanguageList::NewLC(R_WAYFINDER_LANGUAGE_LIST_NEW);
   CLanguageList::TWfLanguageList & lang = langList->FindBySymbianCode(iLangValue);
   iVoicesFullName = lang.GetAudioSyntaxesLC();
   CleanupStack::Pop(iVoicesFullName);

   // Set to the first item for now. This will be updated later on, 
   // if a match to the provided aVoice is found.
   SetInternalValue(0);

   // List all available voices
   TInt i;
   for (i = 0; i < iVoicesFullName->MdcaCount(); ++i) {
      TPtrC fullname = iVoicesFullName->MdcaPoint(i);
      TPtrC voiceName = ExtractVoice(fullname);
      TPtrC syntaxName = ExtractSyntax(fullname);
      HBufC * name = HBufC::NewLC(voiceName.Length()+1+syntaxName.Length());
      name->Des().Format(_L("%S %S"), &voiceName, &syntaxName);
      if (0 == fullname.Compare(iVoice)) {
         // Found the user selection - update the internal pointer.
         SetInternalValue(i);
         SetExternalValue(i);
         SetSelectedIndex(i);
      }
      voices->AppendL( new(ELeave) CAknEnumeratedText(i, name));
      CleanupStack::Pop(name);  // FIXME - Is this right? Does CAknEnumeratedText take over ownership of the name? 
   }

   CleanupStack::PopAndDestroy(langList);

   SetEnumeratedTextArrays(voices, poppedUp);

   CleanupStack::Pop(poppedUp);
   CleanupStack::Pop(voices);

   //// Does the (old) specified value still exist? Default to 
   //// "always ask" otherwise.
   //if (IndexFromValue(ExternalValue()) < 0) {
   //   SetInternalValue(0);
   //   SetExternalValue(0);
   //}

}

void CVoiceSyntaxSettingItem::StoreL() {
   CAknEnumeratedTextPopupSettingItem::StoreL();
   if (iTempValue >= 0 && iTempValue < iVoicesFullName->MdcaCount()) {
      iVoice.Copy(iVoicesFullName->MdcaPoint(iTempValue));
   }
}

// End of File
