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
#include <algorithm>
#include "nav2util.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtFavoriteMess.h"
#include "GuiProt/GuiProtSearchMess.h"
#include "GuiProt/GuiProtMapMess.h"
#include "GuiProt/MapClasses.h"

namespace isab{
   using namespace MapEnums;
   //// GetMapMess /////////////////////////////////////////////
#define GETMAPMESS_INITLIST(bb, iW, iH, vbW, vbH, iF, numIt, numInf)    \
   m_bb(bb),                                                            \
   m_imageWidth(iW),                                                    \
   m_imageHeight(iH),                                                   \
   m_viewboxWidth(vbW),                                                 \
   m_viewboxHeight(vbH),                                                \
   m_imageFormat(iF),                                                   \
   m_items(numIt, (MapItem*)NULL),                                      \
   m_infos(numInf,(ExtraMapInfo*)NULL)

   GetMapMess::GetMapMess(Buffer* buf) :
      GuiProtMess(buf), 
      GETMAPMESS_INITLIST(NULL, 0, 0, 0, 0, MapEnums::PNG, 0, 0)
   {
      m_bb = BoundingBox::deserialize(buf);
      m_imageWidth = buf->readNextUnaligned16bit();
      m_imageHeight = buf->readNextUnaligned16bit();
      m_viewboxWidth = buf->readNextUnaligned16bit();
      m_viewboxHeight = buf->readNextUnaligned16bit();
      m_imageFormat = MapEnums::ImageFormat(buf->readNextUnaligned16bit());
      uint16 num = buf->readNextUnaligned16bit();
      m_items.resize(num, NULL);
      for(int i = 0; i < num; ++i){
         m_items[i] = MapItem::deserialize(buf);
      }
      num = buf->readNextUnaligned16bit();
      m_infos.resize(num, NULL);
      for(int j = 0; j < num; ++j){
         m_infos[j] = new ExtraMapInfo(buf);
      }
   }

   GetMapMess::GetMapMess(const BoundingBox& bb,
                          uint16 imageWidth, uint16 imageHeight,
                          uint16 viewBoxWidth, uint16 viewBoxHeight,
                          MapEnums::ImageFormat imageFormat) : 
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::GET_MAP),
      GETMAPMESS_INITLIST(NULL, imageWidth, imageHeight, viewBoxWidth, 
                          viewBoxHeight, imageFormat, 0, 0)
   {
      m_bb = bb.clone();
   }


   GetMapMess::GetMapMess(const BoundingBox& bb, 
                          uint16 imageWidth, uint16 imageHeight,
                          uint16 viewboxWidth, uint16 viewboxHeight,
                          MapEnums::ImageFormat imageFormat,
                          unsigned numItem, const MapItem*const* items,
                          unsigned numInfo, const ExtraMapInfo*const* infos) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::GET_MAP),
      GETMAPMESS_INITLIST(NULL, imageWidth, imageHeight, viewboxWidth, 
                          viewboxHeight, imageFormat, numItem, numInfo)
   {
      m_bb = bb.clone();
      for(unsigned i = 0; i < numItem; ++i){
         m_items[i] = items[i]->clone();
      }
      for(unsigned j = 0; j < numInfo; ++j){
         m_infos[j] = new ExtraMapInfo(*(infos[j]));
      }
   }

   GetMapMess::GetMapMess(const BoundingBox& bb, 
                          uint16 imageWidth, uint16 imageHeight,
                          uint16 viewboxWidth, uint16 viewboxHeight,
                          MapEnums::ImageFormat imageFormat,
                          unsigned numItem, const MapItem* items,
                          unsigned numInfo, const ExtraMapInfo* infos) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::GET_MAP),
      GETMAPMESS_INITLIST(NULL, imageWidth, imageHeight, viewboxWidth, 
                          viewboxHeight, imageFormat, numItem, numInfo)
   {
      m_bb = bb.clone();
      for(unsigned i = 0; i < numItem; ++i){
         m_items[i] = items[i].clone();
      }
      for(unsigned j = 0; j < numInfo; ++j){
         m_infos[j] = new ExtraMapInfo(infos[j]);
      }
   }


   GetMapMess::~GetMapMess()
   {
   }

   void GetMapMess::deleteMembers()
   {
      delete m_bb;
      std::for_each(m_items.begin(), m_items.end(), Delete<MapItem*>());
      std::for_each(m_infos.begin(), m_infos.end(), Delete<ExtraMapInfo*>());
   }

   void GetMapMess::serializeMessData(Buffer* buf) const
   {
      m_bb->serialize(buf);
      buf->writeNextUnaligned16bit(m_imageWidth);
      buf->writeNextUnaligned16bit(m_imageHeight);
      buf->writeNextUnaligned16bit(m_viewboxWidth);
      buf->writeNextUnaligned16bit(m_viewboxHeight);
      buf->writeNextUnaligned16bit(m_imageFormat);
      buf->writeNextUnaligned16bit(m_items.size());
      std::for_each(m_items.begin(), m_items.end(), Serialize(*buf));
      buf->writeNextUnaligned16bit(m_infos.size());
      std::for_each(m_infos.begin(), m_infos.end(), Serialize(*buf));      
   }

   uint16 GetMapMess::getImageWidth() const
   {
      return m_imageWidth;
   }

   uint16 GetMapMess::getImageHeight() const
   {
      return m_imageHeight;
   }

   uint16 GetMapMess::getViewboxWidth() const
   {
      return m_viewboxWidth;
   }

   uint16 GetMapMess::getViewboxHeight() const
   {
      return m_viewboxHeight;
   }

   const BoundingBox* GetMapMess::getBoundingBox() const
   {
      return m_bb;
   }

   MapEnums::ImageFormat GetMapMess::getImageFormat() const
   {
      return m_imageFormat;
   }

   unsigned GetMapMess::numberOfItems() const
   {
      return m_items.size();
   }

   const MapItem* GetMapMess::item(unsigned index) const
   {
      return index < m_items.size() ? m_items[index] : NULL;
   }
   unsigned GetMapMess::numberOfInfos() const
   {
      return m_infos.size();
   }

   const ExtraMapInfo* GetMapMess::info(unsigned index) const
   {
      return index < m_infos.size() ? m_infos[index] : NULL;
   }


   ///// MapReplyMess ////////////////////////////////////////////
#define MAPREPLYMESS_INITLIST(top, left, bottom, right, iW, iH, rww, rwh,   \
                              iF, bufSize)                                  \
      m_topLatitude(top),                                                   \
      m_leftLongitude(left),                                                \
      m_bottomLatitude(bottom),                                             \
      m_rightLongitude(right),                                              \
      m_imageWidth(iW),                                                     \
      m_imageHeight(iH),                                                    \
      m_realWorldWidth(rww),                                                \
      m_realWorldHeight(rwh),                                               \
      m_imageFormat(iF),                                                    \
      m_imageBuffer(bufSize)
   
	MapReplyMess::MapReplyMess(Buffer* buf) :
      GuiProtMess(buf), 
      MAPREPLYMESS_INITLIST(0, 0, 0, 0, 0, 0, 0, 0, MapEnums::PNG, 0)
   {
      m_topLatitude     = buf->readNextUnaligned32bit();
      m_leftLongitude   = buf->readNextUnaligned32bit();
      m_bottomLatitude  = buf->readNextUnaligned32bit();
      m_rightLongitude  = buf->readNextUnaligned32bit();
      m_imageWidth      = buf->readNextUnaligned16bit(); 
      m_imageHeight     = buf->readNextUnaligned16bit();
      m_realWorldWidth  = buf->readNextUnaligned32bit();
      m_realWorldHeight = buf->readNextUnaligned32bit();
      uint32 size       = buf->readNextUnaligned32bit();
      m_imageFormat     = MapEnums::ImageFormat(buf->readNextUnaligned16bit());
      m_imageBuffer.writeNextByteArray(buf->accessRawData(), size);
   }

   MapReplyMess::MapReplyMess(uint32 topLatitude, uint32 leftLongitude,
                              uint32 bottomLatitude, uint32 rightLongitude,
                              uint16 imageWidth, uint16 imageHeight,
                              uint32 realWorldWidth, uint32 realWorldHeight,
                              MapEnums::ImageFormat imageFormat, 
                              unsigned datasize, const uint8* data) :
      GuiProtMess(GuiProtEnums::type_and_data, GuiProtEnums::GET_MAP_REPLY),
      MAPREPLYMESS_INITLIST(topLatitude, leftLongitude, bottomLatitude, 
                            rightLongitude, imageWidth, imageHeight, 
                            realWorldWidth, realWorldHeight, imageFormat, 
                            datasize)
   {
      m_imageBuffer.writeNextByteArray(data, datasize);
   }


   MapReplyMess::~MapReplyMess()
   {
   }

   void MapReplyMess::deleteMembers()
   {
   }

  
   void MapReplyMess::serializeMessData(Buffer* buf) const
   {
      buf->writeNextUnaligned32bit(m_topLatitude);
      buf->writeNextUnaligned32bit(m_leftLongitude);
      buf->writeNextUnaligned32bit(m_bottomLatitude);
      buf->writeNextUnaligned32bit(m_rightLongitude);
      buf->writeNextUnaligned16bit(m_imageWidth); 
      buf->writeNextUnaligned16bit(m_imageHeight);
      buf->writeNextUnaligned32bit(m_realWorldWidth);
      buf->writeNextUnaligned32bit(m_realWorldHeight);
      buf->writeNextUnaligned32bit(m_imageBuffer.getLength());
      buf->writeNextUnaligned16bit(m_imageFormat);
      buf->writeNextBuffer(m_imageBuffer);
   }

   uint16 MapReplyMess::getImageWidth() const
   {   return m_imageWidth;
   }

   uint16 MapReplyMess::getImageHeight() const
   {
      return m_imageHeight;
   }

   uint32 MapReplyMess::getImageBufferSize() const
   {
      return m_imageBuffer.getLength();
   }

   const uint8* MapReplyMess::getImageBuffer(uint32* size) const
   {
      if(size){
         *size = m_imageBuffer.getLength();
      }
      return m_imageBuffer.accessRawData(0);
   }

   MapEnums::ImageFormat MapReplyMess::getImageFormat() const
   {
      return m_imageFormat;
   }

   uint32 MapReplyMess::getTopLat() const
   {
      return m_topLatitude;
   }

   uint32 MapReplyMess::getLeftLon() const
   {
      return m_leftLongitude;
   }

   uint32 MapReplyMess::getBottomLat() const
   {
      return m_bottomLatitude;
   }

   uint32 MapReplyMess::getRightLon() const
   {
      return m_rightLongitude;
   }

   uint32 MapReplyMess::getRealWorldWidth() const
   {
      return m_realWorldWidth;
   }

   uint32 MapReplyMess::getRealWorldHeight() const
   {
      return m_realWorldHeight;
   }

}
