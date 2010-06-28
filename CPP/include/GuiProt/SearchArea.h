/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GUI_PROT_SEARCH_AREA
#define GUI_PROT_SEARCH_AREA

namespace isab{

   class SearchArea : public Serializable {
   public:
      SearchArea(const char* name, const char* id);
      SearchArea(Buffer* buf);
      SearchArea(const SearchArea& item);
      virtual ~SearchArea();
      virtual void serialize(Buffer* buf) const;
      const char* getName() const;
      const char* getID() const;
      bool operator==(const SearchArea& rhs) const;
   private:
      char* m_name;
      char* m_id;
      const SearchArea& operator=(const SearchArea& rhs);
   };

   char* idStringFromIdMapCountry(uint32 id, uint32 mapid, uint32 country);
   bool idMapCountryFromIdString(uint32& id, uint32& mapid, uint32& country,
                                 const char* idString);
}

#endif /* GUI_PROT_SEARCH_AREA */
