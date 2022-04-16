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
#define LOGGER_ERROR

#include "SAX2Count.hpp"
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <unordered_map>
#include <vector>
#include "zzoflaw.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"

#include <xercesc/validators/schema/XSDLocator.hpp>
#include <xercesc/sax/Locator.hpp>

#define tr XMLString::transcode

//#define LOGGER_ERROR // don't use log level error since macro conflicts with handlers error method
#ifndef LOGGING_LEVEL_LINER
	#include "logging.h"
#endif

static spdlog::logger logger = getLog();

// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Constructors and Destructor
// ---------------------------------------------------------------------------
SAX2CountHandlers::SAX2CountHandlers() :

	  fAttrCount(0)

	, fCharacterCount(0)
	, fSpaceCount(0)

	, first_citation(NULL) //char*
	, first_equiv_relations_type(NULL) //char*
	, fElementCount(0)
	, first_X_v_Y(0)
	, latest_date(NULL) //char*
	, first_case_citation_other(0)
	, first_standard_cases(0)
	, citation_line_numbers(0)
	, local_dict(0)
	, fSawErrors(false)
	, mp_attributes(0)

	, flocator(0)


{
	current_line = 0; //int
}

SAX2CountHandlers::~SAX2CountHandlers()
{
}

// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Implementation of the SAX DocumentHandler interface
// ---------------------------------------------------------------------------


void SAX2CountHandlers::startElement(const XMLCh* const  uri
                                   , const XMLCh* const localname
                                   , const XMLCh* const  qname
                                   , const Attributes& attrs) {

    fElementCount++;
    fAttrCount += attrs.getLength();

    char* el_localname = XMLString::transcode(localname);

    const XMLCh* line_num;
    if ((line_num = attrs.getValue(tr("line"))) != NULL){
    	char* line_int = tr(line_num); //or XMLString::parseInt
    	std::cout<<line_int<<std::endl;
    	logger.warn(format("line number in attr: {}",line_int)); // @suppress("Invalid arguments")
    	//logger.info(format("line from locator: {}", locator->getLineNumber()));// @suppress("Invalid arguments")

    	logger.warn(format("parser in handler: {}",fparser ->getScanner()->getReaderMgr()->getLineNumber())); //->getScanner();//->getLocator();//->getLineNumber();

    }

    if (XMLString::compareString(localname, tr("citation")) == 0 ){
    	mp_attributes = &attrs;
    	const XMLCh* xmlkey = attrs.getValue(tr("id"));
    	citations[xmlkey]= (void*)mp_attributes;

        if (attrs.getValue(tr("entry_type")) == NULL){

        	SPDLOG_ERROR("ERROR: \"entry_type\" not found in attributes of citation");

        	//const XSDLocator &loc = XSDLocator();
        	//const Locator* locator = this->getScanner()->getLocator();
        	/*loc.setValues(const XMLCh* const systemId,
        	              const XMLCh* const publicId,
        	              const XMLFileLoc lineNo,
        				  const XMLFileLoc columnNo);

        	const XMLCh* shit = tr("shit");
        	const SAXParseException e = SAXParseException(shit,loc);
        	this->error(e);*/


        }

    }



    //XMLString::release(&attr_name);
    	//XMLString::release(&attr_value);
    //XMLString::release(&el_localname);
}

xercesc::SAX2XMLReaderLoc* SAX2CountHandlers::fparser = NULL;

void SAX2CountHandlers::characters(  const   XMLCh* const chars
								    , const XMLSize_t length) {
    fCharacterCount += length;
    char* c_chars = XMLString::transcode(chars);
    //std::cout << c_chars;
    XMLString::release(&c_chars);

}

void SAX2CountHandlers::ignorableWhitespace( const   XMLCh* const /* chars */
										    , const XMLSize_t length)
{
    fSpaceCount += length;
}

void SAX2CountHandlers::startDocument()
{
    fAttrCount = 0;
    fCharacterCount = 0;
    fElementCount = 0;
    fSpaceCount = 0;
}


// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Overrides of the SAX ErrorHandler interface
// ---------------------------------------------------------------------------
void SAX2CountHandlers::error(const SAXParseException& e)
{
    fSawErrors = true;
    XERCES_STD_QUALIFIER cerr << "\nError at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void SAX2CountHandlers::fatalError(const SAXParseException& e)
{
    fSawErrors = true;
    XERCES_STD_QUALIFIER cerr << "\nFatal Error at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void SAX2CountHandlers::warning(const SAXParseException& e)
{
    XERCES_STD_QUALIFIER cerr << "\nWarning at file " << StrX(e.getSystemId())
		 << ", line " << e.getLineNumber()
		 << ", char " << e.getColumnNumber()
         << "\n  Message: " << StrX(e.getMessage()) << XERCES_STD_QUALIFIER endl;
}

void SAX2CountHandlers::resetErrors()
{
    fSawErrors = false;
}
