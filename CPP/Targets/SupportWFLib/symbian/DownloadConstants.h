/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef _DOWNLOAD_CONSTANTS_H_
#define _DOWNLOAD_CONSTANTS_H_

enum download_messages {
   download_handler_alreadydownloading_msg         = 0,
   download_handler_badpackagelist_msg             = 1,
   download_handler_cantconnect_msg                = 2,
   download_handler_downloadcomplete_msg           = 3,
   download_handler_downloadcomplete_text          = 4,
   download_handler_downloadfailed_msg             = 5,
   download_handler_downloadfailed_text            = 6,
   download_handler_downloading_msg                = 7,
   download_handler_downloading_prompt_text        = 8,
   download_handler_downloadparts1_text            = 9,
   download_handler_downloadparts2_text            = 10,
   download_handler_empty_text                     = 11,
   download_handler_installing_text                = 12,
   download_handler_installreturnederror_msg       = 13,
   download_handler_installwaiterror_msg           = 14,
   download_handler_nopackagelist_msg              = 15,
   download_handler_nopackagelist_text             = 16,
   download_handler_no_user_file_msg               = 17,
   download_handler_packagelistread_msg            = 18,
   download_handler_partof1_text                   = 19,
   download_handler_partof2_text                   = 20,
   download_handler_readpackagelist_text           = 21,
   download_handler_spacewarning1_msg              = 22,
   download_handler_spacewarning2_msg              = 23,
   download_handler_spacewarning3_msg              = 24,
   download_handler_start_the_download             = 25,
   download_handler_unreadablepackagelist_msg      = 26,
   download_handler_unreadable_user_file_msg       = 27,
   download_handler_uptodate2_msg                  = 28,
   download_handler_uptodate3_msg                  = 29,
   download_handler_internal_error_msg             = 30,
   download_handler_download_not_needed_version_ok = 31,
};

#endif /* _DOWNLOAD_CONSTANTS_H_ */

