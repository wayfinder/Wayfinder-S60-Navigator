/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <bt_sock.h>
#include "SdpAttributeParser.h"
#include "SdpAttributeNotifier.h"

#include "SdpAttributeParser.pan"

#define ASSERT_SIZE(list, size)                                  \
 __ASSERT_ALWAYS(aNodeList[aNodeListSize].iCommand == EFinished, \
                 User::Panic(_L("SdpParser"), 0))

TSdpAttributeParser::TSdpAttributeParser(const struct SSdpAttributeNode* aNodeList, 
                                         TUint aNodeListSize,
                                         MSdpAttributeNotifier& aObserver) :
   iObserver(&aObserver), iNodeList(aNodeList), iNodeListSize(aNodeListSize),
   iCurrentNodeIndex(0)
{
   //ASSERT_SIZE(aNodeList, aNodeListSize);
   // no implementation required
}

TSdpAttributeParser::TSdpAttributeParser(const struct SSdpAttributeNode* aNodeList, 
                                         TUint aNodeListSize) :
   iObserver(NULL), iNodeList(aNodeList), iNodeListSize(aNodeListSize),
   iCurrentNodeIndex(0)
{
   ASSERT_SIZE(aNodeList, aNodeListSize);
   // no implementation required
}

TInt TSdpAttributeParser::Parse(class CSdpAttrValue& aValue)
{
   TRAPD(err, aValue.AcceptVisitorL(*this));
   return err;
}


TBool TSdpAttributeParser::HasFinished() const
{
   return CurrentNode().iCommand == EFinished;
}

TBool TSdpAttributeParser::ParseComplete() const
{
   return HasFinished();
}

void TSdpAttributeParser::VisitAttributeValueL(class CSdpAttrValue& aValue, 
                                               enum TSdpElementType aType)
{
   switch(CurrentNode().iCommand){
   case ECheckType:
      CheckTypeL(aType);
      break;   
   case ECheckValue:
      CheckTypeL(aType);
      CheckValueL(aValue);
      break;
   case ECheckEnd:
      User::Leave(KErrGeneral);   //  list element contains too many items
      break;
   case ESkip:
      break;  // no checking required
   case EReadValue:
      CheckTypeL(aType);
      ReadValueL(aValue);
      break;
   case EFinished:
      User::Leave(KErrGeneral);   // element is after value should have ended
      return;
   default:
      Panic(ESdpAttributeParserInvalidCommand);
   }
   
   AdvanceL();
}

void TSdpAttributeParser::StartListL(class CSdpAttrValueList& /*aList*/)
{
   // no checks done here
}

void TSdpAttributeParser::EndListL()
{
   // check we are at the end of a list
   if (CurrentNode().iCommand != ECheckEnd){
      User::Leave(KErrGeneral);
   }
   
   AdvanceL();
}

void TSdpAttributeParser::CheckTypeL(enum TSdpElementType aElementType) const
{
   if(CurrentNode().iType != aElementType){
      User::Leave(KErrGeneral);
   }
}

void TSdpAttributeParser::CheckValueL(class CSdpAttrValue& aValue) const
{
   switch(aValue.Type()){
   case ETypeNil:
      Panic(ESdpAttributeParserNoValue);
      break;
   case ETypeUint:
      if (aValue.Uint() != (TUint)CurrentNode().iValue){
         User::Leave(KErrArgument);
      }
      break;
   case ETypeInt:
      if (aValue.Int() != CurrentNode().iValue){
         User::Leave(KErrArgument);
      }
      break;
   case ETypeBoolean:
      if (aValue.Bool() != CurrentNode().iValue){
         User::Leave(KErrArgument);
      }
      break;
   case ETypeUUID:
      if (aValue.UUID() != TUUID(CurrentNode().iValue)){
         User::Leave(KErrArgument);
      }
      break;
      // these are lists, so have to check contents
   case ETypeDES:
   case ETypeDEA:
      Panic(ESdpAttributeParserValueIsList);
      break;
      // these aren't supported - use EReadValue and leave on error
      //case ETypeString:
      //case ETypeURL:
      //case ETypeEncoded:
   default:
      Panic(ESdpAttributeParserValueTypeUnsupported);
      break;
   }
}

void TSdpAttributeParser::ReadValueL(class CSdpAttrValue& aValue) const
{
   if(iObserver){ 
      iObserver->FoundElementL(CurrentNode().iValue, aValue); 
   }
}

const TSdpAttributeParser::SSdpAttributeNode& 
TSdpAttributeParser::CurrentNode() const
{
   return  iNodeList[iCurrentNodeIndex];
}

void TSdpAttributeParser::AdvanceL()
{
   // check not at end
   if (CurrentNode().iCommand == EFinished){
      User::Leave(KErrEof);
   }
   
   // move to the next item
   ++iCurrentNodeIndex;
}

