/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef MATCHES_H
#define MATCHES_H
#include "Buffer.h"
#include "nav2util.h"
namespace isab{
   class OverviewMatch;

   /** Common superclass for Match and OverviewMatch, since they share most 
    * fields and functions.*/
   class SuperMatch {
   public:
      /** Returns the mapID of the match.
       * @return the mapID.*/
      uint32 getMapID() const;
      /** Returns the item ID of the match.
       * @return the item ID.*/
      uint32 getItemID() const;
      const char* getItemStringID() const;
      bool matchID(const char* idString) const;
      /** Returns the type of the match. The type encodes what kind of
       * map object this match represents, and the value will match
       * one of the values in the Matchtype enum. See also the is
       * Methods.
       * @return the match type.*/
      uint32 getType() const;
      /** Returns the country id of the country this map item is located in.
       * @return the country id.*/
      uint32 getCountryID() const;
      /** Returns the name of this Match.
       * @return the name. */
      const char* getName() const;
      /** The destructor. */ 
      ~SuperMatch();
      /** Returns the number of regions associated with this match.
       * @return the number of regions available through the getRegion method.
       */
      int nbrOfRegions() const;
      /** Returns one of the super regions of this Match. 
       * @i      the index of the region wanted.
       * @return a pointer to a region, or null if i was out of range.*/
      const OverviewMatch* getRegion(int i) const;
      /** 'Packs' this Match so that it looks like matches did before
       * server protocol 5 was introduced. This means that all regions
       * associated with this object are thrown away, but not before
       * their names are concatenated with this matches name. Any
       * Municpal regions name will be parenthesized. */
      void pack();

      /** Tests if this match represents a municipal.
       * @return true if the test is positive.*/
      bool isMunicipal() const;
      /** Tests if this match represents a Built Up Area.
       * @return true if the test is positive.*/
      bool isBuiltUpArea() const;
      /** Tests if this match represents a City Part.
       * @return true if the test is positive.*/
      bool isCityPart() const;
      /** Tests if this match represents a Zip Code.
       * @return true if the test is positive.*/
      bool isZipCode() const;
      /** Tests if this match represents a Zip Area.
       * @return true if the test is positive.*/
      bool isZipArea() const;
      /** Tests if this match represents a Street.
       * @return true if the test is positive.*/
      bool isStreet() const;
      /** Tests if this match represents a Point Of Interest.
       * @return true if the test is positive.*/
      bool isPoi() const;
      /** Tests if this match represents a Category.
       * @return true if the test is positive.*/
      bool isCategory() const;
      /** Tests if this match represents a Misc.
       * @return true if the test is positive.*/
      bool isMisc() const;

      /// Encodes what kind of match something is.
      enum MatchType{
         /// A street. 
         STREET = 0x01,
         /// A point of interest. 
         POI = 0x02,
         /// A category, expand to get categories or companies if leaf
         /// of category tree.
         CATEGORY = 0x04,
         /// A different type of item. Currently parks, lakes and some more. 
         MISC = 0x08,
         /// A municipal. 
         MUNICIPAL = 0x20,
         /// A built up area. 
         BUILT_UP_AREA = 0x40,
         /// A part of a city. 
         CITY_PART = 0x80,
         /// A zip code area. 
         ZIP_CODE = 0x100,
         /// A named zip area. 
         ZIP_AREA = 0x200,
      };


   protected:
      /** SuperMatch constructor that allows setting all fields. 
       * Should be used very rarely.
       * @param mapID     the mapID of the new SuperMatch
       * @param itemID    the itemID of the new SuperMatch
       * @param type      the type of the new SuperMatch
       * @param topRegion the topRegion of the new SuperMatch
       * @param oms       a pointer to a vector of OverviewMatches that will 
       *                  be applied to this SuperMatch. 
       * @param noOms     the number of OverviewMatches in the oms vector.
       * @param name      the mapID of the new SuperMatch
       */
      SuperMatch(uint32 mapID, uint32 itemID, uint32 type, uint32 topRegion, 
                 const OverviewMatch* oms, int noOms, const char* name);
      /** Creates a SuperMatch from the contents of a Buffer.
       * @param buf a buffer to read a serialized SuperMatch from.*/
      SuperMatch(Buffer& buf);
      /** Default constructor.*/
      SuperMatch();
      /** Copyconstructor.
       * @param sm the SuperMatch object to copy.*/
      SuperMatch(const SuperMatch& sm);
      /** Replaces the current name with a new one.
       * @param name the new name.
       * @return the new name.  */
      const char* setName(const char* name);
      /** The id of the map this match can be found in.*/
      uint32 m_mapID;
      /** The id of this map item in the map.*/
      uint32 m_itemID;
      /** The string id of this match.*/
      char* m_itemIDstring;
      /** The type of this match. Should be a value from the Matchtype enum.*/
      uint32 m_type;
      /** The country id of the country this match is located in.*/
      uint32 m_countryID;
      /** Ths number of OverviewMatches available in the m_regions vector. */
      uint32 m_nbrRegions;
      /** The name of this match.*/
      char* m_name;
      /** A pointer to an array of pointer to the OverviewMatches
       * associated with this Match. The array contains m_nbrRegions
       * valid pointers.*/
      OverviewMatch** m_regions;
   private:
      const SuperMatch& operator=(const SuperMatch& rhs);
   };

   class OverviewMatch : public SuperMatch{
   public:
      OverviewMatch(uint32 mapID, uint32 itemID, uint32 type, uint32 topRegion,
                    const OverviewMatch* oms, int noOms, const char* name);
      OverviewMatch(Buffer& buf);
      ~OverviewMatch();
      OverviewMatch(const OverviewMatch& om);
      const OverviewMatch& operator=(const OverviewMatch& rhs);
   private:
   };

   class Match : public SuperMatch{
   public:
      Match(uint32 mapID, uint32 itemID, uint32 type,
            uint32 topRegion, const OverviewMatch* oms, int noOms, 
            int32 lat, int32 lon, const char* name);
      Match(Buffer& buf);
      Match(const Match& match);
      ~Match();
      int32 getLat() const;
      int32 getLon() const;
   private:
      int32 m_lat;
      int32 m_lon;
   private:
      const Match& operator=(const Match& rhs);
   };

   //======================================================================
   //======== Inlines for SuperMatch ======================================
   inline uint32 SuperMatch::getMapID() const
   {
      return m_mapID;
   }

   inline uint32 SuperMatch::getItemID() const
   {
      return m_itemID;
   }

   inline const char* SuperMatch::getItemStringID() const
   {
      return m_itemIDstring;
   }

   inline bool SuperMatch::matchID(const char* idString) const
   {
      return 0 == strcmp(idString, m_itemIDstring);
   }

   inline uint32 SuperMatch::getType() const
   {
      return m_type;
   }

   inline uint32 SuperMatch::getCountryID() const
   {
      return m_countryID;
   }

   inline const char* SuperMatch::getName() const
   {
      return m_name;
   }

   inline int SuperMatch::nbrOfRegions() const
   {
      return m_nbrRegions;
   }

   inline const OverviewMatch* SuperMatch::getRegion(int i) const
   {
      if(i >= 0 && unsigned(i) < m_nbrRegions){
         return m_regions[i];
      }
      return NULL;
   }

   inline SuperMatch::SuperMatch() :
      m_mapID(0), m_itemID(0), m_itemIDstring(NULL), m_type(0), m_countryID(0),
      m_nbrRegions(0), m_name(NULL), m_regions(NULL)
   {
   }

   inline bool SuperMatch::isMunicipal() const
   {
      return m_type == MUNICIPAL;
   }
   inline bool SuperMatch::isBuiltUpArea() const
   {
      return m_type == BUILT_UP_AREA;
   }
   inline bool SuperMatch::isCityPart() const
   {
      return m_type == CITY_PART;
   }
   inline bool SuperMatch::isZipCode() const
   {
      return m_type == ZIP_CODE;
   }
   inline bool SuperMatch::isZipArea() const
   {
      return m_type == ZIP_AREA;
   }
   inline bool SuperMatch::isStreet() const
   {
      return m_type == STREET;
   }
   inline bool SuperMatch::isPoi() const
   {
      return m_type == POI;
   }
   inline bool SuperMatch::isCategory() const
   {
      return m_type == CATEGORY;
   }
   inline bool SuperMatch::isMisc() const
   {
      return m_type == MISC;
   }

   //=====================================================================
   //========= Inlines for OverviewMatch==================================

   inline OverviewMatch::OverviewMatch(uint32 mapID, uint32 itemID, 
                                       uint32 type, uint32 topRegion,
                                       const OverviewMatch* oms,
                                       int noOms, const char* name) :
      SuperMatch(mapID, itemID, type, topRegion, oms, noOms, name)
   {
   }

   inline OverviewMatch::OverviewMatch(const OverviewMatch& om) :
      SuperMatch(om)
   {
   }



   //=====================================================================
   //========= Inlines for Match =========================================

   inline Match::Match(/*uint8 itemType,*/ uint32 mapID, uint32 itemID, 
                       uint32 type, uint32 topRegion, 
                       const OverviewMatch* oms,
                       int noOms, int32 lat, int32 lon, const char* name) :
      SuperMatch(mapID, itemID, type, topRegion, oms, noOms, name), 
      m_lat(lat), m_lon(lon)
   {
   }
   
   inline Match::Match(const Match& match) : 
      SuperMatch(match), m_lat(match.m_lat), m_lon(match.m_lon)
   {
   }
      
   inline int32 Match::getLat() const 
   {
      return m_lat;
   }
   
   inline int32 Match::getLon() const
   {
      return m_lon;
   }
}

#include "GuiProt/HelperEnums.h"
namespace isab {
   inline 
   GuiProtEnums::RegionType MatchType2RegionType(SuperMatch::MatchType type)
   {
      switch(type){
      case SuperMatch::STREET:        return GuiProtEnums::address;
      case SuperMatch::POI:           return GuiProtEnums::invalid;
      case SuperMatch::CATEGORY:      return GuiProtEnums::invalid;
      case SuperMatch::MISC:          return GuiProtEnums::invalid;
      case SuperMatch::MUNICIPAL:     return GuiProtEnums::municipal;
      case SuperMatch::BUILT_UP_AREA: return GuiProtEnums::city;
      case SuperMatch::CITY_PART:     return GuiProtEnums::cityPart;
      case SuperMatch::ZIP_CODE:      return GuiProtEnums::zipcode;
      case SuperMatch::ZIP_AREA:      return GuiProtEnums::zipcode;
      }
      return GuiProtEnums::invalid;
   }

   inline 
   GuiProtEnums::RegionType MatchType2RegionType(unsigned matchtype)
   {
      SuperMatch::MatchType mType = SuperMatch::MatchType(matchtype);
      return MatchType2RegionType(mType);
   }

}

#endif
