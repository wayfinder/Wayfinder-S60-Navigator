/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef COPYRIGHT_HANDLER 
#define COPYRIGHT_HANDLER 

#include "config.h"

#include "BitBuffer.h"
#include <vector>
#include "MC2SimpleString.h"
#include "MC2BoundingBox.h"
#include "CopyrightHolder.h"

/**
 *    Class that handles determining which copyright string for
 *    map providers that should be shown for a certain area.
 */
class CopyrightHandler {
public:

   /**
    *    Constructor.
    */
   CopyrightHandler();
   
   /**
    *    Set the copyright holder.
    *    @param holder  The copyright holder.
    */
   void setCopyrightHolder( const CopyrightHolder* holder );
   
   
   /**
    *    Get the copyright string for the specified area. 
    *    @param screenBoxes      The bounding boxes of the area.
    *    @param copyrightString  Pre allocated string which will be set to
    *                            contain copyright information.
    *    @param maxLength        The maximum allowed length that the
    *                            copyright string may contain.
    */
   void getCopyrightString( const std::vector<MC2BoundingBox>& screenBoxes,
                            char* copyrightString,
                            int maxLength );
  
   /**
    *
    */
   void getCopyrightString( char* copyrightString,
                            int maxLength );

   /**
    *    Set the static copyright string to use in case it's not possible
    *    to create a dynamic copyright string.
    */
   void setStaticCopyrightString( const MC2SimpleString& crString );

private:

   /**
    *    Initiate with hardcoded data.
    */
   void initWithHardcodedData();

   /**
    *    Recursive method that checks map supplier coverage.
    */
   int64 recursiveCheckCoverage( const MC2BoundingBox& screenBox,
                                int id,
                                std::vector<std::pair<int64,int> >& covByCopyrightId );
  
   /// The copyright holder.
   const CopyrightHolder* m_holder;

   /// Static copyright string. Empty if no static copyright must be used.
   MC2SimpleString m_staticCopyrightString;
   
   /// The current (last) copyright string used.
   MC2SimpleString m_curCopyrightString;
   
   typedef CopyrightHolder::range_t range_t;
   typedef CopyrightHolder::mapIt_t mapIt_t;
};

// -- Inlined functions

#endif // COPYRIGHT_HANDLER 

