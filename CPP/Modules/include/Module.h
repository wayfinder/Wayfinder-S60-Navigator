/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULE_H
#define MODULE_H


#include "arch.h"
#include "MsgBufferEnums.h"
#include "ModuleQueue.h"
#include "Log.h"
#include "Nav2Error.h"
#include "ErrorObj.h"

#include <set>
namespace isab {


/**
 * Implements a basic module. Although this class can be instantiated 
 * as-is it is intended to be derived from. The module has at least one
 * thread which is created at startup. Extra initialization can be 
 * performed in preStartSetup().
 *
 * This class implements attachments from above and address 
 * space negotiation.
 */
   class Module : public Thread {
   public:
      
      /** Used in calls to CtrlHub::addModule(). */
      enum ModuleIds {
         CtrlHubModuleId       = 0,
         NavTaskModuleId       = 0x10,
         ParameterModuleId     = 0x11,
         NavServerComModuleId  = 0x12,
         DisplaySerialModuleId = 0x13,
         BtManagerModuleId     = 0x14,
         DestinationsModuleId  = 0x15,

         MaxModuleId
      };

      /** Multicast groups. Doesn't include parameter multicast groups.*/
      enum MulticastAddress {
         /** Position and route messages. */
         NavTaskMessages            = 0x0a,
         /** Server com progress messages. */
         NSCProgressMessages        = 0x0b,
         BtMessages                 = 0x0c,
         DestMessages               = 0x0d,
         /** Error messages */
         ErrorMessages              = 0x0e,
         /** Channel for distributing Latest News Images.*/
         LatestNews                 = 0x0f,
         /** 
          * No multicast address reserved here may have a larger value
          * than this.
          */
         MaxAllowedMulticastAddress = 0x39,
      };

      /** 
       * Convert a ModuleId into a module address.
       * @param id The ModuleId.
       * @return The 32-bit module address.
       */
      static uint32 addrFromId(enum ModuleIds id) {
         return uint32(id) << 24;
      }

      /** 
       * Creates a new Module. 
       * @param name The name to be given to the main thread
       * @param l The Log object to use. If NULL, the Module will
       *          create a Log object of its own using the module name
       *          as the prefix.
       */
      Module(const char *name, class Log* l = NULL);

      /** 
       * Destroy a module and deallocate objects that are 
       * still allocated. It is assumed that an orderly shutdown
       * has already taken place. 
       */
      virtual ~Module();

      /** 
       * Main loop of the Module. This function sleeps on the queue
       * awaiting incoming messages. Once a message has been received
       * it is decoded and dispatched through dispatch().
       */
      virtual void run();

      /**
       * Retreive a combination of source address and a request id
       * @return a value suitable for use as a source address in a 
       *         message buffer.
       */
      uint32 getRequestId();

      enum ConnectionCtrl{
         CONNECT,
         DISCONNECT,
         QUERY
      }
;
      /**
       * Connection states used by the connectionNotify functions.
       * The order of the items or their number may not change as they
       * are used as indexes into an array of error messages in
       * NavServerCom.
       */
      enum ConnectionNotify{
         CONNECTING       = 0,
         CONNECTED        = 1,
         DISCONNECTING    = 2,
         CLEAR            = 3,
         WAITING_FOR_USER = 4,
      };
      
      /**
       * This enum is divided into bitfields. It's assumed that the
       * enum is encoded in a 32 bit unsigned integer.
       * 
       *  The 8 least significant bits must be very abstact connection
       *  concepts, not associated with the carrier medium.
       * 
       *  The rest of the enum is free to use, but it can be a good
       *  idea to always include a proper abstract code in every
       *  reason.
       */
      enum ConnectionNotifyReason{
         ///No idea why this happened
         UNSPECIFIED      = 0x00,
         ///The action was requested by controller
         REQUESTED        = 0x01,
         ///The action was initiated by the other party.
         BY_PEER          = 0x02,
         ///Address lookup of some kind failed, which forced some action
         LOOKUP_FAILED    = 0x03,
         ///The connection was broken, which forced this action
         CHANNEL_FAILED   = 0x04,
         ///The other party is not responding, forcing some action.
         NO_RESPONSE      = 0x05,
         ///Internal error caused this action.
         INTERNAL_ERROR   = 0x06,
         ///The method string was undecodable, causing this action.
         METHOD_ERROR     = 0x07,
         ///The transport (GPRS etc) could not be set up
         TRANSPORT_FAILED = 0x08,
      };
      enum MulticastCmd{
         LEAVE,
         JOIN
      };

      /** 
       * Retrieves a pointer to the ModuleQueue associated with this
       * Module. This function shoul only be used by ModulePublic's
       * constructor.
       * @return The ModuleQueue.
       */
      class ModuleQueue * getQueue() {
         return m_queue;
      }

      /**
       * Retrieve the netmask.
       * @return The netmask of this module.
       */
      uint32 getNetmask()
      {
         return m_netmask;
      }

   protected:
      /** Logging facility */
      class Log * m_log;

      /** Did we create this log object? */
      bool m_ownLog;

      /** The incoming message queue */
      class ModuleQueue * m_queue;

      /** Address of this module. See also m_netmask. */
      uint32 m_address;
      /**
       * Netmask of this module and the immediat subtrees. 
       * See also m_address. 
       */
      uint32 m_netmask;
      /**
       * Netmask of this whole tree. Used to decide whether to
       * send a message upwards or not.
       */
      uint32 m_treeNetmask;
      /** The latest used request id. See getRequestId(). */
      uint32 m_lastRequestId;

      /** Used in detecting timeouts during module shutdown. */
      uint16 m_shutdownTimer;

      /** 
       * Ths public interface of the peer that is closer to the root
       * than us. Only one such peer is allowed. This is the one to
       * return address negotiation to. NULL if no module has attached
       * yet.
       */
      class ModulePublic * m_rawRootPublic;
      
      /**
       * Collects data for one of potentially several lower modules.
       * Lower modules are the modules connected to this module in a
       * downstream direction from the CtrlHub.
       */
      struct LowerModule {
         /** The ModulePublic of the lower Module. */
         class ModulePublic *theInterface;
         /** The address of the lower module. */
         uint32 address;
         /** Contains the multicast subscriptions of the lowe module. */
         std::set <uint16, std::less<uint16> > subscriptions; 
         bool shutdownPrepared;
         bool orderedToShutdown;
         bool shutdown;
      };

#define REAL_MAX_LOWER_MODULES 10
#define REAL_MAX_SHUTDOWN_WAIT 10000
#define REAL_SHUTDOWN_WAIT_DIFF 1000

      /** Maximum number of lower modules. This should be made dynamic
       * in the future. */
#ifdef _MSC_VER
      /* Broken Microsoft compiler rejects valid C++ ideoms */
      enum {
         MaxLowerModules = REAL_MAX_LOWER_MODULES, 
         MaxShutdownWait = REAL_MAX_SHUTDOWN_WAIT, /* in ms */
         ShutdownWaitDiff = REAL_SHUTDOWN_WAIT_DIFF /* in ms */
      };
#else
      static const int MaxLowerModules = REAL_MAX_LOWER_MODULES;
      static const int MaxShutdownWait = REAL_MAX_SHUTDOWN_WAIT;
      static const int ShutdownWaitDiff = REAL_SHUTDOWN_WAIT_DIFF;
#endif

      /** Number of current lower modules */
      int m_numLowerModules;

      /** A list of all the lower modules. */
      struct LowerModule m_lowerModules[MaxLowerModules];

      /** 
       * We are now in the state ShutdownPrepare - take no
       * initiatives.
       */
      bool m_shutdownPrepared;

      /**
       * We are awaiting shutdownPrepared responses from the lower
       * modules.
       */
      bool m_awaitingShutdownPrepareComplete;

      /** 
       * We are awaiting shutdownComplete responses from the lower
       * modules.
       */
      bool m_awaitingShutdownComplete;

      /** Terminate the message loop the next time around */
      bool m_exiting;

      /** 
       * This function is called after the Module has been setup but
       * before the thread is started and the constructor returns.
       * This is the place to add initialization of the subclasses.
       */
      virtual void preStartSetup();

      /** 
       * Examine an incoming buffer and decide what to do with it.
       * The version in the superclass should be called _before_ the
       * version in the subclass. If the superclass returns NULL the
       * processing should terminate.
       * @param buf The incoming MsgBuffer.
       * @return The buffer or NULL if the buffer was processed and
       *         either passed on or destroyed.
       */
      virtual class MsgBuffer * dispatch(class MsgBuffer *buf);

      // Functions called from a ModulePublic via the queue.
      int negotiateAddressSpace(uint32 parentAddr, uint32 prefix, uint32 netmask);
      int lowlevelConnectTo(class ModulePublic * peerPublic);

      // Handle multicast join/leave
      virtual void decodedManageMulticast(enum MulticastCmd cmd, int16 group, uint32 src);

      // Initiated by the CtrlHub
      virtual void decodedStartupComplete();

      // Initiated by the CtrlHub
      virtual void decodedShutdownPrepare( int16 upperTimeout );

      // Answer to shutdownPrepare from the lower modules.
      virtual void decodedShutdownPrepareComplete(uint32 src);

      // Initiated by the CtrlHub
      virtual void decodedShutdownNow( int16 upperTimeout );

      // Answer to shutdownNow from the lower modules.
      virtual void decodedShutdownComplete(uint32 src);

      // Generic order to (dis)connect to the peer module
      virtual void decodedConnectionCtrl(enum ConnectionCtrl, const char *method, uint32 src);

      // Generic report of the connection status of the module
      virtual void decodedConnectionNotify(enum ConnectionNotify, 
                                           enum ConnectionNotifyReason,
                                           uint32 src);

      // Internally generated time message
      virtual void decodedExpiredTimer(uint16 timerid);

      // Stubs for subclasses to implement.
      virtual int highlevelConnectTo();

      /** Tell the superclass about a newly connected lower module.
       * This is needed since the superclass manages the address 
       * negotiation.
       *
       * @param lowerModule A pointer to the public interface of 
       *                    the new lower module
       */
      int newLowerModule(class ModulePublic *lowerModule, uint32 newAddr = 0);

      /** Multicast messages are transmitted to the CtrlHub which in
       * turn retransmits them out the tree. This method is overridden 
       * in CtrlHub to actually mangle the packets suitably. Here
       * the messages is only relayed to m_rootPublic 
       */
      virtual void relayMulticastUp(class MsgBuffer * buf);

      /** Retransmit to all interested underlying modules.
       * This function does _not_ delete the buffer, the 
       * caller must do that. No references to the buffer are
       * held once this function returns.
       */
      void relayMulticastDown(class MsgBuffer *buf);

      //
      // All subclasses should implement a function of this form:
      //   CorrectModulePublic rootPublic();
      // that casts m_rootPublic to the correct form.

      /** This module and all submodules are now prepared to shut down.
        If there is no root module this module will handle it by itself */
      virtual void treeIsShutdownPrepared();
      
      /** This module and all submodules are now shut down. Notify
       * the root, if there is one, and terminate.
       * This is the place to do cleanups after all lower modules
       * are disabled. */
      virtual void treeIsShutdown();

      /** An internal module message was received. This function
       * needs not call the default handler (Module::internalMsgRx)
       */
      virtual void internalMsgRx(uint32 pktId, uint32 type, int size, const void *data);

      /** Send a message to the thread of this module. The message
       * will appear as a call to internalMsgRx.
       *
       * @param type Passed to internalMsgRx. Normally an enum local
       *             the the module.
       *
       * @param size    Size of the data to be passed
       *
       * @param data    pointer to the data to be passed. Data is passed
       *                by copy.
       *
       * @return  the address that will appear as pktId in internalMsgRx.
       *
       */
      uint32 sendInternalMsg(uint32 type, uint32 size, const void *data);

      /**
       * All error messages that causes request to other modules in 
       * Nav2 to fail, ends up here.
       *
       * @param err      The error causing the request to fail.
       * @param src      Address and message id from the module sending
       *                 the error.
       * @param dst      Address and message id of the request that 
       *                 failed.
       */
      virtual void decodedSolicitedError( const ErrorObj& err,
                                          uint32 src, uint32 dst );

      /**
       * Send an unsolicited error to whoever listens.
       *
       * @param theError Number of the error.
       * @param dst Use this method to send the error message
       *            to a specific module instead of multicasting it.
       *            Default ADDR_DEFAULT.
       * @param errorMessage The better than default error message. Default
       *                     not set.
       */
      uint32 unsolicitedError( const ErrorObj& err,
                               uint32 dst = MsgBufferEnums::ADDR_DEFAULT ) const;



   private:
      class ErrorProviderPublic * m_errorProvider;
      
};

class ModulePublic {
   public:
      /** 
       * This public interface is created by a module and 
       * handed to another module as a part of the connect 
       * sequence.
       */
      ModulePublic(ModuleQueue *q);

      /** 
       * This public interface is created by an underlying module
       * wishing to send messages to a not-directly connected
       * (i.e. an addressed) module. The ModulePublic parameter 
       * passed to the constructor is the public interface of the 
       * immediate parent module.
       * DANGER - not implemented yet. Use the version below instead.
       */
      ModulePublic(ModulePublic *mp);

      /** 
       * This public interface is created by an underlying module
       * wishing to send messages to a not-directly connected
       * (i.e. an addressed) module. The Module parameter 
       * is the owner module (i.e. the module using this public 
       * interface.
       */
      ModulePublic(Module *m);

      ModuleQueue * getQueue() {
         return m_queue;
      }

      /**
       * @param owner is the module which will provide request id:s.
       */
      void setOwnerModule(Module * owner) {
         m_owner = owner;
      }

      /** 
       * Change the default recipient of packets encoded with this 
       * public interface.
       * @param dst is the new destination
       * @return the old default destination
       */
      uint32 setDefaultDestination(uint32 dst) {
         uint32 olddest = m_defaultDst;
         m_defaultDst = dst;
         return olddest;
      }
      
      /**
       * Returns the argument address if that address is to the same
       * module as the default address. If it is not, it will return
       * the default address.
       * @param The address that shall be tested against the default
       *        address.
       * @return The address argument if the top 8 bits of address
       *         matches the default address. Otherwise the default
       *         address.
       */
      uint32 addrOrDefault(uint32 address);

      /** 
       * Intentionally no destination parameter - this message is
       * _only_ sent to the link local address 
       */
      uint32 negotiateAddressSpace(uint32 parentAddr, uint32 prefix, 
                                   uint32 netmask);

      /** Intentionally no destination parameter - this message is _only_ sent
       * to the link local address */
      uint32 startupComplete();

      /** Intentionally no destination parameter - this message is _only_ sent
       * to the link local address */
      uint32 manageMulticast(enum Module::MulticastCmd cmd, int16 group);

      uint32 connectionCtrl(enum Module::ConnectionCtrl order, const char* method, 
                         uint32 dst = MsgBufferEnums::ADDR_DEFAULT);
      uint32 connectionNotify(enum Module::ConnectionNotify status, 
                              enum Module::ConnectionNotifyReason reason = Module::UNSPECIFIED,
                              uint32 dst = MsgBufferEnums::ADDR_DEFAULT);

      uint32 shutdownPrepare( int16 upperTimeout );
      uint32 shutdownPrepareComplete();
      uint32 shutdownNow( int16 upperTimeout );
      uint32 shutdownComplete();

      /** 
       * This function is virtual to allow it to be overriden by
       * CtrlHubProviderPublic where it shortcuts the messages. In the 
       * future this may be replaced by a common shortcut method.
       */
      virtual void preformedMessage(class MsgBuffer *buf);

      /**
       * This message sends the error causing a request to fail to a module
       * that made the request.
       *
       * @param err The error causing the request to fail.
       * @param dst      The source address and message id of the message
       *                 that was used for sending the request that failed.
       */
      uint32 solicitedError( const ErrorObj& err, uint32 dst ) const;

   protected:
      uint32 lowlevelConnectTo(class ModulePublic * peerPublic);
      ModuleQueue * m_queue;
      Module * m_owner;
      uint32 m_defaultDst;
};

} /* namespace isab */

#endif
