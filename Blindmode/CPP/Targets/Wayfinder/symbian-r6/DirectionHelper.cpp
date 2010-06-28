/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DirectionHelper.h"
#include "FormatDirection.h"
#include "WFTextUtil.h"
#include <stdio.h>
#include <eikenv.h>
#include <e32base.h>
#include "RsgInclude.h"

void 
DirectionHelper::AppendDirection( TPtr& connector,
                                  const MC2Direction& direction,
                                  int heading,
                                  bool headingUsable,
                                  MC2Direction::RepresentationType directionType )
{
   HBufC* degrees = NULL;

   if ( ! headingUsable ) {
      // Fall back to direction based if no reliable heading.
      directionType = MC2Direction::DirectionBased;
   }

   if( directionType == MC2Direction::ClockBased ) {
      degrees = FormatDirection::ParseLC(direction.Hour(heading));
   }
   else {
      degrees = FormatDirection::ParseLC(direction.CompassPoint());
      
   }
      
   connector.Append(*degrees);

   if (directionType == MC2Direction::DirectionBased ) {
      // Also add actual degrees.
      char* tmp = new char[10];
      sprintf( tmp, ", %d", direction.GetAngle() );
      HBufC* angle = WFTextUtil::AllocLC( tmp );

      connector.Append( *angle );

      HBufC* degreeSign = CEikonEnv::Static()->AllocReadResourceLC(R_WF_DEGREE_SIGN);
      connector.Append( *degreeSign );
      
      delete[] tmp;
      CleanupStack::PopAndDestroy( degreeSign );
      CleanupStack::PopAndDestroy( angle );
   }
   
   CleanupStack::PopAndDestroy(degrees);
}
