/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef BROWSER_COMMAND_TYPES_H
#define BROWSER_COMMAND_TYPES_H

#include "ControlStateObserver.h"
#include <map>

/**
 * Class describing a general control in the WABrowser.
 * The class keeps track of a command that should be sent down
 * to the WABrowser when triggering an action of this control. It
 * also keeps the resource id for the text to be displayed on
 * the middle softkey when the control has focus.
 */
class BrowserCommandType  {
public:

   /**
    * Constructor that takes two parameters.
    * @parameter aCommand, sets iCommand defined in TWAXHTMLControlCommand.
    * @parameter aTextResourceId, sets iTextResourceId.
    */
   BrowserCommandType(int aCommand,
                      int aTextResourceId) :
      iCommand(aCommand),
      iTextResourceId(aTextResourceId) {}

   /**
    * Base function, in general ordinary controls will
    * only have one type, not checked and unchecked like
    * the checkbox.
    * @return TBool, EFalse.
    */   
   virtual TBool DifferentTypeWhenFocused() {
      return EFalse;
   }
   
   /**
    * Base function, simply returns the resource id
    * for the text that should be displayed on the
    * middle softkey.
    * @return int, the resource id for the text.
    */
   virtual int GetTextResourceId() {
      return iTextResourceId;
   }

   /**
    * Base function, simply returns the command for
    * this control.
    * @return int, the command for this control.
    */
   virtual int GetCommand() {
      return iCommand;
   }

   /**
    * Sets the iControlState when the state has changed for the control.
    * @param aControlState, the new state of this control defined in 
    *                       TWAXHTMLControlState.
    */
   virtual void ControlStateChanged(int aControlState) {
      iControlState = aControlState;
   }

   /**
    * Checks if the control is in its default state.
    * For generic controls that only can have one state
    * this function returns true.
    * @param, true for generic controls.
    */
   virtual bool IsInDefaultState() {
      return true;
   }

   /**
    * This struct links together a WABrowser component control state
    * such as ECheckboxChecked with a WABrowser control type, same type
    * as the member first in control_cmd_array_pair_t struct.
    */
   typedef struct {
      /// WABrowser control type.
      int command;
      /// WABrowser component control state
      int state;
      /// Text resource id
      int textId;
   } state_cmd_text_array_pair_t;

   /// WABrowser command type.
   int iCommand;
   /// Text resource id for the command. 
   int iTextResourceId;
   /// Current state of this control.
   int iControlState;
};

/**
 * Class describing a checkbox. The difference between this class
 * and the base class is simply that a checkbox can have two states,
 * checked and unchecked. However, the control type is different 
 * when the state is changing:
 * ECheckBoxControlMarked will have the state ECheckboxChecked and
 * ECheckBoxControlUnmarked will have the state ECheckboxUnchecked.
 * To be able to separate this in CServiceView::StateChanged this 
 * class is needed.
 */
class CheckBoxCommandType : public BrowserCommandType {
public:
   /**
    * Constructor, just calls base constructor with its
    * parameters.
    * @param aCommand, the command related to this instance.
    * @param aTextResourceId, the text to be set on the middle softkey.
    */
   CheckBoxCommandType(int aCommand, int aTextResourceId) :
      BrowserCommandType(aCommand, aTextResourceId) {}

   /**
    * Overloaded, returns true since this class, as described
    * in the class description will have to command types
    * for its two different states.
    * @return, ETrue.
    */
   virtual TBool DifferentTypeWhenFocused() {
      return ETrue;
   }
};

/**
 * Class describing the three kinds of inputfield classes that
 * exists in the WABrowser: 
 * - EInputTextEditorControl
 * - EinputPasswordEditorControl
 * - EinputNumberEditorControl
 * These classes can have two states EInputboxNormal and
 * EInputboxEditable and they will have two different commands
 * realated to its state:
 * EWXHTMLCmdEdit when having the state EInputboxNormal
 * EWXHTMLCmdDone when having the state EInputboxEditable
 * This class will therefore overload two functions, GetTextResourceId
 * and GetCommand. These functions will base its return value
 * on which state the control has.
 */
class InputFieldCommandType : public BrowserCommandType {
public:

   /**
    * Constructor taking an array with commands, states and textids 
    * related to each other. The default command and textid will be
    * set by calling base constructor with the object located first
    * in the array.
    * @param aStateCmdTextBindings, array containing state_cmd_text_array_pair_t
    *                               defining what command, state and textid that
    *                               belongs together.
    * @param aCount, simply the number of elements in the array.
    */
   InputFieldCommandType(const state_cmd_text_array_pair_t* aStateCmdTextBindings, int aCount) : 
      BrowserCommandType(aStateCmdTextBindings[0].command, aStateCmdTextBindings[0].textId) 
   {
      iControlState = aStateCmdTextBindings[0].state;
      iDefaultState = iControlState;
      for ( int i = aCount - 1; i >= 0; --i ) {
         // Add command and textId from aStateCmdTextBindings into the iCmdTextMap
         iCmdTextMap.insert(std::make_pair(aStateCmdTextBindings[i].command,
                                           aStateCmdTextBindings[i].textId));

         // Add state and command from aStateCmdTextBindings into the iStateCmdMap
         iStateCmdMap.insert(std::make_pair(aStateCmdTextBindings[i].state,
                                            aStateCmdTextBindings[i].command));
      }
   }

   /**
    * Overloaded function, returns the correct text resource id
    * based on the current state. What the function does is that
    * it will search in the iStateCmdMap with the current state of
    * the object. It will then use the found post and use its second
    * (that is the command for this object) to search in iCmdTextMap
    * and then return the found text resource id.
    * @return int, the correct text resource id based on the current
    *               state of this control.
    */ 
   virtual int GetTextResourceId() {
      // Use the current state to search in iStateCmdMap to
      // get the command of this control.
      std::map<int, int>::const_iterator it = 
         iStateCmdMap.find(iControlState);
      if (it == iStateCmdMap.end()) {
         // Didn't find it, can't proceed.
         return -1;
      }
      // Use the command found above to locate the correct
      // text resource and return it to the caller.
      std::map<int, int>::const_iterator it2 = 
         iCmdTextMap.find(it->second);
      if (it2 == iCmdTextMap.end()) {
         return -1;
      }
      return it2->second;
   }
 
   /**
    * Overloaded function that returns the correct command
    * based on the current state of this control.
    * @return int, the command based on the current state.
    */
   virtual int GetCommand() {
      std::map<int, int>::const_iterator it = 
         iStateCmdMap.find(iControlState);
      if (it == iStateCmdMap.end()) {
         // Didn't find it, can't proceed.
         return -1;
      }
      return it->second;
   }

   /**
    * Checks if the control is in its default state.
    * @param, true if the default state (iDefaultState)
    *         of this control == the current state 
    *         (iControlState) of this control.
    */
   virtual bool IsInDefaultState() {
      return iDefaultState == iControlState;
   }

   /// Map containing commands and resource ids for texts
   /// related to each other. For example the command EWXHTMLCmdEdit
   /// that will set the input editor into edit mode should have the
   /// text "Edit".
   std::map<int, int> iCmdTextMap;
   /// Map containing states and commands related to each other.
   /// For example the state EInputboxNormal should be related to
   /// the command EWXHTMLCmdEdit.
   std::map<int, int> iStateCmdMap;

   int iDefaultState;
};

#endif
