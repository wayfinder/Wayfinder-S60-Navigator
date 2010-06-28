/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef AUDIOCLIPTABLE_H
#define AUDIOCLIPTABLE_H

//#include "config.h"

/**
 *    Class containing translation between audio clip enums and
 *    filenames.
 */
class AudioClipTable {
public:
   /**
    *    Returns the filename corresponding to the
    *    sent-in AudioClipEnum.
    */
   virtual const char* getFileName(int audioClipEnum) = 0;

   /// Returns true if the table seems ok.
   virtual bool sanityCheck() = 0;
};




/**
 *    Class containing translation between audio clip enums and
 *    filenames.
 */
class AudioClipTableTableDriven : public AudioClipTable {
public:
   /**
    *    Returns the filename corresponding to the
    *    sent-in AudioClipEnum.
    */
   virtual const char* getFileName(int audioClipEnum);

   /// Returns true if the table seems ok.
   virtual bool sanityCheck();
   
#ifndef _MSC_VER
protected:
#else
   // Public due to broken MS VC++ compiler.
public:
#endif
   typedef struct int_to_clipname {
#ifdef __WINS__
      // VC++ cannot initialize structs the normal way, it seems
      int_to_clipname(int a, const char* const name ) :
            m_audioEnum(a), m_fileName(name) {}
#endif
      /// Value from AudioClipsEnum
      int m_audioEnum;
      /// Filename of corresponding file.
      const char* const m_fileName;
   } int_to_clipname_t;

protected:
   /// Returns a sorted table containing the clipnames.
   virtual const int_to_clipname_t * getClipTable1() = 0;
   virtual const int_to_clipname_t * getClipTable2() = 0;
   virtual int getNumClips1() = 0;
   virtual int getNumClips2() = 0;

   /// Class for binary search in table
   class AudioClipTableSearchComp {
   public:
      inline int operator()( const int_to_clipname_t& a,
                             int b) const;
      inline int operator()(const int_to_clipname_t& a,
                            const int_to_clipname_t& b ) const;
   };
   
};




/**
 * Old static version
 */
class OldAudioClipTable : public AudioClipTableTableDriven {
public:

protected:
   /// Actual static table. 
   static const int_to_clipname_t m_staticClipTable1[];
   static const int_to_clipname_t m_staticClipTable2[];

   /// Returns a sorted table containing the clipnames.
   const int_to_clipname_t * getClipTable1();
   const int_to_clipname_t * getClipTable2();
   int getNumClips1();
   int getNumClips2();
};




class ScriptAudioClipTable : public AudioClipTable {
public:
   /* Constructor. Does not assume ownership of the passed filenames */
   ScriptAudioClipTable(int num_filenames, const char * const * filenames);
   virtual ~ScriptAudioClipTable();

   /**
    *    Returns the filename corresponding to the
    *    sent-in AudioClipEnum.
    */
   virtual const char* getFileName(int audioClipEnum);

   /// Returns true if the table seems ok.
   virtual bool sanityCheck();

protected:
   int m_numFiles;
   char * m_buffer;
   char ** m_fileNames;
};





inline int
AudioClipTableTableDriven::AudioClipTableSearchComp::operator()(
   const AudioClipTableTableDriven::int_to_clipname_t& a, int b) const
{   
   return a.m_audioEnum < b;
}

inline int
AudioClipTableTableDriven::AudioClipTableSearchComp::operator()
   (const AudioClipTableTableDriven::int_to_clipname_t& a,
    const AudioClipTableTableDriven::int_to_clipname_t& b ) const
{
      return a.m_audioEnum < b.m_audioEnum;
}


#endif
