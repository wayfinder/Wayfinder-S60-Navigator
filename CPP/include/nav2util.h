/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef __NAV2_UTIL_H__
#define __NAV2_UTIL_H__

#include "arch.h"
#include <string.h>
#include <functional>

namespace isab {

   /** A C++ version of strdup, that uses new[] instead of malloc,
    * thereby allowing the use of delete[] instead of free. 
    * @param src the string to copy. May be NULL.
    * @return a dynamically allocated duplicate of the src param, 
    *         or NULL if <code>src</code> is NULL. Remember
    *         to not forget to delete[] it when you are done.*/
   char* strdup_new(const char* src);

   /** Copies src to dst. Checks with strlen(dst) if there is enough
    * memory available and reallocates if necessary. If dst is NULL
    * new memory is allocated.
    * @param dst the destination pointer. Note that it is a reference.
    * @param src the String to copy.
    * @return a pointer to the destination string <code>dst</code>.
    */
   char* replaceString(char*& dst, const char* src);

   /** Compares two strings for equality.
    * @param a the first string.
    * @param b the second string.
    * @return true if the strings are identical, false otherwise.*/
   inline bool strequ(const char* a, const char* b)
   {
      return 0 == strcmp(a, b);
   }

   inline bool strcaseequ(const char* a, const char* b)
   {
      return 0 == strcasecmp(a, b);
   }

   /** If *stringp is NULL, the strsep() function returns NULL and
    * does nothing else. Otherwise, this function finds the first
    * token in the string *stringp, where tokens are delimited by
    * symbols in the string delim.  This token is terminated with a
    * `\0' character (by overwriting the delimiter) and *stringp is
    * updated to point past the token.  In case no delimiter was
    * found, the token is taken to be the entire string *stringp,
    * and *stringp is made NULL.
    *
    * @param stringp a pointer to a pointer to the string to be separated.
    *                Points to the remaining string after a succesfull 
    *                call.
    * @param delim   a pointer to an array of delimiter characters.
    * @return a pointer to the token, that is, the original 
    *         value of *stringp
    */
   char* strsep(char**stringp, const char* delim);

   /** Analyses a string to see if it fits the pattern <[host:]port>.
    * Note that the arg string is modified, and the host pointer will
    * refer to parts of this string. However, if the function returns false
    * the arg string remains untouched.
    * @param arg   the string to analyse.
    * @param host  the place where the host name is found after the call.
    *              *host is set to NULL if there was no host part in arg.
    * @param port  the return variable for the port part.
    * @return true if arg was parsed succesfully, false if the arg string 
    *              didn't match the pattern. If the function returns 
    *              false, the arg string was not modified.
    */
   bool parseHost(char* arg, char** host, uint16* port);

   /** Analyses a string to see if it fits the regexp
    * [0-9]+[.][0-9]+[.][0-9]+. If it does the numerical values are
    * stored in order in the array <code>target</code>.
    * @param target        an array where as many as possible of the 
    *                      numbers are stored.
    * @param versionString a string to examine.
    * @return true if the string matched the regexp above. */
   bool parseVersion(uint32 target[3], const char* versionString);

   /** Converts a c-style string to a unicode string by copying each 
    * character from one string to the other. The target string is
    * always nul terminated.
    * @param n   the maximum number of bytes written.
    * @param dst the destination unicode string. Note taht it is the 
    *            callers responibility to ensure that there are at least n
    *            16 bit characters available to write to.
    * @param src the source string.
    * @return the number of characters written, including the nul character.
    */
   int char2unicode(int n, uint16* dst, const char* src);

   /** Collapses an int to -1 or 1 depending on if the number is 
    *  negative or positive. If the value is 0, 1 is returned.
    * 
    *  @param n The number to check.
    *
    *  @return  Returnes  -1 or 1 depending on if the number is 
    *           negative or positive. If the value is 0, 1 is 
    *           returned.
    */
   inline int sign(int n)
   {
      int result = 1;
      if (n < 0){
         result = -1;
      }
      else if(n == 0){
         result = 0;
      }
      return result;
   }

#if defined(_MSC_VER) && !defined(__SYMBIAN32__)
   typedef TCHAR tchar;
   /** Creates a tchar copy of a char string, with all '\n' replaced with
    * "\r\n". This function allocates memory and uses <code>duplicate</code>
    * to return it. It's the callers responsibility to delete[] it.
    * @param original  a pointer to a char-string.
    * @param duplicate a reference to a pointer to a tchar-string. This is 
    *                  where the duplicate is returned.
    * @return the number of bytes in the duplicate, including terminating 
    *         '\0'-byte.
    */
   int dupChar2Tchar(const char* original, tchar*& duplicate);
#endif

   /** Unary functor class used to delete all objects from any container of 
       pointers to objects. Note: uses ordinary delete.*/
   template<class T>
   class Delete : public std::unary_function<T, void>{
   public:
      //result_type operator()(argument_type arg)
      void operator()(T arg){
         delete arg;
      }
   };

   /** Unary functor class used to delete all objects from any container of 
       pointers to arrays of objects. Note: uses ordinary delete.*/
   template<class T>
   class DeleteArray : public std::unary_function<T, void>{
   public:
      void operator()(T arg){
         delete[] arg;
      }
   };

   /** Unary functor class that can be used to Clone any object that has
       a copy constructor.*/
   template<class To, class From = To>
   class Clone : public std::unary_function<const From*, To*>{
   public:
      //result_type operator()(argument_type arg)
      To* operator()(const From* arg)
      {
         return new To(*arg);
      }
   };

   /// Binary functor that lets you use any of the standard operator
   /// functor objects from <functional> on containers of pointer.
   template<class T, class op>
   class PointerOperator : 
      public std::binary_function<const T*, const T*, 
                                  typename op::result_type>{
      op m_functor;
   public:
      //result_type operator()(first_argument_type a, 
      //                       second_argument_type b) const
      typename op::result_type operator()(const T* a, 
                             const T* b) const
      {
         return m_functor(*a, *b);
      }
   };


   /// A more than a bit arcane predicate functor that can be used
   /// to find thing in container of pointer to objects that have a
   /// function that returns a string.
   template<class T>
   class MatchString : public std::unary_function<const T*, bool>{
   public:
      typedef const char* (T::*stringFun)() const;
   private:
      const char* mtch;
      stringFun call;
   public:
      MatchString(const char* s, stringFun fun) : mtch(s), call(fun) {}
      //result_type operator()(argument_type arg)
      bool operator()(const T* arg){
         return strequ((arg->*call)(), mtch);
      }
   };

   ///Converts a member function with no parameters to a function with
   ///no parameters.
   template<class T, class Result>
   class MemFunPtr{
   public:
      typedef Result (T::*function_t)();
      MemFunPtr(T* obj, function_t func) : m_obj(*obj), m_func(func) {}
      MemFunPtr(T& obj, function_t func) : m_obj(obj), m_func(func) {}
      Result operator()()
      {
         return (m_obj.*m_func)();
      }
   private:
      T& m_obj;
      function_t m_func;
   };


   /// Squares a.
   template<class T>
   inline T square( const T& a ) {
      return a*a;
   }


   /** 
    * The number of bits set in x. (Copied from Utility.h in mc2).
    * 
    * @param x The uint32 to count set bits in.
    * @return The number of bits set in x. 
    */ 
   inline uint32 nbrBits( uint32 x ){ 
      x = (x >> 1 & 0x55555555) + (x & 0x55555555); 
      x = (x >> 2 & 0x33333333) + (x & 0x33333333); 
      x = (x >> 4 & 0x0f0f0f0f) + (x & 0x0f0f0f0f); 
      x = (x >> 8 & 0x00ff00ff) + (x & 0x00ff00ff); 
      return (x >> 16) + (x & 0xffff); 
   }

} // End namespace isab
#endif
