/*
  Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
  * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef ML_CONVIGURATOR_H
#define ML_CONVIGURATOR_H

#include "config.h"

/**
 *    Configuration interface for MapLib functionality.
 *    All strings are in utf-8.
 */
class MLConfigurator {
public:
   /**
    *   Set the language using the specified iso639 string.   
    *   The string can consist of two or three letters and must be
    *   NULL terminated. 
    *   @param isoStr The string, containing two or three letters.
    *   @return   The language as iso639 that was set to MapLib.
    */
   virtual const char* setLanguageAsISO639( const char* isoStr ) = 0;

   /**
    *    Set the DPI correction factor. The factor 1 corresponds
    *    a value suitable for phone model with 128 DPI screen.
    *    For newer devices with displays with higher
    *    DPI, the factor should be correspond with how many times
    *    greater the DPI is compared to 128 DPI.
    */
   virtual void setDPICorrectionFactor( uint32 factor ) = 0;

   // -- Cache handling
   
   /**
    *    Creates the read/write cache with the supplied max size.
    *    @param path    The path to the cache in utf-8.
    *    @param maxSize The maximum size of the cache.
    */
   virtual int addDiskCache( const char* path,
                             uint32 maxSize ) = 0;

   /**
    *    Sets the max size of the disk cache if it exists.
    *    @param nbrBytes The maximum size of the R/W cache in bytes.
    */
   virtual int setDiskCacheSize( uint32 nbrBytes ) = 0;

   /**
    *    Clears the R/W disk cache.
    */
   virtual int clearDiskCache() = 0;

};

#endif
