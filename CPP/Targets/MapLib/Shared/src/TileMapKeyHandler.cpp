/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "TileMapKeyHandler.h"

#include "TileMapToolkit.h"
#include "TileMapUtil.h"
#include "TileMapHandler.h"
#include "Cursor.h"

#include "TileMapUtil.h"

#ifdef __SYMBIAN32__
#include <e32math.h>
#else
#include <math.h>
#endif

#define TIMER_PERIOD 10

class TMKDestination {
public:
   TMKDestination( const MC2Coordinate& coord,
                   const double& scale ) :
         m_coord( coord ), m_scale( scale ) {}

   bool operator==(const TMKDestination& other ) const {
      return m_coord == other.m_coord && m_scale == other.m_scale;
   }

   bool operator!=(const TMKDestination& other ) const {
      return !(*this == other);
   }
   
   MC2Coordinate m_coord;
   double        m_scale;

#ifdef __unix__
   friend ostream& operator<<(ostream& ost, const TMKDestination& tmk ) {
      return ost << tmk.m_coord << ":" << tmk.m_scale;
   }
#endif   
};

#ifdef AUTOMOVE_ON

#define SLOW_TEST_PERIOD 1000
#define FAST_TEST_PERIOD 1
#include "TMKAutoMoveList.h"

#endif // AUTOMOVE_ON



inline void
TileMapKeyHandler::resetAllKeys()
{
   memset( m_key, 0, sizeof( m_key[0]) * NO_KEY ); 
}

TileMapKeyHandler::TileMapKeyHandler( 
                                MapMovingInterface* mapMovingInterface,
                                MapDrawingInterface* mapDrawingInterface,
                                TileMapToolkit* toolkit,
                                Cursor* cursor,
                                TileMapKeyHandlerCallback* callback,
                                TileMapInfoCallback* infoCallback ) 
   : m_staticCursor( false ),
     m_topLeftMapPos( 0, 0 ),
     m_firstPointOnScreen( 0, 0 ),
     m_infoCallback( infoCallback )
{
//   setMapBox( tileMapHandler->getMapSizePixels() );
   m_mapRedrawer    = mapDrawingInterface;
   m_cursor         = cursor;
   m_callBack       = callback;
   m_toolkit        = toolkit;
   m_tileMapHandler = mapMovingInterface;
   m_timerListener  =
      new TileMapTimerListenerTemplate<TileMapKeyHandler>(this);
#ifdef AUTOMOVE_ON
   m_timer          =      
      toolkit->createPeriodicTimer( m_timerListener,
                                    TileMapToolkit::PRIO_LOW);
#else
   m_timer          =      
      toolkit->createPeriodicTimer( m_timerListener,
                                    TileMapToolkit::PRIO_STANDARD);
#endif   
   m_mapState = STATIC;

#ifdef AUTOMOVE_ON
   // Automovement
   // Not used until we do the tests
   m_origPlace = new TMKDestination( m_tileMapHandler->getCenter(),
                                     m_tileMapHandler->getScale() );
   m_autoMoveList = new TMKAutoMoveList( mapMovingInterface,
                                         mapDrawingInterface );
   // End automovement
#endif

   resetAllKeys();
   deltaDefaults();

   //m_tileMapHandler->setDetectMovementBySelf( false );
}

TileMapKeyHandler::~TileMapKeyHandler()
{
   m_infoCallback = NULL;
   // Remove infocallback from tilemaphandler
   m_tileMapHandler->getInfoForFeatureAt( MC2Point(0,0),
                                          true,
                                          m_infoCallback ); // NULL
   m_timer->stop();
   delete m_timer;
   delete m_timerListener;
#ifdef AUTOMOVE_ON
   delete m_origPlace;
   delete m_autoMoveList;
#endif
   m_callBack = NULL;
   m_mapRedrawer = NULL;
}

inline void
TileMapKeyHandler::setMapState( TileMapKeyHandler::map_state_t state ) 
{
   // Check if entering or leaving moving state.
   if ( state == MOVING || state == ZOOMING ) {
      // Entering MOVING state.
      m_tileMapHandler->setMovementMode( true );
   } else if ( m_mapState == MOVING || m_mapState == ZOOMING ) {
      // Exiting MOVING state.
      m_tileMapHandler->setMovementMode( false );
   }
   m_mapState = state;
}

inline bool
TileMapKeyHandler::setMapStateIf( bool condition,
                                  TileMapKeyHandler::map_state_t state )
{
   if ( condition ) {
      setMapState( state );
   }
   return condition;
}

inline bool
TileMapKeyHandler::handleKeyDownEvent( TileMapKeyHandler::key_t key,
                                       bool& repaint,
                                       const MC2Point* dragToPoint )
{
   repaint = false;
   // Don't eat the key events when autozooming in.
   if ( m_mapState == AUTO_ZOOMING ) {
      return false;
   }
   bool handled = false;
   bool startTimerIfHandled = true;
   
   switch ( key ) {
      case MOVE_CURSOR_UP_KEY:
         handled = true;
         repaint = move( 0, -iMovDelta );
         if ( repaint ) {
            setMapState( MOVING );
         }         
         break;
      case MOVE_CURSOR_DOWN_KEY:
         handled = true;
         repaint = move( 0, iMovDelta );
         if ( repaint ) {
            setMapState( MOVING );
         }
         break;
      case MOVE_CURSOR_LEFT_KEY:
         handled = true;
         repaint = move( -iMovDelta, 0 );
         if ( repaint ) {
            setMapState( MOVING );
         }
         break;
      case MOVE_CURSOR_RIGHT_KEY:
         handled = true;
         repaint = move( iMovDelta, 0 );
         if ( repaint ) {
            setMapState( MOVING );
         }
         break;
      case MOVE_UP_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( 0, -iMovDelta );
         break;
      case MOVE_DOWN_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( 0, iMovDelta );
         break;
      case MOVE_LEFT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( -iMovDelta, 0 );
         break;
      case MOVE_RIGHT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( iMovDelta, 0 );
         break;
      case MOVE_UP_RIGHT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( iMovDelta, -iMovDelta );
         break;
      case MOVE_DOWN_RIGHT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( iMovDelta, iMovDelta );
         break;
      case MOVE_UP_LEFT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( -iMovDelta, -iMovDelta );
         break;
      case MOVE_DOWN_LEFT_KEY:
         handled = repaint = true;
         setMapState( MOVING );
         prepareMove();
         TileMapKeyHandler::move( -iMovDelta, iMovDelta );
         break;
      case ZOOM_IN_KEY:
      case ZOOM_OUT_KEY:
         handled = repaint = true;
         break;
      case AUTO_ZOOM_KEY:
         handled = repaint = true;
         // Save orig scale for the jump key
         m_autoZoomFinalScale = m_tileMapHandler->getScale();
         break;
      case ROTATE_LEFT_KEY:
      case ROTATE_RIGHT_KEY:
         handled = repaint = true;
         break;
      case RESET_ROTATION_KEY:
         handled = repaint = true;
         startTimerIfHandled = false;
         // Set angle to 0
         setMapState( STATIC );
         m_tileMapHandler->setAngle( 0 );
         break;
      case HIGHLIGHT_NEXT_FEATURE_KEY:
         setMapState( STATIC );
         handled = true;
         repaint = false;
         startTimerIfHandled = false;
         highlightNextFeature();
         break;
      case CENTER_MAP_AT_CURSOR_KEY:
         setMapState( STATIC );
         handled = true;
         repaint = true;
         startTimerIfHandled = false;
         centerMapAtCursor();
         break;
      case CENTER_MAP_AT_POSITION_KEY:
         setMapState( STATIC );
         handled = true;
         repaint = true;
         startTimerIfHandled = false;
         centerMapAtPosition(dragToPoint);
         break;
#ifdef AUTOMOVE_ON
      case FAST_TEST_KEY:
      case SLOW_TEST_KEY:
         // Only save start point when starting the test for the first time.
         if ( m_mapState != SLOW_TEST_RUNNING &&
              m_mapState != FAST_TEST_RUNNING ) {
            *m_origPlace = TMKDestination( m_tileMapHandler->getCenter(),
                                           m_tileMapHandler->getScale() );
            m_autoMoveList->reset();
         } else {
            m_autoMoveList->addRun();
         }
         handled = repaint = true;
         if ( key == FAST_TEST_KEY ) {
            setMapState( FAST_TEST_RUNNING );
         } else if ( key == SLOW_TEST_KEY ) {
            setMapState( SLOW_TEST_RUNNING );
         }
         break;
#endif
      case DRAG_TO:
         MC2_ASSERT( dragToPoint != NULL );
         m_firstPointOnScreen = *dragToPoint;
         // Store the coordinate of the dragToPoint in m_dragCoord.
         m_tileMapHandler->inverseTransform( m_dragCoord, 
                                             *dragToPoint );
#ifdef HANDLE_SCREEN_AS_BITMAP
         if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
            m_tileMapHandler->setBitmapDragPoint( *dragToPoint );
         }
#endif
         setMapState( MOVING );
         handled = true;
         repaint = false;
         startTimerIfHandled = false;
         break;
      case ZOOM_IN_ONE_STEP_KEY:
         setMapState( STATIC );
         zoom( 0.909091f );
         handled = true;
         repaint = true;
         startTimerIfHandled = false;
         break;
      case ZOOM_OUT_ONE_STEP_KEY:
         setMapState( STATIC );
         zoom( 1.1f );
         handled = true;
         repaint = true;
         startTimerIfHandled = false;
         break;
      case NO_KEY:
         // Not handled
         break;
   }
   
   if ( handled ) {
      if ( startTimerIfHandled ) {
         switch ( m_mapState ) {
#ifdef AUTOMOVE_ON
            case SLOW_TEST_RUNNING:
               m_timer->start( SLOW_TEST_PERIOD );
               break;
            case FAST_TEST_RUNNING:
               m_timer->start( FAST_TEST_PERIOD );
               break;
#endif
            default:
               m_timer->start( TIMER_PERIOD );
               break;
         }
         /* store the first keypress time only if it is not a repeated press */
         if(m_key[ key ] == 0) {
            m_key[ key ] = TileMapUtil::currentTimeMillis();
            m_lastDrawTime = TileMapUtil::currentTimeMillis() /* - 100 */;
         }
      }
   }
   
   return handled;
}

inline bool 
TileMapKeyHandler::handleKeyRepeatEvent( TileMapKeyHandler::key_t key,
                                         bool& repaint,
                                         const MC2Point* dragToPoint )
{
   repaint = false;
#ifdef AUTOMOVE_ON
   // Do not handle key repeat when the test is running.
   if ( m_mapState == SLOW_TEST_RUNNING ||
        m_mapState == FAST_TEST_RUNNING ) {
      return false;
   }
#endif

   switch ( key ) {
      case DRAG_TO:
         setMapState( MOVING );
         repaint = true;
         MC2_ASSERT( dragToPoint != NULL );
         if( abs(dragToPoint->getX() - m_firstPointOnScreen.getX()) > 3  ||
             abs(dragToPoint->getY() - m_firstPointOnScreen.getY()) > 3 ){
            dragTo( *dragToPoint );
         }
         break;
      default:
         break;
   }
   return repaint;
}

inline bool
TileMapKeyHandler::handleKeyUpEvent( TileMapKeyHandler::key_t key,
                                     bool& repaint,
                                     const char*& info,
                                     const MC2Point* dragToPoint )
{
   info = NULL;
   repaint = false;
#ifdef AUTOMOVE_ON
   // Do not handle key up when the test is running.
   if ( m_mapState == SLOW_TEST_RUNNING ||
        m_mapState == FAST_TEST_RUNNING ) {
      return false;
   }
#endif
  
   switch ( key ) {
      case DRAG_TO:
         repaint = true;
         MC2_ASSERT( dragToPoint != NULL );
         if( abs(dragToPoint->getX() - m_firstPointOnScreen.getX()) > 3  ||
             abs(dragToPoint->getY() - m_firstPointOnScreen.getY()) > 3 ){
            dragTo( *dragToPoint );
         }
         break;
      case CENTER_MAP_AT_POSITION_KEY:
         setMapState( STATIC );
         repaint = true;
         centerMapAtPosition(dragToPoint);
         break;
      default:
         break;
   }

   if ( ! m_key[ AUTO_ZOOM_KEY ] ) {
      /* set map stuff to defaults */
      setMapState( STATIC );
      deltaDefaults();
      if ( m_timer->active() ) {
         m_timer->stop();
      }
   } else {
      setMapState( AUTO_ZOOMING );
   }
   
   // Check if to highlight the cursor.
   info = checkHighlight();

   /* set key to not pressed */
   bool retVal = m_key[ key ];
   resetAllKeys();
   return retVal;
}

bool
TileMapKeyHandler::
handleKeyEvent( TileMapKeyHandler::key_t key,
                TileMapKeyHandler::kind_of_press_t upordown,
                const char*& info,
                const MC2Point* dragToPoint,
                bool* movement )
{
   info = NULL;
   // If auto zooming -> return
   if ( m_mapState == AUTO_ZOOMING ) {
      return false;
   }

   bool handled = false;
   bool repaint = false;

   switch ( upordown ) {
      case KEY_DOWN_EVENT:
         handled = handleKeyDownEvent( key, repaint, dragToPoint );
         break;
      case KEY_REPEAT_EVENT:
         handled = handleKeyRepeatEvent( key, repaint, 
                                         dragToPoint );
         break;
      case KEY_UP_EVENT:
         handled = handleKeyUpEvent( key, repaint, info, 
                                     dragToPoint );
         break;
      default:
         // No action on repeats.
         break;
   }
   
   if ( handled ) {
      if ( m_callBack ) {
         if ( m_callBack->keyHandlerCallback() ) {
            repaint = true;
         }
      }
      if ( repaint ) {
         m_mapRedrawer->requestRepaint();
      }
   }

   if ( movement != NULL ) {
      *movement = (m_mapState == MOVING || m_mapState == ZOOMING);
   }
   
   return handled;
}

void
TileMapKeyHandler::stop()
{
  
   // Reset autozoom
   
   if ( m_mapState == AUTO_ZOOMING || m_key[ AUTO_ZOOM_KEY ] ) {
      m_tileMapHandler->setScale( m_autoZoomFinalScale );
      if ( m_callBack ) {
         m_callBack->keyHandlerCallback();
      }
      m_mapRedrawer->requestRepaint();
   }
   /* zero out key array if we don't have key event focus */
   resetAllKeys();
   setMapState( STATIC );
   
   if ( m_timer->active() ) {
      m_timer->stop();
   }
}

#ifdef AUTOMOVE_ON
void
TileMapKeyHandler::handleTestStateTimer()
{
   if ( m_autoMoveList->handleStep() ) {
      // Nothing more needs doing
   } else {
      // Done - go back to normal state and orig place.
      setMapState( STATIC );
      if ( m_timer->active() ) {
         m_timer->stop();
      }
      m_tileMapHandler->setCenter( m_origPlace->m_coord );
      m_tileMapHandler->setScale( m_origPlace->m_scale );
      m_mapRedrawer->requestRepaint();
      resetAllKeys();
   }
}
#endif

bool
TileMapKeyHandler::handleKeyTimer()
{
#ifdef AUTOMOVE_ON   
   if ( m_mapState == SLOW_TEST_RUNNING ||
        m_mapState == FAST_TEST_RUNNING ) {
      handleTestStateTimer();
      return true;
   }
#endif
   
   /* zoom keys */
   if( m_key[ ZOOM_OUT_KEY ] || m_key[ AUTO_ZOOM_KEY ] ) //zoom out
   {
      if(m_mapState != ZOOMING) {
         if ( m_key[ AUTO_ZOOM_KEY ] ) {
            // Zoom out from center.
            TileMapKeyHandler::zoom( iZoomOutFactor );
         } else {
            zoom( iZoomOutFactor );
         }
         setMapState( ZOOMING );
      } else {
         if ( m_key [ AUTO_ZOOM_KEY ] ) {
            updateZoom( m_key[ AUTO_ZOOM_KEY ] );
            if(iZoomOutFactor != 1.0f) {
               // Zoom out from center.
               TileMapKeyHandler::zoom( iZoomOutFactor );
               return true;
            }
         } else {
            updateZoom( m_key[ ZOOM_OUT_KEY ] );
            if(iZoomOutFactor != 1.0f) {
               zoom( iZoomOutFactor );
               return true;
            }
         }
      }
   } else if(m_key[ ZOOM_IN_KEY ] ||
             ( m_mapState == AUTO_ZOOMING ) ) {
      // zoom in
      if(m_mapState != ZOOMING && m_mapState != AUTO_ZOOMING ) {
         zoom( iZoomInFactor );
         setMapState( ZOOMING );
         return true;
      } else if ( m_mapState != AUTO_ZOOMING ) {
         updateZoom(m_key[ ZOOM_IN_KEY ]);
         if(iZoomInFactor != 1.0f) {
            zoom( iZoomInFactor );
            return true;
         }
      } else {
         // Autozooming
         double scaleLeft = m_autoZoomFinalScale /
            m_tileMapHandler->getScale();
         if ( fabs( scaleLeft - 1.0 ) < 0.05 ) {
            m_tileMapHandler->setScale(
               m_tileMapHandler->getScale()* scaleLeft );
            // Ok - reset stuff
            setMapState( STATIC );
            m_timer->stop();
            resetAllKeys();
         } else {
            m_tileMapHandler->setScale(
               m_tileMapHandler->getScale()* MAX( 0.5, scaleLeft ) );
         }
         return true; // Always update repait when autozooming.
      }
   }
   /* movement handling, this is for repeated keypress */
   else if( m_key[ MOVE_CURSOR_DOWN_KEY ]) {
      updateMove(m_key[ MOVE_CURSOR_DOWN_KEY ]);
      if(iMovDelta) {
         return setMapStateIf( move( 0, iMovDelta ), MOVING );
      }
   }
   else if( m_key[ MOVE_CURSOR_UP_KEY ] ) {
      updateMove( m_key[ MOVE_CURSOR_UP_KEY ] );
      if(iMovDelta) {
         return setMapStateIf( move( 0, -iMovDelta ), MOVING );
      }
   }
   else if( m_key[ MOVE_CURSOR_LEFT_KEY ] ) {
      updateMove( m_key[ MOVE_CURSOR_LEFT_KEY ] );
      if(iMovDelta) {
         return setMapStateIf( move( -iMovDelta, 0 ), MOVING );
      }
   }
   else if( m_key[ MOVE_CURSOR_RIGHT_KEY ] ) {
      updateMove( m_key[ MOVE_CURSOR_RIGHT_KEY ] );
      if(iMovDelta) {
         return setMapStateIf( move( iMovDelta, 0 ), MOVING );
      }
   }
   else if( m_key[ MOVE_DOWN_KEY ]) {
      updateMove(m_key[ MOVE_DOWN_KEY ]);
      if(iMovDelta) {
         return TileMapKeyHandler::move( 0, iMovDelta );
      }
   }
   else if( m_key[ MOVE_UP_KEY ] ) {
      updateMove( m_key[ MOVE_UP_KEY ] );
      if(iMovDelta) {
         return TileMapKeyHandler::move( 0, -iMovDelta );
      }
   }
   else if( m_key[ MOVE_LEFT_KEY ] ) {
      updateMove( m_key[ MOVE_LEFT_KEY ] );
      if(iMovDelta) {
         return TileMapKeyHandler::move( -iMovDelta, 0 );
      }
   }
   else if( m_key[ MOVE_RIGHT_KEY ] ) {
      updateMove( m_key[ MOVE_RIGHT_KEY ] );
      if(iMovDelta) {
         return TileMapKeyHandler::move( iMovDelta, 0 );
      }
   }
   else if( m_key[ MOVE_DOWN_RIGHT_KEY ]) {
      updateMove(m_key[ MOVE_DOWN_RIGHT_KEY ]);
      if(iMovDelta) {
         return TileMapKeyHandler::move( iMovDelta, iMovDelta );
      }
   }
   else if( m_key[ MOVE_UP_RIGHT_KEY ] ) {
      updateMove( m_key[ MOVE_UP_RIGHT_KEY ] );
      if(iMovDelta) {
         return TileMapKeyHandler::move( iMovDelta, -iMovDelta );
      }
   }
   else if( m_key[ MOVE_DOWN_LEFT_KEY ]) {
      updateMove(m_key[ MOVE_DOWN_LEFT_KEY ]);
      if(iMovDelta) {
         return TileMapKeyHandler::move( -iMovDelta, iMovDelta );
      }
   }
   else if( m_key[ MOVE_UP_LEFT_KEY ] ) {
      updateMove( m_key[ MOVE_UP_LEFT_KEY ] );
      if(iMovDelta) {
         return TileMapKeyHandler::move( -iMovDelta, -iMovDelta );
      }
   }
   /* rotate keys */
   else
      if(m_key[ ROTATE_LEFT_KEY ])  // rotate left
      {
         //Stop tracking
         if(m_mapState != ROTATING) {
            setMapState( ROTATING );
            m_tileMapHandler->rotateLeftDeg(iRotDelta);
            return true;
         }
         else {
            updateRotation(m_key[ ROTATE_LEFT_KEY ]);
            if(iRotDelta) {
               m_tileMapHandler->rotateLeftDeg(iRotDelta);
               return true;
            }
         }
      }
      else if(m_key[ ROTATE_RIGHT_KEY ])   // rotate right
      {
         //Stop tracking
         if(m_mapState != ROTATING) {
            setMapState( ROTATING );
            m_tileMapHandler->rotateLeftDeg( -iRotDelta);
            return true;
         } else {
            updateRotation(m_key[ ROTATE_RIGHT_KEY ]);
            if(iRotDelta) {
               m_tileMapHandler->rotateLeftDeg(-iRotDelta);
               return true;
            }
         }
      }
   else
   {
      setMapState( STATIC );
      deltaDefaults();
      return false;
   }
   return false;
}

void
TileMapKeyHandler::timerExpired( uint32 )
{
   if ( handleKeyTimer() ) {
      if ( m_callBack ) {
         m_callBack->keyHandlerCallback();
      }
#ifdef HANDLE_SCREEN_AS_BITMAP
      if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
         m_mapRedrawer->requestRepaint();
      } else {
         m_mapRedrawer->repaintNow();
      }
#else
      m_mapRedrawer->repaintNow();
#endif
   }
}


// millisecond thresholds for keypresses
#define THRESHOLDONE 250
#define THRESHOLDTWO 500
#define THRESHOLDTHREE 1000

/* constant transform deltas */
// movement
#define  MOVEDELTA_DEFAULT  2
// rotation
#define  ROTDELTA_DEFAULT  15
#define  ROTDELTA_THRESHOLDONE  60
#define  ROTDELTA_THRESHOLDTWO  60
#define  ROTDELTA_THRESHOLDTHREE  60
#define  ROTDELTA_THRESHOLDFOUR  60
#define  ROTDELTA_MAX  90
// zooming ln(1.5) = 0.40546
#define  ZOOMDELTA_THRESHOLDONE  0.40546f
// zooming ln(1.75) = 0.55961f
#define  ZOOMDELTA_THRESHOLDTWO  0.55961f
// zooming ln(2.0) = 0.69314f
#define  ZOOMDELTA_THRESHOLDTHREE 0.69314f
// zooming ln(2.5) = 0.91629f
#define  ZOOMDELTA_THRESHOLDFOUR 0.91629f
#define  ZOOMDELTA_MAX           0.40546f

// Quicker zooming which almost only works on 7650
// ln(20)
//#define ZOOMDELTA_THRESHOLDFOUR 2.99573227355399099343f
//#define ZOOMDELTA_MAX ZOOMDELTA_THRESHOLDFOUR

/*
 * thresholds[] and deltas[] should have an equal number of values.
 * maxValue is used to check for bounds.
 */
inline float32
TileMapKeyHandler::getDelta(const float32* thresholds, 
                            const float32* deltas,
                            int nbr,
                            const float32* maxValues,
                            uint32 firstKeyPress) {
   float32 curDelta = 0;

   uint32 curKeyPress = TileMapUtil::currentTimeMillis();
   
   // calculate the time between keypresses
   uint32 curTime = curKeyPress - firstKeyPress;

   // time since last drawn.
   uint32 timeSinceLastDrawn = curKeyPress - m_lastDrawTime;
   
   float32 maxValue = 0.0;
   
   // check the curTime against the thresholds 
   // (currently, hardcoded for nbr values)
   for ( int i = 0; i < nbr; ++i ) {
      if ( curTime < thresholds[i] ) {
         curDelta = deltas[i] * ((float32) timeSinceLastDrawn / 1000.0f);
#ifdef ARCH_OS_WINDOWS
         // XXX: Until the deltas can be sent into TMKH, 
         // hardcode that windows needs 5 times larger deltas.
         curDelta *= 5;
#endif         
         maxValue = maxValues[ i ];
         break;
      }
   }

   
   
   // check against maxValue 
   if(curDelta > maxValue) {
      curDelta = maxValue;
   }

   if ( curDelta > 0 ) {
      m_lastDrawTime = curKeyPress;
   }
   
   return(curDelta);
}

/* updates the deltas according to the 
   time consumed for previous transform */
void TileMapKeyHandler::updateRotation(uint32 keyTime) {

   // create threshold and delta arrays
   static const float32
      thresholds[4] = { THRESHOLDONE, THRESHOLDTWO, THRESHOLDTHREE,
                        float(MAX_UINT32) };
   static const float32 deltas[4] = { ROTDELTA_THRESHOLDONE, 
                                      ROTDELTA_THRESHOLDTWO, 
                                      ROTDELTA_THRESHOLDTHREE,
                                      ROTDELTA_THRESHOLDFOUR };

   static const float32 maxVals[] = { ROTDELTA_MAX, 
                                      ROTDELTA_MAX, 
                                      ROTDELTA_MAX, 
                                      ROTDELTA_MAX };

   // get delta based on time
   float32 delta = getDelta(thresholds, deltas,
                            sizeof(thresholds) / sizeof( thresholds[0] ),
                            maxVals, keyTime);

   /* adjust the value, since it will be truncated to zero
      on conversion to integer */
   if(delta < 1.0f) {
      delta = 1.0f;
   }

   // set rotation delta to calculated value
   iRotDelta = (uint32) delta;

   return;
}

void TileMapKeyHandler::updateMove(uint32 keyTime) {

   // create threshold and delta arrays
   static const float32
      thresholds[] = { 125,
                       THRESHOLDONE,   // 250
                       THRESHOLDTWO,   // 500
                       THRESHOLDTHREE, // 1000
                       2000,
                       2500,
                       float(MAX_UINT32) };
   
   static const float32 deltas[] = { 0, 50, 100, 150, 150, 150, 150 };
      
#ifndef ENABLE_MAPPERFORMANCE_DEBUGGING 
   // Production values.
   static const float32 maxVals[] = { 0, 7, 7, 10, 15, 50, 50 };
#else   
   // Test values.
   static const float32 maxVals[] = { 0, 1, 1, 1, 1, 1, 1 };
#endif
   // get delta based on time
   float32 delta = getDelta(thresholds, deltas,
                            sizeof(thresholds) / sizeof( thresholds[0] ),
                            maxVals, keyTime);
   /* adjust the value, since it will be truncated to zero
      on conversion to integer */
   if(delta < 1.0f) {
      delta = 1.0f;
   }

   // set movement delta to calculated value
   iMovDelta = (uint32) delta;

   return;
}

void TileMapKeyHandler::updateZoom(uint32 keyTime) {

   // create threshold and delta arrays for zooming
   static const float32
      thresholds[4] = { THRESHOLDONE, THRESHOLDTWO, THRESHOLDTHREE,
                        float(MAX_UINT32) };
   static const float32
      deltas[4] = { ZOOMDELTA_THRESHOLDONE, 
                    ZOOMDELTA_THRESHOLDTWO, 
                    ZOOMDELTA_THRESHOLDTHREE,
                    ZOOMDELTA_THRESHOLDFOUR };
   
   static const float32 maxVals[] = { ZOOMDELTA_MAX, 
                                      ZOOMDELTA_MAX, 
                                      ZOOMDELTA_MAX, 
                                      ZOOMDELTA_MAX };
   
   // get zoom-in delta based on time
   const float32 delta = getDelta(thresholds, deltas,
                                  sizeof(thresholds) / sizeof( thresholds[0] ),
                                  maxVals, keyTime);
   // set zoom-in delta to calculated value
#ifdef __SYMBIAN32__
   TReal expDelta = 1.0;
   Math::Exp( expDelta, delta );
#else
   double expDelta = exp( delta );
#endif
   iZoomInFactor = 1 / expDelta;
  
   // set zoom-out delta to calculated value
   iZoomOutFactor = expDelta;

   return;
}

/* sets delta values to default */
void TileMapKeyHandler::deltaDefaults() {
   /* set to default values */
   iMovDelta = MOVEDELTA_DEFAULT;
   iRotDelta = ROTDELTA_DEFAULT;
   iZoomInFactor = 0.909091f;
   iZoomOutFactor = 1.1f;
   return;
}

void
TileMapKeyHandler::prepareMove()
{

}

bool
TileMapKeyHandler::move( int32 moveDeltaX, int32 moveDeltaY ) 
{
   // Actually moves the map.
   m_tileMapHandler->move( moveDeltaX, moveDeltaY );
   
#ifdef HANDLE_SCREEN_AS_BITMAP
   if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
      // Also move a screen snapshot.
      // Note that we don't negate the deltas.
      m_tileMapHandler->moveBitmap( moveDeltaX, moveDeltaY );
   }
#endif
   return true;
}

void
TileMapKeyHandler::zoom( double factor )
{
   m_tileMapHandler->zoom( factor );
   
#ifdef HANDLE_SCREEN_AS_BITMAP
   if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
      m_tileMapHandler->zoomBitmapAtCenter( factor );
   }
#endif   
}

void
TileMapKeyHandler::dragTo( const MC2Point& pointOnScreen ) 
{
   m_tileMapHandler->setPoint( m_dragCoord,
                               pointOnScreen );

#ifdef HANDLE_SCREEN_AS_BITMAP
   if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
      m_tileMapHandler->setPointBitmap( pointOnScreen );
   }
#endif
}

void
TileMapKeyHandler::highlightNextFeature()
{

}

void
TileMapKeyHandler::centerMapAtCursor()
{

}

void
TileMapKeyHandler::centerMapAtPosition(const MC2Point* aPosition)
{

}

void 
TileMapKeyHandler::resetZoomCoord()
{
   
}

MC2Point
TileMapKeyHandler::getCursorPosInMap() const
{
   if ( m_staticCursor ) {
      // The static cursor is always in the middle.
      return getMapBox().getCenterPoint();  
   }
   
   // This is the position of the cursor on the screen.
   MC2Point cursorPosOnScreen = m_cursor->getCursorPos();
   // However we need to return the position of the cursor relative to the
   // map, i.e. (0,0) should be the upper left corner.

   return cursorPosOnScreen - m_topLeftMapPos;
}

void
TileMapKeyHandler::setCursorVisible( bool visible )
{
   m_cursor->setCursorVisible( visible );
   // Also disable autohighlighting.
   m_tileMapHandler->setHighlight( visible );
}
  
void
TileMapKeyHandler::setCursorPos( const MC2Point& pos )
{
   if ( m_staticCursor ) {
      // The static cursor shall always be in the middle.
      m_cursor->setCursorPos(getMapBox().getCenterPoint());
   }
   else {
      // This the position of the cursor on the screen.
      m_cursor->setCursorPos( pos );
   }
}

void
TileMapKeyHandler::setCursorPosInMap( const MC2Point& pos )
{
   setCursorPos( pos + m_topLeftMapPos );
}

const char*
TileMapKeyHandler::checkHighlight()
{
   // Nothing needs to be done here. 
   return NULL;
}

void TileMapKeyHandler::cancelInfoCallback()
{
   m_infoCallback = NULL;
   // Remove infocallback from tilemaphandler
   if(m_tileMapHandler) {
      m_tileMapHandler->getInfoForFeatureAt( MC2Point(0,0),
                                             true,
                                             m_infoCallback ); // NULL
   }
}

void TileMapKeyHandler::cancelTileMapKeyHandlerCallback() {
	m_callBack = NULL;
}

void 
TileMapKeyHandler::setMapBox( const PixelBox& mapBox )
{
   m_mapBox = mapBox;
   m_topLeftMapPos = mapBox.getTopLeft();
}

PixelBox
TileMapKeyHandler::getMapBox() const 
{
   if ( m_mapBox.isValid() ) {
      return m_mapBox;
   } else {
      // XXX: Until all clients have started to use setMapBox.
      //return m_tileMapHandler->getMapSizePixels();
      return m_mapBox;
   }
}

const ClickInfo&
TileMapKeyHandler::getInfoForFeatureAt( 
                     const MC2Point& point,
                     bool onlyPois,
                     TileMapInfoCallback* infoCallback )
{
   return m_tileMapHandler->getInfoForFeatureAt( point,
                                                 onlyPois,
                                                 infoCallback );
}

void
TileMapKeyHandler::centerCursor()
{
   setCursorPos( getMapBox().getCenterPoint() ); 
}

void TileMapKeyHandler::setStaticCursor( bool staticCursor )
{
   m_staticCursor = staticCursor;
}

// -----------------------------------------------------------------------
// ------------------------ TileCursorKeyHandler -------------------------
// -----------------------------------------------------------------------

TileCursorKeyHandler::TileCursorKeyHandler( 
                         MapMovingInterface* mapMovingInterface,
                         MapDrawingInterface* mapDrawingInterface,
                         TileMapToolkit* toolkit,
                         Cursor* cursor,
                         TileMapKeyHandlerCallback* callback,
                         TileMapInfoCallback* infoCallback ) :
         TileMapKeyHandler( mapMovingInterface,
                            mapDrawingInterface,
                            toolkit, 
                            cursor,
                            callback,
                            infoCallback ),
         m_showCursor( true )
{

}

TileCursorKeyHandler::~TileCursorKeyHandler()
{
   
}

bool
TileCursorKeyHandler::move( int32 moveDeltaX, int32 moveDeltaY ) 
{
   if (m_staticCursor) {
      return TileMapKeyHandler::move(moveDeltaX, moveDeltaY);
   }

   PixelBox cursorBox;
   m_cursor->getCursorBox( cursorBox );
   
   MC2Point moveDeltas( moveDeltaX, moveDeltaY );
   cursorBox.move( moveDeltas );
   
   MC2Point offset = getMapBox().snapToBox( cursorBox );
 
   MC2Point center = cursorBox.getCenterPoint();
   
   if ( offset == moveDeltas ) {
      // The cursor is moved outside the screen.
      // Move the map.
      m_tileMapHandler->move( moveDeltaX, moveDeltaY );

#ifdef HANDLE_SCREEN_AS_BITMAP
      if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
         // Also move a screen snapshot.
         // Note that we don't negate the deltas.
         m_tileMapHandler->moveBitmap( moveDeltaX, moveDeltaY );
      }
#endif      
      // Don't highlight.
      m_cursor->setHighlight( false );
      return true;
   } else {
      // Move the cursor.
     
      // Check if to highlight
      checkHighlight( center, true );

      // Move the cursor.
      setCursorPos( center );
      
      // The cursor is moved, so it's now shown.
      m_showCursor = true;
      return false;
   }
}

void
TileCursorKeyHandler::zoom( double factor )
{
   // If the cursor is not shown, zoom in at the center.
   if ( ! m_showCursor ) {
      TileMapKeyHandler::zoom( factor );
      return;
   }

   MC2Point p = getCursorPosInMap();
   if ( m_mapState != ZOOMING ) {
      // First zoom.
      
      // Get the coordinate at the cursor.
      m_tileMapHandler->inverseTransform( m_zoomCoord, p );
 
      // Disable highlighting.
      m_cursor->setHighlight( false );
   }

   m_tileMapHandler->zoom( factor, m_zoomCoord, p );

#ifdef HANDLE_SCREEN_AS_BITMAP
   if ( m_tileMapHandler->canHandleScreenAsBitmap() ) {
      m_tileMapHandler->zoomBitmapAtPoint( factor, p );
   }
#endif
}

void
TileCursorKeyHandler::resetZoomCoord()
{
   m_zoomCoord = m_tileMapHandler->getCenter();
}

void
TileCursorKeyHandler::highlightNextFeature()
{
   if ( ! m_showCursor ) {
      // Highlighting not allowed when the cursor is not shown.
      return;
   }
   
   MC2Point p( 0, 0 );
   if ( m_tileMapHandler->getNextHighlightPoint( p ) ) {
      // Found a coordinate to highlight.

      // Enable highlighting.
      m_cursor->setHighlight( true );
      // Move the cursor to there.
      setCursorPosInMap( p );
   }
}

void
TileCursorKeyHandler::centerMapAtCursor()
{
   // Only allowed to do this when the cursor is visible. 
   if ( ! m_showCursor ) {
      return;
   }

   MC2Point p = getCursorPosInMap();

   MC2Coordinate centerCoord;
   // Get the coordinate at the cursor.
   m_tileMapHandler->inverseTransform( centerCoord, p );
 
   // Disable highlighting.
   m_cursor->setHighlight( false );
   
   // Set cursor to center of screen.
   centerCursor();
   
   // Set center coordinate.
   m_tileMapHandler->setCenter( centerCoord );
   
}

void
TileCursorKeyHandler::centerMapAtPosition(const MC2Point* aPosition)
{
   if(aPosition) {
      MC2Coordinate centerCoord;
      // Get the coordinate at the cursor.
      m_tileMapHandler->inverseTransform( centerCoord, *aPosition );

      // Set center coordinate.
      m_tileMapHandler->setCenter( centerCoord );
   }
}

void
TileCursorKeyHandler::prepareMove()
{
   // Remove any highlighting.
   m_cursor->setHighlight( false );
}

void
TileCursorKeyHandler::setCursorVisible( bool visible )
{
   m_showCursor = visible;
   m_cursor->setCursorVisible( visible );
}

const char*
TileCursorKeyHandler::checkHighlight( const MC2Point& pos, 
                                      bool onlyBitmaps ) 
{
   const ClickInfo& res = 
      m_tileMapHandler->getInfoForFeatureAt( pos,
                                             onlyBitmaps,
                                             m_infoCallback );
   
   if ( res.shouldHighlight() ) {
      // Enable highlighting.
      m_cursor->setHighlight( true );
   } else {
      // Disable highlighting.
      m_cursor->setHighlight( false );
   }
   m_featureName = res.getName();
   return m_featureName.empty() ? NULL : m_featureName.c_str() ;
}

const char*
TileCursorKeyHandler::checkHighlight()
{
   if ( ! m_showCursor ) {
      return NULL;
   }
   // Check highlight of the cursor.
   MC2Point p = getCursorPosInMap();
   // Don't just check bitmaps. The feature names of other feature types
   // are also interesting.
   return checkHighlight( p, false ); 
}
