/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef CELL_ID_REQUEST_DATA_H
#define CELL_ID_REQUEST_DATA_H

#include "arch.h"
#include "NParam.h"
#include "NParamBlock.h"

/**
 * Abstract super class for all cellID lookup request data.
 */
class CellIDRequestData 
{
public:
   CellIDRequestData(const char* networkType, 
                     const char* signalStrength);

   virtual ~CellIDRequestData();

   virtual void addParamsTo(isab::NParamBlock& rd) const = 0;
   
protected:
   char* m_networkType;
   char* m_signalStrength;
};

/**
 * CellID lookup request data for 3GPP, gsm, networks.
 */
class TGPPCellIDRequestData : public CellIDRequestData 
{
public:
   TGPPCellIDRequestData(const char* networkType, 
                         const char* signalStrength,
                         const char* currentMCC,
                         const char* currentMNC,
                         const char* currentLAC,
                         const char* cellID);

   virtual ~TGPPCellIDRequestData();

   void addParamsTo(isab::NParamBlock& rd) const;

private:
   char* m_currentMCC;
   char* m_currentMNC;
   char* m_currentLAC;
   char* m_cellID;
};

/**
 * CellID lookup request data for CDMA networks.
 */
class CDMACellIDRequestData : public CellIDRequestData 
{
public:
   CDMACellIDRequestData(const char* networkType,
                         const char* signalStrength,
                         const char* currentSID,
                         const char* currentNID,
                         const char* currentBID);

   virtual ~CDMACellIDRequestData();

   void addParamsTo(isab::NParamBlock& rd) const;

private:
   char* m_currentSID;
   char* m_currentNID;
   char* m_currentBID;
};

/**
 * CellID lookup request data for iDEN networks.
 */
class IDENCellIDRequestData : public CellIDRequestData 
{
public:
   IDENCellIDRequestData(const char* networkType,
                         const char* signalStrength,
                         const char* currentMCC,
                         const char* currentDNC,
                         const char* currentSAID,
                         const char* currentLLAID,
                         const char* currentCELLID);

   virtual ~IDENCellIDRequestData();
   
   void addParamsTo(isab::NParamBlock& rd) const;
   
private:
   char* m_currentMCC;
   char* m_currentDNC;
   char* m_currentSAID;
   char* m_currentLLAID;
   char* m_currentCELLID;
};

#endif //CELL_ID_REQUEST_DATA_H
