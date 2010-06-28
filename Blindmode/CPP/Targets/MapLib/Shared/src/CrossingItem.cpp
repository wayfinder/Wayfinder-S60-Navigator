/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "CrossingItem.h"
#include <set>

void CrossingItem::buildSimpleName()   
{
   m_name.clear();

   /* Alright, the IntersectingRoads are guaranteed
    * to be in sorted order. So we create a small set
    * to keep track of which roads have already been
    * added. Since they are already in sorted order,
    * they get added in correct order in this compressed
    * format aswell. */

   std::set<MC2SimpleString> alreadyAdded;
      
   for(  Crossing::iterator street = 
            m_crossing.begin();
         street != m_crossing.end();
         street++) 
   {
      if(alreadyAdded.find(street->name)!=alreadyAdded.end())
         continue;

      if( street!= m_crossing.begin() )
         m_name.append(" / ");
            
      alreadyAdded.insert(street->name);
      
      m_name.append (street->name);
   }
}

const Crossing CrossingItem::GetCrossingsAbsolute()
{
   //Easy. The internal crossing vector is already
   //sorted with respect to north.

   return m_crossing;
}

const Crossing CrossingItem::GetCrossingsRelative(int heading)
{
   Crossing ret = m_crossing;

   std::sort(ret.begin(),
             ret.end(),
             IntersectingRoadComparator(heading));

   return ret;
}
