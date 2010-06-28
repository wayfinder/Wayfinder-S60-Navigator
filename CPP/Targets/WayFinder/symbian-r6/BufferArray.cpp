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
#include "BufferArray.h"
#include "memlog.h"
// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------
// CBufferArray::CBufferArray()
// Constructor
// Frees reserved resources
// ----------------------------------------------------
//
CBufferArray::CBufferArray(isab::Log* aLog):iLog(aLog)
{
   iElements = NULL;
}

// ----------------------------------------------------
// CBufferArray::~CBufferArray()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CBufferArray::~CBufferArray()
{
   ResetArray();
}

void CBufferArray::InitArray()
{
   if( iElements != NULL )
      ResetArray();
   iElements = new  CArrayPtrFlat<CBufferElement>(4);
   LOGNEW(iElements, CArrayPtrFlat<CBufferElement>);
}

TInt CBufferArray::GetNbrElements()
{
   return iElements->Count();
}

HBufC* CBufferArray::GetElementAt( TInt aIdx )
{
   return ((*iElements)[aIdx])->GetText();
}

void CBufferArray::AddElementL( TDesC &aString, isab::Log* aLog )
{
   CBufferElement* element = new (ELeave) CBufferElement(aLog);
   LOGNEW(element, CBufferElement);
   element->SetTextL( aString );
   iElements->AppendL( element );
}

void CBufferArray::InsertElementL( TDesC &aString, TInt aIdx, isab::Log* aLog )
{
   CBufferElement* element = new (ELeave) CBufferElement(aLog);
   LOGNEW(element, CBufferElement);
   element->SetTextL( aString );
   iElements->InsertL( aIdx, element );
}

void CBufferArray::RemoveElement( TInt aIdx )
{
   iElements->Delete( aIdx );
}

void CBufferArray::ResetArray()
{
   if( iElements != NULL ){
#ifdef MEMORY_LOG
      for(TInt i = 0; i < iElements->Count(); ++i){
         CBufferElement* q = iElements->At(i);
         LOGDEL(q);
         q = q;
      }
#endif
      iElements->ResetAndDestroy();
      LOGDEL(iElements);
      delete iElements;
      iElements = NULL;
   }
}

TBool CBufferArray::IsDuplicate(const TDesC &aString )
{
   TBool dup = EFalse;
   HBufC* cmpStr;

   if( GetNbrElements() > 0 ){
      for( TInt i=0; i < GetNbrElements(); i++ ){
         cmpStr = GetElementAt(i);
         if( aString.Compare( cmpStr->Des() ) == 0 ){
            dup = ETrue;
            break;
         }
      }
   }
   return dup;
}

// Methods from CBufferElement class

CBufferElement::CBufferElement(isab::Log* aLog) : iBuf(NULL), iLog(aLog)
{
}

CBufferElement::~CBufferElement()
{
   LOGDEL(iBuf);
   delete iBuf;
}

void CBufferElement::Clear()
{
   if(iBuf){
      LOGDEL(iBuf);
      delete iBuf;
      iBuf = NULL;
   }
}

void CBufferElement::SetTextL( const TDesC& aText )
{
   if( !iBuf ){
      iBuf  = HBufC::NewL( aText.Length() );
      LOGNEW(iBuf, HBufC);
      *iBuf = aText;	
      return;
   }

   if( aText.Length() > iBuf->Length() )
   {
      LOGDEL(iBuf);
      delete iBuf;
      iBuf  = HBufC::NewL(aText.Length());
      LOGNEW(iBuf, HBufC);
      *iBuf = aText;
   }
   else{
      *iBuf = aText;
      if (aText.Length() < iBuf->Length())
         iBuf  = iBuf->ReAllocL(aText.Length()); // reclaim space
   }
}

HBufC* CBufferElement::GetText() const
{
   return iBuf;
}


// End of File  
