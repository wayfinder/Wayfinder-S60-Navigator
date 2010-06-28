/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef REQ_FAILED_STRINGS_H
#define REQ_FAILED_STRINGS_H

#include "machine.h"
#include "GuiProt/GuiProtEnums.h"
#include "Nav2Error.h"

namespace isab {

namespace GuiRequestFailed{



   /**
    * Struct which maps requests and failed request
    * error strings together.
    */
   typedef struct { GuiProtEnums::MessageType request;
                    ::isab::Nav2Error::ErrorNbr errorNum;
   } nav2FailedGuiRequestElement;

   /**
    * Strings telling what happend when different requests fails.
    *
    * Used with potocol version 3.
    *
    * NB! This vector must have at least 2 elements in order to be
    *     able to determine its length.
    */
   const nav2FailedGuiRequestElement nav2FailedGuiRequestVector[] = {
      {GuiProtEnums::GET_TOP_REGION_LIST,      Nav2Error::GUIPROT_FAILED_GET_TOP_REGION_LIST         },
      {GuiProtEnums::GET_FAVORITES,            Nav2Error::GUIPROT_FAILED_GET_FAVORITES               },
      {GuiProtEnums::GET_FAVORITES_ALL_DATA,   Nav2Error::GUIPROT_FAILED_GET_FAVORITES_ALL_DATA      },
      {GuiProtEnums::SORT_FAVORITES,           Nav2Error::GUIPROT_FAILED_SORT_FAVORITES              },
      {GuiProtEnums::SYNC_FAVORITES,           Nav2Error::GUIPROT_FAILED_SYNC_FAVORITES              },
      {GuiProtEnums::GET_FAVORITE_INFO,        Nav2Error::GUIPROT_FAILED_GET_FAVORITE_INFO           },
      {GuiProtEnums::ADD_FAVORITE,             Nav2Error::GUIPROT_FAILED_ADD_FAVORITE                },
      {GuiProtEnums::ADD_FAVORITE_FROM_SEARCH, Nav2Error::GUIPROT_FAILED_ADD_FAVORITE_FROM_SEARCH    },
      {GuiProtEnums::REMOVE_FAVORITE,          Nav2Error::GUIPROT_FAILED_REMOVE_FAVORITE             },
      {GuiProtEnums::CHANGE_FAVORITE,          Nav2Error::GUIPROT_FAILED_CHANGE_FAVORITE             },
      {GuiProtEnums::ROUTE_TO_FAVORITE,        Nav2Error::GUIPROT_FAILED_ROUTE_TO_FAVORITE           },
      {GuiProtEnums::ROUTE_TO_HOT_DEST,        Nav2Error::GUIPROT_FAILED_ROUTE_TO_HOT_DEST           },
      {GuiProtEnums::DISCONNECT_GPS,           Nav2Error::GUIPROT_FAILED_DISCONNECT_GPS              },
      {GuiProtEnums::CONNECT_GPS,              Nav2Error::GUIPROT_FAILED_CONNECT_GPS                 },
   };

   /**
    * Get the string telling what happened when a specific request
    * failed.
    *
    * Used with potocol version 3, i.e. the Nav2 protocol.
    *
    * @param request The message type of the request that failed.
    */
   const char* getFailedRequestString(GuiProtEnums::MessageType request,
         Nav2Error::Nav2ErrorTable * errorTable);

} // Nav2Error

} // isab






#endif // REQ_FAILED_STRINGS_H


