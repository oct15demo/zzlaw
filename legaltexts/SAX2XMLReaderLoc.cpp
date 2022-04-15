/*
 * XMLReaderLoc.cpp
 *
 *  Created on: Apr 14, 2022
 *      Author: charles
 */
#include "SAX2XMLReaderLoc.hpp"
#include <xercesc/util/IOException.hpp>
#include <xercesc/util/RefStackOf.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax2/LexicalHandler.hpp>
#include <xercesc/sax2/DeclHandler.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
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




XERCES_CPP_NAMESPACE_BEGIN
class MemoryManager;
class XMLGrammarPool;



SAX2XMLReaderLoc::~SAX2XMLReaderLoc() {

}


/*SAX2XMLReaderLoc::SAX2XMLReaderLoc(  MemoryManager* const  manager,
		                             XMLGrammarPool* const gramPool);
*/


SAX2XMLReaderLoc * SAX2XMLReaderLoc::createXMLReader(  MemoryManager* const  manager,
		                             XMLGrammarPool* const gramPool)


{
    SAX2XMLReaderLoc* pImpl=new (manager) SAX2XMLReaderLoc(manager, gramPool);
    return pImpl;

}
}
