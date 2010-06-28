/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* An object of the class Route stores a whole or partitial route. It
* is responsible for assembling the route from chunks of data, 
* parse it and ansers queries concerning it. 
*
* It is also responsible for managing the storage required.
*/

#include "arch.h"
#include <stdio.h>
#ifndef UNDER_CE
# include <unistd.h>
#endif

#include "Route.h"
#include "Point.h"

#include "LogMacros.h"
#include "TraceMacros.h"

#undef DBG
#define DBG DROP


namespace isab {
	/**
	* Standard constructor for Route class.
	* Sets the inital values of the member variables.
	* 
	* @param log   Used to log output.
	*/
	Route::Route(Log *log) : 
m_mergeBuf(ExpectedHeaderLength > RouteDatumSize ?
ExpectedHeaderLength : RouteDatumSize),
					   m_decoderState(Idle), m_stringTable(NULL), m_colordata(NULL), m_log(log)
{
	m_routevector.reserve(InitialRouteDataSize);
	m_fileHeader.version = 0;
	strcpy(m_fileHeader.fileName, "");
	m_fileHeader.routeVectorSize = 0;
	m_routeHeader.stringLength = 0;
}

Route::~Route()
{
	if (m_stringTable) {
		delete[] m_stringTable;
	}
	if (m_colordata) {
		delete[] m_colordata;
	}
}

Nav2Error::ErrorNbr Route::prepareNewRoute(int length, 
										   bool routeOrigFromGPS,
										   int32 origLat, int32 origLon,
										   int32 destLat, int32 destLon) 
{
	//DBG("prepareNewRoute(%i)", length);
	if (length < ExpectedHeaderLength) {
		//WARN("Route::prepareNewRoute: not even a header\n");
		m_decoderState = IgnoringRestOfRoute;
		return Nav2Error::NAVTASK_ROUTE_INVALID;
	}
	if (m_stringTable) {
		delete[] m_stringTable;
		m_stringTable = NULL;
		m_routeHeader.stringLength = 0;
	}
	if (m_colordata) {
		delete[] m_colordata;
		m_colordata = NULL;
		m_routeHeader.colorTableSize = 0;
	}

	/** Clear the routevector to start putting new routedatums in 
	* the beginning of it when we receive a new route. */
	std::vector<RouteDatum> tmpEmptyVector;
	tmpEmptyVector.clear();
	m_routevector.swap(tmpEmptyVector);
	m_routevector.reserve(InitialRouteDataSize);

	m_decoderState = ReadingHeader;
	//m_numData = 0;
	m_expectedDataSize = length;
	m_dataReceivedSoFar = 0;
	m_mergeBuf.clear();
	m_stringBytes = 0;

	m_routeHeader.origLat = origLat;
	m_routeHeader.origLon = origLon;
	m_routeHeader.destLat = destLat;
	m_routeHeader.destLon = destLon;

	m_routeHeader.routeOrigFromGPS = routeOrigFromGPS;

	return Nav2Error::NO_ERRORS;
}

int Route::checkFinishedRoute()
{

	/* FIXME - extra sanity checks here on the newly downloaded route. */
	return 1;
}

void Route::buildStringIndex()
{
	uint8 *ptr = m_stringTable;
	uint8 *nextPtr;
	int length = m_routeHeader.stringLength;

	m_stringIndex.erase(m_stringIndex.begin(), m_stringIndex.end());
	do {
		/* Remember this string, then search for the end of it to locate
		* the beginning of the next string */
		m_stringIndex.push_back(ptr);
//		TRACE_DBG("string is : '%s'.", ptr);
		nextPtr = (uint8 *)(memchr(ptr, 0, length));
		if (nextPtr) {
			nextPtr++; /* Skip the terminating zero */
			length -= (nextPtr - ptr);
		}
		ptr = nextPtr;
	} while (ptr && (length > 0) );

}

void Route::buildColorTable()
{
	RGBColor nextColor = {0xff,0xff,0xff};
	if( m_routeHeader.colorTableSize%3 ){
		//Somethings wrong with the data, but we make sure there's one color
		m_colorvector.push_back( nextColor );
		//TRACE_DBG("Error in tha hood");
	}
	else if( !m_routeHeader.colorTableSize ){
		//There's no color table, but we make sure there's one color
		m_colorvector.push_back( nextColor );
		//TRACE_DBG("Error in tha hood2");
	}
	else{
		int nbrColors = m_routeHeader.colorTableSize;
		for( int i=0; i < nbrColors; i+=3 ){
			nextColor.red = m_colordata[i];
			nextColor.green = m_colordata[i+1];
			nextColor.blue = m_colordata[i+2];
			m_colorvector.push_back( nextColor );
			
// 			TRACE_DBG("Red value '%d'.", nextColor.red);
// 			TRACE_DBG("Green value '%d'.", nextColor.green);
// 			TRACE_DBG("Blue value '%d'.", nextColor.blue);
		}
	}
}


int Route::routeChunk(bool failedRoute, int chunkLength, const uint8 *chunkData,
					  Nav2Error::ErrorNbr &errval)
{
	//DBG("routeChunk(%i, %i, xxxxx)", failedRoute, chunkLength);
	RouteDatum rdVectorElement;

	/* An error message should be sent by NSC or a reply is sent from NSC to 
	* UiCtrl. No sure if we should do something if the route reply is 
	* broken and NavTask was the initiator of the route. This occurs on
	* truncated routes. */
	if (failedRoute) {
		m_decoderState = Idle;
		errval = Nav2Error::NO_ERRORS;
		return -1;
	}

	while (chunkLength > 0) {
		//DBG("State %i, Chunk left:%i", m_decoderState, chunkLength);
		switch (m_decoderState) {
			case Idle:
				//WARN("Got chunk in Idle\n");
				errval = Nav2Error::NAVTASK_NSC_OUT_OF_SYNC;
				return -1;

			case IgnoringRestOfRoute:
				DBG("Still ignoring rest of route...");
				/* Error was already reported when transitioning to this state - ignoring */
				errval = Nav2Error::NO_ERRORS;
				return -1;   

			case ReadingHeader:
				{
					int headerLeft = ExpectedHeaderLength - m_mergeBuf.getLength();
					if ( headerLeft <= chunkLength) {
						m_mergeBuf.writeNextByteArray(chunkData, headerLeft);
						m_routeHeader.routeId   = m_mergeBuf.readNext64bit();
						//read route bounding box
						m_routeHeader.topLat    = m_mergeBuf.readNextUnaligned32bit();
						m_routeHeader.leftLon   = m_mergeBuf.readNextUnaligned32bit();
						m_routeHeader.bottomLat = m_mergeBuf.readNextUnaligned32bit();
						m_routeHeader.rightLon  = m_mergeBuf.readNextUnaligned32bit();
						m_routeHeader.truncatedDistance = m_mergeBuf.readNext32bit();
						m_mergeBuf.readNext32bit();   // Used to be m_distToNextWptFromTrunc
						m_routeHeader.phoneHomeDistance = m_mergeBuf.readNext32bit();
						m_routeHeader.stringLength      = m_mergeBuf.readNext16bit();
						m_routeHeader.colorTableSize    = m_mergeBuf.readNext16bit();
						DBG("truncatedDistance:%i, phoneHomeDistance:%i, stringLength:%i", 
							m_routeHeader.truncatedDistance, m_routeHeader.phoneHomeDistance, m_routeHeader.stringLength);
						if (m_routeHeader.stringLength+m_routeHeader.colorTableSize > (m_expectedDataSize-ExpectedHeaderLength)) {
							//WARN("Route::routeChunk:stringTable & colorTable longer than packet\n");
							m_decoderState = Idle;
							errval = Nav2Error::NAVTASK_ROUTE_INVALID;
							return -1;
						}
						m_stringTable = new uint8[m_routeHeader.stringLength];
						m_stringBytes = 0;
						m_dataReceivedSoFar += headerLeft;
						chunkLength -= headerLeft;
						chunkData += headerLeft;
						m_decoderState = ReadingColorTable;
					} else {
						m_mergeBuf.writeNextByteArray(chunkData, chunkLength);
						m_dataReceivedSoFar += chunkLength;
						chunkLength = 0;
					}
				}
				break;

			case ReadingColorTable:
				{
					int colorTableSize = m_routeHeader.colorTableSize;
					m_colordata = new uint8[colorTableSize];
					//TRACE_DBG("colorTableSize == '%d'.", colorTableSize);
					if( colorTableSize ){
						memcpy(m_colordata, chunkData, colorTableSize);
						m_dataReceivedSoFar += colorTableSize;
						chunkLength -= colorTableSize;
						chunkData += colorTableSize;
						buildColorTable();
					}
					m_decoderState = ReadingStringTable;
				}
				break;

			case ReadingStringTable:
				{
					int stringLeft = m_routeHeader.stringLength - m_stringBytes;
					if (stringLeft <= chunkLength) {
						memcpy(m_stringTable + m_stringBytes,
							chunkData, stringLeft);
						m_dataReceivedSoFar += stringLeft;
						chunkLength -= stringLeft;
						chunkData += stringLeft;
						buildStringIndex();
						m_decoderState = ReadingRouteDatum;
					} else {
						memcpy(m_stringTable + m_stringBytes,
							chunkData, chunkLength);
						m_stringBytes += chunkLength;
						m_dataReceivedSoFar += chunkLength;
						chunkLength = 0;
					}
				}
				break;

			case ReadingRouteIncompleteDatum:
				{
					/* A chunk may contain a partial datum at the beginning,
					* a number of datums in the middle and a partial datum
					* at the end. */
					int bytesLeft = RouteDatumSize - m_mergeBuf.getLength();
					if (bytesLeft <= chunkLength) {
						m_mergeBuf.writeNextByteArray(chunkData, bytesLeft);
						chunkLength -= bytesLeft;
						chunkData += bytesLeft;
						m_dataReceivedSoFar += bytesLeft;
						//m_route[m_numData++].fill(m_mergeBuf.accessRawData(0));
						// Dynamic version
						rdVectorElement.fill(m_mergeBuf.accessRawData(0));
						m_routevector.push_back(rdVectorElement);
						// ---
						if ( (m_expectedDataSize - m_dataReceivedSoFar) < 
							RouteDatumSize) {
								m_decoderState = Idle;
								return checkFinishedRoute();
						}
						m_decoderState = ReadingRouteDatum;
					} else {
						m_mergeBuf.writeNextByteArray(chunkData, chunkLength);
						m_dataReceivedSoFar += chunkLength;
						chunkLength = 0;
					}
				}
				break;

			case ReadingRouteDatum:
				{
					if (chunkLength >= RouteDatumSize) {
						//m_route[m_numData++].fill(chunkData);
						// Dynamic version
						rdVectorElement.fill(chunkData);
						m_routevector.push_back(rdVectorElement);
						// ---
						chunkData += RouteDatumSize;
						chunkLength -= RouteDatumSize;
						m_dataReceivedSoFar += RouteDatumSize;
						if ( (m_expectedDataSize - m_dataReceivedSoFar) < 
							RouteDatumSize) {
								m_decoderState = Idle;
								return checkFinishedRoute();
						}
					} else {
						m_mergeBuf.clear();
						m_mergeBuf.writeNextByteArray(chunkData, chunkLength);
						m_dataReceivedSoFar += chunkLength;
						chunkLength = 0;
						m_decoderState = ReadingRouteIncompleteDatum;
					}
				}
				break;

		} /* switch */
	} /* while */
	return 0;
}

int Route::writeRoute(const char* aFileName)
{
	//DBG("Route::writeRoute started");     

	m_fileHeader.version = (float)0.01;
	strcpy(m_fileHeader.fileName, aFileName);
	m_fileHeader.routeVectorSize = m_routevector.size();

	FILE* pSaveFile = fopen(aFileName, "wb");
	if (pSaveFile != NULL) {
		fwrite(reinterpret_cast<uint8*>(&m_fileHeader), sizeof(m_fileHeader), 1, pSaveFile);
		fwrite(reinterpret_cast<uint8*>(&m_routeHeader), sizeof(m_routeHeader), 1, pSaveFile);
		fwrite(m_stringTable, m_routeHeader.stringLength*sizeof(*m_stringTable), 1, pSaveFile);
		fwrite(reinterpret_cast<uint8*>(&m_routevector.front()), m_fileHeader.routeVectorSize*RouteDatumSize, 1, pSaveFile);
		fwrite(m_colordata, m_routeHeader.colorTableSize*sizeof(*m_colordata), 1, pSaveFile);
		fclose(pSaveFile);
	}
	else {
		return -1;
	}

	return 0;
}

int Route::readRoute(const char* aFileName)
{
	//DBG("Route::readRoute started");        

	std::vector<RouteDatum> tmpEmptyVector;
	tmpEmptyVector.clear();
	m_routevector.swap(tmpEmptyVector);
	m_routevector.reserve(InitialRouteDataSize);

	if (m_stringTable) {
		delete[] m_stringTable;
	}

	if (m_colordata) {
		delete[] m_colordata;
	}

	FILE* pReadFile = fopen(aFileName, "rb");
	if (pReadFile != NULL) {
		fread(reinterpret_cast<uint8*>(&m_fileHeader), sizeof(m_fileHeader), 1, pReadFile);
		fread(reinterpret_cast<uint8*>(&m_routeHeader), sizeof(m_routeHeader), 1, pReadFile);
		//Above we delete stringtable first depending on if we have one already.
		m_stringTable = new uint8[m_routeHeader.stringLength];
		fread(m_stringTable, m_routeHeader.stringLength*sizeof(*m_stringTable), 1, pReadFile);
		m_routevector.resize(m_fileHeader.routeVectorSize);
		fread(reinterpret_cast<uint8*>(&m_routevector.front()), m_fileHeader.routeVectorSize*RouteDatumSize, 1, pReadFile);
		//Above we delete colordata first depending on if we have one already.
		m_colordata = new uint8[m_routeHeader.colorTableSize];
		fread(m_colordata, m_routeHeader.colorTableSize*sizeof(*m_colordata), 1, pReadFile);
		fclose(pReadFile);
	}
	else {
		return -1;
	}
	// Build new StringIndex from the stringTable we just loaded.
	buildStringIndex();
	// Build the color table
	buildColorTable();

	return 0;
}

int Route::deleteRoute(const char* aFileName)
{
	//INFO("Route::deleteRoute started on file \"%s\"", aFileName);
	int retval = remove(aFileName);
	return retval;
}

RouteDatum* Route::getDatum(int index) 
{
	return &m_routevector[index];
}

int Route::findFirstPoint(class Point &p)
{
	//p.getFirstPoint(m_route, this);
	return p.getFirstPoint(&m_routevector.front(), this);
}


} /* namespace isab */

