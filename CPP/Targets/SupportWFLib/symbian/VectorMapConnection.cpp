/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#include "machine.h"
#include "VectorMapConnection.h"
#include "GuiProt/ServerEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "GuiProt/GuiProtMessageSender.h"
#include "HttpClientConnectionListener.h"


CVectorMapConnection::CVectorMapConnection(class MMessageSender* aSender)
{
   iSender = aSender;
   iListener = NULL;
}

void CVectorMapConnection::SingleMapReply(const class isab::DataGuiMess& reply)
{
   if(iListener){
      iListener->bytesReceived(reply.getMessageID(), //id
                               200,                   //http-request status
                               reply.getData(), //data
                               0,  //startoffset,
                               reply.getSize(), //nbrBytes 
                               reply.getSize()); //contentLength
   }
}

void CVectorMapConnection::MultiMapReply(const class isab::DataGuiMess& reply)
{
   if(iListener){
      iListener->bytesReceived(reply.getMessageID(), //id
                               200,                   //http-request status
                               reply.getData(), //data
                               0,  //startoffset,
                               reply.getSize(), //nbrBytes 
                               reply.getSize()); //contentLength
   }
}


void CVectorMapConnection::MapErrorReply(uint16 aRequestId)
{
   if(iListener){
      iListener->bytesReceived(aRequestId, //id
                               900,                   //http-request status - FIXME - use better code?
                               NULL, //data
                               -1,  //startoffset,
                               -1, //nbrBytes 
                               -1); //contentLength
   }
}


int CVectorMapConnection::requestOne(const char* descr)
{
   isab::GenericGuiMess request(isab::GuiProtEnums::GET_VECTOR_MAP, descr);
   int ret = iSender->SendMessageL(request);
   request.deleteMembers();
   return ret;
}

int CVectorMapConnection::requestMany(const uint8* buf, int nbrBytes)
{
   isab::DataGuiMess request(isab::GuiProtEnums::GET_MULTI_VECTOR_MAP, 
                             nbrBytes, buf);
   int ret = iSender->SendMessageL(request);
   request.deleteMembers();
   return ret;
}

void CVectorMapConnection::setListener(HttpClientConnectionListener* listener)
{
   iListener = listener;
}

