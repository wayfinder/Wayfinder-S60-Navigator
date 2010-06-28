/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
*  Description:
*     Specifies the colsole output functions.
*/

#ifndef __MESSAGEFACTORY_H__
#define __MESSAGEFACTORY_H__


/*! 
  @class MMessageFactory
  
  @discussion This class specifies the colsole output functions.
  */
class MMessageFactory
{

public:

   /*!
    @function PrintNotify

    @discussion Displays the descriptor as text (with attributes) on the console
    @param the text to be displayed
    @param attributes e.g. CEikGlobalTextEditor::EItalic
    */
   virtual void FromBytes() = 0;

   /*!
    @function PrintNotify

    @discussion Displays the descriptor as text (with attributes) on the console
    @param the text to be displayed
    @param attributes e.g. CEikGlobalTextEditor::EItalic
    */
   virtual void ToBytes() = 0;

   /*!
    @function PrintNotify

    @discussion Displays the integer as a decimal number on the console
    @param aNumber the number to be displayed
    */
   virtual void SetID( TInt aId ) = 0;

   /*!
    @function ErrorNotify

    @discussion Displays an error message
    @param aErrMessage error message text
    @param aErrCode error code number
    */
   virtual TInt GetID() = 0;

   /*!
    @function SetStatus

    @discussion Displays the text in the 'status' window
    @param aStatus new status description text
    */
   virtual void SetType( TInt aType ) = 0;

   /*!
    @function ErrorNotify

    @discussion Displays an error message
    @param aErrMessage error message text
    @param aErrCode error code number
    */
   virtual TInt GetType() = 0;

};

#endif
