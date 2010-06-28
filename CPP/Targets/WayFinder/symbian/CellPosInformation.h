/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CELL_POS_INFORMATION_H
#define CELL_POS_INFORMATION_H

#include "arch.h"
#include "MC2Coordinate.h"

/**
 * An update of current whereabouts that is not precise enough to
 * be a point. That is represented by the area of a circle sector
 * that has a smaller radius circle sector removed from it. Like
 * a pie slice that has the pointy tip bitten of.
 *
 * Received in LocationListener call back function.
 */
class CCellPosInformation : public CBase
{
public:

   /**
    * Default constructor.
    */
   CCellPosInformation();

   /**
    * Constructor.
    * 
    * @param position The center of the circle sector.
    * @param altitude The altitude in meters.
    * @param innerRadius The inner radius of the circle sector.
    * @param outerRadius The outer radius of the circle sector.
    * @param startAngle The start angle of the circle sector.
    * @param endAngle The end angle of the circle sector.
    */
   CCellPosInformation(const Nav2Coordinate& position, 
                      int32 altitude, 
                      uint32 innerRadius, 
                      uint32 outerRadius, 
                      uint16 startAngle, 
                      uint16 endAngle);

   /**
    * Destructor.
    */
   virtual ~CCellPosInformation();

   /**
    * Get the altitude.
    *
    * @return The altitude in meters.
    */
   int32 getAltitude() const;

   /**
    * Get the center coordinate of the circle sector.
    *
    * @return The center of the circle sector.
    */
   Nav2Coordinate getPosition() const;

   /**
    * Get the inner radius of the circle sector.
    *
    * @return The inner radius in meters.
    */
   uint32 getInnerRadius() const;

   /**
    * Get the outer radius of the circle sector.
    *
    * @return The inner radius in meters.
    */
   uint32 getOuterRadius() const;

   /**
    * Get the start angle of the circle sector.
    *
    * @return The start angle in degrees.
    */
   uint16 getStartAngle() const;

   /**
    * Get the end angle of the circle sector.
    *
    * @return The end angle in degrees.
    */
   uint16 getEndAngle() const;

private:
   /// The coordinate.
   Nav2Coordinate m_position;

   /// The altitude.
   int32 m_altitude;

   /// The inner radius.
   uint32 m_innerRadius;

   /// The outer radius.
   uint32 m_outerRadius;

   /// The start angle.
   uint16 m_startAngle;

   /// The end angle.
   uint16 m_endAngle;
};

#endif // CELL_POS_INFORMATION_H
