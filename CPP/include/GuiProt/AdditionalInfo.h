/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_ADDITIONAL_INFO
#define GUI_PROT_ADDITIONAL_INFO

#include "Serializable.h"

namespace isab {
   ///This class holds a single Additional Info item as part of a
   ///FullSearchItem. Each Additional Info item consist of a key-value
   ///pair. Both key and value are strings, and could be presented to
   ///the user as <key>: <value>. There is also a type property which
   ///should be one of the values from
   ///GuiProtEnums::AdditionalInfoType. The type could specify display
   ///modes or actions on an item.
   class AdditionalInfo : public  Serializable{
   public:
      ///@name Constructors and destructors.
      //@{
      ///Constructor.
      ///@param key   the key value. This value is copied.
      ///@param value the value value. This value is copied. 
      ///@param type the type. A value from
      ///       GuiProtEnums::AdditionalInfoType.
      AdditionalInfo(const char* key, const char* value, uint32 type);
      ///Constructor. Constructs an object from a buffer containing a
      ///serialized representation of an AdditionalInfo object.
      ///@param buf a buffer containing a serialized representation of
      ///           an AdditionalInfo object
      AdditionalInfo(Buffer* buf);
      ///Copy Constructor. This copy constructor also creates copies
      ///of all member variables.
      ///@param rhs the object to copy.
      AdditionalInfo(const AdditionalInfo& rhs);
      ///Destructor. Destorys any owned strings. 
      virtual ~AdditionalInfo();
      //@}

      ///@name Derived from Serializable. 
      //@{
      ///Writes a serialized representation
      ///of this object into a Buffer.
      ///@param buf the Buffer to write to.
      virtual void serialize(Buffer* buf) const;
      //@}
      
      /**
       * Get the size of the object packed into a Buffer.
       */
      uint32 getSize() const;

      ///@name Properity functions.
      //@{
      ///@return this AdditionalInfo objects type value. 
      uint32 getType() const;
      ///@return this AdditionalInfo objects key value. Note that this
      ///        is a pointer to this objects local copy of the string.
      const char* getKey() const;
      ///@return this AdditionalInfo objects value value. Note that this
      ///        is a pointer to this objects local copy of the string.
      const char* getValue() const;
      /**
       * Sets the value of this instance.
       * @param value, the new value.
       */ 
      void setValue(const char* value) ;

      //@}
      ///@name Comparison operators.
      //@{
      ///Equality operator.
      ///@param rhs the object to compare to.
      ///@return true if all properties of this object are equal to
      ///        those of rhs.
      bool operator==(const AdditionalInfo& rhs) const;
      //@}
   private:
      ///@name Property varibles.
      //@{
      ///The type. Shold be a value from GuiProtEnums::AdditionalInfoType.
      uint32 m_type;
      ///The key property.
      char* m_key;
      ///The value property
      char* m_value;
      //@}
      ///The assignment operator is declared private and can not be
      ///used.
      const AdditionalInfo& operator=(const AdditionalInfo& rhs);
   };
}

#endif /* GUI_PROT_ADDITIONAL_INFO */
