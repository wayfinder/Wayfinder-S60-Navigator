/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SPROCKETSENGINE_H
#define SPROCKETSENGINE_H

#include <in_sock.h>
#include "Log.h"
#include "Nav2.h"

//Abstract class interface for diffenrent kinds of engines used by MessageHandler.
class MMessageEngine {
public:
   virtual ~MMessageEngine() = 0;
   virtual void ConnectL() = 0;
   virtual void Disconnect() = 0;
   virtual void Release() = 0;
   virtual TBool IsConnected() const = 0;
   virtual void WriteL( class isab::Buffer* aBuffer ) = 0;

};

inline MMessageEngine::~MMessageEngine()
{
}


class CSprocketsEngine : public CActive, public MMessageEngine
{

public: // Constructors & Destructor

   static CSprocketsEngine* NewL( class MSprocketTalker* aConsole,
                                  class isab::Nav2::Channel* nav2, 
                                  const char* name = "SprocketsEngine");
   static CSprocketsEngine* NewLC( class MSprocketTalker* aConsole,
                                   class isab::Nav2::Channel* nav2,
                                   const char* name);

   /**
    * ~CSprocketsEngine
    * Destroy the object and release all memory objects
    */
   ~CSprocketsEngine();

   void SetLogMasterL(class isab::LogMaster* aLogMaster, 
                      const char* name = "SprocketsEngine");
public: // new methods


   /**
    * ConnectL
    * Initiate connection of socket, using iServerName and iPort
    */
   void ConnectL();

   /**
    * Disconnect
    * Disconnect socket
    */
   void Disconnect();

   /** Release and forget the QueueSerial module*/
   void Release();

   /**
    * WriteL
    * Write data to socket
    * @param aData data to be written
    */
   void WriteL( class isab::Buffer* aBuffer );

   /**
    * Write
    * Write data to socket. Writes the complete buffer.
    */
   void Write( const TDesC8& aBytes );

   /**
    * Connected
    * Is socket fully connected?
    * @return true if socket is connected
    */
   TBool IsConnected() const;

protected:
   ///@name From CActive.
   //@{
   /**
    * DoCancel
    * cancel any outstanding operation
    */
   void DoCancel();

   /**
    * RunL
    * called when operation complete
    */
   void RunL();
   //@}
private: // New methods

   ///Tests whether iBuffer contains at least one more complete
   ///GuiProt message. If iBuffer Contains at least 4 bytes, the
   ///length field will be read into the aLength parameter.
   ///@param aLength if possible, the length field of the next message
   ///               or part of message will be written into the
   ///               referred TInt variable.
   ///@return ETrue if it's possible to decode a GuiProt message from 
   ///        the contents of iBuffer.
   TBool OneMoreMessage(TUint& aLength);
   ///If it's possible to read more messages from iBuffer, this
   ///function will cause the active scheduler to call RunL again, if
   ///not we will rearm and wait for a completion event from the
   ///connected QueueSerial object.
   void ArmOrCompleteL();


   /**
    * CSprocketsEngine
    * Perform the first phase of two phase construction
    * @param aConsole the console to use for ui output
    */
   CSprocketsEngine( class MSprocketTalker* aConsole, 
                     class isab::Nav2::Channel* nav2);

   /**
    * ConstructL
    * Perform the second phase construction of a CSprocketsEngine 
    */
   void ConstructL(const char* name, class isab::LogMaster* aLogMaster = NULL);

   enum TSprocketsEngineState 
   {
      ENotConnected,
      EConnected,
   };

   /**
    * ChangeStatus
    * handle a change in this object's status
    * @param aNewStatus new status
    */
   void ChangeStatus(enum TSprocketsEngineState aNewStatus);

   /**
    * Print
    * display text on the console
    * @param aDes text to display
    */
   void Print(const TDesC& aDes);

private: // Member variables

   /// This object's current status
   enum TSprocketsEngineState  iEngineStatus;

   /// Console for displaying text etc
   class MSprocketTalker*       iConsole;

   /// Connection object (Nav2 module)
   class isab::Nav2::Channel* iToNav2;

   /// Buffer for remnants of messages.
   class isab::Buffer* iBuffer;

   class isab::Log* iLog;
};

#endif // SPROCKETSENGINE_H
