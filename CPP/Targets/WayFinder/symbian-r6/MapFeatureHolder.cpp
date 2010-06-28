/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <e32base.h>
#include "arch.h"

#include "DirectedPolygon.h"
#include "UserDefinedBitMapFeature.h"
#include "MC2Point.h"

#include "GuiProt/Favorite.h"
#include "MapFeatureHolder.h"
#include "WFLayoutUtils.h"

using namespace std;


#define POSITION_POLY_COUNT 4
#define DARK_SHADE_POLY_COUNT 3
#define LIGHT_SHADE_POLY_COUNT 3

const int16 positionPoly[POSITION_POLY_COUNT*2] = {
   0, -8,
   8,  8,
   0,  5,
   -8,  8,
};

const int16 darkShadePoly[DARK_SHADE_POLY_COUNT*2] = {
   0, -7,
   -7,  7,
   0,  4,
};

const int16 lightShadePoly[LIGHT_SHADE_POLY_COUNT*2] = {
   0, -7,
   7,  7,
   0,  4,
};

#define PED_POSITION_POLY_COUNT 16
#define PED_POSITION_FILL_POLY_COUNT 16

const int16 pedPositionPoly[PED_POSITION_POLY_COUNT*2] = {
    130, 314,
    240, 240,
    314, 130,
    340,   0,
    314,-130,
    240,-240,
    130,-314,
      0,-340,
   -130,-314,
   -240,-240,
   -314,-130,
   -340,   0,
   -314, 130,
   -240, 240,
   -130, 314,
      0, 340,
};

const int16 pedPositionFillPoly[PED_POSITION_FILL_POLY_COUNT*2] = {
   115, 277,
   212, 212,
   277, 115,
   300,   0,
   277,-115,
   212,-212,
   115,-277,
     0,-300,
  -115,-277,
  -212,-212,
  -277,-115,
  -300,   0,
  -277, 115,
  -212, 212,
  -115, 277,
     0, 300,
};

const int16 arrowPoly[4*2] = {
   0,-14,
   8,  0,
   0, 14,
   -8,  0,
};

const int16 northPoly[3*2] = {
   0,-12,
   6,  0,
   -6,  0,
};

const int16 southPoly[3*2] = {
   -6, 1,
   6, 1,
   0,12,
};

CMapFeatureHolder::CMapFeatureHolder()
{
   iCurrentFavIconSize = 0;
   iPositionPolyScale = 1.0;
   iPedestrianPositionPolyScale = 1.0;
}

CMapFeatureHolder::~CMapFeatureHolder()
{
/*    RemoveAllFavorites(); */
/*    RemoveAllFeatures(); */
}

void
CMapFeatureHolder::AddFeature(UserDefinedFeature *udf, CMapFeature *mf)
{
   // Replaces the element if udf already exists.
   iFeatures[udf] = mf;
   iUserDefFeatures.push_back(udf);
}

void
CMapFeatureHolder::MoveFeatureLast(UserDefinedFeature *udf)
{
   TBool found = EFalse;
   /* Move the feature to the end of the drawing queue. */
   vector<UserDefinedFeature*>::iterator it = iUserDefFeatures.begin();
   while( it != iUserDefFeatures.end() ){
      if( *it == udf ){
         found = ETrue;
         break;
      } else {
         ++it;
      }
   }          
   if( found ){
      iUserDefFeatures.erase(it);
      iUserDefFeatures.push_back(udf);
   }
}

CMapFeature*
CMapFeatureHolder::RemoveFeature(UserDefinedFeature *udf)
{
   my_type::iterator cur =
      iFeatures.find(udf);
   if (cur == iFeatures.end()) {
      /* Not found. */
      return NULL;
   }
   CMapFeature* mf = cur->second;

   TBool found = EFalse;
   vector<UserDefinedFeature*>::iterator it = iUserDefFeatures.begin();
   while( it != iUserDefFeatures.end() ){
      if( *it == udf ){
         found = ETrue;
         break;
      } else {
         ++it;
      }
   }
   if( found ){
      iUserDefFeatures.erase(it);
   }
   iFeatures.erase(cur);
   return mf;
}

CMapFeature*
CMapFeatureHolder::GetFeature(UserDefinedFeature *udf)
{
   my_type::iterator cur =
      iFeatures.find(udf);
   if (cur == iFeatures.end()) {
      /* Not found. */
      return NULL;
   }
   return cur->second;
}

udf_vector_type*
CMapFeatureHolder::GetUDFVector()
{
   return &iUserDefFeatures;
}

void
CMapFeatureHolder::InitPoints(const int16* poly, int num, vector<MC2Point>& points)
{
   points.clear();
   points.reserve(num);
   for (int i = 0; i < num; i++) {
      points.push_back( MC2Point(poly[i*2], poly[i*2+1]));
   }
}

void
CMapFeatureHolder::InitPolygons()
{
   iPositionPolyShown = EFalse;
   iPedPositionPolyShown = EFalse;
   iCompassPolyShown = EFalse;
   iPointBitmapShown = EFalse;
   iStartBitmapShown = EFalse;
   iEndBitmapShown = EFalse;
   iScaleFeatureShown = EFalse;

   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   MC2Point outsideScreen(-1000,-1000);
   vector<MC2Point> points;
   InitPoints(positionPoly, POSITION_POLY_COUNT, points);
   iPositionPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0x243D69, 1, dpiCorrectionFactor);
   iPositionPoly->rescalePoints(iPositionPolyScale);
   iPositionPoly->setClickable(0);
   iPositionPoly->setAlways2d( false );

   InitPoints(darkShadePoly, DARK_SHADE_POLY_COUNT, points);
   iDarkShadePoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0x243D69, 1, dpiCorrectionFactor);
   iDarkShadePoly->rescalePoints(iPositionPolyScale);
   iDarkShadePoly->setClickable(0);
   iDarkShadePoly->setAlways2d( false );

   InitPoints(lightShadePoly, LIGHT_SHADE_POLY_COUNT, points);
   iLightshadePoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0xB2ADF4, 1, dpiCorrectionFactor);
   iLightshadePoly->rescalePoints(iPositionPolyScale);
   iLightshadePoly->setClickable(0);
   iLightshadePoly->setAlways2d( false );

   InitPoints(pedPositionPoly, PED_POSITION_POLY_COUNT, points);
   iPedPositionPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0x000000, 1, dpiCorrectionFactor);
   iPedPositionPoly->rescalePoints(iPedestrianPositionPolyScale);
   iPedPositionPoly->setClickable(0);
   iPedPositionPoly->setAlways2d( false );

   InitPoints(pedPositionFillPoly, PED_POSITION_FILL_POLY_COUNT, points);
   iPedPositionFillPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0x8CB229, 1, dpiCorrectionFactor);
   iPedPositionFillPoly->rescalePoints(iPedestrianPositionPolyScale);
   iPedPositionFillPoly->setClickable(0);
   iPedPositionFillPoly->setAlways2d( false );

   InitPoints(arrowPoly, 4, points);
   iArrowPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0x000000, 1, dpiCorrectionFactor);
   iArrowPoly->rescalePoints();
   iArrowPoly->setAngle(0);
   iArrowPoly->setClickable(0);
   iArrowPoly->setAlways2d( true );

   InitPoints(northPoly, 3, points);
   iNorthPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0xff0000, 1, dpiCorrectionFactor);
   iNorthPoly->rescalePoints();
   iNorthPoly->setAngle(0);
   iNorthPoly->setClickable(0);
   iNorthPoly->setAlways2d( true );

   InitPoints(southPoly, 3, points);
   iSouthPoly = new (ELeave) DirectedPolygon(points,
         outsideScreen, ETrue, 0xffffff, 1, dpiCorrectionFactor);
   iSouthPoly->rescalePoints();
   iSouthPoly->setAngle(0);
   iSouthPoly->setClickable(0);
   iSouthPoly->setAlways2d( true );

   iPointBitmap = new (ELeave) UserDefinedBitMapFeature(
      Nav2Coordinate(0, 0), "mappin", false );
   iPointBitmap->setClickable(0);

   iStartBitmap = new (ELeave) UserDefinedBitMapFeature(
      Nav2Coordinate(0, 0), "route_orig", false );
   iStartBitmap->setClickable(0);

   iEndBitmap = new (ELeave) UserDefinedBitMapFeature(
      Nav2Coordinate(0, 0), "route_dest", false );
   iEndBitmap->setClickable(0);

   iWaitSymbol = new (ELeave) UserDefinedBitMapFeature(
      Nav2Coordinate(0, 0), "hourglass", false );
   iWaitSymbol->setClickable(0);
   iWaitSymbol->setVisible( false );
   CMapFeature* waitFeature = new (ELeave) CMapFeature( map_feature_wait, 0);
   AddFeature( iWaitSymbol, waitFeature );
   
   iCellIdBitmap = new (ELeave) UserDefinedBitMapFeature(
      MC2Coordinate(0,0), "position_cellid", true );
   
   iCellIdBitmap->setClickable(0);
   iCellIdBitmap->setVisible( false );
   
   CMapFeature* cellIdFeature = new (ELeave) CMapFeature( map_feature_cellid_ring, 0);
   AddFeature( iCellIdBitmap, cellIdFeature );
   
   /* Scale indicator. */
   iScaleFeature = NULL;
}

void
CMapFeatureHolder::RemoveAll(udf_vector_type& vec)
{
   udf_vector_type::iterator it;
   CMapFeature* mf;

   it = vec.begin();
   while (it != vec.end()) {
      mf = RemoveFeature(*it);
      delete mf;
      delete *it;
      it++;
   }
   vec.clear();
}

void
CMapFeatureHolder::UpdateFavorites(favorite_vector_type* favVector, 
                                   const char* imgName, 
                                   const char* imgNameSmall)
{
   RemoveAll(iFavoriteUserDefFeatures);
   if (iCurrentFavIconSize == 0) {
      return;
   }
   uint32 id = 0;
   int32 lat = 0;
   int32 lon = 0;
   isab::Favorite* favorite;
   CMapFeature* mf;
   UserDefinedBitMapFeature* udbf;

   vector<isab::Favorite*>::iterator it;

   if( favVector->size() > 0 ){
      for( it = favVector->begin(); it != favVector->end(); it++ ){
         favorite = *it;
         id = favorite->getID();
         lat = favorite->getLat();
         lon = favorite->getLon();
         if (iCurrentFavIconSize == 2) {
            if (imgName) {
               udbf = new (ELeave) UserDefinedBitMapFeature(
                  Nav2Coordinate(0, 0), imgName, false );
            } else {
               udbf = new (ELeave) UserDefinedBitMapFeature(
                  Nav2Coordinate(0, 0), "favorite", false );
            }
         } else {
            if (imgNameSmall) {
               udbf = new (ELeave) UserDefinedBitMapFeature(
                  Nav2Coordinate(0, 0), imgNameSmall, false );
            } else {
               udbf = new (ELeave) UserDefinedBitMapFeature(
                  Nav2Coordinate(0, 0), "favorite_small", false );
            }
         }
         udbf->setCenter(Nav2Coordinate(lat, lon));
         if ( favorite->getName() != NULL ) {
            udbf->setName( favorite->getName() );
         }
         mf = new (ELeave) CMapFeature(map_feature_favorite, id);
         AddFeature( udbf, mf );
         iFavoriteUserDefFeatures.push_back(udbf);
      }
   }
//    MoveFeatureLast(iShadingPoly);
   MoveFeatureLast(iPositionPoly);
   MoveFeatureLast(iPedPositionPoly);
   MoveFeatureLast(iPedPositionFillPoly);
   MoveFeatureLast(iDarkShadePoly);
   MoveFeatureLast(iLightshadePoly);
   MoveFeatureLast(iArrowPoly);
   MoveFeatureLast(iNorthPoly);
   MoveFeatureLast(iSouthPoly);
   MoveFeatureLast(iWaitSymbol);
   MoveFeatureLast(iCellIdBitmap);
}

TInt
CMapFeatureHolder::GetCurrentFavIconSize()
{
   return iCurrentFavIconSize;
}

void
CMapFeatureHolder::SetCurrentFavIconSize(TInt aIcon)
{
   iCurrentFavIconSize = aIcon;
}

void
CMapFeatureHolder::RescalePolygons()
{
   TInt dpiCorrectionFactor = WFLayoutUtils::CalculateDpiCorrectionFactor();
   vector<MC2Point> points;
   if(iPositionPoly) {
      InitPoints(positionPoly, POSITION_POLY_COUNT, points);
      iPositionPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iPositionPoly->rescalePoints(points, iPositionPolyScale);
   }
   if(iDarkShadePoly) {
      InitPoints(darkShadePoly, DARK_SHADE_POLY_COUNT, points);
      iDarkShadePoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iDarkShadePoly->rescalePoints(points, iPositionPolyScale);
   }
   if(iLightshadePoly) {
      InitPoints(lightShadePoly, LIGHT_SHADE_POLY_COUNT, points);
      iLightshadePoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iLightshadePoly->rescalePoints(points, iPositionPolyScale);
   }
   if(iPedPositionPoly) {
      InitPoints(pedPositionPoly, PED_POSITION_POLY_COUNT, points);
      iPedPositionPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iPedPositionPoly->rescalePoints(points, iPedestrianPositionPolyScale);
   }
   if(iPedPositionFillPoly) {
      InitPoints(pedPositionFillPoly, PED_POSITION_FILL_POLY_COUNT, points);
      iPedPositionFillPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iPedPositionFillPoly->rescalePoints(points, iPedestrianPositionPolyScale);
   }
   if(iArrowPoly) {
      InitPoints(arrowPoly, 4, points);
      iArrowPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iArrowPoly->rescalePoints(points);
   } 
   if(iNorthPoly) {
      InitPoints(northPoly, 3, points);
      iNorthPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iNorthPoly->rescalePoints(points);
   }
   if(iSouthPoly) {
      InitPoints(southPoly, 3, points);
      iSouthPoly->setDpiCorrectionFactor(dpiCorrectionFactor);
      iSouthPoly->rescalePoints(points);
   }
}

void CMapFeatureHolder::SetPositionPolyScale(TReal aPositionPolyScale)
{
   if(iPositionPolyScale != aPositionPolyScale) {
      iPositionPolyScale = aPositionPolyScale;
      RescalePolygons();
   }
}
void CMapFeatureHolder::SetPedestrianPositionPolyScale(TReal aPolyScale)
{
   if(iPedestrianPositionPolyScale != aPolyScale) {
      iPedestrianPositionPolyScale = aPolyScale;
      RescalePolygons();
   }
}
