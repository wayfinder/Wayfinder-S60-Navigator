/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONTACT_BK_ENGINE_H
#define CONTACT_BK_ENGINE_H

#include <e32base.h>
#include <cntdb.h>

/**
 * The contact book engine class.
 * This class provides functionality to search for contacts and to 
 * retrieve the information stored in them, e.g. phone number, 
 * name, address...
 * 
 */
class CContactBkEngine : public CBase, 
                         public MIdleFindObserver
{
private:
   /**
    * Class constructor
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async search has completed.
    */
   CContactBkEngine(class MContactBkObserver* aObserver);

   /**
    * Symbian second stage ConstructL.
    */
   void ConstructL();

public:
   /**
    * Class Destructor
    */
   virtual ~CContactBkEngine();

   /**
    * Symbian static NewLC function.
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async search has completed, forwarded to 
    *                    the class constructor.
    * @return            A ContactBkEngine instance.
    */
   static CContactBkEngine* NewLC(class MContactBkObserver* aObserver);

   /**
    * Symbian static NewL function.
    *
    * @param aObserver   The observer that want to get notified when
    *                    a async search has completed, forwarded to 
    *                    the class constructor.
    * @return            A ContactBkEngine instance.
    */
   static CContactBkEngine* NewL(class MContactBkObserver* aObserver);

   /**
    * Inherited from MFindIdleObserver, this is called during async search
    * in the contact database. It is called for every 16 items searched.
    */
   virtual void IdleFindCallback();

   /**
    * Called by the user of this class to clear the current search 
    * result before starting a new search.
    */
   void ClearSearch();

   /**
    * Async function to search in the contact book, stores result in 
    * internal ContactId array. This one only searches in the name fields.
    *
    * @param aName   The name to search for in the contact database.
    */
   void SearchContactsByNameAsyncL(const TDesC& aName);

   /**
    * Async function to search in the contact book, stores result in 
    * internal ContactId array. This one searches in most fields.
    *
    * @param aString   The string to search for in the contact database.
    */
   void SearchContactsByAllFieldsAsyncL(const TDesC& aString);

   /**
    * Counts and returns the number of hits from a search.
    *
    * @return   Number of search hits.
    */
   TInt CountSearchResults();

   /**
    * Creates a sorted list of all contacts (not async) and stores
    * the result in the internal ContactId array.
    */
   void FindAllContactsL();

   /**
    * Function called by the user of this class to retrieve the
    * information stored in the internal ContactId array which has 
    * been created from earlier calls to search and find functions.
    * 
    * @param aNameArray   The result gets appended in this array.
    */
   void GetSearchResultNames(CDesCArray& aNameArray);

   /**
    * Function called by the user of this class to retrieve the
    * information stored in the internal ContactId array which has 
    * been created from earlier calls to search and find functions.
    * This "Tabbed" version adds a "\t" in the beginning of the 
    * array so we can present the result in a S60 AknSelectionList.
    *
    * @param aNameArray   The result gets appended in this array.
    */
   void GetSearchResultNamesTabbed(CDesCArray& aNameArray);

   /**
    * Called by user of this class to set the current contact that 
    * we want to retrieve information from.
    *
    * @param aIndex   Sets the internal ContactId that we're currently 
    *                 interested in retrieving information from.
    */
   void SetCurrentContact(TInt aIndex);

   /**
    * Function to get all phone numbers stored on a contact from 
    * the contact book. Can be called after previous calls to 
    * SetCurrentContact.
    *
    * @param aNumberArray   The phone numbers gets appended in this array.
    * @param aWithLabel     Flag to decide if we want to get the labels
    *                       from the contact book appended to the array.
    * @see                  "ContactBkEngine::SetCurrentContact(...)"
    */
   void GetContactNumbers(CDesCArray& aNumberArray, 
                          TBool aWithLabel = EFalse);

   /**
    * Function to get all address information stored on a contact from 
    * the contact book. Can be called after previous calls to 
    * SetCurrentContact.
    *
    * @param aAddressArray   The addresses gets appended in this array.
    * @param aWithLabel      Flag to decide if we want to get the labels
    *                        from the contact book appended to the array.
    * @see                   "ContactBkEngine::SetCurrentContact(...)"
    */
   void GetContactAddresses(CDesCArray& aAddressArray, 
                            TBool aWithLabel = EFalse);

   /**
    * Function to get all email information stored on a contact from 
    * the contact book. Can be called after previous calls to 
    * SetCurrentContact.
    *
    * @param aEmailArray     The email adresses gets appended in this array.
    * @param aWithLabel      Flag to decide if we want to get the labels
    *                        from the contact book appended to the array.
    * @see                   "ContactBkEngine::SetCurrentContact(...)"
    */
   void GetContactEmails(CDesCArray& aEmailArray, 
                         TBool aWithLabel = EFalse);

   /**
    * Static function to validate a phone number.
    * This validates against a preset string of valid chars.
    *
    * @param aNumber   The phone number to validate.
    * @return          True if it's a valid phone number.
    */
   static TBool IsValidPhoneNumber(const TDesC& aNumber);

   /**
    * Static function to validate a phone number.
    * This accepts a string of valid chars to validate against.
    * 
    * @param aNumber       The phone number to validate.
    * @param aValidChars   String of valid chars.
    * @return              True if it's a valid phone number.
    */
   static TBool IsValidPhoneNumber(const TDesC& aNumber, 
                                   const TDesC& aValidChars);

private:
   /**
    * Private helper function to retrieve all the specified contact 
    * fields from a contact. 
    *
    * @param aFieldType   The type of field we want to get the data from.
    * @param aArray       The array to append the data to.
    * @param aWithLabel   Flag to decide about with or without labels.
    * @see                "ContactBkEngine::GetContactNumbers(...)"
    * @see                "ContactBkEngine::GetContactAddresses(...)"
    */
   void GetContactFields(TFieldType aFieldType, 
                         CDesCArray& aArray, 
                         TBool aWithLabel);
   /**
    * Private helper function to retrieve one specified contact 
    * field from a contact. 
    *
    * @param aFieldType   The type of field we want to get the data from.
    * @param aValue       The string to insert the data to.
    */
   void GetContactField(TFieldType aFieldType, 
                        TDes& aValue);

private:
   /** 
    * The user of this class inherits from the observer interface. 
    */
   class MContactBkObserver* iObserver;

   /**
    * The contact database, closed and cleaned up on destruction.
    */
   class CContactDatabase* iContactDb;

   /**
    * The internal array of search and find results.
    */
   class CContactIdArray* iCurrentContactArray;

   /**
    * The currently selected contact from the above array.
    */
   TInt iCurrentContactCardId;

   /**
    * Total number of contacts in contact book.
    */
   TInt iNumberOfContacts;

   /**
    * Number of search hits.
    */
   TInt iNumberOfSearchResults;

   /**
    * The async finder used by the async search functions.
    */
   class CIdleFinder* iAsyncFinder;

   /**
    * The field definitions that tells the async finder what 
    * to search for in the contact database.
    */
   class CContactItemFieldDef* iFieldDef;

   /**
    * Flag set when we're searching so that we cant't start
    * a new search before we're done with the ongoing search.
    */
   TBool iSearching;

};

#endif
