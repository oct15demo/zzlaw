/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

//#if !defined(XERCESC_INCLUDE_GUARD_XMLREADERFACTORY_HPP)
//#define XERCESC_INCLUDE_GUARD_XMLREADERFACTORY_HPP
#define XML_FACTORY
#ifndef SAX2XML_READER_LOC
#include "SAX2XMLReaderLoc.hpp"
#endif
#include <xercesc/sax/SAXException.hpp>

//XERCES_CPP_NAMESPACE_BEGIN

//class MemoryManager;
//class XMLGrammarPool;

/**
  * Creates a SAX2 parser (SAX2XMLReader).
  *
  * <p>Note: The parser object returned by XMLReaderFactory is owned by the
  * calling users, and it's the responsibility of the users to delete that
  * parser object, once they no longer need it.</p>
  *
  * @see SAX2XMLReader#SAX2XMLReader
  */
//class SAX2_EXPORT XMLReaderFactory


class XMLReaderFactoryLoc
{
protected:                // really should be private, but that causes compiler warnings.
	XMLReaderFactoryLoc() ;
	~XMLReaderFactoryLoc() ;

public:
	static xercesc_3_2::SAX2XMLReaderLoc * createXMLReaderLoc(  xercesc_3_2::MemoryManager* const  manager = xercesc_3_2::XMLPlatformUtils::fgMemoryManager
                                           , xercesc_3_2::XMLGrammarPool* const gramPool = 0
                                          ) ;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLReaderFactoryLoc(const XMLReaderFactoryLoc&);
    XMLReaderFactoryLoc& operator=(const XMLReaderFactoryLoc&);
};

//XERCES_CPP_NAMESPACE_END

//#endif
