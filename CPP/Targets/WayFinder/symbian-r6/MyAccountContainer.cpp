/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// S60 includes
#include <barsread.h>
#include <stringloader.h>
#include <eiklabel.h>
#include <eikenv.h>
#include <gdi.h>
#include <eikseced.h>
#include <aknviewappui.h>
#include <eikappui.h>
#include <aknutils.h>
#include <aknsdrawutils.h>// add this include file skin
#include <aknsbasicbackgroundcontrolcontext.h> //add this include file skin

#include <wayfinder8_s60v5.rsg>
#include "wayfinder.hrh"
#include "WayFinderConstants.h"

#include "MyAccountView.h"
#include "MyAccountContainer.h"

const TInt kXMargin = 10;

/**
 * First phase of Symbian two-phase construction. Should not 
 * contain any code that could leave.
 */
CMyAccountContainer::CMyAccountContainer(RBuf& aUsername) :
   iUsername(aUsername)
{
   iInstructionsLabel = NULL;
   iUserNameLabel = NULL;
   iSecretEditor = NULL;
   iPasswordLabel = NULL;
}

/** 
 * Destroy child controls.
 */
CMyAccountContainer::~CMyAccountContainer()
{
   delete iInstructionsLabel;
   iInstructionsLabel = NULL;
   delete iUserNameLabel;
   iUserNameLabel = NULL;
   delete iSecretEditor;
   iSecretEditor = NULL;
   delete iPasswordLabel;
   iPasswordLabel = NULL;
   iUserNameAndLabel.Close();
   delete iBackground;
}

/**
 * Construct the control (first phase).
 */
CMyAccountContainer*
CMyAccountContainer::NewL(const TRect& aRect, const CCoeControl* aParent,
                          MEikCommandObserver* aCommandObserver,
                          RBuf& aUsername)
{
   CMyAccountContainer* self = CMyAccountContainer::NewLC(aRect, aParent,
         aCommandObserver, aUsername);
   CleanupStack::Pop(self);
   return self;
}

/**
 * Construct the control (first phase).
  */
CMyAccountContainer*
CMyAccountContainer::NewLC(const TRect& aRect, const CCoeControl* aParent,
                           MEikCommandObserver* aCommandObserver,
                           RBuf& aUsername)
{
   CMyAccountContainer* self = new (ELeave) CMyAccountContainer(aUsername);
   CleanupStack::PushL(self);
   self->ConstructL(aRect, aParent, aCommandObserver);
   return self;
}

/**
 * Construct the control (second phase).
 */
void
CMyAccountContainer::ConstructL(const TRect& aRect, const CCoeControl* aParent,
                                MEikCommandObserver* aCommandObserver)
{
   if (aParent == NULL) {
      CreateWindowL();
   } else {
      SetContainerWindowL(*aParent);
   }
   iFocusControl = NULL;
   iCommandObserver = aCommandObserver;
   InitializeControlsL();
   SetRect(aRect);
   iBackground = CAknsBasicBackgroundControlContext::NewL(
         KAknsIIDQsnBgAreaMain, Rect(), EFalse);// new a background
   ActivateL();
}

/**
 * Return the number of controls in the container (override)
 * @return count
 */
TInt
CMyAccountContainer::CountComponentControls() const
{
   return (int) ELastControl;
}

/**
 * Get the control with the given index (override)
 * @param aIndex Control index [0...n) (limited by #CountComponentControls)
 * @return Pointer to control
 */
CCoeControl*
CMyAccountContainer::ComponentControl(TInt aIndex) const
{
   switch (aIndex) {
   case EInstructionsLabel:
      return iInstructionsLabel;
   case EUserNameLabel:
      return iUserNameLabel;
   case ESecretEditor:
      return iSecretEditor;
   case EPasswordLabel:
      return iPasswordLabel;
   }
   return NULL;
}

/**
 *	Handle resizing of the container. This implementation will lay out
 *  full-sized controls like list boxes for any screen size, and will layout
 *  labels, editors, etc. to the size they were given in the UI designer.
 *  This code will need to be modified to adjust arbitrary controls to
 *  any screen size.
 */
void
CMyAccountContainer::SizeChanged()
{
   CCoeControl::SizeChanged();
   LayoutControls();
}

/**
 * Layout components as specified in the UI Designer
 */
void
CMyAccountContainer::LayoutControls()
{
   if (iBackground) {
      iBackground->SetRect(Rect());
   }

   TRect screenRect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, screenRect);

   // find the x values 
   TInt Xstart = kXMargin;
   TInt width = screenRect.Width() - kXMargin - kXMargin;

   // find the y values by spliting the heigth into 15 lines 
   TInt h = screenRect.Height() / 15;

 
   RBuf instructionStringBuf;
   TInt noLines = 0;
   
   // for smaller screens the instructions need to be in a smaller font
   if (screenRect.Height() < 400 )
   {
      // small font for FP2 type devices
      const CFont* legendFont = iEikonEnv->LegendFont();
      iInstructionsLabel->SetFont(legendFont);

      noLines = LayoutInstructions(instructionStringBuf, iInstructionsLabel->Font());
      iInstructionsLabel->SetTextL(instructionStringBuf);

      iUserNameLabel->SetFont(legendFont);
      iPasswordLabel->SetFont(legendFont);
      iSecretEditor->SetExtent(TPoint(Xstart, (noLines + 5) * h), TSize(width, 2 * h));
      iPasswordLabel->SetExtent(TPoint(Xstart, ((noLines + 4) * h)-(h/2)), TSize(width, (2 * h) -5));
  }
   else {
      // Normal font on edition 5 type devices
      const CFont* normalFont = CCoeEnv::Static()->NormalFont();
      iInstructionsLabel->SetFont(normalFont);

      noLines = LayoutInstructions(instructionStringBuf, iInstructionsLabel->Font());
      iInstructionsLabel->SetTextL(instructionStringBuf);

      iUserNameLabel->SetFont(normalFont);
      iPasswordLabel->SetFont(normalFont);
      iSecretEditor->SetExtent(TPoint(Xstart, (noLines + 5) * h), TSize(width, 2 * h));
      iPasswordLabel->SetExtent(TPoint(Xstart, ((noLines + 4) * h)-(h/2)), TSize(width, (2 * h) -5));
    }
   instructionStringBuf.Close();
   
   iInstructionsLabel->SetExtent(TPoint(Xstart, 0), TSize(width, 8 * h));

   iUserNameLabel->SetExtent(TPoint(Xstart, (noLines + 1) * h), TSize(width, 3 * h));

}

/**
 * Handle key events.
 */
TKeyResponse
CMyAccountContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                    TEventCode aType)
{
   if (iFocusControl != NULL && iFocusControl->OfferKeyEventL(aKeyEvent, aType)
         == EKeyWasConsumed) {
      
      // a key event has gone to the editor so if password length >0 enable update LSK
      TBuf<KBuf64Length> passordText;
      iSecretEditor->GetText(passordText);
      CEikButtonGroupContainer* buttonGroupContainer=CEikButtonGroupContainer::Current();

      if (passordText.Length()) {
         buttonGroupContainer->MakeCommandVisible(EAknSoftkeyOk,ETrue);
      }
      else {
         buttonGroupContainer->MakeCommandVisible(EAknSoftkeyOk,EFalse);
      }
      return EKeyWasConsumed;
   }
   return CCoeControl::OfferKeyEventL(aKeyEvent, aType);
}

/**
 * Initialize each control upon creation.
 */
void
CMyAccountContainer::InitializeControlsL()
{
   // Get the current text color for the current skin
   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   TRgb txtColor;
   AknsUtils::GetCachedColor(skin, txtColor, KAknsIIDQsnTextColors,
         EAknsCIQsnTextColorsCG6);

   // block of text containing instructions
   iInstructionsLabel = new (ELeave) CEikLabel;
   iInstructionsLabel->SetContainerWindowL(*this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC(reader,
            R_MY_ACCOUNT_VIEW_INSTRUCTIONS_LABEL);
      iInstructionsLabel->ConstructFromResourceL(reader);
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   iInstructionsLabel->OverrideColorL(EColorLabelText, txtColor);

   iUserNameLabel = new (ELeave) CEikLabel;
   iUserNameLabel->SetContainerWindowL(*this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC(reader,
            R_MY_ACCOUNT_VIEW_USER_NAME_LABEL);
      iUserNameLabel->ConstructFromResourceL(reader);
      CleanupStack::PopAndDestroy(); // reader internal state

      // Read the "Username" string resource
      HBufC* buf = CCoeEnv::Static()->AllocReadResourceLC(R_S60_USER_NAME_TXT);
      iUserNameAndLabel.Create(buf->Length() + 2 + KBuf64Length);
      iUserNameAndLabel.Copy(*buf);
      CleanupStack::PopAndDestroy(buf);

      // add the real username eg "fred" to "Username:" to make "User name:Fred"
      iUserNameAndLabel.Append(_L("\n"));
      iUserNameAndLabel.Append(iUsername);

      iUserNameLabel->SetTextL(iUserNameAndLabel);
   }
   iUserNameLabel->OverrideColorL(EColorLabelText, txtColor);

   // secret editor
   iSecretEditor = new (ELeave) CEikSecretEditor;
   iSecretEditor->SetContainerWindowL(*this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC(reader, R_MY_ACCOUNT_VIEW_SECRET_EDITOR);
      iSecretEditor->ConstructFromResourceL(reader);
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   iSecretEditor->SetMaxLength(KBuf64Length);
   iSecretEditor->SetText(_L(""));
   iSecretEditor->AknSetAlignment(CGraphicsContext::ECenter);
   iSecretEditor->SetBorder( TGulBorder::ESingleBlack );
   iSecretEditor->OverrideColorL(EColorControlText, txtColor);
   iSecretEditor->SetFocus(ETrue);
   iFocusControl = iSecretEditor;
     
   // Password label
   iPasswordLabel = new (ELeave) CEikLabel;
   iPasswordLabel->SetContainerWindowL(*this);
   {
      TResourceReader reader;
      iEikonEnv->CreateResourceReaderLC(reader,
            R_MY_ACCOUNT_VIEW_PASSWORD_LABEL);
      iPasswordLabel->ConstructFromResourceL(reader);
      CleanupStack::PopAndDestroy(); // reader internal state
   }
   iPasswordLabel->OverrideColorL(EColorLabelText, txtColor);

}

/** 
 * Handle global resource changes, such as scalable UI or skin events (override)
 */
void
CMyAccountContainer::HandleResourceChange(TInt aType)
{
   CCoeControl::HandleResourceChange(aType);
   SetRect(iAvkonViewAppUi->View(KMyAccountViewId)->ClientRect());
}

/**
 *	Draw container contents.
 */
void
CMyAccountContainer::Draw(const TRect& aRect) const
{
   // draw background
   CWindowGc& gc = SystemGc();
   MAknsSkinInstance* skin = AknsUtils::SkinInstance();
   MAknsControlContext* cc = AknsDrawUtils::ControlContext(this);
   AknsDrawUtils::Background(skin, cc, this, gc, aRect);
   
   /* add a 1 pixel border around the secret editor */
   TRect border = iSecretEditor->Rect();
   border.iTl.iX -=1;
   border.iTl.iY -=1;
   border.iBr.iX +=1;
   border.iBr.iY +=1;
  
   gc.SetPenStyle(CGraphicsContext::ESolidPen);
   TRgb txtColor;
   AknsUtils::GetCachedColor(skin, txtColor, KAknsIIDQsnTextColors,
         EAknsCIQsnTextColorsCG6);
   gc.SetPenColor(txtColor);
   gc.DrawRect(border);
}

TTypeUid::Ptr
CMyAccountContainer::MopSupplyObject(TTypeUid aId)
{
   if (aId.iUid == MAknsControlContext::ETypeId && iBackground) {
      return MAknsControlContext::SupplyMopObject(aId, iBackground);
   }

   return CCoeControl::MopSupplyObject(aId);
}

TInt CMyAccountContainer::LayoutInstructions(RBuf& aInstructionStringBuf,
                                        const CFont* aFont)
{
   // Read the instruction string from resource into RBuf
   HBufC* txt = CCoeEnv::Static()->AllocReadResourceLC(R_S60_INSTRUCTION_TXT);

   // Find the screen width
   TRect screenRect;
   AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, screenRect);
   TInt screenWidth = screenRect.Width() - kXMargin - kXMargin;

   // Wrap the text to fit
   CArrayFix<TPtrC16>* textArray = new (ELeave) CArrayFixFlat<TPtrC> (10);
   CleanupStack::PushL(textArray);
   AknTextUtils::WrapToArrayL(*txt, screenWidth, *aFont, *textArray);

   // add newline to every line and pack back together into aInstructionStringBuf
   aInstructionStringBuf.Close();
   aInstructionStringBuf.Create(txt->Length() + 10);
   for (TInt i = 0; i < textArray->Count(); i++) {
      aInstructionStringBuf.Append(textArray->At(i));
      aInstructionStringBuf.Append(_L("\n"));
   }

   TInt noLines = textArray->Count();
   CleanupStack::PopAndDestroy(textArray);
   CleanupStack::PopAndDestroy(txt);
   
   // return the number of lines needed
   return noLines;
}

void CMyAccountContainer::GetPassword(RBuf& aPassword)
{
   iSecretEditor->GetText(aPassword);
}
