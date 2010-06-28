/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <aknform.h>
#include <eikedwob.h>
#include "WayFinderAppUi.h"

namespace isab{
   class Log;
}

/**
 *  This class handles the Upgrade/Activation dialog.
 */
class CUpgradeDialog : public CAknForm, public MEikEdwinObserver
{
public:
   /** @name Constructors and destructor. */
   //@{
   /**
    * Static constructor.
    * @param aAppUi Pointer to the AppUi class. 
    * @param aLog   Pointer to existing Log object that will be used for
    *               memory logging.
    * @param aMode  The mode of this dialog; upgrade or activate.
    * @return A new CUpgradeDialog object.
    */
   static class CUpgradeDialog* NewL(class CWayFinderAppUi* aAppUi, 
                                     class isab::Log* aLog,
                                     enum CWayFinderAppUi::TUpgradeMode aMode);

   /** Virtual destructor. */
   virtual ~CUpgradeDialog();
private:
   /**
    * Constructor. 
    * @param aLog   Pointer to existing Log object that will be used for
    *               memory logging.
    */
   CUpgradeDialog(class isab::Log* aLog);
   
   /**
    * Second-phase constructor.
    * @param aAppUi Pointer to the AppUi class. 
    * @param aMode  The mode of this dialog; upgrade or activate.
    */
   void ConstructL( class CWayFinderAppUi* aAppUi, 
                    enum CWayFinderAppUi::TUpgradeMode aMode);
   //@}

   void DeleteBufs();

public:
   /** @name From CAknDialog. */
   //@{
   virtual TInt ExecuteLD( TInt aResourceId );
   virtual void PrepareLC( TInt aResourceId );
   //@}

private:
   /** @name Functions that set and get text in CEikEdwins. */
   //@{
   /**
    * Sets a text to a CEikEdwin identified by a unique id. If no
    * edwin with that id is present in this form, no action is taken.
    * @param aId   The id of the target edwin. 
    * @param aText The text to set.
    */
   void SetEdwinTextIfPresentL(TInt aId, const TDesC& aText);
   /**
    * Gets the content of a CEikEdwin identified by a unique id. If no
    * edwin with that id is present in this form, the function returns
    * NULL.
    * @param aId The id of the target edwin. 
    * @return a HBufC containing the text, or NULL if no edwin was found. 
    */
   HBufC* GetEdwinTextIfPresentL(TInt aId);
   //@}
public: // New functons

   void Close();

   /**@name Sets the content of the different fields. */
   //@{
   void SetCountry( const TDesC &aCountry );
   void SetPhone( const TDesC &aPhone );
   void SetRegNumber( const TDesC &aRegNumber );
   void SetName(const TDesC& aName);
   void SetEmail(const TDesC& aEmail);
   //@}
   void SetNotInProgress() { iInProgress=EFalse; }

private:

   void SelectCountryL();

public: // Functions from base classes

   void HandleEdwinEventL(class CEikEdwin* aEdwin,
                          enum TEdwinEvent aEventType);

   enum TKeyResponse OfferKeyEventL(const struct TKeyEvent& aKeyEvent,
                                    enum TEventCode aType);

   TBool OkToExitL(TInt aButtonId);

private:  // Functions from base class

   template<class T>
   T* GetControl(T*& aPtr, TInt aId)
   {
      return aPtr = static_cast<T*>(ControlOrNull(aId));
   }

   template<class T>
   T* GetControl(TInt aId)
   {
      T* ptr = NULL;
      return GetControl(ptr, aId);
   }
          
   /**
    * From CEikDialog 
    * Set default field value to member data.
    */
   void PostLayoutDynInitL();

   /**
    * From CEikDialog 
    * Load the correct bitmap.
    */
   void PreLayoutDynInitL();

   /**
    * From CAknForm 
    * Show save query. If user answers "No" to this query.
    * return field value to the value which is before editing.
    */
   TBool QuerySaveChangesL();

   /**
    * From CAknForm 
    * Save the contents of the form.
    */
   TBool SaveFormDataL();

   void DynInitMenuPaneL( TInt aMenuId, class CEikMenuPane* aMenuPane );

   void ProcessCommandL( TInt aCommandId );

   TInt AddFormItemL(const TDesC &promt, const TDesC &defText);

private: //data

   TInt   iCountryId;
   HBufC* iCountry;
   HBufC* iPhone;
   HBufC* iRegNumber;
   HBufC* iName;
   HBufC* iEmail;

   TBool iHasFocus;
   TInt iNewItemId;
   
   class CWayFinderAppUi* iAppUi;
   class isab::Log* iLog;
   TBool iInProgress;
   enum CWayFinderAppUi::TUpgradeMode iUpgradeMode;
};

#endif

// End of File
