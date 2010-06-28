/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef S60_WF_VIEW_STACK_H
#define S60_WF_VIEW_STACK_H

#include "WFViewStack.h"

class CWayFinderAppUi;

/**
 * S60 implementation of a siew stack.
 * Derives from WFViewStack.
 *
 * TODO: Perhaps interface not CWayFinderAppUi.
 */
class S60WFViewStack : public WFViewStack {
   public:
      /**
       * Constructor.
       */
      S60WFViewStack( CWayFinderAppUi* appUi, 
                      ViewID invalidViewID, GroupID invalidGroupID,
                      ViewID mainViewID, ViewID routePlanerViewID );

      /**
       * Goto a grouped view, called by pop by default.
       */      
      virtual void gotoView( ViewID v, bool isPop );

      /**
       * Goto a grouped view, called by pop by default.
       */
      virtual void gotoGroupView( ViewID v, GroupID g, bool isPop );

      /**
       * Error pop called on empty stack. 
       */
      virtual void errorEmptyPop();

      /**
       * MessageID for next push, is reset in goto[Group]View, that is
       * next pop/push, call.
       */
      void setNextMessagId( TInt aId ) {
         m_messageId = aId;
      }

      void setNextMessage( const TDesC8& aMessage ) {
         m_message = aMessage;
      }

   private:
      /**
       * The view changer.
       */
      CWayFinderAppUi* m_appUi;

      /**
       * The next message id for push.
       */
      TInt m_messageId;
      TBuf8<16> m_message;
};


#endif // S60_WF_VIEW_STACK_H

