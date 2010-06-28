/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
*/

#ifndef ISABTIME_H
#define ISABTIME_H
#include "arch.h"
#include "TimeUtils.h"

namespace isab{
  /** The base time type. A very small class, best used on automatic
   * storage. Is comparable and supports some arithmetic
   * operations. 
   *
   * All Time objects measure milliseconds since the first time the
   * static start operation is called. Start is called any time a
   * time object is created.
   */
  class isabTime{
    /** Holds the time of this object. Cannot be negative. */
    uint32 m_time;
  public:
    /** Gets the value of the m_time property.*/
    inline uint32 millis() const;
    /** Default constuctor. Sets this isabTime object's time to now. */
    inline isabTime();
    /** Copy constructor. */
    inline isabTime(const isabTime& t);
    /** Adds millisecond to this isabTime object. 
     * @param millis the number of milliseconds to add. May be
     *               negative.
     * @return a reference to this object.
     */
    inline isabTime& add(uint32 millis);
    /** Adds another isabTime object to this object. After the call this
     * object will represent the sum of the two isabTime objects. 
     * @param t a reference to a isabTime object who's value will be added
     *          to this object. 
     * @return a reference to this object. 
     */
    inline isabTime& add(const isabTime& t);
    /** Takes the difference between two isabTime objects, this and the
     * parameter. 
     * @param t a reference to a isabTime object.
     * @return the difference in milliseconds between this object and
     *         t. If this object is larger than t, the result will be
     *         positive, if this and t are equal the result will be
     *         negative. 
     */
     inline int32 diff(const isabTime& t) const;
    /** Sets this objects value to represent now. 
     * @return a reference to this object. 
     */
    inline isabTime& now();
    /** Subtracts milliseconds from this isabTime object.
     * @param millis the number of milliseconds to subtract. 
     *               mat be negative.
     * @return a refernce to this object.
     */
    inline isabTime& sub(uint32 millis);
    /** Subtract the value of a isabTime object from this object. 
     * @param t a Time object who's value will be subtracted from this
     *          objects value.
     * @return a reference to this object.
     */
    inline isabTime& sub(const isabTime& t);
     /** Sets the time.
      * @param t a isab Time object who's value will be copied to this 
      *          object.
      * @return a reference to this isabTime object.
      */
     inline isabTime& setTime(const isabTime& t);
    /** A standard operator<.
     * @param t the right hand isabTime object.
     * @return true if this object is earlier than t.
     */
    inline bool operator<(const isabTime& t) const;
    /** A standard operator>.
     * @param t the right hand isabTime object.
     * @return true if this object is later than t.
     */
    inline bool operator>(const isabTime& t) const;
    /** A standard operator<=.
     * @param t the right hand isabTime object.
     * @return true if this object is earlier than or equal to t. 
     */
    inline bool operator<=(const isabTime& t) const;
    /** A standard operator>=.
     * @param t the right hand isabTime object.
     * @return true if this object is later than or equal to t.
     */
    inline bool operator>=(const isabTime& t) const;
    /** A standard operator==.
     * @param t the right hand isabTime object.
     * @return true if this object is equal to t.
     */
    inline bool operator==(const isabTime& t) const;
    /** A standard operator!=.
     * @param t the right hand isabTime object.
     * @return true if this object is not equal to t.
     */
    inline bool operator!=(const isabTime& t) const;
    /** Gives the reference time all isabTime objects are compared to.
     * @return the time the program started, more or less.
     */
    inline static uint32 start();
    
  };
  //=============================================================
  //======= inline methods of isabTime. =============================
  inline uint32 isabTime::millis() const
  {
    return m_time;
  }
  inline isabTime::isabTime()
  {
    now();
  }
  inline isabTime::isabTime(const isabTime& t)
  {
    m_time = t.m_time;
  }
  inline isabTime& isabTime::add(uint32 millis)
  {
    m_time = m_time + millis;
    if (m_time >= WRAP_AROUND_MILLIS) {
       m_time -= WRAP_AROUND_MILLIS;
    }
    return *this;
  }
  inline isabTime& isabTime::add(const isabTime& t)
  {
     return add(t.m_time);  
  }
  inline int32 isabTime::diff(const isabTime& t) const
  {
     int32 diff = m_time - t.m_time;
     if (diff >= int(WRAP_AROUND_MILLIS/2)) {
        diff -= WRAP_AROUND_MILLIS;
     } else if (diff < -int(WRAP_AROUND_MILLIS/2)) {
        diff += WRAP_AROUND_MILLIS;
     }
     return diff;
  }
  inline isabTime& isabTime::now()
  {
    m_time = TimeUtils::millis() - start();
    return *this;
  }
  inline isabTime& isabTime::sub(uint32 millis)
  {
    m_time -= millis;
    if (int(m_time) < 0) {
       m_time += WRAP_AROUND_MILLIS;
    }
    return *this;
  }
  inline isabTime& isabTime::sub(const isabTime& t)
  {
     return sub(t.m_time);
  }
   inline isabTime& isabTime::setTime(const isabTime& t)
   {
      m_time = t.m_time;
      return *this;
   }
  inline bool isabTime::operator<(const isabTime& t) const
  {
    return diff(t) < 0;
  }
  inline bool isabTime::operator>(const isabTime& t) const
  {
     return diff(t) > 0;
  }
  inline bool isabTime::operator<=(const isabTime& t) const
  {
    return diff(t) <= 0;
  }
  inline bool isabTime::operator>=(const isabTime& t) const
  {
    return diff(t) >= 0;
  }
  inline bool isabTime::operator==(const isabTime& t) const
  {
    return m_time == t.m_time;
  }
  inline bool isabTime::operator!=(const isabTime& t) const
  {
    return m_time != t.m_time;
  }
  inline uint32 isabTime::start()
  {
// STATIC   static int32 start = TimeUtils::millis();
/*     static int32 start = TimeUtils::millis(); */
/*     return start; */
      return 0;
  }
}
#endif
