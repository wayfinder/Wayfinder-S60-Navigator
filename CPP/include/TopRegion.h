/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TOPREGION_H
#define TOPREGION_H
#include "arch.h"
#include <vector>
#include "Buffer.h"
namespace isab{

   class Log; // forward declaration

   /** This class represents a single top region.*/
   class TopRegion{
   public:
      /** Constructs a new TopRegion object by deserializing the contents of
       * a Buffer.
       * @param buf the buffer to read data from. Data is read from the 
       *        Buffers current readPos, and the readpos is not restored 
       *        once enough data has been read.*/
      TopRegion(Buffer& buf, int convertToUtf8 = 0);
      TopRegion(uint32 id, uint32 type, const char* name);
      ~TopRegion();
      int serialize(Buffer& buf) const;
      uint32 getId() const;
      uint32 getType() const;
      const char* getName() const;
      void log(Log& log) const;
   private:
      uint32 m_id;
      uint32 m_type;
      char* m_name;
   };

   class TopRegionList{
   public:
      TopRegionList(Buffer& buf, int convertToUtf8 = 0);
      TopRegionList();
      int serialize(Buffer& buf) const;
      int size() const;
      const TopRegion* getRegion(unsigned index) const;
      uint32 getVersion() const;
      void log(Log& m_log) const;
      ~TopRegionList();
   private:
      uint32 m_version;
      //      uint32 m_checksum;
      std::vector<TopRegion*> m_regionList;
   };
   
   inline TopRegionList::TopRegionList() :
      m_version(0xffffffff)
   {
   }

   inline uint32 TopRegion::getId() const
   {
      return m_id;
   }

   inline uint32 TopRegion::getType() const
   {
      return m_type;
   }

   inline const char* TopRegion::getName() const
   {
      return m_name;
   }

   inline int TopRegionList::size() const
   {
      return m_regionList.size();
   }

   inline uint32 TopRegionList::getVersion() const
   {
      return m_version;
   }


}
#endif
