/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "IAPObject.h"

CIAPObject::CIAPObject() 
{
}

CIAPObject::~CIAPObject() 
{
   delete iImeiNbr;
   delete iIAPNbr;
   delete iIAPName;
}

void CIAPObject::ConstructL(const TDesC& aImeiNbr, 
                            const TDesC& aIAPNbr,
                            const TDesC& aIAPName)
{
   iImeiNbr = aImeiNbr.AllocL();
   iIAPNbr  = aIAPNbr.AllocL();
   iIAPName = aIAPName.AllocL();
}

void CIAPObject::ConstructL()
{
}

class CIAPObject* CIAPObject::NewLC()
{
   class CIAPObject* self = new (ELeave) CIAPObject();
   CleanupStack::PushL(self);
   self->ConstructL();
   return self;
}
class CIAPObject* CIAPObject::NewLC(const TDesC& aImeiNbr, 
                                    const TDesC& aIAPNbr,
                                    const TDesC& aIAPName)
{
   class CIAPObject* self = new (ELeave) CIAPObject();
   CleanupStack::PushL(self);
   self->ConstructL(aImeiNbr, aIAPNbr, aIAPName);
   return self;

}
class CIAPObject* CIAPObject::NewL()
{
   class CIAPObject* self = CIAPObject::NewLC();
   CleanupStack::Pop(self);
   return self;
}
class CIAPObject* CIAPObject::NewL(const TDesC& aImeiNbr, 
                                   const TDesC& aIAPNbr,
                                   const TDesC& aIAPName)
{
   class CIAPObject* self = CIAPObject::NewLC(aImeiNbr, aIAPNbr, aIAPName);
   CleanupStack::Pop(self);
   return self;
}

TPtrC CIAPObject::GetImeiNbr() const
{
   if(!iImeiNbr) {
      return KNullDesC();
   }
   return TPtrC(*iImeiNbr);
}

TPtrC CIAPObject::GetIAPNbr() const
{
   if(!iIAPNbr) {
      return KNullDesC();
   }
   return TPtrC(*iIAPNbr);
}

TPtrC CIAPObject::GetIAPName() const
{
   if(!iIAPName) {
      return KNullDesC();
   }
   return TPtrC(*iIAPName);
}

void CIAPObject::SetImeiNbr(const TDesC& aImeiNbr)
{
   iImeiNbr = aImeiNbr.AllocL();
}

void CIAPObject::SetIAPNbr(const TDesC& aIAPNbr)
{
   iIAPNbr = aIAPNbr.AllocL();
}

void CIAPObject::SetIAPName(const TDesC& aIAPName)
{
   iIAPName = aIAPName.AllocL();
}
