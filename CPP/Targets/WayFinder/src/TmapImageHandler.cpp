/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "nav2util.h"
#include "TmapImageHandler.h"

using namespace isab;


TmapImageHandler::TmapImageHandler(GuiProtMessageHandler& messageSender,
                                   TmapImageObserver& observer) :
   m_messageSender(messageSender), 
   m_observer(observer),
   m_seqId(0)
{
   //m_dataBuffers.reserve(5);
   m_imageNamesVec.reserve(5);
}
   
TmapImageHandler::~TmapImageHandler()
{
   for(bufMapIt_t it = m_dataBuffers.begin(); it != m_dataBuffers.end(); ++it) {
      delete it->second;
   }
   m_dataBuffers.clear();

   std::vector<char*>::iterator nit = m_imageNamesVec.begin();
   while(nit != m_imageNamesVec.end()) {
      delete[] (*nit);
      ++nit;
   }
   m_imageNamesVec.clear();
}

#define MAX_PACKET_LENGTH (60*1024) 
#define IMAGE_URL_PREFIX "http://TMap/"
#define IMAGE_PREFIX "B"

void TmapImageHandler::FetchImage(const char* imageName)
{
   char* pch = strrchr(imageName, '\\');
   pch++;

   char* completeName = new char[strlen(IMAGE_PREFIX) + strlen(IMAGE_URL_PREFIX) + 
                                 strlen(pch) + 1];
   strcpy(completeName, IMAGE_URL_PREFIX);
   strcat(completeName, IMAGE_PREFIX);
   strcat(completeName, pch);

   m_imageNamesVec.push_back(strdup_new(imageName));

   FetchData(m_seqId, completeName, 0);
   m_seqId++;

   delete[] completeName;

}

void TmapImageHandler::FetchData(uint32 seqId, const char* aUrl, uint32 startByte)
{
   isab::DataGuiMess *dgm = 
      isab::HttpTunnelGuiMess::CreateTunnelMess(1, seqId, 
                                                aUrl, 
                                                startByte);

   m_messageSender.SendMessage( *dgm, this );
   dgm->deleteMembers();
}

bool TmapImageHandler::GuiProtReceiveMessage(isab::GuiProtMess* mess)
{
   if( mess->getMessageType() == isab::GuiProtEnums::GUI_TUNNEL_DATA_REPLY || 
       mess->getMessageType() == isab::GuiProtEnums::GUI_TO_NGP_DATA_REPLY ) {
      isab::HttpTunnelGuiMess::ParseTunnelMess(static_cast<const isab::DataGuiMess*>(mess), this);
      return true;
   } else if( mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED ) {
      isab::RequestFailedMess* rf = static_cast<isab::RequestFailedMess*>(mess);
      //uint8 req_id = rf->getFailedRequestMessageNbr();
      TUint eNbr = rf->getErrorNbr();
      if (eNbr == isab::Nav2Error::NSC_EXPIRED_USER) {
         // XXX Do something if expired user not handle before??
      }
   }
   return false;
}

#define HEADER_DELIMITER "\r\n\r\n"

void TmapImageHandler::DataReceived(uint32 seqId,
                                              uint32 startByte,
                                              uint32 endByte,
                                              uint32 total,
                                              const char* url,
                                              const char* data,
                                              uint32 dataLength)
{
   char* start = const_cast<char*>(data);
   start = strstr(data, HEADER_DELIMITER);
   if (start == NULL) {
      // Ehm... Big problem. Set to Zero and hope for the best. 
      start = const_cast<char*>(data);
   } else {
      // Add length of CRLF.
      start += strlen(HEADER_DELIMITER);
   }

   //datalength without head and delimiter
   uint32 len = dataLength - (start - data);

   // See if this is the first chunk or if we already has
   // a part of this buffer
   bufMapIt_t it = m_dataBuffers.find(seqId);
   if (it == m_dataBuffers.end() ) {
      // New buffer from the server, create the buffer
      // and store it in the container
      isab::Buffer* buff = new isab::Buffer(dataLength);
      buff->writeNextByteArray((uint8*)start, len);
      m_dataBuffers.insert(std::make_pair(seqId, buff));      
   } else {
      // Already got a part of this buffer, append it
      // to the correct buffer
      it->second->writeNextByteArray((uint8*)start, len);
   }
   if ((endByte+1) != total && total > endByte) {
      // Need to rerequest for rest of data
      FetchData(seqId, url, endByte+1);
   } else {
      // Everything transferred
      if (WriteToFile(seqId)) {
         m_observer.ImageReceived(m_imageNamesVec.at(seqId));
      } else {
         m_observer.ImageReceivedError(m_imageNamesVec.at(seqId));
      }
   }
}

bool TmapImageHandler::WriteToFile(uint32 seqId)
{
   // Get the complete path and filename for the buffer with seqId
   char* fileName = m_imageNamesVec.at(seqId);
   if (fileName == NULL) {
      // No file exists
      return false;
   }
   // Create the file, if file already exists the content is erased.
   FILE* pSaveFile = fopen(fileName, "wb");
   if (pSaveFile == NULL) {
      return false;
   }
   // Get the buffer for writing
   bufMapIt_t it = m_dataBuffers.find(seqId);
   if (it != m_dataBuffers.end()) {
      isab::Buffer* imgBuf = it->second;
      if (imgBuf != NULL) {
         // Write the buffer to the file
         fwrite(imgBuf->accessRawData(0), imgBuf->getLength(), 1, pSaveFile);
         fclose(pSaveFile);
         return true;
      }
   }
   // No buffer, this should never happen
   fclose(pSaveFile);
   return false;
}
