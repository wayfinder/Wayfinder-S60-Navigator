/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "PlotterAdapter3dTests.h"
#include "TestFramework.h"
#include "PlotterAdapter3d.h"
#include "config.h"

#include "MapPlotter.h"
#include "MapPlotterTypes.h"
#include "MC2SimpleString.h"

namespace isab {
class PointVector;
}



class NullPlotter : public isab::MapPlotter {
public:
   virtual ~NullPlotter() {}
   
   
   virtual void clearScreen() {}   
   virtual void setBackgroundColor( unsigned int red,
                                    unsigned int green,
                                    unsigned int blue ) {}
   
   virtual void setPenColor( unsigned int red,
                             unsigned int green,
                             unsigned int blue ) {};
   
   virtual void getMapSizePixels(isab::Rectangle& size) const {
      size = isab::Rectangle( 0, 0, 320, 240 );
   };
   
   virtual isab::PointVector* createPointVector(int size) { return NULL; };
   virtual void deletePointVector(isab::PointVector* vect) {};


   virtual void drawPolygonWithColor( std::vector<MC2Point>::const_iterator begin,
                                      std::vector<MC2Point>::const_iterator end,
                                      uint32 fillcolor ) {};

   virtual bool returnsVisibleSizeOfBitmaps() const {
      return true;
   }

   virtual void setFont( const STRING& fontName, int size ) {};

   virtual STRING* createString( const char* text ) { return NULL; };

   virtual void deleteString( STRING* text ) {};

   virtual int getStringLength( const STRING& text ) { return 0; };
   
   /**
    *   Draws a polyline defined by the array of points.
    */
   virtual void drawPolyLine( std::vector<MC2Point>::const_iterator begin,
                              std::vector<MC2Point>::const_iterator end ) {};
   
   virtual void drawPolyLineWithColor( std::vector<MC2Point>::const_iterator begin,
                                       std::vector<MC2Point>::const_iterator end,
                                       uint32 color,
                                       int lineWidth ) {};
   
};

void
PlotterAdapter3dTests::testCompareFloatAndInteger( TestReporter& reporter )
{
   // NEW_TEST_FUNCTION( reporter );
   // NullPlotter* plotter = new NullPlotter;
   // isab::PlotterAdapter3d* plotterAdapter =
   //    new isab::PlotterAdapter3d( plotter );
   // isab::Rectangle rect;
   // plotter->getMapSizePixels( rect );
   // MC2Point centerPoint = MC2Point( rect.getWidth() / 2,
   //                                  rect.getHeight() / 2 );
   // MC2Pointf resFloat = plotterAdapter->transform3DTo2D_slowf( centerPoint );

   // MC2Point resInt = plotterAdapter->transform3DTo2D_slow( centerPoint );
}


