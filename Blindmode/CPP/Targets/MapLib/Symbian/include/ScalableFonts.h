/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Defines names for logical fonts that can be
// used on s60v3.

#ifndef SCALABLE_FONTS_H
#define SCALABLE_FONTS_H

#define PRIMARY_LOGICAL_FONT         "primaryLogicalFont"
#define PRIMARY_SMALL_LOGICAL_FONT   "primarySmallLogicalFont"
#define SECONDARY_LOGICAL_FONT       "secondaryLogicalFont"
#define TITLE_LOGICAL_FONT           "titleLogicalFont"
#define DIGITAL_LOGICAL_FONT         "digitalLogicalFont"

_LIT(KPrimaryLogicalFont, "primaryLogicalFont");
_LIT(KPrimarySmallLogicalFont, "primarySmallLogicalFont");
_LIT(KSecondaryLogicalFont, "secondaryLogicalFont");
_LIT(KTitleLogicalFont, "titleLogicalFont");
_LIT(KDigitalLogicalFont, "digitalLogicalFont");

enum TScalableFonts {
   EPrimaryLogicalFont      = 0,
   EPrimarySmallLogicalFont = 1,
   ESecondaryLogicalFont    = 2,
   ETitleLogicalFont        = 3,
   EDigitalLogicalFont      = 4,
   ETotalNbrFonts           = 5, 
};

#endif
