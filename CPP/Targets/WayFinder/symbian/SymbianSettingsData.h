/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SYMBIAN_SETTINGS_DATA_H
#define SYMBIAN_SETTINGS_DATA_H

#include "GenericSettingsData.h"


/**
 * SymbianSettingsData
 *
 * This class is designed to be an intermediate settings holder
 * which can include symbian specific settings.
 */
class SymbianSettingsData
 : public GenericSettingsData
{
public:
   /**
    * Constructor
    */
   SymbianSettingsData();
   /**
    * Destructor
    */
   virtual ~SymbianSettingsData();

   /**
    * setUserRights - Set the user rights from the data.
    * This method must be implemented by the GUI specific class.
    */
   virtual void setUserRights(const int32* data, int32 numEntries) = 0;

public:
   /**
    * Used to indicate the latest news checksum for startup.
    */
   int32 m_latestNewsChecksum;               // STARTUP
   /**
    * Used to indicate the last shown latest news checksum for startup.
    */
   int32 m_latestShownNewsChecksum;          // STARTUP
   /**
    * Indicates current wayfinder type.
    */
   int32 m_wayfinderType;                    // STARTUP
   /**
    * Indicates current volume in percent.
    */
   int32 m_soundVolume;                      // AUDIO
   /**
    * Indicates if speaker should be used (mute) (unused?)
    */
   int32 m_useSpeaker;                       // AUDIO

   /**
    * Indicates if the settings should include web username.
    */
   int32 m_canShowWebUsername;
   /**
    * Indicates if the settings should include web password.
    */
   int32 m_canShowWebPassword;
   /**
    * Is GPS allowed, ie should the GPS setting be shown?
    */
   int32 m_allowGps;
   /**
    * Is the client in Earth mode (hides some other settings)
    */
   int32 m_isIron;
   /**
    * Should settings show languages with no voice installed?
    */
   int32 m_showLangsWithoutVoice;
   /**
    * Should settings show languages with no text strings installed?
    */
   int32 m_showLangsWithoutResource;

   /**
    * Path to application.
    */
   HBufC* m_applicationPath;                                   
   /**
    * Application DLL name.
    */
   HBufC* m_applicationDllName;
   /**
    * Audio base path.
    */
   HBufC* m_audioBasePath;
   /**
    * Application name as path.
    */
   HBufC* m_applicationNameAsPath;
   /**
    * Audio format prefix
    */
   HBufC* m_audioFormatPrefix;
   /**
    * Resource without the extension.
    */
   HBufC* m_resourceNoExt;

};

#endif

