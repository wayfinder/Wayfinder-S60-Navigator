/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _TILEMAPCONTROL_H_
#define _TILEMAPCONTROL_H_

// FORWARD DECLARATIONS

class DBufConnection;
class SharedHttpDBufRequester;
class SymbianTCPConnectionHandler;
class SymbianTileMapToolkit;
class TileMapEventListener;
class TileMapTextSettings;

#include <coecntrl.h>
#include <vector>
#include <e32std.h>
#include "config.h"
#include "MapRenderer.h"
#include "TileMapHandler.h"
#include "userbitmap.h"
#include "Cursor.h"
#include "MapRectInterface.h"

class MapLib;
class TileMapKeyHandler;

class CTileMapControl : public CCoeControl, public Cursor, public MapRectInterface
{
   /* variables */
   private:
      
      RArray<TUserBitmap*> iBitmapArray;
      /* platform specific toolkit */
      SymbianTileMapToolkit* iToolkit;
      /* the map plotter */
      isab::MapRenderer* iPlotter;
      /* for handling HTTP requests from/to the TMap Server */
      DBufRequester* iRequester;
      /* main class for displaying/requesting the TileMaps */
      TileMapHandler* iMapHandler;
      /** New main class for MapLib. Let's see if we will be able to replace
         the frequent use of tilemapHandler */
      MapLib* m_mapLib;

      /**
       * The TileMapKeyHandler. Is used to be able to delete
       * the keyhandler in the correct order.
       */
      TileMapKeyHandler* m_keyHandlerToDelete;
   
      /* counts the number of draws which happen */
      mutable uint32 iNumDraws;

      /* if set to true, draws the crosshair */
      bool iDrawCrosshair;

      /* if true, then feature name is drawn */
      bool iDrawFeatureName;

      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *   
       *   stores the current feature name and position 
       */
      std::vector<STRING*> m_infoStrings;
      
      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *   
       *   Stores the current text of the left button for the
       *   info stuff. (Somewhat series 60 specific)
       */
      STRING* m_infoDisplayLeftButton;
      
      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *   
       *   Stores the current text of the right button for the
       *   info stuff. (Somewhat series 60 specific)
       */
      STRING* m_infoDisplayRightButton;

      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       */
      void drawInfoText() const;

      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       */
      inline PixelBox getButtonPosition( bool left, STRING* text ) const;
      
   protected:
   public:


   /* methods */
   private:
      /* private constructor */
      CTileMapControl();
      /* second-phase constructor */
      void ConstructL( DBufRequester* aRequester, const TRect aRect,
                       TileMapEventListener* eventListener = NULL );
      /* Other second-phase constructor */
      void ConstructL( DBufConnection* aConnection,
                       const TRect& aRect,
                       TileMapEventListener* eventListener = NULL );
      
      /* draws the map ... from CCoeControl */
      void Draw(const TRect& aRect) const;

      /* handles a size change .. resizes the Plotter */
      void SizeChanged();

   public:

      /* Preferred allocation methods */
      static CTileMapControl*
         NewLC( DBufConnection* aRequester,
                const TRect& aRect,
                TileMapEventListener* eventListener = NULL );
      
      static CTileMapControl*
         NewL( DBufConnection* aRequester,
               const TRect& aRect,
               TileMapEventListener* eventListener = NULL );

      
      /* allocation methods */
      static CTileMapControl*
         NewLC( DBufRequester* aRequester,
                const TRect& aRect,
                TileMapEventListener* eventListener = NULL );
      
      static CTileMapControl*
         NewL( DBufRequester* aRequester,
               const TRect& aRect,
               TileMapEventListener* eventListener = NULL );

      /**
       *    Implements abstract method in Cursor.
       *    Currently doesn't do anything.
       */
      void setHighlight( bool highlight );

      /**
       *    Implements abstract method in Cursor.
       *    Currently doesn't do anything.
       */
      void setCursorPos( const MC2Point& pos );
      
      /**
       *    Implements abstract method in Cursor.
       *    Get the current cursor position.
       *    @return  The cursor position.
       */
      MC2Point getCursorPos() const;
         
      /**
       *    Implements abstract method in Cursor.
       *    Set if the cursor should be visible or hidden.
       *    @param   visible  If true, the cursor is visible, 
       *                      otherwise hidden.
       */
      void setCursorVisible( bool visible );

      /**
       *    Implements abstract method in Cursor.
       *    Get the box of the cursor.
       *    @param   box   PixelBox that will be set to the current
       *                   box of the cursor.
       */
      void getCursorBox( PixelBox& box ) const;
      
      /**
       *   Receives extra maps and gives them to the TileMapHandler.
       */
      void receiveExtraMaps( const uint8* serverMess,
                             int length );
      
      /* destructor */
      ~CTileMapControl();

      /* connects to the TMap Server and starts reuqesting\displaying maps */
      void Connect();

      /* moves the map by the specified pixels */
      void MoveMap(TInt dX, TInt dY);

      /* rotates the map to the left by the specified degrees */
      void RotateMapLeft(TInt angle);

      /* rotates the map to the right by the specified degrees */
      void RotateMapRight(TInt angle);

      /* zooms the map */
      void ZoomMap(float32 zoomFactor);

      /** @return The zoom scale of the map */
      double getScale() const;

      /* prints the name of the feature at the specified coordinate */
      void PrintFeatureName(const MC2Point& coord);

      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *
       *   Sets the info text of the info-square.
       *   @param utf8text If NULL no text will be displayed.
       */
      void setInfoText( const char* utf8text,
                        const char* leftButtonText = NULL,
                        const char* rightButtonText = NULL );

      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *
       *   Sets the info text of the info-square.
       *   @param utf8rows Each row of text in a vector.
       */
      void setInfoText( const std::vector<const char*>& utf8rows );

      /**
       * Gets the name of the feature at the specified coordinate.
       * @param   point    The screen coordinate.
       * @return  NULL if no feature name was found, 
       *          or else the feature name.
       */
      const char* getFeatureName(const MC2Point& point);

      /**
       *   Sets the text settings ( fonts ) for text placement
       *   in maps.
       */
      void setTextSettings( const TileMapTextSettings& settings );

      /* enables or disables the crosshair */
      void DrawCrosshair(bool drawflag)
      {
         iDrawCrosshair = drawflag;
         return;
      }
      
      /**
       *   DEPRECATED! Use MapInfoControl instead to draw info text.
       *
       *   Deletes the strings in the info-string vector and the buttons.
       */
      void deleteInfoStrings();

      /* resets the map rotation to zero */
      void ResetMapRotation() const {
         iMapHandler->setAngle(0);
         iMapHandler->requestRepaint();
         return;
      }

      /* get the TileMapHandler handle */
      TileMapHandler& Handler() const {
         return(*iMapHandler);
      }
     
      /** Return a pointer to the MapLib */
      MapLib* getMapLib() const {
         return m_mapLib;
      }

      /* get the MapPlotter handle */
      MapRenderer& Plotter() const {
         return(*iPlotter);
      }

      /// @return The tookit
      SymbianTileMapToolkit* getToolkit() const {
         return iToolkit;
      }

      DBufRequester* getTopRequester() const {
         return iRequester;
      }
      
      /* adds a user defined bitmap to bitmap array*/
      void addBitmapToArray(TUserBitmap& aUserBitmap);
      
      /* removes a user defined bitmap from bitmap array*/
      void removeBitmapFromArray(TUserBitmap& aUserBitmap);
      
      void drawUserDefinedBitmap() const;

      /// 
      PixelBox getMapRect() const {
         return Rect();
      }

      /**
       * Turns the night mode on or off.
       **/
      void SetNightModeL( bool aOn );
    
      /**
       * Sets the keyhandler and ownership, so that
       * it can be deleted by TileMapControl in a safe
       * way.
       */
      void setKeyHandlerAndOwnership( TileMapKeyHandler* keyHandler );

      /// Hidden cursor point.
      MC2Point m_hiddenCursorPoint;
};

#endif


