/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef TMAP_IMAGE_HANDLER_H
#define TMAP_IMAGE_HANDLER_H

#include <map>
#include "arch.h"
#include "HttpTunnelGuiMess.h"
#include "GuiProt/GuiProtMessageHandler.h"
#include "GuiProt/GuiProtMess.h"

/**
 * Observer that will receive callbacks when
 * a request is done to FetchImage. 
 * Interface that has to be implemented
 * by the receiver of ImageReceived
 * class TmapImageObserver
 */
class TmapImageObserver
{
public:

   /**
    * Function gets called by TmapImageHandler
    * when a requested image is downloaded.
    * @param imageName, The name of the downloaded image
    */
   virtual void ImageReceived(const char* imageName) = 0;

   /**
    * Function gets called by TmapImageHandler
    * when a requested image failed to be downloaded.
    * @param imageName, The name of the image that failed to be downloaded
    */
   virtual void ImageReceivedError(const char* imageName) = 0;

};

/**
 * Class that handles image downloading, used
 * by TmapDispatcher when images is
 * needed from the server.
 * class TmapImageHandler
 */
class TmapImageHandler : 
   public GuiProtMessageReceiver,
   public isab::HttpTunnelGuiMessObserver
{
public:

   /**
    * Constructor that creates an instance of this class.
    * @param messageSender, Used for sending messages to Nav2
    * @param observer, The observer that will receive callbacks
    */
   TmapImageHandler(GuiProtMessageHandler& messageSender,
                    TmapImageObserver& observer);

   /**
    * Destructor
    */
   virtual ~TmapImageHandler();

   /**
    * Sends a request to the server for downloading a image.
    * Takes the full path with image name where to write the image when 
    * it has been downloaded. The name of the image needs to be the same 
    * on the server as on the local file system.
    * @param imageName, The name and path of the image to be downloaded
    */
   void FetchImage(const char* imageName);

   /**
    * Typedef for iterator
    */
   typedef std::map<uint32, isab::Buffer*>::iterator bufMapIt_t;

   /**
    * Typedef for map
    */
   typedef std::map<uint32, isab::Buffer*> bufMap_t;


public: // From base classes

   /**
    * Callback from HttpTunnelGuiMess class when
    * we receied a message from the GuiProtMessageHandler
    * with a chunk of data. The HttpTunnelGuiMess parses the message
    * for us and that results in a call to this function. Here we can
    * check if we have gotten the complete image or if we need to fetch
    * more chunks.
    * @param seqId      Sequence id from message
    * @param startByte  Index of first byte in data
    * @param endByte    Index of last byte in data
    * @param total      Total number of bytes in the file
    * @param url        URL
    * @param data       Data pointer
    * @param dataLength Length of the data
    */
   void DataReceived(uint32 seqId,
                     uint32 startByte,
                     uint32 endByte,
                     uint32 total,
                     const char* url,
                     const char* data,
                     uint32 dataLength); 

   /**
    * From GuiProtMessageReceiver, called when received a reply
    * to a request.
    * @param mess, The message containing the data that was requested
    */
   bool GuiProtReceiveMessage(isab::GuiProtMess* mess);

private:

   /**
    * Writes down the data when all chunks are downloaded
    * @param The sequence id that corresponds to this request,
    *        that is also the index in m_imageNamesVec that holds 
    *        the names of the images requested from the server.
    */
   bool WriteToFile(uint32 seqId);

   /**
    * Calles HttpTunnelGuiMess::CreateTunnelMess to put together
    * a message and then sends this message to the server.
    * @param seqId, The sequence id for this request
    * @param aUrl, The url to get the data from
    * @startByte, The startByte, if a new request this is zero.
    *             If its not the first chunc this is the endByte+1
    *             see DataReceived.
    */
   void FetchData(uint32 seqId, const char* aUrl, uint32 startByte);

private:
   
   /// Used for sending messages to Nav2/server
   GuiProtMessageSender& m_messageSender;
   /// The observer to be called when an image is downloaded
   TmapImageObserver& m_observer;
   /// The id of a request
   uint32 m_seqId;
   /// Map containing buffers received from the server, index is
   /// the sequence id for the request
   bufMap_t m_dataBuffers;
   /// Vector containing all requested names
   std::vector<char*> m_imageNamesVec;
};

#endif
