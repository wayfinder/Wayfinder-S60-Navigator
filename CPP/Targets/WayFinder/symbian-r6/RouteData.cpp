/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "RouteData.h"

#include "WFTextUtil.h"

CRouteData::CRouteData(GuiProtEnums::PositionType aOriginType,
                       int32 aOriginLatitude,
                       int32 aOriginLongitude,
                       GuiProtEnums::PositionType aDestinationType,
                       int32 aDestinationLatitude,
                       int32 aDestinationLongitude
                       ) : iOriginType(aOriginType), 
                           iOriginLatitude(aOriginLatitude),
                           iOriginLongitude(aOriginLongitude),
                           iDestinationType(aDestinationType),
                           iDestinationLatitude(aDestinationLatitude),
                           iDestinationLongitude(aDestinationLongitude)
{
}

CRouteData::~CRouteData()
{
   delete[] iOriginId;
   delete[] iDestinationId;
   delete[] iDestinationName;
}

CRouteData* CRouteData::NewL(GuiProtEnums::PositionType aOriginType,
                             const char* aOriginId,
                             int32 aOriginLatitude,
                             int32 aOriginLongitude,
                             GuiProtEnums::PositionType aDestinationType,
                             const char* aDestinationId,
                             int32 aDestinationLatitude,
                             int32 aDestinationLongitude,
                             const char* aDestinationName)
{
   CRouteData* self = CRouteData::NewLC(aOriginType,
                                        aOriginId,
                                        aOriginLatitude,
                                        aOriginLongitude,
                                        aDestinationType,
                                        aDestinationId,
                                        aDestinationLatitude,
                                        aDestinationLongitude,
                                        aDestinationName);
   CleanupStack::Pop(self);
   return self;
}

CRouteData* CRouteData::NewLC(GuiProtEnums::PositionType aOriginType,
                              const char* aOriginId,
                              int32 aOriginLatitude,
                              int32 aOriginLongitude,
                              GuiProtEnums::PositionType aDestinationType,
                              const char* aDestinationId,
                              int32 aDestinationLatitude,
                              int32 aDestinationLongitude,
                              const char* aDestinationName)
{
   CRouteData* self = new (ELeave) CRouteData(aOriginType,
                                              aOriginLatitude,
                                              aOriginLongitude,
                                              aDestinationType,
                                              aDestinationLatitude,
                                              aDestinationLongitude);
   CleanupStack::PushL(self);
   self->ConstructL(aOriginId, aDestinationId, aDestinationName);
   return self;
}

CRouteData* CRouteData::NewL(GuiProtEnums::PositionType aDestinationType,
                             const char* aDestinationId,
                             int32 aDestinationLatitude,
                             int32 aDestinationLongitude,
                             const char* aDestinationName)
{
   CRouteData* self = CRouteData::NewLC(aDestinationType,
                                        aDestinationId,
                                        aDestinationLatitude,
                                        aDestinationLongitude,
                                        aDestinationName);
   CleanupStack::Pop(self);
   return self;
}

CRouteData* CRouteData::NewLC(GuiProtEnums::PositionType aDestinationType,
                              const char* aDestinationId,
                              int32 aDestinationLatitude,
                              int32 aDestinationLongitude,
                              const char* aDestinationName)
{
   CRouteData* self = new (ELeave) CRouteData(GuiProtEnums::PositionTypeCurrentPos,
                                              MAX_INT32,
                                              MAX_INT32,
                                              aDestinationType,
                                              aDestinationLatitude,
                                              aDestinationLongitude);
   CleanupStack::PushL(self);
   self->ConstructL("", aDestinationId, aDestinationName);
   return self;
}

void CRouteData::ConstructL(const char* aOriginId,
                            const char* aDestinationId,
                            const char* aDestinationName)
{
   iOriginId = WFTextUtil::strdupL(aOriginId);
   iDestinationId = WFTextUtil::strdupL(aDestinationId);
   iDestinationName = WFTextUtil::strdupL(aDestinationName);
}

RouteMess* CRouteData::AsRouteMessL()
{
   RouteMess* message = new (ELeave) RouteMess(iOriginType,
                                               iOriginId,
                                               iOriginLatitude,
                                               iOriginLongitude,
                                               iDestinationType,
                                               iDestinationId,
                                               iDestinationLatitude,
                                               iDestinationLongitude,
                                               iDestinationName);
   return message;
}
