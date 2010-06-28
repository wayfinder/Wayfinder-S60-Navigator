/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef NAV2_CLIENT_SERIES60_V28

// INCLUDE FILES
#include <eiklabel.h>  // for labels
#include <eikedwin.h>  // for edwin text boxes
#include <barsread.h>  // for resource reader
#include <gulcolor.h>  // for color
#include <aknutils.h>  // for fonts
#include <txtfmlyr.h>
#include <apmstd.h>
#include <apmrec.h>
#include <apgcli.h>

#if defined NAV2_CLIENT_SERIES60_V2
#include <aknsdrawutils.h>
#endif

#include "RsgInclude.h"
#include "wficons.mbg"
#include "ContentWindowContainer.h"
#include "ContentWindowView.h"
#include "WayFinderConstants.h"
/* #include "CoordinatesControl.h" */
#include "InfoCompass.h"
#include "memlog.h"
#include "WFTextUtil.h"
#include "PositionDisplay.h"
#include "RectTools.h"
#include "Dialogs.h"
#include "GuiProt/GuiProtMess.h"
#include "WFTextUtil.h"

#include "DistancePrintingPolicy.h"

   _LIT(KUrl, "http://www.jni.nu/");
   _LIT(KFile, "c:\\index.html");

   _LIT(Kfilename, "c:\\temp.html");
   _LIT(Kimagename, "c:\\temp.jpg");
   _LIT8(KDataType, "text/html");
   _LIT(KDataType_w, "text/html");
   _LIT(KImageType_w, "image/jpg");
   _LIT(KCharSet, "UTF-8");


enum TControls
{
   ENumberControls
};


class CContentWindowContainer*
CContentWindowContainer::NewL(class CContentWindowView* aParent,
                     const TRect& aRect,
                     const TDesC& aMbmFile,
                     isab::Log* aLog)
{
   class CContentWindowContainer* self = new (ELeave) CContentWindowContainer(aLog);
   CleanupStack::PushL(self);
   self->SetMopParent(aParent);
   self->ConstructL(aParent, aRect, aMbmFile);
   CleanupStack::Pop(self);
   return self;
}

// ---------------------------------------------------------
// CContentWindowContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CContentWindowContainer::ConstructL(CContentWindowView* aView,
      const TRect& aRect,
      const TDesC& aMbmFile)
{
   iView = aView;
   CreateWindowL();

   SetRect(aRect);
   ActivateL();

   iCommandBase = TBrCtlDefs::ECommandIdBase;

   // Initialize member variables
   iBrCtlCapabilities =
      TBrCtlDefs::ECapabilityDisplayScrollBar |
      TBrCtlDefs::ECapabilityLoadHttpFw |
      TBrCtlDefs::ECapabilityClientResolveEmbeddedURL |
      TBrCtlDefs::ECapabilityClientNotifyURL;
   TRect rect(Position(), Size());
   iBrCtlInterface = CreateBrowserControlL( this,
         rect,
         iBrCtlCapabilities,
         iCommandBase,
         NULL,
         this,
         NULL,
         NULL,
         NULL);

   iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsAutoLoadImages, 1);
   iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsEmbedded, 1);

   // These observers can be added and removed dynamically

/*    iBrCtlInterface-> */
/*       AddLoadEventObserverL(this); */
}

// Destructor
CContentWindowContainer::~CContentWindowContainer()
{
   delete iBrCtlInterface;
}

void CContentWindowContainer::SizeChanged()
{
}

void
CContentWindowContainer::PositionOfControlChanged(CCoeControl *control, TPoint pos)
{
   control->SetExtent(pos, control->MinimumSize());
}

void CContentWindowContainer::Debug()
{
   if (iBrCtlInterface) {
/*       list<class CResolveItem*>::iterator it = iReqList.begin(); */
/*       if (it != iReqList.end()) { */
/*          class CResolveItem* item = *it; */
/*          iReqList.pop_front(); */
/*          GetFileL(item->getLinkContent(), item->getUrl()); */
/*  */
/*          delete item; */
/*       } else { */
/*          TRAPD(err, */
         iBrCtlInterface->LoadUrlL(KUrl, -1, TBrCtlDefs::ECacheModeNoCache)
         ;
   /*       iBrCtlInterface->LoadFileL(KFile) */
/*          ); */
/*          if (err != KErrNone) { */
            /* Leave. */
/*             _LIT(KApa, "failed"); */
/*             _LIT(KBepa, "Trap leave..."); */
/*             WFDialog::ShowScrollingQueryL(KApa, KBepa, EFalse, 1, 1); */
/*          } */
/*       } */
   }
}


TInt CContentWindowContainer::CountComponentControls() const
{
   return ENumberControls; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CContentWindowContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CContentWindowContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CContentWindowContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CContentWindowContainer::Draw(const TRect& aRect) const
{
}

// ---------------------------------------------------------
// CContentWindowContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CContentWindowContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                          TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

TBool
CContentWindowContainer::ResolveEmbeddedLinkL(
      const TDesC &aEmbeddedUrl,
      const TDesC &aCurrentUrl,
      enum TBrCtlLoadContentType aLoadContentType,
      class MBrCtlLinkContent &aEmbeddedLinkContent)
{
   class CResolveItem* item =
         CResolveItem::NewL(&aEmbeddedLinkContent, aEmbeddedUrl);
   iReqList.push_back(item);
   iView->RequestData(aEmbeddedUrl);
   return ETrue;
}

TBool
CContentWindowContainer::ResolveLinkL(const TDesC &aUrl,
      const TDesC &aCurrentUrl,
      class MBrCtlLinkContent &aBrCtlLinkContent)
{
   class CResolveItem* item = CResolveItem::NewL(&aBrCtlLinkContent, aUrl);
   iReqList.push_back(item);
   iView->RequestData(aUrl);
   return ETrue;
}
void
CContentWindowContainer::CancelAll()
{
   list<class CResolveItem*>::iterator it = iReqList.begin();
   while (it != iReqList.end()) {
      class CResolveItem* item = *it;
      it++;
      delete item; 
      iReqList.pop_front();
   }
}

void
CContentWindowContainer::HandleBrowserLoadEventL(
      enum TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
      TUint aSize, TUint16 aTransactionId)
{
}
void
CContentWindowContainer::NetworkConnectionNeededL(TInt *aConnectionPtr,
      TInt *aSockSvrHandle,
      TBool *aNewConn,
      enum TApBearerType *aBearerType)
{
}
TBool
CContentWindowContainer::HandleRequestL(
      RArray<TUint> *aTypeArray,
      CDesCArrayFlat *aDesArray)
{
   return EFalse;
}
TBool
CContentWindowContainer::HandleDownloadL(
      RArray<TUint> *aTypeArray,
      CDesCArrayFlat *aDesArray)
{
   return EFalse;
}

TKeyResponse CContentWindowContainer::OfferKeyEventL(
      const TKeyEvent& aKeyEvent,
      TEventCode aType )
{
   if ( aType != EEventKey &&
        aType != EEventKeyUp &&
        aType != EEventKeyDown){
      // Is not key event?
      return EKeyWasNotConsumed;
   }
   
   switch ( aKeyEvent.iCode ) // The code of key event is...
   {
   // Switches tab.
   case EKeyLeftArrow: // Left key.
   case EKeyRightArrow: // Right Key.
   case EKeyUpArrow:
   case EKeyDownArrow:
   case EKeyDevice3:
      return iBrCtlInterface->OfferKeyEventL( aKeyEvent, aType );
      break;
      default:
      break;
   }

   switch (aKeyEvent.iScanCode) {
   case EStdKeyHash:
      iView->HandleCommandL(EAknSoftkeyBack);
      return EKeyWasConsumed;
      break;
   default:
      return EKeyWasNotConsumed;
      break;
   }
}


TBool
CContentWindowContainer::GetFileL(
      class MBrCtlLinkContent *aLinkContent, const TDesC& aUrl)
{
   RFs myFs;
   RFile myFile;

   myFs.Connect();

   HBufC8* data = HBufC8::NewLC(1024*100);
   TPtr8 ptr = data->Des();
   
   TBool foo = ETrue;

   _LIT(KJpg, ".jpg");
   if (KErrNotFound != aUrl.Find(KJpg)) {
      foo = EFalse;
   }

   if (foo) {
      myFile.Open(myFs, Kfilename, EFileShareReadersOnly|EFileRead);
   } else {
      myFile.Open(myFs, Kimagename, EFileShareReadersOnly|EFileRead);
   }

   myFile.Read(ptr, 1024*100);
   myFile.Close();

   HBufC* contentType = NULL;
   contentType = RecognizeLC(aUrl, ptr);

   aLinkContent->HandleResolveComplete(*contentType, KCharSet, data);

   CleanupStack::PopAndDestroy(contentType);
   CleanupStack::PopAndDestroy(data);

   myFs.Close();

   return ETrue;
}

HBufC*
CContentWindowContainer::RecognizeLC(
      const TDesC& aFileName, const TDesC8& aData)
{
   TDataRecognitionResult dataType;
   RApaLsSession apaSession;

   TInt ret;
   HBufC* contentTypeString = NULL;

   User::LeaveIfError(apaSession.Connect());
   ret = apaSession.RecognizeData(aFileName, aData, dataType);
   apaSession.Close();

   if (ret == KErrNone &&
      (dataType.iConfidence == CApaDataRecognizerType::ECertain) ||
      (dataType.iConfidence == CApaDataRecognizerType::EProbable)) {
      TPtrC8 mimeTypePtr = dataType.iDataType.Des8();
      TInt len = mimeTypePtr.Length() + 1;
      contentTypeString = HBufC::NewL(len);
      contentTypeString->Des().Copy(mimeTypePtr);
      contentTypeString->Des().ZeroTerminate();
   }
   CleanupStack::PushL(contentTypeString);
   return contentTypeString;
}

void
CContentWindowContainer::DataReceived(const isab::DataGuiMess* aMess,
      const char *aUrl)
{
   list<class CResolveItem*>::iterator it = iReqList.begin();
   if (iBrCtlInterface) {
      class CResolveItem* item = NULL;
      while (it != iReqList.end()) {
         item = *it;
         HBufC* url = WFTextUtil::AllocLC(aUrl);
         if (!url->Find(item->getUrl())) {
            CleanupStack::PopAndDestroy(url);
            iReqList.erase(it);
            DataReceived(item->getLinkContent(), aMess, aUrl);
            delete item;
            return;
         }
         CleanupStack::PopAndDestroy(url);
         it++;
      }
   }
   class CResolveItem* item;
   while (it != iReqList.end()) {
      item = *it;
      it++;
      iReqList.pop_front();
      delete item; 
   }
}

_LIT8(KNeck, "\r\n\r\n");
_LIT8(KLineEnd, "\r\n");
_LIT8(KContentTypeMarker, "content-type: ");
_LIT8(KSemiColon, ";");

void
CContentWindowContainer::DataReceived(class MBrCtlLinkContent* aLinkContent,
      const isab::DataGuiMess* aMess, const char *aUrl)
{
   HBufC8* data = NULL;
   HBufC* contentType = NULL;
   HBufC* url = WFTextUtil::AllocLC(aUrl);
   TPtr8 ptr(const_cast<unsigned char*>(aMess->getData()), aMess->getSize(), aMess->getSize());

   TInt neck = ptr.Find(KNeck());
   if (neck == KErrNotFound) {
      data = HBufC8::NewLC( ptr.Length());
      data->Des().Copy(ptr);
      contentType = WFTextUtil::AllocLC("text/html");

   } else {
      TPtrC8 header = ptr.Left(neck);
      TPtrC8 body = ptr.Mid(neck+4);

      data = HBufC8::NewLC( body.Length());
      data->Des().Copy(body);

      TInt pos = header.Find(KLineEnd);
      TPtrC8 rest = header;
      while (pos != KErrNotFound) {
         TPtrC8 tmp = rest.Left(pos);
         rest.Set(rest.Mid(pos+2));
         pos = rest.Find(KLineEnd);
         TInt ctpos = tmp.FindF(KContentTypeMarker);
         if (ctpos != KErrNotFound) {
            TPtrC8 tmp2 = tmp.Mid(ctpos+KContentTypeMarker().Length());
            TInt scpos = tmp2.Find(KSemiColon);
            if (scpos == KErrNotFound) {
               contentType = HBufC::NewLC(tmp2.Length());
               contentType->Des().Copy(tmp2);
            } else {
               contentType = HBufC::NewLC(tmp2.Left(scpos).Length());
               contentType->Des().Copy(tmp2.Left(scpos));
            }
            break;
         }
      }

      if (!contentType) {
         contentType = WFTextUtil::AllocLC("text/html");
      }
   }

/*    contentType = RecognizeLC(*url, ptr); */
/*    contentType = WFTextUtil::AllocLC("text/html"); */


   aLinkContent->HandleResolveComplete(*contentType, KCharSet, data);

   CleanupStack::PopAndDestroy(contentType);
   CleanupStack::PopAndDestroy(data);
   CleanupStack::PopAndDestroy(url);
}

#endif
// End of File  
