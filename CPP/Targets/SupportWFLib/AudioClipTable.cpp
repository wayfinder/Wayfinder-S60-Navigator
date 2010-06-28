/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioClipTable.h"

#include <algorithm>

template<class ForwardIterator, class StrictWeakOrdering>
bool wf_is_sorted(ForwardIterator first, ForwardIterator last, 
                  StrictWeakOrdering comp)
{
   bool sorted = true;
   for(ForwardIterator p = first; sorted && p+1 != last; ++p){
      sorted = !comp(*(p+1),*p); //(p+1) < p
   }
   return sorted;
}


bool
AudioClipTableTableDriven::sanityCheck()
{
   const int_to_clipname_t * clipTable = getClipTable1();

   // Binary search is good if the clips are sorted
   int maxNbr = getNumClips1();
   const int_to_clipname_t* begin = clipTable;
   const int_to_clipname_t* end   = clipTable + maxNbr;
   bool ret_val = wf_is_sorted( begin, end, AudioClipTableSearchComp() );

   //MC2_ASSERT( ret_val );
   if (!ret_val) return ret_val;

   clipTable = getClipTable2();
   if (clipTable) {
      maxNbr = getNumClips2();
      begin = clipTable;
      end   = clipTable + maxNbr;
      ret_val = wf_is_sorted( begin, end, AudioClipTableSearchComp() );

      //MC2_ASSERT( ret_val );
   }

   return ret_val;
}

const char*
AudioClipTableTableDriven::getFileName(int number)
{   
   const int_to_clipname_t * clipTable = getClipTable1();
   const int maxNbr = getNumClips1();
#if 1
   // Binary search is good if the clips are sorted
   const int_to_clipname_t* begin = clipTable;
   const int_to_clipname_t* end   = clipTable + maxNbr;

   const int_to_clipname_t* found =
      std::lower_bound( begin, end, number,
                        AudioClipTableSearchComp() );
   if ( found != end && found->m_audioEnum == number ) {
     return found->m_fileName;
   }

   clipTable = getClipTable2();
   if (clipTable) {
      const int maxNbr = getNumClips2();

      begin = clipTable;
      end   = clipTable + maxNbr;

      found = std::lower_bound( begin, end, number, AudioClipTableSearchComp() );

      if ( found != end && found->m_audioEnum == number ) {
        return found->m_fileName;
      }
   }

#else
   for ( int i = 0; i < maxNbr; ++i ) {
      if ( clipTable[i].m_audioEnum == number ) {
         return clipTable[i].m_fileName;
      }
   }

   const int_to_clipname_t * clipTable = getClipTable2();
   if (clipTable) {
      const int maxNbr = getNumClips2();
      for ( int i = 0; i < maxNbr; ++i ) {
         if ( clipTable[i].m_audioEnum == number ) {
            return clipTable[i].m_fileName;
         }
      }
   }
#endif
   return NULL;
}


