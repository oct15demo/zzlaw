/*
 * XMLReaderLoc.h
 *
 *  Created on: Apr 14, 2022
 *      Author: charles
 */

//#ifndef XMLREADERLOC_H_
//#define XMLREADERLOC_H_
//#endif
#define XML_READER_LOC
#define LOGGER_ERROR
//Below declares for class SAX2XMLReaderImplLoc vs xercesc for SAX2XMLReaderImpl
#include "SAX2XMLReaderImpl.hpp"
//#include <xercesc/parsers/SAX2XMLReaderImpl.hpp>
#define SAX2XML_READER_LOC
#include <xercesc/util/PlatformUtils.hpp>

// Note: Static field XMLPlatformUtils::fgMemoryManager breaks without
// namespace declaration
XERCES_CPP_NAMESPACE_BEGIN


//class PARSERS_EXPORT SAX2XMLReaderLoc : public SAX2XMLReaderImpl{
class SAX2XMLReaderLoc : public SAX2XMLReaderImplLoc{
	//: public xercesc_3_2::SAX2XMLReaderImpl {

public:

	SAX2XMLReaderLoc(MemoryManager* const manager,XMLGrammarPool* const gramPool):
		SAX2XMLReaderImplLoc(manager,gramPool){}

virtual ~SAX2XMLReaderLoc();

	//SAX2XMLReaderLoc(const SAX2XMLReaderLoc& sax2ReaderLoc){

	//}


	/*static SAX2XMLReaderLoc * createXMLReaderLoc(
			MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager,
			XMLGrammarPool* const gramPool = 0
	);
*/
};
}
/* XMLREADERLOC_H_ */
