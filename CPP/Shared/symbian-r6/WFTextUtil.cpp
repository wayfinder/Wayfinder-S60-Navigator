/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "WFTextUtil.h"
#include <coemain.h>
#include <stdlib.h>
#include <stdio.h>
#include <utf.h>
#include <algorithm>
#include <ctype.h>
#include "nav2util.h"

enum TWFTextUtilPanics {
   EStrSepArgNull = 0,
};
_LIT(KWFTextUtil, "TextUtil");

#define ASSERT_ALWAYS(cond, panic) \
__ASSERT_ALWAYS((cond), User::Panic(KWFTextUtil, (panic)))

char *
WFTextUtil::stripPhoneNumberL(const char *src)
{
   if (!src) return NULL;
   TInt s = 0;
   TInt d = 0;
   TInt len = strlen(src);
   char *dst = new (ELeave) char[len+1]; /* Cannot get larger than original. */

   if (src[s] == '+') {
      dst[d++] = src[s++];
   }                    
   while (s < len) {
      if (src[s] >= '0' && src[s] <= '9') {
         /* Digit. */
         dst[d++] = src[s];
      }  
      s++;                 
   }
   dst[d] = 0; /* Nultermination. */
   return dst;
}

char *
WFTextUtil::uint32AsStringL(uint32 num)
{
   char *str = new (ELeave) char[16]; /* Uint32 cannot be larger than 12 characters. */
   sprintf(str, "%"PRIu32, num);
   return str;
}

char *
WFTextUtil::TDesCToUtf8L(const TDesC& inbuf)
{
   char *result;
   HBufC8* temp;

   temp = HBufC8::NewLC(inbuf.Length()*4);
   TPtr8 temp_ptr = temp->Des();
   TInt truncated = CnvUtfConverter::ConvertFromUnicodeToUtf8(temp_ptr, inbuf);
   if (truncated != KErrNone) {
      /* truncated now contains either an error or the number of */
      /* characters left untranslated. */
      /* XXX Do stuff here XXX */
   }
   TInt len = temp->Length();
   result = new (ELeave) char[len+4];
   int i = 0;
   while (i < len) {
      result[i] = (*temp)[i];
      i++;
   }
   result[i] = 0;
   CleanupStack::PopAndDestroy(temp);

   return result;
}

char *
WFTextUtil::TDesCToUtf8LC(const TDesC& inbuf)
{
   char *result = TDesCToUtf8L(inbuf);

   CleanupStack::PushL(result);
   return result;
}

TInt
WFTextUtil::Utf8ToTDes(const char *utf8str, TDes &outBuf, int length)
{
   if (!utf8str) {
      outBuf.Zero();
      return 0;
   }

   TPtrC8 tmp;
   tmp.Set(reinterpret_cast<const unsigned char*>(utf8str), length);

   TInt res = CnvUtfConverter::ConvertToUnicodeFromUtf8(outBuf, tmp);

   return res;
}
TInt
WFTextUtil::Utf8ToTDes(const char *utf8str, TDes &outBuf)
{
   if (!utf8str) {
      outBuf.Zero();
      return 0;
   }

   TPtrC8 tmp;
   tmp.Set(reinterpret_cast<const unsigned char*>(utf8str), strlen(utf8str));

   TInt res = CnvUtfConverter::ConvertToUnicodeFromUtf8(outBuf, tmp);

   return res;
}

HBufC*
WFTextUtil::Utf8Alloc(const char *utf8str)
{
   if(!utf8str){
      return NULL;
   }
   HBufC *unicodeStr = HBufC::New(strlen(utf8str)+1);
   if(unicodeStr){
      TPtr temp_ptr = unicodeStr->Des(); /* Yes, this variable is necessary. */
      TInt res = Utf8ToTDes(utf8str, temp_ptr);
      res = res; /* Unused at the moment */
   }
   return unicodeStr;
}

HBufC*
WFTextUtil::Utf8AllocL(const char *utf8str, int length)
{
   HBufC* unicodeStr = Utf8AllocLC(utf8str, length);
   CleanupStack::Pop(unicodeStr);
   return unicodeStr;
}
HBufC*
WFTextUtil::Utf8AllocL(const char *utf8str)
{
   HBufC* unicodeStr = Utf8AllocLC(utf8str);
   CleanupStack::Pop(unicodeStr);
   return unicodeStr;
}
HBufC*
WFTextUtil::Utf8ToHBufCL(const char *utf8str)
{
   return Utf8AllocL(utf8str);
}

HBufC*
WFTextUtil::Utf8AllocLC(const char *utf8str, int length)
{
   if(!utf8str){
      CleanupStack::PushL((TAny*)0);
      return NULL;
   }
   HBufC *unicodeStr = HBufC::NewL(length+1);
   CleanupStack::PushL(unicodeStr);
   TPtr temp_ptr = unicodeStr->Des(); /* Yes, this variable is necessary. */
   TInt res = Utf8ToTDes(utf8str, temp_ptr, length);
   res = res; /* Unused at the moment */
   return unicodeStr;
}
HBufC*
WFTextUtil::Utf8AllocLC(const char *utf8str)
{
   if(!utf8str){
      CleanupStack::PushL((TAny*)0);
      return NULL;
   }
   HBufC *unicodeStr = HBufC::NewL(strlen(utf8str)+1);
   CleanupStack::PushL(unicodeStr);
   TPtr temp_ptr = unicodeStr->Des(); /* Yes, this variable is necessary. */
   TInt res = Utf8ToTDes(utf8str, temp_ptr);
   res = res; /* Unused at the moment */
   return unicodeStr;
}
HBufC*
WFTextUtil::Utf8ToHBufCLC(const char *utf8str)
{
   return Utf8AllocLC(utf8str);
}

TBool
WFTextUtil::MatchesResourceStringL(const TDesC &string, TInt resourceId)
{
   HBufC *tmp;
   TBool result = EFalse;
   tmp = CCoeEnv::Static()->AllocReadResourceLC( resourceId );
   result = string.Find(*tmp) == 0;
   CleanupStack::PopAndDestroy(tmp);
   return result;
}  

char*
WFTextUtil::strsep(char** stringp, const char* delim)
{
   ASSERT_ALWAYS(stringp != NULL, EStrSepArgNull);
   char* const token = *stringp; //token always starts at *stringp
   if(*stringp){
      //count the characters not in delim.
      const size_t tokenLen = strcspn(*stringp, delim); 
      *stringp += tokenLen; //jump to delim.
      if(**stringp != '\0'){ 
         //found a delim
         **stringp = '\0'; //part string
         *stringp += 1;    //start of next token.
      } else {
         //no delim
         *stringp = NULL;  //indicate end of input.
      }
   }
   return token;
}

char*
WFTextUtil::strdupLC(const char* aSrc)
{
   if(!aSrc){
      return NULL;
   }
   TInt len = strlen(aSrc) + 1;
   if(len > KMaxTInt/2){
      User::Leave(KErrArgument);
   }
   char* result = new (ELeave) char[len];
   CleanupStack::PushL(result);
   strcpy(result, aSrc);
   return result;
}

char*
WFTextUtil::strdupL(const char* aSrc)
{
   char* result = NULL;
   if(aSrc){
      result = strdupLC(aSrc);
      CleanupStack::Pop(result);
   }
   return result;
}

HBufC8*
WFTextUtil::DupAllocLC(const char* src)
{
   if(!src){
      CleanupStack::PushL((TAny*)0);
      return NULL;
   }
   HBufC8* ret = HBufC8::NewLC(strlen(src) + 4);
   ret->operator=(reinterpret_cast<const TUint8*>(src));
   return ret;
}

HBufC8*
WFTextUtil::DupAllocL(const char* src)
{
   HBufC8* ret = DupAllocLC(src);
   CleanupStack::Pop(ret);
   return ret;
}

HBufC8*
WFTextUtil::DupAlloc(const char* src)
{
   HBufC8* ret = HBufC8::New(strlen(src) + 4);
   if(ret){
      *ret = reinterpret_cast<const TText8*>(src);
   }
   return ret;
}


TBool
WFTextUtil::ParseHost(const TDesC8& aArg, TDes8& aHost, TUint& aPort)
{
   TBool ret = EFalse;
   // find the ':' character in the string.
   TInt colonPos = aArg.Find(_L8(":"));
   if(colonPos != KErrNotFound && colonPos > 0){
      TPtrC8 host = aArg.Left(colonPos);
      TPtrC8 port = aArg.Mid(colonPos+1);
      TLex8 portParser(port);
      TUint uport = 0;
      if(portParser.Val(uport) == KErrNone && 
         portParser.Remainder().Length() == 0){
         if(aHost.MaxLength() >= host.Length()){
            aPort = uport;
            aHost = host;
            ret = ETrue;
         }
      }
   }
   return ret;
}

TBool
WFTextUtil::ParseHost(const TDesC16& aArg, TDes16& aHost, TUint& aPort)
{
   TBool ret = EFalse;
   // find the ':' character in the string.
   TInt colonPos = aArg.Find(_L16(":"));
   if(colonPos != KErrNotFound && colonPos > 0){
      TPtrC16 host = aArg.Left(colonPos);
      TPtrC16 port = aArg.Mid(colonPos+1);
      TLex16 portParser(port);
      TUint uport = 0;
      if(portParser.Val(uport) == KErrNone && 
         portParser.Remainder().Length() == 0){
         if(aHost.MaxLength() >= host.Length()){
            aPort = uport;
            aHost = host;
            ret = ETrue;
         }
      }
   }
   return ret;
}

const uint32
unicodeToIso8859_1[][2] = {

   { 0x102,   0x41   },   //   iso-8859-2:0xc3
   { 0x103,   0x61   },   //   iso-8859-2:0xe3
   { 0x104,   0x41   },   //   iso-8859-2:0xa1
   { 0x105,   0x61   },   //   iso-8859-2:0xb1
   { 0x106,   0x43   },   //   iso-8859-2:0xc6
   { 0x107,   0x63   },   //   iso-8859-2:0xe6
   { 0x10c,   0x43   },   //   iso-8859-2:0xc8
   { 0x10d,   0x63   },   //   iso-8859-2:0xe8
   { 0x10e,   0x44   },   //   iso-8859-2:0xcf
   { 0x10f,   0x64   },   //   iso-8859-2:0xef
   { 0x110,   0x44   },   //   iso-8859-2:0xd0
   { 0x111,   0x64   },   //   iso-8859-2:0xf0
   { 0x118,   0x45   },   //   iso-8859-2:0xca
   { 0x119,   0x65   },   //   iso-8859-2:0xea
   { 0x11a,   0x45   },   //   iso-8859-2:0xcc
   { 0x11b,   0x65   },   //   iso-8859-2:0xec
   { 0x139,   0x4c   },   //   iso-8859-2:0xc5
   { 0x13a,   0x6c   },   //   iso-8859-2:0xe5
   { 0x13d,   0x4c   },   //   iso-8859-2:0xa5
   { 0x13e,   0x6c   },   //   iso-8859-2:0xb5
   { 0x141,   0x4c   },   //   iso-8859-2:0xa3
   { 0x142,   0x6c   },   //   iso-8859-2:0xb3
   { 0x143,   0x4e   },   //   iso-8859-2:0xd1
   { 0x144,   0x6e   },   //   iso-8859-2:0xf1
   { 0x147,   0x4e   },   //   iso-8859-2:0xd2
   { 0x148,   0x6e   },   //   iso-8859-2:0xf2
   { 0x150,   0x4f   },   //   iso-8859-2:0xd5
   { 0x151,   0x6f   },   //   iso-8859-2:0xf5
   { 0x154,   0x52   },   //   iso-8859-2:0xc0
   { 0x155,   0x72   },   //   iso-8859-2:0xe0
   { 0x158,   0x52   },   //   iso-8859-2:0xd8
   { 0x159,   0x72   },   //   iso-8859-2:0xf8
   { 0x15a,   0x53   },   //   iso-8859-2:0xa6
   { 0x15b,   0x73   },   //   iso-8859-2:0xb6
   { 0x15e,   0x53   },   //   iso-8859-2:0xaa
   { 0x15f,   0x73   },   //   iso-8859-2:0xba
   { 0x160,   0x53   },   //   iso-8859-2:0xa9
   { 0x161,   0x73   },   //   iso-8859-2:0xb9
   { 0x162,   0x54   },   //   iso-8859-2:0xde
   { 0x163,   0x74   },   //   iso-8859-2:0xfe
   { 0x164,   0x54   },   //   iso-8859-2:0xab
   { 0x165,   0x74   },   //   iso-8859-2:0xbb
   { 0x16e,   0x55   },   //   iso-8859-2:0xd9
   { 0x16f,   0x75   },   //   iso-8859-2:0xf9
   { 0x170,   0x55   },   //   iso-8859-2:0xdb
   { 0x171,   0x75   },   //   iso-8859-2:0xfb
   { 0x179,   0x5a   },   //   iso-8859-2:0xac
   { 0x17a,   0x7a   },   //   iso-8859-2:0xbc
   { 0x17b,   0x5a   },   //   iso-8859-2:0xaf
   { 0x17c,   0x7a   },   //   iso-8859-2:0xbf
   { 0x17d,   0x5a   },   //   iso-8859-2:0xae
   { 0x17e,   0x7a   },   //   iso-8859-2:0xbe
   { 0x2c7,   0x20   },   //   iso-8859-2:0xb7 caron
   { 0x2d8,   0x20   },   //   iso-8859-2:0xa2 breve
   { 0x2d9,   0x20   },   //   iso-8859-2:0xff dot above
   { 0x2db,   0x20   },   //   iso-8859-2:0xb2 ogonek
   { 0x2dd,   0x20   },   //   iso-8859-2:0xbd double acute
   { 0x0,   0x0   },
};

char *
WFTextUtil::newTDesDupL(const TDesC16 &inbuf)
{
#if !defined (NAV2_USE_UTF8)
   char *ret;
   int length = inbuf.Length();
   int i = 0;

   User::LeaveIfNull(ret = new char[length+1]);

   while (i < length) {
      if (inbuf[i] > 255) {
         int j = 0;
         while (unicodeToIso8859_1[j][0]) {
            if (unicodeToIso8859_1[j][0] == inbuf[i]) {
               /* Match! */
               ret[i] = unicodeToIso8859_1[j][1];
               break;
            }
            j++;
         }
         if (!unicodeToIso8859_1[j][0]) {
            ret[i] = 0x20; /* Space. */
         }
      } else {
         ret[i] = inbuf[i];
      }
      i++;
   }
   ret[i] = 0;

   return ret;
#else
   return TDesCToUtf8L(inbuf);
#endif
}

char *
WFTextUtil::newTDesDupLC(const TDesC16 &inbuf)
{
#if !defined (NAV2_USE_UTF8)
   char *ret;
   int length = inbuf.Length();
   int i = 0;

   ret = new(ELeave) char[length+1];
   CleanupStack::PushL(ret);

   while (i < length) {
      if (inbuf[i] > 255) {
         int j = 0;
         while (unicodeToIso8859_1[j][0]) {
            if (unicodeToIso8859_1[j][0] == inbuf[i]) {
               /* Match! */
               ret[i] = unicodeToIso8859_1[j][1];
               break;
            }
            j++;
         }
         if (!unicodeToIso8859_1[j][0]) {
            ret[i] = 0x20; /* Space. */
         }
      } else {
         ret[i] = inbuf[i];
      }
      i++;
   }
   ret[i] = 0;

   return ret;
#else
   return TDesCToUtf8LC(inbuf);
#endif
}

char* WFTextUtil::newTDesDupL(const TDesC8 &inbuf)
{
   char* ret = newTDesDupLC(inbuf);
   CleanupStack::Pop(ret);
   return ret;
}

char* WFTextUtil::newTDesDupLC(const TDesC8& inbuf)
{
   char* ret = new (ELeave) char[inbuf.Length() + 1];
   CleanupStack::PushL(ret);
   TPtr8 ret_p(reinterpret_cast<TText8*>(ret), inbuf.Length() + 1);
   ret_p = inbuf;
   ret_p.ZeroTerminate();
   return ret;
}

int
WFTextUtil::char2TDes(TDes &dest, const char* src)
{
   dest.Zero();
   if (!src) {
      dest.PtrZ();
      return 0;
   }
#if !defined (NAV2_USE_UTF8)
   int length = strlen(src);
   int i = 0;

   //Crashes if we try to write more than maxlength in the buffer
   if( length >= dest.MaxLength() )
      length = dest.MaxLength()-1;

   while (i < length) {
      dest.Append(uint8(src[i]));
      i++;
   }
   dest.PtrZ();
   return length;
#else
   return Utf8ToTDes(src, dest);
#endif
}
int
WFTextUtil::char2HBufC(HBufC *dest, const char* src)
{
#if !defined (NAV2_USE_UTF8)
   dest->Des().Zero();
   if (!src) {
      dest->Des().PtrZ();
      return 0;
   }

   int length = strlen(src);
   int i = 0;

   //Crashes if we try to write more than maxlength in the buffer
   if( length >= dest->Des().MaxLength() )
      length = dest->Des().MaxLength()-1;

   while (i < length) {
      dest->Des().Append(uint8(src[i]));
      i++;
   }
   dest->Des().PtrZ();
   return length;
#else
   TPtr destPtr = dest->Des();
   return Utf8ToTDes(src, destPtr);
#endif

}

HBufC*
WFTextUtil::AllocLC(const char* src)
{
#if !defined (NAV2_USE_UTF8)
   if(!src){
      CleanupStack::PushL((TAny*)0);
      return NULL;
   }
   HBufC* ret = HBufC::NewLC(strlen(src));
   TPtrC8 tmp;
   tmp.Set(reinterpret_cast<const unsigned char*>(src), strlen(src));
   TPtr ptr = ret->Des();
   TDesWiden(ptr, tmp);
   return ret;
#else
   return Utf8AllocLC(src);
#endif
}

HBufC*
WFTextUtil::AllocL(const char* src)
{
#if !defined (NAV2_USE_UTF8)
   HBufC* ret = AllocLC(src);
   CleanupStack::Pop(ret);
   return ret;
#else
   return Utf8AllocL(src);
#endif
}

HBufC*
WFTextUtil::Alloc(const char* aSrc)
{
#if !defined (NAV2_USE_UTF8)
   HBufC* ret = HBufC::New(strlen(aSrc));
   if(ret){
      TPtrC8 tmp;
      tmp.Set(reinterpret_cast<const unsigned char*>(src), strlen(src));
      TPtr ptr = ret->Des();
      TDesWiden(ptr, tmp);
   }
   return ret;
#else
   return Utf8Alloc(aSrc);
#endif
}

HBufC*
WFTextUtil::AllocLC(const char* aSrc, int aLength)
{
#if !defined (NAV2_USE_UTF8)
   if(!src){
      CleanupStack::PushL((TAny*)0);
      return NULL;
   }
   HBufC* ret = HBufC::NewLC(aLength);
   TPtrC8 tmp;
   tmp.Set(reinterpret_cast<const unsigned char*>(src), aLength);
   TPtr ptr = ret->Des();
   TDesWiden(ptr, tmp);
   return ret;
#else
   return Utf8AllocLC(aSrc, aLength);
#endif
}

HBufC*
WFTextUtil::AllocL(const char* aSrc, int aLength)
{
#if !defined (NAV2_USE_UTF8)
   HBufC* ret = AllocLC(src, aLength);
   CleanupStack::Pop(ret);
   return ret;
#else
   return Utf8AllocL(aSrc, aLength);
#endif
}

TBool
WFTextUtil::TDesWiden(TDes16 &aDst, const TDesC8& aSrc)
{
   TBool ret;
   if((ret = (aDst.MaxLength() >= aSrc.Length()))){
      aDst.SetLength(aSrc.Length());
      for(TInt i = 0; i < aSrc.Length(); ++i){
         aDst[i] = aSrc[i];
      }
   }
   return ret;
}

HBufC8* WFTextUtil::NarrowLC(const TDesC& aSrc)
{
   HBufC8* ret = HBufC8::NewLC(aSrc.Length());
   ret->Des().Copy(aSrc);
   return ret;
}

TBool
WFTextUtil::TDesCopy(TDes& dst, const TDesC& src)
{
   dst.Copy(src.Ptr(), std::min(dst.MaxLength(), src.Length()));
   return dst.MaxLength() >= src.Length();
}

TBool WFTextUtil::TDesCopy(TDes8& dst, const TDesC16& src)
{
#ifndef NAV2_USE_UTF8
   dst.Copy(src.Ptr(), std::min(dst.MaxLength(), src.Length()));
   return dst.MaxLength() >= src.Length();
#else
   return CnvUtfConverter::ConvertFromUnicodeToUtf8(dst, src);
#endif
}


TBool
WFTextUtil::TDesAppend(TDes& dst, const TDesC& src)
{
   TInt available = dst.MaxLength() - dst.Length();
   dst.Append(src.Ptr(), std::min(available, src.Length()));
   return available >= src.Length();
}

TBool
WFTextUtil::TDesAppend(TDes& dst, const char* src)
{
   TBool ret = ETrue;
   if(src){
      HBufC* tmp = HBufC::New(strlen(src) + 4);
      if(tmp){
         char2HBufC(tmp, src);
         ret = TDesAppend(dst, *tmp);
         delete tmp;
      } else {
         ret = EFalse;
      }
   }
   return ret;     
}

HBufC*
WFTextUtil::SearchAndReplaceL(const TDesC& aSrc, 
                              const TDesC& aSearch, 
                              const TDesC& aReplace)
{
   TPtrC lineleft(aSrc);
   TInt pos = 0;
   CPtrCArray* subs = new (ELeave) CPtrCArray(4);
   CleanupStack::PushL(subs);
   while(KErrNotFound != (pos = lineleft.Find(aSearch))){
      TPtrC part(lineleft.Left(pos));
      lineleft.Set(lineleft.Mid(pos + aSearch.Length())); //skip matched part
      subs->AppendL(part);
   }
   subs->AppendL(lineleft);
   TInt lengthOFSubs = 0;
   for(TInt q = 0; q < subs->Count(); ++q){
      lengthOFSubs += (*subs)[q].Length() + aReplace.Length();
   }
   HBufC* constructed = HBufC::NewL(lengthOFSubs);
   for(TInt w = 0; w < subs->Count() - 1; ++w){
      constructed->Des().Append((*subs)[w]);
      constructed->Des().Append(aReplace);
   }
   constructed->Des().Append((*subs)[subs->Count() - 1]);
   CleanupStack::PopAndDestroy(subs);
   return constructed;
}

void
WFTextUtil::SearchAndReplace(TDes& aSrc, TChar aSearch, TText aReplace)
{
   TInt pos = KErrNotFound;
   TPtrC ptr(&aReplace, 1);
   while(KErrNotFound != (pos = aSrc.Locate(aSearch))){
      aSrc.Replace(pos, 1, ptr);
   }
}

HBufC*
WFTextUtil::SearchAndReplaceLC(const TDesC& aSrc, TChar aSearch, 
                                      TText aReplace)
{
   HBufC* dest = aSrc.AllocLC();
   TPtr dest_ptr = dest->Des();
   SearchAndReplace(dest_ptr, aSearch, aReplace);
   return dest;
}

TBool WFTextUtil::Equal(const TText8* aLhs,  const TText8* aRhs)
{
   return Equal(reinterpret_cast<const char*>(aLhs), 
                reinterpret_cast<const char*>(aRhs));
}

TBool WFTextUtil::Equal(const char* aLhs, const char* aRhs)
{
   return isab::strequ(aLhs, aRhs);
}

#define CREATE_TPTRC8(name, from)   \
 TPtrC8 name(from);                 \
 name.Set(from, User::StringLength(from))

TBool WFTextUtil::Equal(const TText8*  aLhs, const TDesC16& aRhs)
{
   CREATE_TPTRC8(lhs, aLhs);
   return Equal(lhs, aRhs);
}

TBool WFTextUtil::Equal(const TText8*  aLhs, const TDesC8& aRhs)
{
   CREATE_TPTRC8(lhs, aLhs);
   return Equal(lhs, aRhs);
}


TBool WFTextUtil::Equal(const char* aLhs, const TDesC16& aRhs)
{
   return Equal(reinterpret_cast<const TText8*>(aLhs), aRhs);
}

TBool WFTextUtil::Equal(const char* aLhs, const TDesC8& aRhs)
{
   return Equal(reinterpret_cast<const TText8*>(aLhs), aRhs);
}

TBool WFTextUtil::Equal(const TDesC16& aLhs, const TDesC8& aRhs)
{
   TBool ret = EFalse;
#ifdef NAV2_USE_UTF8
   HBufC16* wide = HBufC::New(aRhs.Length());
   if(wide){
      TPtr16 wide_p = wide->Des();
      CnvUtfConverter::ConvertToUnicodeFromUtf8(wide_p, aRhs);
      ret = (wide_p == aLhs);
   }
   delete wide;
#else
   ret = (aLhs.Length() == aRhs.Length());
   if(ret){
      for(TInt i = 0; ret && i < aLhs.Length(); ++i){
         ret = (aLhs[i] == aRhs[i];)
      }
   }
#endif
   return ret;
}

// from mc2
void
WFTextUtil::trimEnd(char* s)
{
   if (s != NULL) {
      uint32 tmp = strlen(s)-1;
      while( isspace(s[tmp]) && (tmp > 0)) {
         s[tmp] = '\0';
         tmp--;
      }
   }
}
