/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CellIDRequestData.h"

#include "nav2util.h"

CellIDRequestData::CellIDRequestData(const char* networkType, 
                                     const char* signalStrength)
{
   if (networkType) {
      m_networkType = isab::strdup_new(networkType);
   } else {
      m_networkType = isab::strdup_new("");
   }
   if (signalStrength) {
      m_signalStrength = isab::strdup_new(signalStrength);
   } else {
      m_signalStrength = isab::strdup_new("");
   }
}

CellIDRequestData::~CellIDRequestData()
{
   delete[] m_networkType;
   delete[] m_signalStrength;
}

TGPPCellIDRequestData::TGPPCellIDRequestData(const char* networkType, 
                                             const char* signalStrength,
                                             const char* currentMCC,
                                             const char* currentMNC,
                                             const char* currentLAC,
                                             const char* cellID)
   : CellIDRequestData(networkType, signalStrength)
{
   if (currentMCC) {
      m_currentMCC = isab::strdup_new(currentMCC); 
   } else {
      m_currentMCC = isab::strdup_new("");
   }
   if (currentMNC) {
      m_currentMNC = isab::strdup_new(currentMNC);
   } else {
      m_currentMNC = isab::strdup_new("");
   }
   if (currentLAC) {
      m_currentLAC = isab::strdup_new(currentLAC);
   } else {
      m_currentLAC = isab::strdup_new("");
   }
   if (cellID) {
      m_cellID = isab::strdup_new(cellID);
   } else {
      m_cellID = isab::strdup_new("");
   }
}

TGPPCellIDRequestData::~TGPPCellIDRequestData()
{
   delete[] m_currentMCC;
   delete[] m_currentMNC;
   delete[] m_currentLAC;
   delete[] m_cellID;
}

void TGPPCellIDRequestData::addParamsTo(isab::NParamBlock& rd) const
{
   // Network type
   rd.addParam(isab::NParam(6200, m_networkType));
   // Current SignalStrength
   rd.addParam(isab::NParam(6201, m_signalStrength));
   // Current MCC
   rd.addParam(isab::NParam(6202, m_currentMCC));
   // Current MNC
   rd.addParam(isab::NParam(6203, m_currentMNC));
   // Current LAC
   rd.addParam(isab::NParam(6204, m_currentLAC));
   // Current cellID
   rd.addParam(isab::NParam(6205, m_cellID));
}

CDMACellIDRequestData::CDMACellIDRequestData(const char* networkType,
                                             const char* signalStrength,
                                             const char* currentSID,
                                             const char* currentNID,
                                             const char* currentBID)
   : CellIDRequestData(networkType, signalStrength)
{
   if (currentSID) {
      m_currentSID = isab::strdup_new(currentSID); 
   } else {
      m_currentSID = isab::strdup_new("");
   }
   if (currentNID) {
      m_currentNID = isab::strdup_new(currentNID);
   } else {
      m_currentSID = isab::strdup_new("");
   }
   if (currentBID) {
      m_currentBID = isab::strdup_new(currentBID);
   } else {
      m_currentSID = isab::strdup_new("");
   }
}

CDMACellIDRequestData::~CDMACellIDRequestData()
{
   delete[] m_currentSID;
   delete[] m_currentNID;
   delete[] m_currentBID;
}

void CDMACellIDRequestData::addParamsTo(isab::NParamBlock& rd) const
{
   // Network type
   rd.addParam(isab::NParam(6200, m_networkType));
   // Current SignalStrength
   rd.addParam(isab::NParam(6201, m_signalStrength));
   // CurrentSID
   rd.addParam(isab::NParam(6206, m_currentSID));
   // CurrentNID
   rd.addParam(isab::NParam(6207, m_currentNID));
   // CurrentBID
   rd.addParam(isab::NParam(6208, m_currentBID));
}

IDENCellIDRequestData::IDENCellIDRequestData(const char* networkType,
                                             const char* signalStrength,
                                             const char* currentMCC,
                                             const char* currentDNC,
                                             const char* currentSAID,
                                             const char* currentLLAID,
                                             const char* currentCELLID)
   : CellIDRequestData(networkType, signalStrength)
{
   if (currentMCC) {
      m_currentMCC = isab::strdup_new(currentMCC);
   } else {
      m_currentMCC = isab::strdup_new("");
   }
   if (currentDNC) {
      m_currentDNC = isab::strdup_new(currentDNC);
   } else {
      m_currentDNC = isab::strdup_new("");
   }
   if (currentSAID) {
      m_currentSAID = isab::strdup_new(currentSAID);
   } else {
      m_currentSAID = isab::strdup_new("");
   }
   if (currentLLAID) {
      m_currentLLAID = isab::strdup_new(currentLLAID);
   } else {
      m_currentLLAID = isab::strdup_new("");
   }
   if (currentCELLID) {
      m_currentCELLID = isab::strdup_new(currentCELLID);
   } else {
      m_currentCELLID = isab::strdup_new("");
   }
}

IDENCellIDRequestData::~IDENCellIDRequestData()
{
   delete[] m_currentMCC;
   delete[] m_currentDNC;
   delete[] m_currentSAID;
   delete[] m_currentLLAID;
   delete[] m_currentCELLID;
}

void IDENCellIDRequestData::addParamsTo(isab::NParamBlock& rd) const
{
   // Network type
   rd.addParam(isab::NParam(6200, m_networkType));
   // Current SignalStrength
   rd.addParam(isab::NParam(6201, m_signalStrength));
   // CurrentMCC
   rd.addParam(isab::NParam(6209, m_currentMCC));
   // CurrentDNC
   rd.addParam(isab::NParam(6210, m_currentDNC));
   // CurrentSA_ID
   rd.addParam(isab::NParam(6211, m_currentSAID));
   // CurrentLLA_ID
   rd.addParam(isab::NParam(6212, m_currentLLAID));
   // CurrentCell_ID
   rd.addParam(isab::NParam(6213, m_currentCELLID));
}
