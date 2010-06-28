/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RequestList.h"

namespace isab{
   RequestList::RequestList(int nbrOfItems) :
      m_first(NULL), m_last(NULL), m_free(NULL), 
      m_count(0), m_numElem(0), m_ideal(nbrOfItems)
   {
      for(int i = 0; i < nbrOfItems; ++i){
         RequestListItem* item = new RequestListItem();
         deallocate(item);
      }
   }

   RequestListItem* RequestList::allocate()
   {
      RequestListItem* elem;
      
      /* Bad parameter or no free elements? */
      if (!m_free) {
         return new RequestListItem();
      }
      
      elem = m_free;
      m_free = elem->m_next;
      
      elem->request   = 0;
      elem->reqID     = 0;
      elem->resend    = 0;
      elem->errorType = 0;
      elem->status    = 0;
      elem->data      = 0;
      
      m_numElem--;
      return elem;
   }

   bool RequestList::deallocate(RequestListItem *elem)
   {
      bool retval = true;
      if (!elem) {
         retval = false;
      } else if(m_numElem >= m_ideal){
         delete elem;
      } else {
         elem->m_next = m_free;
         m_free = elem;
         m_numElem++;
      }
      return retval;
   }

   bool RequestList::enqueue(RequestListItem *always)
   {
      if(!always) {
         return 0;
      }
      
      m_count++;
      m_numElem++;
      if (m_last == NULL) {
         /* Empty queue. */
         m_first = m_last = always;
         always->m_prev = NULL;
         always->m_next = NULL;
      } else {
         /* Append last. */
         always->m_prev = m_last;
         always->m_prev->m_next = always;
         always->m_next = NULL;
         m_last = always;
      }
      
      return 1;
   }
   RequestListItem* RequestList::dequeue(uint16 requestID)
   {
      RequestListItem* elem;
      
      /* Find the element with correct request id. */
      /* We assume (tm) that the element is at the */
      /* front of the list. (We operate FIFO...) */
      elem = m_first;
      while (elem) {
         if (elem->reqID == requestID) {
            /* This is the one. */
            break;
         }
         elem = elem->m_next;
      }
      if (elem) {
         dequeue(elem);
      }
      /* May be, and may not be, NULL. */
      return elem;
   }

   RequestListItem* RequestList::dequeue(uint32 requestID)
   {
      RequestListItem* elem;
      
      /* Find the element with correct request id. */
      /* We assume (tm) that the element is at the */
      /* front of the list. (We operate FIFO...) */
      elem = m_first;
      while (elem) {
         if (elem->src == requestID) {
            /* This is the one. */
            break;
         }
         elem = elem->m_next;
      }
      if (elem) {
         dequeue(elem);
      }
      
      /* May be, and may not be, NULL. */
      return elem;
   }

   RequestListItem* RequestList::dequeue(RequestIterator& iterator)
   {
      RequestIterator p = begin();
      RequestListItem* ret = NULL;
      // ensure that the iterator refers to an element in this list.
      while(p != end() && p != iterator){
         p++;
      }
      if(p == iterator){
         --iterator;
         ret = dequeue(&(*p));
      }   
      return ret;
   }

   RequestListItem* RequestList::dequeue(RequestListItem* elem){
      m_count--;
      m_numElem--;
      if (elem->m_prev) {
         /* We're not first. */
         if (NULL != (elem->m_prev->m_next = elem->m_next)) {
            /* We're no last. */
            elem->m_next->m_prev = elem->m_prev;
         } else {
            /* We're last in the list. */
            m_last = elem->m_prev;
         }
      } else {
         /* We're the first element. */
         if (NULL != (m_first = elem->m_next)) {
            /* We're not last though. */
            elem->m_next->m_prev = NULL;
         } else {
            /* Dang, we're the only one! */
            m_last = NULL;
         }
      }
      return elem;
   }


   void RequestList::concatenate(RequestList& list, bool reverse)
   {
      RequestListItem* thisFirst = m_first;
      RequestListItem* thisLast  = m_last;
      RequestListItem* thatFirst = list.m_first;
      RequestListItem* thatLast  = list.m_last;

      // concatenate outstanding requests.
      if(thisLast != NULL && thatLast != NULL){
         if(reverse){
            m_first = thatFirst;
            thatLast->m_next = thisFirst;
            thisFirst->m_prev = thatLast;
         } else {
            m_last = thatLast;
            thisLast->m_next = thatFirst;
            thatFirst->m_prev = thisLast;
         }
         m_count   += list.m_count;
         m_numElem += list.m_count;
         list.m_count = 0;
      } else if(thisLast != NULL && thatLast == NULL){
         // do nothing, the list is empty.
      } else if(thisLast == NULL && thatLast != NULL){
         m_first = thatFirst;
         m_last = thatLast;
      } else {
         // no lists, do nothing;
      }
      m_count   += list.m_count;
      m_numElem += list.m_count;
      list.m_count = 0;
      list.m_first = NULL;
      list.m_last = NULL;

      //move free requests.
      while(list.m_free != NULL){
         RequestListItem* item = list.allocate();
         deallocate(item);
      }

   }


   RequestList::~RequestList()
   {
      while(m_first != NULL){
         RequestListItem *p = dequeueFirst();
         delete p;
      }
      while(m_free != NULL){
         RequestListItem* p = allocate();
         delete p;
      }
   } 

   //definetion of the pure virtual destructor.
   //Yes, there has to be one.
   RequestData::~RequestData()
   {
   }
}

