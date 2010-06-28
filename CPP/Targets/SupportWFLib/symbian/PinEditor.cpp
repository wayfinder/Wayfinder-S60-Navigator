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

#include "PinEditor.h"
#include <barsread.h>
#include <eikenv.h>

CPinEditor::CPinEditor()
{
}

void CPinEditor::ConstructL(TInt aFlags, TInt aMinDigits, TInt aMaxDigits,
                            const TDesC& aStartText, const TDesC& aErrorText)
{
   CEikEdwin::ConstructL(aFlags, aMaxDigits, aMaxDigits, 1);
   SetTextL(&aStartText);
   SetMaxDigits(aMaxDigits);
   SetMinDigits(aMinDigits);
   iErrorMessage = aErrorText.AllocL();
}

void CPinEditor::ConstructL()
{
   const TInt flags = (ENoLineOrParaBreaks | EOnlyASCIIChars );
   ConstructL(flags, 8, 8, KNullDesC, _L("<%d"));
}

void CPinEditor::ConstructFromResourceL(class TResourceReader& aReader)
{
   const TInt flags = (aReader.ReadUint32() | 
                       ENoLineOrParaBreaks | EOnlyASCIIChars);
   const TInt min = aReader.ReadUint16();
   const TInt max = aReader.ReadUint16();
   const TPtrC starttext = aReader.ReadTPtrC();
   const TPtrC errortext = aReader.ReadTPtrC();
   ConstructL(flags, min, max, starttext, errortext);
}

class CPinEditor* CPinEditor::NewL(class CCoeControl& aParent)
{
   class CPinEditor* self = NewLC(aParent);
   CleanupStack::Pop(self);
   return self;
}

class CPinEditor* CPinEditor::NewLC(class CCoeControl& aParent)
{
   class CPinEditor* self = new (ELeave) CPinEditor();
   CleanupStack::PushL(self);
   self->SetContainerWindowL(aParent);
   self->ConstructL();
   return self;
}

CPinEditor::~CPinEditor()
{
}

void CPinEditor::SetMaxDigits(TInt aMax)
{
   iMaxDigits = aMax;
   SetTextLimit(aMax);
   if(iMinDigits > iMaxDigits){ 
      //we cannot let iMinDigits be more than iMaxDigits
      SetMinDigits(aMax);
   }
}

void CPinEditor::SetMinDigits(TInt aMin)
{
   if(aMin > iMaxDigits){
      //we cannot let iMinDigits be more than iMaxDigits
      SetMaxDigits(aMin);
   }
   iMinDigits = aMin;
}

enum TKeyResponse CPinEditor::OfferKeyEventL(const struct TKeyEvent& aKeyEvent,
                                             enum TEventCode aType)
{
   enum TKeyResponse retval = EKeyWasNotConsumed;
   if(aType == EEventKey){         
      TChar code(aKeyEvent.iCode); 
      if((! code.IsPrint()) || code.IsDigit()){
         //let CEikEdwin::OfferKeyEventL handle all EEventKeys that
         //are digits or not printable
         retval = CEikEdwin::OfferKeyEventL(aKeyEvent, aType);
      }
   } else {
      //let CEikEdwin::OfferKeyEventL handle all other keyevents
      retval = CEikEdwin::OfferKeyEventL(aKeyEvent, aType);
   }
   return retval;
}

class TCoeInputCapabilities CPinEditor::InputCapabilities() const
{
   //fetch the inputcapabilities of the superclass
   class TCoeInputCapabilities caps = CEikEdwin::InputCapabilities();
   //read the capabilities bitfield
   TUint bits = caps.Capabilities();
   //turn off all except ENavigation and EWesternNumericIntegerPositive
   bits &= ~(TCoeInputCapabilities::EWesternNumericIntegerNegative |
             TCoeInputCapabilities::EWesternNumericReal |
             TCoeInputCapabilities::EWesternAlphabetic | 
             TCoeInputCapabilities::EJapaneseHiragana | 
             TCoeInputCapabilities::EJapaneseKatakanaHalfWidth | 
             TCoeInputCapabilities::EJapaneseKatakanaFullWidth | 
             TCoeInputCapabilities::EDialableCharacters | 
             TCoeInputCapabilities::ESecretText | 
             TCoeInputCapabilities::EAllText);
   //enable EWesternNumericIntegerPositive just in case
   bits |= TCoeInputCapabilities::EWesternNumericIntegerPositive;
   //set the bitfield
   caps.SetCapabilities(bits);
   return caps;
}

void CPinEditor::PrepareForFocusLossL()
{
   HBufC* text = GetTextInHBufL();
   CleanupStack::PushL(text);
   
   TInt length = text ? text->Length() : 0;

   if(length < iMinDigits){ 
      //to few digits
      if(iErrorMessage && iErrorMessage->Length() > 0){
         //we should check that iErrorMessage contains ONE format
         //specifier that can be used with a TInt argument.
         HBufC* error = HBufC::NewLC(iErrorMessage->Length() + 16);
         error->Des().Format(*iErrorMessage, iMinDigits);
         //show info banner
         CEikonEnv::Static()->InfoMsg(*error);
         //leave. The leave will stop the user from unfocusing this
         //control
         User::Leave(KLeaveWithoutAlert);
         CleanupStack::PopAndDestroy(error);
      } else {
         //_LIT(KPinErrorFmt, "The code must be at least %d digits long");
         User::Leave(KErrUnderflow); //maybe we can find a better code. 
      }
   }

   CleanupStack::PopAndDestroy(text);
}

