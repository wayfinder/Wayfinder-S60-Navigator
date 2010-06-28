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
#include "MsgBufferEnums.h"
#include "Module.h"
#include "Serial.h"
#include "Mutex.h"
#include "QueueSerial.h"

#include <map>
#include "GuiProt/GuiProtEnums.h"
#ifndef NO_LOG_OUTPUT
# define NO_LOG_OUTPUT
#endif
#include "LogMacros.h"

#include "Buffer.h"
#include "MsgBuffer.h"

/// The number of buffers that can be stored in the outbound queue
/// before the overflow policy is invoked.
#define QS_OVERFLOW_LIMIT 200

isab::QueueSerial::QueueSerial(const char* name, class OverflowPolicy* policy,
                               int microSecondsPoll) :
   Module(name), m_connectState(CONNECTING), m_pollInterval(microSecondsPoll),
   m_policy(policy)
#ifdef __SYMBIAN32__
   ,m_guiSideRequestStatus(NULL)
#endif
{
}

void isab::QueueSerial::decodedStartupComplete()
{
   DBG("decodedStartupComplete");
   if(!m_incoming.empty() && rootPublic()){
      connect();
      while(! m_incoming.empty()){
         Buffer* buf = m_incoming.front();
         m_incoming.pop();
         write(buf->accessRawData(0), buf->getLength());
         delete buf;
      }
   } else if(rootPublic()){
      rootPublic()->connectionNotify(m_connectState);
   }
}

bool isab::QueueSerial::write(const uint8* data, int length)
{
   bool ret = false;
   if(isAlive()){
      if(rootPublic() && m_connectState != CONNECTED){
         m_connectState = CONNECTED;
         rootPublic()->connectionNotify(CONNECTED);
      }
      m_queue->lock(); 
      if(rootPublic()){
         //DBGDUMP("write", data, length);
         uint32 src = rootPublic()->receiveData(length, data);
         src = src;
         DBG("Message sent with id: %0#10"PRIx32, src);
         ret = true;
      } else {
         Buffer* buf = new Buffer(0);
         buf->writeNextByteArray(data, length);
         m_incoming.push(buf);
      }
      m_queue->unlock();
   }
   return ret;
}

void isab::QueueSerial::connect(){
   DBG("connect");
   m_connectState = CONNECTED;
   if(rootPublic()){
      rootPublic()->connectionNotify(CONNECTED);
   }
}

void isab::QueueSerial::disconnect(){
   DBG("disconnect");
   m_connectState = DISCONNECTING;
   if(isAlive()){
      rootPublic()->connectionNotify(DISCONNECTING);
   }
}

int isab::QueueSerial::read( uint8* data, int maxlength)
{
   if(m_connectState != CONNECTED){
      m_connectState = CONNECTED;
      rootPublic()->connectionNotify(CONNECTED);
   }
   int retval = 0;
   m_outMutex.lock();
   while(!m_outQue.empty() && retval < maxlength){
      Buffer* front = m_outQue.front();
      int n = front->readNextByteArray(data + retval, maxlength - retval);
      retval += n;
      if(front->remaining() <= 0){
         m_outQue.pop_front();
         delete front;
      }
   }
   m_outMutex.unlock();
   //DBGDUMP("read data", data, retval);
   return retval;
}

bool isab::QueueSerial::read(isab::Buffer* buf)
{
   bool ret = false;
   if((ret = isAlive())){
      if(m_connectState != CONNECTED){
         m_connectState = CONNECTED;
         rootPublic()->connectionNotify(CONNECTED);
      }
      m_outMutex.lock();
      buf->reserve(buf->getLength() + internalAvailable());
      DBG("read: outque size: %d", m_outQue.size());
      while(!m_outQue.empty()){
         Buffer* front = m_outQue.front();
         //DBGDUMP("front", front->accessRawData(0), front->getLength());
         buf->writeNextByteArray(front->accessRawData(0), front->getLength());
         m_outQue.pop_front();
         delete front;
      }
      //DBGDUMP("read", buf->accessRawData(0), buf->getLength());
      m_outMutex.unlock();
   }
   return ret;
}

#ifdef __SYMBIAN32__
void isab::QueueSerial::armReader(TRequestStatus *aStatus)
{
   m_outMutex.lock();
   // It is a major error if this object already is armed. FIMXE - check.
   if (m_outQue.empty()) {
      m_guiSideRequestStatus = aStatus;
      RThread thisThread;
      m_guiSideThread = thisThread.Id();
      thisThread.Close();
      *aStatus = KRequestPending;
   } else {
      User::RequestComplete(aStatus, KErrNone);
   }
   m_outMutex.unlock();
}

void isab::QueueSerial::cancelArm()
{
   m_outMutex.lock();
   if (m_guiSideRequestStatus) {
      RThread otherThread;
      otherThread.Open(m_guiSideThread);
      otherThread.RequestComplete(m_guiSideRequestStatus, KErrCancel);
      otherThread.Close();
      m_guiSideRequestStatus=NULL;
   }
   m_outMutex.unlock();
}
#endif

int isab::QueueSerial::internalAvailable() const
{
   int retval = 0;
   Container::const_iterator q;
   for(q = m_outQue.begin(); q != m_outQue.end(); ++q){
      retval = (*q)->remaining();
   }
   return retval;
}

int isab::QueueSerial::available() const
{
   m_outMutex.lock();
   int retval = internalAvailable();
   m_outMutex.unlock();
   DBG("Available: %d", retval);
   return retval;
}

bool isab::QueueSerial::empty() const
{
   DBG("empty? %s", m_outQue.empty() ? "true": "false");
   return m_outQue.empty();
}

isab::SerialProviderPublic * isab::QueueSerial::newPublicSerial()
{
   DBG("newPublicSerial");
   SerialProviderPublic* spp = new SerialProviderPublic(m_queue);
   return spp;
}

void isab::QueueSerial::decodedSendData(int length, const uint8 *data, 
                                         uint32 /*src*/)
{
   DBG("enter decodedSendData");
   m_outMutex.lock();
   //DBGDUMP("decodedSendData", data, length);
   Buffer* buf = new Buffer(length);
   buf->writeNextByteArray(data, length);
   m_outQue.push_back(buf);
   DBG("push_back, outque size: %d", m_outQue.size());
   if(m_outQue.size() > QS_OVERFLOW_LIMIT){
      INFO("Queue overflow, removing excess messages now");
      removeOverflow();
   }
#ifdef __SYMBIAN32__
   if (m_guiSideRequestStatus) {
      DBG("complete sprocket, outque size: %d", m_outQue.size());
      // Notify the other (non-Nav2) side reader
      RThread otherThread;
      otherThread.Open(m_guiSideThread);
      otherThread.RequestComplete(m_guiSideRequestStatus, KErrNone);
      otherThread.Close();
      m_guiSideRequestStatus=NULL;
   }
#endif
   m_outMutex.unlock();
   DBG("exit decodedSendData");
}


void isab::QueueSerial::decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                               const char *method, 
                                               uint32 /*src*/)
{
   switch(ctrl){
   case Module::CONNECT:
      WARN("CONNECT %s", method);
      break;
   case Module::DISCONNECT:
      WARN("DISCONNECT %s", method);
      break;
   case Module::QUERY:
      WARN("QUERY %s", method);
      rootPublic()->connectionNotify(m_connectState);
      break;
   default:
      ERR("ConnectionCtrl: Unknown ctrl value: %d", ctrl);
   }
}

isab::SerialConsumerPublic * isab::QueueSerial::rootPublic()
{
   return static_cast<SerialConsumerPublic*>(m_rawRootPublic);
}


isab::MsgBuffer * isab::QueueSerial::dispatch(MsgBuffer *buf)
{
   if(buf) buf = m_providerDecoder.dispatch(buf, this);
   if(buf) buf = Module::dispatch(buf);
   return buf;
}

isab::QueueSerial::~QueueSerial()
{
   delete m_policy;
   while(!m_outQue.empty()){
      Buffer* front = m_outQue.front();
      m_outQue.pop_front();
      delete front;
   }
}

void isab::KeepLatestGuiMessage::clean(QueueSerial::Container& deq)
{
   //this method presumes that each buffer in the deque contains
   //exactly one complete Gui-message
   typedef std::map<uint16, Buffer*> Tree;
   Tree keepers;
   while(!deq.empty()){
      Buffer* front = deq.front();
      deq.pop_front();
      int pos = front->setReadPos(0);
      uint8 protocol = *(front->accessRawData(0));
      uint32 length = front->readNextUnaligned32bit() & 0x0ffffff;
      length = length;
      uint16 message = front->readNextUnaligned16bit();
      switch(protocol){
      case 0:
      case 3:
         break;
      }
      std::map<uint16, Buffer*>::iterator prev = keepers.find(message);
      if(prev != keepers.end()){
         delete (*prev).second;
         (*prev).second = front;
      } else {
         keepers[message] = front;
      }
      front->setReadPos(pos);
   }
   for(Tree::iterator p = keepers.begin(); p != keepers.end(); ++p){
      deq.push_back((*p).second);
   }
}

void isab::KeepLatestBuffer::clean(QueueSerial::Container& deq)
{
   while(deq.size() > 1){
      Buffer* tmp = deq.front();
      deq.pop_front();
      delete tmp;
   }
}

void isab::RemoveGpsAndRoute::clean(QueueSerial::Container& deq)
{
   QueueSerial::Container tmp;
   while(!deq.empty()){
      Buffer* front = deq.front();
      deq.pop_front();
      front->setReadPos(0);
      /*uint32 length = */ front->readNextUnaligned32bit();
      /*uint8 datatype = */front->readNext8bit();
      uint8 message = front->readNext8bit();
      front->setReadPos(0);
      switch(message){
      case isab::GuiProtEnums::UPDATE_POSITION_INFO:
         // case isab::GuiProtEnums::UPDATE_ROUTE_INFO:
         delete front;
      default:
         tmp.push_back(front);
      }
   }
   std::swap(tmp,deq);
}
