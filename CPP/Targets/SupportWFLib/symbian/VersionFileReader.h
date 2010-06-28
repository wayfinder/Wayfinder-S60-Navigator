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

#ifndef VERSION_FILE_READER_H
#define VERSION_FILE_READER_H

#include <e32base.h>

class CVersionFileReader : public CBase
{
public:
   /**
    * @name Enums used to select which part of the version number you
    *       want.
    */
   //@{
   /** This enum selects parts of the newversion file. */
   enum TNewVersionType {
      /** The full (major, minor, misc) app version. Period-separated. */
      ENewAppFullVersion       = 0, 
      /** The major and minor app version. Period separated. */
      ENewAppMajorMinorVersion = 1,
      /** The misc app version.*/
      ENewAppMiscNumber        = 2,
      /** The full (major, minoe, misc) voice version. Period separated. */
      ENewResFullVersion       = 3,
      /** The major and minor voice version. Period separated. */
      ENewResMajorMinorVersion = 4,
      /** The misc voice version. */
      ENewResMiscNumber        = 5,
      /** The version number of the mlfw upgrade.*/
      ENewMlfwVersion          = 6,
      /** 
       * This is for internal use. Using it as an argument in any
       * function will probably cause a panic.
       */
      ENewVersionArraySize     = 7
   };
   
   /** This enum selects parts of the oldversion file. */
   enum TOldVersionType {
      /** 
       * Full version (major, minor, misc). Major and minor are
       * separated by a period. minor and misc are separated by an
       * underscore.
       */
      EOldFullVersion          = 0,
      /** Major and minor version. Period separated*/
      EOldMajorMinorVersion    = 1,
      /** Misc version. */
      EOldMiscNumber           = 2,
      /** MLFW upgrade version*/
      EOldMlfwVersion          = 0,
   };
   //@}

   /** @name Constructors and destructor. */
   //@{
private:
   /** Default constructor. */
   CVersionFileReader();
   /** Second phase constructor. */
   void ConstructL();
public:
   /**
    * Static constructor. 
    * @return a new CVersionFileReader object.
    */
   static class CVersionFileReader* NewL();
   /**
    * Static constructor. 
    * @return a new CVersionFileReader object, still on the top of the
    *         cleanupstack.
    */
   static class CVersionFileReader* NewLC();
   /** Virtual destructor. */
   virtual ~CVersionFileReader();   
   //@}

   TInt ReadNewVersionFileL(const TDesC& versionFile);
   TInt ReadOldVersionFileL(const TDesC& aFilename);
   TPtrC8 GetNewVersion(enum TNewVersionType aVersionType);
   TPtrC8 GetOldVersion(enum TOldVersionType aVersionType);
   class TVersion NewVersion(enum TNewVersionType aVersionType);
   class TVersion OldVersion(enum TOldVersionType aVersionType);

private:
   class CDesC8Array* iNewVersionArray;
   class CDesC8Array* iOldVersionArray;
};

#endif /* VERSION_FILE_READER_H */
