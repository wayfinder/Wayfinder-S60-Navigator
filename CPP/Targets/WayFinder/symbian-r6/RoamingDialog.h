/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ROAMINGDIALOG_H
#define ROAMINGDIALOG_H

#include <e32base.h>
#include <aknstaticnotedialog.h>
#include <coecobs.h>


class MTimerCallback
{
public:
  virtual void HandleTimedOut() = 0;
};

class CTimerCallback : public CTimer
{
public:
  static CTimerCallback* NewL(MTimerCallback& aCallback, TInt aTimeout);
  
private:
  CTimerCallback(MTimerCallback& aCallback);
  void ConstructL(TInt aTimeout);
  
  void RunL();
private:
  MTimerCallback& iCallback;
};

class MRoamingCallback
{
public:

  enum TRoamingStatus {
    ETimeout,
    EAccept,
    EExit
  };

  virtual void HandleRoamingDialogDone(TRoamingStatus aStatus) = 0;
};

class CRoamingDialog : public CAknStaticNoteDialog, public MTimerCallback
{
public:
  static CRoamingDialog* NewL(TDes& aDataText, MRoamingCallback* aCallback);
  ~CRoamingDialog();

public:
  //MTimerCallback
  void HandleTimedOut();
public:
  TBool OkToExitL(TInt aButtonId);
public:
  // CAknTextQueryDialog
  void SetTone(TTone aTone);
  void SetTimeout(TTimeout aTimeout);
  void PostLayoutDynInitL();
  void Start();
  void ExitDialogL();
 private:
  MRoamingCallback* iRoamingCallback;
  CTimerCallback* iCallBack;
  TTimeout iTimeout;
  TTone iTone;
private:
  CRoamingDialog(TDes& aDataText, MRoamingCallback* aCallback);

};
#endif //  ROAMINGDIALOG_H
