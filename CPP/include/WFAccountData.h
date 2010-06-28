/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef WF_ACCOUNT_DATA_H
#define WF_ACCOUNT_DATA_H

#include "arch.h"
#include "GuiProt/AccountStatus.h"
#include "GuiProt/ServerEnums.h"

/**
 */
class WFAccountData
{
public:
   WFAccountData(char *usernameClipString) :
      m_wfType(isab::GuiProtEnums::InvalidWayfinderType),
      m_accountStatus(isab::GuiProtEnums::AS_Unknown),
      m_user(NULL), m_pass(NULL), m_webuser(NULL), m_webpass(NULL),
      m_imei(NULL), m_clipString(usernameClipString)
   {}

   WFAccountData(enum isab::GuiProtEnums::WayfinderType wftype,
         enum isab::GuiProtEnums::AccountStatus accountStatus,
         char *usernameClipString) :
      m_wfType(wftype),
      m_accountStatus(accountStatus),
      m_user(NULL), m_pass(NULL), m_webuser(NULL), m_webpass(NULL),
      m_imei(NULL), m_clipString(usernameClipString)
   {}

   ~WFAccountData() {
      delete[] m_user;
      delete[] m_pass;
      delete[] m_webuser;
      delete[] m_webpass;
      delete[] m_imei;
      delete[] m_clipString;
   }

   void setAccountStatus(enum isab::GuiProtEnums::AccountStatus status) {
      m_accountStatus = status;
   }
   void setWfType(enum isab::GuiProtEnums::WayfinderType wftype) {
      m_wfType = wftype;
   }

   /**
    * set username to string. will take ownership of char *
    */
   void setUserName(char *user)
   { delete[] m_user; m_user = user; }
   /**
    * set password to string. will take ownership of char *
    */
   void setPassword(char *pass)
   { delete[] m_pass; m_pass = pass; }
   /**
    * set web username to string. will take ownership of char *
    */
   void setWebUserName(char *webuser)
   { delete[] m_webuser; m_webuser = webuser; }
   /**
    * set web password to string. will take ownership of char *
    */
   void setWebPassword(char *webpass)
   { delete[] m_webpass; m_webpass = webpass; }
   /**
    * set imei to string. will take ownership of char *
    */
   void setImei(char *imei)
   { delete[] m_imei; m_imei = imei; }

   /**
    * Getter methods.
    */
   enum isab::GuiProtEnums::WayfinderType getWfType() { return m_wfType; }
   enum isab::GuiProtEnums::AccountStatus getAccountStatus()
   { return m_accountStatus; }
   const char* getUsername() { return m_user; }

   const char* getPassword() { return m_pass; }
   const char* getUnclippedWebUserName() { return m_webuser; }
   const char* getWebUserName()
   {
      if (m_clipString &&
          m_webuser &&
          strlen(m_webuser) > strlen(m_clipString) &&
          !strncmp(m_clipString, m_webuser, strlen(m_clipString))) {
         return &m_webuser[strlen(m_clipString)];
      }
      return m_webuser;
   }
   const char* getWebPassword() { return m_webpass; }
   const char* getImei() { return m_imei; }

private:
   enum isab::GuiProtEnums::WayfinderType m_wfType;
   enum isab::GuiProtEnums::AccountStatus m_accountStatus;

   char* m_user;
   char* m_pass;
   char* m_webuser;
   char* m_webpass;
   char* m_imei;
   char* m_clipString;
};

#endif

