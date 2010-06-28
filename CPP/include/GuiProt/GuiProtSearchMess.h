/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GUIPROITSEARCHMESS_H
#define GUIPROITSEARCHMESS_H

namespace isab{
   /**
    *   SerializableClassName description.
    *
    */

   class SearchItem;
   class SearchArea;
   class FullSearchItem;

   class SearchMess : public GuiProtMess
   {
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      SearchMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      SearchMess(uint32 topRegion, const char* areaID, 
                 const char* areaQuery, const char* itemQuery,
                 int32 origLat=MAX_INT32,
                 int32 origLon=MAX_INT32);


      /** Destructor. */
      virtual ~SearchMess();

      /** GuiProtMess interface methods. */
      //@{  
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();  
      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       data getData() const;
      */
      uint32 getTopRegion() const;
      const char* getAreaID() const;
      const char* getAreaQuery() const;
      const char* getItemQuery() const;
      int32 getOrigLat() const;
      int32 getOrigLon() const;
      bool areaSelected() const;
   protected:
      /**
       * Member variables.
       */
      //@{
      uint32 m_topRegion;
      const char* m_areaID;
      const char* m_areaQuery;
      const char* m_itemQuery;
      int32 m_origLat;
      int32 m_origLon;
      //@}

   };


   /**
    *   SearchAreaReplyMess description.
    *
    */
   class SearchAreaReplyMess : public GuiProtMess
   {
   public:
      typedef std::vector<const SearchArea*> SearchArea_list;

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      SearchAreaReplyMess(const SearchArea*const* areas, unsigned num);

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      SearchAreaReplyMess(Buffer* buf);


      /** Destructor. */
      virtual ~SearchAreaReplyMess();

      /** GuiProtMess interface methods. */
      //@{  
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();  
      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       */
      const SearchArea* operator[](int index) const;
      int size() const;
 
   protected:
      /**
       * Member variables.
       */
      //@{
      SearchArea_list m_areas;
      //@}
   private:
      const SearchAreaReplyMess& operator=(const SearchAreaReplyMess& rhs);
      SearchAreaReplyMess(const SearchAreaReplyMess& sarm);

   };

   /**
    *   SearchItemReplyMess description.
    *
    */
   class SearchItemReplyMess : public GuiProtMess
   {
   public:
      typedef std::vector<const SearchItem*> SearchItem_list;

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      SearchItemReplyMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      SearchItemReplyMess(const SearchItem*const* items, unsigned num,
                          uint16 totalHits, uint16 startIndex);

      /** Destructor. */
      virtual ~SearchItemReplyMess();

      /** GuiProtMess interface methods. */
      //@{  
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();  
      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       data getData() const;
      */
      const SearchItem* operator[](unsigned index);
      int size() const;
      uint16 getTotalHits() const;
      uint16 getStartIndex() const;
   protected:
      /**
       * Member variables.
       */
      //@{
      SearchItem_list m_items;
      uint16 m_totalHits;
      uint16 m_startIndex;
      //@}

   }; // SearchItemReplyMess


   /**
    *   FullSearchDataReplyMess description.
    *
    */
   class FullSearchDataReplyMess : public GuiProtMess
   {
   public:
      typedef std::vector<const FullSearchItem*> FullItemList;
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      FullSearchDataReplyMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      FullSearchDataReplyMess(const FullSearchItem * const * items,
                              unsigned num);
      /**
       * Copy constructor.
       */
      FullSearchDataReplyMess(FullSearchDataReplyMess* mess);

      /** Destructor. */
      virtual ~FullSearchDataReplyMess();

      /** GuiProtMess interface methods. */
      //@{  
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       */
      virtual void deleteMembers();  
      /**
       * Writes the data of this message to a buffer.
       *
       * @param buf The buffer to write the message to.
       */
      virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       */
      unsigned size() const;
      const FullSearchItem* operator[](unsigned index) const;
   protected:
      /**
       * Member variables.
       */
      //@{
      FullItemList m_items;
      //@}

   }; // FullSearchDataReplyMess

   class FullSearchDataFromItemIdReplyMess : public FullSearchDataReplyMess
   {
   public:
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      FullSearchDataFromItemIdReplyMess(Buffer* buf) : FullSearchDataReplyMess(buf) {};

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      FullSearchDataFromItemIdReplyMess(const FullSearchItem * const * items,
            unsigned num);

      /**
       * Copy constructor.
       */
      FullSearchDataFromItemIdReplyMess(FullSearchDataFromItemIdReplyMess* mess);

   }; // FullSearchDataFromItemIdReplyMess

/**
  *   GetMoreDataMess description.
  *
  */
   class GetMoreDataMess : public GuiProtMess
   {
   public:
      
      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GetMoreDataMess(Buffer* buf);
      
      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      GetMoreDataMess(uint16 index, const char* value);

      /** Destructor. */
      virtual ~GetMoreDataMess();

      /** GuiProtMess interface methods. */
      //@{  
         /**
          * When calling this method, all members of the message
          * is deleted. I.e. if this method is not called, all
          * the message members must be taken out of the message
          * and be deleted.
          */
         virtual void deleteMembers();  
         /**
          * Writes the data of this message to a buffer.
          *
          * @param buf The buffer to write the message to.
          */
         virtual void serializeMessData(Buffer* buf) const;
      //@}

      /**
       * Returns the data of message. This data is not deleted 
       * by the message destructor.
       *
       * @return
       *
       */
      uint16 getIndex() const;
      const char* getValue() const;
   protected:
      /**
       * Member variables.
       */
      //@{
      uint16 m_index;
      const char* m_value;
      //@}
   private:
      GetMoreDataMess(const GetMoreDataMess& from);
      const GetMoreDataMess& operator=(const GetMoreDataMess& rhs);
   }; // GetMoreDataMess
   
}
#endif
