/*
 * XMLReaderLoc.cpp
 *
 *  Created on: Apr 14, 2022
 *      Author: charles
 */

#include <xercesc/util/IOException.hpp>
#include <xercesc/util/RefStackOf.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax2/LexicalHandler.hpp>
#include <xercesc/sax2/DeclHandler.hpp>
//#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/internal/XMLScannerResolver.hpp>
#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>
#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <string.h>

#ifndef XML_Factory
#include "XMLReaderFactory.hpp"
#endif
#ifndef SAX2XML_READER_LOC
#include "SAX2XMLReaderLoc.hpp"
#endif
#include <stdio.h>
#include <iostream>

#define LOGGER_ERROR
#include "logging.h"

static spdlog::logger logger = getLog();

//
class MemoryManager;
class XMLGrammarPool;



xercesc_3_2::SAX2XMLReaderLoc::~SAX2XMLReaderLoc() {

}


/*SAX2XMLReaderLoc::SAX2XMLReaderLoc(  MemoryManager* const  manager,
		                             XMLGrammarPool* const gramPool);
*/


xercesc_3_2::SAX2XMLReaderLoc * XMLReaderFactoryLoc::createXMLReaderLoc(  xercesc_3_2::MemoryManager* const  manager,
		xercesc_3_2::XMLGrammarPool* const gramPool)

{
	std::cout<<"What'd I figure out before to see?"<<std::endl;
	printf("xerces prints %s instead of values, and omits multiple white space   here\n\n\n", "never seen", "absent");
	logger.warn("ERROR: print ERROR in message due to namespace issues, other error methods presently");
	xercesc_3_2::SAX2XMLReaderLoc* pImpl=new (manager) xercesc_3_2::SAX2XMLReaderLoc(manager, gramPool);
    return pImpl;

}

