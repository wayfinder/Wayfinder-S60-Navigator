/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUIPROTFAVORITEMESS_H
#define GUIPROTFAVORITEMESS_H
#include "GuiProtMess.h"
namespace isab{

   class Favorite;
   class GuiFavorite;

   /**
    *   GetFavoritesReplyMess description.
    *
    */
   class GetFavoritesReplyMess : public GuiProtMess
   {
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * All Guifavorite objects read from the buffer are copied into
       * memory owned by the new GetFavoritesReplyMess object.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GetFavoritesReplyMess(Buffer* buf);

      /**
       * Use this constructor to crate a message to send.
       *
       * The new GetFavoritesReplyMess keeps a pointer to the
       * argument, but does not copy it.
       *
       * @param guiFavorites The gui favorites to send as data in
       *                     the message. The data is not deleted
       *                     in the message destructor.
       */
      GetFavoritesReplyMess(std::vector<GuiFavorite*>* guiFavorites);

      /** Destructor. Doesn't free any resources. */
      virtual ~GetFavoritesReplyMess();


      /** GuiProtMess interface methods. */
      //@{ 
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       * 
       * Deletes the favorites and frees all memory associated with them. 
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
       * @return A vector with gui favorites.
       */
      std::vector<GuiFavorite*>* getGuiFavorites() const;

   protected:
      /**
       * Member variables.
       */
      //@{
      std::vector<GuiFavorite*>* m_guiFavorites;
      //@}


   }; // GetFavoritesReplyMess


   /**
    *   GetFavoritesAllDataReplyMess description.
    */
   class GetFavoritesAllDataReplyMess : public GuiProtMess
   {
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       *
       * The Favorite objects read from the buffer are copied into
       * memory owned by this object and have to be freed by a call to
       * deleteMembers before the life time of the new object ends.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      GetFavoritesAllDataReplyMess(Buffer* buf);

      /**
       * Use this constructor to crate a message to send.
       *
       * The argument is not copied, so be careful about calling
       * deleteMembers.
       *
       * @param favorites The favorites to send as data in
       *                  the message. The data is not deleted
       *                  in the message destructor.
       */
      GetFavoritesAllDataReplyMess(std::vector<Favorite*>* favorites);

      /** Destructor. Doesn't free any resources.*/
      virtual ~GetFavoritesAllDataReplyMess();


      /** GuiProtMess interface methods. */
      //@{
      /**
       * When calling this method, all members of the message
       * is deleted. I.e. if this method is not called, all
       * the message members must be taken out of the message
       * and be deleted.
       *
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
       * @return A vector with favorites.
       */
      std::vector<Favorite*>* getFavorites() const;

   protected:
      /**
       * Member variables.
       */
      //@{
      std::vector<Favorite*>* m_favorites;
      //@}

   }; // GetFavoritesAllDataReplyMess



   /**
    *   GetFavoriteInfoReplyMess description.
    *
    */
   class GetFavoriteInfoReplyMess : public GuiProtMess
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
      GetFavoriteInfoReplyMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param favorite This favorite should contain the 
       *                 info to send. This favorite is not
       *                 deleted by the message destructor.
       */
      GetFavoriteInfoReplyMess(Favorite* favorite);
      

      /** Destructor. */
      virtual ~GetFavoriteInfoReplyMess();


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
       * Returns the a pointer to the favorite contained in the 
       * message. This data is not deleted  by the message destructor.
       *
       * @return The favorite contained in the message. Use the 
       *         get methods of this favorite to obtain data from it.
       */
      Favorite* getFavorite() const;

      /**
       *  Same as above but transfers ownership.
       *  Favorite is deleted by the caller.
       *  Favorite here is set to null.
       */
      Favorite* getFavoriteD();
 
   protected:
      /**
       * Member variables.
       */
      //@{
      /**
       * This member contains the data of the message.
       */
      Favorite* m_favorite;
      //@}

   }; // GetFavoriteInfoReplyMess


   /**
    *   AddFavoriteMess is used when a favorite is created in the 
    *   GUI.
    *
    */
   class AddFavoriteMess : public GuiProtMess
   {
   public:

      /**
       * Use this constructor to reconstruct the message from
       * a buffer.
       * A Favorite object is created and allocated in dynamic
       * memory. This favorite object is not deleted by the
       * destructor, but by the deleteMembers function.
       *
       * @param buf A buffer which contains this message at the
       *            read position. This position is moved to 
       *            the beginning of next message.
       */
      AddFavoriteMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * The Favorite object used as argument is not copied, but a
       * pointer is kept for the duration of the AddFavoriteMess
       * object. If deleteMembers is called on this object the
       * Favorite object will be deleted.
       * 
       * @param favorite The id of this favorite should be set
       *                 to Favorite::INVALID_FAV_ID.
       *                 The favorite is not deleted in the 
       *                 message destructor.
       */
      AddFavoriteMess(Favorite* favorite);

 

      /** Destructor. Does not delete the Favorite object pointed to
          by m_favorite. */
      virtual ~AddFavoriteMess();

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
       */
      Favorite* getFavorite() const;
       
 
   protected:
      /**
       * Member variables.
       */
      //@{
      Favorite* m_favorite;
      //@}

   }; // AddFavoriteMess

   /**
    *   ChangeFavoriteMess changes a favorite stored in 
    *   Nav2. This favorie is identified by the id of the 
    *   favorite contained in the message. The data of the 
    *   favorite contained in the message set in this favortie
    *   in Nav2.
    *
    */
   class ChangeFavoriteMess : public GuiProtMess
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
      ChangeFavoriteMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param favorite The id of this favorite identyfies the
       *                 favorite to change. The favorite is 
       *                 changed to contain the data of this 
       *                 favorite.
       *                 The favorite is not deleted in the 
       *                 message destructor.
       */
      ChangeFavoriteMess(Favorite* favorite);

      /** Destructor. */
      virtual ~ChangeFavoriteMess();


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
       */
      Favorite* getFavorite() const;
       
   protected:
      /**
       * Member variables.
       */
      //@{
      Favorite* m_favorite;
      //@}


   }; // ChangeFavoriteMess

}
#endif
