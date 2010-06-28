/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GUI_PROT_FULL_SEARCH_ITEM
#define GUI_PROT_FULL_SEARCH_ITEM

#include "GuiProt/SearchItem.h"

namespace isab {
   ///Forward declaration of a class defined in AdditionalInfo.h
   class AdditionalInfo;

   ///This class holds all data associated with a search result item.
   ///As this class is derived from SearchItem, most of the classes
   ///funcitonality lies there.
   class FullSearchItem : public SearchItem {
   public:
      ///@name Constructors and destructors.
      //@{

      ///Constructor. Arguments name to lon are forwarded to the
      ///matching SearchItem constructor.
      ///@param name       the name of the match. This value is 
      ///                  copied to memory owned by the new object.
      ///@param id         the id string of the match. This value 
      ///                  is copied to memory owned by the new object.
      ///@param type       the type of this match. Should be a value from 
      ///                  the NavServerComEnums::LocationSearchType.
      ///@param subtype    the subtype of this match. Unused?
      ///@param distance   the distance to this match.
      ///@param regions    an array of SearchRegion pointers to 
      ///                  SearchRegion objects associated with this match.
      ///                  The SearchRegion objects are copied to memory 
      ///                  owned by the new object
      ///@param numRegions the size of the regions vector.
      ///@param lat        the latitude of this match.
      ///@param lon        the longitude of this match.
      ///@param alt        the altitude of this match.
      ///@param flags      ?
      ///@param AIbegin    an array of pointers to AdditionalInfo objects
      ///                  associated with this match. The
      ///                  AdditionalInfo objects are copied to memory
      ///                  owned by the new object.  
      ///@param numInfo    the size of the AIbegin vector.
      FullSearchItem(const char* name, const char* id, uint16 type, 
                     uint16 subtype, uint32 distance, 
                     const SearchRegion*const* regions, 
                     unsigned numRegions, int32 lat, int32 lon, 
                     int32 alt, uint32 flags, 
                     const AdditionalInfo*const* AIbegin, 
                     unsigned numInfo);
      ///Constructor. Creates a FullSearchItem object with only part
      ///of the data defined. This is useful in some situations.  The
      ///distance and altitude properties are set to 0, and no regions
      ///or AdditionalInfo objects are added to the new object.  All
      ///of the parameters are handled by the SearchInfo constructor.
      ///@param name       the name of the match. This value is 
      ///                  copied to memory owned by the new object.
      ///@param id         the id string of the match. This value 
      ///                  is copied to memory owned by the new object.
      ///@param type       the type of this match. Should be a value from 
      ///                  the NavServerComEnums::LocationSearchType.
      ///@param subtype    the subtype of this match. Unused?
      ///@param lat        the latitude of this match.
      ///@param lon        the longitude of this match.
      FullSearchItem(const char* name, const char* id, uint16 type, 
                     uint16 subtype, int32 lat, int32 lon);
      ///Constructor for 'fake' FullSearchItems, that only use the
      ///name and coords parts. All other properties are set to 0.
      ///All parameters are handled by the SearchItem constructor.
      ///@param name       the name of the match. This value is 
      ///                  copied to memory owned by the new object.
      ///@param lat        the latitude of this match.
      ///@param lon        the longitude of this match.
      FullSearchItem(const char* name, int32 lat, int32 lon);
      ///Constructor that recreates a previously serialized
      ///FullSearchItem.  The name and id properties, as well as
      ///SearchRegions and AdditionalInfo objects are copied to
      ///storage owned by this object.
      ///@param buf the buffer to read the serialized representation from.
      FullSearchItem(Buffer* buf);
      ///Destructor. Frees memory assocatied with the name and id
      ///properties as well as any SearchRegions and AdditionalInfo
      ///objects owned by this object.
      virtual ~FullSearchItem();
      //@}

      ///@name Derived from Serializable. 
      //@{
      ///Writes a serialized representation
      ///of this object into a Buffer.
      ///@param buf the Buffer to write to.
      virtual void serialize(Buffer* buf) const;
      //@}

      ///The container type used to hold AdditionalInfo objects owned
      ///by this obeject.
      typedef std::vector<AdditionalInfo*> InfoList;
      ///@return the number of AdditionalInfo objects owned by this object.
      unsigned noAdditionalInfo() const;
      ///Get a pointer to an AdditionalInfo object identifed by it's index.
      ///@param index the index to find the AdditionalInfo object.
      ///@return an AdditionalInfo object pointer.
      const AdditionalInfo* getInfo(unsigned index) const;
      ///@return the altitude propety.
      int32 getAlt() const;
      ///@return the flags property.
      uint32 getSearchModuleStatusFlags() const;
      ///Copy constructor. All properties held in dynamic memory will
      ///be copied into memory owned by the new object.
      ///@param rhs the object to copy.
      FullSearchItem(const FullSearchItem& rhs);
      ///Assigment operator. All properties held in dynamic memory
      ///will be copied into memory owned by the assigned to object
      ///All properties owned by this object previous to assignment
      ///are freed.
      ///@param rhs the object to copy.
      ///@return a reference to this object.
      const FullSearchItem& operator=(const FullSearchItem& rhs);
      ///Add a new AdditionalInfo object to the end of the list of
      ///AdditionalInfo objects owned by this object. The
      ///AdditionalInfo object will be copied into memory owned by
      ///this object.
      ///@param ai the AdditionalInfo object to add.
      ///@return the number of AdditionalInfo objects owned by this
      ///        object including the new one.
      InfoList::size_type addInfo(const AdditionalInfo& ai);
   private:
      ///@name Member variables.
      //@{
      ///Holds pointers to this objects AdditionalInfo objects.
      InfoList m_additionalInfo;
      ///The altitude property.
      int32 m_alt;
      ///The flags property.
      uint32 m_searchModuleStatusFlags;
      //@}

   };

   inline FullSearchItem::FullSearchItem(const char* name, int32 lat, 
                                         int32 lon) :
      SearchItem(name, "", 0, 0, 0, NULL, 0, lat, lon), m_additionalInfo(0), 
      m_alt(0), m_searchModuleStatusFlags(0)
   {
   }

}

#endif /* GUI_PROT_FULL_SEARCH_ITEM */
