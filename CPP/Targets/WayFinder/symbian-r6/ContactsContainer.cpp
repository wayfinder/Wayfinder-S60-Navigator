/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// INCLUDE FILES
#include "ContactsContainer.h"
#include "WayFinderConstants.h"

#include <eiklabel.h>  // for example label control
#include "memlog.h"
// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CContactsContainer::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CContactsContainer::ConstructL(const TRect& aRect)
{
   CreateWindowL();

   iLabel = new (ELeave) CEikLabel;
   LOGNEW(iLabel, CEikLabel);
   iLabel->SetContainerWindowL( *this );
   iLabel->SetTextL( _L("Contacts page") );

   SetRect(aRect);
   ActivateL();
}

// Destructor
CContactsContainer::~CContactsContainer()
{
   LOGDEL(iLabel);
   delete iLabel;
}

// ---------------------------------------------------------
// CContactsContainer::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CContactsContainer::SizeChanged()
{
   // TODO: Add here control resize code etc.
   iLabel->SetExtent( TPoint(10,10), iLabel->MinimumSize() );
}

// ---------------------------------------------------------
// CContactsContainer::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CContactsContainer::CountComponentControls() const
{
   return 1; // return nbr of controls inside this container
}

// ---------------------------------------------------------
// CContactsContainer::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CContactsContainer::ComponentControl(TInt aIndex) const
{
   switch ( aIndex )
   {
   case 0:
      return iLabel;
   default:
      return NULL;
   }
}

// ---------------------------------------------------------
// CContactsContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CContactsContainer::Draw(const TRect& aRect) const
{
   CWindowGc& gc = SystemGc();
   // TODO: Add your drawing code here
   // example code...
   gc.SetPenStyle(CGraphicsContext::ENullPen);
   gc.SetBrushColor( TRgb( KBackgroundRed, KBackgroundGreen, KBackgroundBlue ) );
   gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
   gc.DrawRect(aRect);
}

// ---------------------------------------------------------
// CContactsContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CContactsContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                            TCoeEvent /*aEventType*/)
{
   // TODO: Add your control event handler code here
}

// End of File  
