/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _INTERPOLATIONHINTCONSUMER_H_
#define _INTERPOLATIONHINTCONSUMER_H_

/**
 *   A velocity path is defined as a series of points with
 *   a coordinate attribute and a velocity attribute.
 *
 *   A class that intends to use this information can derive
 *   from this class. It is then up to the application to
 *   provide the data to the class, via the external MapLib
 *   interface.
 *   
 */ 

class MC2Coordinate;

class InterpolationHintConsumer {
public:
   /**
    *   Prepares a new set of data points, starting at the
    *   current point in time.
    */ 

   void prepareNewData();
   
   /**
    *   Prepares a new set of data points, starting at a
    *   predefined point in time.
    */ 

   virtual void prepareNewData( unsigned int startTimeMilliSec ) = 0;

   /**
    *   Adds a new data point to the current set being built.
    *
    *   @param   coord   The coordinate member of the data point.
    *   @param   velocityCmPS   The velocity member of the data point.
    */
   
   virtual void addDataPoint( const MC2Coordinate& coord,
                              int velocityCmPS ) = 0;

   /**
    *   Finalizes the data which has been added.
    */ 
   
   virtual void finalizeNewData() = 0;

   /**
    *  Cycles the interpolation configuration forwards. 
    */
   
   virtual void cycleConfigurationForward() = 0;

   /**
    *  Cycles the interpolation configuration backwards.
    */ 
   
   virtual void cycleConfigurationBackward() = 0;
};

#endif /* _INTERPOLATIONHINTCONSUMER_H_ */
