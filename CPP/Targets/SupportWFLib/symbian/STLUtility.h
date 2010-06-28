/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef STL_UTILITY_H
#define STL_UTILITY_H

#include "arch.h"

/// calls a member function for a specific instance.
/// i.e use when having a vector of function pointers.
/// Example of using:
/// typedef DoCallback<Class>::Action ClassCallback
/// typedef std::vector<ClassCallback> ptmf_vector_type;
/// m_ptmfVec.push_back(&Class::Function);
/// std::for_each(m_ptmfVec.begin(), m_ptmfVec.end(), makeCallback(*Instance));
template <typename Instance>
struct DoCallback {
   typedef void (Instance::* Action)();
   explicit DoCallback( Instance& i ):m_instance( i ) { }
   void operator()( Action action ) {
      (m_instance.*action)();
   }
   Instance& m_instance;
};

/// helper function to create a DoCallback.
/// Instead of explicit naming the type,
/// i.e for_each( begin, end, DoCallback<InstanceType>( instance ) );
/// use for_each( begin, end, makeCallback( instance ) );
template <typename T>
DoCallback<T> makeCallback( T& instance ) {
   return DoCallback<T>( instance );
}

#endif
