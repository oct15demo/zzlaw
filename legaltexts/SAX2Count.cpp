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


/* values docket number is set to the docket relations, for */
void doc_rel_val(unordered_map<std::string, std::string>* vals_map, std::vector<XMLCh*>* first_cite_type, const char* cite_type, std::unordered_map<std::string, std::string>* doc_rel){
	if (!first_cite_type->empty()){
		for(XMLCh* case_id:*first_cite_type){
			if( doc_rel->count(tr(case_id)) >0 ){
				std::string doc_num = std::string("docket_number");
				logger.debug(((*vals_map)[doc_num]));
				//convert string to char* with &string[0]
				if(logger.level() == spdlog::level::debug){
					if(true){//((*vals_map)[doc_num])){
						logger.error(format("values docket_number {} for case_id {} in {} replaced by {}", (*vals_map)["docket_number"],tr(case_id),cite_type, &(*doc_rel)[tr(case_id)][0])); // @suppress("Invalid arguments")
					}else{
						logger.debug(format("docket_number {} for case_id {} in {} added to values_map", &(*doc_rel)[tr(case_id)][0], cite_type, tr(case_id))); // @suppress("Invalid arguments")
					}
				}
				(*vals_map)["docket_number"] = &(*doc_rel)[tr(case_id)][0];
			}else{
				logger.debug(format("case_id {} in first_other not found in docket_relations",tr(case_id))); // @suppress("Invalid arguments")
			}
		}
	}else{
		logger.debug(format("{} is empty", cite_type)); // @suppress("Invalid arguments")
	}
}


int parseBuf(unsigned char* fileBuf, int fileBufSize, const char* filename, SAX2XMLReader* parser, unordered_map<std::string,std::string>* values_map){

	XMLCh* xmlch_filename = XMLString::transcode(filename);

	MemBufInputSource xmlBuf((const XMLByte*)fileBuf, fileBufSize, xmlch_filename , false);

	xmlBuf.setCopyBufToStream(false);

    unsigned long duration;
    bool          errorOccurred = false;

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

		//TODO: Fix stupid redundancy copied from python where first_X_v_Ys overwrites first_other
		doc_rel_val(values_map, &hand->first_case_citations_other, "first_other, ", &hand->docket_relations);
		doc_rel_val(values_map, &hand->first_X_v_Ys, "first_X_v_Ys", &hand->docket_relations);
		if (hand->first_standard_cases.size()>0){
			std::unordered_map<std::string, std::string> new_case = *values_map;

			for (XMLCh* standard_case : *&hand->first_standard_cases){
				VecAttributesImpl* next_citation = hand->citations[std::string(tr(standard_case))];
				if( next_citation->getValue(tr("standard_reporter")) != NULL &&
					(*values_map)[std::string("standard_reporter")]== "" && //could use find instead of []
					std::string(tr(next_citation->getValue(tr("standard_reporter")))) == std::string((*values_map)["standard_reporter"])){

					for(std::string attr_name : {"volume","page_number","year"}){
						if (next_citation->getValue(tr(attr_name.c_str()))!= NULL && values_map->find(attr_name)== values_map->end()){
							new_case[attr_name] = std::string(tr(next_citation->getValue(tr(attr_name.c_str())))); //add the citation attribute to the copy of values
						}
					}
				}
			}
		}

		/*
		if len(first_standard_cases)>0:
		        output = []
		        for standard_case in first_standard_cases:
		            new_case = values.copy()
		            next_citation = citations[standard_case]
		            ## if 'standard reporter' matches, they are assumed to be the same citation
		            ## (or a conflict).  Otherwise, they are assumed to be another "version"
		            if ('standard_reporter' in next_citation) and  ('standard_reporter' in values) \
		              and (next_citation['standard_reporter']==values['standard_reporter']):
		                for attrib in ['volume','page_number','year']:
		                    if (attrib in next_citation) and (not attrib in values):
		                        new_case[attrib]=next_citation[attrib]



		            ###############  STILL TO DO  ###############


		            elif not 'standard_reporter' in next_citation:
		                raise Exception('No standard reporter in citation:'+str(next_citation))
		            else:
		                for attrib in ['standard_reporter','volume','page_number','year']:
		                    if attrib in next_citation:
		                        if (attrib == 'year') and ((not latest_date) or later_date_string(next_citation[attrib],latest_date)):
		                            latest_date = next_citation[attrib]
		                        new_case[attrib]=next_citation[attrib]
		                if next_citation['id'] in docket_relations:
		                    new_case['docket_number'] = docket_relations[next_citation['id']]
		            output.append(new_case)
		        logger.debug(f'{[values]} , {latest_date}')
		        return(f'{str(output)} , {latest_date}')
		    else:
		        logger.debug(f'{[values]} , {latest_date}')
		        return(str([values]), latest_date)

*/
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
