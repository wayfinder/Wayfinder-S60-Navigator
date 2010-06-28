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

#ifndef POSITIONINTERFACE_H
#define POSITIONINTERFACE_H
#include <e32std.h>

///The MPositionInterface collects functions common to different
///interfaces used to access positioning services. The main purpose of
///the interface is to simplify code using different interfaces in
///different situations.
class MPositionInterface
{
public:
   ///Convenience function that closes and deletes any object that
   ///implements the MPositionInterface mixin. If this function Leaves
   ///it's because the DisconnectL function Leaves. In case of Leaves
   ///the MPositionInterface object is not deleted.
   ///@param aMPI reference to a pointer to a MPositionInterface object. 
   ///            Will be NULL when this function exits.
   ///@return ETrue if the MPositionInterface object was connected 
   ///        when CloseAndDeleteL was called. 
   static TBool CloseAndDeleteL(class MPositionInterface*& aMPI);
   ///Default constructor.
   MPositionInterface();
   ///Connects the interface to a position provider. Typically a GPS
   ///unit or a Tcp connection to simulated GPS data.
   virtual void ConnectL() = 0;
   ///Disconnects the position provider.
   virtual void DisconnectL() = 0;
   ///Tests if the interface is connected to a position provider.
   ///@return ETrue if the interface is connected, EFalse otherwise.
   virtual TBool IsConnected() const = 0;
   virtual TBool IsBusy() const = 0;
   /** 
    * @name Address setting functions. 
    * Set the address of the connection. The address is a vector of
    * 8-bit strings. Typically this could be three stings containing
    * Bluetooth address high part, Bluetooth address low part, and
    * name, but it's open for future expansion. Some classes that
    * implement MPositionInterface don't need these functions, so they
    * have default implementations that does nothing.
    */
   //@{
   /**
    * Sets the address using an array of char*.
    * @param aData pointer to a const array of const char pointers. 
    * @param aNum  the number of const char pointers in the array. 
    * @return ETrue if the address coudl be parsed. 
    */
   virtual TBool SetAddress(const char** aData, TInt aNum);
   /**
    * Sets the address using a MDesC8Array.
    * @param aData the array.
    * @return ETrue if the address could be parsed. 
    */
   virtual TBool SetAddress(const class MDesC8Array& aData);
   //@}
   ///First disconnect, then clear the address, then search for a new GPS. 
   virtual void ReConnect();
   ///Virtual destructor. 
   virtual ~MPositionInterface() = 0;
private:
   ///Disabled assignment operator.
   const class MPositionInterface& operator=(const class MPositionInterface&);
   ///Disabled copy constructor.
   MPositionInterface(const class MPositionInterface& aFrom); 
};

///This class, while still a mixin, implements all virtual functions
///of MPositionInterface as passthroughs to another object derived
///from MPositionInterface. The object to which all function calls are
///redirected can be set either in the constructor or at a later time
///by a call to <code>SetPositionInterfaceRedirect</code>.
///If no redirect recipient is set, all calls are no-ops.
class MPositionInterfaceRedirect : public MPositionInterface
{
   MPositionInterfaceRedirect(const class MPositionInterfaceRedirect& aFrom);
   const class MPositionInterfaceRedirect& operator=(const class MPositionInterfaceRedirect&);
public:
   ///Default constructor. No redirect recipient set.
   MPositionInterfaceRedirect();
   ///Constructor that also sets the redirect recipient. 
   ///@param aTo pointer to the redirect recipient.
   MPositionInterfaceRedirect(class MPositionInterface* aTo);
   ///Set a new redirect recipient. 
   ///@param aTo pointer to the redirect recipient.
   ///@return the previous redirect recipient. 
   class MPositionInterface* 
   SetPositionInterfaceRedirect(class MPositionInterface* aTo);
   ///@name From MPositionInterface
   ///All of the functions from MPositionInterface redirects to the
   ///redirect recipient. If no redirect recipient is set, the
   ///functions does nothing. Functions with return value of type
   ///TBool return EFalse if no redirect recipient is set.
   //@{
   virtual void ConnectL();
   virtual void DisconnectL();
   virtual TBool IsConnected() const;
   virtual TBool IsBusy() const;

   virtual TBool SetAddress(const char **aData, TInt aNum);
   virtual TBool SetAddress(const class MDesC8Array& aData);
   virtual void ReConnect();
   //@}
   class MPositionInterface* Redirect();
private:
   ///The redirect recipient. 
   class MPositionInterface* iTo;
};

#endif
