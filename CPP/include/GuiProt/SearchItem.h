/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_SEARCH_ITEM
#define GUI_PROT_SEARCH_ITEM
#include <vector>
#include "GuiProt/HelperEnums.h"
#include "Serializable.h"
namespace isab {

   ///The SearchRegion class is defined in SearchRegion.h
   class SearchRegion;

   ///hold the information of one Search hit from the server.
   class SearchItem: public Serializable {
   public:
      ///Type for holding The SearchRegions surrounding a SearchItem
      typedef std::vector<const SearchRegion*> RegionList;

      ///Constructor with all fields.
      ///@param name       the name of this SearchItem. Probably matches 
      ///                  the search string.
      ///@param id         the map id of this SearchItem. Useful in further 
      ///                  contacts with the server.
      ///@param type
      ///@param subtype
      ///@param distance   the distance from the current position to this 
      ///                  SearchItem. Should be updated before sending to 
      ///                  Gui. Set to 0 if no position is known.
      ///@param regions    pointer to the vector of SearchRegions that this 
      ///                  SearchItem is situated in. All SearchRegions are 
      ///                  copied into the SearchRegion object.
      ///@param num        the number of SearchRegions in the regions vector.
      ///@param lat        the latitude of this SearchItem.
      ///@param lon        the longitude of this SearchItem.
      ///@param version    the version of this class, if 2 then
      ///                  m_imageName will be serialized. If 1
      ///                  the old format will be used.
      ///@param imageName  Name of the image for this searchItem, the image
      ///                  is the same as a poi image. 
      ///@param advert     If the search item is an advertisement advert is 1.
      SearchItem(const char* name, const char* id, uint16 type,
                 uint16 subtype, uint32 distance, 
                 const SearchRegion* const* regions, unsigned num,
                 int32 lat, int32 lon, uint16 version = 1, 
                 const char* imageName = "", uint8 advert = 0);

      ///Construct a SearchItem object from it's serialized version.
      ///@param buf pointer to a Buffer containing the serialized SearchItem.
      SearchItem(Buffer* buf, uint16 version = 1);
      ///Copy Constructor. Probably best avoided.
      ///@param item the SearchItem to copy.
      SearchItem(const SearchItem& item);

      ///Destructor. Also destoys any contained SearchRegions.
      virtual ~SearchItem();

      ///Write a serialized version of this SearchItem into a Buffer.
      ///@param buf the buffer to write into.
      virtual void serialize(Buffer* buf) const;

      ///Sets the distance value.
      ///@param newDistance the new distance value.
      void setDistance(uint32 newDistance);
      ///Reads the distance value.
      ///@return the current distance value.
      uint32 getDistance() const;

      ///Gets the name of this SearchItem.
      ///@return the name.
      const char* getName() const;
      ///Gets the id of this SearchItem.
      ///@return the id string.
      const char* getID() const;
      ///Gets the type of this SearchItem.
      ///@return the type value of this SearchItem.
      uint16 getType() const;
      uint16 getSubType() const;
      ///Gets the number of SearchRegions contained by this SearchItem.
      ///@return the number of SearchRegions.
      unsigned int noRegions() const;
      ///Returns a SearchRegion.
      ///@param index the index of the SearchRegion to return.
      ///@return a pointer to a SearchRegion. If the index was outside 
      ///        theSearchRegion vector, NULL is returned.
      const SearchRegion* getRegion(unsigned index) const;
      ///Finds a SearchRegion of a certain type amongst this SearchItem's 
      ///SearcgRegions. Searching starts from the beginning unless a valid 
      ///value is supplied to <code>idx</code>. Using the idx variable it's 
      ///possible to find more than one SearchRegion with the same type.
      ///@param type the type to look for.
      ///@param idx  a pointer to a SearchRegion contained by this SearchItem.
      ///            The search starts from the beginning if idx is set to 
      ///            NULL, which is the default value, or to a SearchRegion 
      ///            not contained by this SearchItem 
      ///@return a pointer to the first found SearchRegion of the specified 
      ///        type, or NULL if no such SearchRegion was found.
      const SearchRegion* getRegionOfType(GuiProtEnums::RegionType type,
                                          const SearchRegion* idx= NULL) const;
      int32 getLat() const;
      int32 getLon() const;

      uint16 getVersion() const;
      const char* getImageName() const;
      uint8 getAdvert() const;

      ///Compares this SearchItem to another SearchRegion.
      ///@param rhs the SearchItem to compare with.
      ///@return true if this SearchItem is equal to <code>rhs</code.
      bool operator==(const SearchItem& rhs) const;
      RegionList::size_type addRegion(const SearchRegion& sr);
      const SearchItem& operator=(const SearchItem& rhs);
   private:
      char* m_name;
      char* m_id;
      uint16 m_matchType;
      uint16 m_matchSubType;
      uint32 m_distance;
      RegionList m_regionList;
      int32 m_lat;
      int32 m_lon;
      uint16 m_version;
      char* m_imageName;
      uint8 m_advert;

   };

   inline void SearchItem::setDistance(uint32 newDistance)
   {
      m_distance = newDistance;
   }

   class DistanceLess
   {
   public:
      typedef bool result_type;
      typedef class SearchItem* first_argument_type;
      typedef class SearchItem* second_argument_type;
      result_type operator()(const first_argument_type first,
                               const second_argument_type second)
      {
         return first->getDistance() < second->getDistance() || 
            (first->getDistance() == second->getDistance() && 
             0 > strcmp(first->getName(), second->getName()));
      }
   };




}
#endif /* GUI_PROT_SEARCH_ITEM */
