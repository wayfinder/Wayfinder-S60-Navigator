/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/
#ifndef FAVORITE_H
#define FAVORITE_H
#include "arch.h"

#include "GuiProt/Serializable.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/HelperEnums.h"
#include <vector>
#include <map>


namespace isab {

class AdditionalInfo;
   
/**
  *   A GuiFavorite contains a subset of the data contained in 
  *   a Favorite. Use the ID to map the favorite together with 
  *   the favorite object.
  *
  */
class GuiFavorite : public Serializable
{
   public:
      /**
       * Use this constructor to reconstruct the gui favorite
       * from a buffer.
       *
       * The name property IS copied to memory owned by this
       * object.
       *
       * @param buf A buffer which contains this gui favorite 
       *            at the read position. This position is 
       *            moved to after the end of this gui favorite.
       */
      GuiFavorite(class isab::Buffer* buf);

      /**
       * Constructs a gui favorite from the data to be contained.
       *
       * The name property IS copied to memory owned by this object.
       * 
       * @param id   Unique id used for mapping this favorite to a 
       *             Favorite object.
       * @param name The name of the favorite.
       * @param lmsID The name of the landmark store.
       */
      GuiFavorite(uint32 id, const char* name, int32 lat, int32 lon,
                  uint32 lmid = MAX_UINT32, const char* lmsID = NULL);

      /**
       * Constructs a gui favorite from a GuiFavorite.
       *
       * The name property IS copied to memory owned by this object.
       * 
       * @param guiFav  The favorite to be copied.
       */
      GuiFavorite(const GuiFavorite& guiFav);

      /**
       * Max size of a GuiFavorite in serialized form.
       * DO NOT TRUST THIS! Name is 255 chars and them sometimes description
       * is added too, and some extra chars.
       */
#ifndef _MSC_VER
      static const uint32 MAX_GUI_FAV_SIZE =
                                4 + // id
                                255 +  // Favorite::MAX_SIZE_NAME
                                4 +  // lat
                                4 +  // lon
                                4 + // lm id
                                255; // Favorite::MAX_SIZE_LMS_ID
#else
      enum {MAX_GUI_FAV_SIZE =  4 + // id
                                255 + // Favorite::MAX_SIZE_NAME
                                4 +  // lat
                                4 +  // lon
                                4 + // lm id
                                255 // Favorite::MAX_SIZE_LMS_ID
      };
#endif

      /**
       * Destructor. Frees the memory associated with the name propetry.
       */
      virtual ~GuiFavorite();

      /**
       * @name Serializable interface methods.
       */
      //@{
         /**
          * Writes this object to a buffer.
          *
          * @param buf The buffer to write the object to.
          */
         virtual void serialize(class isab::Buffer* buf) const;
      //@}

      /**
       * @return The ID of this object.
       */
      uint32 getID() const;

      /**
       * @return The name of this object.
       */
      const char* getName() const;
  
      /**
       * @return Latitude of the favorites' position. 
       *         [Nav2 coordinates].
       */
      int32 getLat() const;
   
      /**
       * @return Longitude of the favorites' position. 
       *         [Nav2 coordinates].
       */
      int32 getLon() const;

      /**
       * @return The ID of this object.
       */
      uint32 getLmID() const;

      /**
       * @return The ID of this object.
       */
      const char* getLmsID() const;
   

   protected:
      /**
       * Member variables.
       */
      //@{
   /** The id of this favorite. */
         uint32 m_ID;
   /** The name of this favorite. */
         char*  m_name;
   /** The lat of this favorite. */
         int32  m_lat;
   /** The lon of this favorite. */
         int32  m_lon;
   /** The id of the landmark. */
         uint32 m_lmID;
   /** The id of the landmark store. */
         char* m_lmsID;
      //@}

}; // GuiFavorite


/**
  *   Favorite description.
  *
  */
class Favorite : public Serializable
{
   //not implemented
   const class Favorite& operator=(const class Favorite&);
public:
      typedef std::vector< AdditionalInfo* > InfoVect;

      /**
       * Static constants
       */
      //@{
         /**
          * This value means that the id of the favorite has not
          * been set.
          */
      enum { INVALID_FAV_ID = MAX_UINT32 };

         /**
          * This value is the lowest local favorite id that a favorite can 
          * have before it is synced to the server. 
          * (comes from DestinationInternal)
          */
      enum { LOWEST_LOCAL_CLIENT_FAV_ID = (uint32(MAX_INT32) + 1) };

         /**
          * Max size of individual strings in the favorite.
          */
         //@{
            enum { MAX_SIZE_NAME = 255};
            enum { MAX_SIZE_SHORTNAME = 30};
            enum { MAX_SIZE_DESCRIPTION = 255};
            enum { MAX_SIZE_CATEGORY = 255};
            enum { MAX_SIZE_MAPICONPATH = 255};
//             enum { MAX_SIZE_LMS_ID = 255};
            enum { MAX_SIZE_MD5SUM = 16};
         //@}
      //@} // Static constants

   /** @name Constructors and destructor. */
   //{@
      /**
       * Copy constructor. 
       * All fields that are allocated by the original Favorite are
       * allocated by the created object.
       * @param aFav The original Favorite. 
       */
      Favorite(const class Favorite& aFav);

      /**
       * Use this constructor to reconstruct the favorite from
       * a buffer.
       *
       * The five string properties name, shortname, description,
       * category, and mapiconname ARE copied to memory owned by this
       * object.
       *
       * @param buf A buffer which contains this favorite at
       *            the read position. This position is moved
       *            to after the end of this gui favorite.
       * @param convertToUTF8 If non-zero, convert the strings
       *            to UTF8.
       * @param version If infos is present in buf version is 3, we default 
       *            to latest version which is currently 4 (with LMS data).
       */
      Favorite( Buffer* buf, int convertToUTF8 = 0, int version = 4 );

      /**
       * Constructs a favorite from the data to be contained.
       *
       * The five string properties name, shortname, description,
       * category, and mapiconname ARE copied to memory owned by this
       * object.
       *
       * @param id   Unique id used for mapping this favorite to a 
       *             Favorite object.
       * @param lat  Latitude of the favorites' position. 
       *             [Nav2 coordinates].
       * @param lon  Longitude of the favorites' position.
       *             [Nav2 coordinates].
       * @param name The name of the favorite.
       * @param shortName    The short name of the favorite.
       *                     May be "".
       * @param description  The description of the favorite.
       *                     May be "".
       * @param category     The category of the favorite.
       *                     May be "".
       * @param mapIconName  The map icon name of the favorite. 
       *                     May be "".
       * @param lmID         The id of the landmark
       * @param imeiCrcHex   The imei of the current phone crc32 and sprintf(08x)
       * @param lmsID        The id of the landmark store
       *                     May be "".
       * @param md5sum       The md5sum of the favorite
       *                     May be "".
       */
      Favorite(uint32 id, int32 lat, int32 lon,
               const char* name, const char* shortName, 
               const char* description, const char* category,
               const char* mapIconName, bool synced = false,
               uint32 lmID = MAX_UINT32, const char* imeiCrcHex = NULL,
               const char* lmsID = NULL, const uint8* md5sum = NULL);

      /**
       * Constructs a favorite when no id can be assigned, from 
       * the data to be contained.
       *
       * The five string properties name, shortname, description,
       * category, and mapiconname ARE copied to memory owned by this
       * object.
       *
       * @param lat  Latitude of the favorites' position. 
       *             [Nav2 coordinates].
       * @param lon  Longitude of the favorites' position.
       *             [Nav2 coordinates].
       * @param name The name of the favorite.
       * @param shortName    The short name of the favorite.
       *                     May be "".
       * @param description  The description of the favorite.
       *                     May be "".
       * @param category     The category of the favorite.
       *                     May be "".
       * @param mapIconName  The map icon name of the favorite. 
       *                     May be "".
       * @param lmID         The id of the landmark
       * @param imeiCrcHex   The imei of the current phone crc32 and sprintf(08x)
       * @param lmsID        The id of the landmark store
       *                     May be "".
       * @param md5sum       The md5sum of the favorite
       *                     May be "".
       */
      Favorite(int32 lat, int32 lon,
               const char* name, const char* shortName, 
               const char* description, const char* category,
               const char* mapIconName, bool synced = false,
               uint32 lmID = MAX_UINT32, const char* imeiCrcHex = NULL,
               const char* lmsID = NULL, const uint8* md5sum = NULL);

      /**
       * Destructor.
       * 
       * Frees all memory associated with owned strings.
       */
      virtual ~Favorite();
   //@}

   static uint8* calcMd5Sum(const char* name, const char* desc, uint32 lat, uint32 lon); 


      /**
       * Serializable interface methods.
       *
       * @param buf Buffer to add data to.
       * @param version If version 3 add infos to buf, default to latest version, 
       *                which is currently 4 (containing lms data).
       */
      //@{
      virtual void serialize( Buffer* buf ) const { serialize( buf, 4 ); }
      virtual void serialize( Buffer* buf, int version ) const;
      //@}


      /**
       * Tells whether this favorite has been synced
       * with the server, i.e. its id is server assigned.
       */
      bool isSynced() const;

      /**
       * Truncates the strings of the favorite if they are
       * to long to be stored in MC2. Uses the variables 
       * MAX_SIZE... included in this class to determine if
       * a string is to long.
       *
       * @return Returns true if one or more strings were 
       *         truncated, otherwise false.
       */
      bool truncateStrings();

      /**
       * Get methods.
       */
      //@{
         /**
          * Allocates and returns a GuiFavorite, which is used for 
          * display. This favorite containes less data than a usual 
          * favorite.
          *
          * NB! If the short name exists, the name of the returned
          *     GuiFavorite contains the short name. Otherwise, 
          *     the name of the GuiFavorite contains the name.
          *
          * @return A GuiFavorite containing a subset of the data
          *         from the Favorite. This object should be 
          *         deallocated by the caller.
          */
         GuiFavorite* getGuiFavorite() const;

         
         /**
          * @retrun The ID of this favorite. If no id has been 
          *         assigned for the favorite, 
          *         Favorite::INVALID_FAV_ID is returned.
          */
         uint32 getID() const;

         /**
          * @return Latitude of the favorites' position. 
          *         [Nav2 coordinates].
          */
         int32 getLat() const;

         /**
          * @return Longitude of the favorites' position. 
          *         [Nav2 coordinates].
          */
         int32 getLon() const;

         /**
          * @return The name of the favorite.
          */
         const char* getName() const;

         /**
          * @return The short name of the favorite.
          *         May be "".
          */
         const char* getShortName() const;

         /**
          * @return The description of the favorite.
          *         May be "".
          */
         const char* getDescription() const;

         /**
          * @return The category of the favorite.
          *         May be "".
          */
         const char* getCategory() const;

         /**
          * @return The map icon name of the favorite.
          *         May be "".
          */
         const char* getMapIconName() const;

         /**
          * @return The landmark id.
          */
         uint32 getLmID() const;

         /**
          * @return The landmark store id.
          */
         const char* getLmsID() const;

         /**
          * @return The md5sum of the favorite.
          */
         const uint8* getMd5sum() const;

         /** 
          * Get the info entries for the favorite.
          *
          * @return The info entries for the favorite.
          */
         const InfoVect& getInfos() const;
      //@}

      /**
       * If favorite has any info field of a type.
       */
      bool hasInfoType( GuiProtEnums::AdditionalInfoType type ) const;

      /**
       * Set methods.
       */
      //@{
         /**
          * Use this method to assign an id to a favorite
          * that has been created with no ID.
          *
          * @param The id to set the id of this favorite to.
          */
         void setID( uint32 id );

         /**
          * @param lat Latitude of the favorites' position. 
          *         [Nav2 coordinates].
          */
         void setLat( int32 lat );

         /**
          * @param lon Longitude of the favorites' position. 
          *         [Nav2 coordinates].
          */
         void setLon( int32 lon );

         /**
          * @param name The name of the favorite.
          */
         void setName( const char* name );

         /**
          * @param sname The short name of the favorite.
          *         May be "".
          */
         void setShortName( const char* sname );

         /**
          * @param desc The description of the favorite.
          *         May be "".
          */
         void setDescription( const char* desc );

         /**
          * @param cat The category of the favorite.
          *         May be "".
          */
         void setCategory( const char* cat );

         /**
          * @param mapIcon The map icon name of the favorite.
          *         May be "".
          */
         void setMapIconName( const char* mapIcon );

         /**
          * Tells whether this favorite has been synced
          * with the server, i.e. its id is server assigned.
          * Use with care you might get duplicates.
          *
          * @param sync New value.
          */
         void setSynced( bool sync );

         /**
          * @param lmID The landmark id.
          * @param imeiCrcHex, The imei of the current phone crc32 and sprintf(08x)
          */
         void setLmID( uint32 lmID, const char* imeiCrcHex );

         /**
          * @param lmsID The landmark store id.
          */
         void setLmsID( const char* lmsID );

         /**
          * @param md5sum The md5sum of the favorite.
          */
         void setMd5sum( const uint8* md5sum );

      //@}

      /**
       * Add methods.
       */
      //@{
         /**
          * Add a AdditionalInfo.
          *
          * @param a The AdditionalInfo to add, this class takes ownership
          *          of object.
          */
         void addAdditionalInfo( AdditionalInfo* a );
      //@}


      /**
       * Get the size of the favorite packed into a Buffer.
       */
      uint32 getSize() const;

      bool isMd5sumSet() const;


   protected:
      /**
       * Member variables.
       */
      //@{
         uint32 m_ID;
         int32  m_lat;
         int32  m_lon;
         char*  m_name;
         char*  m_shortName;
         char*  m_description;
         char*  m_category;
         char*  m_mapIconName;
         uint32 m_lmID;
         char*  m_lmsID;
         uint8* m_md5sum;

         InfoVect m_infos;

         /**
          * Tells whether this favorite has been synced
          * with the server, i.e. its id is server assigned.
          */
         bool m_synced;
      //@}

      /**
       * Protected methods.
       */
      //@{
         /**
          * Called by all constructors.
          */
         void initMembers();

      //@}


   //This function is actually a global function, but declared only
   //here as a part of the friend statement.
   /**
    * Creates a new Favorite object. 
    * The new object is created in dynamic memory.
    * If any of the allocations involved fails, the function returns NULL.
    * @param aFav The object that shall be copied.
    * @return A copy of the argument. 
    */
   friend Favorite* CloneFavorite(const class Favorite& aFav);

}; // Favorite



/**
 * Function object used to sort favorites according to different
 * criteria.
 * The supported sorting criteria are:
 * <ul>
 * <li>alphabeticalOnName: use strcasecmp on the m_name member of the
 *     Favorite. Note that this sort doesn't care whether the string
 *     is ASCII, Latin-1, or UTF-8</li>
 * <li>newSort: Uses UTF8Util::strCollateUtf8 on the m_name
 *     member. Note that this sort requires utf-8 encoding.</li>
 * </ul>
 */
class FavoriteCmp
{
public:
   /**
    * Constructor. 
    * @param sortOrder The sort used. Note that FavoriteCmp only
    *                  supports two of the sortings specified in the
    *                  SortingType enum; alphabeticalOnName and
    *                  newSort. If another sort is specified
    *                  alphabeticalOnName will be used.
    */
   FavoriteCmp(enum isab::GuiProtEnums::SortingType sortOrder);
   /**
    * The comparison function. Implements (first < second).
    * @param first The left side part of the less-than comparison.
    * @param second The right side part of the less-than comparison.
    * @return true if <code>*first < *second</code> according to the
    *         sort criteria.
    */
   bool operator()(const class isab::Favorite* first,
                   const class isab::Favorite* second) const;

private:
   /** Store the sorting criteria. */
   enum isab::GuiProtEnums::SortingType  m_sortingOrder;
};


/**
 * Function object used to sort GuiFavorites according to different
 * criteria.
 * The supported sorting criteria are:
 * <ul>
 * <li>alphabeticalOnName: use strcasecmp on the m_name member of the
 *     Favorite. Note that this sort doesn't care whether the string
 *     is ASCII, Latin-1, or UTF-8</li>
 * <li>newSort: Uses UTF8Util::strCollateUtf8 on the m_name
 *     member. Note that this sort requires utf-8 encoding.</li>
 * </ul>
 */
class GuiFavoriteCmp
{
public:
   /**
    * Constructor. 
    * @param sortOrder The sort used. Note that FavoriteCmp only
    *                  supports two of the sortings specified in the
    *                  SortingType enum; alphabeticalOnName and
    *                  newSort. If another sort is specified
    *                  alphabeticalOnName will be used.
    */
   GuiFavoriteCmp(enum isab::GuiProtEnums::SortingType sortOrder);
   /**
    * The comparison function. Implements (first < second).
    * @param first The left side part of the less-than comparison.
    * @param second The right side part of the less-than comparison.
    * @return true if <code>*first < *second</code> according to the
    *         sort criteria.
    */
   bool operator()(const class isab::GuiFavorite* first,
                   const class isab::GuiFavorite* second) const;

private:
   /** Store the sorting criteria. */
   enum isab::GuiProtEnums::SortingType  m_sortingOrder;
};

/**
 * Function object used to sort favorite's info fields.
 */
class FavoriteInfoCmp {
   public:
      /**
       * Constructor. sortOrder might be added if needed.
       */
      FavoriteInfoCmp();

      /**
       * The comparison function. Implements (first < second).
       * 
       * @param first The left side part of the less-than comparison.
       * @param second The right side part of the less-than comparison.
       * @return true if <code>*first < *second</code>.
       */
      bool operator()( const class isab::AdditionalInfo* first,
                       const class isab::AdditionalInfo* second ) const;

   private:
      typedef std::map< uint32, uint32 > prioMap;
      /**
       * Prioritized types. 
       */
      prioMap m_prios;
};

struct FavLmIDCmp : public std::binary_function<Favorite*, Favorite*, bool>
{
   bool operator()(const Favorite* fav, const Favorite* fav2) const {
      return fav->getLmID() == fav2->getLmID();
   }
};


} // namespace isab

#endif // FAVORITE_H.

