/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ML_SYMBIAN_HOLDER_H
#define ML_SYMBIAN_HOLDER_H

#include <coecntrl.h>

class DBufConnection;
class MLConfigurator;
class MLMapInterface;

namespace {
   class MLSymbianHolderStorage;
}


/**
 *   Class that holds the data necessary for using MapLib
 *   in Symbian.
 */
class MLSymbianHolder {
public:
   /**
    *   Creates a new MLSymbianHolder
    *   @param parent  The parent control for the map control.
    *   @param mapConn Connection to fetch vector maps with.
    */
   static MLSymbianHolder* NewL( CCoeControl& parent,
                                 DBufConnection* mapConn );

   /// Deletes the MLSymbianHolder and its contents
   ~MLSymbianHolder();

   /// Returns the CCoeControl associated with map drawing. 
   CCoeControl* getControl() const;
   
   /// Returns the configurator object.
   MLConfigurator* getConfigurator() const;

   /// Returns the MLMapInterface. (For moving etc.)
   MLMapInterface* getMapInterface() const;
   
private:
   /// Creates a new MLSymbianHolder
   MLSymbianHolder( CCoeControl& parent,
                    DBufConnection* mapConn );
   /// Storage
   MLSymbianHolderStorage* m_storage;
   
};

#endif
