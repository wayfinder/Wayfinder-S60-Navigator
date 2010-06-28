/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Quality.h"
#include "AudioCtrlScript.h"
#include "RouteEnums.h"
#include "AudioClipsEnum.h"
#include "Log.h"
#define LOGPOINTER if(m_log)m_log
#include "LogMacros.h"

/* ******************
 
   Scripted audio syntax. Loads files prepared by the SyntaxEdit 
   java application.

********************* */

namespace isab{
   using namespace RouteEnums;
   using namespace AudioClipsEnum;

   AudioCtrlLanguageScript::AudioCtrlLanguageScript() : 
      AudioCtrlLanguage(), m_lastDistanceIndex(-1)
   {
      // FIXME - XXX - Only for debugging
      m_log = NULL;

      m_file = NULL;
      m_numSounds = 0;
      m_clipFileNames = NULL;
      m_clipFileNameBuffer = NULL;
      m_macros = NULL;
      // Metric is the safe bet - all languages support it
      InitDistanceTableMetric();
      m_soundListNormalMacro = -1;
      m_crossingListMacro = -1;
      m_newCrossingMacro = -1;
      m_cameraMacro = -1;
      m_atDestMacro = -1;
      m_offTrackMacro = -1;
      m_wrongWayMacro = -1;
      m_gpsChangeMacro = -1;
      m_positiveMacro = -1;
      m_negativeMacro = -1;
      m_verbosity = VerbosityNormal;
   }

   AudioCtrlLanguageScript::~AudioCtrlLanguageScript()
   {
      delete[] m_clipFileNames;
      delete[] m_clipFileNameBuffer;
      distanceTable.clear();
      delete m_log;
   }

   class AudioCtrlLanguage* AudioCtrlLanguageScript::New()
   { 
      AudioCtrlLanguageScript * a = new AudioCtrlLanguageScript(); 
      return a; 
   }

   int AudioCtrlLanguageScript::supportsFeetMiles()
   {
      return 1;
   }

   int AudioCtrlLanguageScript::supportsYardsMiles()
   {
      return 1;
   }



/********** Start of temporary code ***************/
/* borrowed from AudioCtrlStd */

#define DPB distanceTable.push_back
   void AudioCtrlLanguageScript::InitDistanceTableMetric()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(2000,  DistanceInfo(        2000,                 1700,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1000,  DistanceInfo(        1000,                  800,                 2050 ),  9999     ));
      DPB(DistanceTableEntry( 500,  DistanceInfo(         500,                  450,                 1050 ),  9999     ));
      DPB(DistanceTableEntry( 200,  DistanceInfo(         200,                  170,                  550 ),  9999     ));
      DPB(DistanceTableEntry( 100,  DistanceInfo(         100,                   60,                  210 ),    16     ));
      DPB(DistanceTableEntry(  50,  DistanceInfo(          55,                   35,                  110 ),     5     ));
      DPB(DistanceTableEntry(  25,  DistanceInfo(          30,                   20,                   60 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   30 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   50 ),  9999     )); /* Never spoken! */
      m_currentUnits = MetricUnits;
   }

   void AudioCtrlLanguageScript::InitDistanceTableFeetMiles()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(3219,  DistanceInfo(        3219,                 2919,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1609,  DistanceInfo(        1609,                 1409,                 3269 ),  9999     ));
      DPB(DistanceTableEntry( 803,  DistanceInfo(         803,                  753,                 1659 ),  9999     ));
      DPB(DistanceTableEntry( 402,  DistanceInfo(         402,                  352,                  853 ),  9999     ));
      DPB(DistanceTableEntry( 152,  DistanceInfo(         152,                  122,                  452 ),    16     ));
      DPB(DistanceTableEntry(  61,  DistanceInfo(          66,                   46,                  162 ),     5     ));
      DPB(DistanceTableEntry(  31,  DistanceInfo(          36,                   26,                   71 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   36 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   61 ),  9999     )); /* Never spoken! */
      m_currentUnits = ImperialFeetMilesUnits;
   }

   void AudioCtrlLanguageScript::InitDistanceTableYardsMiles()
   {
      distanceTable.clear();
      /*           spokenDistance,              sayAtDistance, abortTooShortDistance, abortTooFarDistance, ignoreIfFaster       */
      DPB(DistanceTableEntry(  -1,  DistanceInfo(  NoDistance,                   -1,                   -1 ),  9999     ));  /* priosound */
      DPB(DistanceTableEntry(3219,  DistanceInfo(        3219,                 2919,            MAX_INT32 ),  9999     ));
      DPB(DistanceTableEntry(1609,  DistanceInfo(        1609,                 1409,                 3269 ),  9999     ));
      DPB(DistanceTableEntry( 803,  DistanceInfo(         803,                  753,                 1659 ),  9999     ));
      DPB(DistanceTableEntry( 402,  DistanceInfo(         402,                  352,                  853 ),  9999     ));
      DPB(DistanceTableEntry( 182,  DistanceInfo(         182,                  152,                  452 ),  9999     ));
      DPB(DistanceTableEntry(  91,  DistanceInfo(          91,                   51,                  192 ),    16     ));
      DPB(DistanceTableEntry(  46,  DistanceInfo(          51,                   31,                  101 ),     5     ));
      DPB(DistanceTableEntry(  23,  DistanceInfo(          28,                   18,                   56 ),     2     ));
      DPB(DistanceTableEntry(   0,  DistanceInfo(          18,                   -1,                   34 ),  9999     ));
      DPB(DistanceTableEntry(  -1,  DistanceInfo(    NeverSay,                   -1,                   56 ),  9999     )); /* Never spoken! */
      m_currentUnits = ImperialYardsMilesUnits;
   }
#undef DPB

   void AudioCtrlLanguageScript::selectUnits(enum SpokenUnits units)
   {
      if (units == m_currentUnits) {
         return;
      }
      switch (units) {
         case MetricUnits:
            InitDistanceTableMetric();
            resetState();
            break;
         case ImperialFeetMilesUnits:
            if (supportsFeetMiles()) {
               InitDistanceTableFeetMiles();
               resetState();
            } else {
               // This call must not fail!!!
               selectUnits(MetricUnits);
            }
            break;
         case ImperialYardsMilesUnits:
            if (supportsYardsMiles()) {
               InitDistanceTableYardsMiles();
               resetState();
            } else {
               // This call must not fail!!!
               selectUnits(ImperialFeetMilesUnits);
            }
            break;
         default:
            // FIXME - warn here?
            break;
      }
   }

   /* FIXME - r.distToWpt är inte bra, använd extrapolerad position! */
   int32 AudioCtrlLanguageScript::policyNextDistance(const RouteInfo &r, DistanceInfo &deadlines)
   {
      int32 spokenDistance;

      if (r.onTrackStatus != OnTrack) {
         // WARN("policyNextDistance called with onTrackStatus != OnTrack");
         deadlines = distanceTable[0].info;
         m_lastDistanceIndex = 0;
         spokenDistance = 0;  // Placeholder, not used
         return spokenDistance;
      }
      int lastSound;
      if (m_lastDistanceIndex < 0) {
         lastSound = 0;
      } else {
         lastSound = m_lastDistanceIndex;
      }

      int nextSound = lastSound + 1;

      /* Accept any sound within the interval sayAtDistance..abortTooShortDistance
         if this is the first sound after a priority sound. We want to say something
         as soon as possible.
         This is done by switching from sayAtDistance to abortTooShortDistance
         as a criteria.
      */
      if (lastSound == 0) {
         /* Find the first possible distance. This is the next sound to play,
          * were it not for the user override settings. Those settings are 
          * taken into account in the next while loop. */
         while (distanceTable[nextSound].info.abortTooShortDistance > r.distToWpt ) {
            ++nextSound;
         }
      } else {
         /* Find the first possible distance. This is the next sound to play,
          * were it not for the user override settings. Those settings are 
          * taken into account in the next while loop. */
         while (distanceTable[nextSound].info.sayAtDistance > r.distToWpt ) {
            ++nextSound;
         }
      }
#if 1
      int tooFarDeadline = distanceTable[nextSound].info.abortTooFarDistance;
      /* Find the first allowed sound, and carry the abortTooFarDistance from the
       * first match above. */
      // This loop terminates since the last elements in the distance tables 
      // cannot be prohibited by the user.
      while (1) {
         bool prohibited = false;
 
         // Supress some extra distances if verbosity is set to "less"
         if (m_verbosity == VerbosityReduced) {
            if (distanceTable[nextSound].spokenDistance > 1700) {
               prohibited = true;
            }
            if (lastSound != 0) {
               // If a sound has already been played we skip down to 200m or 100m depending
               // on the current speed.
               // This limits are chosen to coincide with the limits in the distanceTable.
               int distanceLimit;
               if (r.speed >= 16) {
                  distanceLimit = 450;
               } else {
                  distanceLimit = 160;
               }
               if (distanceTable[nextSound].spokenDistance > distanceLimit) {
                  prohibited = true;
               }
            }
         }

         // Supress some distances when driving fast.
         if (r.speed > distanceTable[nextSound].ignoreIfFaster)
            prohibited = true;
         // Supress "Finally" when closer than 100 meters to the goal.
         if ( (RouteAction(r.currCrossing.action) == Finally) &&
              (distanceTable[nextSound].info.sayAtDistance <= 50) )
            prohibited = true;
         // Always use the "no next sound" sound.
         if (distanceTable[nextSound].info.sayAtDistance == NeverSay)
            prohibited = false;
         if (!prohibited) {
            deadlines = distanceTable[nextSound].info;
            deadlines.abortTooFarDistance = tooFarDeadline;
            spokenDistance = distanceTable[nextSound].spokenDistance;
            if (deadlines.sayAtDistance >= 0) {
               m_lastDistanceIndex = nextSound;
            }
            return spokenDistance;
         }
         ++nextSound;
      }
#else
      deadlines = distanceTable[nextSound].info;
      spokenDistance = distanceTable[nextSound].spokenDistance;
      // If this is the special last marker sound, reuse the last m_lastDistanceIndex
      if (deadlines.sayAtDistance >= 0) {
         m_lastDistanceIndex = nextSound;
      }
#endif
      return spokenDistance;

   }

   int32 AudioCtrlLanguageScript::policyAdditionalDistance(int32 distance)
   {
      int nextSound = 1;
      /* Find the first possible distance. This is the next sound to play,
       * were it not for the user override settings. Those settings are 
       * taken into account in the next while loop. */
      if (distance < 0) {
         distance = 0;
      }
      while (distanceTable[nextSound].info.sayAtDistance > distance ) {
         ++nextSound;
      }
      return (distanceTable[nextSound].spokenDistance);
   }


/********** End of temporary code ***************/


   int AudioCtrlLanguageScript::executeSpecialMacro(
         int macro,
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      RunState rs(soundClips);
      rs.xings[0].action = 0; 
      rs.xings[0].spokenDist= 0;  
      rs.xings[0].valid = false;
      rs.xings[0].exitCount = 0;
      rs.xings[0].side = 0;
      rs.xings[0].gpsQ = 0;

      rs.xings[1].action = 0;
      rs.xings[1].spokenDist= 0;  
      rs.xings[1].valid = false;
      rs.xings[1].exitCount = 0;
      rs.xings[1].side = 0;
      rs.xings[1].gpsQ = 0;

      if (macro >= 0) {
         runGraph( macro, rs );
      } else {
         WARN("No special macro\n");
      }
     
      resetState();

      return 0;
   }

   int AudioCtrlLanguageScript::newCrossingSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      if (m_newCrossingMacro >= 0) {
         executeSpecialMacro(m_newCrossingMacro, deadlines, soundClips);
      } else {
         WARN("No NewCrossing macro\n");
      }
      return 0;
   }


   int AudioCtrlLanguageScript::newCameraSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      if (m_cameraMacro >= 0) {
         executeSpecialMacro(m_cameraMacro, deadlines, soundClips);
      } else {
         WARN("No SoundListCamera macro\n");
      }
      return 0;
   }

   int AudioCtrlLanguageScript::badSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      if ( m_negativeMacro >= 0 ) {
         executeSpecialMacro( m_negativeMacro, deadlines, soundClips );
      } else {
         WARN( "No Bad sound macro\n" );
      }
      return 0;
   }

   int AudioCtrlLanguageScript::goodSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundClips)
   {
      if ( m_positiveMacro >= 0 ) {
         executeSpecialMacro( m_positiveMacro, deadlines, soundClips );
      } else {
         WARN( "No Good sound macro\n" );
      }
      return 0;
   }

   int AudioCtrlLanguageScript::statusOfGPSChangedSoundList( 
      const Quality& q, DistanceInfo &deadlines,
      SoundClipsList &soundList )
   {
      if ( m_gpsChangeMacro >= 0 ) {
         deadlines.sayAtDistance = -1;
         deadlines.abortTooShortDistance = -1;
         deadlines.abortTooFarDistance = -1;
         RunState rs( soundList );
         rs.xings[0].action = 0;
         rs.xings[0].exitCount = 0;
         rs.xings[0].spokenDist = 0;
         rs.xings[0].side = 0;
         rs.xings[0].valid = true;
         rs.xings[0].gpsQ = q;
         rs.xings[1].action = 0;
         rs.xings[1].exitCount = 0;
         rs.xings[1].spokenDist = 0;
         rs.xings[1].side = 0;
         rs.xings[1].valid = false;
         rs.xings[1].gpsQ = 0;
         runGraph( m_gpsChangeMacro, rs );
      }
      return 0;
   }
   

   void AudioCtrlLanguageScript::immediateDeadline(DistanceInfo &deadlines)
   {
      deadlines = distanceTable[0].info;
      m_lastDistanceIndex = 0;
   }
   
   void AudioCtrlLanguageScript::resetState()
   {
      m_lastDistanceIndex = -1;
   }



   int AudioCtrlLanguageScript::syntheziseSoundList(const RouteInfo &r,
         bool onTrackStatusChanged, 
         DistanceInfo &deadlines,
         SoundClipsList &soundList)
   {
      // FIXME - XXX - Only for debugging
      if (NULL == m_log) {
         m_log = new Log("AudioScript");
      }

      switch (r.onTrackStatus) {
         case OnTrack:
            { 

               RunState rs(soundList);
               // Extract information from the crossing c
               rs.xings[0].action = r.currCrossing.action;
               rs.xings[0].exitCount = r.currCrossing.exitCount;
               rs.xings[0].valid = r.currCrossing.valid;
               if (rs.xings[0].valid) {
                  rs.xings[0].spokenDist= policyNextDistance(r, deadlines);
               }
               rs.xings[0].side = r.currSeg.isLeftTraffic() ? 0 : 1;
               rs.xings[0].gpsQ = 0;

               rs.xings[1].action = r.nextCrossing1.action;
               rs.xings[1].exitCount = r.nextCrossing1.exitCount;
               rs.xings[1].valid = r.nextCrossing1.valid;
               rs.xings[1].spokenDist= policyAdditionalDistance(r.currCrossing.distToNextCrossing);
               rs.xings[1].side = r.nextSeg1.isLeftTraffic() ? 0 : 1;
               rs.xings[1].gpsQ = 0;

               if (m_verbosity == VerbosityReduced) {
                  rs.xings[1].valid = false;
               }

               if ( ! (rs.xings[0].valid) ) {
                  // No valid first crossing.
                  return 0;
               }

               if (m_soundListNormalMacro >= 0) {
                  runGraph( m_soundListNormalMacro, rs );
               } else {
                  WARN("No SoundListNormal macro\n");
               }
            }
            break;
         case OffTrack:
            if (onTrackStatusChanged) {
               executeSpecialMacro(m_offTrackMacro, deadlines, soundList);
            }
            break;
         case WrongWay:
            if (onTrackStatusChanged) {
               executeSpecialMacro(m_wrongWayMacro, deadlines, soundList);
            }
            break;
         case Goal:
            if (onTrackStatusChanged) {
               executeSpecialMacro(m_atDestMacro, deadlines, soundList);
            }
            break;
      }

      return 0;
   }

   int AudioCtrlLanguageScript::syntheziseCrossingSoundList(const RouteInfoParts::Crossing &c,
         DistanceInfo &deadlines,
         SoundClipsList &soundList)
   {
      // FIXME - XXX - Only for debugging
      if (NULL == m_log) {
         m_log = new Log("AudioScript");
      }

      deadlines.sayAtDistance = -1;
      deadlines.abortTooShortDistance = -1;
      deadlines.abortTooFarDistance = -1;

      RunState rs(soundList);
      // Extract information from the crossing c
      rs.xings[0].action = c.action;
      rs.xings[0].spokenDist= 100;   // Hack to get the roundabout sounds right.
      rs.xings[0].valid = c.valid;
      rs.xings[0].exitCount = c.exitCount;
      rs.xings[0].side = 0;
      rs.xings[0].gpsQ = 0;

      rs.xings[1].action = 0;        // Should never be selected
      rs.xings[1].spokenDist= 0;  
      rs.xings[1].valid = false;
      rs.xings[1].exitCount = 0;
      rs.xings[1].side = 0;
      rs.xings[1].gpsQ = 0;

      if ( ! (rs.xings[0].valid) ) {
         // No valid first crossing.
         return 0;
      }

      if (m_crossingListMacro >= 0) {
         runGraph( m_crossingListMacro, rs );
      } else {
         WARN("No crossingListMacro\n");
      }

      return 0;
   }

   bool AudioCtrlLanguageScript::gotoNodeInMacro(RunState & rs, int nextNode) {
      DBG("gotoNodeInMacro m=%s, nextNode=%i\n", rs.gs.macro->name, nextNode);
      // Loop until a real node is reached
      while (1) {
         if (nextNode >= 0) {
            // Regular node
            rs.gs.node = &(rs.gs.macro->nodes[nextNode]);
            return true;
         } else {
            // Fake node (i.e. return, fail etc)
            switch (static_cast<KnownNodeIds>(nextNode)) {
               case Fail:
                  {
                     while (1) {
                        if (rs.callStack.empty()) {
                           // We have failed out of the start macro. No
                           // audio will be played.
                           rs.soundList.erase(rs.soundList.begin(), rs.soundList.end());
                           return false;
                        }
                        RunState::CallStackEntry cse = rs.callStack.top();
                        rs.callStack.pop();
                        if (cse.type == RunState::CallStackEntry::Try) {
                           rs.gs = cse.oldGraphState;
                           rs.soundList.erase(rs.soundList.begin()+cse.clipListSize, rs.soundList.end());
                           nextNode = rs.gs.node->falseId;
                           break;
                        }
                     }
                  }
                  break;
               case None:
                  // Fall through
               case Return:
                  // Fall through
               default:
                  // Internal error - what to do? Treat as return for now
                  {
                     while (1) {
                        if (rs.callStack.empty()) {
                           // Return off of the start macro. We are done.
                           return false;
                        }
                        RunState::CallStackEntry cse = rs.callStack.top();
                        rs.callStack.pop();
                        if (cse.type == RunState::CallStackEntry::Call) {
                           rs.gs = cse.oldGraphState;
                           nextNode = rs.gs.node->nextOrTrueId;
                           break;
                        }
                     }
                  }
                  break;
            }
            DBG("  once more m=%s, nextNode=%i\n", rs.gs.macro->name, nextNode);
         }
      }
   }

   bool AudioCtrlLanguageScript::moveToNextNode(RunState & rs) {
      return gotoNodeInMacro(rs, rs.gs.node->nextOrTrueId);
   }

   bool AudioCtrlLanguageScript::evalCondition(const RunState & rs, const Node & node) {
      int16 varVal;
      DBG("evalCondition %i %i %i\n", node.variable, node.relation, node.limit);
      switch (static_cast<Node::Variable>(node.variable)) {
         case Node::VarDist:
            varVal = rs.xings[rs.gs.xing].spokenDist;
            break;
         case Node::VarTurn:
            varVal = rs.xings[rs.gs.xing].action;
            break;
         case Node::VarXing:
            varVal = rs.gs.xing+1;  // One-based test
            break;
         case Node::VarExit:
            varVal = rs.xings[rs.gs.xing].exitCount;
            break;
         case Node::VarSide:
            varVal = rs.xings[rs.gs.xing].side;
            break;
         case Node::VarGpsQ:
            varVal = rs.xings[rs.gs.xing].gpsQ;
            break;
         case Node::VarZero:
         default:
            varVal = 0;
            break;
      }
      DBG("  val=%i\n", varVal);
      int16 limit = node.limit;
      switch (static_cast<Node::Relation>(node.relation)) {
         case Node::Equal:
            return varVal == limit;
         case Node::NotEqual:
            return varVal != limit;
         case Node::LessThan:
            return varVal < limit;
         case Node::GreaterThan:
            return varVal > limit;
         case Node::LessThanOrEqual:
            return varVal <= limit;
         case Node::GreaterThanOrEqual:
            return varVal >= limit;
         default:
            return false;
      }
   }

   void AudioCtrlLanguageScript::runGraph( int startMacro, RunState & rs) {
      rs.gs.macro = &(m_macros[startMacro]);
      rs.gs.node = &(rs.gs.macro->nodes[rs.gs.macro->startnode]);
      // Assume the first crossing on entry
      rs.gs.xing = 0;

      while (1) {
         //Macro & macro = *rs.gs.macro;
         DBG("Node addr %p\n", rs.gs.node);
         Node & node = *rs.gs.node;
         switch (static_cast<Node::Type>(node.type)) {
            case Node::Clip:
               DBG("Got clip %i\n", node.clipOrMacroId);
               rs.soundList.push_back(node.clipOrMacroId);
               DBG("  clip2\n");
               if ( !  moveToNextNode(rs)) {
                  return;
               }
               DBG("  clip3\n");
               break;
            case Node::Macro:
               {
                  RunState::CallStackEntry cse;
                  cse.type=RunState::CallStackEntry::Call;
                  cse.oldGraphState=rs.gs;
                  rs.callStack.push(cse);
                  rs.gs.macro=&(m_macros[node.clipOrMacroId]);
                  DBG("Got Macro call to %s\n", rs.gs.macro->name);
                  if ( ! gotoNodeInMacro(rs, rs.gs.macro->startnode) ) {
                     return;
                  }
               }
               break;
            case Node::Boolean:
               {
                  DBG("Got boolean\n");
                  int nodeId;
                  if (evalCondition(rs, node)) {
                     nodeId=rs.gs.node->nextOrTrueId;
                  } else {
                     nodeId=rs.gs.node->falseId;
                  }
                  if ( ! gotoNodeInMacro(rs, nodeId)) {
                     return;
                  }
               }
               break;
            case Node::Try:
               {
                  DBG("Got try\n");
                  RunState::CallStackEntry cse;
                  cse.type=RunState::CallStackEntry::Try;
                  cse.oldGraphState=rs.gs;
                  cse.clipListSize = rs.soundList.size();
                  rs.callStack.push(cse);
                  if ( !  moveToNextNode(rs)) {
                     return;
                  }
               }
               break;
            case Node::Xing:
               DBG("Got xing\n");
               rs.gs.xing = node.xing-1;   // Zero-based index
               if ( ! (rs.xings[rs.gs.xing].valid) ) {
                  // No such crossing - fail
                  if ( ! gotoNodeInMacro(rs, static_cast<int>(Fail))) {
                     return;
                  }
               }
               if ( !  moveToNextNode(rs)) {
                  return;
               }
               break;
            default:
               return;
         }
      } /* while */
   }



   /************ File reader *****************/
   int AudioCtrlLanguageScript::readInt8(int8 & i) {
      int num_read = fread(&i, 1, 1, m_file);
      return (num_read!=1);
   }

   int AudioCtrlLanguageScript::readInt16(int16 & i) {
      uint8 low, high;
      if (1 != fread(&low, 1, 1, m_file)) {
         return 1;
      }
      if (1 != fread(&high, 1, 1, m_file)) {
         return 1;
      }
      i = static_cast<int16>((( static_cast<uint16>(high) ) << 8) | low);
      return 0;
   }

   int AudioCtrlLanguageScript::skipString() {
      int8 length;
      if ( readInt8(length)) {
         return 1;
      }
      fseek(m_file, length, SEEK_CUR);
      return 0;
   }

   int AudioCtrlLanguageScript::readString(char * str) {
      int8 length;
      if ( readInt8(length)) {
         return 1;
      }
      if ( 1 != fread(str, length, 1, m_file)) {
         return 1;
      }
      str[length]=0;
      return 0;
   }

   int AudioCtrlLanguageScript::readAllocString(char * & str) {
      int8 length;
      if ( readInt8(length)) {
         return 1;
      }
      str = new char[length+1];
      if (NULL==str) {
         return 1;
      }
      if ( 1 != fread(str, length, 1, m_file)) {
         delete[] str;
         str = NULL;
         return 1;
      }
      str[length]=0;
      return 0;
   }

   bool AudioCtrlLanguageScript::isInvalidNodeId(int16 id) {
      if ( (id < -3) || (id >= (int)(m_curM->nodes.size())) ) { 
         return true; 
      }
      return false;
   }

   bool AudioCtrlLanguageScript::isInvalidClipId(int16 id) {
      // Allow the timing marker.
      if ( id == -3) {
         return false;
      }
      if ( (id < 0) || (id >= m_numSounds) ) { 
         return true; 
      }
      return false;
   }

   bool AudioCtrlLanguageScript::isInvalidMacroId(int16 id) {
      if ( (id < 0) || (id >= m_readMacros) ) { 
         return true; 
      }
      return false;
   }

   int AudioCtrlLanguageScript::readStartNode() {
      int16 id;
      if (readInt16(id) || isInvalidNodeId(id)) { 
         return 1; 
      }
      m_curM->startnode = id;
      return 0;
   }

   int AudioCtrlLanguageScript::readSoundClipNode() {
      Node n;
      if (readInt16(n.nextOrTrueId) || isInvalidNodeId(n.nextOrTrueId)) { 
         return 1; 
      }
      
      if (readInt16(n.clipOrMacroId) || isInvalidClipId(n.clipOrMacroId)) { 
         return 1; 
      }
      n.type = Node::Clip;
      m_curM->nodes.push_back(n);
      return 0;
   }

   int AudioCtrlLanguageScript::readMacroCallNode() {
      Node n;
      if (readInt16(n.nextOrTrueId) || isInvalidNodeId(n.nextOrTrueId)) { 
         return 1; 
      }
      if (readInt16(n.clipOrMacroId) || isInvalidMacroId(n.clipOrMacroId)) { 
         return 1; 
      }
      n.type = Node::Macro;
      m_curM->nodes.push_back(n);
      return 0;
   }

   int AudioCtrlLanguageScript::readBooleanNode() {
      Node n;
      if (readInt8(n.variable)) {
         return 1;
      }
      if (readInt8(n.relation)) {
         return 1;
      }
      if (readInt16(n.limit)) {
         return 1;
      }
      DBG("Read boolean node %i %i %i\n", n.variable, n.relation, n.limit);
      if (readInt16(n.nextOrTrueId) || isInvalidNodeId(n.nextOrTrueId)) { 
         return 1; 
      }
      if (readInt16(n.falseId) || isInvalidNodeId(n.falseId)) { 
         return 1; 
      }
      n.type = Node::Boolean;
      m_curM->nodes.push_back(n);
      return 0;
   }

   int AudioCtrlLanguageScript::readTryCatchNode() {
      Node n;
      if (readInt16(n.nextOrTrueId) || isInvalidNodeId(n.nextOrTrueId)) { 
         return 1; 
      }
      if (readInt16(n.falseId) || isInvalidNodeId(n.falseId)) { 
         return 1; 
      }
      n.type = Node::Try;
      m_curM->nodes.push_back(n);
      return 0;
   }

   int AudioCtrlLanguageScript::readSelectXingNode() {
      Node n;
      if (readInt16(n.nextOrTrueId) || isInvalidNodeId(n.nextOrTrueId)) { 
         return 1; 
      }
      if (readInt8(n.xing)) { 
         return 1; 
      }
      n.type = Node::Xing;
      m_curM->nodes.push_back(n);
      return 0;
   }


   int AudioCtrlLanguageScript::selectSyntaxVersion(const char *fileName, 
                                                    int & numClips, 
                                                    char ** & clipFileNames)
   {
      // FIXME - XXX - Only for debugging
      if (NULL == m_log) {
         m_log = new Log("AudioScript");
      }

      DBG("selectSyntaxVersion %s", fileName);

      static const char * const expectedHeader = "WF_AUDIO_SYNTAX: 1\n";

      m_file = fopen(fileName, "rb");
      if (m_file == NULL) {
         // File not found?
         return 1;
      }

      char header[19];
      if (1 != fread(header,19,1,m_file)) {
         // Failed to read header
         goto out;
      }
      if (0 != memcmp(header, expectedHeader, strlen(expectedHeader))) {
         // Wrong file format
         goto out;
      }

      // Read out all options
      // Ignore all options for now
      int8 option;
      do {
         if (readInt8(option)) {
            // Read past end of file
            goto out;
         }
      } while (option != 0x00);

      // Read the sound clips
      // Ignore the name, only the filename is important
      if (readInt16(m_numSounds)) { goto out; }
      numClips = m_numSounds;
      {
         clipFileNames = new char*[m_numSounds];
         for (int soundNo = 0; soundNo<m_numSounds; ++soundNo) {
            // name - ignore
            if (skipString()) { goto out; }
            if (readAllocString(clipFileNames[soundNo])) { goto out; }
         }
      }

      // Read the distrance table
      // Not implemented yet.

      // Read the macros
      m_soundListNormalMacro = -1;
      m_crossingListMacro = -1;
      m_newCrossingMacro = -1;
      m_cameraMacro = -1;
      m_atDestMacro = -1;
      m_wrongWayMacro = -1;
      m_offTrackMacro = -1;
      m_gpsChangeMacro = -1;
      m_positiveMacro = -1;
      m_negativeMacro = -1;
      int16 nummacros;
      if (readInt16(nummacros)) { goto out; }
      if (m_macros) {
         delete[] m_macros;
      }
      m_macros = new Macro[nummacros];
      for (m_readMacros = 0; m_readMacros<nummacros; ++m_readMacros) {
         m_curM = m_macros+m_readMacros;
         char tmpName[50];
         if (readString(tmpName)) { goto out; }
         if (0 == strcmp(tmpName, "SoundListNormal")) {
            m_soundListNormalMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListXing")) {
            m_crossingListMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "NewCrossing")) {
            m_newCrossingMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListSpeedCam")) {
            m_cameraMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListAtDest")) {
            m_atDestMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListOffTrack")) {
            m_offTrackMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListWrongWay")) {
            m_wrongWayMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "SoundListGpsChange")) {
            m_gpsChangeMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "Positive")) {
            m_positiveMacro = m_readMacros;
         } else if (0 == strcmp(tmpName, "Negative")) {
            m_negativeMacro = m_readMacros;
         }
         // Store the macro name - debugging only
         strcpy(m_curM->name, tmpName);

         // Read the graph
         int8 nodeType;
         do {
            if (readInt8(nodeType)) { goto out; }
            switch (nodeType) {
               case 0:
                  // Start node
                  if (readStartNode()) { goto out; }
                  break;
               case 1:
                  // Play sound node
                  if (readSoundClipNode()) { goto out; }
                  break;
               case 2:
                  // Macro call node
                  if (readMacroCallNode()) { goto out; }
                  break;
               case 3:
                  // Boolean condition node
                  if (readBooleanNode()) { goto out; }
                  break;
               case 4:
                  // Try/catch node
                  if (readTryCatchNode()) { goto out; }
                  break;
               case 5:
                  // Select xing node
                  if (readSelectXingNode()) { goto out; }
                  break;
               default:
                  goto out;
            }
         } while (nodeType != 0x00);

      }


      fclose(m_file);
      m_file = NULL;

      return 0;

out:
      fclose(m_file);
      m_file = NULL;
      return 1;
   }


   int AudioCtrlLanguageScript::setVerbosity(AudioVerbosity verbosity)
   {
      m_verbosity = verbosity;
      resetState();
      return 0;
   }


} /* namespace isab */


