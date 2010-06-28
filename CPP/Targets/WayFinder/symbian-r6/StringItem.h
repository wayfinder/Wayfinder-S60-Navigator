/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STRINGITEM_H
#define STRINGITEM_H

#include "Comparable.h"
#include "Vector.h"
#include "Item.h"

/**
 * Abstract class representing an entry in the stringtable.
 */
class StringItem : public PosComparable
{
	public:
      /**
       * Constructor.
       *
       * @param str is the widechar string to be stored.
       * @param length is the length of the string including the 
       *        terminating \0, i.e. wcslen(str) + 1. 
       */
		StringItem( const wchar_t* str, int length  );

      /**
       * Constructor.
       *
       * @param str is the char string to be stored as widechar.
       * @param length is the length of the string.
       */
		StringItem( const char* str, int length  );

      /**
       * Destructor.
       */
      virtual ~StringItem();

      /**
       * Adds a pointer to an item.
       *
       * @param item is the pointer to add.
       */
      void addItem( Item* item );

      /**
       * Update hte names with a new stringindex
       */
      void updateNames( uint32 newIndex );

      /**
       * Adds items to vector.
       */
      void addItemsToVector( Vector& vector );

      /**
       * Adds only the items with the right mask to the vector.
       * 
       * @param vector Outparameter The vector, which the items are added to.
       * @param mask Mask, put together with ItemTypes::itemMask values.
       */
      void addItemsToVector( Vector& vector, uint32 mask );

      /**
       * Returns the string in this item.
       */
      wchar_t* getString();

      /**
       *  Returns true if the items has a valid string.
       *
       * @return true if the item has a valid string.
       */
      bool hasString();

      bool hasItems();

      /**
       *   @name  operators. 
       *   To make it possible to sort and search for the elements in 
       *   ObjVector. If the elements not should be sorted or searched
       *   for the functions doesn't need to be implemented.
       */
      //@ {
         ///   equals
         virtual bool operator== (const Comparable& comp) const = 0;
   
         ///   not equal
         virtual bool operator != (const Comparable& comp) const = 0;
   
         ///   greater
         virtual bool operator > (const Comparable& comp) const = 0;
      
         ///   less
         virtual bool operator < (const Comparable& comp) const = 0;
      //@ }

      /**
       * Compare the n first positions.
       *
       * @param   comp  The comparable to compare with.
       * @param   n     The number of positions to compare.
       * @return  An integer less than, equal to, or greater than zero if the 
       *          n first positions of THIS comparable  is found, respectively, to be less
       *          than, to match, or be greater than comp.
       */
      virtual int compareFirstPos(const PosComparable& comp, int n) const = 0;

	protected:

      /**
       * The string to store.
       */
		wchar_t* m_string;

      /**
       * A vector with items to store.
       */
		Vector m_items;
};
#endif
