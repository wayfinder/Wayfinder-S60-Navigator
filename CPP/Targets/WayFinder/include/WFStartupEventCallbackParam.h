/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WF_STARTUP_EVENT_CALLBACK_PARAM_H
#define WF_STARTUP_EVENT_CALLBACK_PARAM_H

#include <arch.h>

/**
 * Class used to encapsulate parameters to the callback events in startup
 */
class WFStartupEventCallbackParameter
{
public:
   /**
    * Constructor, no data
    */
   WFStartupEventCallbackParameter()
      : m_type(0), m_error(0), m_requestType(0), m_errorMsg(NULL) {}
   /**
    * Constructor, data
    * @param type       Type of event
    * @param error      Type of error
    * @param reqType    Type of request
    * @param errorMsg   Optional error string
    */
   WFStartupEventCallbackParameter(int32 type, int32 error,
         int32 reqType, char* errorMsg = NULL)
      : m_type(type), m_error(error),
        m_requestType(reqType), m_errorMsg(errorMsg)
   {}

   /**
    * Destructor
    */
   virtual ~WFStartupEventCallbackParameter() 
   {
      delete[] m_errorMsg;
   }

   /**
    * getType
    * @return type of event
    */
   virtual int32 getType() { return m_type; }
   /**
    * getError
    * @return type of error
    */
   virtual int32 getError() { return m_error; }
   /**
    * getRequestType
    * @return type of request
    */
   virtual int32 getRequestType() { return m_requestType; }
   /**
    * getErrorMsg
    * @return error string
    */
   virtual char* getErrorMsg() { return m_errorMsg; } 

   /**
    * Type of event
    */
   int32 m_type;
   /**
    * Type of error
    */
   int32 m_error;
   /**
    * Type of request
    */
   int32 m_requestType;
   /**
    * Error string
    */
   char* m_errorMsg;
};

/**
 * Upgrade callback parameter
 * Used to encapsulate upgrade data for callback to GUI
 */
class WFStartupEventUpgradeCallbackParameter :
   public WFStartupEventCallbackParameter
{
public:
   /**
    * Constructor, no data
    */
   WFStartupEventUpgradeCallbackParameter()
      : WFStartupEventCallbackParameter(SP_Upgrade, 0, 0), 
        m_keyStr(NULL), m_phone(NULL), m_name(NULL), 
        m_email(NULL), m_countryId(-1) {}
   /**
    * Constructor, data
    * @param keystr        Activation key string
    * @param phone         Phone number string
    * @param name          Name string
    * @param email         Email string
    * @param optional      Optional parameter string
    * @param countryid     Country id
    */
   WFStartupEventUpgradeCallbackParameter(char* keystr, char* phone,
         char* name, char* email, char* optional, int32 countryid)
      : WFStartupEventCallbackParameter(SP_Upgrade, 0, 0), 
        m_keyStr(keystr), m_phone(phone), m_name(name), 
        m_email(email), m_optional(optional), m_countryId(countryid) {}

   /**
    * Destructor
    */
   virtual ~WFStartupEventUpgradeCallbackParameter() 
   {
      delete[] m_keyStr;
      delete[] m_phone;
      delete[] m_name;
      delete[] m_email;
      delete[] m_optional;
   }

   char* m_keyStr;
   char* m_phone;
   char* m_name;
   char* m_email;
   char* m_optional;
   int32 m_countryId;
};

/**
 * Upgrade failed parameter
 * Used to encapsulate upgrade failed data
 */
class WFStartupEventUpgradeFailedCallbackParameter :
   public WFStartupEventCallbackParameter
{
public:
   /**
    * Constructor, no data
    */
   WFStartupEventUpgradeFailedCallbackParameter()
      : WFStartupEventCallbackParameter(SP_UpgradeFailed, 0, 0), 
        m_keyStrOk(false), m_phoneOk(false), m_nameOk(false), 
        m_emailOk(false), m_countryIdOk(false) {}
   /**
    * Constructor, data
    * @param keystrOk      True if activation string was ok
    * @param phoneOk       True if phone string was ok
    * @param nameOk        True if name string was ok
    * @param emailOk       True if email string was ok
    * @param countryidOk   True if countryid was ok
    */
   WFStartupEventUpgradeFailedCallbackParameter(bool keystrOk, bool phoneOk,
         bool nameOk, bool emailOk, bool countryidOk)
      : WFStartupEventCallbackParameter(SP_UpgradeFailed, 0, 0), 
        m_keyStrOk(keystrOk), m_phoneOk(phoneOk), m_nameOk(nameOk), 
        m_emailOk(emailOk), m_countryIdOk(countryidOk) {}

   /**
    * Destructor
    */
   virtual ~WFStartupEventUpgradeFailedCallbackParameter() {}

   bool m_keyStrOk;
   bool m_phoneOk;
   bool m_nameOk;
   bool m_emailOk;
   bool m_countryIdOk;
};

/**
 * String data with success and failure url callback parameter
 * Used to encapsulate one string data and two urls for callback to GUI
 */
class WFStartupEventStringAndUrlsCallbackParameter :
   public WFStartupEventCallbackParameter
{
public:
   /**
    * Constructor, no data
    */
   WFStartupEventStringAndUrlsCallbackParameter()
      : WFStartupEventCallbackParameter(SP_None, 0, 0), 
        m_stringData(NULL), m_successUrl(NULL), m_failureUrl(NULL) 
   {}

   /**
    * Constructor, data
    * @param strdata      The string that we want to pass to the event handler
    * @param successurl   Url to goto when the event completes successfully
    * @param failureurl   Url to goto when the event completes with a failure
    * @param type         Type of parameter
    */
   WFStartupEventStringAndUrlsCallbackParameter(char* strdata, 
                                                char* successurl,
                                                char* failureurl, 
                                                int32 type)
      : WFStartupEventCallbackParameter(type, 0, 0), 
        m_stringData(strdata),
        m_successUrl(successurl), 
        m_failureUrl(failureurl) 
   {}
   
   /**
    * Destructor
    */
   virtual ~WFStartupEventStringAndUrlsCallbackParameter() 
   {
      delete[] m_stringData;
      delete[] m_successUrl;
      delete[] m_failureUrl;
   }

   char* m_stringData;
   char* m_successUrl;
   char* m_failureUrl;
};

#endif
