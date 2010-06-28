/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef GUI_PROT_SERVER_ENUMS_H
#define GUI_PROT_SERVER_ENUMS_H

namespace isab {

   namespace GuiProtEnums {

      enum WayfinderType {
         InvalidWayfinderType = -1,
         Trial  = 0,
         Silver = 1,
         Gold   = 2,
         Iron   = 3,
      };

      enum UserMessageType {
         invalidMessageType = 0,
         HTML_email     = 1,
         non_HTML_email = 2,
         SMS            = 3,
         MMS            = 4,
         FAX            = 6,
         InstantMessage = 5,
       };

      /// The type of object to send. Select one from the
      /// ObjectType enum.
       enum ObjectType{
         invalidObjectType     = 0,
         DestinationMessage    = 1,
         SearchItemMessage     = 2,
         FullSearchItemMessage = 3,
         RouteMessage          = 4,
         ItineraryMessage      = 5,
         MapMessage            = 6,
         PositionMessage       = 7,
       };

       enum PositionType {
          PositionTypeInvalid     = 0,
          PositionTypeSearch      = 1,
          PositionTypeFavorite    = 2,
          PositionTypePosition    = 3,
          PositionTypeHotDest     = 4,
          PositionTypeCurrentPos  = 5,
       };

      ///Used to signal what the server is doing in the
      ///PROGRESS_INDICATOR messages.
      enum ServerActionType {
         InvalidActionType,
         DownloadingMap,  // R_WAYFINDER_CREATING_MAP_MSG
         CreatingRoute,   // R_WAYFINDER_ROUTING_MSG
         PerformingSearch,// R_WAYFINDER_SEARCHING_MSG
         Synchronizing,   // R_WAYFINDER_SYNCH_MSG
         Upgrading,       // R_WAYFINDER_PROCESSING_MSG
         SettingPasswd,   // R_WAYFINDER_PROCESSING_MSG
         RetrievingInfo,  // R_WAYFINDER_PROCESSING_MSG
         CellInfoReport,  // R_WAYFINDER_PROCESSING_MSG
         VectorMap,       // nothing
      };

      ///Used to find different kinds of expire limits in the XXX
      ///parameter vector.
      enum ExpireVectorIndex {
         ///Index to the part of the expire vector that holds the days
         ///left until the users region access expires.
         expireDay = 0,
         ///Index to the part of the expire vector that holds the
         ///number of transactions left on the users account.
         transactionsLeft,
         ///Index to the part of the expire vector that holds the
         ///number of transaction days left. A transaction days is
         ///defined as the number of nonconsecutive days of use left.
         transactionDaysLeft,
         ///The size of the vector. This name should always be the
         ///last in the enums.
         EXPIRE_VECTOR_LENGTH
      };


      /// The level of client tracking.
      enum TrackingLevel_t {
         tracking_level_none      = 0,
         tracking_level_minimal   = 1,
         tracking_level_log       = 2,
         tracking_level_log_often = 3,
         tracking_level_often     = 4,
         tracking_level_live      = 5,
      };


      /**
       * The different user rights.
       */
      enum userRightService {
         UR_MYWAYFINDER                     = 0x4,
         UR_TRAFFIC                         = 0x80,
         UR_SPEEDCAM                        = 0x100,
         UR_POSITIONING                     = 0x2000,
         UR_FLEET                           = 0x10000,

         UR_USE_GPS                         = 0x5,
         UR_ROUTE                           = 0x6,
         UR_SEARCH                          = 0x7,
         UR_POI                             = 0x9,
         UR_FINGAL_USE_GPS                  = 0xf,
      };

   }
}

#endif

