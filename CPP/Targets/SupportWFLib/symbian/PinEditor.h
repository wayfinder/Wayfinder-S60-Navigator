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

#ifndef CPINEDITOR_H
#define CPINEDITOR_H
#include <eikedwin.h>
class CPinEditor : public CEikEdwin
{
   /** @name Constructors and destructor. */
   //@{
   /**
    * Default second phase constructor.  Sets CEikEdwin flags to ...,
    * mindigits to four, maxdigits to 8, starttext to "", and
    * errortext to "<%d".
    */
   void ConstructL();
   /** 
    * Second phase constructor. 
    * @param aFlags Flags passed to CEikEdwin superclass.
    * @param aMinDigits Minimum allowed digits. 
    * @param aMaxDigits Maximum allowed digits.
    * @param aStartText The initial text.
    * @param aErrorText The message shown when PrepareForFocusLossL
    *                   leaves. Must include a %d formatting
    *                   directive.
    */ 
   void ConstructL(TInt aFlags, TInt aMinDigits, TInt aMaxDigits,
                   const TDesC& aStartText,
                   const TDesC& aErrorText);
public:
   /**
    * Constructor. 
    */
   CPinEditor::CPinEditor();
   /**
    * Second phase constructor used when constructed from resource. 
    * @param aReader The resource reader to use to read the resource.
    */
   void ConstructFromResourceL(class TResourceReader& aReader);
   /**
    * Static constructor
    * @param aParent Parent control.
    */
   static class CPinEditor* NewL(class CCoeControl& aParent);
   /**
    * Static constructor
    * @param aParent Parent control.
    */
   static class CPinEditor* NewLC(class CCoeControl& aParent);
   /**
    * Virtual destructor. 
    */
   virtual ~CPinEditor();
   //@}
private:
   /**
    * Sets the maximum allowed digits in this PinEditor. The limit is
    * enforced by the CEikEdwin superclass.
    * @param aMax The maximum number of digits.
    */
   void SetMaxDigits(TInt aMax);
   /**
    * Sets the mimium allowed number of digits in this PinEditor.
    * The limit is enforced by the PrepareForFocusLossL function.
    * @param aMin The minimum number of digits. 
    */
   void SetMinDigits(TInt aMin);

   /**
    * @name From CCoeControl.
    * Also overridden in CEikEdwin.
    */
   //@{
   /**
    * Ignores all printable characters that are not digits. 
    * All non-ignored keys are passed on to CEikEdwin::OfferKeyEventL.
    * @param aKeyEvent
    * @param aType
    * @return 
    */
   virtual enum TKeyResponse OfferKeyEventL(const struct TKeyEvent& aKeyEvent, 
                                            enum TEventCode aType);
   /**
    * Describes what keys are accepted by OfferKeyEventL. Claims to
    * support navigation keys and positive western numeric. It seems
    * this has no effect regarding the keys that are actually received
    * through OfferKeyEventL.
    * @return The TCoeInputCapabilities from
    *         CEikEdwin::InputCapabilities, but with the Capabilities
    *         property set as described above.
    */
   virtual class TCoeInputCapabilities InputCapabilities() const;
   /**
    * Enforces the minimum digits property.
    *
    * This function is called each time a control of this type loses
    * focus. When included in a dialog the control loses focus when
    * the user treis to move to another control or when the dialog
    * tries to close with ok. Cancel will still work by some magic
    * means.
    *
    * If it contains less than iMinDigits characters it will display
    * an info banner and leave with KLeaveWithoutAlert. As long as it
    * leaves focus will not leave this control. 
    */
   virtual void PrepareForFocusLossL();
   //@}

   /** The minum digits required by this control. */
   TInt iMinDigits;
   /** The maximum digits allowed by this control. */
   TInt iMaxDigits;
   /** The "Too few digits" message. */
   HBufC* iErrorMessage;
};

#endif 
