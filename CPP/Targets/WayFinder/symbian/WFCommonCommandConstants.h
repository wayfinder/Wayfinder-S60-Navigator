/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//Enumeration of custom messages sent to views being activated.
//Uses strange number since stupid uiq3 sometimes 
//sends 1 as message id which interferes with our messages.
enum TCustomMessageIds {
   ERangeHolderStart =         60,
   //Used on all platforms
   ENoMessage =                61,
   EKeepOldStateMessage =      62,
   EBackOnExit =               63,
   EExitOnExit =               64,
   EBackHistoryThenView =      65,
   EBackHistory =              66,
   EShutdown =                 67,
   ERoutePlannerSetAsOrigin =  68, 
   ERoutePlannerSetAsDest =    69,
   EMapAsOverview =            70,
   EMapAsNavigation =          71,
   //Used on uiq only
   ESearchDetailFavoriteInfo = 72,
   //Used on s60 only
   ESearchReply =              73,
   ENewSearch =                74,
   EFromNewDest =              75,
   EFromConnect =              76,
   EShowUpgrade =              77,
   EShowNews =                 78,
   EFullScreen =               79,
   EToggleRingCycle =          80,
   EShowOnMap =                81,
   EAddFavorite =              82,
   EEditFavorite =             83,
   EShowMapFromItineraryView = 84,
   ERangeHolderEnd =           85
};

inline TBool IsWFCustomCommandId(TInt aCommand)
{
   if (aCommand >= ERangeHolderStart && 
       aCommand <= ERangeHolderEnd) {
      return ETrue;
   } else {
      return EFalse;
   }
}
