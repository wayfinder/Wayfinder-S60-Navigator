/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TileMapConfig.h"

#include "LangTypes.h"

#ifndef __SYMBIAN32__

#include <map>

LangTypes::strMap_t LangTypes::c_stringAsLanguageShort = LangTypes::strMap_t();

LangTypes::strMap_t LangTypes::c_stringAsLanguageFull  = LangTypes::strMap_t();

/**
 * Initialize the map of strings as language. Done once.
 */
bool 
LangTypes::stringAsLanguageInitialized = LangTypes::initStringAsLanguage();

bool
LangTypes::initStringAsLanguage()
{   
   // Fill in the maps using the array.
   for (uint32 i = 0; i < LangTypes::nbrLanguages; i++) {
         c_stringAsLanguageShort.insert(
            make_pair(languageAsString[i][0], LangTypes::language_t(i)));
         c_stringAsLanguageFull.insert(
            make_pair(languageAsString[i][1], LangTypes::language_t(i)));
         
   }
   return true;
}

#endif

// Note that the order must be the same as in LangTypes::language_t
// and that nbrLanguages must be last in the array.
// Only ASCII-characters are allowed. No едц:s etc.
const char* const LangTypes::languageAsString[][4] =
   { {"eng", "english",                     "en", "eng"},
     {"swe", "swedish",                     "sv", "swe"},
     {"ger", "german",                      "de", "ger"},
     {"dan", "danish",                      "da", "dan"},
     {"ita", "italian",                     "it", "ita"},
     {"dut", "dutch",                       "nl", "dut"},
     {"spa", "spanish",                     "es", "spa"},
     {"fre", "french",                      "fr", "fre"},
     {"wel", "welch",                       "cy", "wel"},
     {"fin", "finnish",                     "fi", "fin"},
     {"nor", "norwegian",                   "no", "nor"},
     {"por", "portuguese",                  "pt", "por"},
     {"ame", "english (us)",                "us", "eng"},
     {"cze", "czech",                       "cs", "cze"},
     {"alb", "albanian",                    "sq", "alb"},
     {"baq", "basque",                      "eu", "baq"},
     {"cat", "catalan",                     "ca", "cat"},
     {"fry", "frisian",                     "fy", "fry"},
     {"gle", "irish",                       "ga", "gai"},
     {"glg", "galician",                    "gl", "glg"},
     {"ltz", "letzeburgesch",               "",   "ltz"},
     {"roh", "raeto romance",               "rm", "roh"},
     {"scr", "serbo croatian",              "sh", "scr"},
     {"slv", "slovenian",                   "sl", "slv"},
     {"val", "valencian",                   "",   "val"},
     {"hun", "hungarian",                   "hu", "hun"},
     {"gre", "greek",                       "el", "gre"},
     {"pol", "polish",                      "pl", "pol"},
     {"slo", "slovak",                      "sk", "slo"},
     {"rus", "russian",                     "ru", "rus"},
     {"grl", "greek latin syntax",          "",   "grl"},
     {"invalidLanguage", "invalidLanguage", "xx", "xxx"},
     {"rul", "russian latin syntax",        "",   "rul"},
     {"tur", "turkish",                     "tr", "tur"},
     {"ara", "arabic",                      "ar", "ara"},
     {"nbrLanguages", "nbrLanguages",       "yy", "yyy"},
   };


const char*
LangTypes::getLanguageAsString(LangTypes::language_t langType, 
                               bool fullName)
{
   // Check the array size and add 1 since nbrLanguages is also in list.
   CHECK_ARRAY_SIZE( languageAsString, LangTypes::nbrLanguages + 1);
   if (langType < LangTypes::nbrLanguages) {
      return languageAsString[langType][fullName];
   } else {
      return languageAsString[LangTypes::invalidLanguage][fullName];
   }
}


const char* 
LangTypes::getLanguageAsISO639( LangTypes::language_t langType, bool two )
{
   if ( langType > LangTypes::nbrLanguages ) {
      langType = LangTypes::invalidLanguage;
   }
   return languageAsString[ langType ][ 2 + (two?0:1) ];
}

LangTypes::language_t
LangTypes::getISO639AsLanguage( const char* langType )
{
   int nbrLangs = LangTypes::nbrLanguages;

   int nbrChars = strlen( langType ); 
   
   // Only allow two or three letters.
   if ( nbrChars != 2 && nbrChars != 3 ) {
      return LangTypes::invalidLanguage;  
   }
   
   // The offset is the same as nbr of characters. 
   int offset = nbrChars;
  
   for( int i = 0; i < nbrLangs; ++i ) {
      if ( strcasecmp(languageAsString[i][offset], langType) == 0 ) {
         // Found a match - return
         return LangTypes::language_t(i);
      }
   }
   return LangTypes::invalidLanguage;  
}


LangTypes::language_t
LangTypes::getStringAsLanguage(const char* langType, bool fullName)
{
#ifndef __SYMBIAN32__
   const char* cmpString = langType;

   // Get the right map for the job
   const strMap_t& stringAsLanguage
      = fullName ? c_stringAsLanguageFull : c_stringAsLanguageShort;

   // Look in the map
   strMap_t::const_iterator it =
      stringAsLanguage.find(cmpString);
   
   if (it != stringAsLanguage.end()) {
      return it->second;
   } else {
      return LangTypes::invalidLanguage;
   }
#else
   int nbrLangs = LangTypes::nbrLanguages;
   // Use offset 1 for fullname and 0 for short name.
   int offset = fullName ? 1 : 0;
   for( int i = 0; i < nbrLangs; ++i ) {
      if ( strcasecmp(languageAsString[i][offset], langType) == 0 ) {
         // Found a match - return
         return LangTypes::language_t(i);
      }
   }
   return LangTypes::invalidLanguage;
#endif
}


LangTypes::language_t 
LangTypes::getNavLangAsLanguage( uint32 language ) {
   switch( language ) {
      case 0x00:
         return english;
      case 0x01:
         return swedish;
      case 0x02:
         return german;
      case 0x03:
         return danish;
      case 0x04:
         return finnish;
      case 0x05:
         return norwegian;
      case 0x06:
         return italian;
      case 0x07:
         return dutch;
      case 0x08:
         return spanish;
      case 0x09:
         return french;
      case 0x0a:
         return welch;
      case 0x0b:
         return portuguese;
      case 0x0c:
         return czech;
      case 0x0d:
         return american;
      case 0x0e:
         return hungarian;
      case 0x0f:
         return greek;
      case 0x10:
         return polish;
      case 0x11:
         return slovak;
      case 0x12:
         return russian;
      case 0x13:
         return slovenian;
      case 0x14:
         return turkish;
      case 0x15:
         return arabic;

      default:
         mc2log << warn << "LangTypes::getNavLangAsLanguage unknwon "
                << "language " << int(language) << " using english."
                << endl;
         return english;
   }
}
