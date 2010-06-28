/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "MapUtility.h"
#include <utf.h>
#include <string.h>
#include <eikenv.h>
#include <bautils.h>
#include <eikapp.h>
#include <eikdoc.h>
#include <eikappui.h>

namespace MapUtility
{
   /* Converts a ASCIIZ string to a Symbian Unicode descriptor */
   /* if no length is specified, the whole string is converted */
   int ConvertStringToUnicode(const unsigned char* aText, TDes& aDesc,
                              int aLength)
   {
     if( aText != NULL ) {
       /* initialize variables */
       aDesc.Zero();
       int textlen;
         /* check the length of the string */
         if( aLength == -1 ) {
         textlen = strlen((char*)aText);
       } else {
           textlen = aLength;
        }
        /* start copying the string if it's not zero-length */
       if( textlen > 0 ) {
         /* compensate for longer text than descriptor */
          if( textlen >= aDesc.MaxLength() ){
            textlen = aDesc.MaxLength()-1;
         }
         /* copy the data */
         for( TInt j=0; j < textlen; j++ )
            {
               aDesc.Append( aText[j] );
            }
         aDesc.PtrZ();
         }
     }else{
         return(0);
      }
     return(aDesc.Length());
   }

   /* Converts a UTF-8 encoded string to a Symbian Unicode descriptor */
   /* if no length is specified, the whole string is converted */
   int ConvertUTF8ToUnicode(const unsigned char* aText,
                            TDes& aDesc,
                            int aLength)
   {
      if ( aText == NULL ) {
         return 0;
      }
      /* create a UTF8 descriptor from the character data */
      int descLength = ( aLength > 0 ) ? aLength : strlen( (const char*)aText );
      TPtrC8 UTFBuf(NULL, 0);
      /* Set the buffer again. Peter says that the constructors do not work */
      UTFBuf.Set( aText, descLength);
      /* do the conversion */
      int numleft = CnvUtfConverter::ConvertToUnicodeFromUtf8(aDesc, UTFBuf);
      
      /* calculate and return the actual number of characters converted */
      return(aDesc.Length() - numleft);
   }
   
   /* returns the complete path to a file located in the application's
      folder */
   int GetCompletePath(const TDesC& fileName, TDes& fullPath) {
      /* zero out the destination descriptor */
      fullPath.Zero();
      
      /* get our application's path using the CEikApplication */
      /* the trailing slash is present after the folder name */
      fullPath.Copy(
         BaflUtils::DriveAndPathFromFullName(
            CEikonEnv::Static()->EikAppUi()->Application()->AppFullName()));

      /* append the filename to the path */
      fullPath.Append(fileName);
      
      /* success */
      return(0);
   }

   HBufC* utf8ToUnicode( const char* utf8 ) {
      // Length should be right if english and a bit too long if not.
      HBufC* retVal = HBufC::New( strlen( utf8 ) + 1 );
      TPtr retPtr( retVal->Des() );
      ConvertUTF8ToUnicode( reinterpret_cast<const unsigned char*>(utf8),
                           retPtr );
      return retVal;
   }

} // namespace MapUtility
