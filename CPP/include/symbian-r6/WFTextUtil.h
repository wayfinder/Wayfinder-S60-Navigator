/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */ 
#ifndef WFTEXTUTIL_H
#define WFTEXTUTIL_H

#include "arch.h"


/**
 */
class WFTextUtil
{
public:

   /**
    * @param aFileName The path and file name of the file to check for existance
    * @param aSymbianErrorCode OutParameter Error code from the system,
    *                          if any. Otherwise, KErrNone is returned.
    * @return Returns false if an error occurs. 
    *
    */
   static char *stripPhoneNumberL( const char *src);

   static char* uint32AsStringL(uint32 num);

   /**
   * Convert the TDesC to an UTF-8 char string.
   */
   static char * TDesCToUtf8L(const TDesC& inbuf);
   static char * TDesCToUtf8LC(const TDesC& inbuf);
   static HBufC* Utf8Alloc(const char *utf8str);
   static HBufC* Utf8AllocL(const char *utf8str, int length);
   static HBufC* Utf8AllocL(const char *utf8str);
   static HBufC* Utf8AllocLC(const char *utf8str, int length);
   static HBufC* Utf8AllocLC(const char *utf8str);
   static HBufC* Utf8ToHBufCL(const char *utf8str);
   static HBufC* Utf8ToHBufCLC(const char *utf8str);
   static TInt Utf8ToTDes(const char *utf8str, TDes &outBuf, int length);
   static TInt Utf8ToTDes(const char *utf8str, TDes &outBuf);
#if 0
   static TInt Utf8ToTDesL(const char *utf8str, TDes &outBuf);
#endif
   static TBool MatchesResourceStringL(const TDesC &string, TInt resourceId);

   static char* strsep(char**stringp, const char* delim);

   ///strdup for Symbian, complete with new (ELeave) and CleanupStack::PushL.
   static char* strdupL(const char* aSrc);
   static char* strdupLC(const char* aSrc);

   ///Duplicate string into a HBufC8. If aSrc == NULL, the function
   ///will return NULL as well. Note that DupAllocLC will push the
   ///NULL pointer on the cleanupstack.
   static HBufC8* DupAllocLC(const char* aSrc);
   static HBufC8* DupAllocL(const char* aSrc);
   static HBufC8* DupAlloc(const char* aSrc);


   /** Analyses a string to see if it fits the pattern <host:port>.
    * @param arg   the string to analyse.
    * @param host  the place where the host name is found after the call.
    * @param port  the return variable for the port part.
    * @return true if arg was parsed succesfully, false if the arg string 
    *              didn't match the pattern. 
    */
   static TBool ParseHost(const TDesC8& aArg, TDes8& aHost, TUint& aPort);
   static TBool ParseHost(const TDesC16& aArg, TDes16& aHost, TUint& aPort);

   /**
    * Allocates a character array with a copy of the
    * data from inbuf.
    *
    * BEWARE! The TDes can contain 16bit values, these are
    * truncated to fit in a 8 bit char!
    *
    * It is the responsibility of the caller to deallocate
    * the character array.
    */
   static char* newTDesDupL(const TDesC16 &inbuf);

   /**
    * Same as above, but pushes to cleanup stack.
    */
   static char* newTDesDupLC(const TDesC16& inbuf);

   static char* newTDesDupL(const TDesC8 &inbuf);
   static char* newTDesDupLC(const TDesC8& inbuf);


   static int char2TDes(TDes &dest, const char* src);
   static int char2HBufC(HBufC *dest, const char* src);

   ///Allocates a HBufC object and copies the contents from src into
   ///it using the TDesWiden function. Note that if src == NULL, then
   ///the function will return NULL as well. Note that AllocLC will
   ///push the NULL pointer on the ClenaupStack in this case.
   //@{
   static HBufC* AllocLC(const char* src, int length);
   static HBufC* AllocL(const char* src, int length);
   static HBufC* AllocLC(const char* src);
   static HBufC* AllocL(const char* src);
   static HBufC* Alloc(const char* src);
   //@}
   static TBool TDesWiden(TDes16 &aDst, const TDesC8& aSrc);

   static HBufC8* NarrowLC(const TDesC& aSrc);

   ///Safely copies the contents of one descriptor into another.
   ///Beware! If the contents of the source descriptor is longer than
   ///the maximum length of the destination descriptor, the contents
   ///will be truncated to fit.
   ///@param dst the destination descriptor. If it is not large enough
   ///           to hold the contents of the destination descriptor,
   ///           the eventual contents of <code>src</code> will be as
   ///           much of the source descriptor contents as will
   ///           fit, starting at the beginning.
   ///@param src the source descriptor.
   ///@return    returns EFalse if truncation occured.
   static TBool TDesCopy(TDes& dst, const TDesC& src);
   static TBool TDesCopy(TDes8& dst, const TDesC16& src);

   ///Safely appends the contents of one descriptor to another.
   ///Beware! If the total contents of the two descriptors is longer than
   ///the maximum length of the destination descriptor, the contents
   ///will be truncated to fit.
   ///@param dst the destination descriptor. If it is not large enough
   ///           to append the contents of the source descriptor,
   ///           the eventual contents of <code>dst</code> will be as
   ///           the original contents plus whatever of the source 
   ///           descriptor contents as fit, starting at the beginning.
   ///@param src the source descriptor.
   ///@return    returns EFalse if truncation occured.   
   static TBool TDesAppend(TDes& dst, const TDesC& src);

   ///Safely appends the contents of a C-style char string to a
   ///descriptor.  Beware! If the total contents of the descriptor and
   ///C-string is longer than the maximum length of the destination
   ///descriptor, the contents will be truncated to fit.
   ///@param dst the destination descriptor. If it is not large enough
   ///           to append the contents of the C-string,
   ///           the eventual contents of <code>dst</code> will be as
   ///           the original contents plus whatever of the C-string 
   ///           contents as fit, starting at the beginning.
   ///@param src the source C-string.
   ///@return    returns EFalse if truncation occured.   
   static TBool TDesAppend(TDes& dst, const char* src);

   ///Performs a classic Search-And-Replace. As the source text is
   ///const, the resuling text is placed in a new HBufC that is
   ///returned. Ownership of the HBufC is naturally transferred to the
   ///caller.
   ///@param aSrc     the source text that will be searched.
   ///@param aSearch  the string to search for.
   ///@param aReplace the string that will be substituted for each
   ///                occurence of aSearch.
   ///@return a new HBufC with the substitutions performed. 
   static HBufC* SearchAndReplaceL(const TDesC& aSrc, 
                                   const TDesC& aSearch, 
                                   const TDesC& aReplace);
   ///Performs a character based in-place search and replace.
   ///@param aSrc     the string to change.
   ///@param aSearch  the character to replace. 
   ///@param aReplace the character to substitute for each aSearch.
   static void SearchAndReplace(TDes& aSrc, TChar aSearch, TText aReplace);
   ///Performs a character based search and replace. The source text
   ///is constant, so the resulting text will be returned in a new
   ///HBufC.
   ///@param aSrc     the string to change.
   ///@param aSearch  the character to replace. 
   ///@param aReplace the character to substitute for each aSearch.
   ///@return a new HBufC containing the processed text.
   static HBufC* SearchAndReplaceLC(const TDesC& aSrc, TChar aSearch, 
                                    TText aReplace);

   static TBool Equal(const char*    aLhs, const char* aRhs);
   static TBool Equal(const TText8*  aLhs, const TText8* aRhs);
   static TBool Equal(const char*    aLhs, const TDesC16& aRhs);
   static TBool Equal(const TText8*  aLhs, const TDesC16& aRhs);
   static TBool Equal(const char*    aLhs, const TDesC8& aRhs);
   static TBool Equal(const TText8*  aLhs, const TDesC8& aRhs);
   static TBool Equal(const TDesC16& aLhs, const TDesC8& aRhs);

   template<class A, class B>
   static TBool Equal(const A& aLhs,const B& aRhs)
   {
      return Equal(aRhs, aLhs);
   }

   template<class T>
   static TBool Equal(const T& aLhs, const T& aRhs)
   {
      return 0 == aLhs.Compare(aRhs);
   }

   /**
    * from mc2.
    * Not symbian specific, where do i put this type of functions?
    * Removes whitespace from end of string by overwriting with
    * zero-termination characters.
    * @param s The string to trim.
    */
   static void trimEnd(char* s);
};

/**
 * This class template extends the symbian TLex with two functions.
 */
template<typename TLexType, typename TBufType>
class TLexExt : public TLexType
{
   public:
      /// Constructor
      template<typename TDesCType> TLexExt(const TDesCType& a) : TLexType(a) {}

   /**
    * Calls Inc on the tlex unless at end of string already.
    * Avoids possible panics.
    * @param lex The lexer.
    * @return True if success, false if not possible to inc.
    */
   bool tryInc() {
      if (!TLexType::Eos()) {
         TLexType::Inc();
         return true;
      }
      return false;
   }

protected:
   //use these functions to access the Val functions in a
   //type-consistent manner.
   TInt MyVal(TLexType& aLex, TUint&   aVal){ return aLex.Val(aVal, EDecimal);}
   TInt MyVal(TLexType& aLex, TUint8&  aVal){ return aLex.Val(aVal, EDecimal);}
   TInt MyVal(TLexType& aLex, TUint16& aVal){ return aLex.Val(aVal, EDecimal);}
   TInt MyVal(TLexType& aLex, TUint32& aVal){ return aLex.Val(aVal, EDecimal);}
   TInt MyVal(TLexType& aLex, TInt&    aVal){ return aLex.Val(aVal);          }
   TInt MyVal(TLexType& aLex, TInt8&   aVal){ return aLex.Val(aVal);          }
   TInt MyVal(TLexType& aLex, TInt16&  aVal){ return aLex.Val(aVal);          }
   TInt MyVal(TLexType& aLex, TInt32&  aVal){ return aLex.Val(aVal);          }
   TInt MyVal(TLexType& aLex, TInt64&  aVal){ return aLex.Val(aVal);          }
public:
   /**
    * Reads the next value from the lexer.
    * @param lex The lexer.
    * @param val The variable that will hold the new value.
    * @param debugMess The debugMessage to display if anything fails.
    * @param tryinc If true, a call to tryInc is made after trying 
    *               to read the value.
    * @return True if ok, false if not.
    */
   template<typename integerType> 
   bool readNextVal(integerType& val, const char* debugMess, bool tryinc) {
      bool result = false;
      TLexType::Mark();
      TLexType::SkipCharacters();
      if (TLexType::TokenLength() > 0) {
         TBufType b(TLexType::MarkedToken());
         TLexType l(b);
         integerType tempVal = 0;
         TInt err = MyVal(l, tempVal);
         val = tempVal;
         if (err != KErrNone) {
            return result;
         } else {
            result = true;
         }
      }
      if (tryinc) {
         tryInc();
      }
      return result;
   }
};

typedef TLexExt<TLex16, TBuf16<100> > TLexExt16;
typedef TLexExt<TLex8,  TBuf8<100>  > TLexExt8;
#if defined(_UNICODE)
   typedef TLexExt16 TLexExts;
#else
   typedef TLexExt8  TLexExts;
#endif


#endif
