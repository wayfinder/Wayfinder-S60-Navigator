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
 *    wait_many for sockets.
 */
#ifndef SELECTOR_H
#define SELECTOR_H

#include <arch.h>
#include "Sockets.h"

namespace isab {
class SelectorSet; // Hidden in Socket.cpp
class Selectable;  // Forward decl.

/**
 *   A Selector waits for something to happen on one
 *   or more Selectables. Currently only sockets are
 *   supported.<br />
 *   Currently a timeout is used to determine if the Selector
 *   should terminate.<br />
 *   Future enhancements could include the possibilty to wait
 *   for ModuleQueues too. The Selector could have a socketpair
 *   or pipe that could be handed out to the non-socket selectables
 *   when they are registered in addSelectable and when they are
 *   ready they something on the pipe/socket and make the Selector
 *   wake up. The ModuleQueue will also need a non-blocking dequeue
 *   method for this.
 */
class Selector {
public:

   /**
    *   Enum that describes if the selectable should be
    *   selected when ready for writing, reading or both.
    */
   enum selectfor_t {
      /// Use when selecting for writing.
      WRITING   = 1,
      /// Use when selecting for reading.
      READING   = 2,
      /// Use when selecting for both reading and writing.
      READWRITE = 3
   };
   
   /** 
    *   Constructor. Sets up the necessary lists.
    */
   Selector();

   /**
    *   Removes the internal lists.
    */
   ~Selector();

   /**
    *   Sets the terminated flag. The Selector will return
    *   from the select method after a while. <br />
    *   <em>Not</em> synchronized. Does it have to be?<br />
    *   <em>Not</em> realtime.
    */
   inline void terminate();
   
   /** 
    *  Wait for the previously inserted selectables to get ready.<br />
    *  Will check every <code>m_maxWaitMicros</code> for termination.
    *  Don't use the function as a real time timer.
    *  @param micros The max number of microseconds to wait.
    *                -1 means wait forever.
    *  @param ready  One of the ready Selectable is returned here.
    *  
    *  @return  1 if <code>ready</code> contains a Selectable.<br />
    *           0 if timeout.<br />
    *          -1 if errror. <br />
    *          -2 if terminated.
    */
   int select(int32 micros, Selectable*& ready);

   /**
    *  Adds a Selectable that will make select return when 
    *  the Selectable is ready for writing. <br />
    *  <em>Not</em> threadsafe.
    *  @param target  The Selectable.
    *  @param forWhat What to select for.
    *  @return -1 if error.
    */
   int addSelectable(Selectable* target, selectfor_t forWhat);

   /**
    *   Adds multiple selectables at once.
    *   <em>Not</em> threadsafe.
    *   @param num     The number of Selectables.
    *   @param target  Array of Selectables.
    *   @param forWhat What to select for. 
    *   @return 
    */
   int addSelectable(int num, Selectable* target[], selectfor_t forWhat);
   
   /**
    *   Removes a selectable from the set of Selectables.
    *   <em>Not</em> threadsafe.
    *   @param target  The selectable to remove.
    *   @param forWhat What to stop selecting for.
    *   @return 
    */
   int removeSelectable(Selectable* target, selectfor_t forWhat);

   /**
    *   Removes the array of selectables.
    *   <em>Not</em> threadsafe.
    *   @param num     The number of selectables in <code>target</code>
    *                  to remove.
    *   @param target  An array of selectables.
    *   @param forWhat What to stop selecting for.
    *   @return
    */
   int removeSelectable(int num, Selectable* target[], selectfor_t forWhat);

private:

   /**
    *   Looks in the fd_set and returns a Selectable if the
    *   fds match. The fdset is cleared for the found socket.
    *   @param selectorSet The selector set to look in.
    *   @param fdset       The fdset to
    *   @return A selectable if it is found in both sets or NULL.
    */
   Selectable* findAndClear(SelectorSet* selectorSet,
                            fd_set* fdset);
   
   /** Set of sockets for writing */
   SelectorSet* m_reading;
   /** Set of sockets for reading */
   SelectorSet* m_writing;
   /** True if something was selected the last time select was called */
   bool m_selected;
   /** True if the selector shouldn't do anything anymore */
   bool m_terminated;
   /** FD set for reading (and kept here for round-robin) */
   fd_set m_readfds;
   /** FD set for writing (and kept here for round-robin) */
   fd_set m_writefds;
   /** Maximum wait time before checking for termination */
   int32 m_maxWaitMicros;
};

// --------------- Implementation of inlined methods --------
inline void
Selector::terminate()
{
   m_terminated = true;
}

}
#endif




