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

#ifndef WFSYMBIANUTIL_H
#define WFSYMBIANUTIL_H

#include "arch.h"
namespace isab{
   class Log;
}
#include <e32std.h>
class RFs;



/**
 * Utility methods to be used only in Symbian.
 */
class WFSymbianUtil
{
public:

   /**
    * @param aFileName The path and file name of the file to check for existance
    * @param aSymbianErrorCode OutParameter Error code from the system,
    *                          if any. Otherwise, KErrNone is returned.
    * @return Returns false if an error occurs. 
    *
    */
   static TBool doesFileExist(RFs& fsSession, const TDesC& aFileName,
                              TInt& aSymbianErrorCode );

   /**
    * Same as above, but takes an open session and doesn't close it.
    */
   static TBool doesFileExistKeepFs(RFs& fsSession, const TDesC& aFileName,
                                    TInt& aSymbianErrorCode  );

   /**
    * @param aFileName The path and file name of the file to open.
    * @param aFileContent Outparameter A descriptor large enough to 
    *                     contain the complete file content.
    * @param aSymbianErrorCode OutParameter Error code from the system,
    *                          if any. Otherwise, KErrNone is returned.
    * @return Returns false if an error occurs. In that case aFileContent
    *         is set to the empty string.
    *
    * Example use:
    *
    *    _LIT(fileName, "\\system\\apps\\WayFinder\\version.txt");
    *    TBuf8<KBuf256Length> fileContent;
    *    TInt symbErrCode;
    *    TBool error = 
    *       WFSymbianUtil::getBytesFromFile(fileName, fileContent, symbErrCode);
    */
   static TBool getBytesFromFile(RFs& fsSession, const TDesC& aFileName,
                                 TDes8& aFileContent, TInt& aSymbianErrorCode,
                                 TBool aUseFsAsIs = EFalse);

   static TBool getBytesFromFile(RFs& fsSession, const TDesC& aFileName,
                                 TDes16& aFileContent, TInt& aSymbianErrorCode );

   /**
    * Find the correct device and path to the specified file.
    *
    * @return Returns 1 if the file was found, 0 (zero) if it wasn't.
    *
    */
   static int FindBasePath(RFs& fsSession, const TDesC& aFileName,
                           TDes& aResourcePath, TBool doNotConnectDisconnect=EFalse);


   static int writeBytesToFile(RFs &fsSession, const char *filename,
         const uint8 *data, int32 length, isab::Log* memLog);
   static int writeBytesToFile(RFs &fsSession,
      TDesC& realFilename, const uint8 *data, int32 length, isab::Log* memLog);

   enum writeBytesToFileError {
      writeBytesToFileErrorFsSession            = -1,
      writeBytesToFileErrorCantWrite            = -2,
      writeBytesToFileErrorCantDeleteOriginal   = -3,
      writeBytesToFileErrorCantRenameTemp       = -4,
      writeBytesToFileErrorBelowCriticalSpace   = -5,      
      writeBytesToFileErrorNoMMCPresent         = -6,      
      writeBytesToFileErrorNoMMCSupport         = -7,      
      writeBytesToFileErrorCLCheckError         = -8,
   };

   /**
    * Appends a 8bit descriptor to a HBufC8. If the HBufC8 is not
    * large enough it will be realloced.
    * @param aDst   The destination HBufC8. MUST BE ON TOP OF THE CLEANUPSTACK.
    * @param aSrc   The source descriptor.
    * @param aIncBy If the HBufC8 needs to be realloced, it's new size
    *               will be either its old length multiplied by aIncBy
    *               or the total length of aDst and aSrc, whichever is
    *               longer. The default value of aIncBy is 2.
    * @return A pointer to the destination HBufC. If the buffer has
    *         been reallocated it points to the new buffer, otherwise
    *         to the old. Note that the returned HBufC8 is on top of
    *         the CleanupStack.
    */
   static HBufC8* AppendReallocPopLC(HBufC8* aDst, const TDesC8& aSrc, 
                              TReal aIncBy = 2);

   /**
    * Reads the content of a file into a 8 bit HBufC. 
    * @param aFs       An already connected RFs. Not closed by 
    *                  this function.
    * @param aFileName The name of the file to open. 
    * @return A newly allocated HBufC containing the file content. 
    */
   static HBufC8* ReadFileL(class RFs& aFs, const TDesC& aFileName);

   /**
    * Reads the content of a file into a 8 bit HBufC. 
    * @param aFs       An already connected RFs. Not closed by 
    *                  this function.
    * @param aFileName The name of the file to open. 
    * @return A newly allocated HBufC containing the file content.
    *         The HBufC8 is pushed to the cleanupstack. 
    */
   static HBufC8* ReadFileLC(class RFs& aFs, const TDesC& aFileName);

   /**
    * Reads the content of a file containing UTF-8 
    * text and converts it to UCS-2.
    * @param aFs       An already connected RFs. Not closed by 
    *                  this function.
    * @param aFileName The name of the file to open. 
    * @return A newly allocated HBufC containing the file content. 
    */
   static HBufC16* ReadUtf8FileL(class RFs& aFs, const TDesC& aFileName);
   
   /**
    * Reads the content of a file containing UTF-8 
    * text and converts it to UCS-2.
    * @param aFs       An already connected RFs. Not closed by 
    *                  this function.
    * @param aFileName The name of the file to open. 
    * @return A newly allocated HBufC containing the file content. 
    *         The HBufC16 is pushed to the cleanupstack. 
    */
   static HBufC16* ReadUtf8FileLC(class RFs& aFs, const TDesC& aFileName);


};

#endif
