/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef QUEUESERIAL_H
#define QUEUESERIAL_H

#include <queue>
namespace isab {

   /** A class that acts as a serial provider to Nav2 and as a
    * I/O-queue to anyone else.*/
   class QueueSerial : public Module, 
                       public SerialProviderInterface
   {
   public:
      typedef std::deque<class Buffer*> Container;

      class OverflowPolicy{
      public:
         virtual void clean(Container& deq) = 0;
      };

      /** Constructor. 
       * @param name the name of the Module thread. Must be unique. 
       * @param microSecondsPoll a hint to whoever is reading data from 
       *                         this module what a suitable poll interval 
       *                         might be. In microseconds. 
       *                         Defaults to 0.2 seconds. */
      QueueSerial(const char* name, OverflowPolicy* policy, 
                  int microSecondsPoll = 200000);

      virtual ~QueueSerial();
      
      /** Creates a new SerialProviderPublic object used to reach this 
       * module.
       * @return a new SerialProviderPublic object connected to the queue.
       */
      class SerialProviderPublic * newPublicSerial();

      
      /** Handles incoming data relayed from the public interface. 
       * @param length the number of bytes to deal with. 
       * @param data   a pointer to an array of char at least length 
       *               bytes long.
       * @param src    the address of the sender of this package.
       */
      virtual void decodedSendData(int length, const uint8 *data, uint32 src);
      /** Handles orders regarding this objects connection.
       * @param ctrl   the order: CONNECT, DISCONNECT ...
       * @param method a string argument detailing the order.
       * @param src    the address of the sender of the order. */
      virtual void decodedConnectionCtrl(enum ConnectionCtrl ctrl, 
                                         const char *method, 
                                         uint32 src);
      /** Signals that the module tree has started up correctly and is
       * ready to relay messages. Causes QueueSerial to send a
       * connectionNotify message.*/
      virtual void decodedStartupComplete();

      /** Used from the non-Nav2 side to signal to Nav2 that a
       * connection has been set up.*/
      void connect();
      /**Used from the non-Nav2 side to signal to Nav2 that a
       * connection has been disconnected .*/
      void disconnect();
      /** Used from the non-Nav2 side to send data to Nav2.*/
      bool write(const uint8* data, int length);
      /** Used from the non-Nav2 side to read data sent from Nav2.
       * @param data   a pointer to a data area where the data will be
       *               written.  
       * @param length the length of the data area.
       * @return the number of bytes written to data.*/
      int read(uint8* data, int length);
      /** Used from the non-Nav2 side to read data sent from Nav2.
       * @param buf a Buffer that will receive the data. */
      bool read(class Buffer* buf);
      /** Checks how much data is available that has been sent from Nav2.
       * This is a blocking function, and not recursively so.
       * @return the amount of bytes available to read.*/
      int available() const;
      /** Tests if the I/O-queue is empty.
       * @return true of no data is in the queue. */
      bool empty() const;
#ifdef __SYMBIAN32__
      /** Signal that the caller wants to be notified of when more data
       * becomes available for reading. */
      void armReader(TRequestStatus *aStatus);
      void cancelArm();
#endif

      int getPollInterval() const;
   private:
      ///This function is private as it is meant only for internal
      ///use.  This function accesses member data without first
      ///locking the mutex, which means it may only be called from
      ///within a critical region.
      ///@return the amount of bytes available to read.*/
      int internalAvailable() const;
   protected:

      void removeOverflow();

      /** This function is used in this class to send messages to the
       * Module immediatly above us, i.e. closer to the CtrlHub. */
      class SerialConsumerPublic * rootPublic();
      /** Decoder for SerialProvider-messages */
      SerialProviderDecoder m_providerDecoder;
      
      /** Decides what should be done with incoming Buffers. */
      virtual class MsgBuffer * dispatch(class MsgBuffer *buf);
      
      /** The queue where data from nav2 is stored until it is read out 
       * with one of the read methods.*/
      Container m_outQue;

      /** A mutex to protect the queue.*/
      mutable Mutex m_outMutex;

      ///connectsState is very relative. Let the state be connected as
      ///soon as the other end tries to read. for the first time.
      enum ConnectionNotify m_connectState;

      ///The recommended Poll interval for this QueueSerial. In microseconds.
      int m_pollInterval;

      OverflowPolicy* m_policy;
      std::queue<class Buffer*> m_incoming;
#ifdef __SYMBIAN32__
      TThreadId m_guiSideThread;
      TRequestStatus *m_guiSideRequestStatus;
#endif
   };
   
   inline int QueueSerial::getPollInterval() const
   {
      return m_pollInterval;
   }

   inline void QueueSerial::removeOverflow()
   {
      if(m_policy)m_policy->clean(m_outQue);
   }

   class KeepLatestGuiMessage : public QueueSerial::OverflowPolicy{
   public:
      virtual void clean(QueueSerial::Container& deq);
      typedef QueueSerial::Container argument_type;
      typedef void result_type;
      result_type operator()(argument_type& deq)
      {
         clean(deq);
      }
   };

   class KeepLatestBuffer : public QueueSerial::OverflowPolicy{
   public:
      virtual void clean(QueueSerial::Container& deq);
      typedef QueueSerial::Container argument_type;
      typedef void result_type;
      result_type operator()(argument_type& deq)
      {
         clean(deq);
      }
   };

   class RemoveGpsAndRoute : public QueueSerial::OverflowPolicy{
   public:
      virtual void clean(QueueSerial::Container& deq);
      typedef QueueSerial::Container argument_type;
      typedef void result_type;
      result_type operator()(argument_type& deq)
      {
         clean(deq);
      }
   };

} /* namespace isab */

#endif
