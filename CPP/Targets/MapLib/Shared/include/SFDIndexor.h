/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFD_INDEXOR_H
#define SFD_INDEXOR_H

#include "config.h"
#include "NewStrDup.h"
#include "MC2SimpleString.h"

/**
 *   Should not be used. Makes copy-constructor etc. smaller.
 */
class SFDIndexorBase {
protected:
   /// Boa Constructor
   SFDIndexorBase() : m_str( NULL ),
                      m_strNbr( MAX_UINT32 ),
                      m_strOffset( MAX_UINT32 ),
                      m_strAllocSize(0),
                      m_stringRead(false) {}
   /// Destructor
   ~SFDIndexorBase() { delete [] m_str; }

   // Vars - don't forget to update swap function.
   
   /// The string of the Indexor, if read.
   char* m_str;
   /// The index of the string in the file
   uint32 m_strNbr;
   /// The offset of the characters in the file
   uint32 m_strOffset;
   /// Size allocated for strings
   uint16 m_strAllocSize;
   /// True if the string has been read from file
   bool m_stringRead;   
};

/**
 *   Represents a string in the index.
 */
class SFDIndexor : public SFDIndexorBase {
public:

   /// Construct empty. Use base constructor only.
   inline SFDIndexor() {}
   
   /// Copy constructor
   inline SFDIndexor( const SFDIndexor& other );

   /// Assignment operator
   inline SFDIndexor& operator=( const SFDIndexor& other );
   
   /// Less than operator. Compares only the strings.
   bool operator<( const SFDIndexor& other ) const;     
   /// Less than operator. Compares only the strings.
   bool operator<( const MC2SimpleString& other ) const;
   /// Less than operator. Compares only the strings.
   bool operator<( const char* other ) const;
   /// Less than operator. Compares only the strings.
   friend bool operator<( const MC2SimpleString& s1,
                          const SFDIndexor& s2 );

   /// True if the string has been read
   inline bool stringRead() const { return m_stringRead; }

   /// Returns the string. Use only if stringRead.
   inline const char* getStr() const {
      MC2_ASSERT( stringRead() );
      return m_str;
   }

   /// Returns the string number
   inline uint32 getStrNbr() const { return m_strNbr; }

   void allocStr( int size ) {
      delete [] m_str;
      m_str = new char[ size ];
      m_strAllocSize = size;
   }

   /**
    *   Swaps the contents of this SFDIndexor with the other
    *   one. Note that this is used when searching to swap
    *   with SFDFileIndexor in a special way.
    */
   void swap( SFDIndexor& other );
   
protected:


};

// Inlined functions that are long.

inline
SFDIndexor::SFDIndexor( const SFDIndexor& other ) : SFDIndexorBase( other )
{
   if ( m_str ) {
      // Duplicates the string and returns the length.
      m_strAllocSize = NewStrDup::newStrDupLen( m_str );
   }
}

inline SFDIndexor&
SFDIndexor::operator=( const SFDIndexor& other )
{
   // Slower?
   if ( this != &other ) {      
      // We can maybe re-use the old string.
      char* old_str = m_str;
      uint16 old_size = m_strAllocSize;
      // Do the base assignment.
      SFDIndexorBase::operator=(other);
      if ( ! m_str ) {
         delete [] old_str;
      } else {
         if ( old_size >= m_strAllocSize ) {
            m_strAllocSize = old_size;         
         } else {
            delete [] old_str;
            old_str = new char[m_strAllocSize];
         }
         strcpy( old_str, m_str );
         m_str = old_str;
      }
   }
   return *this;
}


#endif
