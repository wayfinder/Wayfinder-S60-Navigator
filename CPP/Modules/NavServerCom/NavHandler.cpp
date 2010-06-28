/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavHandler.h"
#include "NavPacket.h"
#include "LogMacros.h"
#include "NavServerComEnums.h"
#include "Buffer.h"
#include "NParamBlock.h"
#include "ParameterBlock.h"
#include "NavPacket.h"
#include "Tracking.h"
#include "TraceMacros.h"
#include "TimeUtils.h"
#include "NavRequestType.h"
#include "WFID.h"

using namespace std;

namespace isab{

using namespace NavServerComEnums;

#define SERVER_INFO_TIMEOUT 30*60


NavHandler::NavHandler( uint8 protoVer )
      : m_crcmap( makeCRCMap() ),
        m_userLogin( NULL ), m_userPassword( NULL ), m_userLicense( NULL ),
        m_clientType( NULL ), m_clientTypeOptions( NULL ), 
        m_languageOfClient( 0 ), m_wayfinderType( 0 ), 
        m_sessionKey( NULL ), m_lastServerInfo( TimeUtils::time(NULL) ),
        m_lastParams( new ParameterBlock() ),
        m_log( new Log( "NavHandler" ) ),
        m_secondaryServer( false ),
        m_protoVer( protoVer ), m_tracking( NULL ),
        m_licenseKeyType( NULL )
{
   m_programVersion[ 0 ] = 0;
   m_programVersion[ 1 ] = 0;
   m_programVersion[ 2 ] = 0;
}


NavHandler::~NavHandler() {
   delete [] m_crcmap;
   delete [] m_userLogin;
   delete [] m_userPassword;
   delete [] m_userLicense;
   delete [] m_clientType;
   delete [] m_clientTypeOptions;
   delete [] m_sessionKey;
   delete [] m_licenseKeyType;
   delete m_lastParams;
   delete m_log;
   for (std::vector<isab::HWID*>::iterator it = m_vhardwareIDs.begin();
        it != m_vhardwareIDs.end(); it++) {
      delete *it;
   }
   m_vhardwareIDs.clear();
}


NavReplyPacket*
NavHandler::handleReply( uint8 protoVer, uint16 type, uint8 reqID, 
                         uint8 reqVer, uint8 statusCode, 
                         const char* statusMessage, 
                         const uint8* buff, uint32 buffLen )
{

   // The request packet
   DBG( "NavHandler::handleReply" );
   NavReplyPacket* pack = new NavReplyPacket( 
      protoVer, type, reqID, reqVer, statusCode, statusMessage, 
      buff, buffLen );

   DBG( "NavHandler::handleReply NavReplyPacket constructed" );

   // Debug print all params
//   pack->getParamBlock().dump( m_log, false );
   
   return pack;
}


static bool authParams( NParamBlock& params, ParameterBlock* paramBlock )
{
   bool any = false;

   if ( (params.getParam( 1 ) || params.getParam( 8 ) || 
         params.getParam( 9 )) && params.getParam( 2 ) )
   {
      // 1 User ID
      if ( params.getParam( 1 ) ) {
         paramBlock->setUserLogin( params.getParam( 1 )->getString() );
      }
      // 2 User Password
      if ( params.getParam( 2 ) ) {
         paramBlock->setUserPassword( params.getParam( 2 )->getString() );
      }
      // 8 User Login
      if ( params.getParam( 8 ) ) {
         paramBlock->setUserLogin( params.getParam( 8 )->getString() );
         paramBlock->setPublicUserName( params.getParam( 8 )->getString());
      }
      // 9 User UIN
      if ( params.getParam( 9 ) ) {
         paramBlock->setUserLogin( params.getParam( 9 )->getString() );
      }

      any =  true;
   }

   return any;
}

void handleSearchParams( Buffer& buff, NParamBlock& params ) {
   // Search list type  1  
   uint8 searchListType = 0;
   uint16 nbrMatches = 0;
   uint16 totalNbrMatches = 0;
   uint16 startIndex = 0;
   Buffer* matchData = NULL;
   Buffer* regionData = NULL;
   uint16 nbrRegions = 0;
   if ( params.getParam( 1302 ) ) {
//      DBG( "1302" );
      // Region data
      regionData = new Buffer( 
         params.getParam( 1302 )->getLength()*2 );
      uint16 pos = 0;
      if ( regionData != NULL ) {
         while ( pos < params.getParam( 1302 )->getLength() ) {
//            DBG( "1302 pos %u len %u", pos, 
//                 params.getParam( 1302 )->getLength() );
            // Type
            regionData->writeNextUnaligned32bit( 
               params.getParam( 1302 )->getUint16( 0 ) );
            pos += 2;
            // ID str
            regionData->writeNextCharString( 
               params.getParam( 1302 )->getString( pos ) );
            pos += strlen( 
               params.getParam( 1302 )->getString( pos ) ) + 1;
            // Name str
            regionData->writeNextCharString( 
               params.getParam( 1302 )->getString( pos ) );
            pos += strlen( 
               params.getParam( 1302 )->getString( pos ) ) + 1;
            nbrRegions++; 
         }
      }
   }

   bool addItems = true;
   if ( params.getParam( 1300 ) ) {
//      DBG( "1300" );
      searchListType = 1;
      startIndex = params.getParam( 1300 )->getUint16Array( 0 );
      totalNbrMatches = params.getParam( 1300 )->getUint16Array( 1 );
      if ( totalNbrMatches == 1 ) {
         // Unique area hit send Item matches
      } else if ( params.getParam( 1303 ) && regionData != NULL ) {
         addItems = false;
//         DBG( "1303" );
         matchData = new Buffer( 
            params.getParam( 1303 )->getLength()*2 );
         // Read area matches and put into matchData
         if ( matchData != NULL ) {
            // XXX: ADD REGON NAME(S) AS ', Lund' to match name(V8 likes that)
            vector< const char* > rNs;
            for ( uint32 i = 0 ; i < nbrRegions ; ++i ) {
               // Type
               regionData->readNextUnaligned32bit();
               // ID
               regionData->getNextCharString();
               // Name
               rNs.push_back( regionData->getNextCharString() );
            }
            uint16 pos = 0;
            while ( pos < params.getParam( 1303 )->getLength() ) {
//               DBG( "1303 pos %u len %u", pos, 
//                    params.getParam( 1303 )->getLength() );
               uint32 type = params.getParam( 1303 )->getUint16( 0 );
               pos += 2;
               // ID str
               matchData->writeNextCharString( 
                  params.getParam( 1303 )->getString( pos ) );
               pos += strlen( 
                  params.getParam( 1303 )->getString( pos ) ) + 1;
               // Name str
               const char* name = params.getParam( 1303 )->getString( pos );
               pos += strlen( 
                  params.getParam( 1303 )->getString( pos ) ) + 1;
               // Nbr regions 1
               uint8 nbrRegions = *(params.getParam( 1303 )->getBuff() +
                                    pos);
               pos++;
               // Location type 4
               // RegionIDs 
               uint16 firstRegion = 0;
               if ( nbrRegions > 0 ) {
                  firstRegion = params.getParam( 1303 )->getUint16( pos );
               }
               for ( uint8 i = 0 ; i < nbrRegions ; ++i ) {
                  pos += 2;
               }

               // Make up name
               uint32 size = strlen( name ) + 1;
               if ( nbrRegions > 0 ) {
                  size += strlen( rNs[ firstRegion ] ) + 3;
               }
               char* fname = new char[ size ];
               if ( fname != NULL ) {
                  strcpy( fname, name );
                  if ( nbrRegions > 0 ) {
                     strcat( fname, ", " );
                     strcat( fname, rNs[ firstRegion ] );
                  }
                  // Name
                  matchData->writeNextCharString( fname );
               } else {
                  // Name
                  matchData->writeNextCharString( name );
               }

               // Nbr regions 1
               matchData->writeNext8bit( /*nbrRegions No regions*/0 );
               // Location type 4
               matchData->writeNextUnaligned32bit( type );

               nbrMatches++;

               delete [] fname;
            }
         }

         // No regions needed
         nbrRegions = 0;
         delete regionData;
         regionData = NULL;
      }
   }

   if ( addItems && params.getParam( 1301 ) ) {
//      DBG( "1301" );
      searchListType = 0;
      startIndex = params.getParam( 1301 )->getUint16Array( 0 );
      totalNbrMatches = params.getParam( 1301 )->getUint16Array( 1 );
      if ( params.getParam( 1304 ) ) {
//         DBG( "1304" );
         matchData = new Buffer( 
            params.getParam( 1304 )->getLength()*2 );
         if ( matchData != NULL ) {
            // Read item matches and put into matchData
            uint16 pos = 0;
            while ( pos < params.getParam( 1304 )->getLength() ) {
//               DBG( "1304 pos %u len %u", pos, 
//                    params.getParam( 1304 )->getLength() );
               uint16 type = params.getParam( 1304 )->getByte( pos );
               pos++;
               uint16 subType = params.getParam( 1304 )->getByte( pos );
               pos++;
               // ID str
               const char* id = params.getParam( 1304 )->getString( 
                  pos );
               pos += strlen( 
                  params.getParam( 1304 )->getString( pos ) ) + 1;
               matchData->writeNextCharString( id );
               // Type 2
               matchData->writeNextUnaligned16bit( type );
               // SubType 2
               matchData->writeNextUnaligned16bit( subType );
               const char* name = params.getParam( 1304)->getString(
                  pos );
               pos += strlen( 
                  params.getParam( 1304 )->getString( pos ) ) + 1;
               // lat
               matchData->writeNextUnaligned32bit( 
                  params.getParam( 1304 )->getInt32( pos ) );
               pos += 4;
               // lon
               matchData->writeNextUnaligned32bit( 
                  params.getParam( 1304 )->getInt32( pos ) );
               pos += 4;
               // alt 4 XXX: Hardcoded to 0 here
               matchData->writeNextUnaligned32bit( 0 );
               // Nbr regions 1
               uint8 nbrRegions = *(params.getParam( 1304 )->getBuff() +
                                    pos);
               matchData->writeNext8bit( nbrRegions );
               pos++;
               // Name str
               matchData->writeNextCharString( name );
               // 20031015 client does ','-separatin for all types 
               //          not just POI, remove all ','s if not POI
               if ( type != 0x02 /*poi*/ ) {
                  uint32 size = strlen( name ) + 1;
                  uint8* data = const_cast<uint8*>( 
                     matchData->accessRawData( 
                        matchData->getWritePos() - size ) );
                  for ( uint32 i = 0 ; i < size ; ++i ) {
                     if ( data[ i ] == ',' ) {
                        data[ i ] = ' ';
                     }
                  }
               }
               // RegionIDs 
               for ( uint8 i = 0 ; i < nbrRegions ; ++i ) {
                  matchData->writeNextUnaligned16bit( 
                     params.getParam( 1304 )->getUint16( pos ) );
                  pos += 2;
               }
               nbrMatches++;
            }
         }
      }
   }
//   DBG( "Writing data" );
   buff.writeNext8bit( searchListType );
   // nbr matches  2  Number of matches below.
   buff.writeNextUnaligned16bit( nbrMatches );
   // Matches starting index  2
   buff.writeNextUnaligned16bit( startIndex );
   // Total matches  2  
   buff.writeNextUnaligned16bit( totalNbrMatches );
   // Number regions  2  
   buff.writeNextUnaligned16bit( nbrRegions );
   // Regions X
   if ( regionData ) {
      buff.writeNextByteArray( regionData->accessRawData( 0 ), 
                               regionData->getLength() );
   }
   // Match   Y
   if ( matchData ) {
      buff.writeNextByteArray( matchData->accessRawData( 0 ), 
                               matchData->getLength() );
   }

   delete matchData;
   delete regionData;
}

void handlRouteParams( class Buffer& buff, class NParamBlock& params ) 
{
   // Align short
   size_t apos = buff.getWritePos();
   Buffer::align16bit( apos );
   buff.setWritePos( apos );
   // Route id
   unsigned int routeID = 0;
   unsigned int routeCTime = 0;
   if ( params.getParam( 1100 ) ) { 
      sscanf( params.getParam( 1100 )->getString(), "%X_%X", 
              &routeID, &routeCTime );
//       DBG( "XXX handlRouteParams routeID %s = %X_%X", 
//            params.getParam( 1100 )->getString(), routeID, routeCTime );
   }
   buff.writeNextUnaligned64bit( MakeInt64( routeID, routeCTime ) );
   // Route bbox 
   if ( params.getParam( 1106 ) ) {
      buff.writeNextUnaligned32bit( 
         params.getParam( 1106 )->getInt32Array( 0 ) );
      buff.writeNextUnaligned32bit(
         params.getParam( 1106 )->getInt32Array( 1 ) );
      buff.writeNextUnaligned32bit( 
         params.getParam( 1106 )->getInt32Array( 2 ) );
      buff.writeNextUnaligned32bit( 
         params.getParam( 1106 )->getInt32Array( 3 ) );
   } else {
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
   }
   // Truncated dist
   if ( params.getParam( 1101 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 1101 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
   // dist2nextWPTFromTrunk
   if ( params.getParam( 1102 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 1102 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
   // phoneHomeDist
   if ( params.getParam( 1103 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 1103 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }


   // Align short
   apos = buff.getWritePos();
   Buffer::align16bit( apos );
   buff.setWritePos( apos );

   // Stringtable
   const NParam* ps = params.getParam( 1104 );
   // Write the length of the string table.
   if ( ps ) {
      buff.writeNextUnaligned16bit( ps->getLength() );
   } else {
      buff.writeNextUnaligned16bit( 0 );
   }

   // Color table
   const NParam* pc = params.getParam( 1108 );
   // Write the color table before the string table
   if( pc ){
      buff.writeNextUnaligned16bit( pc->getLength() );
      buff.writeNextByteArray( pc->getBuff(), pc->getLength() );
   } else {
      buff.writeNextUnaligned16bit( 0 );
   }

   if ( ps ) {
      //Write the string table
      // Align short
      apos = buff.getWritePos();
      Buffer::align16bit( apos );
      buff.setWritePos( apos );
      uint16 pos = 0;

      while ( pos < ps->getLength() ) {
         buff.writeNextCharString( ps->getString( pos ) );
         pos += strlen( ps->getString( pos ) ) + 1;
      }
   }
   
   // Align short
   apos = buff.getWritePos();
   Buffer::align16bit( apos );
   buff.setWritePos( apos );
   // Route data
   vector< const NParam* > pr;
   params.getAllParams( 1105, pr );
   for ( uint32 i = 0 ; i < pr.size() ; ++i ) {
      buff.writeNextByteArray( pr[ i ]->getBuff(), pr[ i ]->getLength() );
   }
}


void handleGpsAddressParams( Buffer& /*buff*/, NParamBlock& /*params*/ )
{
   // XXX: Fill in here
}


void handleMapParams( Buffer& buff, NParamBlock& params )
{
   const NParam* pbbox = params.getParam( 1700 );
   if ( pbbox ) {
      buff.writeNextUnaligned32bit( pbbox->getInt32Array( 0 ) );
      buff.writeNextUnaligned32bit( pbbox->getInt32Array( 1 ) );
      buff.writeNextUnaligned32bit( pbbox->getInt32Array( 2 ) );
      buff.writeNextUnaligned32bit( pbbox->getInt32Array( 3 ) );
   } else{
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
      buff.writeNextUnaligned32bit( MAX_INT32 );
   }
   if ( params.getParam( 1701 ) ) {
      buff.writeNextUnaligned16bit( 
         params.getParam( 1701 )->getUint16Array( 0 ) );
      buff.writeNextUnaligned16bit( 
         params.getParam( 1701 )->getUint16Array( 1 ) );
   } else {
      buff.writeNextUnaligned16bit( 0 );
      buff.writeNextUnaligned16bit( 0 );
   }
   if ( params.getParam( 1702 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 1702 )->getUint32() );
      buff.writeNextUnaligned32bit( 
         params.getParam( 1702 )->getUint32Array( 1 ) );
   } else {
      buff.writeNextUnaligned32bit( 0 );
      buff.writeNextUnaligned32bit( 0 );
   }
   if ( params.getParam( 1704 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 1704 )->getLength() );
      if ( params.getParam( 1703 ) ) {
         buff.writeNextUnaligned16bit( params.getParam( 1703 )->getByte());
      } else {
         buff.writeNextUnaligned16bit( 0 );
      }
      buff.writeNextByteArray( params.getParam( 1704 )->getBuff(),
                               params.getParam( 1704 )->getLength() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
      if ( params.getParam( 1703 ) ) {
         buff.writeNextUnaligned16bit( params.getParam( 1703 )->getByte());
      } else {
         buff.writeNextUnaligned16bit( 0 );
      }
   }
}


void handleInfoParams( Buffer& buff, NParamBlock& params ) {
   // for all 1900 add some
   vector< const NParam* > infos;
   params.getAllParams( 1900, infos );
   // Nbr items 2 
   buff.writeNextUnaligned16bit( infos.size() );
   for ( uint32 i = 0 ; i < infos.size() ; ++i ) {
      uint16 pos = 0;
      // ID
      buff.writeNextCharString( infos[ i ]->getString( pos ) );
      pos += strlen( infos[ i ]->getString( pos ) ) + 1;
      // nbrTuples
      uint8 nbrTuples = infos[ i ]->getByte( pos++ );
      buff.writeNext8bit( nbrTuples );
      for ( uint8 j = 0 ; j < nbrTuples ; ++j ) {
         // Type
         buff.writeNextUnaligned32bit( infos[ i ]->getByte( pos++ ) );
         // Key
         buff.writeNextCharString( infos[ i ]->getString( pos ) );
         pos += strlen( infos[ i ]->getString( pos ) ) + 1;
         // Value
         buff.writeNextCharString( infos[ i ]->getString( pos ) );
         pos += strlen( infos[ i ]->getString( pos ) ) + 1;
      }
   }
}


void handleMessageParams( Buffer& buff, NParamBlock& params ) {
   if ( params.getParam( 2100 ) ) {
      buff.writeNextUnaligned32bit( 
         params.getParam( 2100 )->getLength() );
      buff.writeNextByteArray( params.getParam( 2100 )->getBuff(),
                               params.getParam( 2100 )->getLength() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
}


void
NavHandler::handleUpgradeParams( Buffer& buff, NParamBlock& params ) {
   bool ok = true;
   if ( params.getParam( 2300 ) ) {
      buff.writeNext8bit( params.getParam( 2300 )->getByte() );
      ok = params.getParam( 2300 )->getByte() != 0;
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }
   if ( params.getParam( 2301 ) ) {
      buff.writeNext8bit( params.getParam( 2301 )->getByte() );
      ok = params.getParam( 2301 )->getByte() != 0;
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }
   if ( params.getParam( 2302 ) ) {
      buff.writeNext8bit( params.getParam( 2302 )->getByte() );
      ok = params.getParam( 2302 )->getByte() != 0;
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }
   if ( params.getParam( 2303 ) ) {
      buff.writeNext8bit( params.getParam( 2303 )->getByte() );
      if ( ok ) {
         m_lastParams->setWayfinderType( 
            params.getParam( 2303 )->getByte() );
      }
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }
   if ( params.getParam( 2304 ) ) {
      // lat,lon,scale
      // Used in newCenterPoint
   }
   if ( params.getParam( 2306 ) ) { //name
      buff.writeNext8bit( params.getParam( 2306 )->getByte() );
      ok = params.getParam( 2306 )->getByte() != 0;
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }
   if ( params.getParam( 2305 ) ) { //email
      buff.writeNext8bit( params.getParam( 2305 )->getByte() );
      ok = params.getParam( 2305 )->getByte() != 0;
   } else {
      buff.writeNext8bit( 0 );
      ok = false;
   }

   if ( ok ) {
      // May need new info after upgrade, see also ServerParser.
      m_lastServerInfo = 0;
   }
}


void handleVectorParams( Buffer& buff, NParamBlock& params ) {
   if ( params.getParam( 2500 ) ) {
      buff.writeNextCharString( params.getParam( 2500 )->getString() );
   } else {
      buff.writeNextCharString( "" );
   }
   if ( params.getParam( 2501 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 2501 )->getLength() );
      buff.writeNextByteArray( params.getParam( 2501 )->getBuff(),
                               params.getParam( 2501 )->getLength() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
}


void handleMultiVectorParams( Buffer& buff, NParamBlock& params ) {
   if ( params.getParam( 4700 ) ) {
      vector< const NParam* > ms;
      params.getAllParams( 4700, ms );
      uint32 size = 0;
      uint32 i = 0;
      for ( i = 0 ; i < ms.size() ; ++i ) {
         size += ms[ i ]->getLength();
      }
      buff.writeNextUnaligned32bit( size );
      for ( i = 0 ; i < ms.size() ; ++i ) {
         buff.writeNextByteArray( ms[ i ]->getBuff(), 
                                  ms[ i ]->getLength() );
      }
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
}


void handleCellParams( Buffer& buff, NParamBlock& params ) {
   if ( params.getParam( 5100 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 5100 )->getLength() );
      buff.writeNextByteArray( params.getParam( 5100 )->getBuff(),
                               params.getParam( 5100 )->getLength() ); 
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
}


void 
NavHandler::handleWhoAmIParams( Buffer& buff, NParamBlock& params ) {
   ParameterBlock* paramBlock = new ParameterBlock();
   // 3 User License Key
   if ( params.getParam( 3 ) ) {
      paramBlock->setUserLicense( params.getParam( 3 )->getBuff(),
                                  params.getParam( 3 )->getLength() );
   }
   // 7 WFST
   if ( params.getParam( 7 ) ) {
      paramBlock->setWayfinderType( params.getParam( 7 )->getByte() );
   }
   authParams( params, paramBlock );
   // 12 Transactions left
   if ( params.getParam( 12 ) ) {
      paramBlock->setTransactionsLeft( 
         params.getParam( 12 )->getUint32() );
   }
   
   buff.writeNextBuffer( *paramBlock );

   // Set local auth params
   setAuthParams( paramBlock );

   delete paramBlock;
}


void handleNOPParams( Buffer& /*buff*/, NParamBlock& /*params*/ )
{
   // More than status?
}


void 
NavHandler::handleTopRegionParams( Buffer& buff, NParamBlock& params ) {
   // Make poll buffer with topregiondata
   // Transfer Type 0x13 == Top Region List.
   buff.writeNextUnaligned32bit( 0x13 );
   // Crc
   if ( params.getParam( 4300 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 4300 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
   
   uint32 nbrRegions = 0;
   Buffer* tData = NULL;
   if ( params.getParam( 3100 ) ) {
      const NParam* t = params.getParam( 3100 );
      uint32 pos = 0;
      uint32 tLen = t->getLength();
      tData = new Buffer( size_t( t->getLength() ) );
      while ( pos < tLen ) {
         // ID
         tData->writeNextUnaligned32bit( t->getUint32( pos ) );
         pos += 4;
         // Type
         tData->writeNextUnaligned32bit( t->getUint32( pos ) );
         pos += 4;
         // Name
         const char* name = t->getString( pos );
         tData->writeNextCharString( name );
         pos += strlen( name ) + 1;
         ++nbrRegions;
      }
   }

   // Nbr regions
   buff.writeNextUnaligned32bit( nbrRegions );
   if ( tData ) {
      buff.writeNextByteArray( tData->accessRawData( 0 ), 
                               tData->getLength() );
   }
   
   delete tData;
}


void handleLatestNewsParams( Buffer& buff, NParamBlock& params ) {
   // Type 0x16 == Latest News
   buff.writeNextUnaligned32bit( 0x16 ); //LatestNewsFile
   // Crc
   if ( params.getParam( 4302 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 4302 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
   if ( params.getParam( 3300 ) ) {
      // Length
      buff.writeNextUnaligned32bit( params.getParam( 3300 )->getLength() );
      // Data
      buff.writeNextByteArray( params.getParam( 3300 )->getBuff(),
                               params.getParam( 3300 )->getLength() );
   } else {
      // Length
      buff.writeNextUnaligned32bit( 0 );
   }
}


void handleCategoriesParams( Buffer& buff, NParamBlock& params ) {
   // Type 0x15 == Categories
   buff.writeNextUnaligned32bit( 0x15 ); //CategoriesFile
   // Crc
   if ( params.getParam( 4303 ) ) {
      buff.writeNextUnaligned32bit( params.getParam( 4303 )->getUint32() );
   } else {
      buff.writeNextUnaligned32bit( 0 );
   }
   if ( params.getParam( 3500 ) ) {
      uint32 nbrCategories = 0;
      uint32 pos = 0;
      while ( pos < params.getParam( 3500 )->getLength() ) {
         pos += strlen( params.getParam( 3500 )->getString( pos ) ) + 1;
         pos += strlen( params.getParam( 3500 )->getString( pos ) ) + 1;
         ++nbrCategories;
      }
      // Nbr categories
      buff.writeNextUnaligned32bit( nbrCategories );
      // Data
      buff.writeNextByteArray( params.getParam( 3500 )->getBuff(),
                               params.getParam( 3500 )->getLength() );
      if ( params.getParam( 3501 ) ) {
         // There are images in the buffer.
         buff.writeNextUnaligned32bit( 3501 );
         // Image names
         buff.writeNextByteArray( params.getParam( 3501 )->getBuff(),
                                  params.getParam( 3501 )->getLength() );
      }
      if ( params.getParam( 3502 ) ) {
         // There are category number ids in the buffer.
         buff.writeNextUnaligned32bit( 3502 );
         // Category int id:s
         /* Not sure which param readout to prefer. Second one should be faster.
         but firs might be more correct.*/
         /*for( int i=0; i<nbrCategories; i++ ){
            buff.writeNextUnaligned16bit(params.getParam( 3502 )->getUint16(i));
         }*/
         buff.writeNextByteArray( params.getParam( 3502 )->getBuff(),
                                  params.getParam( 3502 )->getLength() );
      }
   } else {
      // Nbr categories
      buff.writeNextUnaligned32bit( 0 );
      return;
   }
}


void handleCallcenterListParams( Buffer& buff, NParamBlock& params ) {
   const char* callNbr = "";
   unsigned int crc = 0;
   if ( params.getParam( 4305 ) ) {
      crc = params.getParam( 4305 )->getUint32();
   }
   if ( params.getParam( 3700 ) ) {
      callNbr = params.getParam( 3700 )->getString();
   }
   char* callC = new char[ 12 + 1 + strlen( callNbr ) + 1 ];
   sprintf( callC, "0x%X!%s", crc, callNbr );
   ParameterBlock* paramBlock = new ParameterBlock();
   
   paramBlock->setCallCenterMSISDN( callC );

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;
   delete[] callC;
}


void handleServerListParams( Buffer& buff, NParamBlock& params ) {
   const char* sList = "";
   unsigned int crc = 0;
   if ( params.getParam( 4307 ) ) {
      crc = params.getParam( 4307 )->getUint32();
   }
   if ( params.getParam( 3900 ) ) {
      sList = params.getParam( 3900 )->getString();
   }
   char* serverList = new char[ 10 + 1 + strlen( sList ) + 1 ];
   sprintf( serverList, "%u!%s", crc, sList );
   ParameterBlock* paramBlock = new ParameterBlock();
   
   paramBlock->setAlternativeServer( serverList );

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;
   delete [] serverList;
}


void
NavHandler::handleServerInfoParams( Buffer& buff, NParamBlock& params ) {
   // All is handled in makePollPackets
   // But some are handled here
#ifndef NO_LOG_OUTPUT
   DBG("handleServerInfoParams");
   params.dump(m_log, true);
#endif
   ParameterBlock* paramBlock = new ParameterBlock();
   
   // Public user name
   if ( params.getParam( 4306 ) ) {
      paramBlock->setPublicUserName( 
         params.getParam( 4306 )->getString() );
   }

   // WFST
   if ( params.getParam( 7 ) ) {
      paramBlock->setWayfinderType( params.getParam( 7 )->getByte() );
   }

   // Days left
   if ( params.getParam( 18 ) ) {
      paramBlock->setSubscriptionLeft( 
         params.getParam( 18 )->getUint16() );
   }

   // User Rights field
   if ( params.getParam( 21 ) ) {
      // Size/4 uint32s
      uint32 size = params.getParam( 21 )->getLength() / 4;
      uint32* a = new uint32[ size ];
      for ( uint32 i = 0 ; i < size ; ++i ) {
         a[ i ] =  params.getParam( 21 )->getUint32( 4*i );
      }
      DBG("Received userrights param "
          "int32[%"PRId32"] = {%"PRIx32", %"PRIx32", %"PRIx32"}",
          size, size > 0 ? a[0] : 0, size > 1 ? a[1] : 0,
          size > 2 ? a[2] : 0);
      paramBlock->setUserRights( a, size );
      delete [] a;
   }

   // New version
   if ( params.getParam( 23 ) ) {
      paramBlock->setNewVersion( params.getParam( 23 )->getString() );
   }

   // Url for downloading new software
   if ( params.getParam( 34 ) ) {
      paramBlock->setNewVersionUrl( params.getParam( 34 )->getString() );
   }

   // Latest News Id
   if ( params.getParam( 27 ) ) {
      paramBlock->setLatestNewsId( params.getParam( 27 )->getString() );
   }

   // Favorites CRC
   if ( params.getParam( 4903 ) ) {
      paramBlock->setFavoritesCRC( params.getParam( 4903 )->getString() );
   }

   if ( params.getParam( 1107 ) ) {
      paramBlock->setServerPtui( params.getParam( 1107 )->getUint32() );
   }
   // If server whants client to change user
   bool any = authParams( params, paramBlock );
   
   // Set local auth params (user is changed)
   if ( any ) {
      setAuthParams( paramBlock );
   }

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;
}


void handleNewPasswordParams( Buffer& buff, NParamBlock& params ) {
   ParameterBlock* paramBlock = new ParameterBlock();
   
   const char* passwd = "";
   if ( params.getParam( 2 ) ) {
      passwd = params.getParam( 2 )->getString();
   }

   paramBlock->setUserPassword( passwd );

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;  
}


void 
NavHandler::handleChangedLicenseParams( Buffer& buff, NParamBlock& params )
{
   ParameterBlock* paramBlock = new ParameterBlock();
   
   if ( params.getParam( 3 ) ) {
      paramBlock->setOldUserLicense( params.getParam( 3 )->getBuff(),
                                     params.getParam( 3 )->getLength() );
      // Don't send changed again.
      m_lastParams->setOldUserLicense( params.getParam( 3 )->getBuff(),
                                       params.getParam( 3 )->getLength() );
   }

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;  
}


void 
NavHandler::handleServerAuthBobParams( Buffer& buff, NParamBlock& params )
{
   ParameterBlock* paramBlock = new ParameterBlock();
   
   if ( params.getParam( 19 ) ) {
      paramBlock->setServerAuthBob( params.getParam( 19 )->getBuff(),
                                    params.getParam( 19 )->getLength() );
      // Set local value too
      m_lastParams->setServerAuthBob( params.getParam( 19 )->getBuff(),
                                      params.getParam( 19 )->getLength() );
   }
   if ( params.getParam( 20 ) ) {
      paramBlock->setServerAuthBobChecksum( params.getParam( 20 )
                                            ->getUint32() );
      // Set local value too
      m_lastParams->setServerAuthBobChecksum( params.getParam( 20 )
                                              ->getUint32() );
   }

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;  
}


void
NavHandler::handleParamReplyParams( Buffer& buff, NParamBlock& params ) {
   DBG( "NavHandler::handleParamReplyParams" );
   ParameterBlock* paramBlock = new ParameterBlock();
   
   if ( params.getParam( 4307 ) && params.getParam( 3900 ) ) {
      unsigned int crc = params.getParam( 4307 )->getUint32();
      const char* sList = params.getParam( 3900 )->getString();
      char* serverList = new char[ 10 + 1 + strlen( sList ) + 1 ];
      sprintf( serverList, "%u!%s", crc, sList );
      paramBlock->setAlternativeServer( serverList );
      DBG( "NavHandler::handleParamReplyParams serverlist %s", serverList );
      delete [] serverList;
      // New server new settings?
      m_lastServerInfo = 0;
   }
   bool any = authParams( params, paramBlock );
   if ( any ) {
      // Set local auth params
      setAuthParams( paramBlock );
   }

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;
}


void
NavHandler::makeReplyBuffer( Buffer& buff, NavReplyPacket* pack, 
                             uint8 reg_ver, uint8 statusCode,
                             uint8 /*req_id*/, uint16 type,
                             uint8 protoVer )
{
   // Header is written already
   class NParamBlock& params = pack->getParamBlock();

   switch ( type ) {
      case NAV_SERVER_SEARCH_REPLY:
         handleSearchParams( buff, params );
         break;
      case NAV_SERVER_ROUTE_REPLY :
         handlRouteParams( buff, params );
         break;
      case NAV_SERVER_GPS_ADDRESS_REPLY :
         handleGpsAddressParams( buff, params );
         break;
      case NAV_SERVER_MAP_REPLY :
         handleMapParams( buff, params );
         break;
      case NAV_SERVER_INFO_REPLY :
         handleInfoParams( buff, params );
         break;
      case NAV_SERVER_MESSAGE_REPLY :
         handleMessageParams( buff, params );
         break;
      case NAV_SERVER_UPGRADE_REPLY :
         handleUpgradeParams( buff, params );
         break;
      case NAV_SERVER_VECTOR_MAP_REPLY :
         handleVectorParams( buff, params );
         break;
      case NAV_SERVER_MULTI_VECTOR_MAP_REPLY :
         handleMultiVectorParams( buff, params );
         break;
      case NAV_SERVER_CELL_CONFIRM :
         handleCellParams( buff, params );
         break;
      case navRequestType::NAV_WHOAMI_REPLY:
         handleWhoAmIParams( buff, params );
         break;
      case navRequestType::NAV_TOP_REGION_REPLY :
         handleTopRegionParams( buff, params );
         break;
      case navRequestType::NAV_LATEST_NEWS_REPLY :
         handleLatestNewsParams( buff, params );
         break;
      case navRequestType::NAV_CATEGORIES_REPLY :
         handleCategoriesParams( buff, params );
         break;
      case navRequestType::NAV_CALLCENTER_LIST_REPLY :
         handleCallcenterListParams( buff, params );
         break;
      case navRequestType::NAV_SERVER_LIST_REPLY :
         handleServerListParams( buff, params );
         break;
      case navRequestType::NAV_SERVER_INFO_REPLY :
         handleServerInfoParams( buff, params );
         break;
      case navRequestType::NAV_NEW_PASSWORD_REPLY :
         handleNewPasswordParams( buff, params );
         break;
      case navRequestType::NAV_CHANGED_LICENSE_REPLY :
         handleChangedLicenseParams( buff, params );
         break;
      case navRequestType::NAV_SERVER_AUTH_BOB_REPLY :
         handleServerAuthBobParams( buff, params );
         break;
      case NAV_SERVER_PARAMETER_REPLY :
         handleParamReplyParams( buff, params );
         break;
      default:
      {
         // All NGP goes in this case. All old types listed above and
         // all new are NGP!
         buff.writeNext8bit( protoVer );
         buff.writeNextUnaligned32bit( type );
         buff.writeNext8bit( reg_ver );
         buff.writeNext8bit( statusCode );
         buff.writeNextCharString( pack->getStatusMessage() );
         vector< byte > buf;
         params.writeParams( buf, protoVer, false/*no gzip internally*/ );
         buff.writeNextByteArray( &buf.front(), buf.size() );
         //TRACE_DBG( "makeReplyBuffer for %"PRIu16" ver %"PRIu8, type, reg_ver );
      }
      break;
   };

}


void 
NavHandler::setSearchParams( Buffer* content, NParamBlock& params ) {
   uint16 startIndex = content->readNextUnaligned16bit();
   NParam& p = params.addParam( NParam( 1200 ) );
   p.addUint16( startIndex ); // Area start index
   const char* city = content->getNextCharString();
   // Perhaps set itemStartIndex to MAX_UINT32 if area search to get
   // all area matches and no itemmatches
   if ( city[ 0 ] != '\0' ) {
      params.addParam( NParam( 1201, city ) );
      p.addUint16( startIndex ); // Item start index
   } else {
      p.addUint16( startIndex ); // Item start index
   }
   const char* areaID = content->getNextCharString();
   if ( areaID[ 0 ] != '\0' ) {
      params.addParam( NParam( 1202, areaID ) );
   }
   const char* destination = content->getNextCharString();
   if ( destination[ 0 ] != '\0' ) {
      if ( destination[ 0 ] == 0x18 ) {
         params.addParam( NParam( 1204, destination + 1 ) );
      } else {
         params.addParam( NParam( 1203, destination ) );
      }
   }
   uint32 TID = content->readNextUnaligned32bit();
   if ( TID != MAX_UINT32 ) {
      params.addParam( NParam( 1205, TID ) );
   }
   int32 lat = content->readNextUnaligned32bit();
   int32 lon = content->readNextUnaligned32bit();
   if ( lat != MAX_INT32 && 
        (lat <= MAX_INT32/2 && lat >= int32(MIN_INT32)/2) ) 
   {
      NParam& p = params.addParam( NParam( 1206 ) );
      p.addInt32( lat );
      p.addInt32( lon );
   }
   bool isSet = false;
   uint8 maxNbrMatches = m_lastParams->getMaxSearchMatches( &isSet );
   if ( isSet ) {
      params.addParam( NParam( 1207, uint16( maxNbrMatches ) ) );
   }
//   uint8 heading = content->readNext8bit();
}


void
NavHandler::setRouteParams( Buffer* content, NParamBlock& params ) {
   NParam& porigCoord = params.addParam( NParam( 1000 ) );
   porigCoord.addInt32( content->readNextUnaligned32bit() );
   porigCoord.addInt32( content->readNextUnaligned32bit() );
   //Destination coordinate
   class NParam& pdestCoord = params.addParam( NParam( 1001 ) );
   pdestCoord.addInt32( content->readNextUnaligned32bit() );
   pdestCoord.addInt32( content->readNextUnaligned32bit() );
   // timeToTrunk
   uint32 timeToTrunk = content->readNextUnaligned32bit();
   if ( timeToTrunk != MAX_UINT32 ) {
      params.addParam( NParam( 1002, timeToTrunk ) );
   }
   // angle, added to origin coordinate parameter 1000
   porigCoord.addUint16( content->readNextUnaligned16bit() );
   // speed
   params.addParam( NParam( 1004, uint16(content->readNextUnaligned16bit()) ) );
   //flags contain several options. 
   uint32 flags = 0;
   // routeType
   flags |= (content->readNext8bit())<<8;
   // vehicle
   flags |= (content->readNext8bit())<<16;
   // routeCost
   uint8 routeCost = content->readNext8bit();
   if ( (routeCost & 0x0f) == 0x00 ) {
      flags |= 0x1;
   } else if ( (routeCost & 0x0f) == 0x01 ) {
      flags |= 0x2;
   } else if ( (routeCost & 0x0f) == 0x02 ) {
      flags |= 0x4;
   }
   // avoidTollRoads
   flags |= ((routeCost>>7) != 0)<<5;
   // avoidHighway
   flags |= (((routeCost>>6) & 0x01) != 0) <<4;
   // landmarks
   flags |= (((routeCost>>5) & 0x01) != 0)<<7;
   // abbreviate
   flags |= (!(((routeCost>>4) & 0x01) != 0))<<6;
   params.addParam( NParam( 1005, flags ) );
   
   //route id, added as id and time
   char tmp[ 256 ];
   unsigned int routeID = content->readNextUnaligned32bit();
   unsigned int routeTime = content->readNextUnaligned32bit();
   sprintf( tmp, "%X_%X", routeID, routeTime );
   params.addParam( NParam( 1006, tmp ) );

   //reroute reason
   params.addParam( NParam( 1013, uint8(content->readNext8bit()) ) );
}


void setGpsAddressParams( Buffer* /*content*/, NParamBlock& /*params*/ )
{
   // XXX: Fill in here
}


void setMapBBoxParams( Buffer* content, NParamBlock& params )
{
   uint16 bboxType = content->readNextUnaligned16bit();
   switch( bboxType ) {
      case 1 : {// BoundingBox
         NParam pbbox( 1600 );
//         NParam& pbbox = params.addParam( NParam( 1600 ) );
         pbbox.addInt32( content->readNextUnaligned32bit() );
         pbbox.addInt32( content->readNextUnaligned32bit() );
         pbbox.addInt32( content->readNextUnaligned32bit() );
         pbbox.addInt32( content->readNextUnaligned32bit() );
         params.addParam( pbbox );
      } break;
      case 2 : {// VectorBox
         NParam ppos( 1601 );
         ppos.addInt32( content->readNextUnaligned32bit() );
         ppos.addInt32( content->readNextUnaligned32bit() );
         params.addParam( ppos );
         params.addParam( NParam( 1602, uint16( content->readNextUnaligned16bit() ) ) );
         params.addParam( NParam( 1603, uint16( content->readNext8bit() ) ) );
         content->readNext8bit(); // Padding
         params.addParam( NParam( 1604, uint32( content->readNextUnaligned32bit() / 2 ) ) );
      } break;
      case 3 : {// RadiusBox
         NParam ppos( 1601 );
         ppos.addInt32( content->readNextUnaligned32bit() );
         ppos.addInt32( content->readNextUnaligned32bit() );
         params.addParam( ppos );
         params.addParam( NParam( 1604, uint32( content->readNextUnaligned32bit() / 2 ) ) );
         content->readNextUnaligned32bit(); // Padding
      } break;
      case 4 : { // RouteBox
         unsigned int a = content->readNextUnaligned32bit();
         unsigned int b = content->readNextUnaligned32bit();
         char tmpStr[ 256 ];
         sprintf( tmpStr, "%X_%X", a, b );
         params.addParam( NParam( 1605, tmpStr ) );
         content->readNextUnaligned32bit(); // Padding
         content->readNextUnaligned32bit(); // Padding
      } break;
   }
}


void setMapParams( Buffer* content, NParamBlock& params ) {
   setMapBBoxParams( content, params );
   {
      NParam piwh( 1606 );
      piwh.addUint16( content->readNextUnaligned16bit() );
      piwh.addUint16( content->readNextUnaligned16bit() );
      params.addParam( piwh );
      NParam pvwh( 1607 );
      pvwh.addUint16( content->readNextUnaligned16bit() );
      pvwh.addUint16( content->readNextUnaligned16bit() );
      params.addParam( pvwh );
      params.addParam( NParam( 1608, static_cast<uint8>(content->readNextUnaligned16bit()) ) );
   }
   {
      uint16 nbrItems = content->readNextUnaligned16bit();
      for ( uint16 i = 0 ; i < nbrItems ; i++ ) {
         uint16 type = content->readNextUnaligned16bit();
         if ( type == 1 /*Route*/ ) {
            unsigned int a = content->readNextUnaligned32bit();
            unsigned int b = content->readNextUnaligned32bit();
            char tmpStr[ 256 ];
            sprintf( tmpStr, "%X_%X", a, b );
            params.addParam( NParam( 1605, tmpStr ) );
         } else {
            NParam pmapItem( 1609 );
            pmapItem.addUint16( type );
            unsigned int a = content->readNextUnaligned32bit();
            unsigned int b = content->readNextUnaligned32bit();
            pmapItem.addUint32( a );
            pmapItem.addUint32( b );
            params.addParam( pmapItem );
         }
      }
   }
   {
      uint16 nbrExtraInfo = content->readNextUnaligned16bit();
      for ( uint16 i = 0 ; i < nbrExtraInfo ; i++ ) {
         NParam pmapInfo( 1610 );
         pmapInfo.addUint16( content->readNextUnaligned16bit() );
         pmapInfo.addUint32( content->readNextUnaligned32bit() );
         params.addParam( pmapInfo );
      }
   }
}


void setInfoParams( Buffer* content, NParamBlock& params ) {
   uint16 nbrItems = content->readNextUnaligned16bit();
   for ( uint16 i = 0 ; i < nbrItems ; ++i ) {
      NParam& p = params.addParam( 
         NParam( 1800, content->getNextCharString() ) );
      p.addString( content->getNextCharString() );
   }
}


void setMessageParams( Buffer* content, NParamBlock& params ) {
   uint8 messType = content->readNext8bit();
   params.addParam( NParam( 2000, messType ) );
   uint8 objType = content->readNext8bit();
   params.addParam( NParam( 2001, objType ) );
   const char* objStr = content->getNextCharString();
   const char* receiver = content->getNextCharString();
   const char* sender = content->getNextCharString();
   char tmpStr[30];
   params.addParam( NParam( 2002, content->getNextCharString() ) );
   switch ( objType ) {
      case 1: // Fav
         params.addParam( 
            NParam( 2008, uint32( strtoul( objStr, NULL, 0 ) ) ) );
         break;
      case 2:
      case 3: // Search
         params.addParam( NParam( 2009, objStr ) );
         break;
      case 4:
      case 5: { // Route
         unsigned int routeID = 0;
         unsigned int routeTime = 0;
         sscanf( objStr, "0x%8x%8x", 
                 &routeID, &routeTime );
         sprintf( tmpStr, "%X_%X", routeID, routeTime );
         params.addParam( NParam( 2012, tmpStr ) );
//         DBG("XXXXXXXX routeID %s; %X,%X; %s",
//             objStr, routeID, routeTime, tmpStr );
         } break;
      case 7: { // Position
         int lat = 0;
         int lon = 0;
         char* posStr = new char[ strlen( objStr ) + 1 ];
         if ( sscanf( objStr, "%i %i %[^\n\r]", &lat, &lon, posStr ) 
              == 3 ) 
         {
            NParam coord( 2010, int32( lat ) );
            coord.addInt32( lon );
            params.addParam( coord );
            params.addParam( NParam( 2011, posStr ) );
         } else if ( sscanf( objStr, "%i %i ", &lat, &lon ) == 2 ) {
            NParam coord( 2010, int32( lat ) );
            coord.addInt32( lon );
            params.addParam( coord );
         } else {
            // Errore
         }
         delete [] posStr;
      } break;
   };

   switch ( messType ) {
      case 1:
      case 2: // Html
         params.addParam( NParam( 2004, receiver ) );
         params.addParam( NParam( 2005, sender ) );
         break;
      case 3:
      case 4: // ?MS
         params.addParam( NParam( 2006, receiver ) );
         params.addParam( NParam( 2007, sender ) );
         break;
   };
}


void setUpgradeParams( Buffer* content, NParamBlock& params ) {
   params.addParam( NParam( 2202, content->readNextUnaligned32bit() ));//region
   params.addParam( NParam( 2200, content->getNextCharString() ) ); //key
   params.addParam( NParam( 2201, content->getNextCharString() ) ); //phone
   params.addParam( NParam( 2204, content->getNextCharString() ) ); //name
   params.addParam( NParam( 2203, content->getNextCharString() ) ); //email
   params.addParam( NParam( 2205, content->getNextCharString() ) ); //name
}


void setVectorParams( Buffer* content, NParamBlock& params ) {
   params.addParam( NParam( 2400, content->getNextCharString() ) );
}


void setMultiVectorParams( Buffer* content, NParamBlock& params ) {
//   content->readNextUnaligned32bit(); // dataLength
   uint32 startOffset = content->readNextUnaligned32bit();
   if ( startOffset != 0 ) {
      params.addParam( NParam( 4600, startOffset ) );
   }
   uint32 maxSize = content->readNextUnaligned32bit();
   if ( maxSize != 2048 ) {
      params.addParam( NParam( 4601, maxSize ) );
   }
   NParam& data = params.addParam( NParam( 4602 ) );
   while( content->remaining() > 0 ) {
      data.addString( content->getNextCharString() );
   }
}


void setCellReportParams( Buffer* content, NParamBlock& params ) {
   uint32 size = content->readNextUnaligned32bit();
   NParam& data = params.addParam( NParam( 5000 ) );
   for ( uint32 i = 0 ; i < size ; ++i ) {
      data.addByte( content->readNext8bit() );
   }
}


NavRequestPacket* 
NavHandler::handleRequest( uint8 protoVer, uint16 type, uint8 reqID, 
                           Buffer* content )
{
   NavRequestPacket* pack = new NavRequestPacket( 
      protoVer, type, reqID, 1 );
   NParamBlock& params = pack->getParamBlock();
   content->setReadPos( 0 ); // reset I hope
   
   // Handle v8 packets here, convert to v10
   switch ( type ) {
      case NAV_SERVER_SEARCH_REQ :
         setSearchParams( content, params );
         break;
      case NAV_SERVER_ROUTE_REQ :
         pack->setRequestVer( 2 ); // Supports Lanes & Signposts.
         setRouteParams( content, params );
         break;
      case NAV_SERVER_GPS_ADDRESS_REQ :
         setGpsAddressParams( content, params );
         break;
      case NAV_SERVER_MAP_REQ :
         setMapParams( content, params );
         break;
      case NAV_SERVER_INFO_REQ :
         setInfoParams( content, params );
         break;
      case NAV_SERVER_MESSAGE_REQ :
         setMessageParams( content, params );
         break;
      case NAV_SERVER_UPGRADE_REQ :
         pack->setRequestVer( 2 ); // Supports UPGRADE_MUST_CHOOSE_REGION
         setUpgradeParams( content, params );
         break;
      case NAV_SERVER_VECTOR_MAP_REQ :
         setVectorParams( content, params );
         break;
      case NAV_SERVER_MULTI_VECTOR_MAP_REQ :
         setMultiVectorParams( content, params );
         break;
      case NAV_SERVER_CELL_REPORT :
         setCellReportParams( content, params );
         break;
      case NAV_SERVER_BINARY_UPLOAD_REQ :
         // All binary uploads moved to own requests now, favorites
         // even is ngp all the way to Destinations module!
         break;
      case NAV_REQUEST: {
         // Params in content
         pack->setProtoVer( content->readNext8bit() );
         pack->setType( navRequestType::RequestType( 
                           content->readNextUnaligned32bit() ) );
         pack->setRequestVer( content->readNext8bit() );
         NParamBlock contParams( content->accessRawData( 
                                    content->getReadPos() ), 
                                 content->remaining(), protoVer );
         params.assignToThis( contParams );
         } break;
      
      default:
         break;
   };

   return pack;
}


void 
NavHandler::handleParameters( Buffer& data, bool* secondaryServer ) {
   if ( secondaryServer != NULL ) {
      m_secondaryServer = *secondaryServer;
   }
   m_lastParams->clear();
   m_lastParams->writeNextBuffer( data );
   m_lastParams->decode();

   setAuthParams( m_lastParams );
}


void
NavHandler::setAuthParams( ParameterBlock* lastParams ) {
   bool isSet = true;
// m_lastParams->getTopRegionChecksum( &isSet )
   // getMaxSearchMatches

   const char* userLogin = lastParams->getUserLogin( &isSet );
   if ( isSet ) {
      delete[] m_userLogin;
      m_userLogin = strdup_new( userLogin );
   }

   const char* userPassword = lastParams->getUserPassword( &isSet );
   if ( isSet ) {
      delete[] m_userPassword;
      m_userPassword = strdup_new( userPassword );
   }

   int userLicenseLen = 0;
   const uint8* userLicense = lastParams->getUserLicense( 
      userLicenseLen, &isSet );
   if ( isSet ) {
      delete[] m_userLicense;
      m_userLicenseLen = userLicenseLen;
      m_userLicense = new uint8[ m_userLicenseLen ];
      memcpy( m_userLicense, userLicense, m_userLicenseLen );
   }
   
   const char* clientType = lastParams->getClientType( &isSet );
   if ( isSet ) {
      delete[] m_clientType;
      m_clientType = strdup_new( clientType );
   }

   const char* clientTypeOptions = lastParams->getClientTypeOptions( 
      &isSet );
   if ( isSet ) {
      delete[] m_clientTypeOptions;
      m_clientTypeOptions = strdup_new( clientTypeOptions );
   }

   const uint32* programVersion = lastParams->getProgramVersion( 
      &isSet );
   if ( isSet ) {
      m_programVersion[ 0 ] = programVersion[ 0 ];
      m_programVersion[ 1 ] = programVersion[ 1 ];
      m_programVersion[ 2 ] = programVersion[ 2 ];
   }
   
   uint32 languageOfClient = lastParams->getLanguageOfClient( &isSet );
   if ( isSet ) {
      m_languageOfClient = languageOfClient;
   }

   uint8 wayfinderType = lastParams->getWayfinderType( &isSet );
   if ( isSet ) {
      m_wayfinderType = wayfinderType;
   }
}


void 
NavHandler::addAuthParams( NParamBlock& params ) {
   if ( m_userLogin ) {
      params.addParam( NParam( 1, m_userLogin ) );
   }
   if ( m_userPassword && m_userPassword[ 0 ] != '\0' ) {
      params.addParam( NParam( 2, m_userPassword ) );
   }	
   for ( std::vector<HWID*>::iterator iterat = m_vhardwareIDs.begin();
                                      iterat != m_vhardwareIDs.end(); iterat++ ){
      HWID* hwid = *iterat;
      params.addParam( NParam( 3, (const uint8*)hwid->m_key,
                                  (uint32)strlen(hwid->m_key) ) ); 
      params.addParam( NParam( 29, (const uint8*)(hwid->m_hardWareType), 
                                   (uint32)strlen(hwid->m_hardWareType) + 1 ) );
   }
   if ( m_userLicense ) {
      params.addParam( NParam( 3, m_userLicense, m_userLicenseLen ) );
   }
   
   if ( m_clientType && m_clientType[ 0 ] != '\0' ) {
      params.addParam( NParam( 4, m_clientType ) );
   }
   if ( m_clientTypeOptions && m_clientTypeOptions[ 0 ] != '\0' ) {
      params.addParam( NParam( 5, m_clientTypeOptions ) );
   }
   params.addParam( NParam( 11, m_programVersion, uint16(3) ) );
   params.addParam( NParam( 6, uint16( m_languageOfClient ) ) );
   params.addParam( NParam( 7, m_wayfinderType ) );
   if ( m_secondaryServer ) {
      bool isSet = false;
      int len = 0;
      const uint8* data = m_lastParams->getServerAuthBob( len, &isSet );
      if ( isSet ) {
         params.addParam( NParam( 19, data, len ) );
      }
   }

   if ( m_licenseKeyType != NULL ) {
      params.addParam( NParam( 29, (const uint8*) m_licenseKeyType, 
                               strlen( m_licenseKeyType ) + 1 ) );
   }
}


bool
NavHandler::haveAuthParams( bool& haveLicense ) {
   bool haveUser = false;

   if ( m_userLogin ) {
      haveUser = true;
   }

   if ( m_userLicense || !m_vhardwareIDs.empty()) {
      haveLicense = true;
   } else {
      haveLicense = false;
   }

   return haveUser;
}


bool
NavHandler::haveChangedLicense() {
   bool isSet = false;
   int oldUserLicenseLen = 0;
   const uint8* oldUserLicense = m_lastParams->getOldUserLicense( 
      oldUserLicenseLen, &isSet );
   if ( isSet && m_userLicense && oldUserLicenseLen > 1 &&
        (oldUserLicenseLen != int32(m_userLicenseLen) ||
         memcmp( oldUserLicense, m_userLicense, oldUserLicenseLen ) != 0) )
   {
      // Needs update
      return true;
   } else {
      return false;
   }
}


void
NavHandler::addChangedLicense( NParamBlock& params ) {
   if ( haveChangedLicense() ) {
      bool isSet = false;
      int oldUserLicenseLen = 0;
      const uint8* oldUserLicense = m_lastParams->getOldUserLicense( 
         oldUserLicenseLen, &isSet );
      params.addParam( NParam( 17, oldUserLicense, oldUserLicenseLen ) );

      if ( m_licenseKeyType != NULL ) {
         params.addParam( NParam( 30, (const uint8*) m_licenseKeyType, 
                                  strlen( m_licenseKeyType ) + 1 ) );
      }
   }
}



const char*
NavHandler::getSessionKey() const {
   return m_sessionKey;
}
      

void
NavHandler::setSessionKey( const char* key ) {
   delete[] m_sessionKey;
   m_sessionKey = strdup_new( key );
}


void
NavHandler::addSessionParams( NParamBlock& params ) {
   if ( m_sessionKey ) {
      params.addParam( NParam( 14, m_sessionKey ) );
   }
   params.addParam( NParam( 6, uint16( m_languageOfClient ) ) );
}


uint32 
NavHandler::crc32( const uint8* buf, uint32 len, uint32 crc ) const {
   if ( len < 4 ) {
      WARN( "MC22CRC32::crc32 buffer shorter than 4 bytes, "
           "no crc calculated" );
      return crc;
   }

   crc = *buf++ << 24;
   crc |= *buf++ << 16;
   crc |= *buf++ << 8;
   crc |= *buf++;
   crc = ~ crc;
   len -= 4;
    
   for ( uint32 i = 0 ; i < len ; i++ ) {
      crc = (crc << 8 | *buf++) ^ m_crcmap[ crc >> 24 ];
   }
    
   return ~crc;
}


#define POLYQUOTIENT 0x04C11DB7

uint32*
NavHandler::makeCRCMap() {
   uint32* crcmap = new uint32[ 256 ];
   uint32 staticrcMap[ 256 ] = {
      0x00000000,  0x04C11DB7,  0x09823B6E,  0x0D4326D9,  0x130476DC, 
      0x17C56B6B,  0x1A864DB2,  0x1E475005,  0x2608EDB8,  0x22C9F00F, 
      0x2F8AD6D6,  0x2B4BCB61,  0x350C9B64,  0x31CD86D3,  0x3C8EA00A, 
      0x384FBDBD,  0x4C11DB70,  0x48D0C6C7,  0x4593E01E,  0x4152FDA9, 
      0x5F15ADAC,  0x5BD4B01B,  0x569796C2,  0x52568B75,  0x6A1936C8, 
      0x6ED82B7F,  0x639B0DA6,  0x675A1011,  0x791D4014,  0x7DDC5DA3, 
      0x709F7B7A,  0x745E66CD,  0x9823B6E0,  0x9CE2AB57,  0x91A18D8E, 
      0x95609039,  0x8B27C03C,  0x8FE6DD8B,  0x82A5FB52,  0x8664E6E5, 
      0xBE2B5B58,  0xBAEA46EF,  0xB7A96036,  0xB3687D81,  0xAD2F2D84, 
      0xA9EE3033,  0xA4AD16EA,  0xA06C0B5D,  0xD4326D90,  0xD0F37027, 
      0xDDB056FE,  0xD9714B49,  0xC7361B4C,  0xC3F706FB,  0xCEB42022, 
      0xCA753D95,  0xF23A8028,  0xF6FB9D9F,  0xFBB8BB46,  0xFF79A6F1, 
      0xE13EF6F4,  0xE5FFEB43,  0xE8BCCD9A,  0xEC7DD02D,  0x34867077, 
      0x30476DC0,  0x3D044B19,  0x39C556AE,  0x278206AB,  0x23431B1C, 
      0x2E003DC5,  0x2AC12072,  0x128E9DCF,  0x164F8078,  0x1B0CA6A1, 
      0x1FCDBB16,  0x018AEB13,  0x054BF6A4,  0x0808D07D,  0x0CC9CDCA, 
      0x7897AB07,  0x7C56B6B0,  0x71159069,  0x75D48DDE,  0x6B93DDDB, 
      0x6F52C06C,  0x6211E6B5,  0x66D0FB02,  0x5E9F46BF,  0x5A5E5B08, 
      0x571D7DD1,  0x53DC6066,  0x4D9B3063,  0x495A2DD4,  0x44190B0D, 
      0x40D816BA,  0xACA5C697,  0xA864DB20,  0xA527FDF9,  0xA1E6E04E, 
      0xBFA1B04B,  0xBB60ADFC,  0xB6238B25,  0xB2E29692,  0x8AAD2B2F, 
      0x8E6C3698,  0x832F1041,  0x87EE0DF6,  0x99A95DF3,  0x9D684044, 
      0x902B669D,  0x94EA7B2A,  0xE0B41DE7,  0xE4750050,  0xE9362689, 
      0xEDF73B3E,  0xF3B06B3B,  0xF771768C,  0xFA325055,  0xFEF34DE2, 
      0xC6BCF05F,  0xC27DEDE8,  0xCF3ECB31,  0xCBFFD686,  0xD5B88683, 
      0xD1799B34,  0xDC3ABDED,  0xD8FBA05A,  0x690CE0EE,  0x6DCDFD59, 
      0x608EDB80,  0x644FC637,  0x7A089632,  0x7EC98B85,  0x738AAD5C, 
      0x774BB0EB,  0x4F040D56,  0x4BC510E1,  0x46863638,  0x42472B8F, 
      0x5C007B8A,  0x58C1663D,  0x558240E4,  0x51435D53,  0x251D3B9E, 
      0x21DC2629,  0x2C9F00F0,  0x285E1D47,  0x36194D42,  0x32D850F5, 
      0x3F9B762C,  0x3B5A6B9B,  0x0315D626,  0x07D4CB91,  0x0A97ED48, 
      0x0E56F0FF,  0x1011A0FA,  0x14D0BD4D,  0x19939B94,  0x1D528623, 
      0xF12F560E,  0xF5EE4BB9,  0xF8AD6D60,  0xFC6C70D7,  0xE22B20D2, 
      0xE6EA3D65,  0xEBA91BBC,  0xEF68060B,  0xD727BBB6,  0xD3E6A601, 
      0xDEA580D8,  0xDA649D6F,  0xC423CD6A,  0xC0E2D0DD,  0xCDA1F604, 
      0xC960EBB3,  0xBD3E8D7E,  0xB9FF90C9,  0xB4BCB610,  0xB07DABA7, 
      0xAE3AFBA2,  0xAAFBE615,  0xA7B8C0CC,  0xA379DD7B,  0x9B3660C6, 
      0x9FF77D71,  0x92B45BA8,  0x9675461F,  0x8832161A,  0x8CF30BAD, 
      0x81B02D74,  0x857130C3,  0x5D8A9099,  0x594B8D2E,  0x5408ABF7, 
      0x50C9B640,  0x4E8EE645,  0x4A4FFBF2,  0x470CDD2B,  0x43CDC09C, 
      0x7B827D21,  0x7F436096,  0x7200464F,  0x76C15BF8,  0x68860BFD, 
      0x6C47164A,  0x61043093,  0x65C52D24,  0x119B4BE9,  0x155A565E, 
      0x18197087,  0x1CD86D30,  0x029F3D35,  0x065E2082,  0x0B1D065B, 
      0x0FDC1BEC,  0x3793A651,  0x3352BBE6,  0x3E119D3F,  0x3AD08088, 
      0x2497D08D,  0x2056CD3A,  0x2D15EBE3,  0x29D4F654,  0xC5A92679, 
      0xC1683BCE,  0xCC2B1D17,  0xC8EA00A0,  0xD6AD50A5,  0xD26C4D12, 
      0xDF2F6BCB,  0xDBEE767C,  0xE3A1CBC1,  0xE760D676,  0xEA23F0AF, 
      0xEEE2ED18,  0xF0A5BD1D,  0xF464A0AA,  0xF9278673,  0xFDE69BC4, 
      0x89B8FD09,  0x8D79E0BE,  0x803AC667,  0x84FBDBD0,  0x9ABC8BD5, 
      0x9E7D9662,  0x933EB0BB,  0x97FFAD0C,  0xAFB010B1,  0xAB710D06, 
      0xA6322BDF,  0xA2F33668,  0xBCB4666D,  0xB8757BDA,  0xB5365D03, 
      0xB1F740B4 };
   for ( uint32 i = 0 ; i < 256 ; i++ ) {
      crcmap[ i ] = staticrcMap[ i ];
   }

   return crcmap;
}


bool
NavHandler::needParameterSync() const 
{
   time_t now = TimeUtils::time(NULL);
   DBG("needParameterSync - now: %"PRIu32", last: %"PRIu32", timeout: %d : %s",
       uint32(now), uint32(m_lastServerInfo), SERVER_INFO_TIMEOUT, 
       (( now - m_lastServerInfo ) > SERVER_INFO_TIMEOUT ) ? "yes" : "no" );
   if ( ( now - m_lastServerInfo ) > SERVER_INFO_TIMEOUT ) {
      return true;
   } else {
      return false;
   }
}


void
NavHandler::makePollPackets( NavReplyPacket* pack, 
                             vector<NavRequestPacket*>& polls )
{
   m_lastServerInfo = TimeUtils::time(NULL);

   // For all known parameter type check if set and needs update
   bool isSet = false;
   uint32 crc = 0;

   // TopRegionChecksum
   crc = m_lastParams->getTopRegionChecksum( &isSet );
   if ( pack->getParamBlock().getParam( 4300 ) != NULL && isSet ) {
      if ( crc != pack->getParamBlock().getParam( 4300 )->getUint32() ) {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_TOP_REGION_REQ, 
                                  pack->getReqID(), 1 ) );
         polls.back()->getParamBlock().addParam( NParam( 4300, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // Latest news crc
   crc = m_lastParams->getLatestNews( &isSet );
   if ( pack->getParamBlock().getParam( 4302 ) != NULL && isSet ) {
      if ( crc != pack->getParamBlock().getParam( 4302 )->getUint32() ) {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_LATEST_NEWS_REQ, 
                                  pack->getReqID(), 1 ) );
         polls.back()->getParamBlock().addParam( NParam( 4302, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // Categories crc
   crc = m_lastParams->getCategories( &isSet );
   if ( pack->getParamBlock().getParam( 4303 ) != NULL && isSet ) {
      if ( crc != pack->getParamBlock().getParam( 4303 )->getUint32() ) {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_CATEGORIES_REQ, 
                                  pack->getReqID(), 2 /*1*/ ) );
         polls.back()->getParamBlock().addParam( NParam( 4303, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // CallCenter Checksum
   crc = m_lastParams->getCallCenterChecksum( &isSet );
   if ( pack->getParamBlock().getParam( 4305 ) != NULL && isSet ) {
      if ( crc != pack->getParamBlock().getParam( 4305 )->getUint32() ) {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_CALLCENTER_LIST_REQ, 
                                  pack->getReqID(), 1 ) );
         polls.back()->getParamBlock().addParam( NParam( 4305, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // Server List Checksum 
   // Never set in client so send if not set in client or different
   crc = m_lastParams->getServerListChecksum( &isSet );
   if ( pack->getParamBlock().getParam( 4307 ) != NULL ) {
      if ( !isSet || 
           crc != pack->getParamBlock().getParam( 4307 )->getUint32() ) 
      {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_SERVER_LIST_REQ,
                                  pack->getReqID(), 1 ) );
         polls.back()->getParamBlock().addParam( NParam( 4307, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // Server auth bob checksum
   crc = m_lastParams->getServerAuthBobChecksum( &isSet );
   if ( pack->getParamBlock().getParam( 20 ) != NULL ) {
      if ( !isSet || 
           crc != pack->getParamBlock().getParam( 20 )->getUint32() ) 
      {
         // Make packet
         polls.push_back( 
            new NavRequestPacket( m_protoVer, 
                                  navRequestType::NAV_SERVER_AUTH_BOB_REQ,
                                  pack->getReqID(), 1 ) );
         polls.back()->getParamBlock().addParam( NParam( 20, crc ) );
         addAuthParams( polls.back()->getParamBlock() );
      }
   }

   // New Password
   const char* newPw = m_lastParams->getNewPasswordSet( &isSet );
   if ( isSet ) {
      // Make packet
      polls.push_back( 
         new NavRequestPacket( m_protoVer, 
                               navRequestType::NAV_NEW_PASSWORD_REQ,
                               pack->getReqID(), 1 ) );
      polls.back()->getParamBlock().addParam( NParam( 4000, newPw ) );
      addAuthParams( polls.back()->getParamBlock() );
   }

   // Tracking PINs CRC
   if ( pack->getParamBlock().getParam( 5204 ) != NULL && 
        m_tracking != NULL ) 
   {
      m_tracking->newPINCRC( pack->getParamBlock().getParam( 5204 )
                             ->getUint32() );
   }

   // Server UTC time
   if ( pack->getParamBlock().getParam( 25 ) != NULL && 
        m_tracking != NULL ) 
   {
      m_tracking->setServerTime( pack->getParamBlock().getParam( 25 )
                                 ->getUint32() );
   }
   
}


bool
NavHandler::changedWFST( uint8 wfst ) {
   bool isSet = false;
   uint8 wayfinderType = m_lastParams->getWayfinderType( &isSet );
   if ( isSet ) {
      return (wfst != wayfinderType);
   } else {
      return false;
   }
}


void
NavHandler::makeChangedWFSTBuffer( Buffer& buff, uint8 wfst ) {
   m_lastParams->setWayfinderType( wfst );
   ParameterBlock* paramBlock = new ParameterBlock();
   
   // WFST
   paramBlock->setWayfinderType( wfst );

   buff.writeNextBuffer( *paramBlock );

   delete paramBlock;
}


bool
NavHandler::upgradeParams( NavReplyPacket* pack, Buffer* wfB ) {
   bool dataToSend = false;
   NParamBlock& params = pack->getParamBlock();
   ParameterBlock* paramBlock = new ParameterBlock();

   dataToSend = authParams( params, paramBlock );
   if ( dataToSend ) {
      // Set local auth params
      setAuthParams( paramBlock );
   }

   if ( params.getParam( 2304 ) ) {
      // lat,lon,scale in 2304
      // Centerpoint
      paramBlock->setCenterPointAndScale( 
         params.getParam( 2304 )->getBuff(), 
         params.getParam( 2304 )->getLength() );

      dataToSend =  true;
   }

   if ( dataToSend ) {
      wfB->writeNextBuffer( *paramBlock );
   }
   
   delete paramBlock;
   
   return dataToSend;
}


void
NavHandler::setTracking( Tracking* tracking ) {
   m_tracking = tracking;
}

void
NavHandler::setLicenseKeyType( const char* licenseKeyType )
{
   delete[] m_licenseKeyType;
   m_licenseKeyType = strdup_new( licenseKeyType );
}

void NavHandler::addHWID(HWID* hwid)
{
   HWID* newHWID = new HWID(*hwid); 
   m_vhardwareIDs.push_back(newHWID);
}

void NavHandler::setWayfinderIdStartUp( bool wayfinderIdStartUp )
{
   m_wayfinderIdStartUp = wayfinderIdStartUp;
}

bool NavHandler::isWayfinderIdStartUp() const
{
   return m_wayfinderIdStartUp;
}

} // End namespace isab
