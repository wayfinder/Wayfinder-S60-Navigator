/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "LanguageList.h"

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <coemain.h>
#include <barsread.h>

#include "CleanupSupport.h"
#include "LanguageList.hrh"

CLanguageList::TWfLanguageList::TWfLanguageList(class TResourceReader& aReader)
   : handle(-1), symbianCode(-1), nav2code(-1), hasRXX(EFalse), hasAudio(EFalse)
{
   symbianCode = aReader.ReadInt16();
   fullname.Set(aReader.ReadTPtrC());
   isoname.Set(aReader.ReadTPtrC());
   symbianname.Set(aReader.ReadTPtrC());
   nav2code = aReader.ReadInt16();
}

CLanguageList::CLanguageList()
   : iLangList(NULL), iResource(NULL)
{
}

CLanguageList::~CLanguageList()
{
   delete iLangList;
   delete iResource;
}

void CLanguageList::ConstructL(TInt aResource, 
                               const TDesC& aResourcePath,
                               const TDesC& aAudioPath,
                               const TDesC& aAudioFormatPrefix,
                               const CDesCArray* aAllowedLangs)
{
   iResource = CCoeEnv::Static()->AllocReadResourceAsDes8L(aResource);
   class TResourceReader reader;
   reader.SetBuffer(iResource);

   TInt numElement = reader.ReadInt16();
   if (numElement < 1) {
      User::Leave(KErrNotFound);
   }
   iLangList = new(ELeave) CArrayFixSeg<TWfLanguageList>(numElement+2);

   class RFs fs;
   fs.Connect();
   class TParse* parser = new (ELeave) TParse;
   CleanupStack::PushL(parser);
   class TFindFile* finder = new (ELeave) TFindFile(fs);
   CleanupStack::PushL(finder);

   _LIT(KRExt2, ".r%02d");
   _LIT(KRExt3, ".r%03d");
   for (TInt i = 0; i < numElement; ++i) {
      iLangList->AppendL(TWfLanguageList(reader));
      struct TWfLanguageList& elem = iLangList->At(iLangList->Count() - 1);
      // Pre-allow unless aAllowedLangs is passed to us, in which case the 
      // allowed-bit is set later in this function.
      if (aAllowedLangs) {
         elem.isAllowed = EFalse;
      } else {
         elem.isAllowed = ETrue;
      }

      // Log the "system default" as having resources and audio. 
      // This behaviour is chosen to mesh with how CLanguageList
      // is used by the language setting.
      if (elem.symbianCode == ELangSystemDefault) {
         elem.hasRXX = ETrue;
         elem.hasAudio = ETrue;
      } else {
         // Check if the rXX file is available
         TBuf<64> fileAndExt;
         if (elem.symbianCode < 100) { 
            fileAndExt.Format( KRExt2, TInt(elem.symbianCode));
         } else {
            fileAndExt.Format( KRExt3, TInt(elem.symbianCode));
         }
         parser->Set(fileAndExt, &aResourcePath, NULL);
         if(KErrNone == finder->FindByDir(parser->NameAndExt(), 
                                         parser->DriveAndPath())) {
            elem.hasRXX = ETrue;
         } else {
            elem.hasRXX = EFalse;
         }
         
         // Check if the default voice is available
         _LIT(KSoundVersion, "resource_version.short");
         TBuf<32> audioName = elem.symbianname;
         parser->Set(aAudioPath, 
                     &KSoundVersion, NULL);
         //parser->AddDir(aAudioFormatPrefix); // Not any more with new symbian paths.
         parser->AddDir(audioName);

         if(KErrNone == finder->FindByDir(parser->NameAndExt(), parser->Path())){
            elem.hasAudio = ETrue;
         } else {
            elem.hasAudio = EFalse;
         }
      }
   }
   CleanupStack::PopAndDestroy(finder);
   CleanupStack::PopAndDestroy(parser);
   fs.Close();

   if (aAllowedLangs) {
      for (TInt i = 0; i < aAllowedLangs->MdcaCount(); i++) {
         // Now find by symbian name since iso name is a lot of duplicates.
         FindBySymbianName(aAllowedLangs->MdcaPoint(i)).isAllowed = ETrue;
      }
   }
}

CLanguageList * CLanguageList::NewLC(TInt aResource, 
                               const TDesC& aResourcePath,
                               const TDesC& aAudioPath,
                               const TDesC& aAudioFormatPrefix,
                               const CDesCArray* aAllowedLangs)
{
   CLanguageList * tmp = new(ELeave) CLanguageList();
   CleanupStack::PushL(tmp);
   tmp->ConstructL(aResource, aResourcePath, aAudioPath, aAudioFormatPrefix, aAllowedLangs);
   return tmp;
}

CLanguageList * CLanguageList::NewLC(TInt aResource) {
   return NewLC(aResource, KNullDesC, KNullDesC, KNullDesC, NULL);
}

CLanguageList * CLanguageList::NewL(TInt aResource,
                               const TDesC& aResourcePath,
                               const TDesC& aAudioPath,
                               const TDesC& aAudioFormatPrefix,
                               const CDesCArray* aAllowedLangs)
{
   CLanguageList * tmp = NewLC(aResource, aResourcePath, aAudioPath, aAudioFormatPrefix, aAllowedLangs);
   CleanupStack::Pop(tmp);
   return tmp;
}

CLanguageList * CLanguageList::NewL(TInt aResource) {
   return NewL(aResource, KNullDesC, KNullDesC, KNullDesC, NULL);
}

TInt CLanguageList::Count()
{
   return iLangList->Count();
}

class CLanguageList::TLanguageListIterator CLanguageList::begin()
{
   return TLanguageListIterator(iLangList);
}

class CLanguageList::TLanguageListIterator CLanguageList::end()
{
   return TLanguageListIterator();
}

CLanguageList::TWfLanguageList & CLanguageList::Get(TInt aIndex)
{
   return (*iLangList)[aIndex];
}

CDesCArray* CLanguageList::GenerateLanguageNameArrayLC()
{
   CDesCArray* langArray = new (ELeave) CDesCArraySeg(8);
   CleanupStack::PushL(langArray);
   
   for(CLanguageList::iterator q = begin(); q != end(); ++q){
      langArray->AppendL(q->fullname);
   }
   return langArray;
}


CLanguageList::TWfLanguageList & CLanguageList::FindByHandle(TInt aHandle)
{
   TInt i;
   for (i=0; i < iLangList->Count(); ++i) {
      TWfLanguageList & elem = (*iLangList)[i];
      if ( elem.handle == aHandle) {
         return elem;
      }
   }
   return (*iLangList)[0];
}

CLanguageList::TWfLanguageList & CLanguageList::FindBySymbianCode(TInt aValue)
{
   TInt i;
   for (i=0; i < iLangList->Count(); ++i) {
      TWfLanguageList & elem = (*iLangList)[i];
      if ( elem.symbianCode == aValue) {
         return elem;
      }
   }
   return (*iLangList)[0];
}

CLanguageList::TWfLanguageList & CLanguageList::FindByFullName(const TDesC& aName)
{
   TInt i;
   TInt sepPos = 0;
   for (i=0; i < iLangList->Count(); ++i) {
      TWfLanguageList & elem = (*iLangList)[i];
      sepPos = aName.Locate('*');
      if (sepPos != KErrNotFound) {
         if ( elem.fullname.Compare(aName.Mid(1)) == 0) {
            return elem;
         }
      } else {
         if ( elem.fullname.Compare(aName) == 0) {
            return elem;
         }
      }
   }
   return (*iLangList)[0];
}

CLanguageList::TWfLanguageList & CLanguageList::FindByIsoName(const TDesC & aName)
{
   TInt i;
   for (i=0; i < iLangList->Count(); ++i) {
      TWfLanguageList & elem = (*iLangList)[i];
      if ( elem.isoname.CompareF(aName) == 0) {
         return elem;
      }
   }
   return (*iLangList)[0];
}

CLanguageList::TWfLanguageList & CLanguageList::FindBySymbianName(const TDesC & aName)
{
   TInt i;
   for (i=0; i < iLangList->Count(); ++i) {
      TWfLanguageList & elem = (*iLangList)[i];
      if ( elem.symbianname.CompareF(aName) == 0) {
         return elem;
      }
   }
   return (*iLangList)[0];
}

CLanguageList::TLanguageListIteratorC::TLanguageListIteratorC(CArrayFix<TWfLanguageList>* aList, TInt aIndex) :
   iList(aList), iIndex(aIndex)
{}

CLanguageList::TLanguageListIteratorC::TLanguageListIteratorC(CArrayFix<TWfLanguageList>* aList) :
   iList(aList), iIndex(0)
{}

CLanguageList::TLanguageListIteratorC::TLanguageListIteratorC() :
   iList(NULL), iIndex(0)
{}

class CLanguageList::TLanguageListIteratorC 
CLanguageList::TLanguageListIteratorC::operator++()
{
   TLanguageListIteratorC tmp = *this;
   this->operator++(1);
   return tmp;
}

class CLanguageList::TLanguageListIteratorC& 
CLanguageList::TLanguageListIteratorC::operator++(int)
{
   if(++iIndex >= iList->Count()){
      iList = NULL;
   }
   return *this;
}

class CLanguageList::TLanguageListIteratorC 
CLanguageList::TLanguageListIteratorC::operator--()
{
   return TLanguageListIteratorC(iList, iIndex--);
}

class CLanguageList::TLanguageListIteratorC& 
CLanguageList::TLanguageListIteratorC::operator--(int)
{
   --iIndex;
   return *this;
}

const struct CLanguageList::TWfLanguageList& 
CLanguageList::TLanguageListIteratorC::operator*() const
{
   return iList->At(iIndex);   
}

const struct CLanguageList::TWfLanguageList* 
CLanguageList::TLanguageListIteratorC::operator->() const
{
   return &(iList->At(iIndex));
}

TBool CLanguageList::TLanguageListIteratorC::operator==(const class CLanguageList::TLanguageListIteratorC& aOther) const
{
   return (aOther.iList == iList && 
           (iList == NULL || aOther.iIndex == iIndex));
}

TBool CLanguageList::TLanguageListIteratorC::operator!=(const class CLanguageList::TLanguageListIteratorC& aOther) const
{
   return !(*this == aOther);
}

CLanguageList::TLanguageListIterator::TLanguageListIterator(CArrayFix<CLanguageList::TWfLanguageList>* aList, TInt aIndex) : 
   iList(aList), iIndex(aIndex)
{}

CLanguageList::TLanguageListIterator::TLanguageListIterator(CArrayFix<CLanguageList::TWfLanguageList>* aList) :
   iList(aList), iIndex(0)
{}

CLanguageList::TLanguageListIterator::TLanguageListIterator() :
   iList(NULL), iIndex(0)
{}

class CLanguageList::TLanguageListIterator 
CLanguageList::TLanguageListIterator::operator++()
{
   TLanguageListIterator tmp = *this;
   this->operator++(1);
   return tmp;
}

class CLanguageList::TLanguageListIterator& 
CLanguageList::TLanguageListIterator::operator++(int)
{
   if(++iIndex >= iList->Count()){
      iList = NULL;
   }
   return *this;
}

class CLanguageList::TLanguageListIterator 
CLanguageList::TLanguageListIterator::operator--()
{
   return TLanguageListIterator(iList, iIndex--);
}

class CLanguageList::TLanguageListIterator& 
CLanguageList::TLanguageListIterator::operator--(int)
{
   --iIndex;
   return *this;
}

struct CLanguageList::TWfLanguageList& 
CLanguageList::TLanguageListIterator::operator*()
{
   return iList->At(iIndex);
}

struct CLanguageList::TWfLanguageList* 
CLanguageList::TLanguageListIterator::operator->()
{
   return &(iList->At(iIndex));
}

TBool CLanguageList::TLanguageListIterator::operator==(const class CLanguageList::TLanguageListIterator& aOther) const
{
   return (aOther.iList == iList && 
           (iList == NULL || aOther.iIndex == iIndex));
};

TBool CLanguageList::TLanguageListIterator::operator!=(const class CLanguageList::TLanguageListIterator& aOther) const
{
   return !(*this == aOther);
};

