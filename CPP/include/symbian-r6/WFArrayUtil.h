/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
   
#include <badesca.h>
#include <coemain.h>
class WFArrayUtil 
{
   /** @name Functions for creating char** from MDesC8Array.*/
   //@{
private:
   /**
    * Utility class for cleanup of char** on the CleanupStack. 
    * Instead of pushinf the char** and the individual char*'s on the
    * CleanupStack, a TCharChar object is created and pushed. The
    * TCharChar object 'owns' the char** and its component strings,
    * and in case of a Leave or PopAndDestroy it will delet all
    * component strings. The only problem is that in case of an
    * ordinary Pop the TCharChar object will be leaked.
    */
   class TCharChar{
   public:
      /**
       * Default constructor.
       * @param aCount sets the iCount member. 
       */
      TCharChar(TInt aCount = 0);
      /** The array of strings. */
      char** iArray;
      /** The size of the array.*/
      TInt iCount;
   };
   /** 
    * A function that matches the TCleanupOperation function pointer. 
    * Pushed to the CleanupStack with a TCharChar object.
    * @param aTCharChar pointer that will be cast to a TCharChar
    *                   pointer and deleted.
    */
   static void CleanupCharCharPtr(TAny* aTCharChar);
public:
   /**
    * Allocate a char** object and push it to the cleanupstack.
    * All char* in the char** will be set to NULL.
    *
    * The function returns a char***. The reason for this is that the
    * pointer then can be used as an argument to
    * <code>CleanupPopCharChar</code>. The
    * <code>CleanupPopCharChar</code> function will Pop the
    * CleanupStack without any leaks. 
    *
    * If PopAndDestroy is used it should be possible to use the
    * pointer for that purpose as well, but itäs alwasy safe to use
    * the CleanupStack::PopAndDestroy.
    *
    * @param aSize the size of the array.
    * @return pointer to a new char**. 
    */
   static char*** AllocCharCharLC(TInt aSize);
   /**
    * Allocate a char** object and push it to the cleanupstack.
    * All char* in the char** will be set to newly allocated copies of
    * the content in aArray.
    *
    * The function returns a char***. The reason for this is that the
    * pointer then can be used as an argument to
    * <code>CleanupPopCharChar</code>. The
    * <code>CleanupPopCharChar</code> function will Pop the
    * CleanupStack without any leaks. 
    *
    * If PopAndDestroy is used it should be possible to use the
    * pointer for that purpose as well, but itäs alwasy safe to use
    * the CleanupStack::PopAndDestroy.
    *
    * @param aArray the array to copy.
    * @return pointer to a new char** with content that matches
    *         aArray.
    */   
   static char*** AllocCharCharLC(const class MDesC8Array& aArray);
   /**
    * Allocate a char** object and that is a copy of aArray.
    * @param aArray the array to copy.
    * @return pointer to a new char** with content that matches
    *         aArray.
    */
   static char** AllocCharCharL(const class MDesC8Array& aArray);

   /**
    * This function must be used to Pop a char** that was created by
    * any of the AllocCharCharLC functions.
    * @param aPtr a pointer that must be one received from one of the
    *             AllocCharCharLC functions.
    * @return the char** that aPtr pointed to. 
    */
   static char** CleanupPopCharChar(char*** aPtr);
   //@}

   /**
    * Copies the content of a char** array into a CDesC8Array. 
    * @param aDst the destination array.
    * @param aSrc the source array.
    * @param aSrcSize the size of the source array.
    * @return aDst.
    */
   static class CDesC8Array& CopyArrayL(class CDesC8Array& aDst, 
                                        const char*const* aSrc,
                                        TInt aSrcSize);
   /**
    * Creates a CDesC8Array that is a copy of a char** object.
    * @param aSrc the source array.
    * @param aSrcSize the size of the source array.
    * @return a new CDesC8Array object pushed on the CleanupStack.
    */
   static class CDesC8Array* AllocArrayLC(const char*const* aSrc,
                                          TInt aSrcSize);
   /**
    * Creates a CDesC8Array that is a copy of a char** object.
    * @param aSrc the source array.
    * @param aSrcSize the size of the source array.
    * @return a new CDesC8Array object.
    */
   static class CDesC8Array* AllocArrayL(const char*const* aSrc,
                                         TInt aSrcSize);
   /**
    * Compares a C-style string array (char**) with a Symbian array of
    * 8-bit descriptors. The arrays are considered equal if they are
    * of equal length an each element is equal. It's allowed for the
    * C-style array to contain NULL pointers. These are considered to
    * be equal to KNullDesC8.
    * @param aLhs  the C-style array (char**).
    * @param aSize the length of aLhs.
    * @param aRhs  the MDesC8Array.
    * @param       ETrue if the arrays are considered equal, EFalse
    *              otherwise.
    */
   static TBool ArrayEqual(const char* const* aLhs, const TInt aSize, 
                           const class MDesC8Array& aRhs);
   /**
    * Compares the N first elements of a C-style string array (char**)
    * with a Symbian array of 8-bit descriptors. The arrays are
    * considered equal if the MDesC8Array has at least N elements and
    * each of the N first array elements are equal. It's allowed for
    * the C-style array to contain NULL pointers. These are considered
    * to be equal to KNullDesC8.
    * @param aN    the number of elements to compare.
    * @param aLhs  the C-style array (char**). Must have at least aN
    *              elements.
    * @param aRhs  the MDesC8Array.
    * @param       ETrue if the arrays are considered equal, EFalse
    *              otherwise.
    */
   static TBool ArrayNEqual(const TInt aN, 
                            const char* const* aLhs,
                            const class MDesC8Array& aRhs);

   template<class T>
   static T* FilterArrayAllocLC(const class MDesC16Array& aOriginal, 
                                const TDesC16& aRemove)
   {
      T* filtered = new (ELeave) T(aOriginal.MdcaCount());
      CleanupStack::PushL(filtered);
      for(TInt a = 0; a < aOriginal.MdcaCount(); ++a){
         if(aRemove != aOriginal.MdcaPoint(a)){
            filtered->AppendL(aOriginal.MdcaPoint(a));
         }
      }
      return filtered;
   }
   
   template<class T>
   static T* FilterArrayAllocL(const class MDesC16Array& aOriginal, 
                               const TDesC16& aRemove)
   {
      T* filtered = FilterArrayAllocLC<T>(aOriginal, aRemove);
      CleanupStack::Pop(filtered);
      return filtered;
   }
   
   template<class T>
   static T* FilterArrayAllocL(const class MDesC16Array& aOriginal, 
                               TInt aRemove)
   {
      HBufC* remove = CCoeEnv::Static()->AllocReadResourceLC(aRemove);
      T* filtered = FilterArrayAllocL<T>(aOriginal, *remove);
      CleanupStack::PopAndDestroy(remove);
      return filtered;
   }

   /**
    * Appends the content of one RArray to another. The two RArrays
    * doesn't have to be templated on the same type as long as a
    * conversion exits.
    * @param aDst the array to append to.
    */
   template<class T, class U>
   static void AppendArrayL(RArray<T>& aDst, RArray<U>& aSrc)
   {
      for(TInt i = 0; i < aSrc.Count(); ++i){
         User::LeaveIfError(aDst.Append(aSrc[i]));
      }
   }
   
   /**
    * Searches for a value in a RArray and if it is found it will be
    * removed.  Note that if a value is present multiple times in the
    * array only the first instance will be removed.
    * @param aArray the array to search.
    * @param aValue the value to search for.
    * @return KErrNone or KErrNotFound. 
    */
   template<class T>
   static TInt FindRemove(RArray<T>& aArray, const T& aValue)
   {
      TInt pos = aArray.Find(aValue);
      if(pos != KErrNotFound){
         aArray.Remove(pos);
         pos = KErrNone;
      }
      return pos;
   }
   
   /**
    * Removes all instances in aDst of values listed in aRemove. 
    * @param aDst    The array to remove from.
    * @param aRemove The array holding values to remove. 
    */
   template<class T, class U>
   static void RemoveAll(RArray<T>& aDst, const RArray<U>& aRemove)
   {
      for(TInt i = 0; i < aRemove.Count(); ++i){
         while(KErrNone == FindRemove(aDst, aRemove[i])){
            //keep going to remove all intances.
         }
      }
   }

   /**
    * Appends elements that are in aLhs but not in aRhs to aDst.
    * This is the same as the set operation aDst = aLhs (-) aRhs.
    * If any Append operation fails the aDst array will be reverted to
    * it's original state and the function will return the error code
    * of the failed Append.
    * @param aDst the array that values will be appended to. 
    * @param aLhs the values of this array will be compared to the
    *             values of aRhs. If a value is not present in aRhs it
    *             will be appended to aDst.
    * @param aRhs the array that lists the values that shall not be
    *             appended to aDst.
    * @return the number of elements appended, or one of the status
    *         wide error codes.
    */
   template<class T>
   static TInt SetDifference(RArray<T>& aDst, const RArray<T>& aLhs, 
                             const RArray<T>& aRhs)
   {
      TInt count = 0;
      for(TInt i = 0; i < aLhs.Count(); ++i){
         const T& val = aLhs[i];
         if(KErrNotFound == aRhs.Find(val)){
            TInt err = KErrNone;
            if(KErrNone != (err = aDst.Append(val))){
               //append failed. Roll back. 
               while(count--){
                  aDst.Remove(aDst.Count() - 1);
               }
               count = err;
               break; //the for loop
            }
            ++count;
         }
      }
      return count;
   }

   /**
    * Calls SetDifference and then Leaves if SetDifference returns
    * anything other than KErrNone. The leav value is the return value
    * of SetDifference.
    *
    * @param aDst the array that values will be appended to. 
    * @param aLhs the values of this array will be compared to the
    *             values of aRhs. If a value is not present in aRhs it
    *             will be appended to aDst.
    * @param aRhs the array that lists the values that shall not be
    *             appended to aDst.
    */
   template<class T>
   static TInt SetDifferenceL(RArray<T>& aDst, const RArray<T>& aLhs, 
                              const RArray<T>& aRhs)
   {
      return User::LeaveIfError(SetDifference(aDst, aLhs, aRhs));
   }

   /**
    * Takes two arrays, that don't have to be of equal length, and
    * merges them into a new array. The resulting array will have the
    * same number of entries as the longest of the two input
    * arrays. Each entry in the new array will be the result of
    * concatenating the corresponding entries in the two input arrays
    * with a prefix, delimiter, and postfix (prefix + left[i] +
    * delimiter + right[i] + postfix).
    * If one array is shorter than the other, the missing entries in
    * that array will be considered to be the empty string.
    * @param aLeftDelim   The prefix, or left delimiter, string. 
    * @param aLeft        The array which entries will be places between 
    *                     the prefix and delimiter.
    * @param aMiddleDelim The delimeter that is placed betwen the two
    *                     strings taken from the arrays.
    * @param aRight       The array which entries will be places between 
    *                     the delimiter and the postfix.
    * @param aRightDelim  The postfix, or right delimiter, string.
    *                     Defaults to KNullDesC. 
    * @return A new CDesCArray, placed on top of the cleanup stack
    */
   static CDesCArray* PasteArrayLC(const TDesC& aLeftDelim,
                                   const MDesCArray* aLeft,
                                   const TDesC& aMiddleDelim,
                                   const MDesCArray* aRight,
                                   const TDesC& aRightDelim = KNullDesC);

   /**
    * Takes two arrays, that don't have to be of equal length, and
    * merges them into a new array. The resulting array will have the
    * same number of entries as the longest of the two input
    * arrays. Each entry in the new array will be the result of
    * concatenating the corresponding entries in the two input arrays
    * with a delimiter in between (left[i] + delimiter + right[i]).
    * If one array is shorter than the other, the missing entries in
    * that array will be considered to be the empty string.
    * @param aLeft        The array which entries will be places between 
    *                     the prefix and delimiter.
    * @param aMiddleDelim The delimeter that is placed betwen the two
    *                     strings taken from the arrays.
    * @param aRight       The array which entries will be places between 
    *                     the delimiter and the postfix.
    * @return A new CDesCArray, placed on top of the cleanup stack
    */
   static CDesCArray* PasteArrayLC(const MDesCArray* aLeft,
                                   const TDesC& aMiddleDelim,
                                   const MDesCArray* aRight);
};

