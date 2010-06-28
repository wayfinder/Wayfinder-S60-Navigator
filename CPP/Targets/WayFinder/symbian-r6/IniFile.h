/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INIFILE_H
#define INIFILE_H
#include "arch.h"
#include "GuiProt/ServerEnums.h"
namespace isab{
   class Buffer;
}
class RFs;

/// Class representing the ini-file.
class IniFile{
   ///Ini file version constants
   enum version{ 
      ///The supported version number.
      MY_INIFILE_VERSION_NUMBER = 1
   };
   ///Decode data read from file and fill in the member variables.
   ///@param buf a pointer to a <code>Buffer</code> containing the IniFile 
   ///           data.
   ///@return  0 if everything goes well.
   ///        -1 if there is less than 4 bytes of data or if the version 
   ///           is unsupported.
   ///        -2 if there was to little data in the buffer.
   int DecodeData(isab::Buffer* buf);

   class RFs* m_session;
   const TDesC* m_path;

public:
   int32 inifileVersion;    
   int32 latestNewsChecksum;    //checksum for latest news image
   int32 shownNewsChecksum;     //determine whether to show news image
   int32 numTransactionsLeft;   //transactions left on user account
   int16 latestNewsImageFailed; //something wrong with the latest news image
   int16 useGpsStartup;         //connect to gps at startup
   uint32 lastGpsID1;           //latest GPS used
   uint32 lastGpsID2;           //latest GPS used more bits
   int16 goldRegistered;        //has paid for gold version
   int16 goldDaysLeft;          //day left on gold subscription
   int16 showWelcomeSetting;    //?
   int16 showNewsSetting;       //?
   int16 trialVersion;          //still in trial
   int16 firstRun;              //first time ever!
   int16 sendtSilverSms;        //has sent the silver reg sms
   int16 avoidAkm;              //avoid the AKM
   int16 firstRun2;             //first time ever! Second one...
   int16 firstRun3;             //first time ever! Third one...
   int32 wayfinderType;         //Actually hold GuiProtEnums::WayfinderType;
   int16 showPrivacyStatement;   //determine whether to show privacy statement or not
   int16 showEndUserWarningMessage; //determine whether to show end user warning message every time or not.

   // Old deprecated way of handling WayfinderType.
   TBool isTrialDoNotUse() const;
   TBool isSilverDoNotUse() const;
   TBool isGoldDoNotUse() const;
   void setTrialOldStyle();
   void setSilverOldStyle();
   void setGoldOldStyle();

   void setWayfinderType(enum isab::GuiProtEnums::WayfinderType aType);
   enum isab::GuiProtEnums::WayfinderType getWayfinderType() const;
//    enum isab::GuiProtEnums::WayfinderType IniFile::getWayfinderType() const;
      
   IniFile(class RFs& session, const TDesC& wayfinderpath);
   void ReadL();
   int Write();
   void Reset();
   void SetShownNewsChecksum();

   typedef int16 IniFile::*member_variable;
   typedef int (*bool_function)();
   template<class functor>
   TBool doOnce(int16 doIf, member_variable var, functor func)
   {
      TBool ret = EFalse;
      if(!(ret = !(this->*var == doIf)) && (ret = func()) ){
         this->*var = !doIf;
         Write();
      }
      return ret;
   }

   TInt64 getGpsId() const;
   void setGpsId(TInt64 newId);

};


inline TBool IniFile::isTrialDoNotUse() const
{
   return wayfinderType == isab::GuiProtEnums::Trial;
   // trialVersion && !goldRegistered;
}

inline TBool IniFile::isSilverDoNotUse() const
{
   return wayfinderType == isab::GuiProtEnums::Silver;
   //!trialVersion && !goldRegistered;
}

inline TBool IniFile::isGoldDoNotUse() const
{
   return wayfinderType == isab::GuiProtEnums::Gold;
   //goldRegistered && !trialVersion;
}



#endif
