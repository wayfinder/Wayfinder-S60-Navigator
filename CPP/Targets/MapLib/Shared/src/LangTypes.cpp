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
     {"ltz", "letzeburgesch",               "lb", "ltz"},
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
     {"chi", "chinese",                     "zh", "chi"},
     {"chl", "chinese latin syntax",        "",   "chl"},
     {"est", "estonian",                    "et", "est"},
     {"lav", "latvian",                     "lv", "lav"},
     {"lit", "lithuanian",                  "lt", "lit"},
     {"tha", "thai",                        "th", "tha"},
     {"bul", "bulgarian",                   "bg", "bul"},
     {"cyt", "cyrillic transcript",         "",   "cyt"},
     {"ind", "indonesian",                  "id", "ind"},
     {"may", "malay",                       "ms", "may"},
     {"isl", "icelandic",                   "is", "isl"},
     {"jpn", "japanese",                    "ja", "jpn"},
     {"amh", "amharic",                     "am", "amh"},
     {"hye", "armenian",                    "hy", "hye"},
     {"tgl", "tagalog",                     "tl", "tgl"},
     {"bel", "belarusian",                  "be", "bel"},
     {"ben", "bengali",                     "bn", "ben"},
     {"mya", "burmese",                     "my", "mya"},
     {"hrv", "croatian",                    "hr", "hrv"},
     {"fas", "farsi",                       "fa", "fas"},
     {"gla", "gaelic",                      "gd", "gla"},
     {"kat", "georgian",                    "ka", "kat"},
     {"guj", "gujarati",                    "gu", "guj"},
     {"heb", "hebrew",                      "he", "heb"},
     {"hin", "hindi",                       "hi", "hin"},
     {"kan", "kannada",                     "kn", "kan"},
     {"kaz", "kazakh",                      "kk", "kaz"},
     {"khm", "khmer",                       "km", "khm"},
     {"kor", "korean",                      "ko", "kor"},
     {"lao", "lao",                         "lo", "lao"},
     {"mkd", "macedonian",                  "mk", "mkd"},
     {"mal", "malayalam",                   "ml", "mal"},
     {"mar", "marathi",                     "mr", "mar"},
     {"mol", "moldavian",                   "mo", "mol"},
     {"mon", "mongolian",                   "mn", "mon"},
     {"pan", "punjabi",                     "pa", "pan"},
     {"ron", "romanian",                    "ro", "ron"},
     {"srp", "serbian",                     "sr", "srp"},
     {"sin", "sinhalese",                   "si", "sin"},
     {"som", "somali",                      "so", "som"},
     {"swa", "swahili",                     "sw", "swa"},
     {"tam", "tamil",                       "ta", "tam"},
     {"tel", "telugu",                      "te", "tel"},
     {"bod", "tibetan",                     "bo", "bod"},
     {"tir", "tigrinya",                    "ti", "tir"},
     {"tuk", "turkmen",                     "tk", "tuk"},
     {"ukr", "ukrainian",                   "uk", "ukr"},
     {"urd", "urdu",                        "ur", "urd"},
     {"vie", "vietnamese",                  "vi", "vie"},
     {"zul", "zulu",                        "zu", "zul"},
     {"sot", "sesotho",                     "st", "sot"},
     {"bun", "bulgarian latin syntax",      "",   "bun"},
     {"bos", "bosnian",                     "",   "bos"},
     {"sla", "slavic",                      "",   "sla"},
     {"bet", "belarusianLatinStx",          "",   "bet"},
     {"mat", "macedonianLatinStx",          "",   "mat"},
     {"scc", "serbianLatinStx",             "",   "scc"},
     {"ukl", "ukrainianLatinStx",           "",   "ukl"},
     {"mlt", "maltese",                     "",   "mlt"},
     {"zht", "chinese traditional",    "zh-hant", "zht"},
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
      case 0x16 : // SWISS_FRENCH
         return french;
      case 0x17 : // SWISS_GERMAN
         return german;
      case 0x18 : // ICELANDIC
         return icelandic;
      case 0x19 : // BELGIAN_FLEMISH
         return dutch;   // XXX: Update when lang avail.
      case 0x1a : // AUSTRALIAN_ENGLISH
         return english;
      case 0x1b : // BELGIAN_FRENCH
         return french;
      case 0x1c : // AUSTRIAN_GERMAN
         return german;
      case 0x1d : // NEW_ZEALAND_ENGLISH
         return english;
      case 0x1e : // CHINESE_TAIWAN
         return chineseTraditional;
      case 0x1f : // CHINESE_HONG_KONG
         return chineseTraditional;
      case 0x20 : //CHINESE_PRC (People's Republic of China)
         return chinese;
      case 0x21 : // JAPANESE
         return japanese;
      case 0x22 : // THAI
         return thai;
      case 0x23 : // AFRIKAANS 
         return dutch;   // XXX: Update when lang avail.
      case 0x24 : // ALBANIAN
         return albanian;
      case 0x25 : // AMHARIC
         return amharic;
      case 0x26 : // ARMENIAN
         return armenian;
      case 0x27 : // TAGALOG
         return tagalog;
      case 0x28 : // BELARUSIAN
         return belarusian;
      case 0x29 : // BENGALI
         return bengali;
      case 0x2a : // BULGARIAN
         return bulgarian;
      case 0x2b : // BURMESE
         return burmese;
      case 0x2c : // CATALAN
         return catalan;
      case 0x2d : // CROATIAN
         return croatian;
      case 0x2e : // CANADIAN_ENGLISH
         return english;
      case 0x2f : // SOUTH_AFRICAN_ENGLISH
         return english;
      case 0x30 : // ESTONIAN
         return estonian;
      case 0x31 : // FARSI
         return farsi;
      case 0x32 : // CANADIAN_FRENCH
         return french;
      case 0x33 : // GAELIC
         return gaelic;
      case 0x34 : // GEORGIAN
         return georgian;
      case 0x35 : // GREEK_CYPRUS
         return greek;
      case 0x36 : // GUJARATI
         return gujarati;
      case 0x37 : // HEBREW
         return hebrew;
      case 0x38 : // HINDI
         return hindi;
      case 0x39 : // INDONESIAN
         return indonesian;
      case 0x3a : // IRISH
         return irish;
      case 0x3b : // SWISS_ITALIAN
         return italian;
      case 0x3c : // KANNADA (Indian language)
         return kannada;
      case 0x3d : // KAZAKH
         return kazakh;
      case 0x3e : // KHMER
         return khmer;
      case 0x3f : // KOREAN
         return korean;
      case 0x40 : // LAO
         return lao;
      case 0x41 : // LATVIAN
         return latvian;
      case 0x42 : // LITHUANIAN
         return lithuanian;
      case 0x43 : // MACEDONIAN
         return macedonian;
      case 0x44 : // MALAY
         return malay;
      case 0x45 : // MALAYALAM
         return malayalam;
      case 0x46 : // MARATHI
         return marathi;
      case 0x47 : // MOLDAVIAN
         return moldavian;
      case 0x48 : // MONGOLIAN
         return mongolian;
      case 0x49 : // NYNORSK
         return norwegian; // XXX: Update when lang avail.
      case 0x4a : // BRAZILIAN_PORTUGUESE
         return portuguese; // XXX: Update when lang avail.
      case 0x4b : // PUNJABI
         return punjabi;
      case 0x4c : // ROMANIAN
         return romanian;
      case 0x4d : // SERBIAN
         return serbian;
      case 0x4e : // SINHALESE
         return sinhalese;
      case 0x4f : // SOMALI
         return somali;
      case 0x50 : // LATIN_AMERICAN_SPANISH
         return spanish; // XXX: Update when lang avail.
      case 0x51 : // SWAHILI
         return swahili;
      case 0x52 : // FINNISH_SWEDISH
         return swedish;
      case 0x53 : // TAMIL
         return tamil;
      case 0x54 : // TELUGU
         return telugu;
      case 0x55 : // TIBETAN
         return tibetan;
      case 0x56 : // TIGRINYA
         return tigrinya;
      case 0x57 : // CYPRUS_TURKISH
         return turkish;
      case 0x58 : // TURKMEN
         return turkmen;
      case 0x59 : // UKRAINIAN
         return ukrainian;
      case 0x5a : // URDU
         return urdu;
      case 0x5b : // VIETNAMESE
         return vietnamese;
      case 0x5c : // ZULU
         return zulu;
      case 0x5d : // SESOTHO
         return sesotho;
      case 0x5e : // BASQUE
         return basque;
      case 0x5f : // GALICIAN
         return galician;
      case 0x60 : // ASIA_PACIFIC_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x61 : // TAIWAN_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x62 : // HONG_KONG_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x63 : // CHINA_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x64 : // JAPAN_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x65 : // THAI_ENGLISH
         return english; // XXX: Update when lang avail.
      case 0x66 : // ASIA_PACIFIC_MALAY
         return malay; // XXX: Update when lang avail.
      case 0x67 : // BOSNIAN
         return bosnian;
      case 0x68 : // MALTESE
         return maltese;
      case 0x69 : // CHINESE_TRADITIONAL
         return chineseTraditional;

      default:
         mc2log << warn << "LangTypes::getNavLangAsLanguage unknwon "
                << "language " << int(language) << " using english."
                << endl;
         return english;
   }
}
