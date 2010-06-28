/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef MC2_POINT
#define MC2_POINT

#include "config.h"

#ifdef __SYMBIAN32__
#include <e32std.h>
#endif

/**
 *    Planar point, name MC2Point is chosen as it does not exist
 *    in WIN32.
 */
#ifndef __SYMBIAN32__
class MC2Point {
public:
   typedef int32 coord_t;
   /// Constructor 
   MC2Point(coord_t inx, coord_t iny) : x(inx), y(iny) {}
   
   /// Returns the x value of the point. It is recommended to use.
   inline const coord_t getX() const { return x; }
   
   /// Returns the y value of the point. It is recommended to use. 
   inline const coord_t getY() const { return y; }

   /// Returns a reference to the x value for writing.
   inline coord_t& getX() { return x; }
   /// Returns a reference to the y value for writing.
   inline coord_t& getY() { return y; }

   inline bool operator==(const MC2Point& other) const {
      return ( getX() == other.getX() ) && ( getY() == other.getY() );
   }
   
   inline bool operator!=(const MC2Point& other) const {
      return ( getX() != other.getX() ) || ( getY() != other.getY() );
   }

private:
   /// This should be private, really. Use getX instead.
   coord_t x;
   /// This should be private, really. Use getY instead.
   coord_t y;

#else

class MC2Point : public TPoint {
public:
   typedef TInt coord_t;
   /// Constructor 
   MC2Point(coord_t inx, coord_t iny) : TPoint(inx, iny) {}
   /// Constructor 
   MC2Point(const TPoint& inpoint) : TPoint( inpoint ) {}
    /// Returns the x value of the point. It is recommended to use.
   inline const coord_t getX() const { return iX; }
   
   /// Returns the y value of the point. It is recommended to use. 
   inline const coord_t getY() const { return iY; }

   /// Returns a reference to the x value for writing.
   inline coord_t& getX() { return iX; }
   /// Returns a reference to the y value for writing.
   inline coord_t& getY() { return iY; }
   
#endif
   // Common code!
public:
   inline MC2Point& operator+=(const MC2Point& other ) {
      getX() += other.getX();
      getY() += other.getY();
      return *this;
   }

   inline MC2Point& operator-=(const MC2Point& other ) {
      getX() -= other.getX();
      getY() -= other.getY();
      return *this;
   }
   
   inline MC2Point operator+( const MC2Point& other ) const {
      return MC2Point(*this) += other;
   }
   
   inline MC2Point operator-( const MC2Point& other ) const {
      return MC2Point(*this) -= other;
   }

   /// Shifts the value of the point to the left n times.
   inline MC2Point& operator<<=( int n ) {
      getX() = (getX() << n);
      getY() = (getY() << n);
      return *this;
   }
   
   /// Shifts the value of the point to the right times.
   inline MC2Point& operator>>=( int n ) {
      getX() = (getX() >> n);
      getY() = (getY() >> n);
      return *this;
   }
   
   /// Returns a point shifted with values shifted n times to the left.
   inline MC2Point operator<<( int n ) const {
      MC2Point other = MC2Point(*this);
      other <<= n;
      return other;
   }
   
   /// Returns a point shifted with values shifted n times to the right.
   inline MC2Point operator>>( int n ) const {
      MC2Point other = MC2Point(*this);
      other >>= n;
      return other;
   }
      

   friend std::ostream& operator<<( std::ostream& o, const MC2Point& p ) {
      o << "( " << p.getX() << ", " << p.getY() << ")";
      return o;
   }


   /**
    *   Class to be used when putting MC2Coordinates in e.g.
    *   ClipUtil.h, InsideUtil etc.
    */
   class XYHelper {
   public:
      /// Returns the x value for the coordinate
      coord_t getX( const MC2Point& coord ) const {
         return coord.getX();
      }
      /// Returns the y value for the coordinate
      coord_t getY( const MC2Point& coord ) const {
         return coord.getY();         
      }
      /// Returns the x value for the coordinate
      coord_t getX( const MC2Point* coord ) const {
         return coord->getX();
      }
      /// Returns the y value for the coordinate
      coord_t getY( const MC2Point* coord ) const {
         return coord->getY();         
      }
   };
};

struct MC2Pointf {
   float64 x;
   float64 y;
};

#endif
