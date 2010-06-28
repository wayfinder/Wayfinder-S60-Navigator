/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AUDIO_CONTROL_Script_H
#define AUDIO_CONTROL_Script_H

#include "AudioCtrlStd.h"
#include <stdio.h>
#include <stack>

namespace isab{

   // FIXME - XXX - debugging only
   class Log;

   class AudioCtrlLanguageScript : public AudioCtrlLanguage
   {  
   public:
      AudioCtrlLanguageScript();
      ~AudioCtrlLanguageScript();
      static class AudioCtrlLanguage* New();

   public:
      virtual int syntheziseSoundList(const RouteInfo &r,
            bool onTrackStatusChanged,
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      virtual int syntheziseCrossingSoundList(const RouteInfoParts::Crossing &c,
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      virtual int newCrossingSoundList(
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      virtual int newCameraSoundList(
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      virtual int statusOfGPSChangedSoundList( 
         const Quality& q, DistanceInfo &deadlines,
         SoundClipsList &soundList);
      virtual int badSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundList);
      virtual int goodSoundList(
         DistanceInfo &deadlines,
         SoundClipsList &soundList);
      virtual void resetState();
      virtual void selectUnits(enum SpokenUnits units);
      virtual int selectSyntaxVersion(const char *fileName, int & numClips, char ** & clipFileNames);
      virtual int setVerbosity(AudioVerbosity verbosity);

      virtual int  supportsFeetMiles();
      virtual int  supportsYardsMiles();

   private:
      virtual int32 policyNextDistance(const RouteInfo &r, DistanceInfo &deadlines);
      virtual int32 policyAdditionalDistance(int32 distance);
      void immediateDeadline(DistanceInfo &deadlines);

   private:
      int readInt8(int8 & val);
      int readInt16(int16 & val);
      int readString(char * str);
      int readAllocString(char * & str);
      int skipString();

      int readStartNode();
      int readSoundClipNode();
      int readMacroCallNode();
      int readBooleanNode();
      int readTryCatchNode();
      int readSelectXingNode();
      bool isInvalidClipId(int16 id);
      bool isInvalidNodeId(int16 id);
      bool isInvalidMacroId(int16 id);

   private:
      struct DistanceTableEntry {
         int32 spokenDistance;
         DistanceInfo info;
         int32 ignoreIfFaster;
         DistanceTableEntry(int32 spk, const DistanceInfo &di, int32 ignspd) : 
            spokenDistance(spk), info(di), ignoreIfFaster(ignspd) { };
      };

      std::vector<DistanceTableEntry> distanceTable;

      // Global state. Given what we spoke the last time we know what is resonable
      // to speak the next time.
      int m_lastDistanceIndex;

      // The currently loaded distance table
      enum SpokenUnits m_currentUnits;

      enum AudioVerbosity m_verbosity;

      // Override if another set of distances is desired.
      virtual void InitDistanceTableMetric();
      virtual void InitDistanceTableFeetMiles();
      virtual void InitDistanceTableYardsMiles();

   private:
      FILE * m_file;

   private:
      enum KnownNodeIds { 
         None = -1,
         Return = -2,
         Fail = -3
      };
      class Node {
         public:
            enum Type { Clip, Macro, Boolean, Try, Xing };
            enum Variable { 
               VarZero=0,
               VarDist=1,
               VarTurn=2,
               VarXing=3,
               VarExit=4,
               VarSide=5,
               VarGpsQ=6
            };
            enum  Relation {
               Equal = 0,
               NotEqual = 1,
               LessThan = 2,
               GreaterThan = 3,
               LessThanOrEqual = 4,
               GreaterThanOrEqual = 5
            };
            int8 type;
            int8 variable;
            int8 relation;
            int8 xing;
            int16 limit;
            int16 nextOrTrueId;
            int16 falseId;
            int16 clipOrMacroId;
      };

      class Macro {
         public:
            char name[50];   // for debugging only
            int16 startnode;
            std::vector<Node> nodes;
      };

      int16 m_numSounds;
      char * m_clipFileNameBuffer;
      char **m_clipFileNames;

      Macro * m_macros;
      int16 m_readMacros;
      Macro * m_curM;

      int m_soundListNormalMacro;
      int m_crossingListMacro;
      int m_newCrossingMacro;
      int m_cameraMacro;
      int m_offTrackMacro;
      int m_atDestMacro;
      int m_wrongWayMacro;
      int m_gpsChangeMacro;
      int m_positiveMacro;
      int m_negativeMacro;

   private:
      /* Data used by the graph-excuting subsystem */

      class RunState {
         public:
            RunState(SoundClipsList &sndLst) :
               soundList(sndLst) { }

            SoundClipsList &soundList;

            class Xing {
               public:
                  bool valid;
                  int action;
                  int exitCount;
                  int side;
                  int spokenDist;
                  int gpsQ;
            };

            // All state that is saved by a macro call 
            // or a Try node
            class GraphState {
               public:
                  int xing;
                  Macro * macro;
                  Node * node;
            };

            // The call and try/catch stack
            class CallStackEntry {
               public :
                  enum Type { Try, Call } type;
                  GraphState oldGraphState;
                  int clipListSize;
            };

            Xing xings[2];
            GraphState gs;
            typedef std::stack<CallStackEntry> CallStack;
            CallStack callStack;
      };

   private:
      int executeSpecialMacro(
            int startMacro, 
            DistanceInfo &deadlines,
            SoundClipsList &soundList);
      void runGraph(int startMacro, class RunState & rs);
      bool moveToNextNode(class RunState & rs);
      bool gotoNodeInMacro(class RunState & rs, int nextNodeId);
      bool evalCondition(const RunState & rs, const Node & node);


      // FIXME - XXX - Debugging only
      Log * m_log;


   }; /* AudioCtrlLanguage */

} /* namespace isab */

#endif

