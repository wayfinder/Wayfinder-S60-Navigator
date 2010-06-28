/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ROUTEDATA_H
#define ROUTEDATA_H

#include <e32base.h>

#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtRouteMess.h"

using namespace isab;

/**
 * CRouteData stores information about a route and can return
 * a RouteMess for request dispatch.
 */
class CRouteData : public CBase {
public:
   /**
    * Constructs a CRouteData object.
    * 
    * @param aOriginType The position type of the origin.
    * @param aOriginId The id of the origin.
    * @param aOriginLatitude The latitude of the origin.
    * @param aOriginLongitude The longitude of the origin.
    * @param aDestinationType The position type of the destination.
    * @param aDestinationid The id of the destination.
    * @param aDestinationLatitude The latitude of the destination.
    * @param aDestinationLongitude The longitude of the destination.
    * @param aDestinationName The name of the destination.
    * @return A new object.
    */
   static CRouteData* NewL(GuiProtEnums::PositionType aOriginType,
                           const char* aOriginId,
                           int32 aOriginLatitude,
                           int32 aOriginLongitude,
                           GuiProtEnums::PositionType aDestinationType,
                           const char* aDestinationId,
                           int32 aDestinationLatitude,
                           int32 aDestinationLongitude,
                           const char* aDestinationName);

   /**
    * Constructs a CRouteData object and leaves it on the cleanup stack.
    * 
    * @param aOriginType The position type of the origin.
    * @param aOriginId The id of the origin.
    * @param aOriginLatitude The latitude of the origin.
    * @param aOriginLongitude The longitude of the origin.
    * @param aDestinationType The position type of the destination.
    * @param aDestinationid The id of the destination.
    * @param aDestinationLatitude The latitude of the destination.
    * @param aDestinationLongitude The longitude of the destination.
    * @param aDestinationName The name of the destination.
    * @return A new object.
    */
   static CRouteData* NewLC(GuiProtEnums::PositionType aOriginType,
                            const char* aOriginId,
                            int32 aOriginLatitude,
                            int32 aOriginLongitude,
                            GuiProtEnums::PositionType aDestinationType,
                            const char* aDestinationId,
                            int32 aDestinationLatitude,
                            int32 aDestinationLongitude,
                            const char* aDestinationName);

   /**
    * Constructs a CRouteData object.
    * 
    * @param aDestinationType The position type of the destination.
    * @param aDestinationid The id of the destination.
    * @param aDestinationLatitude The latitude of the destination.
    * @param aDestinationLongitude The longitude of the destination.
    * @param aDestinationName The name of the destination.
    * @return A new object.
    */
   static CRouteData* NewL(GuiProtEnums::PositionType aDestinationType,
                           const char* aDestinationId,
                           int32 aDestinationLatitude,
                           int32 aDestinationLongitude,
                           const char* aDestinationName);
  
   /**
    * Constructs a CRouteData object and leaves it on the cleanup stack.
    * 
    * @param aDestinationType The position type of the destination.
    * @param aDestinationid The id of the destination.
    * @param aDestinationLatitude The latitude of the destination.
    * @param aDestinationLongitude The longitude of the destination.
    * @param aDestinationName The name of the destination.
    * @return A new object.
    */
   static CRouteData* NewLC(GuiProtEnums::PositionType aDestinationType,
                            const char* aDestinationId,
                            int32 aDestinationLatitude,
                            int32 aDestinationLongitude,
                            const char* aDestinationName);
  
   /**
    * The destructor. The class does not own any heap data.
    */
   ~CRouteData();
  
   /**
    * Creates a RouteMess and returns it. The caller takes ownership.
    * @return A new RouteMess containing the route information stored in
    * the object.
    */
   RouteMess* AsRouteMessL();
  
private:
  
   /**
    * Second phase constructor. Duplicates the strings.
    * @param aOriginId The id of the origin.
    * @param aDestinationId The id of the destination.
    * @param aDestinationname The name of the destination.
    */
   void ConstructL(const char* aOriginId,
                   const char* aDestinationId,
                   const char* aDestinationName);
  
   /**
    * The constructor.
    */
   CRouteData(GuiProtEnums::PositionType aOriginType,
              int32 aOriginLatitude,
              int32 aOriginLongitude,
              GuiProtEnums::PositionType aDestinationType,
              int32 aDestinationLatitude,
              int32 aDestinationLongitude
              );

private:

   /** The origin type. */
   GuiProtEnums::PositionType iOriginType;
   /** The origin latitude. */
   int32 iOriginLatitude;
   /** The origin longitude. */
   int32 iOriginLongitude;
   /** The origin id. */
   char* iOriginId;
   /** The destination type. */
   GuiProtEnums::PositionType iDestinationType;
   /** The destination latitude. */
   int32 iDestinationLatitude;
   /** The destination longitude. */
   int32 iDestinationLongitude;
   /** The destination id. */
   char* iDestinationId;
   /** The destination name. */
   char* iDestinationName;
};

#endif // ROUTEDATA_H
