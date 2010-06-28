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

#ifndef DOWNLOAD_GUI_H
#define DOWNLOAD_GUI_H
#include <e32base.h>

class MDownloadGui
{
public:

   virtual void setDownloadInstr(TInt string) = 0;

   virtual void ShowDownloadInformation( TInt aResourceId ) = 0;
   virtual TBool ShowDownloadQuery( TInt aResourceId ) = 0;
   virtual void setDownloadLabel(TInt string) = 0;
   virtual void setDownloadStatus(TInt string) = 0;
   virtual void setDownloadStatus(TInt string, const TDesC& aText) = 0;
   virtual void ShowDownloadError( TInt aResourceId ) = 0;
   virtual void ShowDownloadErrorDialog( TInt aResourceId ) = 0;
   virtual void ShowSocketError( const TDesC& aText ) = 0;
   virtual void ShowSocketError( TInt aResourceId ) = 0;
   virtual void ShowSocketInformation( TInt aResourceId ) = 0;

   virtual void SetInstrDownloadNumPartsL(TInt num) = 0;
   virtual void ShowSpaceWarningL(TInt64 free, TInt needed) = 0;
   virtual void SetInstrDownloadPartXOfYL(TInt partnum, TInt numparts) = 0;
   virtual void ProgressFinishedL() = 0;
   virtual void ProgressStatusL(TInt aProgress) = 0;
   virtual void ProgressStartL() = 0;
   virtual void DownloadComplete() = 0;
   virtual void DownloadFailed() = 0;

   virtual void SetDownloadObserver(class MDownloadGuiObserver* aObserver) = 0;
};

#endif /* DOWNLOAD_GUI_H */
