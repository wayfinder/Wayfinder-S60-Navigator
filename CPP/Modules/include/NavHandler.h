/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVHANDLER_H
#define NAVHANDLER_H

#include "arch.h"
#include <vector>

namespace isab {


class NavRequestPacket;
class NavReplyPacket;
class Buffer;
class NParamBlock;
class ParameterBlock;
class Log;
class Tracking;
class HWID;


/**
 * Class handling NavServerProt, v10+, packets.
 *
 */
class NavHandler {
   public:
      /**
       * Constructor.
       */
      NavHandler( uint8 protoVer );


      /**
       * Destructor.
       */
      ~NavHandler();


      /**
       * Handles a reply.
       */
      NavReplyPacket* handleReply( uint8 protoVer, uint16 type, uint8 reqID, 
                                   uint8 reqVer, uint8 statusCode, 
                                   const char* statusMessage, 
                                   const uint8* buff, uint32 buffLen );
      

      NavRequestPacket* handleRequest( uint8 protoVer, uint16 type, 
                                       uint8 reqID, Buffer* content );

      void handleParameters( Buffer& data, bool* secondaryServer = NULL );


      void addAuthParams( NParamBlock& params );


      bool haveAuthParams( bool& haveLicense );


      bool haveChangedLicense();


      void addChangedLicense( NParamBlock& params );


      /**
       * XXX: Copied from MC2, MC2CRC32, 20040622 10:30.
       * Calculates the CRC for a buffer.
       *
       * @param buf The buffer to calculate CRC for.
       * @param len The length of buf.
       * @param crc The start value for the crc, default 0.
       * @return The CRC for buf.
       */
      uint32 crc32( const uint8* buf, uint32 len, uint32 crc = 0 ) const;


      void makeReplyBuffer( Buffer& buff, NavReplyPacket* pack, 
                            uint8 reg_ver, uint8 statusCode,
                            uint8 req_id, uint16 type,
                            uint8 protoVer );
      

      /**
       * Get the current sessionKey, may be NULL.
       */
      const char* getSessionKey() const;
      
      
      /**
       * Set the current sessionKey, may be NULL.
       */
      void setSessionKey( const char* key );


      void addSessionParams( NParamBlock& params );


      bool needParameterSync() const;


      void makePollPackets( NavReplyPacket* pack,
                            std::vector<NavRequestPacket*>& polls );


      bool changedWFST( uint8 wfst );


      void makeChangedWFSTBuffer( Buffer& buff, uint8 wfst );


      bool upgradeParams( NavReplyPacket* pack, Buffer* wfB );


      /**
       * Sets the tracking.
       */
      void setTracking( Tracking* tracking );

      /**
       * Set the license key type.
       * IMEI if NULL or not set. Otherwise "imei" or "imsi".
       */
      void setLicenseKeyType( const char* licenseKeyType );

      /**
       * Add a hardware ID
       * To start with "imei" or "imsi".
       */
      void addHWID( HWID* hwid);

      /**
       * 
       */
      void setWayfinderIdStartUp( bool wayfinderIdStartUp );

      /**
       * 
       */
      bool isWayfinderIdStartUp() const;

   private:
      /**
       * Handles a WhoAmI reply and sets auth params.
       */
      void handleWhoAmIParams( Buffer& buff, NParamBlock& params );


      /**
       * Handles a Upgrade reply and may set lastServerInfo to 0.
       */
      void handleUpgradeParams( Buffer& buff, NParamBlock& params );


      /**
       * Handles top region reply.
       */
      void handleTopRegionParams( Buffer& buff, NParamBlock& params );


      /**
       * Handles changed license, may set oldlicense in m_lastParams.
       */
      void handleChangedLicenseParams( Buffer& buff, NParamBlock& params );


      /**
       * Handle search request.
       */
      void setSearchParams( Buffer* content, NParamBlock& params );


      /**
       * Handle route request.
       */
      void setRouteParams( Buffer* content, NParamBlock& params );


      /**
       * Handle a server auth bob reply.
       */
      void handleServerAuthBobParams( Buffer& buff, NParamBlock& params );


      /**
       * Handles a server info reply.
       */
      void handleServerInfoParams( Buffer& buff, NParamBlock& params );


      /**
       * Handles a param reply.
       */
      void handleParamReplyParams( Buffer& buff, NParamBlock& params );


      /**
       * Sets the local auth params.
       */
      void setAuthParams( ParameterBlock* lastParams );


      /**
       * Holds the precalculated crc byte values.
       */
      uint32* m_crcmap;


      /**
       * Constructs the precalculated crc values.
       */
      uint32* makeCRCMap();


      /// The user login auth param
      char* m_userLogin;


      /// The user password auth param
      char* m_userPassword;


      /// The user license auth param
      uint8* m_userLicense;


      /// The length of user license auth param
      uint32 m_userLicenseLen;


      /// The client type
      char* m_clientType;


      /// The client type options
      char* m_clientTypeOptions;


      /// The program version
      uint32 m_programVersion[3];


      /// Language
      uint32 m_languageOfClient;


      /// WFST
      uint8 m_wayfinderType;


      /// SessionKey
      char* m_sessionKey;


      /// Last time we sent server info request
      time_t m_lastServerInfo;


      /// The last parameterblock
      ParameterBlock* m_lastParams;


      /// The log object
      Log* m_log;


      /// If running against a secondary server
      bool m_secondaryServer;


      /// The protoVer
      uint8 m_protoVer;


      /// The tracking if any.
      Tracking* m_tracking;

      /// License key type.
      char* m_licenseKeyType;

      // Vector containing all hardware id:s
      std::vector<HWID*> m_vhardwareIDs; 

      // Flag to know if it is wayfinder id startup or not.
      bool m_wayfinderIdStartUp;

};


} /* namespace isab */

#endif // NAVHANDLER_H

