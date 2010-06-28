/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef REQUESTLIST_H
#define REQUESTLIST_H

#include <arch.h>

namespace isab{

   class RequestData{
   public:
      virtual ~RequestData() = 0;
   };

   /** List item holding information about oustanding requests. */
   class RequestListItem{
      friend class RequestIterator;
      friend class RequestList;
      /** The next item in the list. */
      RequestListItem* m_next;
      /** The preceeding item in the list. */
      RequestListItem* m_prev;
   public:

      RequestListItem();

      ~RequestListItem();

      /** ??? */
      uint16 request;
      /** The id used to operate the list. */
      uint16 reqID;

      /** ??? */
      uint16 resend;
      
      /** ??? */
      uint16 errorType;
      /** ??? */
      uint16 status;
      /** Extra data associated with this outstanding request. */
      RequestData* data;
      /** Address of source module. */
      uint32 src;
      /** NavRequestPacket data holder associated with this request. 
       *  Used to be able to enque poll packets as well. */
      RequestData* nrpData;
   };
   /** The list holding individual RequestListItems.     */
   class RequestList{
      /** The first item on the list. NULL if there are no outstanding 
       * requests.*/
      RequestListItem * m_first;
      /** The last item on the list. NULL if there are no outstanding
       * requests. */
      RequestListItem * m_last;
      /** The list of unused itemd. maintained as a singly linked list. */
      RequestListItem * m_free;

      /** The number of outstanding requests. */
      int m_count;
      /** The total number of items on the unused and outstanding lists. */
      int m_numElem;
      /** The ideal number of items in the two lists. */
      int m_ideal;

      RequestListItem* dequeue(RequestListItem* item);
   public:
      /** Constructor. Creates the requested number of items, 
       * initializes them and puts them on the unused list. 
       * @param nbrOfItems the number of items this list will hold. 
       *                   Default value is 10.
       */
      RequestList(int nbrOfItems = 10);

      /** Destructor. Destroys all items. Note that any objects pointed 
       * to by the data pointer in RequestListItem will not be destoyed.*/
      ~RequestList();

      /** Fetches an item from the unused list. Its the callers 
       * responsibility to enqueue or deallocate the item.
       * @return a pointer to an unused RequestListItem. NULL if
       *         there are no unused items.
       */
      RequestListItem* allocate();

      /** Returns a RequestListItem to the unused queue.
       * @param elem a pointer to the RequestListItem to deallocate.
       * @return false if elem is a NULL-pointer, true otherwise.
       */
      bool deallocate(RequestListItem *elem);

      /** Inserts a RequestListItem into the outstanding queue.
       * @param always a pointer to a RequestListItem.
       * @return false if always is a NULL-pointer, true otherwise.
       */
      bool enqueue(RequestListItem *always);

      /** Removes a RequestListItem from the outstanding queue.
       * It's the callers resposibility to deallocate it.
       * @param requestID the id identifying the RequestListItem
       *                  to dequeue.
       * @return the RequestListItem form the outstanding queue 
       *         whos ID match requestID, NULL if no item matched.
       */
      RequestListItem* dequeue(uint16 requestID);

      /** Removes a RequestListItem from the outstanding queue.
       * It's the callers responsibility to deallocate it.
       * @param requestID a 32 bit id matching the src field in 
       *                  RequestListItem.
       * @return the matching RequestListItem, or NULL if no Item 
       *         matched
       */
      RequestListItem* dequeue(uint32 requestID);

      /** Removes a RequestListItem from the outstanding queue.
       * It's the callers responsibility to deallocate it.
       * @param requestID a 32 bit id matching the src field in 
       *                  RequestListItem.
       * @return the matching RequestListItem, or NULL if no Item 
       *         matched
       */
      RequestListItem* dequeue(class RequestIterator& iterator);

      /** Removes the first RequestListItem from the outstanding 
       * queue. It's the callers responsibility to deallocate the 
       * item.
       * @return a pointer to the dequeued RequestListItem, or NULL 
       *         if the outstanding queue was empty.
       */       
      RequestListItem* dequeueFirst();

      /** Reports the number of items on the outstanding list.
       * @return the number of items on the outstanding list. 
       */
      int outstanding();

      /** Reports the total number of items this RequestList has.
       * @return the total number of items on the outstanding list 
       *         and the unused list.
       */
      int capacity();

      /** Reports the number of items on the unused list.
       * @return the number of unused items available.
       */
      int remaining();

      /** Concatenates two lists. */
      void concatenate(RequestList& alist, bool reverse = false);

      void insertList(RequestList& alist);

      class RequestIterator begin();
      class RequestIterator end();
   };

   class RequestIterator {
   public:
      /** Returns a reference to the RequestListItem currently pointed at.
       * @return a reference to a RequestListItem. If the iterator has passed 
       *         the bounds of the list a RequestListItem with all fields 
       *         set to 0 will be retuned.
       */
      RequestListItem& operator*();

      /** Make all of RequestListItem's public members available with
       * the -> notation from an iterator. Make sure that the iterator
       * is valid thou, otherwise you will get a NULL pointer.
       * @return a pointer to the current RequestListitem.
       */
      RequestListItem* operator->();
      /** Prefix ++ operator.
       * @return a reference to this iterator, pointing to the next 
       *         RequestListItem.
       */
      RequestIterator& operator++();

      /** Postfix ++ operator.
       * @return a copy of this iterator, pointing to the same  
       *         RequestListItem as before the operation.
       */
      RequestIterator  operator++(int);
 
      /** Prefix -- operator.
       * @return a reference to this iterator, pointing to the previous 
       *         RequestListItem.
       */
      RequestIterator& operator--();

      /** Postfix -- operator.
       * @return a copy of this iterator, pointing to the same  
       *         RequestListItem as before the operation.
       */
      RequestIterator  operator--(int);

      /** Moves the iterator the specified steps forward.
       * @param plus the number of steps to take.
       * @return a reference to this iterator, updated the correct number 
       *         of steps.
       */
      RequestIterator& operator+=(int plus);

      /** Moves the iterator the specified steps backwards.
       * @param minus the number of steps to take.
       * @return a reference to this iterator, updated the correct number 
       *         of steps.
       */
      RequestIterator& operator-=(int steps);

      /** Compares two iterators to see if they refer to the same 
       * RequestListItem.
       * @param comp the RequestIterator to compare with.
       * @return true if the two iterators refer to the same item.
       */
      bool operator==(const RequestIterator& comp);
      /** Compares two iterators to see if they refer to the same 
       * RequestListItem.
       * @param comp the RequestIterator to compare with.
       * @return true if the two iterators doesn't refer to the same item.
       */ 
      bool operator!=(const RequestIterator& comp);

      /** Assignment operator. Copies the contents of one iterator into 
       * another.
       * @param iter the iterator to copy.
       * @return a reference to this iterator.
       */
      RequestIterator& operator=(const RequestIterator& iter);
      /** Constructor that sets this iterator to a out of bound value.*/
      RequestIterator();
      /** Constructor that sets this iterator to a defined value.
       * @param start the item to start at.
       */
      RequestIterator(RequestListItem* start);
      
      /** Destructor. */
      ~RequestIterator();

   private:
      RequestListItem* current;
      RequestListItem endItem; 
   };

   //===============================================================
   //============== inlines for RequestListItem ====================

   inline RequestListItem::RequestListItem() :
      m_next(NULL), m_prev(NULL), request(0), reqID(0), 
      resend(0), errorType(0), status(0), data(NULL), src(0), 
      nrpData(NULL)
   {}

   inline RequestListItem::~RequestListItem()
   {
      delete data;
      data = NULL;
      delete nrpData;
      nrpData = NULL;
   }

      
   //===============================================================
   //============== inlines for RequestList  =======================

   inline RequestListItem* RequestList::dequeueFirst()
   {
      return (m_first != NULL) ? dequeue(m_first->reqID) : NULL;
   }

   inline int RequestList::outstanding()
   {
      return m_count;
   }

   inline int RequestList::capacity()
   { 
      return m_numElem;
   }

   inline int RequestList::remaining()
   {
      return m_numElem - m_count;
   }

   inline void RequestList::insertList(RequestList& alist)
   {
      concatenate(alist, true);
   }

   inline RequestIterator RequestList::begin()
   {
      return RequestIterator(m_first);
   }
   
   inline RequestIterator RequestList::end()
   {
      return RequestIterator();
   }

   //================================================================
   //================ Inlines for RequestIterator ===================
   
   inline RequestListItem& RequestIterator::operator*()
   {
      return current ? *current : endItem;
   }

   inline RequestListItem* RequestIterator::operator->(){
      return current;
   }


   inline RequestIterator& RequestIterator::operator++()
   {
      current = current->m_next;
      return *this;
   }

   inline RequestIterator RequestIterator::operator++(int)
   {
      RequestIterator tmp = *this;
      current = current->m_next;
      return tmp;
   }

   inline RequestIterator& RequestIterator::operator--()
   {
      current = current->m_prev;
      return *this;
   }
   inline RequestIterator RequestIterator::operator--(int)
   {
      RequestIterator tmp = *this;
      current = current->m_prev;
      return tmp;
   }

   inline RequestIterator& RequestIterator::operator+=(int plus)
   {
      for(int i = 0; i < plus && current != NULL; ++i){
         current = current->m_next;
      }
      return *this;
   }

   inline RequestIterator& RequestIterator::operator-=(int minus)
   {
      for(int i = 0; i < minus && current != NULL; ++i){
         current = current->m_prev;
      }
      return *this;
   }

   inline bool RequestIterator::operator==(const RequestIterator& comp)
   {
      return current == comp.current;
   }
   inline bool RequestIterator::operator!=(const RequestIterator& comp)
   {
      return current != comp.current;
   }

   inline RequestIterator& RequestIterator::operator=(const RequestIterator& iter)
   {
      //no self-assignment check needed.
      current = iter.current;
      return *this;
   }

   inline RequestIterator::RequestIterator():
      current(NULL)
   {
      // XXXX - Allocates one item per requestlist.
      endItem = RequestListItem();
   }

   inline RequestIterator::RequestIterator(RequestListItem* start) :
      current(start)
   {}

   inline RequestIterator::~RequestIterator()
   {}
}
#endif

