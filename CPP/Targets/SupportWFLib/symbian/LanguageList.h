/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef LanguageList_H
#define LanguageList_H

#include <vector>
#include <algorithm>
#include <iterator>
#include <e32base.h>
#include <badesca.h>
///This class is used to analyze what languages are available to
///use. It investigates what languages in the language list resource
///are available as resource files and as audio files.
class CLanguageList : public CBase {
public:

   static class CLanguageList* NewL(TInt aResource);
   static class CLanguageList* NewLC(TInt aResource);
   static class CLanguageList* NewL(TInt aResource, 
                                    const TDesC& aResourcePath,
                                    const TDesC& aAudioPath,
                                    const TDesC& aAudioFormatPrefix,
                                    const CDesCArray* aAllowedLangs);
   static class CLanguageList* NewLC(TInt aResource,
                                    const TDesC& aResourcePath,
                                    const TDesC& aAudioPath,
                                    const TDesC& aAudioFormatPrefix,
                                    const CDesCArray* aAllowedLangs);

   ///Virtual destructor. 
   virtual ~CLanguageList();

   public:
      struct TWfLanguageList {
         TWfLanguageList() : 
            handle(-1), symbianCode(-1), nav2code(-1), hasRXX(EFalse), hasAudio(EFalse), isAllowed(EFalse)
         {}
         TWfLanguageList(class TResourceReader& aReader);
         //XXX remove
         TInt    handle;   // May be assigned
         TInt    symbianCode;
         TPtrC   fullname;
         TPtrC   isoname;
         TPtrC   symbianname;
         TInt    nav2code;
         TBool   hasRXX;
         TBool   hasAudio;
         TBool   isAllowed;
      };

   class TLanguageListIteratorC : 
      public std::iterator<std::bidirectional_iterator_tag, const TWfLanguageList> 
   {
      friend class CLanguageList;
      CArrayFix<TWfLanguageList>* iList;
      TInt iIndex;
      explicit TLanguageListIteratorC(CArrayFix<TWfLanguageList>* aList, 
                                      TInt aIndex);
      explicit TLanguageListIteratorC(CArrayFix<TWfLanguageList>* aList);
   public:
      explicit TLanguageListIteratorC();
      class TLanguageListIteratorC operator++();
      class TLanguageListIteratorC& operator++(int);
      class TLanguageListIteratorC operator--();
      class TLanguageListIteratorC& operator--(int);
      const struct TWfLanguageList& operator*() const;
      const struct TWfLanguageList* operator->() const;
      TBool operator==(const class TLanguageListIteratorC& aOther) const;
      TBool operator!=(const class TLanguageListIteratorC& aOther) const;
   };

   class TLanguageListIterator :
      public std::iterator<std::bidirectional_iterator_tag, TWfLanguageList> 
   {
      friend class CLanguageList;
      CArrayFix<TWfLanguageList>* iList;
      TInt iIndex;
      explicit TLanguageListIterator(CArrayFix<TWfLanguageList>* aList, 
                                     TInt aIndex);
      explicit TLanguageListIterator(CArrayFix<TWfLanguageList>* aList);
   public:
      explicit TLanguageListIterator();
      class TLanguageListIterator operator++();
      class TLanguageListIterator& operator++(int);
      class TLanguageListIterator operator--();
      class TLanguageListIterator& operator--(int);
      struct TWfLanguageList& operator*();
      struct TWfLanguageList* operator->();
      TBool operator==(const class TLanguageListIterator& aOther) const;
      TBool operator!=(const class TLanguageListIterator& aOther) const;
   };

   typedef TLanguageListIteratorC const_iterator;
   typedef TLanguageListIterator iterator;

   class TLanguageListIterator begin();
   class TLanguageListIterator end();

   public:
      TInt Count();
      TWfLanguageList & Get(TInt aIndex);
   ///XXX Remove
      TWfLanguageList & FindByHandle(TInt aHandle);
      TWfLanguageList & FindBySymbianCode(TInt aValue);
      TWfLanguageList & FindByIsoName(const TDesC & aName);
      TWfLanguageList & FindBySymbianName(const TDesC & aName);
      TWfLanguageList & FindByFullName(const TDesC& aName);

   CDesCArray* GenerateLanguageNameArrayLC();

   protected:
      CLanguageList();
   void ConstructL(TInt aResource,
                   const TDesC& aResourcePath,
                   const TDesC& aAudioPath,
                   const TDesC& aAudioFormatPrefix,
                   const CDesCArray* aAllowedLangs);

   private:
      CArrayFix<TWfLanguageList> * iLangList;
      HBufC8 * iResource;
};


#endif /* LanguageList_H */
