/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "arch.h"
#ifdef __linux
# include <iostream>
#endif
#include "ParameterBlock.h"
#include "NavServerComEnums.h"
#ifdef __linux
# include <iostream>
#endif

#define INITLIST                                      \
      m_AttemptReflash(false),                        \
      m_AttemptReflashSet(false),                     \
      m_KeepCallUp(0),                                \
      m_KeepCallUpSet(false),                         \
      m_AttachCrossingMaps(false),                    \
      m_AttachCrossingMapsSet(false),                 \
      m_ImageCrossingMaps(false),                     \
      m_ImageCrossingMapsSet(false),                  \
      m_CrossingMapsImageFormat(0),                   \
      m_CrossingMapsImageFormatSet(false),            \
      m_CrossingMapsImageSize(0),                     \
      m_CrossingMapsImageSizeSet(false),              \
      m_TopRegionChecksum(0),                         \
      m_TopRegionChecksumSet(false),                  \
      m_GetFullRouteGraph(0),                         \
      m_GetFullRouteGraphSet(false),                  \
      m_MaxSearchMatches(0),                          \
      m_MaxSearchMatchesSet(false),                   \
      m_UserLogin(NULL),                              \
      m_UserLoginSet(false),                          \
      m_UserPassword(NULL),                           \
      m_UserPasswordSet(false),                       \
      m_UserLicense(NULL),                            \
      m_UserLicenseSet(false),                        \
      m_UserLicenseLen(0),                            \
      m_oldUserLicense( NULL ),                       \
      m_oldUserLicenseSet( false ),                   \
      m_oldUserLicenseLen( 0 ),                       \
      m_MSISDNNumber(NULL),                           \
      m_MSISDNNumberSet(false),                       \
      m_ProgramVersionSet(false),                     \
      m_newVersion( NULL ),                           \
      m_newVersionSet(false),                         \
      m_newVersionUrl(NULL),                          \
      m_newVersionUrlSet(false),                      \
      m_ClientType(NULL),                             \
      m_ClientTypeSet(false),                         \
      m_ClientTypeOptions(NULL),                      \
      m_ClientTypeOptionsSet(false),                  \
      m_ResourcesVersionSet(false),                   \
      m_SendProgramUpdate(false),                     \
      m_SendProgramUpdateSet(false),                  \
      m_SendResourceUpdate(false),                    \
      m_SendResourceUpdateSet(false),                 \
      m_AlternativeServer(NULL),                      \
      m_AlternativeServerSet(false),                  \
      m_NavID(0),                                     \
      m_NavIDSet(false),                              \
      m_CallCenterMSISDN(NULL),                       \
      m_CallCenterMSISDNSet(false),                   \
      m_LanguageOfClient(NavServerComEnums::ENGLISH), \
      m_LanguageOfClientSet(false),                   \
      m_TransactionsLeft(0),                          \
      m_TransactionsLeftSet(false),                   \
      m_SubscriptionLeft(0),                          \
      m_SubscriptionLeftSet(false),                   \
      m_LatestNews(0),                                \
      m_LatestNewsSet(false),                         \
      m_Categories(0),                                \
      m_CategoriesSet(false),                         \
      m_CallCenterChecksum(0),                        \
      m_CallCenterChecksumSet(false),                 \
      m_WayfinderType(0),                             \
      m_WayfinderTypeSet(false),                      \
      m_NewPasswordSet(NULL),                         \
      m_NewPasswordSetSet(false),                     \
      m_PublicUserName(NULL),                         \
      m_PublicUserNameSet(false),                     \
      m_ServerListChecksum(0),                        \
      m_ServerListChecksumSet(false),                 \
      m_DisableNoRegionCheck(false),                  \
      m_DisableNoRegionCheckSet(false),               \
      m_ReflashMe(false),                             \
      m_ReflashMeSet(false),                          \
      m_MaxPacketSize(0),                             \
      m_MaxPacketSizeSet(false),                      \
      m_serverAuthBob( NULL ),                        \
      m_serverAuthBobSet( false ),                    \
      m_serverAuthBobLen( 0 ),                        \
      m_serverAuthBobChecksum( 0 ),                   \
      m_serverAuthBobChecksumSet( false ),            \
      m_centerPointAndScale( NULL ),                  \
      m_centerPointAndScaleSet( false ),              \
      m_centerPointAndScaleLen( 0 ),                  \
      m_userRights( NULL ),                           \
      m_userRightsSet( false ),                       \
      m_userRightsLen( 0 ),                           \
      m_latestNewsId( NULL ),                         \
      m_latestNewsIdSet( false ),                      \
      m_favoritesCRC( NULL ),                         \
      m_favoritesCRCSet( false ),                     \
      m_serverPtui( 0 ),                              \
      m_serverPtuiSet( false )                        \


namespace isab{
   ParameterBlock::ParameterBlock() : 
      Buffer(), INITLIST
   {
      encode();
   }


   ParameterBlock::ParameterBlock(const uint8* data, int size) : 
      Buffer(size), INITLIST
   {
      clear();
      writeNextByteArray(data, size);
      decode();
   }

   ParameterBlock::ParameterBlock(Buffer& buf) :
      Buffer(buf.remaining()), INITLIST
   {
      clear();
      writeNextByteArray(buf.accessRawData(), buf.remaining());
      buf.jumpReadPos(buf.remaining());
      decode();
   }

   ParameterBlock::~ParameterBlock()
   {
      delete[] m_UserLogin;
      delete[] m_UserPassword;
      delete[] m_UserLicense;
      delete [] m_oldUserLicense;
      delete[] m_MSISDNNumber;
      delete[] m_ClientType;
      delete[] m_ClientTypeOptions;
      delete[] m_AlternativeServer;
      delete[] m_CallCenterMSISDN;
      delete[] m_NewPasswordSet;
      delete[] m_PublicUserName;
      delete [] m_serverAuthBob;
      delete [] m_centerPointAndScale;
      delete [] m_userRights;
      delete [] m_newVersion;
      delete [] m_newVersionUrl;
      delete [] m_latestNewsId;
      delete [] m_favoritesCRC;
   }

   void ParameterBlock::encode(){
      clear();
      if(m_AttemptReflashSet){
         if(m_AttemptReflash){
            writeNextUnaligned32bit(AttemptReflash);
         }
      }
      if(m_KeepCallUpSet){
         if(m_KeepCallUp){
            writeNextUnaligned32bit(KeepCallUp | sizeof(m_KeepCallUp));
            writeNext8bit(m_KeepCallUp);
         }
      }
      if(m_AttachCrossingMapsSet){
         if(m_AttachCrossingMaps){
            writeNextUnaligned32bit(AttachCrossingMaps);
         }
      }
      if(m_ImageCrossingMapsSet){
         if(m_ImageCrossingMaps){
            writeNextUnaligned32bit(ImageCrossingMaps);
         }
      }
      if(m_CrossingMapsImageFormatSet){
         writeNextUnaligned32bit(CrossingMapsImageFormat | 
                                 sizeof(m_CrossingMapsImageFormat));
         writeNext8bit(m_CrossingMapsImageFormat);
      }
      if(m_CrossingMapsImageSizeSet){
         writeNextUnaligned32bit(CrossingMapsImageSize | 
                                 sizeof(m_CrossingMapsImageSize));
         writeNextUnaligned32bit(m_CrossingMapsImageSize);
      }
      if(m_TopRegionChecksumSet){
         writeNextUnaligned32bit(TopRegionChecksum | 
                                 sizeof(m_TopRegionChecksum));
         writeNextUnaligned32bit(m_TopRegionChecksum);
      }
      if(m_GetFullRouteGraphSet){
         writeNextUnaligned32bit(GetFullRouteGraph | 1U);
         writeNext8bit(m_GetFullRouteGraph);
      }
      if(m_MaxSearchMatchesSet){
         writeNextUnaligned32bit(MaxSearchMatches | 1U);
         writeNext8bit(m_MaxSearchMatches);
      }
      if(m_UserLoginSet && !strequ(m_UserLogin, "")){
         writeNextUnaligned32bit(UserLogin | strlen(m_UserLogin) + 1);
         writeNextCharString(m_UserLogin);
         if(m_UserPasswordSet){
            writeNextUnaligned32bit(UserPassword | strlen(m_UserPassword) + 1);
            writeNextCharString(m_UserPassword);
         }
      }
      if(m_UserLicenseSet){
         writeNextUnaligned32bit(UserLicense | m_UserLicenseLen);
         writeNextByteArray(m_UserLicense, m_UserLicenseLen);
      }
      if ( m_oldUserLicenseSet ) {
         writeNextUnaligned32bit( OldUserLicense | m_oldUserLicenseLen );
         writeNextByteArray( m_oldUserLicense, m_oldUserLicenseLen );
      }
      if ( m_serverAuthBobSet ) {
         writeNextUnaligned32bit( ServerAuthBob | m_serverAuthBobLen );
         writeNextByteArray( m_serverAuthBob, m_serverAuthBobLen );
      }
      if ( m_serverAuthBobChecksumSet ){
         writeNextUnaligned32bit( ServerAuthBobChecksum | 
                                  sizeof( m_serverAuthBobChecksum ) );
         writeNextUnaligned32bit( m_serverAuthBobChecksum );
      }
      if ( m_centerPointAndScaleSet ) {
         writeNextUnaligned32bit( 
            CenterPointAndScale | m_centerPointAndScaleLen );
         writeNextByteArray( m_centerPointAndScale, 
                             m_centerPointAndScaleLen );
      }
      if(m_MSISDNNumberSet){
         writeNextUnaligned32bit(MSISDNNumber | strlen(m_MSISDNNumber) + 1);
         writeNextCharString(m_MSISDNNumber);
      }
      if(m_ProgramVersionSet){
         writeNextUnaligned32bit(ProgramVersion | 12U);
         writeNextUnaligned32bit(m_ProgramVersion[0]);
         writeNextUnaligned32bit(m_ProgramVersion[1]);
         writeNextUnaligned32bit(m_ProgramVersion[2]);
      }
      if ( m_newVersionSet ) {
         writeNextUnaligned32bit( NewVersion | strlen( m_newVersion ) +1 );
         writeNextCharString( m_newVersion );
      }
      if ( m_newVersionUrlSet ) {
         writeNextUnaligned32bit( NewVersionUrl | strlen( m_newVersionUrl ) +1 );
         writeNextCharString( m_newVersionUrl );
      }
      if(m_ClientTypeSet){
         writeNextUnaligned32bit(ClientType | strlen(m_ClientType) + 1);
         writeNextCharString(m_ClientType);
      }
      if(m_ClientTypeOptionsSet){
         writeNextUnaligned32bit(ClientTypeOptions | 
                                 strlen(m_ClientTypeOptions) + 1);
         writeNextCharString(m_ClientTypeOptions);
      }
      if(m_ResourcesVersionSet){
         writeNextUnaligned32bit(ResourcesVersion | 12U);
         writeNextUnaligned32bit(m_ResourcesVersion[0]);
         writeNextUnaligned32bit(m_ResourcesVersion[1]);
         writeNextUnaligned32bit(m_ResourcesVersion[2]);
      }
      if(m_SendProgramUpdateSet){
         if(m_SendProgramUpdate){
            writeNextUnaligned32bit(SendProgramUpdate);
         }
      }
      if(m_SendResourceUpdateSet){
         if(m_SendResourceUpdate){
            writeNextUnaligned32bit(SendResourceUpdate);
         }
      }
      if(m_AlternativeServerSet){
         writeNextUnaligned32bit(AlternativeServer | 
                                 strlen(m_AlternativeServer) + 1);
         writeNextCharString(m_AlternativeServer);
      }
      if(m_NavIDSet){
         writeNextUnaligned32bit(NavID | sizeof(m_NavID));
         writeNextUnaligned32bit(m_NavID);
      }
      if(m_CallCenterMSISDNSet){
         writeNextUnaligned32bit(CallCenterMSISDN | 
                                 strlen(m_CallCenterMSISDN) + 1);
         writeNextCharString(m_CallCenterMSISDN);
      }
      if(m_LanguageOfClientSet){
         writeNextUnaligned32bit(LanguageOfClient | 
                                 sizeof(m_LanguageOfClient));
         writeNextUnaligned32bit(m_LanguageOfClient);
      }
      if(m_SubscriptionLeftSet){
         writeNextUnaligned32bit(SubscriptionLeft | 
                                 sizeof(m_SubscriptionLeft));
         writeNextUnaligned16bit(m_SubscriptionLeft);
      }
      if(m_LatestNewsSet){
         writeNextUnaligned32bit(LatestNews | sizeof(m_LatestNews));
         writeNextUnaligned32bit(m_LatestNews);
      }
      if(m_CategoriesSet){
         writeNextUnaligned32bit(Categories | sizeof(m_Categories));
         writeNextUnaligned32bit(m_Categories);
      }
      if(m_TransactionsLeftSet){
         writeNextUnaligned32bit(TransactionsLeft | 
                                 sizeof(m_TransactionsLeft));
         writeNextUnaligned32bit(m_TransactionsLeft);
      }
      if(m_CallCenterChecksumSet){
         writeNextUnaligned32bit(CallCenterChecksum | 
                                 sizeof(m_CallCenterChecksum));
         writeNextUnaligned32bit(m_CallCenterChecksum);
      }
      if(m_WayfinderTypeSet){
         writeNextUnaligned32bit(WayfinderType | sizeof(m_WayfinderType));
         writeNext8bit(m_WayfinderType);
      }
      if(m_NewPasswordSetSet){
         writeNextUnaligned32bit(NewPasswordSet | 
                                 strlen(m_NewPasswordSet) + 1);
         writeNextCharString(m_NewPasswordSet);
      }
      if(m_PublicUserNameSet){
         writeNextUnaligned32bit(PublicUserName | 
                                 strlen(m_PublicUserName) + 1);
         writeNextCharString(m_PublicUserName);
      }
      if(m_ServerListChecksumSet){
         writeNextUnaligned32bit(ServerListChecksum | 
                                 sizeof(m_ServerListChecksum));
         writeNextUnaligned32bit(m_ServerListChecksum);
      }
      if ( m_userRightsSet ) {
         writeNextUnaligned32bit( UserRights | m_userRightsLen );
         for ( int i = 0 ; i < m_userRightsLen ; ++i ) {
            writeNextUnaligned32bit( m_userRights[ i ] );
         }
      }
      if ( m_latestNewsIdSet ) {
         writeNextUnaligned32bit( LatestNewsId | 
                                  strlen( m_latestNewsId ) +1 );
         writeNextCharString( m_latestNewsId );
      }
      if ( m_favoritesCRCSet ) {
         writeNextUnaligned32bit( FavoritesCRC | 
                                  strlen( m_favoritesCRC ) +1 );
         writeNextCharString( m_favoritesCRC );
      }
      if ( m_serverPtuiSet ){
         writeNextUnaligned32bit( ServerPtui | 
                                  sizeof( m_serverPtui ) );
         writeNextUnaligned32bit( m_serverPtui );
      }
      if(m_DisableNoRegionCheckSet){
         if(m_DisableNoRegionCheck){
            writeNextUnaligned32bit(DisableNoRegionCheck);
         }
      }
      if(m_ReflashMeSet){
         if(m_ReflashMe){
            writeNextUnaligned32bit(ReflashMe);
         }
      }
      if(m_MaxPacketSizeSet){
         writeNextUnaligned32bit(MaxPacketSize | sizeof(m_MaxPacketSize));
         writeNextUnaligned16bit(m_MaxPacketSize);
      }
      // end tag
      writeNextUnaligned32bit(EndOfParams);
   }

#define PARAMTRUE(size) (((size == 1) && (readNext8bit() != 0)) || (size == 0))

   void ParameterBlock::decode(){
      setReadPos(0);
      while(remaining() > 3){
         uint32 tag = readNextUnaligned32bit();
         uint8 size = uint8(tag & 0x0ff);
         size_t pos = getReadPos();
         enum ParameterTags pTag = ParameterTags(tag & 0xffffff00);
         switch(pTag){
         case EndOfParams:
            return;
         case PollServer:
            break;
         case AttemptReflash:
            m_AttemptReflashSet = true;
            m_AttemptReflash = (size == 0) || !!readNext8bit();
            break;
         case KeepCallUp:
            m_KeepCallUpSet = true;
            m_KeepCallUp = readNext8bit();
            break;
         case AttachCrossingMaps:
            m_AttachCrossingMapsSet = true;
            m_AttachCrossingMaps = (size == 0) || !!readNext8bit();
            break;
         case ImageCrossingMaps:
            m_ImageCrossingMapsSet = true;
            if(size > 0){
               m_ImageCrossingMaps = !!readNext8bit();
            } else {
               m_ImageCrossingMaps = true;
            }
            break;
         case CrossingMapsImageFormat:
            m_CrossingMapsImageFormatSet = true;
            m_CrossingMapsImageFormat = readNext8bit();
            break;
         case CrossingMapsImageSize:
            m_CrossingMapsImageSizeSet = true;
            m_CrossingMapsImageSize = readNextUnaligned32bit();
            break;
         case TopRegionChecksum:
            m_TopRegionChecksumSet = true;
            m_TopRegionChecksum = readNextUnaligned32bit();
            break;
         case GetFullRouteGraph:
            m_GetFullRouteGraphSet = true;
            m_GetFullRouteGraph = readNext8bit();
            break;
         case MaxSearchMatches:
            m_MaxSearchMatchesSet = true;
            m_MaxSearchMatches = readNext8bit();
            break;
         case UserLogin:
            m_UserLoginSet = true;
            delete[] m_UserLogin;
            m_UserLogin = strdup_new(getNextCharString());
            break;
         case UserPassword:
            m_UserPasswordSet = true;
            delete[] m_UserPassword;
            m_UserPassword = strdup_new(getNextCharString());
            break;
         case UserLicense:
            m_UserLicenseSet = true;
            delete[] m_UserLicense;
            m_UserLicense = new uint8[size];
            readNextByteArray(m_UserLicense, size);
            m_UserLicenseLen = size;
            break;
         case OldUserLicense:
            m_oldUserLicenseSet = true;
            delete [] m_oldUserLicense;
            m_oldUserLicense = new uint8[ size ];
            readNextByteArray( m_oldUserLicense, size );
            m_oldUserLicenseLen = size;
            break;
         case ServerAuthBob:
            m_serverAuthBobSet = true;
            delete [] m_serverAuthBob;
            m_serverAuthBob = new uint8[ size ];
            readNextByteArray( m_serverAuthBob, size );
            m_serverAuthBobLen = size;
            break;
         case ServerAuthBobChecksum:
            m_serverAuthBobChecksumSet = true;
            m_serverAuthBobChecksum = readNextUnaligned32bit();
            break;
         case CenterPointAndScale:
            m_centerPointAndScaleSet = true;
            delete [] m_centerPointAndScale;
            m_centerPointAndScale = new uint8[ size ];
            readNextByteArray( m_centerPointAndScale, size );
            m_centerPointAndScaleLen = size;
            break;
         case MSISDNNumber:
            m_MSISDNNumberSet = true;
            delete[] m_MSISDNNumber;
            m_MSISDNNumber = strdup_new(getNextCharString());
            break;
         case ProgramVersion:
            m_ProgramVersionSet = true;
            m_ProgramVersion[0] = readNextUnaligned32bit();
            m_ProgramVersion[1] = readNextUnaligned32bit();
            m_ProgramVersion[2] = readNextUnaligned32bit();
            break;
          case NewVersion:
            m_newVersionSet = true;
            delete [] m_newVersion;
            m_newVersion = strdup_new( getNextCharString() );
            break;
         case NewVersionUrl:
            m_newVersionUrlSet = true;
            delete [] m_newVersionUrl;
            m_newVersionUrl = strdup_new( getNextCharString() );
            break;
            
        case ClientType:
            m_ClientTypeSet = true;
            delete[] m_ClientType;
            m_ClientType = strdup_new(getNextCharString());
            break;
         case ClientTypeOptions:
            m_ClientTypeOptionsSet = true;
            delete[] m_ClientTypeOptions;
            m_ClientTypeOptions = strdup_new(getNextCharString());
            break;
         case ResourcesVersion:
            m_ResourcesVersionSet = true;
            m_ResourcesVersion[0] = readNextUnaligned32bit();
            m_ResourcesVersion[1] = readNextUnaligned32bit();
            m_ResourcesVersion[2] = readNextUnaligned32bit();
            break;
         case SendProgramUpdate:
            m_SendProgramUpdateSet =  m_SendProgramUpdate = true;
            if(size > 0){
               m_SendProgramUpdate = readNext8bit();
            }
            break;
         case SendResourceUpdate:
            m_SendResourceUpdateSet = m_SendResourceUpdate = true;
            if(size > 0){
               m_SendResourceUpdate = readNext8bit();
            }
            break;
         case AlternativeServer:
            m_AlternativeServerSet = true;
            delete[] m_AlternativeServer;
            m_AlternativeServer = strdup_new(getNextCharString());
            break;
         case NavID:
            m_NavIDSet = true;
            m_NavID = readNextUnaligned32bit();
            break;
         case CallCenterMSISDN:
            m_CallCenterMSISDNSet = true;
            delete[] m_CallCenterMSISDN;
            m_CallCenterMSISDN = strdup_new(getNextCharString());
            break;
         case LanguageOfClient:
            m_LanguageOfClientSet = true;
            m_LanguageOfClient = readNextUnaligned32bit();
            break;
         case SubscriptionLeft:
            m_SubscriptionLeftSet = true;
            m_SubscriptionLeft = readNextUnaligned16bit();
            break;
         case LatestNews:
            m_LatestNewsSet = true;
            m_LatestNews = readNextUnaligned32bit();
            break;
         case Categories:
            m_CategoriesSet = true;
            m_Categories = readNextUnaligned32bit();
            break;
         case TransactionsLeft:
            m_TransactionsLeftSet = true;
            m_TransactionsLeft = readNextUnaligned32bit();
            break;
         case CallCenterChecksum:
            m_CallCenterChecksumSet = true;
            m_CallCenterChecksum = readNextUnaligned32bit();
            break;
         case WayfinderType:
            m_WayfinderTypeSet = true;
            m_WayfinderType = readNext8bit();
            break;
         case NewPasswordSet:
            m_NewPasswordSetSet = true;
            m_NewPasswordSet = strdup_new(getNextCharString());
            break;
         case PublicUserName:
            m_PublicUserNameSet = true;
            m_PublicUserName = strdup_new(getNextCharString());
            break;
         case ServerListChecksum:
            m_ServerListChecksumSet = true;
            m_ServerListChecksum = readNextUnaligned32bit();
            break;
         case UserRights:
            {
            m_userRightsSet = true;
            delete [] m_userRights;
            m_userRights = new uint32[ size ];
            for ( uint32 i = 0 ; i < size ; ++i ) {
               m_userRights[ i ] = readNextUnaligned32bit();
            }
            m_userRightsLen = size;
            }
            break;
         case DisableNoRegionCheck:
            m_DisableNoRegionCheckSet = m_DisableNoRegionCheck =true;
            if(size > 0){
               m_DisableNoRegionCheck = readNext8bit();
            }
            break;
         case ReflashMe:
            m_ReflashMeSet = m_ReflashMe =true;
            if(size > 0){
               m_ReflashMe = readNext8bit();
            }
            break;
         case MaxPacketSize:
            m_MaxPacketSizeSet = true;
            m_MaxPacketSize = readNext8bit();
            break;
          case LatestNewsId:
            m_latestNewsIdSet = true;
            delete [] m_latestNewsId;
            m_latestNewsId = strdup_new( getNextCharString() );
            break;
          case FavoritesCRC:
            m_favoritesCRCSet = true;
            delete [] m_favoritesCRC;
            m_favoritesCRC = strdup_new( getNextCharString() );
            break;
         case ServerPtui:
            m_serverPtuiSet = true;
            m_serverPtui = readNextUnaligned32bit();
            break;
         };
         if(pos == getReadPos()){
            jumpReadPos(size);
         }
      }
   }
    
#ifndef NO_LOG_OUTPUT
   int ParameterBlock::output(Log& log, Log::levels /*level*/) const
   {
      typedef int (Log::*logFunc)(const char *format);
      int ret = 0;
      ret += log.debug("ParameterBlock %p", this);
      if(m_AttemptReflashSet){
         if(m_AttemptReflash){
            ret += log.debug("%08x : true", AttemptReflash);
         }
      }
      if(m_KeepCallUpSet){
         if(m_KeepCallUp){
            ret += log.debug("%08x : %02x", KeepCallUp | sizeof(m_KeepCallUp), 
                      m_KeepCallUp);
         }
      }
      if(m_AttachCrossingMapsSet){
         if(m_AttachCrossingMaps){
            ret += log.debug("%08x : true", AttachCrossingMaps);
         }
      }
      if(m_ImageCrossingMapsSet){
         if(m_ImageCrossingMaps){
            ret += log.debug("%08x : true", ImageCrossingMaps);
         }
      }
      if(m_CrossingMapsImageFormatSet){
         ret += log.debug("%08x : %02x", 
                   CrossingMapsImageFormat | sizeof(m_CrossingMapsImageFormat),
                   m_CrossingMapsImageFormat);
      }
      if(m_CrossingMapsImageSizeSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   CrossingMapsImageSize | sizeof(m_CrossingMapsImageSize), 
                   m_CrossingMapsImageSize);
      }
      if(m_TopRegionChecksumSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   TopRegionChecksum | sizeof(m_TopRegionChecksum), 
                   m_TopRegionChecksum);
      }
      if(m_GetFullRouteGraphSet){
         ret += log.debug("%08x : %02x", 
                   GetFullRouteGraph | sizeof(m_GetFullRouteGraph), 
                   m_GetFullRouteGraph);
      }
      if(m_MaxSearchMatchesSet){
         ret += log.debug("%08x : %02x", 
                   MaxSearchMatches | sizeof(m_MaxSearchMatches), 
                   m_MaxSearchMatches);
      }
      if(m_UserLoginSet && !strequ(m_UserLogin, "")){
         ret += log.debug("%08x : %s\\0", UserLogin | strlen(m_UserLogin) + 1, 
                   m_UserLogin);
         if(m_UserPasswordSet){
            ret += log.debug("%08x : %s\\0", 
                      UserPassword | strlen(m_UserPassword) + 1, 
                      m_UserPassword);
         }
      }
      if(m_UserLicenseSet){
         char title[16] = {0};
         snprintf(title, sizeof(title) - 1, "%08x :", 
                  UserLicense | m_UserLicenseLen);
         ret += log.debugDump(title, m_UserLicense, m_UserLicenseLen);
      }
      if ( m_oldUserLicenseSet ) {
         char title[16] = {0};
         snprintf( title, sizeof(title) - 1, "%08x :", 
                   OldUserLicense | m_oldUserLicenseLen );
         ret += log.debugDump( title, m_oldUserLicense, 
                               m_oldUserLicenseLen );
      }
      if ( m_serverAuthBobSet ) {
         char title[16] = {0};
         snprintf( title, sizeof(title) - 1, "%08x :", 
                   ServerAuthBob | m_serverAuthBobLen );
         ret += log.debugDump( title, m_serverAuthBob, 
                               m_serverAuthBobLen );
      }
      if ( m_serverAuthBobChecksumSet ) {
         ret += log.debug("%08x : %08"PRIx32, 
                          (ServerAuthBobChecksum | 
                           sizeof( m_serverAuthBobChecksum ) ), 
                          m_serverAuthBobChecksum );
      }
      if ( m_centerPointAndScaleSet ) {
         char title[16] = {0};
         snprintf( title, sizeof(title) - 1, "%08x :", 
                   CenterPointAndScale | m_centerPointAndScaleLen );
         ret += log.debugDump( title, m_centerPointAndScale, 
                               m_centerPointAndScaleLen );
      }
      if(m_MSISDNNumberSet){
         ret += log.debug("%08x : %s\\0", 
                          MSISDNNumber | strlen(m_MSISDNNumber) + 1, 
                          m_MSISDNNumber);
      }
      if(m_ProgramVersionSet){
         ret += log.debug("%08x : (%08"PRIx32", %08"PRIx32", %08"PRIx32")", 
                   ProgramVersion | sizeof(m_ProgramVersion), 
                   m_ProgramVersion[0], m_ProgramVersion[1], 
                   m_ProgramVersion[2]);
      }
      if(m_newVersionSet){
         ret += log.debug("%08x : %s", 
                   NewVersion | strlen( m_newVersion ) +1, m_newVersion );
      }
      if(m_newVersionUrlSet){
         ret += log.debug("%08x : %s", 
                   NewVersionUrl | strlen( m_newVersionUrl ) +1, m_newVersionUrl );
      }
      if(m_ClientTypeSet){
         ret += log.debug("%08x : %s\\0", 
                          ClientType | strlen(m_ClientType) + 1, 
                          m_ClientType);
      }
      if(m_ClientTypeOptionsSet){
         ret += log.debug("%08x : %s\\0", 
                   ClientTypeOptions | strlen(m_ClientTypeOptions) + 1, 
                   m_ClientTypeOptions);
      }
      if(m_ResourcesVersionSet){
         ret += log.debug("%08x : (%08"PRIx32", %08"PRIx32", %08"PRIx32")", 
                   ResourcesVersion | sizeof(m_ResourcesVersion),
                   m_ResourcesVersion[0], m_ResourcesVersion[1],
                   m_ResourcesVersion[2]);
      }
      if(m_SendProgramUpdateSet){
         if(m_SendProgramUpdate){
            ret += log.debug("%08x : true", SendProgramUpdate);
         }
      }
      if(m_SendResourceUpdateSet){
         if(m_SendResourceUpdate){
            ret += log.debug("%08x : true", SendResourceUpdate);
         }
      }
      if(m_AlternativeServerSet){
         ret += log.debug("%08x : %s\\0", 
                   AlternativeServer | strlen(m_AlternativeServer) + 1, 
                   m_AlternativeServer);
      }
      if(m_NavIDSet){
         ret += log.debug("%08x : %08"PRIx32, 
                          NavID | sizeof(m_NavID), m_NavID);
      }
      if(m_CallCenterMSISDNSet){
         ret += log.debug("%08x : %s\\0", 
                   CallCenterMSISDN | strlen(m_CallCenterMSISDN) + 1, 
                   m_CallCenterMSISDN);
      }
      if(m_LanguageOfClientSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   LanguageOfClient | sizeof(m_LanguageOfClient), 
                   m_LanguageOfClient);
      }
      if(m_SubscriptionLeftSet){
         ret += log.debug("%08x : %04x", 
                   SubscriptionLeft | sizeof(m_SubscriptionLeft), 
                   m_SubscriptionLeft);
      }
      if(m_LatestNewsSet){
         ret += log.debug("%08x : %08"PRIx32,
                          LatestNews | sizeof(m_LatestNews), m_LatestNews);
      }
      if(m_CategoriesSet){
         ret += log.debug("%08x : %08"PRIx32,
                          Categories | sizeof(m_Categories), m_Categories);
      }
      if(m_TransactionsLeftSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   TransactionsLeft | sizeof(m_TransactionsLeft), 
                   m_TransactionsLeft);
      }
      if(m_CallCenterChecksumSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   CallCenterChecksum | sizeof(m_CallCenterChecksum), 
                   m_CallCenterChecksum);
      }
      if(m_WayfinderTypeSet){
         ret += log.debug("%08x : %02x", 
                          WayfinderType | sizeof(m_WayfinderType), 
                   m_WayfinderType);
      }
      if(m_NewPasswordSetSet){
         ret += log.debug("%08x : %s\\0", 
                   NewPasswordSet | strlen(m_NewPasswordSet) + 1, 
                   m_NewPasswordSet);
      }
      if(m_PublicUserNameSet){
         ret += log.debug("%08x : %s\\0", 
                   PublicUserName | strlen(m_PublicUserName) + 1, 
                   m_PublicUserName);
      }
      if(m_ServerListChecksumSet){
         ret += log.debug("%08x : %08"PRIx32, 
                   ServerListChecksum | sizeof(m_ServerListChecksum), 
                   m_ServerListChecksum);
      }
      if ( m_userRightsSet ) {
         char title[16] = {0};
         snprintf( title, sizeof(title) - 1, "%08x :", 
                   UserRights | m_userRightsLen );
         ret += log.debugDump(title, 
                              reinterpret_cast<const uint8*>(m_userRights), 
                              m_userRightsLen );
      }
      if ( m_latestNewsIdSet ) {
         ret += log.debug( "%08x : %s", 
                           LatestNewsId | strlen( m_latestNewsId ) +1, 
                           m_latestNewsId );
      }
      if ( m_favoritesCRCSet ) {
         ret += log.debug( "%08x : %s", 
                           FavoritesCRC | strlen( m_favoritesCRC ) +1, 
                           m_favoritesCRC );
      }
      if ( m_serverPtuiSet ) {
         ret += log.debug("%08x : %08"PRIx32, 
                          (ServerPtui | 
                           sizeof( m_serverPtui ) ), 
                          m_serverPtui );
      }
      if(m_DisableNoRegionCheckSet){
         if(m_DisableNoRegionCheck){
            ret += log.debug("%08x : true", DisableNoRegionCheck);
         }
      }
      if(m_ReflashMeSet){
         if(m_ReflashMe){
            ret += log.debug("%08x : true", ReflashMe);
         }
      }
      if(m_MaxPacketSizeSet){
         ret += log.debug("%08x : %04x", 
                   MaxPacketSize | sizeof(m_MaxPacketSize), m_MaxPacketSize);
      }
      return ret;
   }
#endif


#define M(arg) m_##arg
#define MSET(arg) m_##arg##Set
   //#define SUBPREFIX(prefix, arg) prefix##arg
#define PREFIX(prefix, arg) (prefix arg)
#define BOOLCMP(param, prefix)                                      \
   ((MSET(param) && PREFIX(prefix, MSET(param)) &&                  \
     (M(param) == PREFIX(prefix, M(param)))) ||                       \
    (!MSET(param) && !PREFIX(prefix, MSET(param))) ||               \
    (MSET(param) && !PREFIX(prefix, MSET(param)) && !M(param)) ||   \
    (!MSET(param) && PREFIX(prefix, MSET(param)) &&                 \
     !PREFIX(prefix, M(param))))                                    \

#ifdef __linux
# define CERR(cond, txt) if(cond){std::cerr << txt << std::endl;}
#else
# define CERR(cond, txt)
#endif

   bool ParameterBlock::operator==(const ParameterBlock& rhs) const
   {
      bool ret = BOOLCMP(AttemptReflash, rhs.);
      CERR(!ret, "Checkpoint " << __LINE__ << ": "<< m_AttemptReflash 
           << " != " << rhs.m_AttemptReflash);

      ret = ret && (m_KeepCallUp == rhs.m_KeepCallUp);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_KeepCallUpSet == rhs.m_KeepCallUpSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && BOOLCMP(KeepCallUp, rhs.);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && BOOLCMP(ImageCrossingMaps, rhs.);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret &&(m_CrossingMapsImageFormat == rhs.m_CrossingMapsImageFormat);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret &&
         (m_CrossingMapsImageFormatSet == rhs.m_CrossingMapsImageFormatSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_CrossingMapsImageSize == rhs.m_CrossingMapsImageSize) &&
         (m_CrossingMapsImageSizeSet == rhs.m_CrossingMapsImageSizeSet) &&
         (m_TopRegionChecksum == rhs.m_TopRegionChecksum) &&
         (m_serverAuthBobChecksum == rhs.m_serverAuthBobChecksum) &&
         (m_serverPtui == rhs.m_serverPtui);
      CERR(!ret, "Checkpoint "  << __LINE__);

      ret = ret && (m_TopRegionChecksumSet == rhs.m_TopRegionChecksumSet);

      ret = ret && (m_serverAuthBobChecksumSet == 
                    rhs.m_serverAuthBobChecksumSet);

      ret = ret && BOOLCMP(GetFullRouteGraph, rhs.);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_MaxSearchMatches == rhs.m_MaxSearchMatches);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret  && (m_MaxSearchMatchesSet == rhs.m_MaxSearchMatchesSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret  && (0 == strcmp(m_UserLogin, rhs.m_UserLogin));
      CERR(!ret, "Checkpoint " << __LINE__ << " : '" << m_UserLogin 
                    << "' != '" << rhs.m_UserLogin << "'");

      ret = ret && (m_UserLoginSet == rhs.m_UserLoginSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (0 == strcmp(m_UserPassword, rhs.m_UserPassword));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_UserPasswordSet == rhs.m_UserPasswordSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_UserLicenseSet == rhs.m_UserLicenseSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_UserLicenseLen == rhs.m_UserLicenseLen);
      CERR(!ret, "Checkpoint " << __LINE__ << " : " << m_UserLicenseLen 
                    << " != " << rhs.m_UserLicenseLen);

      ret = ret && 
         (0 == memcmp(m_UserLicense, rhs.m_UserLicense, m_UserLicenseLen));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_oldUserLicenseSet == rhs.m_oldUserLicenseSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_oldUserLicenseLen == rhs.m_oldUserLicenseLen);
      CERR(!ret, "Checkpoint " << __LINE__ << " : " << m_oldUserLicenseLen 
                    << " != " << rhs.m_oldUserLicenseLen);

      ret = ret && 
         (0 == memcmp( m_oldUserLicense, rhs.m_oldUserLicense, 
                       m_oldUserLicenseLen ));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_serverAuthBobSet == rhs.m_serverAuthBobSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_serverAuthBobLen == rhs.m_serverAuthBobLen);
      CERR(!ret, "Checkpoint " << __LINE__ << " : " << m_serverAuthBobLen 
                    << " != " << rhs.m_serverAuthBobLen);

      ret = ret && 
         (0 == memcmp( m_serverAuthBob, rhs.m_serverAuthBob, 
                       m_serverAuthBobLen ));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_userRightsSet == rhs.m_userRightsSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_userRightsLen == rhs.m_userRightsLen);
      CERR(!ret, "Checkpoint " << __LINE__ << " : " << m_userRightsLen
                    << " != " << rhs.m_userRightsLen);

      ret = ret && 
         (0 == memcmp( m_userRights, rhs.m_userRights, 
                       m_userRightsLen ));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret  && (0 == strcmp(m_MSISDNNumber, rhs.m_MSISDNNumber)) &&
         (m_MSISDNNumberSet == rhs.m_MSISDNNumberSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_ProgramVersionSet == rhs.m_ProgramVersionSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_newVersionSet == rhs.m_newVersionSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_newVersionUrlSet == rhs.m_newVersionUrlSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (0 == strcmp(m_ClientType, rhs.m_ClientType));
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret &&(m_ClientTypeSet == rhs.m_ClientTypeSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && 
         (0 == strcmp(m_ClientTypeOptions, rhs.m_ClientTypeOptions)) &&
         (m_ClientTypeOptionsSet == rhs.m_ClientTypeOptionsSet) &&
         (m_ResourcesVersionSet == rhs.m_ResourcesVersionSet) &&
         BOOLCMP(SendProgramUpdate, rhs.);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && BOOLCMP(SendResourceUpdate, rhs.) &&
         (0 == strcmp(m_AlternativeServer, rhs.m_AlternativeServer)) &&
         (m_AlternativeServerSet == rhs.m_AlternativeServerSet) &&
         (m_NavID == rhs.m_NavID);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_NavIDSet == rhs.m_NavIDSet) &&
         (0 == strcmp(m_CallCenterMSISDN, rhs.m_CallCenterMSISDN)) &&
         (m_CallCenterMSISDNSet == rhs.m_CallCenterMSISDNSet);
      CERR(!ret, "Checkpoint " << __LINE__);
      ret = ret && (m_LanguageOfClient == rhs.m_LanguageOfClient) &&
         (m_LanguageOfClientSet == rhs.m_LanguageOfClientSet);
      CERR(!ret, "Checkpoint " << __LINE__);
      ret = ret && (m_TransactionsLeft == rhs.m_TransactionsLeft) &&
         (m_TransactionsLeftSet == rhs.m_TransactionsLeftSet) &&
         (m_SubscriptionLeft == rhs.m_SubscriptionLeft);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_latestNewsIdSet == rhs.m_latestNewsIdSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_favoritesCRCSet == rhs.m_favoritesCRCSet);
      CERR(!ret, "Checkpoint " << __LINE__);

      ret = ret && (m_serverPtuiSet == 
                    rhs.m_serverPtuiSet);

      ret = ret && (m_SubscriptionLeftSet == rhs.m_SubscriptionLeftSet) &&
         (m_LatestNews == rhs.m_LatestNews) &&
         (m_LatestNewsSet == rhs.m_LatestNewsSet) &&
         (m_Categories == rhs.m_Categories) &&
         (m_CategoriesSet == rhs.m_CategoriesSet) &&
         (m_CallCenterChecksum == rhs.m_CallCenterChecksum) &&
         (m_CallCenterChecksumSet == rhs.m_CallCenterChecksumSet) &&
         BOOLCMP(ReflashMe, rhs.) && 
         (m_MaxPacketSize == rhs.m_MaxPacketSize) &&
         (m_MaxPacketSizeSet == rhs.m_MaxPacketSizeSet);
      return ret;
   }
   
   
   /*uint32 ParameterBlock::getCategories(bool* isSet)
   {
      if(isSet) *isSet = m_CategoriesSet;
      return m_Categories;
   }

   void ParameterBlock::setCategories(uint32 val)
   {
      m_CategoriesSet = true;
      m_Categories = val;
      encode();
   }*/
   
   
   
   
   
   
   
   
   
      bool ParameterBlock::getAttemptReflash(bool* isSet)
      {
         if(isSet) *isSet = m_AttemptReflashSet;
         return m_AttemptReflash;
      }
 
      void ParameterBlock::setAttemptReflash(bool val)
      {
         m_AttemptReflashSet = true;
         m_AttemptReflash = val;
         encode();
      }

      uint8 ParameterBlock::getKeepCallUp(bool* isSet)
      {
         if(isSet) *isSet = m_KeepCallUpSet;
         return m_KeepCallUp;
      }

      void ParameterBlock::setKeepCallUp(uint8 val)
      {
         m_KeepCallUpSet = true;
         m_KeepCallUp = val;
         encode();
      }

      bool ParameterBlock::getAttachCrossingMaps(bool* isSet)
      {
         if(isSet) *isSet = m_AttachCrossingMapsSet;
         return m_AttachCrossingMaps;
      }

      void ParameterBlock::setAttachCrossingMaps(bool val)
      {
         m_AttachCrossingMapsSet = true;
         m_AttachCrossingMaps = val;
         encode();
      }

      bool ParameterBlock::getImageCrossingMaps(bool* isSet)
      {
         if(isSet) *isSet = m_ImageCrossingMapsSet;
         return m_ImageCrossingMaps;
      }

      void ParameterBlock::setImageCrossingMaps(bool val)
      {
         m_ImageCrossingMapsSet = true;
         m_ImageCrossingMaps = val;
         encode();
      }

      uint8 ParameterBlock::getCrossingMapsImageFormat(bool* isSet)
      {
         if(isSet) *isSet = m_CrossingMapsImageFormatSet;
         return m_CrossingMapsImageFormat;
      }

      void ParameterBlock::setCrossingMapsImageFormat(uint8 val)
      {
         m_CrossingMapsImageFormatSet = true;
         m_CrossingMapsImageFormat = val;
         encode();
      }

      uint32 ParameterBlock::getCrossingMapsImageSize(bool* isSet)
      {
         if(isSet) *isSet = m_CrossingMapsImageSizeSet;
         return m_CrossingMapsImageSize;
      }

      void ParameterBlock::setCrossingMapsImageSize(uint32 val)
      {
         m_CrossingMapsImageSizeSet = true;
         m_CrossingMapsImageSize = val;
         encode();
      }

      uint32 ParameterBlock::getTopRegionChecksum(bool* isSet)
      {
         if(isSet) *isSet = m_TopRegionChecksumSet;
         return m_TopRegionChecksum;
      }

      void ParameterBlock::setTopRegionChecksum(uint32 val)
      {
         m_TopRegionChecksumSet = true;
         m_TopRegionChecksum = val;
         encode();
      }

      uint8 ParameterBlock::getGetFullRouteGraph(bool* isSet)
      {
         if(isSet) *isSet = m_GetFullRouteGraphSet;
         return m_GetFullRouteGraph;
      }

      void ParameterBlock::setGetFullRouteGraph(uint8 val)
      {
         m_GetFullRouteGraphSet = true;
         m_GetFullRouteGraph = val;
         encode();
      }

      uint8 ParameterBlock::getMaxSearchMatches(bool* isSet)
      {
         if(isSet) *isSet = m_MaxSearchMatchesSet;
         return m_MaxSearchMatches;
      }

      void ParameterBlock::setMaxSearchMatches(uint8 val)
      {
         m_MaxSearchMatchesSet = true;
         m_MaxSearchMatches = val;
         encode();
      }

      const char* ParameterBlock::getUserLogin(bool* isSet)
      {
         if(isSet) *isSet = m_UserLoginSet;
         return m_UserLogin;
      }

      void ParameterBlock::setUserLogin(const char* val)
      {
         m_UserLoginSet = val ? true : false;
         delete[] m_UserLogin;
         m_UserLogin = strdup_new(val);
         encode();
      }

      const char* ParameterBlock::getUserPassword(bool* isSet)
      {
         if(isSet) *isSet = m_UserPasswordSet;
         return m_UserPassword;
      }

      void ParameterBlock::setUserPassword(const char* val)
      {
         m_UserPasswordSet = val ? true : false;
         delete[] m_UserPassword;
         m_UserPassword = strdup_new(val);
         encode();
      }

      const uint8* ParameterBlock::getUserLicense( int& len, bool* isSet)
      {
         if(isSet) *isSet = m_UserLicenseSet;
         len = m_UserLicenseLen;
         return m_UserLicense;
      }

      void ParameterBlock::setUserLicense(const uint8* val, int len)
      {
         delete[] m_UserLicense;
         m_UserLicense = NULL;
         m_UserLicenseSet = false;
         m_UserLicenseLen = 0;
         if(val && len > 0){
            m_UserLicenseSet = true;
            m_UserLicenseLen = len;
            m_UserLicense = new uint8[len];
            for(int i = 0; i < len; ++i){
               m_UserLicense[i] = val[i];
            }
         }
         encode();
      }

      const uint8* ParameterBlock::getOldUserLicense( int& len, bool* isSet ) {
         if ( isSet ) *isSet = m_oldUserLicenseSet;
         len = m_oldUserLicenseLen;
         return m_oldUserLicense;
      }
 
      void ParameterBlock::setOldUserLicense( const uint8* val, int len ) {
         delete [] m_oldUserLicense;
         m_oldUserLicense = NULL;
         m_oldUserLicenseSet = false;
         m_oldUserLicenseLen = 0;
         if ( val != NULL && len > 0 ) {
            m_oldUserLicenseSet = true;
            m_oldUserLicenseLen = len;
            m_oldUserLicense = new uint8[ len ];
            for ( int i = 0 ; i < len ; ++i ) {
               m_oldUserLicense[ i ] = val[ i ];
            }
         }
         encode();
      }

      const char* ParameterBlock::getMSISDNNumber(bool* isSet)
      {
         if(isSet) *isSet = m_MSISDNNumberSet;
         return m_MSISDNNumber;
      }
 
      void ParameterBlock::setMSISDNNumber(const char* val)
      {
         m_MSISDNNumberSet = val ? true : false;
         delete[] m_MSISDNNumber;
         m_MSISDNNumber = strdup_new(val);
         encode();
      }

      const uint32* ParameterBlock::getProgramVersion(bool* isSet)
      {
         if(isSet) *isSet = m_ProgramVersionSet;
         return m_ProgramVersion;
      }

      void ParameterBlock::setProgramVersion(const uint32* val)
      {
         m_ProgramVersionSet = val ? true : false;
         for(unsigned i = 0; 
             i < sizeof(m_ProgramVersion)/sizeof(*m_ProgramVersion); ++i){
            m_ProgramVersion[i] = val ? val[i] : 0;
         }
         encode();
      }

      const char* ParameterBlock::getNewVersion(bool* isSet)
      {
         if(isSet) *isSet = m_newVersionSet;
         return m_newVersion;
      }

      void ParameterBlock::setNewVersion(const char* val)
      {
         m_newVersionSet = val ? true : false;
         delete[] m_newVersion;
         m_newVersion = strdup_new( val );
         encode();
      }

      // this is the URL to use to load the new version
      const char* ParameterBlock::getNewVersionUrl(bool* isSet)
      {
         if(isSet) *isSet = m_newVersionUrlSet;
         return m_newVersionUrl;
      }

      void ParameterBlock::setNewVersionUrl(const char* val)
      {
         m_newVersionUrlSet = val ? true : false;
         delete[] m_newVersionUrl;
         m_newVersionUrl = strdup_new( val );
         encode();
      }

      const char* ParameterBlock::getClientType(bool* isSet)
      {
         if(isSet) *isSet = m_ClientTypeSet;
         return m_ClientType;
      }

      void ParameterBlock::setClientType(const char*  val)
      {
         m_ClientTypeSet = val ? true : false;
         delete[] m_ClientType;
         m_ClientType = strdup_new(val);
         encode();
      }

      const char*  ParameterBlock::getClientTypeOptions(bool* isSet)
      {
         if(isSet) *isSet = m_ClientTypeOptionsSet;
         return m_ClientTypeOptions;
      }

      void ParameterBlock::setClientTypeOptions(const char* val)
      {
         m_ClientTypeOptionsSet = val ? true : false;
         delete[] m_ClientTypeOptions;
         m_ClientTypeOptions = strdup_new(val);
         encode();
      }

      const uint32* ParameterBlock::getResourcesVersion(bool* isSet)
      {
         if(isSet) *isSet = m_ResourcesVersionSet;
         return m_ResourcesVersion;
      }

      void ParameterBlock::setResourcesVersion(const uint32* val)
      {
         m_ResourcesVersionSet = val ? true : false;
         for(unsigned i = 0; 
             i < sizeof(m_ResourcesVersion)/sizeof(*m_ResourcesVersion); ++i){
            m_ResourcesVersion[i] = val ? val[i] : 0;
         }
         encode();
      }

      bool ParameterBlock::getSendProgramUpdate(bool* isSet)
      {
         if(isSet) *isSet = m_SendProgramUpdateSet;
         return m_SendProgramUpdate;
      }

      void ParameterBlock::setSendProgramUpdate(bool val)
      {
         m_SendProgramUpdateSet = true;
         m_SendProgramUpdate = val;
         encode();
      }

      bool ParameterBlock::getSendResourceUpdate(bool* isSet)
      {
         if(isSet) *isSet = m_SendResourceUpdateSet;
         return m_SendResourceUpdate;
      }

      void ParameterBlock::setSendResourceUpdate(bool val)
      {
         m_SendResourceUpdateSet = true;
         m_SendResourceUpdate = val;
         encode();
      }

      const char* ParameterBlock::getAlternativeServer(bool* isSet)
      {
         if(isSet) *isSet = m_AlternativeServerSet;
         return m_AlternativeServer;
      }

      void ParameterBlock::setAlternativeServer(const char* val)
      {
         m_AlternativeServerSet = val ? true : false;
         delete[] m_AlternativeServer;
         m_AlternativeServer = strdup_new(val);
         encode();
      }

      uint32 ParameterBlock::getNavID(bool* isSet)
      {
         if(isSet) *isSet = m_NavIDSet;
         return m_NavID;
      }

      void ParameterBlock::setNavID(uint32 val)
      {
         m_NavIDSet = true;
         m_NavID = val;
         encode();
      }

      const char* ParameterBlock::getCallCenterMSISDN(bool* isSet)
      {
         if(isSet) *isSet = m_CallCenterMSISDNSet;
         return m_CallCenterMSISDN;
      }

      void ParameterBlock::setCallCenterMSISDN(const char* val)
      {
         m_CallCenterMSISDNSet = val ? true : false;
         delete[] m_CallCenterMSISDN;
         m_CallCenterMSISDN = strdup_new(val);
         encode();
      }

      uint32 ParameterBlock::getLanguageOfClient(bool* isSet)
      {
         if(isSet) *isSet = m_LanguageOfClientSet;
         return m_LanguageOfClient;
      }
 
      void ParameterBlock::setLanguageOfClient(uint32 val)
      {
         m_LanguageOfClientSet = true;
         m_LanguageOfClient = val;
         encode();
      }

      uint32 ParameterBlock::getTransactionsLeft(bool* isSet)
      {
         if(isSet) *isSet = m_TransactionsLeftSet;
         return m_TransactionsLeft;
      }
 
      void ParameterBlock::setTransactionsLeft(uint32 val)
      {
         m_TransactionsLeftSet = true;
         m_TransactionsLeft = val;
         encode();
      }

      uint16 ParameterBlock::getSubscriptionLeft(bool* isSet)
      {
         if(isSet) *isSet = m_SubscriptionLeftSet;
         return m_SubscriptionLeft;
      }

      void ParameterBlock::setSubscriptionLeft(uint16 val)
      {
         m_SubscriptionLeftSet = true;
         m_SubscriptionLeft = val;
         encode();
      }

      uint32 ParameterBlock::getLatestNews(bool* isSet)
      {
         if(isSet) *isSet = m_LatestNewsSet;
         return m_LatestNews;
      }
 
      void ParameterBlock::setLatestNews(uint32 val)
      {
         m_LatestNewsSet = true;
         m_LatestNews = val;
         encode();
      }

      uint32 ParameterBlock::getCategories(bool* isSet)
      {
         if(isSet) *isSet = m_CategoriesSet;
         return m_Categories;
      }

      void ParameterBlock::setCategories(uint32 val)
      {
         m_CategoriesSet = true;
         m_Categories = val;
         encode();
      }

      uint32 ParameterBlock::getCallCenterChecksum(bool* isSet)
      {
         if(isSet) *isSet = m_CallCenterChecksumSet;
         return m_CallCenterChecksum;
      }
 
      void ParameterBlock::setCallCenterChecksum(uint32 val)
      {
         m_CallCenterChecksumSet = true;
         m_CallCenterChecksum = val;
         encode();
      }

      uint8 ParameterBlock::getWayfinderType(bool* isSet)
      {
         if(isSet) *isSet = m_WayfinderTypeSet;
         return m_WayfinderType;
      }

      void ParameterBlock::setWayfinderType(uint8 val)
      {
         m_WayfinderTypeSet = true;
         m_WayfinderType = val;
         encode();
      }

      char* ParameterBlock::getNewPasswordSet(bool* isSet)
      {
         if(isSet) *isSet = m_NewPasswordSetSet;
         return m_NewPasswordSet;
      }

      void ParameterBlock::setNewPasswordSet(const char* val)
      {
         m_NewPasswordSetSet = val ? true : false;
         delete[] m_NewPasswordSet;
         m_NewPasswordSet = strdup_new(val);
         encode();
      }

      char* ParameterBlock::getPublicUserName(bool* isSet)
      {
         if(isSet) *isSet = m_PublicUserNameSet;
         return m_PublicUserName;
      }

      void ParameterBlock::setPublicUserName(const char* val)
      {
         m_PublicUserNameSet = val ? true : false;
         delete[] m_PublicUserName;
         m_PublicUserName = strdup_new(val);
         encode();
      }

      uint32 ParameterBlock::getServerListChecksum(bool* isSet)
      {
         if(isSet) *isSet = m_ServerListChecksumSet;
         return m_ServerListChecksum;
      }

      void ParameterBlock::setServerListChecksum(uint32 val)
      {
         m_ServerListChecksumSet = true;
         m_ServerListChecksum = val;
         encode();
      }

      bool ParameterBlock::getDisableNoRegionCheck(bool* isSet)
      {
         if(isSet) *isSet = m_DisableNoRegionCheckSet;
         return m_DisableNoRegionCheck;
      }

      void ParameterBlock::setDisableNoRegionCheck(bool val)
      {
         m_DisableNoRegionCheckSet = true;
         m_DisableNoRegionCheck = val;
         encode();
      }

      bool ParameterBlock::getReflashMe(bool* isSet)
      {
         if(isSet) *isSet = m_ReflashMeSet;
         return m_ReflashMe;
      }

      void ParameterBlock::setReflashMe(bool val)
      {
         m_ReflashMeSet = true;
         m_ReflashMe = val;
         encode();
      }

      uint16 ParameterBlock::getMaxPacketSize(bool* isSet)
      {
         if(isSet) *isSet = m_MaxPacketSizeSet;
         return m_MaxPacketSize;
      }

      void ParameterBlock::setMaxPacketSize(uint16 val)
      {
         m_MaxPacketSizeSet = true;
         m_MaxPacketSize = val;
         encode();
      }

      const uint8* ParameterBlock::getServerAuthBob( int& len, bool* isSet ) {
         if ( isSet ) *isSet = m_serverAuthBobSet;
         len = m_serverAuthBobLen;
         return m_serverAuthBob;
      }

      void ParameterBlock::setServerAuthBob( const uint8* val, int len ) {
         delete [] m_serverAuthBob;
         m_serverAuthBob = NULL;
         m_serverAuthBobSet = false;
         m_serverAuthBobLen = 0;
         if ( val != NULL && len > 0 ) {
            m_serverAuthBobSet = true;
            m_serverAuthBobLen = len;
            m_serverAuthBob = new uint8[ len ];
            for ( int i = 0 ; i < len ; ++i ) {
               m_serverAuthBob[ i ] = val[ i ];
            }
         }
         encode();
      }


      uint32 ParameterBlock::getServerAuthBobChecksum(bool* isSet)
      {
         if(isSet) *isSet = m_serverAuthBobChecksumSet;
         return m_serverAuthBobChecksum;
      }

      void ParameterBlock::setServerAuthBobChecksum(uint32 val)
      {
         m_serverAuthBobChecksumSet = true;
         m_serverAuthBobChecksum = val;
         encode();
      }

      const uint8* ParameterBlock::getCenterPointAndScale( int& len, bool* isSet ) {
         if ( isSet ) *isSet = m_centerPointAndScaleSet;
         len = m_centerPointAndScaleLen;
         return m_centerPointAndScale;
      }

      void ParameterBlock::setCenterPointAndScale( const uint8* val, int len ) {
         delete [] m_centerPointAndScale;
         m_centerPointAndScale = NULL;
         m_centerPointAndScaleSet = false;
         m_centerPointAndScaleLen = 0;
         if ( val != NULL && len > 0 ) {
            m_centerPointAndScaleSet = true;
            m_centerPointAndScaleLen = len;
            m_centerPointAndScale = new uint8[ len ];
            for ( int i = 0 ; i < len ; ++i ) {
               m_centerPointAndScale[ i ] = val[ i ];
            }
         }
         encode();
      }

      const uint32* ParameterBlock::getUserRights( int& len, bool* isSet )
      {
         if(isSet) *isSet = m_userRightsSet;
         len = m_userRightsLen;
         return m_userRights;
      }

      void ParameterBlock::setUserRights( const uint32* val, int len )
      {
         delete [] m_userRights;
         m_userRights = NULL;
         m_userRightsSet = false;
         m_userRightsLen = 0;
         if ( val != NULL && len > 0 ) {
            m_userRightsSet = true;
            m_userRightsLen = len;
            m_userRights = new uint32[ len ];
            for ( int i = 0 ; i < len ; ++i ) {
               m_userRights[ i ] = val[ i ];
            }
         }
         encode();
      }

      const char* ParameterBlock::getLatestNewsId( bool* isSet )
      {
         if ( isSet ) *isSet = m_latestNewsIdSet;
         return m_latestNewsId;
      }

      void ParameterBlock::setLatestNewsId( const char* val ) {
         m_latestNewsIdSet = val ? true : false;
         delete[] m_latestNewsId;
         m_latestNewsId = strdup_new( val );
         encode();
      }

      const char* ParameterBlock::getFavoritesCRC( bool* isSet )
      {
         if ( isSet ) *isSet = m_favoritesCRCSet;
         return m_favoritesCRC;
      }

      void ParameterBlock::setFavoritesCRC( const char* val ) {
         m_favoritesCRCSet = val ? true : false;
         delete[] m_favoritesCRC;
         m_favoritesCRC = strdup_new( val );
         encode();
      }

      uint32 ParameterBlock::getServerPtui(bool* isSet)
      {
         if(isSet) *isSet = m_serverPtuiSet;
         return m_serverPtui;
      }

      void ParameterBlock::setServerPtui(uint32 val)
      {
         m_serverPtuiSet = true;
         m_serverPtui = val;
         encode();
      }

   
}
