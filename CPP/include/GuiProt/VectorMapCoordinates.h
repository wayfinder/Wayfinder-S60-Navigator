/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef VECTORMAPCOORDINATES_H
#define VECTORMAPCOORDINATES_H

#include "arch.h"
#include "GuiProt/GuiProtMess.h"

namespace isab {

class VectorMapCoordinates
{
public:

   VectorMapCoordinates::VectorMapCoordinates(Buffer* buf);
/*    VectorMapCoordinates(GeneralParameterMess* message); */
   VectorMapCoordinates(int32 scale, int32 lat, int32 lon);
   ~VectorMapCoordinates();

   /**
    * Creates a new GeneralParameterMess.
    */
   GeneralParameterMess* CreateGeneralParameter();

   int32 GetScale() { return m_scale; }
   int32 GetLat()   { return m_lat; }
   int32 GetLon()   { return m_lon; }
   bool Valid()     { return m_valid; }

private:
   int32 m_scale;
   int32 m_lat;
   int32 m_lon;
   bool m_valid;

};

}

#endif /* VECTORMAPCOORDINATES_H */
