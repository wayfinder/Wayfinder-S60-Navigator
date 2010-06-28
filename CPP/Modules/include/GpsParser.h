/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


/** This is a helper class for gps receivers with
 * nmea output.
 */


#ifndef MODULES_GpsParser_H
#define MODULES_GpsParser_H

namespace isab {

   /** This class stores the state of
    * the NMEA packet reassembly state machine. In addition it maintains
    * some information on the current state of the receiver such as 
    * health and the current constellation of satellites etc.
    */
   class GpsParser {
   protected:
      /** The serial port that connects us to the Trimble receiver */
      class SerialProviderPublic * m_serial;
      /** Where to send the calculated data */
      class GpsConsumerPublic    * m_consumer;
      /** The logging facility */
      class Log * m_log;


      /** Read out a fixed point number from a string.
       *
       * @param s     a pointer to the start of the string holding the digits.
       *
       * @param N     the maximum number of digits to read
       */
      int32 readNDigitsInt(char *s, int N);

      /**
       * Read out an integer.
       */
      int32 readInt32(char *s);

      /** Read out a floating point number from a string.
       * At most a given number of digits are converted.
       * No error checking is done.
       *
       * @param s     a pointer to the start of the string holding the digits.
       */
      float readFloat(char *s);

      /** Time support routine */
      static isabTime time_from_gps(int weekno, float tow);

   public:
      /**
       * Called by the owner (a GpsTrimble object) once the connection 
       * to the hardware or a simulator is in place.
       */
      virtual void connection_ready() = 0;

      /** 
       * Set a new consumer. Called from highlevelConnectTo().
       */
      virtual void setConsumer(class GpsConsumerPublic *c) = 0;

      /**
       * Receive data from the serial port.
       * 
       * Returns the number of handled packets and -1 on failure. 
       * 0 means no end of packet was found for a valid packet. 
       * This is normal.
       *  
       * If there is an unfinsihed packet when the input buffer ends the
       * current state of the packet reassembly state machine is stored.
       */
      virtual int receiveData(const uint8 *indata, int len) = 0;

      /** Unconditionally send the current status to the cosnumer,
       * if one exists.
       */
      virtual void sendStatus() = 0;

      GpsParser(class SerialProviderPublic *p, 
                class Log * log);

      /**
       * Virtual destructor for proper deletion.
       */
      virtual ~GpsParser();
   };
   

} /* namespace isab */

#endif /* MODULES_GpsParser_H */
