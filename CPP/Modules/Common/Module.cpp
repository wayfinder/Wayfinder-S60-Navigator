/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//#undef NO_LOG_OUTPUT

#include "arch.h"
#include <stdlib.h>

#include "MsgBufferEnums.h"
#include "Module.h"
#include "ErrorModule.h"

#include "Buffer.h"
#include "MsgBuffer.h"

namespace isab {

#include "LogMacros.h"
#define DBG_CONNECT   DROP
#define DBG_STATUP    DBG
#define DBG_MSG       DROP
#define DBG_NEG_ADDR  DROP
#define DBG_MULTICAST DROP

Module::Module(const char *name, Log * l) : Thread(name), m_log(l), 
                             m_ownLog(false),
                             m_address(0xffffffff),
                             m_netmask(0xffffffff),
                             m_treeNetmask(0xffffffff),
                             m_lastRequestId(0),
                             m_rawRootPublic(NULL),
                             m_numLowerModules(0),
                             m_shutdownPrepared(false),
                             m_awaitingShutdownPrepareComplete(false),
                             m_awaitingShutdownComplete(false),
                             m_exiting(false)
{
   if (!m_log) {
      m_log = new Log(name);
      m_ownLog = true;
   }

   DBG_STATUP("Module(\"%s\") %p\n", name, this);
   m_queue = new ModuleQueue(name);
   m_errorProvider = new ErrorProviderPublic(this);
   m_shutdownTimer = m_queue->defineTimer();
   preStartSetup();
}

Module::~Module()
{
   delete m_errorProvider;
   delete m_queue;
   if (m_log) {
      DBG("~Module - called, m_ownLog %d", m_ownLog);
   }
   if (m_ownLog) {
      DBG("~Module - deleting log");
      delete m_log;
   }
}

void Module::preStartSetup()
{
   // Nothing to do here, may be overriden in subclasses.
}

void Module::relayMulticastUp(MsgBuffer *buf)
{
   /* Send towards the hub if possible, otherwise drop it */
   if (m_rawRootPublic) {
      m_rawRootPublic->preformedMessage(buf);
   } else {
      WARN("Module::relayMulticastUp - no rawRoot\n");
      delete(buf);
   }
}

void Module::relayMulticastDown(MsgBuffer *buf)
{
   int i;
   MsgBuffer * newMsg;
   int group = buf->getDestination() & 0x0000ffff;

   for (i=0 ; i < m_numLowerModules ; i++ ) {
      if (!m_lowerModules[i].orderedToShutdown && 
           m_lowerModules[i].subscriptions.find(group) !=
              m_lowerModules[i].subscriptions.end()) {
         newMsg = new MsgBuffer(*buf);
         m_lowerModules[i].theInterface->preformedMessage(newMsg);
      }
   }
}

void Module::run()
{
   MsgBuffer *buf;
   uint32 dst;

   buf = NULL;
   DBG_STATUP("Module::run()\n");
   while (!m_exiting) {
      DBG_MSG("Module::run - waiting\n");
      buf = m_queue->getMessage();

      /* See if this message is for us, our superiour module or one of
       * our inferiour modules */
      dst = buf->getDestination();
      DBG_MSG("Module::run - got buf (src=0x%08x, dst=0x%08x, type=0x%08x)\n", 
         buf->getSource(), dst, buf->getMsgType());
      if (( (dst & 0xffff0000) == MsgBufferEnums::ADDR_MULTICAST_DOWN)) {
         relayMulticastDown(buf);
         buf = dispatch(buf);
         if (buf) {
            /* Unhandled message. Do not know what to do with this buffer, delete it */
            delete(buf);
         }
      } else if (( (dst & 0xffff0000) == MsgBufferEnums::ADDR_MULTICAST_UP)) {
         relayMulticastUp(buf);
      } else if ((dst == MsgBufferEnums::ADDR_LINK_LOCAL) || 
                 ( (dst & m_netmask) == m_address)      ) {
         /* This buffer is addressed for us */
         buf = dispatch(buf);
         if (buf) {
            /* Unhandled message. Do not know what to do with this buffer, delete it */
            delete(buf);
         }
      } else if ( (dst & m_treeNetmask) != m_address) {
         /* Send towards the hub if possible, otherwise drop it */
         if (m_rawRootPublic) {
            m_rawRootPublic->preformedMessage(buf);
         } else {
            WARN("Module::run - dropped packet - no rawRoot\n");
            delete(buf);
         }
      } else {
         /* Hand over to one of the inferiour public interfaces */
         int i;
         for (i=0; i < m_numLowerModules ; i++) {
            DBG_MSG("  trying %i: 0x%08x, 0x%08x\n", i, m_lowerModules[i].address, m_netmask);
            if ( !m_lowerModules[i].orderedToShutdown &&  
                 ((dst & m_netmask) == m_lowerModules[i].address) ) {
               m_lowerModules[i].theInterface->preformedMessage(buf);
               buf=NULL;
               break; // This was commented in my code by unknown reason.
            }
         }
         if (buf) {
            WARN("Module::run - dropped packet - no matching lower module\n");
            delete(buf);
         }
      }
   }
   DBG("Module::run - exiting\n");
}

MsgBuffer * Module::dispatch(MsgBuffer *buf)
{
   
   DBG_MSG("Module::dispatch : %i\n", buf->getMsgType());
   switch (buf->getMsgType()) {
   case MsgBufferEnums::MODULE_CONNECT_LOWLEVEL:
      {
         ModulePublic *newPeer;
         newPeer = (ModulePublic *)(buf->readNext32bit());
         lowlevelConnectTo(newPeer);
         break;
      }
   case MsgBufferEnums::MODULE_NEGOTIATE_ADDRESSSPACE:
      {
         uint32 parentAddr  = buf->readNext32bit();
         uint32 prefix  = buf->readNext32bit();
         uint32 netmask = buf->readNext32bit();
         negotiateAddressSpace(parentAddr, prefix, netmask);
         break;
      }
   case MsgBufferEnums::MODULE_MANAGE_MULTICAST:
      {
         enum MulticastCmd cmd = MulticastCmd(buf->readNext16bit());
         int16 group = buf->readNext16bit();
         decodedManageMulticast(cmd, group, buf->getSource());
         break;
      }
   case MsgBufferEnums::MODULE_STARTUP_COMPLETE:
      {
         decodedStartupComplete();
         break;
      }
   case MsgBufferEnums::MODULE_SHUTDOWN_PREPARE:
      {
         int16 upperTimeout = buf->readNext16bit();
         decodedShutdownPrepare( upperTimeout );
         break;
      }
   case MsgBufferEnums::MODULE_SHUTDOWN_PREPARE_COMPLETE:
      {
         decodedShutdownPrepareComplete(buf->getSource());
         break;
      }
   case MsgBufferEnums::MODULE_SHUTDOWN_NOW:
      {
         int16 upperTimeout = buf->readNext16bit();
         decodedShutdownNow( upperTimeout );
         break;
      }
   case MsgBufferEnums::MODULE_SHUTDOWN_COMPLETE:
      {
         decodedShutdownComplete(buf->getSource());
         break;
      }
   case MsgBufferEnums::CONNECTION_CTRL:
      {
         enum ConnectionCtrl order = ConnectionCtrl(buf->readNext32bit());
         const char *method = reinterpret_cast <const char *> (buf->accessRawData(buf->getReadPos()));
         decodedConnectionCtrl(order, method, buf->getSource());
         break;
      }
   case MsgBufferEnums::CONNECTION_NOTIFY:
      {
         enum ConnectionNotify status = ConnectionNotify(buf->readNext32bit());
         enum ConnectionNotifyReason reason;
         reason = ConnectionNotifyReason(buf->readNext32bit());
         decodedConnectionNotify(status, reason, buf->getSource());
         break;
      }
   case MsgBufferEnums::TIMER:
      {
         decodedExpiredTimer(buf->getExtraData());
         break;
      }
   case MsgBufferEnums::INTERNAL_MODULE_MSG:
      {
         uint32 type = buf->readNext32bit();
         uint32 size = buf->getLength() - buf->getReadPos();
         const void * ptr = buf->accessRawData(buf->getReadPos());
         internalMsgRx(buf->getSource(), type, size, ptr);
         break;
      }
   case MsgBufferEnums::SOLICITED_ERROR_MSG:
      {
         ErrorObj err( buf );
         decodedSolicitedError( err, buf->getSource(), 
                                buf->getDestination() );
      } break;
   default:
      return buf;
   }

   /* Get here if the message was handled above successfully */
   delete(buf);
   return NULL;
}

int Module::newLowerModule(ModulePublic *lowerModule, uint32 newAddr)
{
   if (m_numLowerModules >= MaxLowerModules)
   {
      ERR("Used up all lower modules slots in newLowerModule()\n");
      exit(1);
   }
   /* A temporary veriable is used in preparation for moving to 
    * a more dynamic way of handling lower modules. */
   LowerModule tmp;
   tmp.theInterface=lowerModule;
   tmp.address=newAddr;
   tmp.shutdownPrepared=false;
   tmp.shutdown=false;
   tmp.orderedToShutdown=false;
   m_lowerModules[m_numLowerModules]=tmp;
   // Automatically add ErrorMessages to the multicast subscriptons, making it Opt-out rather than Opt-in.
   m_lowerModules[m_numLowerModules].subscriptions.insert(Module::ErrorMessages);
   ++m_numLowerModules;
   return 0;
}

void Module::decodedExpiredTimer(uint16 timerid)
{
   if ( m_awaitingShutdownPrepareComplete && timerid==m_shutdownTimer ) {
      ERR("Not all modules reported ShutdownPrepareComplete. Ignoring.");
      treeIsShutdownPrepared();
   } else if ( m_awaitingShutdownComplete && timerid==m_shutdownTimer ) {
      ERR("Not all modules reported ShutdownComplete. Ignoring.");
      treeIsShutdown();
   }
}

void Module::treeIsShutdownPrepared()
{
   m_queue->cancelTimer(m_shutdownTimer);
   DBG("Tree is prepared to shutdown");
   m_shutdownPrepared = true;
   m_awaitingShutdownPrepareComplete = false;
   if (m_rawRootPublic) {
      m_rawRootPublic->shutdownPrepareComplete();
   } else {
      decodedShutdownNow(MaxShutdownWait);
   }
}

void Module::decodedShutdownPrepare( int16 upperTimeout )
{
   DBG("Got ShutdownPrepare");
   /* Prepare to shutdown. Go through all motions that require communications
    * with other modules */
   int i;
   if (m_numLowerModules <= 0) {
      treeIsShutdownPrepared();
   } else {
      m_awaitingShutdownPrepareComplete = true;
      m_queue->setTimer(m_shutdownTimer, upperTimeout - ShutdownWaitDiff );
      for (i=0; i < m_numLowerModules; i++) {
         m_lowerModules[i].theInterface->shutdownPrepare( upperTimeout - ShutdownWaitDiff );
      }
   }
}

void Module::decodedShutdownPrepareComplete(uint32 src)
{
   DBG("Got ShutdownPrepareComplete from 0x%08"PRIx32, src);
   int i;
   if ( !m_awaitingShutdownPrepareComplete ) {
      WARN("Got unexpected ShutdownPrepareComplete from 0x%08"PRIx32, src);
      return;
   }
   for (i=0; i < m_numLowerModules; i++) {
      if (m_lowerModules[i].address == (src & m_netmask) ) {
         /* Fond the sender. Mark it and see if it was the last
          * one to report in. */
         int k;
         m_lowerModules[i].shutdownPrepared=true;
         /* FIXME - put this in isShutdownPrepared() - needs to be called by all that 
          * change the status, or put it in the exit code from run... */
         for (k=0; k < m_numLowerModules ; k++) {
            if (!m_lowerModules[k].shutdownPrepared) {
               return;
            }
         }
         /* This whole tree is prepared to shut down now */
         treeIsShutdownPrepared();
         return;
      }
   }

   /* Huh, unknown module replied?!? */
   WARN("decodedShutdownPrepareComplete - unkown module replied");
}

void Module::treeIsShutdown()
{
   m_queue->cancelTimer(m_shutdownTimer);
   m_queue->removeTimer(m_shutdownTimer);
   m_shutdownTimer=0;
   DBG("Tree is shutdown");
   m_awaitingShutdownPrepareComplete = false;
   if (m_rawRootPublic) {
      m_rawRootPublic->shutdownComplete();
      delete m_rawRootPublic;
      m_rawRootPublic = NULL;
   }
   m_exiting = true;
}

void Module::decodedShutdownNow( int16 upperTimeout )
{
   DBG("Got ShutdownNow");
   /* Disconnect from all other modules and free all objects except
    * the message queue since it may be shared between modules.
    *
    * No more lower module interfaces are needed. Deallocate them after
    * this message. */
   int i;
   if (m_numLowerModules <= 0) {
      treeIsShutdown();
   } else {
      m_awaitingShutdownComplete = true;
      m_queue->setTimer(m_shutdownTimer, upperTimeout - ShutdownWaitDiff );
      for (i=0; i < m_numLowerModules; i++) {
         m_lowerModules[i].orderedToShutdown = true;
         m_lowerModules[i].theInterface->shutdownNow( upperTimeout - ShutdownWaitDiff );
         delete m_lowerModules[i].theInterface;
      }
   }
}

void Module::decodedShutdownComplete(uint32 src)
{
   DBG("Got ShutdownComplete from 0x%08"PRIx32, src);
   int i;
   if ( !m_awaitingShutdownComplete ) {
      WARN("Got unexpected ShutdownComplete from 0x%08"PRIx32, src);
      return;
   }
   for (i=0; i < m_numLowerModules; i++) {
      if (m_lowerModules[i].address == (src & m_netmask) ) {
         /* Found the sender. Mark it and see if it was the last
          * one to report in. Also remove all multicast 
          * subscriptions.  */
         int k;
         m_lowerModules[i].subscriptions.clear();
         m_lowerModules[i].shutdown=true;
         for (k=0; k < m_numLowerModules ; k++) {
            if (!m_lowerModules[k].shutdown) {
               return;
            }
         }
         treeIsShutdown();
         return;
      }
   }

   /* Huh, unknown module replied?!? */
   WARN("decodedShutdownComplete - unkown module replied");
}


void Module::decodedSolicitedError( const ErrorObj& /*err*/, 
                                          uint32 /*src*/, uint32 /*dst*/ )
{
}

void Module::decodedConnectionCtrl(enum ConnectionCtrl, const char* /*method*/, uint32 /*src*/) 
{
}

void Module::decodedConnectionNotify(enum ConnectionNotify, 
                                     enum ConnectionNotifyReason, uint32 /*src*/)
{
}


void Module::decodedManageMulticast(enum MulticastCmd cmd, int16 group, uint32 src)
{
   int i;

   for (i=0; i < m_numLowerModules ; i++) {
      DBG_MULTICAST("  Mcast checking %i: 0x%08x, 0x%08x, 0x%08x\n", 
            i, m_lowerModules[i].address, src, m_netmask);
      if ( (src & m_netmask) == m_lowerModules[i].address) {
         break;
      }
   }
   /* See if the sender has a valid lower module address. */
   if (i >= m_numLowerModules) {
      return;
   }
   m_lowerModules[i].subscriptions.insert(group);
   if (m_rawRootPublic) {
      m_rawRootPublic->manageMulticast(cmd, group);
   }
}

void Module::decodedStartupComplete()
{  
   int i;
   for (i=0; i < m_numLowerModules; i++) {
      m_lowerModules[i].theInterface->startupComplete();
   }
}

uint32 Module::getRequestId()
{
   m_lastRequestId = (m_lastRequestId + 1) & ~m_netmask;
   return m_address | m_lastRequestId;
}

int Module::negotiateAddressSpace(uint32 parentAddr, uint32 prefix, uint32 netmask)
{
   int numAddresses, temp;
   uint32 submoduleIncrement;

   DBG_NEG_ADDR("Module::negotiateAddressSpace (0x%08x, 0x%08x, 0x%08x)\n", 
         parentAddr, prefix, netmask);

   m_treeNetmask = netmask;

   // Find number of bits needed to address us and our subordinates.
   numAddresses = 1 + m_numLowerModules;
   temp = numAddresses - 1;
   while (temp) {
      temp = temp >> 1;  /* Should be unsigned shift, but we will never have
                            more than 32766 subordinates anyway */
      netmask = (netmask >> 1) | 0x80000000;
   }
   prefix = prefix & netmask; /* Just in case */
   m_address = prefix;
   m_netmask = netmask;
   /* This if-statement is only needed when an address is assigned when
    * not connected the the hub. This is only done during debugging.
    */
   if (m_rawRootPublic) {
      m_rawRootPublic->setDefaultDestination(parentAddr);
   }

   submoduleIncrement = (~netmask) + 1;
   DBG_NEG_ADDR("Module::negotiateAddressSpace : 0x%08x, 0x%08x :"
         " subaddrIncr: 0x%08x\n", m_address, m_netmask, submoduleIncrement);
   { 
      int i;
      for (i=0 ; i < m_numLowerModules ; i++) {
         prefix = prefix + submoduleIncrement;
         DBG_NEG_ADDR("Module::negotiateAddressSpace : subadr 0x%08x\n", prefix);
         m_lowerModules[i].address = prefix;
         m_lowerModules[i].theInterface->setDefaultDestination(prefix);
         m_lowerModules[i].theInterface->negotiateAddressSpace(m_address, prefix, m_netmask);

      }
   }
   INFO("%s has address %#0"PRIx32, getName(), m_address);
   return 0;
}

int Module::lowlevelConnectTo(ModulePublic *peerPublic)
{
   DBG_CONNECT("Module::lowlevelConnectTo (%p)\n", peerPublic);
   m_rawRootPublic = peerPublic;
   m_rawRootPublic->setOwnerModule(this);
   highlevelConnectTo();
   return 0;
}

int Module::highlevelConnectTo()
{
   DBG_CONNECT("Module::highlevelConnectTo()\n");
   return 0;
}


void Module::internalMsgRx(uint32 /*pktId*/, uint32 /*type*/, int /*size*/, const void* /*data*/)
{
}

uint32 Module::sendInternalMsg(uint32 type, uint32 size, const void *data)
{
   MsgBuffer *cmdbuf;
   uint32 src = getRequestId();
   
   cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::INTERNAL_MODULE_MSG, size+8);
   cmdbuf->writeNext32bit(type);
   cmdbuf->writeNext32bit(size);
   cmdbuf->writeNextByteArray((uint8 *)data, size);
   m_queue->insert(cmdbuf);
   return src;
}


uint32 Module::unsolicitedError( const ErrorObj& err, uint32 dst ) const
{
   return m_errorProvider->unsolicitedError( err, dst );
}


ModulePublic::ModulePublic(ModuleQueue *q)
   : m_queue(q)
{
   m_defaultDst = uint32(MsgBufferEnums::ADDR_LINK_LOCAL);
//   cout << "ModulePublic::ModulePublic(ModuleQueue)" << endl;
}

ModulePublic::ModulePublic(Module *m)
   : m_queue(m->getQueue()), m_owner(m)
{
//   cout << "ModulePublic::ModulePublic(Module)" << endl;
   m_defaultDst = uint32(MsgBufferEnums::ADDR_LINK_LOCAL); //FIXME error address would be better
}

uint32 ModulePublic::lowlevelConnectTo(ModulePublic *peerPublic)
{
   MsgBuffer * cmdbuf;

//   cout << "ModulePublic::lowlevelConnectTo (" << hex << peerPublic << dec 
//        << ")" << endl;
   cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), 0, 
         MsgBufferEnums::MODULE_CONNECT_LOWLEVEL, 4);
   cmdbuf->writeNext32bit((uint32)(peerPublic));
   m_queue->insert(cmdbuf);
   return 0;
}

uint32 ModulePublic::addrOrDefault(uint32 address)
{
   if((address & 0xff000000) == (m_defaultDst & 0xff000000)){
      return address;
   } else {
      return m_defaultDst;
   }
}

uint32 ModulePublic::negotiateAddressSpace(uint32 parentAddr, uint32 prefix, uint32 netmask)
{
   uint32 src = m_owner->getRequestId();

//   cout << "ModulePublic::negotiateAddressSpace (0x" << hex << prefix 
//        << ", 0x" << netmask << ")" << endl;
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_NEGOTIATE_ADDRESSSPACE, 8);
   cmdbuf->writeNext32bit(parentAddr);
   cmdbuf->writeNext32bit(prefix);
   cmdbuf->writeNext32bit(netmask);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 ModulePublic::startupComplete()
{
   uint32 src = m_owner->getRequestId();

//   cout << "ModulePublic::startupComplete" << endl;
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_STARTUP_COMPLETE, 0);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 ModulePublic::shutdownPrepare( int16 upperTimeout )
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_SHUTDOWN_PREPARE, 0);
   cmdbuf->writeNext16bit( upperTimeout );
   m_queue->insert(cmdbuf);
   return src;
}
uint32 ModulePublic::shutdownPrepareComplete()
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_SHUTDOWN_PREPARE_COMPLETE, 0);
   m_queue->insert(cmdbuf);
   return src;
}
uint32 ModulePublic::shutdownNow( int16 upperTimeout )
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_SHUTDOWN_NOW, 0);
   cmdbuf->writeNext16bit( upperTimeout );
   m_queue->insert(cmdbuf);
   return src;
}
uint32 ModulePublic::shutdownComplete()
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_SHUTDOWN_COMPLETE, 0);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 ModulePublic::manageMulticast(enum Module::MulticastCmd cmd, int16 group)
{
   uint32 src = m_owner->getRequestId();

//   cout << "ModulePublic::manageMulticast (" << cmd << ", 0x" << hex << group 
//        << dec << ")" << endl;
   MsgBuffer * cmdbuf = new MsgBuffer(uint32(MsgBufferEnums::ADDR_LINK_LOCAL), src, 
         MsgBufferEnums::MODULE_MANAGE_MULTICAST, 8);
   cmdbuf->writeNext16bit(cmd);
   cmdbuf->writeNext16bit(group);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 ModulePublic::connectionCtrl(enum Module::ConnectionCtrl order, 
                                 const char* method, uint32 dst)
{
//   cout << "ModulePublic::connectionCtrl (" << order << ", " 
//        << method << ")" << endl;
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer * cmdbuf = new MsgBuffer(dst, src, MsgBufferEnums::CONNECTION_CTRL, 
         strlen(method) + 4);
   cmdbuf->writeNext32bit(order);
   cmdbuf->writeNextCharString(method);
   m_queue->insert(cmdbuf);
   return src;
}

uint32 ModulePublic::connectionNotify(enum Module::ConnectionNotify status, 
                                      enum Module::ConnectionNotifyReason reason,
                                      uint32 dst)
{
   if (dst == MsgBufferEnums::ADDR_DEFAULT) {
      dst = m_defaultDst;
   }
   uint32 src = m_owner->getRequestId();
   MsgBuffer* cmdbuf = new MsgBuffer(dst, src, MsgBufferEnums::CONNECTION_NOTIFY, 0);
   cmdbuf->writeNext32bit(status);
   cmdbuf->writeNext32bit(reason);
   m_queue->insert(cmdbuf);
   return src;
}

void ModulePublic::preformedMessage(MsgBuffer *buf)
{
//   cout << "ModulePublic::preformedMessage()" << endl;
   m_queue->insert(buf);
}

uint32 ModulePublic::solicitedError( const ErrorObj& err, uint32 dst ) const
{
   uint32 src = m_owner->getRequestId();
   MsgBuffer* buf = new MsgBuffer(dst, src, MsgBufferEnums::SOLICITED_ERROR_MSG, 64 + err.getSize() );
   
   err.serialize( buf );
   m_queue->insert(buf);
   return src;
}


} /* namespace isab */

