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


#include <e32def.h>
#include <e32std.h>

/**
 * This class encapsulates the parsing of the lines of a pkglist file. 
 */
class TPkgParser
{
public:
   /**
    * Default constructor. The selected language is set to KNullDesC.
    */
   TPkgParser();
   /**
    * This constructor sets the selected language. Lines that has a
    * language tag that doesn't match the selected language will be
    * skipped.
    * @param aIsoLang a string that will be compared against all
    *                 language tags.
    */
   TPkgParser(const TDesC& aIsoLang);

   /**
    * Parses a line to a packagelist.

    * @param aFileList      the list to add CFileObjects to if the 
    *                       line is a file line.
    * @param aLine          The line to be parsed.
    * @param aTotalDataSize return parameter that will hold the value
    *                       of the totalsize value if it is parsed.
    * @return 1 if the line was successfully parsed, 0 if not.
    */
   TInt ParseLineL(class CFileObjectHolder* &aFileList, 
                   char* aLine, 
                   TUint32& aTotalDataSize);

   /**
    * Parses a data line according to the version in iVersion.
    * The parsed data is entered into the CFileObjectHolder argument.
    * @param aFileList target container for parsed data.
    * @param aLine     the line to parse.
    * @return 1 if the line was succesfully parsed, 0 if not.
    */
   TInt ParseDataLineL(class CFileObjectHolder* &aFileList,  
                       char* aLine);

   static TUint Version();

private:
   TInt32 iVersion;
   TUint32 iTotalDataSize;
   TBuf<32> iSelectedLang;
};
