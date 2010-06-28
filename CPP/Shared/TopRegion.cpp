/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "nav2util.h"
#include "TopRegion.h"
#include <vector>
#include "Log.h"
#include "UTF8Util.h"

namespace isab{
   TopRegion::TopRegion(Buffer& buf, int convertToUtf8) :
      m_name(NULL)
   {
      m_id = buf.readNextUnaligned32bit();
      m_type = buf.readNextUnaligned32bit();
      const char* tmp = buf.getNextCharString();
      if(tmp){
         if (convertToUtf8) {
            m_name = UTF8Util::cleanUtf8(tmp);
         } else {
            m_name = strdup_new(tmp);
         }
      }
      //align buffer.
//       buf.jumpReadPos(-4);
//       buf.readNext32bit();
   }

   TopRegion::TopRegion(uint32 id, uint32 type, const char* name) :
      m_id(id), m_type(type), m_name(NULL)
   {
      if(name){
         m_name = new char[strlen(name) + 1];
         strcpy(m_name, name);
      }
   }

   TopRegion::~TopRegion()
   {
      delete[] m_name;
   }

   int TopRegion::serialize(Buffer& buf) const
   {
      int pos = buf.getWritePos();
      buf.writeNextUnaligned32bit(m_id);
      buf.writeNextUnaligned32bit(m_type);
      buf.writeNextCharString(m_name);
      return buf.getWritePos() - pos;
   }

   void TopRegion::log(Log& log) const
   {  
      log.info("id: %0#10"PRIx32" type: %0#10"PRIx32" name: %s",
            m_id, m_type, m_name ? m_name : "(nil)");
   }
   
   TopRegionList::TopRegionList(Buffer& buf, int convertToUtf8) :
      m_regionList()
   {
      m_version = buf.readNextUnaligned32bit();
      int num = buf.readNextUnaligned32bit();
      m_regionList.reserve(num);
      while(num--){
         m_regionList.push_back(new TopRegion(buf, convertToUtf8));
      }
   }

   TopRegionList::~TopRegionList()
   {
      while(!m_regionList.empty()){
         delete m_regionList.back();
         m_regionList.pop_back();
      }
   }

   int TopRegionList::serialize(Buffer& buf) const
   {
      int pos = buf.getWritePos();
      buf.writeNextUnaligned32bit(m_version);
      buf.writeNextUnaligned32bit(m_regionList.size());
      std::vector<TopRegion*>::const_iterator q;
      for(q = m_regionList.begin(); q != m_regionList.end(); ++q){
         (*q)->serialize(buf);
      }
      return buf.getWritePos() - pos;
   }

   const TopRegion* TopRegionList::getRegion(unsigned index) const
   {
      const TopRegion* ret = NULL;
      if(index < m_regionList.size()){
         ret = m_regionList[index];
      }
      return ret;
   }

   void TopRegionList::log(Log& log) const
   {
      log.info(" -- Start of TopRegionList -- ");
      log.info("Checksum: %#010"PRIx32, m_version);
      std::vector<TopRegion*>::const_iterator q;
      for(q = m_regionList.begin(); q != m_regionList.end(); ++q){
         (*q)->log(log);
      }
      log.info(" --  End  of TopRegionList -- ");
   }

}
