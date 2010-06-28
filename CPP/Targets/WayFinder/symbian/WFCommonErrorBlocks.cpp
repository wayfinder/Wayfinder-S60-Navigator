/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "WFCommonErrorBlocks.h"
#include "GuiProt/GuiProtEnums.h"
#include "GuiProt/GuiProtMess.h"
#include "BackActionEnum.h"
#include "WayfinderSymbianUiBase.h"


GuiProtMessageErrorHandler::GuiProtMessageErrorHandler( WayfinderSymbianUiBase* ui )
      : GuiProtMessageReceiver(), m_ui( ui )
{
}


GuiProtMessageErrorHandler::~GuiProtMessageErrorHandler() {
   // No deletes here yet.
}

bool
GuiProtMessageErrorHandler::GuiProtReceiveMessage( 
   isab::GuiProtMess *mess )
{
   bool ret = false;
   // Some error handling
   const char* errorURL = NULL;
   if ( mess->getMessageType() == isab::GuiProtEnums::REQUEST_FAILED ) {
      errorURL = static_cast<isab::RequestFailedMess*>( mess )->
         getErrorURL();
   } else if ( mess->getMessageType() == 
               isab::GuiProtEnums::MESSAGETYPE_ERROR )
   {
      errorURL = static_cast<isab::ErrorMess*>( mess )->getErrorURL();
   }
   if ( errorURL != NULL && errorURL[ 0 ] != '\0' ) {
      ret = GotErrorURL( errorURL );
   } else if ( !ret ) {
      // Perhaps show error message in popup?
      // m_ui->ShowErrorDialogL( );
      // Or call GotErrorStr( ) method in this class so it can be
      // overloaded
   }
   
   return ret;
}

bool
GuiProtMessageErrorHandler::GotErrorURL( const char* errorURL ) {
   // Show URL in Content Window.
   BackActionEnum back = BackIsHistoryThenView;
   // Check if in startup then set BackIsExit
   if ( m_ui->inStartup() ) {
      back = BackIsExit;
   }
   m_ui->GotoServiceViewUrl( errorURL, back );
   return true;
}
