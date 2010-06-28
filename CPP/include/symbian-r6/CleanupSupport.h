/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef CLEANUPSUPPORT_H
#define CLEANUPSUPPORT_H
#include <coemain.h>
#include <badesca.h>

#include "GuiProt/GuiProtMess.h"

///Function that can be pushed onto the cleanupstack to clean up fonts.
///@param ptr Should be a CFont pointer.
inline void CleanupFont(TAny* ptr)
{
   CCoeEnv::Static()->ScreenDevice()->ReleaseFont(static_cast<CFont*>(ptr));
}

///Constructs and pushes a TCleanupItem object onto the cleanup stask.
///The TCleanupItem encapsulates a CFont object and a function pointer
///to CleanupFont. CleanupFont is invoked by a subsequent call to
///CleanupStack::PopAndDestroy().
///@param aFont a pointer to a CFont object that needs to be released
///             later.
inline void CleanupReleaseFontPushL(CFont* aFont){
   CleanupStack::PushL(TCleanupItem(CleanupFont, aFont)); 
}

///This template class if a helper object for deleting non-CBase
///derived classes and arrays where the class's destructor needs to be
///called. It has only static functions and should never be instantiated. 
template<class T>
class CleanupClass{
   ///private to avoid instantiation.
   CleanupClass();
   ///private to avoid instantiation.
   CleanupClass(const CleanupClass&);
   ///private to avoid instantiation.
   const CleanupClass& operator=(const CleanupClass&);
   ///The type to use on the pointer when deleting it.
   typedef T* ptr_t;
public:
   ///Casts the argument to ptr_t and deletes it.
   ///@param any the poionter to delete.
   static void Cleanup(void* any){
      delete static_cast<ptr_t>(any);
   }
   ///Casts the argument to ptr_t and delete[]s it.
   ///@param any the pointer to delete.
   static void CleanupArray(void* any){
      delete[] static_cast<ptr_t>(any);
   }
};

inline void CleanupCharArrayPushL(char* array)
{
   CleanupStack::PushL(TCleanupItem(CleanupClass<char>::CleanupArray, array)); 
}

///Thes template function matches the CleanupClass::Cleanup function,
///but is unusable in VC6.
//@{
template<typename type>
static void Cleanup(void* any){ delete static_cast<type*>(any); }
template<typename type>
static void CleanupArray(void* any){ delete[] static_cast<type*>(any); }
//@}

template<class T>
void CleanupClassPushL(T* aItem)
{
   CleanupStack::PushL(TCleanupItem(CleanupClass<T>::Cleanup, aItem));
}

///This function matches the TCleanupOperation function pointer
///typedef, and can be used as a TCleanupItemArgument in combination
///with a CDesCArray derived object.
///On a Leave or PopAndDestroy it will remove the last descriptor in the array.
///If the function is called with a empty array as argument, it's a no-op.
///@param aCDesCArray a TAny pointer taht will be static_cast to a 
///                   CDesCArray pointer.
static void PopBack(TAny* aCDesCArray)
{
   CDesCArray& array = *static_cast<CDesCArray*>(aCDesCArray);
   if(array.Count() > 0){
      array.Delete(array.Count() - 1);
   }
}

///This function matches the TCleanupOperation function pointer
///typedef, and can be used as a TCleanupItemArgument in combination
///with a CDesCArray derived object.
///On a Leave or PopAndDestroy it will remove the first descriptor 
///in the array.
///If the function is called with a empty array as argument, it's a no-op.
///@param aCDesCArray a TAny pointer that will be static_cast to a 
///                   CDesCArray pointer.
static void PopFront(TAny* aCDesCArray)
{
   CDesCArray& array = *static_cast<CDesCArray*>(aCDesCArray);
   if(array.Count() > 0){
      array.Delete(0);
   }
}

///This function appends a descriptor to an array, and pushes a
///PopBack cleanupitem to the cleanupstack. Do this if you want the
///array to roll back to its initial state in a leave.
///@param aArray the array to append to.
///@param aDes the descriptor to append.
inline void CleanupPushAppendL(CDesCArray* aArray, const TDesC& aDes)
{
   aArray->AppendL(aDes);
   CleanupStack::PushL(TCleanupItem(PopBack, aArray));
}

///This function inserts a descriptor at the beginning of an array,
///and pushes a PopBack cleanupitem to the cleanupstack. Do this if
///you want the array to roll back to its initial state in a leave.
///@param aArray the array to insert into.  
///@param aDes the descriptor to insert.
inline void CleanupPushInsertL(CDesCArray* aArray, const TDesC& aDes)
{
   aArray->InsertL(0, aDes);
   CleanupStack::PushL(TCleanupItem(PopFront, aArray));
}


template<class T>
class SignalItem
{
public:
   static void Signal(TAny* aAny){
      static_cast<T*>(aAny)->Signal();
   }
};

template<class T>
void CleanupSignalPushL(T& aRef)
{
   CleanupStack::PushL(TCleanupItem(SignalItem<T>::Signal, &aRef));
}


inline void DeleteMembers(TAny* aAny)
{
   class isab::GuiProtMess* mess = static_cast<isab::GuiProtMess*>(aAny);
   mess->deleteMembers();
}

inline void CleanupDeleteMembersPushL(class isab::GuiProtMess* aMess)
{
   CleanupStack::PushL(TCleanupItem(DeleteMembers, aMess));
}

inline void CleanupDeleteMembersPushL(class isab::GuiProtMess& aMess)
{
   CleanupStack::PushL(TCleanupItem(DeleteMembers, &aMess));
}

#endif
