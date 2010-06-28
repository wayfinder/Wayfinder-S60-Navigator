/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef MAPENUMS_H
#define MAPENUMS_H
namespace isab{
   namespace MapEnums{
      enum MapSubject{
         /// Draw a map that contains the entire route.
         OverviewRoute       = -1,
         /// Draw the part of the route that fits in the map image.  
         Route               = 0,
         /// No extra information included in map.
         None                = 1,     
         /// Draw position as user position
         UserPosition        = 2,
         /// Draw position as search item position
         SearchPosition      = 3,
         /// Draw posision as favorite position
         FavoritePosition    = 4,
         /// Draw position as destination position
         DestinationPosition = 5,
         /// Calculate bounding box from speed, heading, and position. 
         /// Otherwise as Route.
         Tracking            = 6,
         /// Draw position without any marker for the position.
         // Use bounding box as for DestinationPosition.
         UnmarkedPosition    = 7,
      };

      /** Used to specify image file format.
       *
       * NB! The first values must harmonize with the imageFromat enum in 
       *     mc2/Server/ServerShared/include/ImageDrawConfig.h
       */
      enum ImageFormat {
         /// PNG Portable Network Graphics
         PNG_OLD_IS_GIF_NOW = 0,
         /// Wireless BitMaP
         WBMP = 1,
         /// Joint Pictures Expert Group
         JPEG = 2,
         /// Graphics Interchange Format (You MUST pay royalties to unisys)
         GIF = 3,
         //end of harmonization with ImageDrawConfig.h.
         PNG = 4,
         
         /// Windows bitmaps used in for instance CBitmaps in Windows CE.
         WIN_CE_BMP = 5,
         
         NBR_OF_IMAGE_TYPES,  //Keep this one next last.
         INVALID_IMAGE_TYPE   //Keep this one last.
      };

      enum MapItemType {
         InvalidMapItemType  = 0,
         MapRouteItem        = 1, 
         MapSearchItem       = 2,
         MapDestinationItem  = 3,
         MapUserPositionItem = 4,
      };
      
      enum MapInfoType{
         InvalidMapInfoType = 0,
         Category           = 1, //payload determines what category
         TrafficInformation = 2, //payload determines what info
         Ruler              = 3, //payload doesn't matter
         Topographic        = 4, //what kind of topographic info.
         MapFormat          = 5, //palette and similar info.
         Rotate             = 6, //rotation angle, 256 parts, 0 north.
      };


   }
}
#endif
