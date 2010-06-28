/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef WAYFINDER_DIALOGS_H
#define WAYFINDER_DIALOGS_H
#include <e32def.h>
#include <e32std.h>
#include <coemain.h>

namespace WFDialog{
   /**\defgroup g1 ShowConfirmationL
    * Displays an information note containing an animated checkbox.
    * The dialog has no buttons, but closes automatically.
    * @{
    */
   /** @param aText the text to be displayed.
    */
   void ShowConfirmationL( TDesC const  &aText );

   /** @param aResourceId the id of a resource containing a string.
    * @param aCoeEnv pointer to the application instance of CCoeEnv. 
    *                Used for reading the resource. If no CCoeEnv pointer 
    *                is available, use <code>ShowConfirmationL(TInt)</code> 
    *                instead.
    */
   void ShowConfirmationL( TInt aResourceId, class CCoeEnv* aCoeEnv );

   /** This method calls <code>CCoeEnv::Static()</code> for resource access.
    * If a <code>CCoeENv</code> pointer is available, prefer 
    * <code>ShowConfirmationL(TInt, CCoeEnv*)</code>.
    * @param aResourceId the resource id of the text to be displayed.
    */
   inline void ShowConfirmationL( TInt aResourceId )
   {
      ShowConfirmationL(aResourceId, CCoeEnv::Static());
   }
   /**@}*/

   /** \defgroup g2 ShowErrorDialogL
    * Display an error dialog containing an red exclamation mark.
    * The dialog has an OK button.
    * A tone is played when the dialog opens.
    *@{
    */
   /** @param aText the string to display.
    */
   void ShowErrorDialogL( const TDesC &aText );

   /** @param aResourceId the resource identifier of the test to be displayed.
    * @param aCoeEnv pointer to the application instance of CCoeEnv. 
    *                Used for reading the resource. If no CCoeEnv pointer 
    *                is available, use <code>ShowErrorDialogL(TInt)</code> 
    *                instead.
    */
   void ShowErrorDialogL( TInt aResourceId, class CCoeEnv* coeEnv );

   /** This method calls <code>CCoeEnv::Static()</code> for resource access.
    * If a <code>CCoeENv</code> pointer is available, prefer 
    * <code>ShowErrorDialogL(TInt, CCoeEnv*)</code>.
    * @param aResourceId the resource identifier of the test to be displayed.
    */
   inline void ShowErrorDialogL( TInt aResourceId ) 
   {
      ShowErrorDialogL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   /**\defgroup g3 ShowWarningDialogL
    * Displays a warning dialog containing an exclamation mark.
    * Has an OK button. Plays a tone when opening.
    * @{*/
   /** @param aText the text to display.
    */
   void ShowWarningDialogL( const TDesC &aText );
   /** @param aResourceId the resource identifier of the test to be displayed.
    * @param aCoeEnv pointer to the application instance of CCoeEnv. 
    *                Used for reading the resource. If no CCoeEnv pointer 
    *                is available, use <code>ShowWarningDialogL(TInt)</code> 
    *                instead.
    */
   void ShowWarningDialogL( TInt aResourceId, class CCoeEnv* aCoeEnv);
   /** This method calls <code>CCoeEnv::Static()</code> for resource access.
    * If a <code>CCoeENv</code> pointer is available, prefer 
    * <code>ShowWarningDialogL(TInt, CCoeEnv*)</code>.
    * @param aResourceId the resource identifier of the test to be displayed.
    */
   inline void ShowWarningDialogL( TInt aResourceId)
   {
      ShowWarningDialogL(aResourceId, CCoeEnv::Static());
   }
   /**\defgroup g3 ShowInfoDialogL
    * Displays a info dialog.
    * Has an OK button. Plays no tone when opening.
    * @{*/
   /** @param aText the text to display.
    */
   void ShowInfoDialogL( const TDesC &aText, TBool aAutoClose );
   /** @param aResourceId the resource identifier of the text to be displayed.
    * @param aCoeEnv pointer to the application instance of CCoeEnv. 
    *                Used for reading the resource. If no CCoeEnv pointer 
    *                is available, use <code>ShowInfoDialogL(TInt)</code> 
    *                instead.
    */
   void ShowInfoDialogL( TInt aResourceId, class CCoeEnv* aCoeEnv, TBool aAutoClose);

  //TInt ShowRoamingDialogL(const TDesC& aText, MCoeControlObserver* aObserver);
  //TInt ShowRoamingDialogL(TInt aResourceId, class CCoeEnv* aCoeEnv, MCoeControlObserver* aObserver);
   /**@}*/

   /**\defgroup g4 ShowDebugDialogL
    * When compiled in debug mode, this function displays a query
    * dialog containg the text in the parameter. It makes no
    * difference whether the user chooses Yes or No.
    * Note! MAX 128 chars! 
    * @{
    */
   /** @param str a C-style string to be displayed. 
    *             It may be max 128 characters long.
    */
   void ShowDebugDialogL( const char *str );

   /** @param aText the text to display.
    */
   void ShowDebugDialogL( const TDesC &aText);
   /**@}*/

   /**\defgroup g5 ShowScrollingWarningDialogL
    * Displays a scrolling dialog with a header saying "Warning" and
    * an OK button.
    * @{
    */
   /** @param aText the text to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowScrollingWarningDialogL(const TDesC &aText, 
                                    class CCoeEnv *aCoeEnv);
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowScrollingWarningDialogL(const TDesC, CCoeEnv*)</code>.
    * @param aText the text to display.
    */
   inline void ShowScrollingWarningDialogL( const TDesC& aText)
   {
      ShowScrollingWarningDialogL(aText, CCoeEnv::Static());
   }
   /** @param aResourceId the resource id for the string to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowScrollingWarningDialogL( TInt aResourceId, class CCoeEnv* coeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowScrollingWarningDialogL(const TDesC, CCoeEnv*)</code>.
    * @param aResourceId the resource id for the string to display.
    */
   inline void ShowScrollingWarningDialogL( TInt aResourceId ) 
   {
      ShowScrollingWarningDialogL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   /**\defgroup g6 ShowScrollingErrorDialogL
    * Displays a scrolling dialog with a header saying "Error" and an 
    * OK button.
    * @{
    */
   /** @param aText the text to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowScrollingErrorDialogL( const TDesC &aText, class CCoeEnv*aCoeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowScrollingErrorDialogL(const TDesC, CCoeEnv*)</code>.
    * @param aText the text to display.
    */
   inline void ShowScrollingErrorDialogL( const TDesC &aText )
   {
      ShowScrollingErrorDialogL( aText, CCoeEnv::Static() );
   }
   /** @param aResourceId the resource id for the string to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowScrollingErrorDialogL( TInt aResourceId, class CCoeEnv* coeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowScrollingErrorDialogL(const TDesC, CCoeEnv*)</code>.
    * @param aResourceId the resource id for the string to display.
    */
   inline void ShowScrollingErrorDialogL( TInt aResourceId ) 
   {
      ShowScrollingErrorDialogL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   //*****ShowScrollingInfoDialogL*****
   void ShowScrollingInfoDialogL( const TDesC &aText, class CCoeEnv* coeEnv );

   void ShowScrollingInfoDialogL( TInt aResourceId, class CCoeEnv* coeEnv );

   inline void ShowScrollingInfoDialogL( const TDesC &aText )
   {
      ShowScrollingInfoDialogL( aText, CCoeEnv::Static() );
   }

   inline void ShowScrollingInfoDialogL( TInt aResourceId ) 
   {
      ShowScrollingInfoDialogL(aResourceId, CCoeEnv::Static()); 
   }

   /**\defgroup g7 ShowScrollingDialogL
    * Displays a scrolling dialog with a user defined header, 
    * and an OK button.
    *@{
    */
   /** @param aHeader the header string.
    * @param aText   the text to display.
    * @param aTone   play tone when opening or not.
    * @param aBitmap unused
    * @param aMask   unused
    */
   void ShowScrollingDialogL( const TDesC &aHeader, const TDesC &aText,
                              TBool aTone, TInt32 aBitmap = -1, 
                              TInt32 aMask = -1 );
   /** @param aCoeEnv a pointer to a CCoeEnv object used to read resources.
    * @param aHeader a resource id identifying the header string.
    * @param aText   a resource id identifying the message string.
    * @param aTone   play tone when opening or not.
    * @param aBitmap unused
    * @param aMask   unused
    */
   void ShowScrollingDialogL(class CCoeEnv* aCoeEnv, TInt aHeader, TInt aText, 
                             TBool aTone, TInt32 aBitmap = -1, 
                             TInt32 aMask = -1);
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowScrollingDialogL(CCoeEnv*, TInt, TInt, TBool, 
    *                            TInt32, TInt32)</code>.
    * @param aHeader a resource id identifying the header string.
    * @param aText   a resource id identifying the message string.
    * @param aTone   play tone when opening or not.
    * @param aBitmap unused
    * @param aMask   unused
    */
   inline void ShowScrollingDialogL( TInt aHeader, TInt aText, TBool aTone, 
                                     TInt32 aBitmap = -1, TInt32 aMask = -1)
   {
      ShowScrollingDialogL(CCoeEnv::Static(), aHeader, aText, aTone, aBitmap, 
                           aMask);
   }

   TBool ShowScrollingQueryL(class CCoeEnv* coeEnv, 
                             TInt aHeader, TInt aText, TBool aTone,
                             TInt32 aBitmap = -1, TInt32 aMask = -1);

   TBool ShowScrollingQueryL( const TDesC &aHeader, const TDesC &aText,
                              TBool aTone, TInt32 aBitmap = -1, TInt32 aMask = -1);
   TBool ShowScrollingQueryL(TInt aResourceId,
                            const TDesC &aHeader, const TDesC &aText,
                            TBool aTone);

   inline TBool ShowScrollingQueryL( TInt aHeader, TInt aText, TBool aTone, 
                                     TInt32 aBitmap = -1, TInt32 aMask = -1)
   {
      return ShowScrollingQueryL(CCoeEnv::Static(),
            aHeader, aText, aTone, aBitmap, aMask);
   }
   /**@}*/

   /**\defgroup ginfo ShowInformationL
    * Display an information note containing an animated 'i'.
    * @{
    */
   /** @param aText the test to display*/
   void ShowInformationL( const TDesC &aText );

   /** @param aResourceId a resource id identifying the string to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowInformationL( TInt aResourceId, class CCoeEnv* coeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowInformationL(TInt, CCoeEnv*)</code>.
    * @param aResourceId a resource id identifying the string to display.
    */
   inline void ShowInformationL( TInt aResourceId ) 
   {
      ShowInformationL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   /** \defgroup gwarn ShowWarningL
    * Display a warning note containing an image of an exclamation mark.
    * @{
    */
   /** @param aText the test to display*/   
   void ShowWarningL( const TDesC &aText );
   /** @param aResourceId a resource id identifying the string to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowWarningL( TInt aResourceId, class CCoeEnv* coeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowWarningL(TInt, CCoeEnv*)</code>.
    * @param aResourceId a resource id identifying the string to display.
    */
   inline void ShowWarningL( TInt aResourceId ) 
   {
      ShowWarningL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   /** \defgroup gwarn ShowErrorL
    * Display a error note containing an image of an exclamation mark.
    * @{
    */
   /** @param aText the test to display*/   
   void ShowErrorL( const TDesC &aText );
   /** @param aResourceId a resource id identifying the string to display.
    * @param aCoeEnv a pointer to a CCoeEnv instance used to read resources.
    */
   void ShowErrorL( TInt aResourceId, class CCoeEnv* coeEnv );
   /** If a CCoeEnv pointer is available, prefer 
    * <code>ShowErrorL(TInt, CCoeEnv*)</code>.
    * @param aResourceId a resource id identifying the string to display.
    */
   inline void ShowErrorL( TInt aResourceId ) 
   {
      ShowErrorL(aResourceId, CCoeEnv::Static()); 
   } 
   /**@}*/

   /**\defgroup gquery ShowQueryL
    * Display a yes no dialog. Contains an image of a question mark.
    * @{
    */
   /** @param aText the test to display.
    * @return ETrue if the user selected Yes, EFalse if the user selected No.
    */
   TBool ShowQueryL( const TDesC &aText );

   /** @param aResourceId a resource id identifying the string to display.
    * @param aCoeEnv a pointer to an CCoeEnv instance used for reading 
    *                resources.
    * @return ETrue if the user selected Yes, EFalse if the user selected No.
    */
   TBool ShowQueryL( TInt aResourceId, class CCoeEnv* coeenv );
}
#endif
