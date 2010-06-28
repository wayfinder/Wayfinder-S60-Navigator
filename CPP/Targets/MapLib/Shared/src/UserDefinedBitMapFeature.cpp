/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include <string.h>

#include "UserDefinedBitMapFeature.h"

UserDefinedBitMapFeature::
UserDefinedBitMapFeature( const ScreenOrWorldCoordinate& center,
                          const char* serverBitMapName,
                          bool shouldScale )
      : UserDefinedFeature( UserDefinedFeature::bitmap,
                            std::vector<MC2Point>(1, MC2Point(0,0)),
                            center ),
        m_shouldScale(shouldScale)
{ 
   m_widthPixels = 0;
   m_heightPixels = 0;
   
   // Default should be 2d.
   m_always2d = true;
   if ( serverBitMapName ) {
      m_bitMapName = new char[strlen(serverBitMapName) + 1];
      strcpy(m_bitMapName, serverBitMapName);
   } else {
      m_bitMapName = NULL;
   }
}

UserDefinedBitMapFeature::~UserDefinedBitMapFeature()
{
   delete [] m_bitMapName;
}

const char*
UserDefinedBitMapFeature::getBitMapName() const
{
   return m_bitMapName;
}

int UserDefinedBitMapFeature::getWidth() const
{
   return m_widthPixels;
}

int UserDefinedBitMapFeature::getHeight() const
{
   return m_heightPixels;
}

void UserDefinedBitMapFeature::setDimensions(int widthPixels, int heightPixels)
{
   m_widthPixels = widthPixels;
   m_heightPixels = heightPixels;
}

bool UserDefinedBitMapFeature::shouldScale() const
{
   return m_shouldScale; 
}

