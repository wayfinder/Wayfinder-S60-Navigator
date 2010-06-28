/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "TestFramework.h"

void TestReporter::addTest()
{
   m_numTests++;
}

void TestReporter::testFailed(const char* msg, const char* file, int line)
{
   FailedTestResult f = { msg, file, line }; 
	m_failedTests.push_back(f);
}

MC2SimpleString TestReporter::getTestSummary( const char* directory ) 
{
   MC2SimpleString ret("* Test Summary:\n---------------\n");

   char numPassed[32];
   char numTests[32];
   
   unsigned int size = m_numTests - m_failedTests.size();
   
   sprintf( numPassed, "%d", size );
   sprintf( numTests, "%d", m_numTests );

   ret.append( MC2SimpleString( numPassed ) );
   ret.append( " / ");
   ret.append( MC2SimpleString( numTests ) );

   ret.append( " tests passed.\n" );

   if( m_failedTests.size() > 0 ) {
      ret.append("* Failed tests\n");
      ret.append("---------------\n");
   }
   
   for( std::vector<FailedTestResult>::iterator fti = m_failedTests.begin();
        fti != m_failedTests.end(); fti++ ) {

      MC2SimpleString res(directory);
      res.append( fti->file );
      res.append( MC2SimpleString( ":" ) );

      char line[32];
      sprintf( line, "%d", fti->line );
      
      res.append( line );
      res.append( ":   " );
      res.append( fti->msg );
      res.append( "\n" );
      
      ret.append( res );
   }

   ret.append("\n");
   
   return ret;
}

void TestReporter::printTestSummary( ostream& os,
                                     const char* directory )
{
//       os << getTestSummary(directory) << std::endl;
}


TestReporter::TestReporter() :
   m_numTests( 0 )
{

}

