/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "TestFramework.h"
#include "PositionInterpolatorTests.h"
#include "PositionInterpolator.h"
#include <cmath>
#include "Utility.h"
#include <stdio.h>

void PositionInterpolatorTests::testConstantSpeed( TestReporter& reporter )
{   
   NEW_TEST_FUNCTION( reporter );
   PositionInterpolator interpolator;
   PositionInterpolatorTester tester( interpolator );
   
   /**
    *   Since we want to test the system using a neutral
    *   coordinate space with meters, we disable the MC2->Meters
    *   conversion inside the interpolator.
    */

   interpolator.disableMC2Conversion();

   InterpolationHintConsumer& pathConsumer = interpolator;
   pathConsumer.prepareNewData( 0 );

   /*       TEST CASE 1
    *       -----------
    *
    *
    *       |         (100, 200, vel=1)
    *       |              *+*
    *       |                -\
    *       |                  -\
    *       |                    -\
    *       |                      -\
    *       |                        -\
    *       |                          -\
    *       |          (100, 100, vel=1) -\
    *       |               *+*-----------*+*
    *       |                |       (200, 100, vel=1)
    *       |                |
    *       |                |
    *       |                |
    *       |                |
    *       |                |
    *       +---------------*+*----------------
    *                  (0, 100, vel=1)
    *
    *
    *       The test case is built up from four nodes, each
    *       with a velocity of 1 units/per second.
    *
    *       We test at 7 different points, which are the
    *       nodes themselves and the midpoints between them.
    */

   static const int cmToM = 100;
   
   pathConsumer.addDataPoint( MC2Coordinate( 100, 0 ), 1 * cmToM );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 100 ), 1 * cmToM );
   pathConsumer.addDataPoint( MC2Coordinate( 200, 100 ), 1 * cmToM );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 200 ), 1 * cmToM );

   pathConsumer.finalizeNewData();

   /**
    *   Seconds to millisecond conversion unit.
    */
   static const int sToMs = 1000;

   /**
    *   The first five positions will be traversed along
    *   the coordinate axes, giving us proportional time
    *   steps.
    */

   MC2Coordinate pos1 = tester.getPosition( 0 );
   MC2Coordinate pos2 = tester.getPosition( 50 * sToMs );
   MC2Coordinate pos3 = tester.getPosition( 100 * sToMs );
   MC2Coordinate pos4 = tester.getPosition( 150 * sToMs );
   MC2Coordinate pos5 = tester.getPosition( 200 * sToMs );

   /**
    *   The last two positions follow a diagonal line. Thus, we need
    *   to calculate the length of this line. Since we use a constant
    *   speed, the length of the path will be equal to the time it takes
    *   to traverse it. Thus, it follows that half of the path will have
    *   been traversed in half that time.
    */

   double fullTime = sqrt( 100 * 100 + 100 * 100 );
   unsigned int time6 = 200 * sToMs +
      static_cast<int>( fullTime / 2.0 * sToMs + 0.5 );
   unsigned int time7 = 200 * sToMs +
      static_cast<int>( fullTime / 1.0 * sToMs + 0.5 );
   
   MC2Coordinate pos6 = tester.getPosition( time6 );

   MC2Coordinate pos7 = tester.getPosition( time7 );

   CHECK( pos1 == MC2Coordinate( 100, 0 ), reporter );
   CHECK( pos2 == MC2Coordinate( 100, 50 ), reporter );
   CHECK( pos3 == MC2Coordinate( 100, 100 ), reporter );
   CHECK( pos4 == MC2Coordinate( 150, 100 ), reporter );
   CHECK( pos5 == MC2Coordinate( 200, 100 ), reporter );
   CHECK( pos6 == MC2Coordinate( 150, 150 ), reporter );
   CHECK( pos7 == MC2Coordinate( 100, 200 ), reporter );
   for ( unsigned int t = 0; t < 200*sToMs; t += 50 * sToMs ) {
      CHECK( tester.getVelocityMPS( t ) == 1, reporter );
   }

}

void PositionInterpolatorTests::testIncreasingSpeed( TestReporter& reporter )
{
   NEW_TEST_FUNCTION( reporter );
   PositionInterpolator interpolator;
   PositionInterpolatorTester tester( interpolator );

   interpolator.disableMC2Conversion();

   InterpolationHintConsumer& pathConsumer = interpolator;
   pathConsumer.prepareNewData( 0 );

   static const int MPSToCmPS = 100;
   
   pathConsumer.addDataPoint( MC2Coordinate( 0, 0 ), 0 );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 0 ), 10 * MPSToCmPS );

   pathConsumer.finalizeNewData();

   /**
    *   Since we have a velocity which will increase from 0 to 10 meters
    *   per second over the interval, we cannot assume a linear time space.
    *
    *   We test the velocity function by choosing three time points within
    *   the interval. At these points, the distance traversed will be 25,
    *   50 and 75 meters, respectively.
    *
    *   To obtain the times when these distances have been traversed, we will
    *   actually need to solve the integral function for these time points.
    *
    *   The velocity functions look like this:
    *
    *   f(t) = k * t + m
    *
    *   And what we are looking for is:
    *
    *   F(t) = distance unit
    *   F(t) = k * t^2 + m * t
    *
    *   The total time to traverse is calculated by using the average speed.
    *
    *   Total time = (total distance) / (avg speed)
    *   Average speed = ( 10 - 0 ) / 2 = 5 meters per second
    *   Total time = 100 meters / 5 meters per second = 20 seconds
    *
    *   We can then draw the velocity function as this:
    *
    *   
    *   | (velocity)        10 mps
    *   |                  /--
    *   |              /---
    *   |          /---
    *   |      /---
    *   |  /---
    *   +------------------------
    *   (time)               20s
    *
    *
    *   The variable k is the slope of this function, thus:
    *
    *   k = 10 / 20 = 0.5
    *
    *   The variable m is the initial speed, which is 0 in our case.
    *   (Also known as the y-intercept).
    *
    *   Our primitive function will then look like this:
    *
    *   F(t) = 0.5 * t^2 / 2 + 0 * t = 0.25 * t^2
    *       
    *   And we want to solve it for 25, 50 and 75 meters:
    *
    *   0.25 * t ^ 2 = 25  =>  t = sqrt( 100 )
    *   0.25 * t ^ 2 = 50  =>  t = sqrt( 200 )
    *   0.25 * t ^ 2 = 75  =>  t = sqrt( 300 )
    *
    *   These are the times we test with below.
    *   
    */

   static const float sToMs = 1000.0f;

   unsigned int firstQuarterTime =
      static_cast<unsigned int>( sqrt( 100 ) * sToMs );
   unsigned int midTime =
      static_cast<unsigned int>( sqrt( 200 ) * sToMs );
   unsigned int thirdQuarterTime =
      static_cast<unsigned int>( sqrt( 300 ) * sToMs );
   
   MC2Coordinate pos1 = tester.getPosition( firstQuarterTime );
   MC2Coordinate pos2 = tester.getPosition( midTime );
   MC2Coordinate pos3 = tester.getPosition( thirdQuarterTime );

   CHECK( pos1 == MC2Coordinate( 25, 0 ), reporter );
   CHECK( pos2 == MC2Coordinate( 50, 0 ), reporter );
   CHECK( pos3 == MC2Coordinate( 75, 0 ), reporter );
   
   /**
    *   The speed will increase linearly, and is therefore
    *   a lot easier to test.
    */
   unsigned int sToMs_int = 1000;
   float64 speed1 = tester.getVelocityMPS( 0 );
   float64 speed2 = tester.getVelocityMPS( 10 * sToMs_int );
   float64 speed3 = tester.getVelocityMPS( 20 * sToMs_int );

   CHECK( speed1 == 0.0, reporter );
   CHECK( speed2 == 5.0, reporter );
   CHECK( speed3 == 10.0, reporter );
   
}

void PositionInterpolatorTests::testDecreasingSpeed( TestReporter& reporter )
{
   NEW_TEST_FUNCTION( reporter );
   PositionInterpolator interpolator;
   PositionInterpolatorTester tester( interpolator );

   interpolator.disableMC2Conversion();

   InterpolationHintConsumer& pathConsumer = interpolator;
   pathConsumer.prepareNewData( 0 );

   static const int MPSToCmPS = 100;
   
   pathConsumer.addDataPoint( MC2Coordinate( 0, 0 ), 10 * MPSToCmPS );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 0 ), 0 * MPSToCmPS );

   pathConsumer.finalizeNewData();

   /**
    *   In this case, some of the variables are different from
    *   testIncreasingSpeed. Since we decrease speed rather than
    *   increase, k will shift signs to be -0.5.
    *
    *   In testIncreasingSpeed, we also had an initial velocity of 0,
    *   giving us a m-value of 0. In this case, we have an initial
    *   velocity of 10, giving us an m equal to 10.
    *
    *   This changes the integral function to:
    *
    *   F(t) = -0.5 * t / 2 + 10 * t
    *
    *   As before, we want to test the following:
    *
    *   F(t) = 25
    *   F(t) = 50
    *   F(t) = 75
    *
    *   For the distance of 25 meters, we get the following equation:
    *
    *   -0.25 * t^2 + 10 * t = 25
    *
    *   Which we can rewrite as:
    *
    *   t^2 - 40 * t + 100 = 0
    *
    *   Giving us:
    *
    *   t = 20 +- sqrt( 20^2 - 100 )
    *
    *   We are only interested in the smaller result of the two. We can
    *   reassure ourselves mentally of this by noting that all times
    *   within the interval will be between 0 and 20.
    *
    *   t = 20 - sqrt( 300 )
    *
    *   The other time points will be equal to 20 - sqrt( 200 ) and 20 -
    *   sqrt( 100 ).
    *
    *   If this is unclear, please consult the revered standard piece
    *   of literature in the field ("Analys i en variabel"), written
    *   by Lars-Christer Boiers (Lars-Christer.Boiers@math.lth.se) and
    *   Arne Persson.
    *
    */

   /**
    *   Seconds to millisecond conversion unit.
    */
   
   static const float sToMs = 1000.0f;
   
   unsigned int firstQuarterTime =
      static_cast<unsigned int>( ( 20.0f - sqrt( 300 ) ) * sToMs );
   unsigned int midTime =
      static_cast<unsigned int>( ( 20.0f - sqrt( 200 ) ) * sToMs );
   unsigned int thirdQuarterTime =
      static_cast<unsigned int>( ( 20.0f - sqrt( 100 ) ) * sToMs);

   MC2Coordinate pos1 = tester.getPosition( firstQuarterTime );
   MC2Coordinate pos2 = tester.getPosition( midTime );
   MC2Coordinate pos3 = tester.getPosition( thirdQuarterTime );

   CHECK( pos1 == MC2Coordinate( 25, 0 ), reporter );
   CHECK( pos2 == MC2Coordinate( 50, 0 ), reporter );
   CHECK( pos3 == MC2Coordinate( 75, 0 ), reporter );

   /**
    *   The speed will decrease linearly, and is therefore
    *   a lot easier to test.
    */
   unsigned int sToMs_int = 1000;
   float64 speed1 = tester.getVelocityMPS( 0 );
   float64 speed2 = tester.getVelocityMPS( 10 * sToMs_int );
   float64 speed3 = tester.getVelocityMPS( 20 * sToMs_int );

   CHECK( speed1 == 10.0, reporter );
   CHECK( speed2 == 5.0, reporter );
   CHECK( speed3 == 0.0, reporter );
}

void PositionInterpolatorTests::testExtremes( TestReporter& reporter )
{
   NEW_TEST_FUNCTION( reporter );
   PositionInterpolator interpolator;
   PositionInterpolatorTester tester( interpolator );
   
   interpolator.disableMC2Conversion();

   /**
    *
    *   No data. No correct calculations or assumptions could be made,
    *   so the position 0, 0 is returned.
    *
    */

   CHECK( !tester.getPosition( 1000 ).isValid(), reporter );

   InterpolationHintConsumer& pathConsumer = interpolator;

   pathConsumer.prepareNewData( 1000 );
   
   MC2Coordinate checkPosition( 1337, 1337 );

   static const int MPSToCmPS = 100;
   
   pathConsumer.addDataPoint( checkPosition, 10 * MPSToCmPS );
   
   pathConsumer.finalizeNewData();

   /**
    *   Only single point, should return point itself.
    */

   CHECK( tester.getPosition( 0 ) == checkPosition, reporter );
   CHECK( tester.getPosition( 1000 ) == checkPosition, reporter );
   CHECK( tester.getPosition( 2000 ) == checkPosition, reporter );

   /**
    *   Two points, test outside scope on both sides of the interval.
    *   Should be clamped to endpoints.
    */
   
   MC2Coordinate endPosition( 1338, 1338 );
   pathConsumer.addDataPoint( endPosition, 10 * MPSToCmPS );

   pathConsumer.finalizeNewData();

   /**
    *   Seconds to millisecond conversion unit.
    */
   
   static const int sToMs = 1000;

   CHECK( tester.getPosition( 0 ) == checkPosition,
          reporter );
   
   CHECK( tester.getPosition( 500000 * sToMs ) == endPosition,
          reporter );
   
}

void PositionInterpolatorTests::testCatchUp( TestReporter& reporter )
{   
   NEW_TEST_FUNCTION( reporter );

   PositionInterpolator interpolator;
   interpolator.disableMC2Conversion();

   /**
    * A helper with friend access to PositionInterpolator's internals.
    */
   PositionInterpolatorTester tester( interpolator );
   
   float64 allowedCatchUpTime = 0.1;
   interpolator.setAllowedCatchUpFactor( allowedCatchUpTime );
   
   static const int sToMs = 1000;
   static const int mToCm = 100;
   unsigned int newDataPeriodSec = 5;

   /**
    *   The time allowed to catch up with the new path,
    *   expressed in milliseconds.
    */
   unsigned int allowedCatchUpTimeMillis =
      static_cast<unsigned int>( sToMs *
                                 newDataPeriodSec *
                                 allowedCatchUpTime );
   
   InterpolationHintConsumer& pathConsumer = interpolator;

   /**
    *   We construct a line of length 100 which will be
    *   traversed at 10 per second.
    */
   pathConsumer.prepareNewData( 0 * newDataPeriodSec * sToMs );
   pathConsumer.addDataPoint( MC2Coordinate( 0, 0 ), 10 * mToCm );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 0 ), 10 * mToCm );
   pathConsumer.finalizeNewData();
   
   InterpolatedPosition testPos =
      interpolator.getInterpolatedPosition( 2 * sToMs );

   CHECK( testPos.coord == MC2Coordinate( 20, 0 ), reporter );

   InterpolatedPosition lastPosFirstSet =
      interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs );

   CHECK( lastPosFirstSet.coord == MC2Coordinate( 50, 0 ), reporter );

   /**
    *   We update the data after 5 seconds, and say that we've already
    *   reached to 60.
    */
   pathConsumer.prepareNewData( 1 * newDataPeriodSec * sToMs );
   pathConsumer.addDataPoint( MC2Coordinate( 60, 0 ), 10 * mToCm );
   pathConsumer.addDataPoint( MC2Coordinate( 100, 0 ), 10 * mToCm );
   pathConsumer.finalizeNewData();

   /**
    *   Check so that the insideCatchUpPhase method is correct.
    */
   
   CHECK( !tester.insideCatchUpPhase( newDataPeriodSec * sToMs - 1) ,
          reporter );

   CHECK( tester.insideCatchUpPhase( newDataPeriodSec * sToMs ) ,
          reporter );
   CHECK( tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                     allowedCatchUpTimeMillis / 2 ) ,
          reporter );
   CHECK( tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                     allowedCatchUpTimeMillis
                                     - 1 ),
          reporter );
   
   CHECK( !tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                      allowedCatchUpTimeMillis ),
          reporter );

   CHECK( !tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                      allowedCatchUpTimeMillis +
                                      1 * sToMs ),
          reporter );

   CHECK( !tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                      allowedCatchUpTimeMillis +
                                      5 * sToMs ),
          reporter );
   CHECK( !tester.insideCatchUpPhase( newDataPeriodSec * sToMs +
                                      allowedCatchUpTimeMillis +
                                      100 * sToMs ),
          reporter );   

   /**
    *   Check so that the first position of the second set is equal
    *   to the last position in the first set.
    */

   InterpolatedPosition firstPosSecondSet =
      interpolator.getInterpolatedPosition( 1 * newDataPeriodSec * sToMs );
   InterpolatedPosition caughtUpPos =
      interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs +
                                            allowedCatchUpTimeMillis );

   CHECK( firstPosSecondSet.coord == lastPosFirstSet.coord, reporter );
   CHECK( caughtUpPos.coord == MC2Coordinate( 65, 0 ), reporter );
}

PositionInterpolatorTester::PositionInterpolatorTester(
   PositionInterpolator& interpolator )
 :m_interpolator( interpolator )
{
    
}

bool PositionInterpolatorTester::insideCatchUpPhase( unsigned int timeMillis )
{
   return m_interpolator.insideCatchUpPhase( timeMillis );
}

void PositionInterpolatorTests::testTraversedDistance( TestReporter& reporter )
{
   NEW_TEST_FUNCTION( reporter );

   PositionInterpolator interpolator;
   interpolator.disableMC2Conversion();

   static const int mToCm = 100;
   
   interpolator.prepareNewData( 0 * 0 );
   interpolator.addDataPoint( MC2Coordinate( 0, 0 ),  100 * mToCm );
   interpolator.addDataPoint( MC2Coordinate( 100, 0 ), 100 * mToCm );
   interpolator.addDataPoint( MC2Coordinate( 100, 100 ), 100 * mToCm );
   interpolator.finalizeNewData();

   float64 dist1Meters = interpolator.getTraversedDistanceAlongPathMeters(
      500, MC2Coordinate( 0, 0 ) );

   float64 dist2Meters = interpolator.getTraversedDistanceAlongPathMeters(
      500, MC2Coordinate( 50, 0 ) );

   float64 dist3Meters = interpolator.getTraversedDistanceAlongPathMeters(
      500, MC2Coordinate( 100, 0 ) );

   float64 dist4Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1000, MC2Coordinate( 100, 0 ) );

   float64 dist5Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1500, MC2Coordinate( 100, 0 ) );
   
   float64 dist6Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1500, MC2Coordinate( 100, 50 ) );

   float64 dist7Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1500, MC2Coordinate( 100, 75 ) );

   float64 dist8Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1500, MC2Coordinate( 100, 25 ) );

   float64 dist9Meters = interpolator.getTraversedDistanceAlongPathMeters(
      1500, MC2Coordinate( 100, 100 ) );

   float64 dist10Meters = interpolator.getTraversedDistanceAlongPathMeters(
      2000, MC2Coordinate( 100, 100 ) );

   float64 dist11Meters = interpolator.getTraversedDistanceAlongPathMeters(
      25600, MC2Coordinate( 100, 100 ) );

   float64 dist12Meters = interpolator.getTraversedDistanceAlongPathMeters(
      25600, MC2Coordinate( 100, 200 ) );
   
   CHECK( dist1Meters == 0.0f, reporter );
   CHECK( dist2Meters == 50.0f, reporter );
   CHECK( dist3Meters == 100.0f, reporter );
   CHECK( dist4Meters == 100.0f, reporter );
   CHECK( dist5Meters == 100.0f, reporter );
   CHECK( dist6Meters == 150.0f, reporter );
   CHECK( dist7Meters == 175.0f, reporter );
   CHECK( dist8Meters == 125.0f, reporter );
   CHECK( dist9Meters == 200.0f, reporter );
   CHECK( dist10Meters == 200.0f, reporter );
   CHECK( dist11Meters == 200.0f, reporter );
   CHECK( dist12Meters == 300.0f, reporter );
}

void PositionInterpolatorTests::testCatchUpDirection( TestReporter& reporter )
{
   NEW_TEST_FUNCTION( reporter );

   PositionInterpolator interpolator;
   interpolator.disableMC2Conversion();

   PositionInterpolatorTester tester( interpolator );
   
   float64 allowedCatchUpTime = 0.1;
   interpolator.setAllowedCatchUpFactor( allowedCatchUpTime );
   
   static const int sToMs = 1000;
   static const int mToCm = 100;
   unsigned int newDataPeriodSec = 5;

   /**
    *   The time allowed to catch up with the new path,
    *   expressed in milliseconds.
    */
   // unsigned int allowedCatchUpTimeMillis =
   //    static_cast<unsigned int>( sToMs *
   //                               newDataPeriodSec *
   //                               allowedCatchUpTime );
   
   InterpolationHintConsumer& pathConsumer = interpolator;


   pathConsumer.prepareNewData( 0 * newDataPeriodSec * sToMs );
   pathConsumer.addDataPoint( MC2Coordinate( 0, 0 ), 10 * mToCm );
   pathConsumer.addDataPoint( MC2Coordinate( 60, 0 ), 10 * mToCm );
   pathConsumer.addDataPoint( MC2Coordinate( 60, 60 ), 10 * mToCm );
   pathConsumer.finalizeNewData();

   interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs );
   
   pathConsumer.prepareNewData( 1 * newDataPeriodSec * sToMs );
   pathConsumer.addDataPoint( MC2Coordinate( 60, 10 ), 10 * mToCm );
   pathConsumer.addDataPoint( MC2Coordinate( 60, 60 ), 10 * mToCm );
   pathConsumer.finalizeNewData();
   
   // InterpolatedPosition startPos =
   //    interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs );
   
   // InterpolatedPosition midPos =
   //    interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs +
   //                                          allowedCatchUpTimeMillis / 2 );
   // InterpolatedPosition endPos =
   //    interpolator.getInterpolatedPosition( newDataPeriodSec * sToMs +
   //                                          allowedCatchUpTimeMillis );
}

float64
PositionInterpolatorTester::getVelocityMPS( unsigned int timeMilliSec ) const
{
   return m_interpolator.getVelocityMPS( timeMilliSec );
}

float64
PositionInterpolatorTester::getDirectionDegrees( unsigned int timeMilliSec ) const
{
   return m_interpolator.getDirectionDegrees( timeMilliSec );
}


MC2Coordinate
PositionInterpolatorTester::getPosition( unsigned int timeMilliSec )
{
   return m_interpolator.getPosition( timeMilliSec );
}





