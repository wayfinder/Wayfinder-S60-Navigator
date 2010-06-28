/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _POSITIONINTERPOLATOR_H_
#define _POSITIONINTERPOLATOR_H_
#include "config.h"
#include <vector>

#include "InterpolationHintConsumer.h"
#include "MC2Coordinate.h"
//#include <iterator>
#include "TimePeriod.h"

struct InterpolatedPosition {

   bool isValid() const {
      return coord.isValid();
   }

   MC2Coordinate coord;
   float64 velocityMPS;
   float64 directionDegrees;
};

class PositionInterpolator : public InterpolationHintConsumer {
public:
   friend class PositionInterpolatorMath;
   friend class PositionInterpolatorTester;

   /** 
    *   Internal node type used for calculations. In addition
    *   to the expected velocity at the node and the coordinate
    *   of the node, a start time value is pre-calculated in
    *   finalizeNewData. This value is used to determine between
    *   which nodes the interpolation calculations should occur.
    *
    */ 

   struct InterpolationNode {
      float64 nodeTimeSec;   // The time we expect the user to pass the node
      MC2Coordinate coord;   // The coordinate of the node
      float64 velocityMPS;   // The velocity of the node
   };
   
   /**
    *   Creates a new position interpolator with no data.
    */
   
   PositionInterpolator();

   virtual ~PositionInterpolator();
   
   /**
    *    @see InterpolationHintConsumer::prepareNewData
    */ 
   
   virtual void prepareNewData( unsigned int startTimeMillis );

   /**
    *    @see InterpolationHintConsumer::addDataPoint
    */ 

   virtual void addDataPoint( const MC2Coordinate& coord,
                              int velocityCmPS );

   /**
    *    @see InterpolationHintConsumer::finalizeNewData
    */ 

   virtual void finalizeNewData();

   /**
    *   Given a time and a final coordinate, find the total distance travelled
    *   along current configuration.
    *
    *   (A)------------(B)
    *                     \
    *                      \
    *                       \
    *                       (C)---x------(D)
    *
    *                       x = lastPositionCoord
    *
    *  If your position is x, you have completely traversed the paths
    *  from A-B, and from B-C. The final result is the sum of these
    *  distances (A-B and B-C) plus the distance from C to the coordinate x.
    */ 

   float64
   getTraversedDistanceAlongPathMeters( unsigned int timeMillis,
                                        MC2Coordinate lastPositionCoord ) const;

   float64
   getTraversedDistanceAlongPathMeters( unsigned int timeMillis ) const;
   
   /**
    *   
    *   Given a time point, calculate a position using the
    *   current interpolation data available. Along with the
    *   position, two additional attributes (velocity and direction)
    *   are calculated which can be seen as properties of the position.
    *
    *   Note that by calling this function, you may alter the internal
    *   state of the position interpolator. See the main class documentation
    *   that describes this behavior.
    *
    *   Repeated calls of this function requires that the time input parameter
    *   is strictly increasing (due to the state changes).
    *
    *   @param   timeMillis   The time of the desired position calculation.
    *   
    */ 
   
   InterpolatedPosition getInterpolatedPosition( unsigned int timeMillis );

   /**
    *    Returns the current interpolation path. Used for
    *    displaying debug interpolation information.
    *
    *    @return   Vector of InterpolationNodes representing the
    *              interpolation path.
    */ 
   
   std::vector<InterpolationNode> getInterpolationVector() const;

   /**
    *    Queries the class to determine whether or not the
    *    current data is useful for any interpolation
    *    calculations. Should be called to avoid unnecessary
    *    calculations.
    *
    *    @return   True if there is useful interpolation data
    *              available.
    */ 
   
   bool hasUsefulInterpolationData() const;

   /**
    *   Determines if we the supplied time is within
    *   the valid interpolation time frame. In other words,
    *   can we supply a position estimation given timeMillis?
    *
    *   @param   timeMillis   The time of the 
    */
   
   bool withinRange( unsigned int timeMillis ) const;

   /**
    *   Sets the allowed time factor for the catchup phase. This
    *   is defined relative to the current interpolation set period.
    *
    *   This means that in milliseconds, the maximum allowed catchup
    *   time is equal to factor * total period.
    */ 
   
   void setAllowedCatchUpFactor( float64 factor );
   
   /**
    *    TESTING: Disables the internal conversion from MC2 units to
    *    meters, causing the class to directly treat the
    *    incoming coordinates as meters instead.
    */ 
   
   void disableMC2Conversion();

   /**
    *   DEBUG: Return the delta error value between the two latest
    *   interpolation data sets. This value is calculated by taking the
    *   time of prepareNewData and calculating points from both sets.
    *  
    */
   
   float64 getPositionDeltaMeters() const;

   /**
    *   DEBUG: Return the speed of the latest interpolation set at the
    *   time of prepareNewData (this is assumed to be the only "correct"
    *   node in the set.
    */
   
   float64 getPrevCorrectSpeedKMPH() const;

   /**
    *   DEBUG: Return the speed of the previous interpolation set at the
    *   time of prepareNewData (this can be seen as the speed the interpolator
    *   thought the caller would have given the previous interpolation data).
    */
   
   float64 getPrevCalcSpeedKMPH() const;

   /**
    *   Determines if the provided time is within the catch up
    *   interval.
    */ 
   
   bool insideCatchUpPhase( unsigned int timeMillis ) const;


   virtual void cycleConfigurationForward();
   virtual void cycleConfigurationBackward();

   /**
    *   Returns the current configuration.
    */ 
   int getConfiguration() const;
   
private:
   void setConfiguration();
   
   /**
    *  @See getInterpolatedPosition.
    *
    *  Does the same calculations, but does not alter the internal state.
    *  I.e. the catch-up algorithm is never applied for the final result.
    */
   
   InterpolatedPosition
   peekInterpolatedPosition( unsigned int timeMillis ) const;
   
   /**  
    *    Calculates the estimated position of the user,
    *    given that the current time is timeMillis.
    *
    *    @param    timeMillis   The time to calculate with.
    */ 
   
   MC2Coordinate getPosition( unsigned int timeMillis ) const;

   /**  
    *    Calculates the estimated direction of the user,
    *    given that the current time is timeMillis.
    *
    *    @param   timeMillis    The time to calculate with.
    *    @return  float64         The estimated angle. 
    */ 
   
   float64 getDirectionDegrees( unsigned int timeMillis ) const;

   /**
    *    Calculates the current velocity of the user,
    *    given that the current time is timeMillis.
    *
    *    @param   timeMillis   The time to calculate with.
    *    @return  float64        The estimated velocity in
    *                            meters per second.
    */

   float64 getVelocityMPS( unsigned int timeMillis ) const;

   /**
    *   We often want to perform calculations on pair of nodes instead
    *   of individual nodes.
    */
   
   typedef std::pair<InterpolationNode, InterpolationNode> NodePair;
   typedef std::vector<InterpolationNode> NodeVec;
   typedef std::pair<NodeVec::const_iterator,
                     NodeVec::const_iterator> NodeIterPair;

   /**
    *   Returns a pair of iterators to the nodes representing
    *   the path that the position will be on the given time.
    */ 
   
   NodeIterPair getCurrentNodeIterPair( float64 timeSec ) const;

   /**
    *   Returns the pair of nodes that holds the path that the
    *   position will be on given that the time is timeSec.
    */ 
   
   NodePair getCurrentNodePair( float64 timeSec ) const;

   void initializeCatchUp();

   InterpolatedPosition getCatchUpPosition( unsigned int timeMillis );

   /**
    *   The collection of nodes that make up the current data
    *   set.
    */
   
   std::vector<InterpolationNode> m_data;

   /**
    *   Determines whether or not the interpolator should interpret
    *   the coordinates as MC2 and convert them to meters.
    */
   
   bool m_convertFromMC2;

   /**
    *   The starting time for the path.
    */
   
   unsigned int m_startTimeMillis;

   /**
    *   The lookahead time for angle calculations.
    */ 
   unsigned int m_angleLookAheadTimeMillis;

   /**
    *   Contains the current time period between successive
    *   calls to prepareNewData.
    */
   
   TimePeriod m_dataPeriodMillis;


   int m_numReturnedPositions;
   
   /**
    *   Catch-up related members.
    */
   
   struct {
      /**
       *   The time allowed to catch up to a new path.
       *   This is defined as a factor of the time period between
       *   the previous and the current path update.
       */
      float64 allowedCatchUpFactor;
      
      /**
       *   The starting position of the catch up interpolation.
       */
      InterpolatedPosition startPosition;

      /**
       *   The final position of the catch up interpolation.
       */

      InterpolatedPosition endPosition;

      TimePeriod periodMillis;
   } m_catchUp;
   
   /**
    *   Internal values that are used for debugging purposes.
    */

   enum InterpolationConfig { CONFIG_A = 0, CONFIG_B, CONFIG_C, CONFIG_D,
                              NUM_CONFIGS };
   
   struct {
      MC2Coordinate deltaCompareCoordinate;
      float64 deltaCompareAngle;
      
      float64 deltaMeters;
      float64 calcSpeedKMPH;
      float64 correctSpeedKMPH;
      
      int curConfig;
      bool disableInterpolation;
   } m_debugVals;
};

#endif /* _POSITIONINTERPOLATOR_H_ */
