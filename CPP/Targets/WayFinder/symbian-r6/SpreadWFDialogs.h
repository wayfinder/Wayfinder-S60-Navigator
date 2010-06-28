/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SPREAD_WF_DIALOGS_H
#define SPREAD_WF_DIALOGS_H

#include <e32base.h>
#include <badesca.h>
#include "EventGenerator.h"
#include "ContactBkObserver.h"

class CSpreadWFDialogs : public CBase, 
                         public MContactBkObserver
{
private:
   CSpreadWFDialogs(class MSmsHandler* aSmsSender);
   void ConstructL(const TDesC& aSmsUrl);

public:
   virtual ~CSpreadWFDialogs();
   static CSpreadWFDialogs* NewLC(class MSmsHandler* aSmsSender, const TDesC& aSmsUrl);
   static CSpreadWFDialogs* NewL(class MSmsHandler* aSmsSender, const TDesC& aSmsUrl);

   enum TSpreadWFEvent {
      ESpreadWFEventInfoDialogOk,
      ESpreadWFEventInfoDialogCancel,
      ESpreadWFEventOptionsOk,
      ESpreadWFEventOptionsCancel,
      ESpreadWFEventEnterToOk,
      ESpreadWFEventEnterToCancel,
      ESpreadWFEventContactBkSearchDialogOk, 
      ESpreadWFEventContactBkSearchDialogCancel, 
      ESpreadWFEventContactBkPhoneNumberDialogOk,
      ESpreadWFEventContactBkPhoneNumberDialogCancel,
      ESpreadWFEventSendToOk, 
      ESpreadWFEventSendToCancel
   };

   void GenerateEvent(enum TSpreadWFEvent aEvent);
   void HandleGeneratedEventL(enum TSpreadWFEvent aEvent);

private:
   typedef CEventGenerator<CSpreadWFDialogs, enum TSpreadWFEvent>
      CSpreadWFEventGenerator;
   CSpreadWFEventGenerator* iEventGenerator;
   
public:
   void StartDialogsL();

   // From MContactBkObserver
   virtual void ContactBkSearchDone(TInt aError);
   virtual void ContactBkInUse();

private:
   void Cleanup();
   void ShowSpreadWayfinderOptionsDialogL();
   void ShowSpreadWayfinderEnterToDialogL();
   void ShowSpreadWayfinderSendToDialogL();
   void ShowAllPhoneNumbersDialogL();
   void ShowGenericInfoDialogL(TSpreadWFEvent aOkEvent, 
                               TSpreadWFEvent aCancelEvent, 
                               TInt aHeader, TInt aText, 
                               TInt aResourceId);

   class CContactBkEngine* iContactBkEngine;
   class MSmsHandler* iSmsSender;
   TInt iContactBkIndex;
   CDesCArray* iContactBkNameArray;
   CDesCArray* iContactBkNumberArray;
   TInt iOptionsIndex;
   TBuf<256> iSpreadWFSmsField;
   HBufC* iSmsUrl;
};

#endif
