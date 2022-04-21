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

#include "SAX2Count.hpp"

#ifndef COUNT_HANDLER_H
#include "SAX2CountHandlers.hpp"
#endif

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>

#include <unordered_map>
#include <set>
#include <vector>
#include "zzoflaw.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"

#include <xercesc/validators/schema/XSDLocator.hpp>
#include <xercesc/sax/Locator.hpp>
#include <iterator>

#define tr XMLString::transcode
#define LOGGER_ERROR_INCLUDE // unnecessary after removing unused ReaderImpl and ReaderLoc files
//#define LOGGER_ERROR // don't use log level error since macro conflicts with handlers error method
#ifndef LOGGING_LEVEL_LINER
#include "logging.h"
#endif

static spdlog::logger logger = getLog();


//static members only
char* entry_type_chptrs[] = { {"case_X_vs_Y"}, {"case_citation_other"}, {"standard_case"}};
std::set<const XMLCh*, decltype(cmpx)> SAX2CountHandlers::entry_types(cmpx);
std::set<int> SAX2CountHandlers::citation_line_numbers;
// ---------------------------------------------------------------------------
//  SAX2CountHandlers: Constructors and Destructor
// ---------------------------------------------------------------------------
SAX2CountHandlers::SAX2CountHandlers() :
	// Since citation line numbers are added only if they aren't already in data structure
	// citation_line_numbers, the python [] list is a set instead of vector.

    //TODO: should more of these be static
	  fAttrCount(0)
	, fCharacterCount(0)
	, fSpaceCount(0)
	, first_citation(NULL) //char*
	, first_equiv_relations_type(NULL) //char*
	, fElementCount(0)
	, first_X_v_Ys(0)
	, latest_date(NULL) //char*
	, first_case_citations_other(0)
	, first_standard_cases(0) //static
	, local_dict(0)
	, fSawErrors(false)
	, mp_attributes(0)
	, flocator(0)


{
	current_line = 0; //int
	// loop because tr needed to make XMLCh*, cast to const* to match attributes
	for (int i=0;i<(sizeof(entry_type_chptrs)/(sizeof(char*)));i++){
		const XMLCh* entry_type_x = (const XMLCh*)tr(entry_type_chptrs[i]);
		entry_types.insert(entry_type_x);
		int count = entry_types.count(entry_type_x);
		logger.debug(format("entry type {}: count {} for {}",i, count,entry_type_chptrs[i]));// @suppress("Invalid arguments")
	}

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
    int line_number;
    const XMLCh* line_num_x;
    if ((line_num_x = attrs.getValue(tr("line"))) != NULL){
    	line_number = atoi(tr(line_num_x)); //or XMLString::parseInt
    	logger.debug(format("current line number in attr: {}, actual line number {}, file {}",line_number, // @suppress("Invalid arguments")
    			    				(unsigned int)flocator->getLineNumber(),tr(flocator->getSystemId()))); // @suppress("Invalid arguments")
    	current_line = line_number;
    } else{
    	char* filename = tr(flocator->getSystemId());
    	logger.error(format("No line number in attr, actual line number {},line_number, file‘{}", // @suppress("Invalid arguments")
    	        	    			    				(unsigned int)flocator->getLineNumber(),tr(flocator->getSystemId()))); // @suppress("Invalid arguments")

        	//logger.warn(format("No line number in attr, actual line number {}, file {}",line_number, // @suppress("Invalid arguments")
        	  //  			    				(unsigned int)flocator->getLineNumber(), tr(flocator->getSystemId()))); // @suppress("Invalid arguments")
    }

    if (XMLString::compareString(localname, tr("citation")) == 0 ){
    	mp_attributes = &attrs;
    	const XMLCh* id_x = attrs.getValue(tr("id"));
    	citations[id_x]= (void*)mp_attributes;
    	//logger.debug(format("Attributes added to citation map with id {}", tr(id_x))); // @suppress("Invalid arguments")
    	char* id = tr(id_x);
    	const XMLCh* entry_type_x = attrs.getValue(tr("entry_type"));
        if (entry_type_x == NULL){
        	//logger.error("ERROR: \"entry_type\" not found in attributes of citation");
        	if(EXIT)exit(EXIT_FAILURE);
        }else{
        	logger.debug(format("entry_type in citation {}: {}", id, tr(entry_type_x))); // @suppress("Invalid arguments")
        }

        // <10 && (XvY,std,other) && ((first XvY,other) or no line attr)
        //logic: What is purpose of first XvY, other, no line? no check of standard?
        if( current_line < 10 && entry_types.count(entry_type_x)!=0 &&
        		((first_X_v_Ys.empty() && first_case_citations_other.empty()) || current_line==0)){
        	logger.debug("< line 10, XvY,std,other, (1st XvY,other empty or line 0");
        	local_dict[id_x]= (void*)mp_attributes;
        	logger.debug(format("Added attributes to local_dict map, id {}",id)); // @suppress("Invalid arguments")
        	if(citation_line_numbers.count(line_number)==0){
        		citation_line_numbers.insert(line_number);
        		//logger.debug(format("line_number {} added to citation_line_numbers",line_number)); // @suppress("Invalid arguments")
        	}else{
        		//not necessarily an error, but log that way to find if ever occurs
        		logger.error(format("Can line_number appear twice? File {} line {}, actual {}",tr(flocator->getSystemId()),line_number, flocator->getLineNumber())); // @suppress("Invalid arguments")
        	}
        	if(XMLString::compareString(entry_type_x,tr("case_X_vs_Y")) == 0){
        		first_X_v_Ys.push_back((XMLCh*)id_x);
        		logger.debug(format("Append to first XvYs {} ",id));// @suppress("Invalid arguments")
            	const XMLCh* year = attrs.getValue(tr("year"));
            	if(year != NULL){
            		logger.debug(format("latest_year {}",tr(year)));// @suppress("Invalid arguments")
            		latest_date = year;
            	} else {
            		logger.error(format("No year for citation {}",id)); // @suppress("Invalid arguments")
            	}
        	}
        } else {
        	logger.debug(format("{}",entry_types.size())); // @suppress("Invalid arguments")
        	//entry_types.insert(entry_type_x);
        	set<const XMLCh *,less<const XMLCh *>,allocator<const XMLCh *>>::iterator iter = entry_types.begin();
        	//logger.debug(format("{}",(const XMLCh*)*iter)); // @suppress("Invalid arguments")
        	std::cout<<tr(*iter++)<<std::endl;
        	std::cout<<tr(*iter++)<<std::endl;
        	std::cout<<tr(*iter)<<std::endl;
        	logger.debug(format("\n{}\n{}\n{}\n{}\n{}\n{}\n{}\n{}", // @suppress("Invalid arguments")
        	        current_line < 10,
					entry_types.count((const XMLCh*)entry_type_x),
					tr(entry_type_x),
					entry_types.count((const XMLCh*)(tr("standard_case"))),
					entry_types.count(tr("standard_case")),
					first_X_v_Ys.empty(),
					first_case_citations_other.empty(),
			        current_line
        	        ));
        }

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


//xercesc::SAX2XMLReaderLoc* SAX2CountHandlers::fparser = NULL;

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


/*// ---------------------------------------------------------------------------
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
*/

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
