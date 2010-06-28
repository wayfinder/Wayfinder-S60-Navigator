/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include "arch.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"

using std::vector;

namespace isab{
   // GetFavoritesReplyMess///////////////////////////////////
   GetFavoritesReplyMess::GetFavoritesReplyMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_guiFavorites = new vector<GuiFavorite*>();
      uint16 nbrOfFavorites = buf->readNextUnaligned16bit();
      for (uint16 i = 0; i < nbrOfFavorites; i++){
         m_guiFavorites->push_back(new GuiFavorite(buf));
      }
   }

   GetFavoritesReplyMess::~GetFavoritesReplyMess()
   {
      // Do not delete m_guiFavorites
   }

   void
   GetFavoritesReplyMess::deleteMembers()
   {
      for (uint16 i = 0; i<m_guiFavorites->size(); i++){
         delete (*m_guiFavorites)[i];
      }
      delete m_guiFavorites;
      m_guiFavorites = NULL;
   }

   void 
   GetFavoritesReplyMess::serializeMessData(Buffer* buf) const
   {
      uint16 nbrOfFavorites = m_guiFavorites->size();
      buf->writeNextUnaligned16bit(nbrOfFavorites);
      for (uint16 i = 0; i < nbrOfFavorites; i++){
         (*m_guiFavorites)[i]->serialize(buf);
      }
   } // serializeMessData 

   GetFavoritesReplyMess::GetFavoritesReplyMess(vector<GuiFavorite*>* guiFavorites) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::GET_FAVORITES_REPLY)
   {
      m_guiFavorites = guiFavorites;
   }

   vector<GuiFavorite*>* 
   GetFavoritesReplyMess::getGuiFavorites() const
   {
      return m_guiFavorites;
   }












   // GetFavoritesAllDataReplyMess///////////////////////////////////
   GetFavoritesAllDataReplyMess::GetFavoritesAllDataReplyMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_favorites = new vector<Favorite*>();
      uint16 nbrOfFavorites = buf->readNextUnaligned16bit();
      for (uint16 i = 0; i < nbrOfFavorites; i++){
         m_favorites->push_back(new Favorite(buf));
      }
   }

   GetFavoritesAllDataReplyMess::~GetFavoritesAllDataReplyMess()
   {
      // Do not m_favorites.
   }

   void
   GetFavoritesAllDataReplyMess::deleteMembers()
   {
      for (uint32 i = 0; i<m_favorites->size(); i++){
         delete (*m_favorites)[i];
      }
      delete m_favorites;
      m_favorites = NULL;
   }

   void 
   GetFavoritesAllDataReplyMess::serializeMessData(Buffer* buf) const
   { 
      uint16 nbrOfFavorites = m_favorites->size();
      buf->writeNextUnaligned16bit(nbrOfFavorites);
      for (uint16 i = 0; i < nbrOfFavorites; i++){
         (*m_favorites)[i]->serialize(buf);
      }
   } // serializeMessData 

   GetFavoritesAllDataReplyMess::GetFavoritesAllDataReplyMess(vector<Favorite*>* favorites) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::GET_FAVORITES_ALL_DATA_REPLY)
   {
      m_favorites = favorites;
   }

   vector<Favorite*>* 
   GetFavoritesAllDataReplyMess::getFavorites() const
   {
      return m_favorites;
   }

















   // GetFavoriteInfoReplyMess///////////////////////////////////
   GetFavoriteInfoReplyMess::GetFavoriteInfoReplyMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_favorite = new Favorite(buf);
   }

   GetFavoriteInfoReplyMess::~GetFavoriteInfoReplyMess()
   {
      // Do not delete the favorite.
   }

   void
   GetFavoriteInfoReplyMess::deleteMembers()
   {
      delete m_favorite;
      m_favorite = NULL;
   }

   void 
   GetFavoriteInfoReplyMess::serializeMessData(Buffer* buf) const
   {
      m_favorite->serialize(buf);
   }


   GetFavoriteInfoReplyMess::GetFavoriteInfoReplyMess(Favorite* favorite) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::GET_FAVORITE_INFO_REPLY)
   {
      m_favorite = favorite;
   }

   Favorite* 
   GetFavoriteInfoReplyMess::getFavorite() const
   {
      return m_favorite;
   }

   Favorite* 
      GetFavoriteInfoReplyMess::getFavoriteD() {
      Favorite* f = m_favorite;
      m_favorite = NULL;
      return f;
   }















   // AddFavoriteMess///////////////////////////////////
   AddFavoriteMess::AddFavoriteMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_favorite = new Favorite(buf);
   }

   AddFavoriteMess::~AddFavoriteMess()
   {
      // Do not delete m_favorite.
   }

   void
   AddFavoriteMess::deleteMembers()
   {
      delete m_favorite;
      m_favorite = NULL;
   }

   void 
   AddFavoriteMess::serializeMessData(Buffer* buf) const
   {
      m_favorite->serialize(buf);
   } // serializeMessData 

   AddFavoriteMess::AddFavoriteMess(Favorite* favorite) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::ADD_FAVORITE )
   {
      m_favorite = favorite;
   }

   Favorite*
   AddFavoriteMess::getFavorite() const
   {
      return m_favorite;
   }














   // ChangeFavoriteMess///////////////////////////////////
   ChangeFavoriteMess::ChangeFavoriteMess(Buffer* buf) :
      GuiProtMess(buf)
   {
      m_favorite = new Favorite(buf);
   }

   ChangeFavoriteMess::~ChangeFavoriteMess()
   {
      // Do not delete m_favorite.
   }

   void
   ChangeFavoriteMess::deleteMembers()
   {
      delete m_favorite;
      m_favorite = NULL;
   }

   void 
   ChangeFavoriteMess::serializeMessData(Buffer* buf) const
   {
      m_favorite->serialize(buf);
   } // serializeMessData 

   ChangeFavoriteMess::ChangeFavoriteMess(Favorite* favorite) :
      GuiProtMess( GuiProtEnums::type_and_data, 
                   GuiProtEnums::CHANGE_FAVORITE)
   {
      m_favorite = favorite;
   }

   Favorite*
   ChangeFavoriteMess::getFavorite() const
   {
      return m_favorite;
   }
}
