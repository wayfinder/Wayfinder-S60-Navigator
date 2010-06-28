/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/


#ifndef OUTPUT_H
#define OUTPUT_H
#include "Mutex.h"
#include "Thread.h"

namespace isab{
   class TCPSocket;
   class Selector;

   /** Superclass for all output objects. */
   class Output : protected Mutex
   {
   public:
      /** Outputs the text to whatever output device this objects has.
       * This function is synchronized.
       * @param txt the text to print.
       * @return the number of characters written. 
       */
      size_t puts(const char* txt);

      /** The virtual destructor. */
      virtual ~Output();

   protected:
      /** Abstract function that does the actual work of outputting to 
       * the actual output device.
       * @param txt the string to output.
       * @return the number of characters printed.
       */
      virtual size_t realPuts(const char* txt) = 0;
   private:
   };

   /** This output class just ignores any output. Used as a placeholder 
    * when a log object shouldn't output anything */
   class NullOutput : public Output
   {
   public:
      /** Overrides Output::puts. This class doesn't do anything with
       * data supplied to it, so puts doesn't need to be synchronized.
       * @param txt this argument is ignored.
       * @return zero.
       */
      size_t puts(const char* txt);

   protected:
      /** This function is never called. 
       * @param txt ignored.
       * @return zero.
       */
      virtual size_t realPuts(const char* txt);
   };

   /** This output class handles output through a TCP/IP port.*/
   class SocketOutput : public Output, protected Thread
   {
   public:
      SocketOutput(const char* threadName, const char* host, short port);

      virtual ~SocketOutput();
   protected:
      virtual size_t realPuts(const char* txt);
      virtual void run();
      virtual void terminate();
   private:
      char* m_host;
      short m_port;
      bool m_stop;
      TCPSocket* m_socket;
      bool m_reconnect;
   };
   class OutputFactory{
   public:
      static Output* createOutput(const char* target = NULL);
   };

}
#endif      
