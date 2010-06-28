/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef BUFFERARRAY_H
#define BUFFERARRAY_H

// INCLUDES
#include "WayFinderConstants.h"
#include "Log.h"

class CBufferElement;

// CONSTANTS
//const ?type ?constant_var = ?constant;

// CLASS DECLARATION

/**
* Message handling class.
* 
*/
class CBufferArray
{

public: // // Constructors and destructor

   CBufferArray(isab::Log* aLog);

   /**
    * Destructor.
    */
   ~CBufferArray();

public: // New functions

   void InitArray();

   TInt GetNbrElements();

   HBufC* GetElementAt( TInt aIdx );

   void AddElementL( TDesC &aString, isab::Log* aLog );
   
   void InsertElementL( TDesC &aString, TInt aIdx, isab::Log* aLog );

   void RemoveElement( TInt aIdx );

   TBool IsDuplicate(const TDesC &aString );

private:

   void ResetArray();

private: //Data

	CArrayPtrFlat<CBufferElement>* iElements;
   isab::Log* iLog;
};


class CBufferElement : public CBase
{

public :

   CBufferElement(isab::Log* aLog);

	~CBufferElement();

   /// Deletes the members.
   void Clear();

	void SetTextL(const TDesC& aText);

   HBufC* GetText() const;

public :

	HBufC* iBuf;
   isab::Log* iLog;

};

#endif

// End of File
