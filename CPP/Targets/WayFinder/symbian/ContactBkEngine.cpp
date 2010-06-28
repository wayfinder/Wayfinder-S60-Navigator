/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ContactBkEngine.h"
#include <badesca.h>
#include <cntitem.h>
#include <cntfield.h>
#include <cntfldst.h>
#include "ContactBkObserver.h"

CContactBkEngine::CContactBkEngine(MContactBkObserver* aObserver) :
   iObserver(aObserver),
   iSearching(EFalse)
{
}

void CContactBkEngine::ConstructL()
{
   iContactDb = CContactDatabase::OpenL();
   iContactDb->SetDbViewContactType(KUidContactCard);
   iNumberOfContacts = iContactDb->CountL();
}

CContactBkEngine::~CContactBkEngine()
{
   if (iCurrentContactArray) {
      iCurrentContactArray->Reset();
   }
   delete iCurrentContactArray;
   delete iAsyncFinder;
   delete iFieldDef;
   delete iContactDb;
}

CContactBkEngine* CContactBkEngine::NewLC(MContactBkObserver* aObserver)
{
   CContactBkEngine* self = new (ELeave) CContactBkEngine(aObserver);
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}

CContactBkEngine* CContactBkEngine::NewL(MContactBkObserver* aObserver)
{
   CContactBkEngine* self = CContactBkEngine::NewLC(aObserver);
   CleanupStack::Pop(self);
   return self;
}

void CContactBkEngine::IdleFindCallback()
{
   if (iAsyncFinder->IsComplete()) {
      if (iAsyncFinder->Error() == KErrNone) {
         iCurrentContactArray = iAsyncFinder->TakeContactIds();
         iNumberOfSearchResults = iCurrentContactArray->Count();
         //Cleanup AsyncFinder before doing CallBack.
         delete iAsyncFinder;
         iAsyncFinder = NULL;
         delete iFieldDef;
         iFieldDef = NULL;
         iSearching = EFalse;
         //Check if we found any contacts, otherwise cleanup.
         if (iNumberOfSearchResults > 0) {
            iCurrentContactCardId = (*iCurrentContactArray)[0];
            iObserver->ContactBkSearchDone(KErrNone);
         }
         else {
            iCurrentContactArray->Reset();
            delete iCurrentContactArray;
            iCurrentContactArray = NULL;
            iCurrentContactCardId = 0;
            iObserver->ContactBkSearchDone(KErrNotFound);
         }
      }
   }
}

void CContactBkEngine::ClearSearch()
{
   if (iCurrentContactArray) {
      iCurrentContactArray->Reset();
   }
   delete iCurrentContactArray;
   iCurrentContactArray = NULL;
   iCurrentContactCardId = 0;
   iNumberOfSearchResults = 0;
   iSearching = EFalse;
}

void CContactBkEngine::SearchContactsByNameAsyncL(const TDesC& aName)
{
   if (!iSearching && !iCurrentContactArray) {
      if (iFieldDef) {
         delete iFieldDef;
         iFieldDef = NULL;
      }
      iFieldDef = new (ELeave) CContactItemFieldDef();
      iFieldDef->AppendL(KUidContactFieldGivenName);
      iFieldDef->AppendL(KUidContactFieldFamilyName);
      iAsyncFinder = iContactDb->FindAsyncL(aName, iFieldDef, this);
      iSearching = ETrue;
   }
   else {
      iObserver->ContactBkInUse();
   }
}

void CContactBkEngine::SearchContactsByAllFieldsAsyncL(const TDesC& aString)
{
   if (!iSearching && !iCurrentContactArray) {
      if (iFieldDef) {
         delete iFieldDef;
         iFieldDef = NULL;
      }
      iFieldDef = new (ELeave) CContactItemFieldDef();
      iFieldDef->AppendL(KUidContactFieldGivenName);
      iFieldDef->AppendL(KUidContactFieldFamilyName);
      iFieldDef->AppendL(KUidContactFieldPhoneNumber);
      iFieldDef->AppendL(KUidContactFieldSms);
      iFieldDef->AppendL(KUidContactFieldMsg);
      iFieldDef->AppendL(KUidContactFieldAddress);
      iFieldDef->AppendL(KUidContactFieldPostcode);
      iFieldDef->AppendL(KUidContactFieldLocality);
      iFieldDef->AppendL(KUidContactFieldCountry);
      iFieldDef->AppendL(KUidContactFieldJobTitle);
      iFieldDef->AppendL(KUidContactFieldCompanyName);
      iAsyncFinder = iContactDb->FindAsyncL(aString, iFieldDef, this);
      iSearching = ETrue;
   }
   else {
      iObserver->ContactBkInUse();
   }
}

TInt CContactBkEngine::CountSearchResults()
{
   return iNumberOfSearchResults;
}

void CContactBkEngine::FindAllContactsL()
{
   if (!iSearching && !iCurrentContactArray) {
      iSearching = ETrue;
      CArrayFix<CContactDatabase::TSortPref>* sortOrder = 
         new (ELeave) CArrayFixFlat<CContactDatabase::TSortPref>(2);
      sortOrder->AppendL(CContactDatabase::TSortPref(KUidContactFieldFamilyName, 
                                                     CContactDatabase::TSortPref::EAsc));
      sortOrder->AppendL(CContactDatabase::TSortPref(KUidContactFieldGivenName, 
                                                     CContactDatabase::TSortPref::EAsc));
      iContactDb->SortL(sortOrder);
      iCurrentContactArray = CContactIdArray::NewL(iContactDb->SortedItemsL());
      iNumberOfSearchResults = iCurrentContactArray->Count();
      if (iNumberOfSearchResults > 0) {
         iCurrentContactCardId = (*iCurrentContactArray)[0];
      }
   }
   else {
      iObserver->ContactBkInUse();
   }
}

void CContactBkEngine::GetSearchResultNames(CDesCArray& aNameArray)
{
   if (iCurrentContactArray) {
      _LIT(KSpace, " ");
      _LIT(KCommaSpace, ", ");
      for (TInt i = 0; i < iNumberOfSearchResults; i++) {
         TInt cardId = (*iCurrentContactArray)[i];
         CContactItem* card = iContactDb->ReadContactL(cardId);
         CleanupStack::PushL(card);
         TInt nField = card->CardFields().Find(KUidContactFieldGivenName);
         TInt fField = card->CardFields().Find(KUidContactFieldFamilyName);
         HBufC* fullName;
         if (nField != KErrNotFound && fField != KErrNotFound) {
            //The contact has both name and familyname.
            fullName = 
               HBufC::NewLC(card->CardFields()[nField].TextStorage()->Text().Length() + 
                            card->CardFields()[fField].TextStorage()->Text().Length() + 4);
            fullName->Des().Copy(card->CardFields()[fField].TextStorage()->Text());
            fullName->Des().Append(KCommaSpace);
            fullName->Des().Append(card->CardFields()[nField].TextStorage()->Text());
         } else if (nField != KErrNotFound) {
            //The contact has only got a name.
            fullName = 
               HBufC::NewLC(card->CardFields()[nField].TextStorage()->Text().Length() + 1);
            fullName->Des().Copy(card->CardFields()[nField].TextStorage()->Text());
         } else if (fField != KErrNotFound) {
            //The contact has only got a familyname.
            fullName = 
               HBufC::NewLC(card->CardFields()[fField].TextStorage()->Text().Length() + 1);
            fullName->Des().Copy(card->CardFields()[fField].TextStorage()->Text());
         } else {
            //The contact had no name and no familyname.
            _LIT(KContact, "Contact");
            fullName = HBufC::NewLC(KContact().Length() + 8);
            fullName->Des().Copy(KContact);
            fullName->Des().Append(KSpace);
            TBuf<4> cid; 
            cid.Num(cardId);
            fullName->Des().Append(cid);
         }
         aNameArray.AppendL(*fullName);
         CleanupStack::PopAndDestroy(fullName);
         iContactDb->CloseContactL(card->Id());
         CleanupStack::PopAndDestroy(card);
      }
   }
}

void CContactBkEngine::GetSearchResultNamesTabbed(CDesCArray& aNameArray)
{
   if (iCurrentContactArray) {
      _LIT(KSpace, " ");
      _LIT(KCommaSpace, ", ");
      _LIT(KTab, "\t");
      for (TInt i = 0; i < iNumberOfSearchResults; i++) {
         TInt cardId = (*iCurrentContactArray)[i];
         CContactItem* card = iContactDb->ReadContactL(cardId);
         CleanupStack::PushL(card);
         TInt nField = card->CardFields().Find(KUidContactFieldGivenName);
         TInt fField = card->CardFields().Find(KUidContactFieldFamilyName);
         HBufC* fullName;
         if (nField != KErrNotFound && fField != KErrNotFound) {
            //The contact has both name and familyname.
            fullName = 
               HBufC::NewLC(card->CardFields()[nField].TextStorage()->Text().Length() + 
                            card->CardFields()[fField].TextStorage()->Text().Length() + 4);
            fullName->Des().Copy(KTab);
            fullName->Des().Append(card->CardFields()[fField].TextStorage()->Text());
            fullName->Des().Append(KCommaSpace);
            fullName->Des().Append(card->CardFields()[nField].TextStorage()->Text());
         } else if (nField != KErrNotFound) {
            //The contact has only got a name.
            fullName = 
               HBufC::NewLC(card->CardFields()[nField].TextStorage()->Text().Length() + 1);
            fullName->Des().Copy(KTab);
            fullName->Des().Append(card->CardFields()[nField].TextStorage()->Text());
         } else if (fField != KErrNotFound) {
            //The contact has only got a familyname.
            fullName = 
               HBufC::NewLC(card->CardFields()[fField].TextStorage()->Text().Length() + 1);
            fullName->Des().Copy(KTab);
            fullName->Des().Append(card->CardFields()[fField].TextStorage()->Text());
         } else {
            //The contact had no name and no familyname.
            _LIT(KContact, "Contact");
            fullName = HBufC::NewLC(KContact().Length() + 8);
            fullName->Des().Copy(KTab);
            fullName->Des().Append(KContact);
            fullName->Des().Append(KSpace);
            TBuf<4> cid; 
            cid.Num(cardId);
            fullName->Des().Append(cid);
         }
         aNameArray.AppendL(*fullName);
         CleanupStack::PopAndDestroy(fullName);
         iContactDb->CloseContactL(card->Id());
         CleanupStack::PopAndDestroy(card);
      }
   }
}

void CContactBkEngine::SetCurrentContact(TInt aIndex)
{
   if (iCurrentContactArray && 
       (iNumberOfSearchResults > aIndex) && (aIndex >= 0)) {
      iCurrentContactCardId = (*iCurrentContactArray)[aIndex];
   }
}

void CContactBkEngine::GetContactNumbers(CDesCArray& aNumberArray, TBool aWithLabel)
{
   GetContactFields(KUidContactFieldPhoneNumber, aNumberArray, aWithLabel);   
}

void CContactBkEngine::GetContactAddresses(CDesCArray& aAddressArray, TBool aWithLabel)
{
   GetContactFields(KUidContactFieldAddress, aAddressArray, aWithLabel);
   GetContactFields(KUidContactFieldPostcode, aAddressArray, aWithLabel);
   GetContactFields(KUidContactFieldLocality, aAddressArray, aWithLabel);
   GetContactFields(KUidContactFieldCountry, aAddressArray, aWithLabel);
}

void CContactBkEngine::GetContactEmails(CDesCArray& aEmailArray, TBool aWithLabel)
{
   GetContactFields(KUidContactFieldEMail, aEmailArray, aWithLabel);   
}

void CContactBkEngine::GetContactFields(TFieldType aFieldType, 
                                        CDesCArray& aArray, 
                                        TBool aWithLabel)
{
   CContactItem* card = iContactDb->ReadContactL(iCurrentContactCardId);
   CleanupStack::PushL(card);
   TInt contField = card->CardFields().Find(aFieldType);
   //Contact field retrieving work horse, depending on if we want the labels for
   //each field or not we construct the array differently to present them 
   //differently on each platform.
   while (contField != KErrNotFound) {
      if (card->CardFields()[contField].TextStorage()->Text().Length() > 0) {
         if (aWithLabel) {
#if defined NAV2_CLIENT_SERIES60_V2 || defined NAV2_CLIENT_SERIES60_V1 || defined NAV2_CLIENT_SERIES60_V3
            _LIT(KTab, "\t");
            HBufC* text = 
               HBufC::NewLC(card->CardFields()[contField].TextStorage()->Text().Length() 
                            + card->CardFields()[contField].Label().Length() + 2);
            text->Des().Copy(card->CardFields()[contField].Label());
            text->Des().Append(KTab);
            text->Des().Append(card->CardFields()[contField].TextStorage()->Text());
            aArray.AppendL(*text);
            CleanupStack::PopAndDestroy(text);
#elif defined NAV2_CLIENT_SERIES80
            _LIT(KLParan, "(");
            _LIT(KRParan, ") ");
            HBufC* text = NULL;
            if (card->CardFields()[contField].Label().Length() > 0) {
               text = 
                  HBufC::NewLC(card->CardFields()[contField].TextStorage()->Text().Length()
                               + card->CardFields()[contField].Label().Length() + 4);
               text->Des().Copy(KLParan);
               text->Des().Append(card->CardFields()[contField].Label());
               text->Des().Append(KRParan);
               text->Des().Append(card->CardFields()[contField].TextStorage()->Text());
            }
            else {
               text = card->CardFields()[contField].TextStorage()->Text().AllocLC();
            }
            aArray.AppendL(*text);
            CleanupStack::PopAndDestroy(text);
#elif defined NAV2_CLIENT_UIQ || defined NAV2_CLIENT_UIQ3
            _LIT(KLParan, "(");
            _LIT(KRParan, ") ");
            HBufC* text = NULL;
            if (card->CardFields()[contField].Label().Length() > 0) {
               text = 
                  HBufC::NewLC(card->CardFields()[contField].TextStorage()->Text().Length()
                               + card->CardFields()[contField].Label().Length() + 4);
               text->Des().Copy(KLParan);
               text->Des().Append(card->CardFields()[contField].Label());
               text->Des().Append(KRParan);
               text->Des().Append(card->CardFields()[contField].TextStorage()->Text());
            }
            else {
               text = card->CardFields()[contField].TextStorage()->Text().AllocLC();
            }
            aArray.AppendL(*text);
            CleanupStack::PopAndDestroy(text);
#else
            aArray.AppendL(card->CardFields()[contField].TextStorage()->Text());
#endif
         }
         else {
            aArray.AppendL(card->CardFields()[contField].TextStorage()->Text());
         }
      }
      contField = card->CardFields().FindNext(aFieldType, contField + 1);
   }
   iContactDb->CloseContactL(card->Id());
   CleanupStack::PopAndDestroy(card);
}

void CContactBkEngine::GetContactField(TFieldType aFieldType, TDes& aValue)
{
   CContactItem* card = iContactDb->ReadContactL(iCurrentContactCardId);
   CleanupStack::PushL(card);
   TInt contField = card->CardFields().Find(aFieldType);
   if (contField != KErrNotFound) {
      if (card->CardFields()[contField].TextStorage()->Text().Length() > 0) {
         aValue.Copy(card->CardFields()[contField].TextStorage()->Text());
      }
   }
   iContactDb->CloseContactL(card->Id());
   CleanupStack::PopAndDestroy(card);
}

TBool CContactBkEngine::IsValidPhoneNumber(const TDesC& aNumber)
{
   if (aNumber.Length() == 0) {
      return EFalse;
   }
   _LIT(KValidChars, "+1234567890()-pw#*");
   for (int i = 0; i < aNumber.Length(); i++) {
      if ((KValidChars().Locate(aNumber[i]) == KErrNotFound) || 
          (aNumber.Length() < 2)) {
         return EFalse;
      }
   }
   return ETrue;
}

TBool CContactBkEngine::IsValidPhoneNumber(const TDesC& aNumber, const TDesC& aValidChars)
{
   if (aNumber.Length() == 0) {
      return EFalse;
   }
   for (int i = 0; i < aNumber.Length(); i++) {
      if (aValidChars.Locate(aNumber[i]) == KErrNotFound) {
         return EFalse;
      }
   }
   return ETrue;
}
