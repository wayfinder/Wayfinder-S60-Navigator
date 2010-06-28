/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/ 

#ifndef __SDP_ATTRIBUTE_PARSER_H__
#define __SDP_ATTRIBUTE_PARSER_H__

#include <btsdp.h>

/*! 
  @class TSdpAttributeParser
  
  @discussion An instance of TSdpAttributeParser is used to check an SDP 
  attribute value, and read selected parts
*/
class TSdpAttributeParser : public MSdpAttributeValueVisitor 
{
public:

   /*!
     @enum TNodeCommand
  
     @discussion The command to be carried out at a node
     @value ECheckType check the type of the value
     @value ECheckValue check the type and the value
     @value ECheckEnd check that a list ends at this point
     @value ESkip do not check this value - can not match a list end
     @value EReadValue pass the value onto the observer
     @value EFinished marks the end of the node list
   */
   enum TNodeCommand
      {
         ECheckType,     
         ECheckValue,  
         ECheckEnd,      
         ESkip,        
         EReadValue,    
         EFinished
      };

   /*! 
     struct SSdpAttributeNode
  
     @discussion An instance of SSdpAttributeNode is used to determine how
     to parse an SDP attribute value data element
   */
   struct SSdpAttributeNode
   {
      /*! @var iCommand the command for the node */
      TNodeCommand iCommand;

      /*! @var iType the expected type */
      TSdpElementType iType;

      /*! @var iValue the expected value for ECheckValue, the value of aKey
        passed to the observer for EReadValue */
      TInt iValue;    
   };

   /*!
     @function TSdpAttributeParser
  
     @discussion Construct a TSdpAttributeParser
     @param aNodeList the list of expected nodes 
     @param aObserver an observer to read specified node values
   */
   TSdpAttributeParser(const struct SSdpAttributeNode* aNodeList, 
                       TUint aNodeListSize,
                       class MSdpAttributeNotifier& aObserver);
   TSdpAttributeParser(const struct SSdpAttributeNode* aNodeList, 
                       TUint aNodeListSize);

   /*!
     @function HasFinished
  
     @discussion Check if parsing processed the whole list
     @result true is the index refers to the EFinished node
   */
   TBool HasFinished() const;
   TBool ParseComplete() const;

   TInt Parse(class CSdpAttrValue& aValue);

public: // from MSdpAttributeValueVisitor

   /*!
     @function VisitAttributeValueL
  
     @discussion Process a data element
     @param aValue the data element 
     @param aType the type of the data element
   */
   void VisitAttributeValueL(class CSdpAttrValue& aValue, 
                             enum TSdpElementType aType);

   /*!
     @function StartListL
  
     @discussion Process the start of a data element list
     @param aList the data element list 
   */
   void StartListL(class CSdpAttrValueList& aList);

   /*!
     @function EndListL
  
     @discussion Process the end of a data element list
   */
   void EndListL();

private:

   /*!
     @function CheckTypeL
  
     @discussion Check the type of the current node is the same as the specified type
     @param aElementType the type of the current data element
   */
   void CheckTypeL(enum TSdpElementType aElementType) const;

   /*!
     @function CheckValueL
  
     @discussion Check the value of the current node is the same as the specified value
     @param aValue the value of the current data element.
   */
   void CheckValueL(class CSdpAttrValue& aValue) const;

   /*!
     @function ReadValueL
  
     @discussion Pass the data element value to the observer
     @param aValue the value of the current data element.
   */
   void ReadValueL(class CSdpAttrValue& aValue) const;

   /*!
     @function CurrentNode
  
     @discussion Get the current node
     @result the current node
   */
   const struct SSdpAttributeNode& CurrentNode() const;

   /*!
     @function AdvanceL
  
     @discussion Advance to the next node. Leaves with KErrEof if at the finished node.
   */
   void AdvanceL();

private:

   /*! @var iObserver the observer to read values */
   class MSdpAttributeNotifier* iObserver;

   /*! @var iNodeList a list defining the expected structure of the value */
   const struct SSdpAttributeNode* iNodeList;
   const TUint iNodeListSize;

   /*! @var iCurrentNodeIndex the index of the current node in iNodeList */
   TInt iCurrentNodeIndex;
};

#endif // __SDP_ATTRIBUTE_PARSER_H__
