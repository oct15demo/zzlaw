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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <string.h>
#include <unordered_map>
#include <vector>

#define COUNT_HANDLER

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/internal/ReaderMgr.hpp>
#ifndef SAX2XML_READER_LOC
#include "SAX2XMLReaderLoc.hpp"
#endif
//typedef basic_string<char, char_traits<char>, allocator<char> > string;

XERCES_CPP_NAMESPACE_USE

class SAX2CountHandlers : public DefaultHandler
{
public:

	char* first_citation; //
	std::vector<std::string> first_X_v_Y;
	std::vector<std::string>first_case_citation_other;
	std::vector<std::string>first_standard_cases;
	std::vector<int>citation_line_numbers;
	char* first_equiv_relations_type; //
	int current_line;
	std::unordered_map<std::string, std::string>local_dict;
	char* latest_date; //
	const Attributes* mp_attributes;
	std::unordered_map<const XMLCh*, void*>citations;
	//const xercesc::ReaderMgr* flocator;
	const xercesc::Locator* flocator;

	static xercesc::SAX2XMLReaderLoc* fparser;

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    SAX2CountHandlers();
    ~SAX2CountHandlers();

    void setDocumentLocator(const Locator* deflocator){
    	this->flocator = deflocator;
    }

    void setLocator(const ReaderMgr* locator){

    	this->flocator = locator;
    }

    void setParser(SAX2XMLReaderLoc* parser){

    	fparser = parser;
    }

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t getElementCount() const
    {
        return fElementCount;
    }

    XMLSize_t getAttrCount() const
    {
        return fAttrCount;
    }

    XMLSize_t getCharacterCount() const
    {
        return fCharacterCount;
    }

    bool getSawErrors() const
    {
        return fSawErrors;
    }

    XMLSize_t getSpaceCount() const
    {
        return fSpaceCount;
    }


    // -----------------------------------------------------------------------
    //  Handlers for the SAX ContentHandler interface
    // -----------------------------------------------------------------------
    void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname, const Attributes& attrs);
    void characters(const XMLCh* const chars, const XMLSize_t length);
    void ignorableWhitespace(const XMLCh* const chars, const XMLSize_t length);
    void startDocument();


    // -----------------------------------------------------------------------
    //  Handlers for the SAX ErrorHandler interface
    // -----------------------------------------------------------------------
	void warning(const SAXParseException& exc);
    void error(const SAXParseException& exc);
    void fatalError(const SAXParseException& exc);
    void resetErrors();


private:
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fAttrCount
    //  fCharacterCount
    //  fElementCount
    //  fSpaceCount
    //      These are just counters that are run upwards based on the input
    //      from the document handlers.
    //
    //  fSawErrors
    //      This is set by the error handlers, and is queryable later to
    //      see if any errors occured.
    // -----------------------------------------------------------------------
    XMLSize_t       fAttrCount;
    XMLSize_t       fCharacterCount;
    XMLSize_t       fElementCount;
    XMLSize_t       fSpaceCount;
    bool            fSawErrors;
};
