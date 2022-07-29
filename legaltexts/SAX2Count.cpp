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
#include <sstream>

#include "SAX2Count.hpp"
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>
#include <xercesc/internal/VecAttributesImpl.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#if defined(XERCES_NEW_IOSTREAMS)
#include <fstream>
#else
#include <fstream.h>
#endif
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>


#include <CoreFoundation/CoreFoundation.h>
#include <string>
#include <iostream>
#include <locale.h>
#include <locale>
#include <clocale>

#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>

#define tr XMLString::transcode

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"

#define LOGGER_ERROR_INCLUDE
#include "logging.h"

static spdlog::logger logger = getLog();

#include "zzoflaw.h"


int parseBuf(unsigned char* fileBuf, int fileBufSize, const char* filename, SAX2XMLReader* parser, unordered_map<const char*, const char*>* values_map){

	XMLCh* xmlch_filename = XMLString::transcode(filename);

	MemBufInputSource xmlBuf((const XMLByte*)fileBuf, fileBufSize, xmlch_filename , false);

	xmlBuf.setCopyBufToStream(false);

    unsigned long duration;
    bool          errorOccurred = false;
	//try{
    if(true){
    try{
		const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();

		parser->parse(xmlBuf);

		SAX2CountHandlers* hand = (SAX2CountHandlers*)parser->getContentHandler();
		//DefaultHandler* saxhandler = (DefaultHandler)handler;

		std::unordered_map<std::string, VecAttributesImpl* >cites = hand->citations;
		logger.debug(cites.size());

		for (std::pair <std::string, VecAttributesImpl*>el: cites) {

			std::stringstream bruce; //bruce stringstream

				bruce << el.first << "    ";
				for(unsigned int i=0;i<((VecAttributesImpl*)(el.second))->getLength();i++){
					if(logger.level() == spdlog::level::debug){
						bruce << tr(((VecAttributesImpl*)(el.second))->getLocalName(i))<<" : "<<tr(((VecAttributesImpl*)(el.second))->getValue(i))<< " | ";
					}
				}
		    logger.debug(bruce.str());
		}
		std::vector<XMLCh*> first_other = hand->first_case_citations_other;
		std::unordered_map<std::string, std::string> doc_rel = hand->docket_relations;

		if (!first_other.empty()){
			for(XMLCh* case_id:first_other){
				if( doc_rel.count(tr(case_id)) >0 ){
					//convert string to char* with &string[0]
					if(logger.level() == spdlog::level::debug){
						if((*values_map)["docket_number"] != NULL){
							logger.error(format("values docket_number {} for case_id {} replaced by {}", (*values_map)["docket_number"],tr(case_id),&doc_rel[tr(case_id)][0])); // @suppress("Invalid arguments")
						}else{
							logger.debug(format("docket_number {} for case_id {} added to values_map", &doc_rel[tr(case_id)][0],tr(case_id))); // @suppress("Invalid arguments")
						}
					}
					(*values_map)["docket_number"] = &doc_rel[tr(case_id)][0];
				}
			}
		}

		const unsigned long endMillis = XMLPlatformUtils::getCurrentMillis();
		duration = endMillis - startMillis;
	} catch (const OutOfMemoryException&) {
		XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
		errorOccurred = true;
		//continue;  //for looping through file list, original in while loop
	} catch (const XMLException& e) {
		XERCES_STD_QUALIFIER cerr << "\nError during parsing: '" << xmlBuf.getSystemId() << "'\n"
			<< "Exception message is:  \n"
			<< StrX(e.getMessage()) << "\n" << XERCES_STD_QUALIFIER endl;
		errorOccurred = true;
		//continue;
	} catch (...) {
		XERCES_STD_QUALIFIER cerr << "\nUnexpected exception during parsing: '" << xmlBuf.getPublicId()<< "'\n";
		errorOccurred = true;
		//continue;
	}
    }

    //Code below from original SAX2Count.cpp xerces example

    //SAX2CountHandlers* handler = (SAX2CountHandlers*)parser->getContentHandler();
	//Print out the stats that we collected and time taken

    /*SAX2CountHandlers* handlergot = (SAX2CountHandlers*)parser->getContentHandler();
	if (!handlergot->getSawErrors()) {

		XERCES_STD_QUALIFIER cout << XMLString::transcode(xmlBuf.getSystemId()) << ": " //<< duration << " ms ("
			<< handlergot->getElementCount() << " elems, "
			<< handlergot->getAttrCount() << " attrs, "
			<< handlergot->getSpaceCount() << " spaces, "
			<< handlergot->getCharacterCount() << " chars" << XERCES_STD_QUALIFIER endl;

	} else {
		errorOccurred = true;
	}*/


    /*delete parser;

	//And call the termination method
	XMLPlatformUtils::Terminate();
*/
	if (errorOccurred)
		return 4;
	else
		return 0;

}
