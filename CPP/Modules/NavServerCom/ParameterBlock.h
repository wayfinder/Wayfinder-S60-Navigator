/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PARAMETERBLOCK_H
#define PARAMETERBLOCK_H
#include "Buffer.h"
#include "nav2util.h"
#include "Log.h"
namespace isab{

   /** A class for controlling the server parameters.*/
   class ParameterBlock : public Buffer{
   public:
      /** The parameter tags.*/
      enum ParameterTags{
         EndOfParams             = 0x00000000,
         PollServer              = 0x01000000,
         AttemptReflash          = 0x01000100,
         KeepCallUp              = 0x01000200,
         AttachCrossingMaps      = 0x02000000,
         ImageCrossingMaps       = 0x02000100,
         CrossingMapsImageFormat = 0x02000200,
         CrossingMapsImageSize   = 0x02000300,
         TopRegionChecksum       = 0x02000400,
         GetFullRouteGraph       = 0x02000500,
         MaxSearchMatches        = 0x02000600,
         UserLogin               = 0x02000700,
         UserPassword            = 0x02000800,
         UserLicense             = 0x02000900,
         MSISDNNumber            = 0x02000a00,
         ProgramVersion          = 0x02000b00,
         ClientType              = 0x02000c00,
         ClientTypeOptions       = 0x02000d00,
         ResourcesVersion        = 0x02000e00,
         SendProgramUpdate       = 0x02000f00,
         SendResourceUpdate      = 0x02001000,
         AlternativeServer       = 0x02001100,
         NavID                   = 0x02001200,
         CallCenterMSISDN        = 0x02001300,
         LanguageOfClient        = 0x02001400,
         SubscriptionLeft        = 0x02001500,
         LatestNews              = 0x02001600,
         Categories              = 0x02001700,
         TransactionsLeft        = 0x02001800,
         CallCenterChecksum      = 0x02001900,
         WayfinderType           = 0x02001a00,
         NewPasswordSet          = 0x02001b00,
         PublicUserName          = 0x02001c00,
         ServerListChecksum      = 0x02001d00,
         DisableNoRegionCheck    = 0x02001e00,
         OldUserLicense          = 0x02010100,
         ServerAuthBob           = 0x02010200,
         ServerAuthBobChecksum   = 0x02010300,
         CenterPointAndScale     = 0x02010400,
         UserRights              = 0x02010500,
         NewVersion              = 0x02010600,
         LatestNewsId            = 0x02010700,
         FavoritesCRC            = 0x02010800,
         ServerPtui              = 0x02010900,
         NewVersionUrl           = 0x02010a00,
         ReflashMe               = 0x03000000,
         MaxPacketSize           = 0x03000100,
      };

   private:
      bool m_AttemptReflash;
      bool m_AttemptReflashSet;
   public:
      bool getAttemptReflash(bool* isSet = NULL);
      void setAttemptReflash(bool val);

   private:
      uint8 m_KeepCallUp;
      bool m_KeepCallUpSet;
   public:
      uint8 getKeepCallUp(bool* isSet = NULL);
      void setKeepCallUp(uint8 val);

   private:
      bool m_AttachCrossingMaps;
      bool m_AttachCrossingMapsSet;
   public:
      bool getAttachCrossingMaps(bool* isSet = NULL);
      void setAttachCrossingMaps(bool val);

   private:
      bool m_ImageCrossingMaps;
      bool m_ImageCrossingMapsSet;
   public:
      bool getImageCrossingMaps(bool* isSet = NULL);
      void setImageCrossingMaps(bool val);

   private:
      uint8 m_CrossingMapsImageFormat;
      bool m_CrossingMapsImageFormatSet;
   public:
      uint8 getCrossingMapsImageFormat(bool* isSet = NULL);
      void setCrossingMapsImageFormat(uint8 val);

   private:
      uint32 m_CrossingMapsImageSize;
      bool m_CrossingMapsImageSizeSet;
   public:
      uint32 getCrossingMapsImageSize(bool* isSet = NULL);
      void setCrossingMapsImageSize(uint32 val);

   private:
      uint32 m_TopRegionChecksum;
      bool m_TopRegionChecksumSet;
   public:
      uint32 getTopRegionChecksum(bool* isSet = NULL);
      void setTopRegionChecksum(uint32 val);

   private:
      uint8 m_GetFullRouteGraph;
      bool m_GetFullRouteGraphSet;
   public:
      uint8 getGetFullRouteGraph(bool* isSet = NULL);
      void setGetFullRouteGraph(uint8 val);

   private:
      uint8 m_MaxSearchMatches;
      bool m_MaxSearchMatchesSet;
   public:
      uint8 getMaxSearchMatches(bool* isSet = NULL);
      void setMaxSearchMatches(uint8 val);

   private:
      char* m_UserLogin;
      bool m_UserLoginSet;
   public:
      const char* getUserLogin(bool* isSet = NULL);
      void setUserLogin(const char* val);

   private:
      char* m_UserPassword;
      bool m_UserPasswordSet;
   public:
      const char* getUserPassword(bool* isSet = NULL);
      void setUserPassword(const char* val);

   private:
      uint8* m_UserLicense;
      bool m_UserLicenseSet;
      int m_UserLicenseLen;
   public:
      const uint8* getUserLicense( int& len, bool* isSet = NULL);
      void setUserLicense(const uint8* val, int len);

   private:
      uint8* m_oldUserLicense;
      bool m_oldUserLicenseSet;
      int m_oldUserLicenseLen;
   public:
      const uint8* getOldUserLicense( int& len, bool* isSet = NULL );
      void setOldUserLicense( const uint8* val, int len );

   private:
      char*  m_MSISDNNumber;
      bool m_MSISDNNumberSet;
   public:
      const char* getMSISDNNumber(bool* isSet = NULL);
      void setMSISDNNumber(const char* val);

   private:
      uint32 m_ProgramVersion[3];
      bool m_ProgramVersionSet;
   public:
      const uint32* getProgramVersion(bool* isSet = NULL);
      void setProgramVersion(const uint32* val);

   private:
      char* m_newVersion;
      bool m_newVersionSet;
   public:
      const char* getNewVersion( bool* isSet = NULL );
      void setNewVersion( const char* val );

   private:
      char* m_newVersionUrl;
      bool m_newVersionUrlSet;
   public:
      const char* getNewVersionUrl( bool* isSet = NULL );
      void setNewVersionUrl( const char* val );

   private:
      char* m_ClientType;
      bool m_ClientTypeSet;
   public:
      const char* getClientType(bool* isSet = NULL);
      void setClientType(const char*  val);

   private:
      char* m_ClientTypeOptions;
      bool m_ClientTypeOptionsSet;
   public:
      const char*  getClientTypeOptions(bool* isSet = NULL);
      void setClientTypeOptions(const char* val);

   private:
      uint32 m_ResourcesVersion[3];
      bool m_ResourcesVersionSet;
   public:
      const uint32* getResourcesVersion(bool* isSet = NULL);
      void setResourcesVersion(const uint32* val);

   private:
      bool m_SendProgramUpdate;
      bool m_SendProgramUpdateSet;
   public:
      bool getSendProgramUpdate(bool* isSet = NULL);
      void setSendProgramUpdate(bool val);

   private:
      bool m_SendResourceUpdate;
      bool m_SendResourceUpdateSet;
   public:
      bool getSendResourceUpdate(bool* isSet = NULL);
      void setSendResourceUpdate(bool val);

   private:
      char* m_AlternativeServer;
      bool m_AlternativeServerSet;
   public:
      const char* getAlternativeServer(bool* isSet = NULL);
      void setAlternativeServer(const char* val);

   private:
      uint32 m_NavID;
      bool m_NavIDSet;
   public:
      uint32 getNavID(bool* isSet = NULL);
      void setNavID(uint32 val);

   private:
      char* m_CallCenterMSISDN;
      bool m_CallCenterMSISDNSet;
   public:
      const char* getCallCenterMSISDN(bool* isSet = NULL);
      void setCallCenterMSISDN(const char* val);

   private:
      uint32 m_LanguageOfClient;
      bool m_LanguageOfClientSet;
   public:
      uint32 getLanguageOfClient(bool* isSet = NULL);
      void setLanguageOfClient(uint32 val);

   private:
      uint32 m_TransactionsLeft;
      bool m_TransactionsLeftSet;
   public:
      uint32 getTransactionsLeft(bool* isSet = NULL);
      void setTransactionsLeft(uint32 val);

   private:
      uint16 m_SubscriptionLeft;
      bool m_SubscriptionLeftSet;
   public:
      uint16 getSubscriptionLeft(bool* isSet = NULL);
      void setSubscriptionLeft(uint16 val);

   private:
      uint32 m_LatestNews;
      bool m_LatestNewsSet;
   public:
      uint32 getLatestNews(bool* isSet = NULL);
      void setLatestNews(uint32 val);

   private:
      uint32 m_Categories;
      bool m_CategoriesSet;
   public:
      uint32 getCategories(bool* isSet = NULL);
      void setCategories(uint32 val);

   private:
      uint32 m_CallCenterChecksum;
      bool m_CallCenterChecksumSet;
   public:
      uint32 getCallCenterChecksum(bool* isSet = NULL);
      void setCallCenterChecksum(uint32 val);

   private:
      uint8 m_WayfinderType;
      bool m_WayfinderTypeSet;
   public:
      uint8 getWayfinderType(bool* isSet = NULL);
      void setWayfinderType(uint8 val);

   private:
      char* m_NewPasswordSet;
      bool m_NewPasswordSetSet;
   public:
      char* getNewPasswordSet(bool* isSet = NULL);
      void setNewPasswordSet(const char* val);

   private:
      char* m_PublicUserName;
      bool m_PublicUserNameSet;
   public:
      char* getPublicUserName(bool* isSet = NULL);
      void setPublicUserName(const char* val);

   private:
      uint32 m_ServerListChecksum;
      bool m_ServerListChecksumSet;
   public:
      uint32 getServerListChecksum(bool* isSet = NULL);
      void setServerListChecksum(uint32 val);

   private:
      bool m_DisableNoRegionCheck;
      bool m_DisableNoRegionCheckSet;
   public:
      bool getDisableNoRegionCheck(bool* isSet = NULL);
      void setDisableNoRegionCheck(bool val);

   private:
      bool m_ReflashMe;
      bool m_ReflashMeSet;
   public:
      bool getReflashMe(bool* isSet = NULL);
      void setReflashMe(bool val);

   private:
      uint16 m_MaxPacketSize;
      bool m_MaxPacketSizeSet;
   public:
      uint16 getMaxPacketSize(bool* isSet = NULL);
      void setMaxPacketSize(uint16 val);

   private:
      uint8* m_serverAuthBob;
      bool m_serverAuthBobSet;
      int m_serverAuthBobLen;
   public:
      const uint8* getServerAuthBob( int& len, bool* isSet = NULL );
      void setServerAuthBob( const uint8* val, int len );

   private:
      uint32 m_serverAuthBobChecksum;
      bool m_serverAuthBobChecksumSet;
   public:
      uint32 getServerAuthBobChecksum(bool* isSet = NULL);
      void setServerAuthBobChecksum(uint32 val);

   private:
      uint8* m_centerPointAndScale;
      bool m_centerPointAndScaleSet;
      int m_centerPointAndScaleLen;
   public:
      const uint8* getCenterPointAndScale( int& len, bool* isSet = NULL );
      void setCenterPointAndScale( const uint8* val, int len );

   private:
      uint32* m_userRights;
      bool m_userRightsSet;
      int m_userRightsLen;
   public:
      const uint32* getUserRights( int& len, bool* isSet = NULL );
      void setUserRights( const uint32* val, int len );

   private:
      char* m_latestNewsId;
      bool m_latestNewsIdSet;
   public:
      const char* getLatestNewsId( bool* isSet = NULL );

      void setLatestNewsId( const char* val );

   private:
      char* m_favoritesCRC;
      bool m_favoritesCRCSet;
   public:
      const char* getFavoritesCRC( bool* isSet = NULL );

      void setFavoritesCRC( const char* val );

   private:
      uint32 m_serverPtui;
      bool m_serverPtuiSet;
   public:
      uint32 getServerPtui(bool* isSet = NULL);
      void setServerPtui(uint32 val);

   public:
      ParameterBlock();
      ParameterBlock(const uint8* data, int size);
      ParameterBlock(Buffer& data);
      virtual ~ParameterBlock();
      void encode();
      void decode();
      bool operator==(const ParameterBlock& rhs) const;

      int output(Log& log, Log::levels level = Log::LOG_DEBUG) const
#ifndef NO_LOG_OUTPUT
         ;
#else
      {
         return 0;
      }
#endif

   };

}
#endif
