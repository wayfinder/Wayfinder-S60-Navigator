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

#include "MapLibSymbianUtil.h"

#include "DBufRequester.h"
#include "MapLib.h"
#include "SharedBuffer.h"
#include "SymbianMFDBufRequester.h"
#include "WFDRMUtil.h"
#include "WFTextUtil.h"

#include <f32file.h>

namespace {
   _LIT(KPrecachedMapsWildcard, "wf-map-*");
   _LIT(KPrecachedWarezMapsWildcard, "wf-wmap-*");
   _LIT(KPrecachedMapsDir, "\\shared\\");
}


bool
MapLibSymbianUtil::insertPrecachedMapsL( RFs& serv,
                                         MapLib& maplib,
                                         const TDesC& userName,
                                         int findWarez )
{
   // Non-empty username needed if not warez to be found.
   if ( userName.Length() == 0 && !findWarez ) {
      return false;
   }

   // Copied from old func.
   TBuf8<64> userNameAs8bit;   
   
   const char* uid = NULL;
   // Use uid if not finding warez. NULL means warez to MapLib
   if ( ! findWarez ) {      
      userNameAs8bit.Copy( userName );
      uid = reinterpret_cast<const char*>(userNameAs8bit.PtrZ());
   }

   // Also copied
   TFindFile finder(serv);
   
   
   TInt retval;   
   CDir * file_list;
   if ( findWarez ) {
      retval = finder.FindWildByDir(KPrecachedWarezMapsWildcard,
                                    KPrecachedMapsDir, file_list);
   } else {
      retval = finder.FindWildByDir(KPrecachedMapsWildcard,
                                    KPrecachedMapsDir, file_list);
   }
   
   while (retval==KErrNone) {
      for ( TInt i=0; i<file_list->Count(); ++i ) {
         TParse fn;
         fn.Set((*file_list)[i].iName, & (finder.File()), NULL);
         // A new matching file name
         char* utf8File = WFTextUtil::TDesCToUtf8LC( fn.FullName() );
         maplib.addMultiFileCache( utf8File, uid );
         // Delete
         CleanupStack::PopAndDestroy( utf8File );
      }
      retval = finder.FindWild(file_list);
   }
   return true;
}

bool
MapLibSymbianUtil::insertPrecachedMapsAsParentsL( RFs& serv,
                                                  DBufRequester* topReq,
                                                  TDesC& userName,
                                                  int findWarez,
                                                  SharedBuffer** xorBuffer,
                                                  MemTracker* memTracker )
                                             
{
   // Non-empty username needed.
   if ( userName.Length() == 0 ) {
      return false;
   }

   TBuf8<64> userNameAs8bit;
   userNameAs8bit.Copy( userName );
   
   
   TFindFile* finder = new (ELeave) TFindFile(serv);
   
   TInt retval;
   CDir * file_list;
   if ( findWarez ) {
      retval = finder->FindWildByDir(KPrecachedWarezMapsWildcard,
                                     KPrecachedMapsDir, file_list);
   } else {
      retval = finder->FindWildByDir(KPrecachedMapsWildcard,
                                     KPrecachedMapsDir, file_list);
   }
   //
   // Create "encryption" buffer
   if ( ! findWarez ) {
      SharedBuffer tmpBuf( (byte*)(userNameAs8bit.Ptr()),
                           userNameAs8bit.Length());
      if ( *xorBuffer == NULL ) {
         *xorBuffer = WFDRMUtil::createXorKey( tmpBuf );
      }
   } else {
      // Warez-buffer.
      const char * tmpstr2 = "ueaag4ha58op0q4yX897t478ufjuf";
      SharedBuffer tmpBuf2( (byte*)tmpstr2, strlen(tmpstr2));
      if ( *xorBuffer == NULL ) {
         *xorBuffer = WFDRMUtil::createXorKey( tmpBuf2 );
      }
   }

   DBufRequester* curParent = topReq->getParent();
   
   while (retval==KErrNone) {
      for ( TInt i=0; i<file_list->Count(); ++i ) {
         TParse fn;
         fn.Set((*file_list)[i].iName, & (finder->File()), NULL);
         // A new matching file name
         SymbianMFDBufRequester * newReq =
            new SymbianMFDBufRequester( curParent,
                                        serv,
                                        fn.FullName(),
                                        memTracker,
                                        0,  
                                        true);
         // Set "encryption" buffer for the requester (owned by us)
         newReq->setXorBuffer( *xorBuffer );
         curParent = newReq;
      }
      retval = finder->FindWild(file_list);
   }
   
   topReq->setParent( curParent );

   delete finder; // Wasn't done in VectorMapContainer. Hope it works.
   
   return true;
}
