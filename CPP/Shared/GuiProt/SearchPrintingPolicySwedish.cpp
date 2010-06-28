/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#include <arch.h>
#include <vector>
#include "nav2util.h"
#include "GuiProt/SearchPrintingPolicySwedish.h"
#include "GuiProt/Serializable.h"
#include "GuiProt/SearchRegion.h"
#include "GuiProt/SearchItem.h"
#include "GuiProt/SearchArea.h"
#include "GuiProt/AdditionalInfo.h"
#include "GuiProt/FullSearchItem.h"
#include "GuiProt/HelperEnums.h"
#include "DistancePrintingPolicy.h"

namespace isab {

char *
SearchPrintingPolicySwedish::parseSearchItem(const SearchItem* item, 
         bool tabs,
         DistancePrintingPolicy::DistanceMode mode,
         bool addName )
{
   const char *city = "";
   const char *address = "";
   const char *streetNumber = "";
   char *m_processedString;

   /* Perform the formatting of the search match string */
#define MAX_STR_LEN 256
   m_processedString = new char[MAX_STR_LEN];
   /* Null termination. */
   int curPos = 0;
   if (tabs) {
      m_processedString[curPos++] = '\t';
   }
   m_processedString[curPos] = 0;

   bool addComma = true;
   const char* tmp = NULL;
   if ( addName ) {
      /* 1. The search match name. */
      tmp = item->getName();
      if ( tmp && (signed int)strlen( tmp ) < (MAX_STR_LEN-curPos) ) {
         strcat(m_processedString, tmp);
      } else {
         strncat( m_processedString, tmp, (MAX_STR_LEN-curPos-4) );
         curPos += (MAX_STR_LEN-curPos-4);
         if ( tabs ) {
            m_processedString[curPos++] = '\t';
         }
         m_processedString[curPos++] = '\0';
         return m_processedString;
      }
      curPos += strlen( tmp );

      /* 2. Next line. */
      if ( tabs ) {
         m_processedString[ curPos++ ] = '\t';
      } else {
         m_processedString[ curPos++ ] = ',';
      }
      m_processedString[ curPos ] = '\0';
   } else {
      addComma = false;
   }


   /* 3. Add distance, if it's valid. */
   if(item->getDistance() != MAX_UINT32){
      char* tmp2 = DistancePrintingPolicy::convertDistance(item->getDistance(),
            mode, DistancePrintingPolicy::Round);
      if (tmp2 && (signed int)strlen(tmp2) < (MAX_STR_LEN-curPos)) {
         strcat(m_processedString, tmp2);
         delete[] tmp2;
         curPos += strlen(tmp2);
      } else {
         return m_processedString;
      }
   } else {
      addComma = false;
   }
   /* Check the regions. */

   /* 4. We need to find the City first. */
   unsigned int numRegions = item->noRegions();
   unsigned int j = 0;
   while (j < numRegions ) {
      /* Is this the city region? */
      const SearchRegion* region = item->getRegion(j);
      if (region->getType() == GuiProtEnums::city) {
         /* Yup, got it. */
         if (addComma) {
            tmp = ", ";
            strcat(m_processedString, tmp);
         } else {
            addComma = true;
         }

         tmp = region->getName();
         if (tmp && (signed int)strlen(tmp) < (MAX_STR_LEN-curPos)) {
            strcat(m_processedString, tmp);
         } else {
            return m_processedString;
         }
         curPos += strlen(tmp);
         city = tmp;
      }
      j++;
   }

   /* 5. Now we check for a street address. */
   j=0;
   while (j < numRegions ) {
      /* Is this the street address region? */
      const SearchRegion* region = item->getRegion(j);
      if (region->getType() == GuiProtEnums::address) {
         /* Yup, got it. */
         if (addComma) {
            tmp = ", ";
            strcat(m_processedString, tmp);
         } else {
            addComma = true;
         }
         tmp = region->getName();
         if (tmp && (signed int)strlen(tmp) < (MAX_STR_LEN-curPos)) {
            strcat(m_processedString, tmp);
         } else {
            return m_processedString;
         }
         curPos += strlen(tmp);
         address = tmp;
      }
      j++;
   }
   if (j == numRegions) {
      /* No street address found. */
      /* Don't care about street number. */
   } else  {
      /* 6. Check for street number. */
      j=0;
      while (j < numRegions ) {
         /* Is this the street number region? */
         const SearchRegion* region = item->getRegion(j);
         if (region->getType() == GuiProtEnums::streetNumber) {
            /* Yup, got it. */
            tmp = " ";
            strcat(m_processedString, tmp);
            tmp = region->getName();
            if (tmp && (signed int)strlen(tmp) < (MAX_STR_LEN-curPos)) {
               strcat(m_processedString, tmp);
            } else {
               return m_processedString;
            }
            curPos += strlen(tmp);
            streetNumber = tmp;
         }
         j++;
      }
   }

   /* 7. Ok, now add the rest of the regions in order. */
   /*    Two important things: */
   /*    a) We need to remove the strings already copied. */
   /*    b) No duplicates are allowed (including the already */
   /*       copied strings. */
   for (j = 0; j < numRegions ; j++ ) {
      const SearchRegion* region = item->getRegion(j);
      /* Is this one of the previously written strings? */
      if (strcaseequ(city, region->getName())) {
         /* Ignore this value. */
         continue;
      }
      if (strcaseequ(address, region->getName())) {
         /* Ignore this value. */
         continue;
      }
      if (strcaseequ(streetNumber, region->getName())) {
         /* Ignore this value. */
         continue;      
      }
      /* The region is not one of the regions above, or any */
      /* region that has the same name (when folded). */
      /* We can add the region. */
      if (addComma) {
         tmp = ", ";
         strcat(m_processedString, tmp);
      } else {
         addComma = true;
      }
      tmp = region->getName();
      if (tmp && (signed int)strlen(tmp) < (MAX_STR_LEN-curPos)) {
         strcat(m_processedString, tmp);
      } else {
         return m_processedString;
      }
      curPos += strlen(tmp);
   }

   return m_processedString;
}


}
