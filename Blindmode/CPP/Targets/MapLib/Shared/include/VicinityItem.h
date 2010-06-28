/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VICINITY_ITEM_H
#define VICINITY_ITEM_H

#include "MC2SimpleString.h"
#include "MC2Coordinate.h"
#include <vector>

#include "VicinityVisitor.h"
#include "MC2Direction.h"

#include "FeatureType.h"

class VicinityItem;
typedef std::vector<VicinityItem*> vicVec_t;

class UserDefinedFeature;

class VicinityItem {
public:

   enum VicinityType { VI_POI = 0, VI_UDF, VI_CROSSING, VI_ALL};

   VicinityItem() : 
      m_distance(0), 
      m_name(""),
      m_serverName(""),
      m_coord(0, 0)
      {}

   VicinityItem(int distance, 
                const MC2SimpleString& name,
                const MC2SimpleString& serverName,
                const MC2Coordinate& coord,
                const MC2Direction& direction) : 
      m_distance(distance),
      m_name(name),
      m_serverName(serverName),
      m_coord(coord),
      m_direction(direction) 
   {}
   
   VicinityItem(int distance, 
                const MC2SimpleString& name,
                const MC2SimpleString& serverName,
                const MC2Coordinate& coord,
                const MC2Direction& direction,
                const MC2SimpleString& categoryName) : 
      m_distance(distance),
      m_name(name),
      m_serverName(serverName),
      m_coord(coord),
      m_direction(direction),
      m_categoryName(categoryName)
   {}
   
   virtual ~VicinityItem() {}
   
   static void ClearAndDelete(vicVec_t& vicVec) {
      for(vicVec_t::iterator vc = vicVec.begin();
          vc!=vicVec.end();
          vc++)
      {
         delete *vc;
      }

      vicVec.clear();
   }

   virtual void Accept(VicinityVisitor* visitor) {
      visitor->Visit(this);
   }
   
   const MC2SimpleString& Name() const {
      return m_name;
   }

   inline const MC2SimpleString& ServerName() const {
      return m_serverName;
   }

   inline const MC2SimpleString& CategoryName() const {
      return m_categoryName;
   }
   
   inline int Distance() const {
      return m_distance;
   }

   inline const MC2Coordinate& Coord() const {
      return m_coord;
   }

   inline const MC2Direction& Direction() const {
      return m_direction;
   }

   virtual VicinityType Type() const {
      return VI_POI;
   }
   
protected:
   int               m_distance;
   MC2SimpleString   m_name;
   MC2SimpleString   m_serverName;
   MC2Coordinate     m_coord;
   MC2Direction         m_direction;
   MC2SimpleString   m_categoryName;
};

namespace VicinityUtil {
   bool distanceComp(const VicinityItem* lhs, const VicinityItem* rhs);
   bool coordComp(const VicinityItem* lhs, const VicinityItem* rhs);
}

class UDFItem : public VicinityItem {
public:
   UDFItem() :VicinityItem() {}
   
   UDFItem(int distance,
           const MC2SimpleString& name,
           const MC2Coordinate& coord,
           const MC2Direction direction,
           UserDefinedFeature* udf);
   
   virtual void Accept(VicinityVisitor* visitor) {
      visitor->Visit(this);
   }

   
   inline VicinityType Type() const {
      return VI_UDF;
   }
   
   UserDefinedFeature* GetFeature();
private:
   UserDefinedFeature* m_udf;
};


#endif
