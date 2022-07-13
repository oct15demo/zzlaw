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
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>
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



int parseBuf(unsigned char* fileBuf, int fileBufSize, const char* filename, SAX2XMLReader* parser){


//int parseThat(){

  /*  const char*                  xmlFile      = 0;
    SAX2XMLReader::ValSchemes    valScheme    = SAX2XMLReader::Val_Auto;
    bool                         doNamespaces = true;
    bool                         doSchema = true;
    bool                         schemaFullChecking = false;
    bool                         identityConstraintChecking = true;
    bool                         doList = false;
    bool                         errorOccurred = false;
    bool                         namespacePrefixes = false;
    bool                         recognizeNEL = false;
    char                         localeStr[64];
    memset(localeStr, 0, sizeof localeStr);
*/
/*    // Initialize the XML4C2 system
	try {
		if (strlen(localeStr)) {
			XMLPlatformUtils::Initialize(localeStr);
		} else {
			XMLPlatformUtils::Initialize();
		}
		if (recognizeNEL) {
			XMLPlatformUtils::recognizeNEL(recognizeNEL);
		}
	} catch (const XMLException& toCatch) {
		XERCES_STD_QUALIFIER cerr << "Error during initialization! Message:\n"
				<< StrX(toCatch.getMessage()) << XERCES_STD_QUALIFIER endl;
		return 1;
	}

	*/
	XMLCh* xmlch_filename = XMLString::transcode(filename);


	MemBufInputSource xmlBuf((const XMLByte*)fileBuf, fileBufSize, xmlch_filename , false);

	xmlBuf.setCopyBufToStream(false);

/*
	SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
	parser->setFeature(XMLUni::fgSAX2CoreNameSpaces, doNamespaces);
	parser->setFeature(XMLUni::fgXercesSchema, doSchema);
	parser->setFeature(XMLUni::fgXercesHandleMultipleImports, true);
	parser->setFeature(XMLUni::fgXercesSchemaFullChecking, schemaFullChecking);
	parser->setFeature(XMLUni::fgXercesIdentityConstraintChecking, identityConstraintChecking);
	parser->setFeature(XMLUni::fgSAX2CoreNameSpacePrefixes, namespacePrefixes);

	if (valScheme == SAX2XMLReader::Val_Auto){
		parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
		parser->setFeature(XMLUni::fgXercesDynamic, true);
	}
	if (valScheme == SAX2XMLReader::Val_Never){
		parser->setFeature(XMLUni::fgSAX2CoreValidation, false);
	}
	if (valScheme == SAX2XMLReader::Val_Always){
		parser->setFeature(XMLUni::fgSAX2CoreValidation, true);
		parser->setFeature(XMLUni::fgXercesDynamic, false);
	}
*/

    //  Create our SAX handler object and install it on the parser, as the
    //  document and error handler.

	//SAX2CountHandlers handler = SAX2CountHandlers();
    //parser->setContentHandler(&handler);
    //parser->setErrorHandler(&handler);
	//parser->fscanner->getLocator();

    unsigned long duration;
    bool                         errorOccurred = false;
	//try{
    if(true){
    try{
		const unsigned long startMillis = XMLPlatformUtils::getCurrentMillis();
		//parser->parse(xmlFile);
		parser->parse(xmlBuf);
		// this works ContentHandler* handler = parser->getContentHandler();
		//DefaultHandler* saxhandler = (DefaultHandler)handler;
		// this works std::unordered_map<const XMLCh*, void*> citations = ((SAX2CountHandlers*)handler)->citations;

		SAX2CountHandlers* hand = (SAX2CountHandlers*)parser->getContentHandler();
		//DefaultHandler* saxhandler = (DefaultHandler)handler;
		std::unordered_map<const XMLCh*, const Attributes*> cites = hand->citations;
		logger.debug(cites.size());
		//const Attributes* vattrs =  cites[0];
		for (auto& el: cites) {
		    if(el.second)std::cout << el.second->getLength()<<tr(el.second->getValue(tr("relation_type"))) << " "<<(el.second==NULL?"NULL":"Not NULL")<<tr(el.first)<<std::endl;//<<el.second->findValue("relation_type");
		    //std::cout<<"\n\n\n\n"<<tr(el.second->getValue(tr("entry_type"))) <<"\n\n\n";
		}
		//vattrs->size();
		//const Attributes* attrs = vattrs->elementAt(0);
		//std::cout<< "\n\n\n oh oh %lu \n\n\n", vattrs->size();//, attrs->elementAt(1)->getValue()
		//std::cout<< "\n\n\n" << vattrs->getValue(tr("relation_type"));
		//std::cout<< "\n\n\n" << (vattrs == NULL?"true":"false");

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

    //SAX2CountHandlers* handler = (SAX2CountHandlers*)parser->getContentHandler();
	// Print out the stats that we collected and time taken
    //SAX2CountHandlers* handler = (SAX2CountHandlers*)parser->getContentHandler();

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
