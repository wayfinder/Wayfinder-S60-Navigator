/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CONTENT_WINDOW_CONTAINER_H
#define CONTENT_WINDOW_CONTAINER_H

#ifdef NAV2_CLIENT_SERIES60_V28

// INCLUDES
#include <coecntrl.h>
#include "Log.h" 
// FORWARD DECLARATIONS
class CContentWindowView;

#include <brctlinterface.h>
#include <brctllinkresolver.h>
#include <brctlspecialloadobserver.h>
#include <list>

namespace isab {
   class DataGuiMess;
};

class CResolveItem : public CBase
{
   CResolveItem() {}
   void ConstructL(
         class MBrCtlLinkContent* aLinkContent, const TDesC& aUrl) {
      iLinkContent = aLinkContent;
      iUrl = aUrl.AllocL();
   }
public:
   static class CResolveItem* NewL(
         class MBrCtlLinkContent* aLinkContent, const TDesC& aUrl) {
      class CResolveItem* self = new (ELeave) CResolveItem;
      CleanupStack::PushL(self);
      self->ConstructL(aLinkContent, aUrl);
      CleanupStack::Pop(self);
      return self;
   }

   ~CResolveItem() {
      delete(iUrl);
   }

   class MBrCtlLinkContent* getLinkContent() { return iLinkContent; }
   const TDesC& getUrl() { return *iUrl; }
private:
   class MBrCtlLinkContent* iLinkContent;
   HBufC* iUrl;
};

// CLASS DECLARATION

/**
 *  CContentWindowContainer  container control class.
 *  
 */
class CContentWindowContainer : public CCoeControl,
   MBrCtlLinkResolver, MBrCtlLoadEventObserver, MBrCtlSpecialLoadObserver,
                       MCoeControlObserver
{
public: // Constructors and destructor
   static class CContentWindowContainer* NewL(class CContentWindowView* aParent,
         const TRect& aRect,
         const TDesC& aMbmFile,
         isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CContentWindowContainer();

private:

   CContentWindowContainer(isab::Log* aLog) : iLog(aLog){}

   /**
    * EPOC default constructor.
    * @param aRect Frame rectangle for container.
    */
   void ConstructL(CContentWindowView* aView, const TRect& aRect, const TDesC& aMbmFile);

public: // New functions

   void PositionOfControlChanged(CCoeControl *control, TPoint pos);

   void Debug();

   void DataReceived(
      const isab::DataGuiMess* aMess,
      const char* aUrl);

   void DataReceived(
      class MBrCtlLinkContent* aLinkContent,
      const isab::DataGuiMess* aMess,
      const char *aUrl);

private: // Functions from base classes


   /**
    * From CoeControl,SizeChanged.
    */
   void SizeChanged();

   /**
    * From CoeControl,CountComponentControls.
    */
   TInt CountComponentControls() const;

   /**
    * From CCoeControl,ComponentControl.
    */
   CCoeControl* ComponentControl(TInt aIndex) const;

   /**
    * From CCoeControl,Draw.
    */
   void Draw(const TRect& aRect) const;

   /**
    * From ?base_class ?member_description
    */
   // event handling section
   // e.g Listbox events
   void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

   TKeyResponse OfferKeyEventL(
         const TKeyEvent& aKeyEvent,
         TEventCode aType );
public:

   /**
    * From MBrCtlLinkResolver:
    */
   TBool ResolveEmbeddedLinkL(const TDesC &aEmbeddedUrl,
         const TDesC &aCurrentUrl,
         enum TBrCtlLoadContentType aLoadContentType,
         class MBrCtlLinkContent &aEmbeddedLinkContent);

   /**
    * From MBrCtlLinkResolver:
    */
    TBool ResolveLinkL(const TDesC &aUrl,
         const TDesC &aCurrentUrl,
         class MBrCtlLinkContent &aBrCtlLinkContent);
   /**
    * From MBrCtlLinkResolver:
    */
    void CancelAll();

   /**
    * From MBrCtlLoadEventObserver:
    */
   void HandleBrowserLoadEventL(enum TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
         TUint aSize, TUint16 aTransactionId);
   /**
    * From MBrCtlSpecialLoadObserver:
    */
   void NetworkConnectionNeededL(TInt *aConnectionPtr,
         TInt *aSockSvrHandle,
         TBool *aNewConn,
         enum TApBearerType *aBearerType);
   /**
    * From MBrCtlSpecialLoadObserver:
    */
   TBool HandleRequestL(RArray<TUint> *aTypeArray,
         CDesCArrayFlat *aDesArray);
   /**
    * From MBrCtlSpecialLoadObserver:
    */
   TBool HandleDownloadL(RArray<TUint> *aTypeArray,
         CDesCArrayFlat *aDesArray);


   TBool GetFileL(class MBrCtlLinkContent *aLinkContent, const TDesC& aUrl);

   HBufC* RecognizeLC(const TDesC& aFileName, const TDesC8& aData);

private: //data

   CContentWindowView* iView;

   isab::Log* iLog;

   TUint iCommandBase;

   TUint iBrCtlCapabilities;

   class CBrCtlInterface* iBrCtlInterface;

   list<class CResolveItem*> iReqList;
};

#endif
#endif

// End of File
