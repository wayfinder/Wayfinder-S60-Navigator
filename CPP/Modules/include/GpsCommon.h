/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** A module to communicate with a gps receiver. Not intended to be 
 * instantiated directly.
 */


#ifndef MODULES_GpsCommon_H
#define MODULES_GpsCommon_H

namespace isab {

class GpsCommon
   : public Module,
     public SerialConsumerInterface,
     public GpsProviderInterface
{
public:
   virtual ~GpsCommon();
   /** Creates a new GpsProviderPublic object used to reach this 
   * module.
   * @return a new GpsProviderPublic object connected to the queue.
    */
   class GpsProviderPublic * newPublicGps();

protected:
   class SerialProviderPublic * m_provider;
   uint16 m_slowGpsTimerId;
   bool m_receiverMissing;
   class GpsParser *m_parser;
   int m_expectedDataInterval;
   GpsCommon::GpsCommon(const char *name, class SerialProviderPublic *p, 
                        int expectedDataInterval);
   virtual class MsgBuffer * dispatch(class MsgBuffer *buf);
   virtual void decodedConnectionCtrl(enum ConnectionCtrl, 
                                      const char *method, uint32 src);
   virtual void decodedReceiveData(int length, const uint8 *data, uint32 src);
   virtual void decodedResetReceiver(uint32 src);
   virtual void decodedStartupComplete();
   virtual void decodedExpiredTimer(uint16 timerid);
   virtual int highlevelConnectTo();
   virtual void treeIsShutdownPrepared();
   inline class GpsConsumerPublic * rootPublic() {
      return reinterpret_cast<class GpsConsumerPublic *>(m_rawRootPublic);
   }

   void sendFakedGpsStatus();

   /** Decoder for SerialConsumer-messages */
   class SerialConsumerDecoder m_consumerDecoder;
   /** Decoder for GpsProvider-messages */
   class GpsProviderDecoder m_providerDecoder;


};

} /* namespace isab */

#endif /* MODULES_GpsCommon_H */
