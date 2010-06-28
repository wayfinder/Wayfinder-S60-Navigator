/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <e32std.h>
#include "WayFinderSettings.h"


#ifdef NAV2_CLIENT_SERIES60_V3

# include <e32cmn.h>
# pragma data_seg(".SYMBIAN")
# define capabilities0 WayFinder_cap_num
# define FIXME_capabilities1 0x00000000u
# define secureid WayFinder_uid3
# define FIXME_vendorid 0x00000000
# define versionval (WayFinder_major_ver<<16 | WayFinder_minor_ver)
//# define versionval 0x00010000
# define FIXME_allowdlldata 1
# ifdef __WINS__
//__EMULATOR_IMAGE_HEADER2(WayFinder_uid1, WayFinder_uid2, WayFinder_uid3, 
//                         EPriorityForeground, 
//                         capabilities0, 
//                         FIXME_capabilities1, 
//                         secureid, 
//                         FIXME_vendorid, 
//                         versionval, 
//                         FIXME_allowdlldata
//                         )
# else
__EMULATOR_IMAGE_HEADER2(WayFinder_uid1, WayFinder_uid2, WayFinder_uid3, 
                         EPriorityForeground, 
                         capabilities0, 
                         FIXME_capabilities1, 
                         secureid, 
                         versionval, 
                         FIXME_allowdlldata
                         )
# endif
# pragma data_seg()

#else


# pragma data_seg(".E32_UID")
/* Uid is as follows: 
 *  First: 0x10000079 for DLL  and  0x1000007a for (EXE|EXEDLL)
 *  Second: Depends on the type of file. 
 *      For applications started with NewApplication it is 0x100039ce
 *      See trgtype.pm
 */
__WINS_UID(WayFinder_uid1, WayFinder_uid2, WayFinder_uid3)
# pragma data_seg()
#endif

