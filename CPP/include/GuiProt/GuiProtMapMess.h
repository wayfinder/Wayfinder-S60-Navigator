/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef GUIPROTMAPMESS_H
#define GUIPROTMAPMESS_H
#include "MapEnums.h"
namespace isab{
   using namespace MapEnums;
   class BoundingBox;
   class MapItem;
   class ExtraMapInfo;
   /**
    *   GetMapMess description.
    *
    */
   class GetMapMess : public GuiProtMess
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
      GetMapMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */
      GetMapMess(const BoundingBox& bb, 
                 uint16 imageWidth, uint16 imageHeight,
                 uint16 viewboxWidth, uint16 viewboxHeight,
                 enum ImageFormat imageFormat,
                 unsigned numItem, const MapItem*const* items,
                 unsigned numInfo, const ExtraMapInfo*const* infos);

      GetMapMess(const BoundingBox& bb, 
                 uint16 imageWidth, uint16 imageHeight,
                 uint16 viewboxWidth, uint16 viewboxHeight,
                 enum ImageFormat imageFormat,
                 unsigned numItem, const MapItem* items,
                 unsigned numInfo, const ExtraMapInfo* infos);

      GetMapMess(const BoundingBox& bb,
                 uint16 imageWidth, uint16 imageHeight,
                 uint16 viewBoxWidth, uint16 viewboxHeight,
                 enum ImageFormat imageFormat);

   public:

      /** Destructor. */
      virtual ~GetMapMess();

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
      const BoundingBox* getBoundingBox() const;
      uint16 getImageWidth() const;
      uint16 getImageHeight() const;
      uint16 getViewboxWidth() const;
      uint16 getViewboxHeight() const;
      enum ImageFormat getImageFormat() const;
      unsigned numberOfItems() const;
      const MapItem* item(unsigned index) const;
      unsigned numberOfInfos() const;
      const ExtraMapInfo* info(unsigned index) const;

   protected:
      /**
       * Member variables.
       */
      //@{
      BoundingBox* m_bb;
      uint16 m_imageWidth;
      uint16 m_imageHeight;
      uint16 m_viewboxWidth;
      uint16 m_viewboxHeight;
      enum ImageFormat m_imageFormat;
      std::vector<MapItem*> m_items;
      std::vector<ExtraMapInfo*> m_infos;
      //@}
   }; // GetMapMess

   /**
    *   MapReplyMess description.
    *
    */
   class MapReplyMess : public GuiProtMess
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
      MapReplyMess(Buffer* buf);

      /**
       * Use this constructor to create a message to send.
       *
       * @param data The data is not deleted in the message 
       *             destructor.
       *
       */

      MapReplyMess(uint32 topLatitude, uint32 leftLongitude,
                   uint32 bottomLatitude, uint32 rightLongitude,
                   uint16 imageWidth, uint16 imageHeight,
                   uint32 realWorldWidth, uint32 realWorldHeight,
                   enum ImageFormat imageFormat, unsigned datasize,
                   const uint8* data);

      /** Destructor. */
      virtual ~MapReplyMess();

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
      enum ImageFormat getImageFormat() const;
      uint32 getTopLat() const;
      uint32 getLeftLon() const;
      uint32 getBottomLat() const;
      uint32 getRightLon() const;
      uint16 getImageWidth() const; 
      uint16 getImageHeight() const;
      uint32 getRealWorldWidth() const;
      uint32 getRealWorldHeight() const;
      uint32 getImageBufferSize() const;
      const uint8* getImageBuffer(uint32* size) const;

   protected:
      /**
       * Member variables.
       */
      //@{
      uint32 m_topLatitude;
      uint32 m_leftLongitude;
      uint32 m_bottomLatitude;
      uint32 m_rightLongitude;
      uint16 m_imageWidth; 
      uint16 m_imageHeight;
      uint32 m_realWorldWidth;
      uint32 m_realWorldHeight;
      enum ImageFormat m_imageFormat;
      Buffer m_imageBuffer;
      //@}
   }; // MapReplyMess
}
#endif
