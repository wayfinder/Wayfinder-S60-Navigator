/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include <e32std.h>  //descriptors
#include <coemain.h> //for CCoeEnv
//#include "WayFinder.rsg" //R_WAYFINDER_TWO_CAPITAL_CHARS_LANG_CODE

#include "WFSymbianUtil.h"
#include "CleanupSupport.h"
#include "Quality.h"
#include "NewSyntax.h"
#include "NavServerComEnums.h"
#include "AudioCtrlEn.h"
#include "AudioCtrlSv.h"
#include "AudioCtrlDe.h"
#include "AudioCtrlDa.h"
#include "AudioCtrlIt.h"
#include "AudioCtrlFr.h"
#include "AudioCtrlEs.h"
#include "AudioCtrlNl.h"
#include "AudioCtrlFi.h"
#include "AudioCtrlNo.h"
#include "AudioCtrlPt.h"
#include "AudioCtrlUs.h"
#include "AudioCtrlHu.h"
#include "AudioCtrlScript.h"
#include "AudioCtrlPl.h"
#include "AudioCtrlCs.h"
#include "AudioCtrlSl.h"
#include "AudioCtrlTr.h"
#include "AudioCtrlNone.h"

namespace{
   using namespace isab;
   using namespace isab::NavServerComEnums;

   ///Maps two-letter language codes to server protocol language
   ///constants and AudioSyntax factory functions.
   struct CodeEnumMap {
      ///Pointer to AudioSyntax factory functions.
      typedef class AudioCtrlLanguage *(*factoryFunc)();

      ///Pointer to the two-letter language code.
      ///This is sort of a hack to appease the visual compiler. It
      ///wouldn't accept references, the initialization of the
      ///CodeEnumMap array below didn't work with references. Stupid
      ///MS.
      const TText* twoCharCode;
      ///The server protocol language constant.
      languageCode langCode;
      ///Pointer to the factory function for this language's audiosyntax.
      factoryFunc factory;
   };

   ///A list of supported languages, and the connection between
   ///two-letter language codes, server protocol language codes and
   ///audio syntax factory functions.
   const static struct CodeEnumMap languageMap[] = { 
      { _S("DA"), DANISH,              AudioCtrlLanguageScript::New },
      { _S("DE"), GERMAN,              AudioCtrlLanguageScript::New },
      { _S("EN"), ENGLISH,             AudioCtrlLanguageScript::New },
      { _S("IT"), ITALIAN,             AudioCtrlLanguageScript::New },
      { _S("FR"), FRENCH,              AudioCtrlLanguageScript::New },
      { _S("ES"), SPANISH,             AudioCtrlLanguageScript::New },
      { _S("SV"), SWEDISH,             AudioCtrlLanguageScript::New },
      { _S("NL"), DUTCH,               AudioCtrlLanguageScript::New },
      { _S("FI"), FINNISH,             AudioCtrlLanguageScript::New },
      { _S("NO"), NORWEGIAN,           AudioCtrlLanguageScript::New },
      { _S("PT"), PORTUGUESE,          AudioCtrlLanguageScript::New },
      { _S("HU"), HUNGARIAN,           AudioCtrlLanguageScript::New },
      { _S("US"), AMERICAN_ENGLISH,    AudioCtrlLanguageScript::New },
      { _S("CS"), CZECH,               AudioCtrlLanguageScript::New },
      { _S("PL"), POLISH,              AudioCtrlLanguageScript::New },

      { _S("SK"), SLOVAK,              AudioCtrlLanguageScript::New },
      { _S("RU"), RUSSIAN,             AudioCtrlLanguageScript::New },
      { _S("EL"), GREEK,               AudioCtrlLanguageScript::New },
      { _S("SL"), SLOVENIAN,           AudioCtrlLanguageScript::New },
      { _S("TR"), TURKISH,             AudioCtrlLanguageScript::New },
      { _S("AR"), ARABIC,              AudioCtrlLanguageScript::New },
   };
   ///An iterator type for the languageMap.
   typedef struct CodeEnumMap* LangIterator;
   ///A constant iterator type for the languageMap.
   typedef const struct CodeEnumMap* const_LangIterator;
   ///The number of items in the languageMap.
   const size_t numLang = sizeof(languageMap)/sizeof(CodeEnumMap);
   ///The start iterator of languageMap.
   const const_LangIterator beginLang = languageMap;
   ///The end iterator of languageMap.
   const const_LangIterator endLang = languageMap + numLang;
}

class isab::AudioCtrlLanguage* NewSyntaxLC(TInt32& nav2lang, TInt aLangCodeRes)
{
   // Decide which audio system to use.
   AudioCtrlLanguage *audioSyntax = NULL;
   TBuf<32> languageCode;
   CCoeEnv::Static()->ReadResource(languageCode, aLangCodeRes);
   const_LangIterator lang;
   for(lang = beginLang; lang != endLang; ++lang){
      if(languageCode == TPtrC(lang->twoCharCode)){
         break;
      }
   }
   if(lang != endLang){
      nav2lang = lang->langCode;
      User::LeaveIfNull(audioSyntax = lang->factory());
   } else {
      User::LeaveIfNull(audioSyntax = new AudioCtrlLanguageNone());
   }
   CleanupStack::PushL(TCleanupItem(CleanupClass<AudioCtrlLanguage>::Cleanup,
                                    audioSyntax));
   return audioSyntax;
}

//TODO: this function is not exported through the NewSyntax.h file. A
//function with this signature should be added, and the real way to do
//this would be to let the old signature be a wrapper for this one,
//which should have the contents of the old one.
class isab::AudioCtrlLanguage* NewSyntaxLC(enum languageCode aNav2Lang, 
                                           TInt aLangCodeRes) 
{
   TInt32 tmpcode = 0;
   class AudioCtrlLanguage* lang = NewSyntaxLC(tmpcode, aLangCodeRes);
   aNav2Lang = languageCode(tmpcode);
   return lang;
}

class isab::AudioCtrlLanguage* NewSyntaxLC(TInt32& nav2lang, const TDesC& aTwoCharCode)
{
   // Decide which audio system to use.
   AudioCtrlLanguage *audioSyntax = NULL;
   TBuf<32> languageCode;
   languageCode.Copy(aTwoCharCode);
   const_LangIterator lang;
   for(lang = beginLang; lang != endLang; ++lang){
      if(languageCode == TPtrC(lang->twoCharCode)){
         break;
      }
   }
   if(lang != endLang){
      nav2lang = lang->langCode;
      User::LeaveIfNull(audioSyntax = lang->factory());
   } else {
      User::LeaveIfNull(audioSyntax = new AudioCtrlLanguageNone());
   }
   CleanupStack::PushL(TCleanupItem(CleanupClass<AudioCtrlLanguage>::Cleanup,
                                    audioSyntax));
   return audioSyntax;
}
