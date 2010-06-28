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

#include "HelpUrlHandler.h"
#include "WFTextUtil.h"

#include <eikenv.h>

CHelpUrlHandler::CHelpUrlHandler(TInt aNoOfMappings) : 
   iNoOfMappings(aNoOfMappings)
{
}

void CHelpUrlHandler::ConstructL(const ViewNameMapping* aViewNameMappings, 
                                 TInt aLangCodeId)
{
   iLangCode = CEikonEnv::Static()->AllocReadResourceL(aLangCodeId);
   iLangCode->Des().LowerCase();
   iViewNameMappings = new ViewNameMapping* [iNoOfMappings];
   for (TInt i = 0; i < iNoOfMappings; i++) {
      ViewNameMapping* viewMapping = new ViewNameMapping;
      viewMapping->iViewId = aViewNameMappings[i].iViewId;
      viewMapping->iViewName = WFTextUtil::strdupL(aViewNameMappings[i].iViewName);
      iViewNameMappings[i] = viewMapping;
   }
}

void CHelpUrlHandler::ConstructL(const ViewNameMapping* aViewNameMappings, 
                                 const TDesC& aLangCode)
{
   iLangCode = aLangCode.AllocL();
   iLangCode->Des().LowerCase();
   iViewNameMappings = new ViewNameMapping* [iNoOfMappings];
   for (TInt i = 0; i < iNoOfMappings; i++) {
      ViewNameMapping* viewMapping = new ViewNameMapping;
      viewMapping->iViewId = aViewNameMappings[i].iViewId;
      viewMapping->iViewName = WFTextUtil::strdupL(aViewNameMappings[i].iViewName);
      iViewNameMappings[i] = viewMapping;
   }
}

CHelpUrlHandler::~CHelpUrlHandler()
{
   delete iLangCode;

   if (iViewNameMappings) {
      for (TInt i = 0; i < iNoOfMappings; i++) {
         delete[] iViewNameMappings[i]->iViewName;
         delete iViewNameMappings[i];
      }
      delete[] iViewNameMappings;
   }
}

CHelpUrlHandler* CHelpUrlHandler::NewLC(const ViewNameMapping* aViewNameMappings, 
                                        TInt aNoOfMappings, 
                                        TInt aLangCodeId)
{
   CHelpUrlHandler* self = 
      new (ELeave) CHelpUrlHandler(aNoOfMappings);
   CleanupStack::PushL(self);
   self->ConstructL(aViewNameMappings, aLangCodeId);
   return self;
}

CHelpUrlHandler* CHelpUrlHandler::NewL(const ViewNameMapping* aViewNameMappings, 
                                       TInt aNoOfMappings, 
                                       TInt aLangCodeId)
{
   CHelpUrlHandler* self = 
      CHelpUrlHandler::NewLC(aViewNameMappings, aNoOfMappings, aLangCodeId);
   CleanupStack::Pop(self);
   return self;
}

CHelpUrlHandler* CHelpUrlHandler::NewLC(const ViewNameMapping* aViewNameMappings, 
                                        TInt aNoOfMappings, 
                                        const TDesC& aLangCode)
{
   CHelpUrlHandler* self = 
      new (ELeave) CHelpUrlHandler(aNoOfMappings);
   CleanupStack::PushL(self);
   self->ConstructL(aViewNameMappings, aLangCode);
   return self;
}

CHelpUrlHandler* CHelpUrlHandler::NewL(const ViewNameMapping* aViewNameMappings, 
                                       TInt aNoOfMappings, 
                                       const TDesC& aLangCode)
{
   CHelpUrlHandler* self = 
      CHelpUrlHandler::NewLC(aViewNameMappings, aNoOfMappings, aLangCode);
   CleanupStack::Pop(self);
   return self;
}

HBufC* CHelpUrlHandler::FormatLC(TInt aViewId, const TDesC& aAnchorString)
{
   _LIT(KFile, "file:///");
   _LIT(KHelp, "help");
   _LIT(KIndex, "index");
   _LIT(KHtml, "html");
   _LIT(KSlash, "/");
   _LIT(KDot, ".");
   _LIT(KHash, "#");
   HBufC* pageName = GetHelpPageNameL(aViewId);
   HBufC* url = HBufC::NewLC(KFile().Length() + KHelp().Length() + 
                             KSlash().Length() + KIndex().Length() + 
                             KDot().Length() + iLangCode->Length() + 
                             KDot().Length() + KHtml().Length() + 
                             KHash().Length() + pageName->Length() + 
                             aAnchorString.Length());
   url->Des().Append(KFile);
   url->Des().Append(KHelp);
   url->Des().Append(KSlash);
   url->Des().Append(KIndex);
   url->Des().Append(KDot);
   url->Des().Append(*iLangCode);
   url->Des().Append(KDot);
   url->Des().Append(KHtml);
   url->Des().Append(KHash);
   url->Des().Append(*pageName);
   url->Des().Append(aAnchorString); //nowadays pagename is actually the anchor.
   delete pageName;
   return url;
}

HBufC* CHelpUrlHandler::FormatLC(TInt aViewId, const HBufC* aAnchorString)
{
   HBufC* url;
   if (aAnchorString) {
      url = FormatLC(aViewId, *aAnchorString);
   } else {
      url = FormatLC(aViewId, KNullDesC);
   }
   return url;
}

HBufC* CHelpUrlHandler::FormatL(TInt aViewId, const TDesC& aAnchorString)
{
   HBufC* url = FormatLC(aViewId, aAnchorString);
   CleanupStack::Pop(url);
   return url;
}

HBufC* CHelpUrlHandler::FormatL(TInt aViewId, const HBufC* aAnchorString)
{
   HBufC* url;
   if (aAnchorString) {
      url = FormatLC(aViewId, *aAnchorString);
   } else {
      url = FormatLC(aViewId, KNullDesC);
   }
   CleanupStack::Pop(url);
   return url;
}

HBufC* CHelpUrlHandler::GetHelpPageNameL(TInt aViewId)
{
   for (TInt i = 0; i < iNoOfMappings; i++) {
      if (iViewNameMappings[i]->iViewId == aViewId) {
         return WFTextUtil::AllocL(iViewNameMappings[i]->iViewName);
      }
   }
   //If the view did not have any help page return the index page.
   return WFTextUtil::AllocL(iViewNameMappings[0]->iViewName);
}
