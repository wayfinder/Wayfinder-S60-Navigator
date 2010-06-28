/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "IniFile.h"
#include "CleanupSupport.h"
#include "WayFinderConstants.h"
#include "WFSymbianUtil.h"
#include "Buffer.h"
#include "WayFinderSettings.h"
/* #include "WayFinderAppUi.h" */

using isab::Buffer;

IniFile::IniFile(class RFs& session, const TDesC& wayfinderpath) :
   m_session(&session), m_path(&wayfinderpath),
   inifileVersion(MY_INIFILE_VERSION_NUMBER),
   latestNewsChecksum(MAX_INT32),
   shownNewsChecksum(MAX_INT32),
   numTransactionsLeft(MAX_INT32),
   latestNewsImageFailed(1),
   useGpsStartup(0),
   lastGpsID1(0),
   lastGpsID2(0),
   goldRegistered(1),
   goldDaysLeft(MAX_INT16),
   showWelcomeSetting(1),
   showNewsSetting(1),
   trialVersion(1),
   firstRun(1),
   sendtSilverSms(0),
   avoidAkm(0),
   firstRun2(1),
   firstRun3(1),
   wayfinderType(isab::GuiProtEnums::InvalidWayfinderType),
   showPrivacyStatement(1),
   showEndUserWarningMessage(1)
{
#if !defined(USE_AKM) && !defined(BRONZE_TRIAL)
   // If not using AKM it can only be gold
   trialVersion = 0;
   goldRegistered = 1;
   wayfinderType = isab::GuiProtEnums::Gold;
#else
   trialVersion = 1;
   goldRegistered = 0;
   wayfinderType = isab::GuiProtEnums::Trial;
#endif
}

int IniFile::DecodeData(Buffer* buf)
{
   /* Read values. */
   if (buf->remaining()>=4) {
      inifileVersion = buf->readNext32bit();
   } else {
      return -1;
   }
   if (inifileVersion != MY_INIFILE_VERSION_NUMBER) {
      /* Abort reading, reinitialize. */
      return -1;
   }
   if (buf->remaining()>=4){latestNewsChecksum = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=4){shownNewsChecksum = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=4){numTransactionsLeft = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=2){latestNewsImageFailed = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){useGpsStartup = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=4){lastGpsID1 = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=4){lastGpsID2 = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=2){goldRegistered = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){goldDaysLeft = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){showWelcomeSetting = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){showNewsSetting = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){trialVersion = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){firstRun = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){sendtSilverSms = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){avoidAkm = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){firstRun2 = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){firstRun3 = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=4){wayfinderType = buf->readNext32bit();}
   else { return -2; }
   if (buf->remaining()>=2){showPrivacyStatement = buf->readNext16bit();}
   else { return -2; }
   if (buf->remaining()>=2){showEndUserWarningMessage = buf->readNext16bit();}
   else { return -2; }

   return 0;

}

void IniFile::ReadL()
{
   TBuf<128> *filename = new (ELeave) TBuf<128>(*m_path);
   CleanupStack::PushL(filename);
   filename->Append(KIniFileName);
   {
      TBuf8<1024> data;
      Buffer *buf = new Buffer(data.Length() +6); //a little margin
      CleanupStack::PushL(TCleanupItem(CleanupClass<Buffer>::Cleanup, buf));
      if(!buf || (0 == buf->capacity())){ //failed to allocate mem
         User::LeaveNoMemory();
      }
      TInt errorCode = KErrNone;
      TBool res = WFSymbianUtil::getBytesFromFile(*m_session, *filename,
                                                  data, errorCode);
      
      /* Initialize values to sane defaults. */
      Reset();

      if (res) {
         /* Success. */
         /* Copy data to intelligent format! */
         for (int i = 0; i < data.Length(); i++ ) {
            buf->writeNext8bit(data[i]);
         }
         buf->setReadPos(0);
         if (DecodeData(buf) <= -3) {
            /* Inconsistent values read, reinitialize. */
            Reset();
         }
      } else {
         if (errorCode != KErrNone) {
            /* Failed to open/read ini-file. */
         } else {
            /* Too much data to read into 1024 bytes! */
         }
      }
      CleanupStack::PopAndDestroy(); //buf
   }
   CleanupStack::PopAndDestroy();//filename
}

int IniFile::Write()
{
   /* Create buffer. */
   Buffer buf(1024);
   if(0 == buf.capacity()){ //no mem allocated
      return KErrNoMemory;
   }
   
   buf.writeNext32bit(inifileVersion);
   buf.writeNext32bit(latestNewsChecksum);
   buf.writeNext32bit(shownNewsChecksum);
   buf.writeNext32bit(numTransactionsLeft);
   buf.writeNext16bit(latestNewsImageFailed);
   buf.writeNext16bit(useGpsStartup);
   buf.writeNext32bit(lastGpsID1);
   buf.writeNext32bit(lastGpsID2);
   buf.writeNext16bit(goldRegistered);
   buf.writeNext16bit(goldDaysLeft);
   buf.writeNext16bit(showWelcomeSetting);
   buf.writeNext16bit(showNewsSetting);
   buf.writeNext16bit(trialVersion);
   buf.writeNext16bit(firstRun);
   buf.writeNext16bit(sendtSilverSms);
   buf.writeNext16bit(avoidAkm);
   buf.writeNext16bit(firstRun2);
   buf.writeNext16bit(firstRun3);
   buf.writeNext32bit(wayfinderType);
   buf.writeNext16bit(showPrivacyStatement);
   buf.writeNext16bit(showEndUserWarningMessage);

   const uint8 *data = buf.accessRawData();
   int32 len = buf.getLength();
   TBuf<128> filename(*m_path);
   filename.Append(KIniFileName);

   if (data) {
      int res = WFSymbianUtil::writeBytesToFile(*m_session, filename, data, 
                                                len, NULL);
      if (res < 0) {
         /* Failed to write ini-file. */
         /* XXX What to do? */
      }
   } else {
      /* XXX What does this mean? */
   }

   return 0;
}


void IniFile::Reset()
{
   // Reset the values to sane defaults.
   *this = IniFile(*m_session, *m_path);
}

void IniFile::SetShownNewsChecksum()
{
   shownNewsChecksum = latestNewsChecksum;
   if(KErrNoMemory == Write()){
      ///XXXX
      //WF_DEBUG_DIALOG("Failed to write the new newschecksum to the inifile");
   }
}


TInt64 IniFile::getGpsId() const
{
   return MakeInt64(lastGpsID1, lastGpsID2);
}

void IniFile::setGpsId(TInt64 newId)
{
   lastGpsID1 = HIGH(newId);
   lastGpsID2 = LOW(newId);
   useGpsStartup = int16(newId != 0);
   Write();
}

void IniFile::setTrialOldStyle()
{
   trialVersion = 1;
   goldRegistered = 0;
   wayfinderType = isab::GuiProtEnums::Trial;
   Write();
}

void IniFile::setSilverOldStyle()
{
   trialVersion = 0;
   goldRegistered = 0;
   wayfinderType = isab::GuiProtEnums::Silver;
   Write();
}

void IniFile::setGoldOldStyle()
{
   trialVersion = 0;
   goldRegistered = 1;
   wayfinderType = isab::GuiProtEnums::Gold;
   Write();
}


void IniFile::setWayfinderType(enum isab::GuiProtEnums::WayfinderType aType)
{
   wayfinderType = aType;
   Write();
}

enum isab::GuiProtEnums::WayfinderType IniFile::getWayfinderType() const
{
   return isab::GuiProtEnums::WayfinderType(wayfinderType);
}
