/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <map>


class UserDefinedScaleFeature;
class UserDefinedFeature;
class UserDefinedBitMapFeature;
class CMapFeature;

enum map_feature_type {
   map_feature_favorite    = 0,
   map_feature_start       = 1,
   map_feature_stop        = 2,
   map_feature_pin         = 3,
   map_feature_compass     = 4,
   map_feature_gps         = 5,
   map_feature_scale       = 6,
   map_feature_wait        = 7,
   map_feature_cellid_ring = 8,
};

typedef std::map<UserDefinedFeature*, CMapFeature *> my_type;
typedef std::vector<UserDefinedFeature*> udf_vector_type;
typedef std::vector<isab::Favorite*> favorite_vector_type;

/**
 * CMapFeature
 * Holding object for map features.
 */
class CMapFeature
{
public:
   /**
    * Constructor
    * @param aType         Type of map feature
    * @param aId           Id of map feature
    */
   CMapFeature(enum map_feature_type aType, int32 aId)
   { iType = aType; iId = aId; }

   /**
    * Type
    * @return the type
    */
   enum map_feature_type Type() { return iType; }
   /**
    * Id
    * @return the id
    */
   int32 Id() { return iId; }

private:
   int32 iId;
   enum map_feature_type iType;
};

/**
 * Container for all map features
 */
class CMapFeatureHolder : public CBase
{
public:
   /**
    * Constructor
    */
   CMapFeatureHolder();
   /**
    * Destructor
    */
   ~CMapFeatureHolder();
   /**
    * AddFeature
    * Add a new feature to the list.
    * @param udf        Feature
    * @param mf         Holder object for feature
    */
   void AddFeature(UserDefinedFeature *udf, CMapFeature *mf);
   /**
    * MoveFeatureLast
    * Moves the specified feature last in the list.
    * @param udf        Feature to move.
    */
   void MoveFeatureLast(UserDefinedFeature *udf);
   /**
    * RemoveFeature
    * Remove the specified feature from the list.
    * @param udf        Feature to remove
    * @return the removed feature
    */
   CMapFeature* RemoveFeature(UserDefinedFeature *udf);
   /**
    * GetFeature
    * Return the specified feature from the list.
    * @param udf        Feature to return
    * @return the feature
    */
   CMapFeature* GetFeature(UserDefinedFeature *udf);
   /**
    * GetUDFVector
    * Gets the whole vector of features.
    * @return the vector of features.
    */
   udf_vector_type* GetUDFVector();
   /**
    * InitPolygons
    * Initialze the standard polygons, compass, position image etc.
    */
   void InitPolygons();
   /**
    * UpdateFavorites
    * Update the favorite features.
    * @param the favorite info
    * @param if supplied, the name of the image to show.
    * @param if supplied, the name of the small image to show.
    */
   void UpdateFavorites(favorite_vector_type* favVector, 
                        const char* imgName, 
                        const char* imgNameSmall);
/*    void RemoveAllFavorites(); */
/*    void RemoveAllFeatures(); */

   void RescalePolygons();

   /**
    * Sets the scale that will be used for the position polygon.
    * The polygons will be rescaled during this call if needed.
    * @param aPositionPolyScale the scale to use
    */
   void SetPositionPolyScale(TReal aPositionPolyScale);

   /**
    * Sets the scale that will be used for the pedestrian position polygon.
    * The polygons will be rescaled during this call if needed.
    * @param aPositionPolyScale the scale to use
    */
   void SetPedestrianPositionPolyScale(TReal aPolyScale);

private:
   /**
    * RemoveAll
    * Removes all features in vec from the list.
    */
   void RemoveAll(udf_vector_type& vec);
   /**
    * InitPoints
    * Convenience routine, creates a MC2Point vector (polygon)
    * from a int8 array.
    * @param poly       The relative coordinates
    * @param num        Number of relative coordinates
    * @param points     Return value, the polygon
    */
   void InitPoints(const int16* poly, int num, std::vector<MC2Point>& points);
 
public:
   TBool iPositionPolyShown;
   DirectedPolygon* iPositionPoly;
   DirectedPolygon* iDarkShadePoly;
   DirectedPolygon* iLightshadePoly;
   TReal iPositionPolyScale;

   TBool iPedPositionPolyShown;
   DirectedPolygon* iPedPositionPoly;
   DirectedPolygon* iPedPositionFillPoly;
   TReal iPedestrianPositionPolyScale;

   TBool iCompassPolyShown;
   DirectedPolygon* iArrowPoly;
   DirectedPolygon* iNorthPoly;
   DirectedPolygon* iSouthPoly;
   
   class UserDefinedBitMapFeature* iCellIdBitmap;
   
   class UserDefinedBitMapFeature* iPointBitmap;
   TBool iPointBitmapShown;
   class UserDefinedBitMapFeature* iStartBitmap;
   TBool iStartBitmapShown;
   class UserDefinedBitMapFeature* iEndBitmap;
   TBool iEndBitmapShown;
   UserDefinedScaleFeature* iScaleFeature;
   TBool iScaleFeatureShown;
   class UserDefinedBitMapFeature* iWaitSymbol;

   TInt GetCurrentFavIconSize();
   void SetCurrentFavIconSize(TInt aIcon);
private:
   my_type iFeatures;
   udf_vector_type iUserDefFeatures;
   udf_vector_type iFavoriteUserDefFeatures;

   TInt iCurrentFavIconSize;
};

