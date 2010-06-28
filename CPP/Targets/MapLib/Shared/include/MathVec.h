/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MATHVEC_H
#define MATHVEC_H

#include "config.h"
#include <math.h>

/**
 *    Math vector template class.
 *    SIZE is the length (dimensions) of the vector.
 *    TYPE is the type, default float.
 *
 */
template<int SIZE, class TYPE = float > class MathVec {
public:
   
   /// Constructor. Sets all elements to zero.
   MathVec() {
     for ( int i = 0; i < SIZE; ++i ) {
        m_elems[i] = 0;
     }
   }

   /// Get reference to index non modifiable.
   const TYPE& operator[](int i) const {
      return m_elems[i];
   }
   
   /// Get reference to index modifiable.
   TYPE& operator[](int i) {
      return m_elems[i];
   }
   
   /// Get reference to index non modifiable.
   const TYPE& operator()(int i) const {
      return m_elems[i];
   }
   
   /// Get reference to index modifiable.
   TYPE& operator()(int i) {
      return m_elems[i];
   }

   /// Calculate the cross product between this and other. (SIZE must be 3).
   MathVec<3> cross( const MathVec<3>& other ) const {
      MathVec<3> res;
      res[0] = (*this)[1] * other[2] - (*this)[2]*other[1];
      res[1] = (*this)[2] * other[0] - (*this)[0]*other[2];
      res[2] = (*this)[0] * other[1] - (*this)[1]*other[0];
      return res;
   }

   /// Calculate the scalar product between this and other.
   TYPE dot( const MathVec& other ) const {
      TYPE sum = 0;
      for ( int i = 0; i < SIZE; ++i ) {
         sum += m_elems[i] * other.m_elems[i];
      }
      return sum;
   }

   /// Calculate the norm (length).
   TYPE norm() const {
     return sqrt( dot(*this) );
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec& operator+=( const MathVec& other ) {
      for ( int i = 0; i < SIZE; ++i ) {
         m_elems[i] += other[i];
      }
      return *this;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec operator+( const MathVec& other ) const {
      return MathVec(*this) += other;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec operator-() const {
      MathVec res;
      for ( int i = 0; i < SIZE; ++i ) {
         res.m_elems[i] = -m_elems[i];
      }
      return res;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec& operator-=( const MathVec& other ) {
      (*this) += -other;
      return *this;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec operator-( const MathVec& other ) const {
      return MathVec(*this) -= other;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec& operator*=( const TYPE& tal ) {
      for ( int i = 0; i < SIZE; ++i ) {
         m_elems[i] *= tal;
      }
      return *this;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec operator*( const TYPE& tal ) const {
      return MathVec(*this) *= tal;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec& operator/=( const TYPE& tal ) {
      for ( int i = 0; i < SIZE; ++i ) {
         m_elems[i] /= tal;
      }
      return *this;
   }

   /// Standard arithmetic operator. Performs the operation on all elements.
   MathVec operator/( const TYPE& tal ) const {
      return MathVec(*this) /= tal;
   }

   /// To print the MathVec on an ostram.
   friend ostream& operator<<( ostream& o, const MathVec& m ) {
      o << "( ";
      const char* comma = "";
      for ( int i = 0; i < SIZE; ++i ) {
         o << comma;
         comma = ", ";
         o << m.m_elems[i];
      }
      o << ")";
      return o;
   }

   /// The elements of the vector.
   TYPE m_elems[SIZE];
   
};
#endif
