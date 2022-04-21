#include <string.h>
#include <stdio.h>
#include <cstdio> //fopen
#include <stdlib.h>

// for file read
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>
#include <time.h>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"

#include "zzoflaw.h"
#include <xercesc/framework/MemBufInputSource.hpp>


#include "SAX2Count.hpp"

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>

#ifndef XML_FACTORY
#include <xercesc/sax2/XMLReaderFactory.hpp>
#endif

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


#ifndef XML_FACTORY
//#include "XMLReaderFactory.hpp"
#endif
#ifndef SAX2XML_READER_LOC
//#include "SAX2XMLReaderLoc.hpp"
#endif
#ifndef COUNT_HANDLER_H
//#include "SAX2CountHandlers.hpp"
#endif


int parseBuf(unsigned char* fileBuf, int fileBufSize, char* filename, SAX2XMLReader* parser);

//ifdef LOGGER_ERROR in logging.h since xercesc/src/framework/XMLErrorReporter function 'error' conflicts
//#define LOGGER_ERROR
#define LOGGER_ERROR_INCLUDE
#include "logging.h"

static spdlog::logger logger = getLog();

/* Used in catch statements, log error and continue or log and rethrow exception to end execution.
}catch(const std::exception& e) {
	logger.debug(format("Error {} {}", "some error info", e.what() ));
	if(EXIT)
		throw;
	return -1;
} */

// ([{'id1': '100000', 'folder': 'scotus', 'full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES', 'standard_reporter': 'U.S.', 'volume': '259', 'page_number': '188', 'year': '1922'}], '1922')
/*
If using Eclipse, properties, C/C++ Build, Settings, GCC C++ Compiler, Dialect, Language standard, you can select ISO C++11 (-std=c++0x).
This was on a MacAir macOS 10.13 Eclipse Oxygen 3a c++ 4.2.1 clang 900 –
*/

static unsigned char* dataPtr;

void setDataPtr(unsigned char* startOfData){
	dataPtr= startOfData;
}

unsigned char* getDataPtr(){
	return dataPtr;
}

int mainfromfillindata(int argc, char**argv){
	if(pr)cout<<"hello from fillindata main"<<n;
	TupplesYear valsyear = getValsYear();
	Tupple* vals = getVals();
	if(pr){
		printf("%s\n",vals[5].key);
		cout<<valsyear.year<<n;
		cout<<valsyear.length<<n;
	}
    return 1;

}

/*******************************************************************
 *
 *                    NEW STUFF FOR makecsv3.cpp
 *
 *******************************************************************/

/* This is the C version of the similarly named function in the python file makecsv3.py
 * An additional parameter 'tupplelength' is added to provide the length of the parameter
 * 'values', an array tupple structures. It is not possible to determine the size of
 * an array passed as a parameter and python Ctypes only works with C structures, vs
 * C++ classes and std data structures like vectors and unordered_maps. To get around
 * this constraint, python lists or dictionaries are passed as arrays of structures and
 * then converted back into vectors or maps in C++. C++ maps and vectors returned to python
 * are first converted into arrays of structures and then passed to python where they can
 * be converted into dictionaries and lists.
 */

//Test function
extern "C"
TupplesYear fillInDataFromIEOutfile(char* IE_file, Tupple* values, int tupplelength, char* txt_file,bool trace=false){

	bool this_is_not_a_test = true;
	bool pr = true; // true usually only in test environment or debugging

	HashMap pydict;
	if(pr)printf("values passed to and printed from c program: \n\n");
	for (int i=0; i<tupplelength; i++){
		if(pr)printf("%20s : %s \n", values[i].key, values[i].value);
		pydict[values[i].key]=values[i].value;

	}
	if(pr)printf("\n");
	static Tupple replaceme = {"key","value"};
	if(this_is_not_a_test){
		return fillInForReal(IE_file, &replaceme, 1, txt_file, trace);
	}else{
		return getValsYear(pydict);
	}
}

//Not a test function
TupplesYear fillInForReal(char* IE_file, Tupple* values, int valuesLength, char* txt_file,bool trace){

	logger.debug("hello fill_in_for_real");
	//TODO: move to header?
	char* first_citation;
	unordered_map<string, string> citations;
	vector<string> first_X_v_Y;
	vector<string>first_case_citation_other;
	vector<string>first_standard_cases;
	vector<int>citation_line_numbers;
	char* first_equiv_relations_type;
	int current_line;
	unordered_map<string, string>local_dict;
	char* latest_date;

	first_citation = "False";
	first_equiv_relations_type = "False";
	current_line = 0;
	latest_date = "False";

	if(fileExists(IE_file)){
		logger.debug(format("file {} exists", IE_file)); // @suppress("Invalid arguments")
	}else{
		logger.error(format("ERROR: file {} does not exist", IE_file)); // @suppress("Invalid arguments")
		if(EXIT)
			throw;// @suppress("Invalid arguments")
	}
	FileIn* fileIn = (FileIn*)malloc(sizeof(FileIn));

	for(int i = 0; i < 1; i++){//looptheloop 1000
		readFile(IE_file, fileIn, true);
	}

    bool                         recognizeNEL = false;
    char                         localeStr[64];
    memset(localeStr, 0, sizeof localeStr);

    // On avoiding seg faults:
    // https://xml.apache.org/xerces-c-new/faq-parse.html#faq-5
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
		exit(-1);//return;
	};
	bool                         errorOccurred = false;
	int x;
	{ // Everything after XMLPlatformUtils::Initialize(); goes into separate code
	  // as per https://xml.apache.org/xerces-c-new/faq-parse.html#faq-5 to
	  // avoid seg fault.
		const char*                  xmlFile      = 0;
		SAX2XMLReader::ValSchemes    valScheme    = SAX2XMLReader::Val_Auto;
		bool                         doNamespaces = true;
		bool                         doSchema = true;
		bool                         schemaFullChecking = false;
		bool                         identityConstraintChecking = true;
		bool                         doList = false;
		bool                         namespacePrefixes = false;

		//SAX2XMLReader* parser = SAX2XMLReaderImpl::createXMLReader();

		SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();

		//SAX2XMLReaderLoc* parser = SAX2XMLReaderLoc::createXMLReader();
		//const SAX2XMLReaderLoc &parser = SAX2XMLReaderLoc();
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

		SAX2CountHandlers handler = SAX2CountHandlers();
		parser->setContentHandler(&handler);
		parser->setErrorHandler(&handler);
		//XMLScanner* scanner = parser->getScanner();

		//const ReaderMgr* locator = scanner->getReaderMgr();
		//handler.setLocator(locator);
		//handler.setParser(parser);
		int run_num = 1; //looptheloop 1000
		for(int i = 0; i < run_num; i++){
			parseBuf(fileIn->fileptr, fileIn->size, "lenomdeficheavec_éàçôï", parser);// fileIn->filename);
		}
		delete parser;

	} // End of code block that followed XMLPlatformUtils::Initialize()

	//And call the termination method
	XMLPlatformUtils::Terminate();

	if (errorOccurred)
		logger.error("ERROR: error occurred");
		//return 4;
	else
		logger.info("process without xerces 'errorOccurred' error before getValsYear");
		//return 0;

	return getValsYear();

}

// https://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
//https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
//https://unix.stackexchange.com/questions/621157/why-is-the-type-of-stat-st-size-not-unsigned-int
//https://stackoverflow.com/questions/62900964/how-to-properly-check-that-an-off-t-value-wont-overflow-when-converted-to-size

off_t fileSize(const char *filename) try {
    struct stat st;
	if (stat(filename, &st) == 0)
		logger.debug(format("File size {} = {}", filename, st.st_size)); // @suppress("Invalid arguments")
	return st.st_size;
}catch (const std::exception& e) {
	logger.debug(format("Error {} {}", filename, e.what() )); // @suppress("Invalid arguments")
	if (EXIT)
		throw;
	return -1; // in production or sometimes debug, log error and continue with next file
}

//https://www.runscripts.com/support/guides/porting/c-integral-types
// to print offsize printf ("%jd", (intmax_t) pos);
size_t castOffSize(off_t offsize, const char* source="unnamed source"){
	size_t castoff;
	if (offsize >= 0 && (uintmax_t)offsize <= SIZE_MAX){
		castoff = (size_t)offsize;
		logger.debug(format("File size {} = {}", source, castoff)); // @suppress("Invalid arguments")
	} else {
		castoff = (size_t)-1; // for printf %jd
		logger.warn(format("ERROR: Failed casting to size_t for {}, (off_t) {}, SIZE_MAX= {}", source, (uintmax_t)offsize, SIZE_MAX )); // @suppress("Invalid arguments")
		if (EXIT) exit(-1);
	}
	return castoff;
}

//https://www.gnu.org/software/libc/manual/html_node/Error-Messages.html
/*
 *  Read file from given path in param filename
 *  Store entire file as one unsigned char* dynamically allocated with malloc
 *  The parameter addRoot is a hack to add <root> </root> to start and end of
 *  file read respectively to otherwise invalid XML which xerces won't parse.
 *  Adding root tags in creation process left as a TODO
 */

//TODO: Make valid XML files that don't require adding root elements

FileInput* readFile(char* filename, FileIn* fileIn, bool addRoot){

	off_t file_off_t = fileSize(filename);
	size_t bytes_read, bytes_expected = castOffSize(file_off_t, filename);
	int fd;
	if ((fd = open(filename,O_RDONLY)) < 0){
		logger.error(format("ERROR: Unable to open file {} fd {}; {} {}", filename, fd, errno, strerror(errno))); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}

	unsigned char* dataptr;
	const char* root_start = addRoot?"<root>\n":"";
	const char* root_end = addRoot?"</root>":"";
	int len_start = strlen(root_start);
	int len_end = strlen(root_end);

	if ((dataptr = (unsigned char*)malloc(bytes_expected + len_start + len_end + 1)) == NULL){ // + 1 set to '\0' for safe strtok
		// printf("%s %zu","data malloc for file ", filename, bytes_expected );
		logger.error(format("ERROR: malloc file {} size_t {}; {} {}", filename, bytes_expected, errno, strerror(errno))); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}

	if (bytes_expected >= 1073741824) //1073741824 GB 1048576 MB 1024 kb
		logger.warn(format("Warning: reading large file size > 1 GB filename {} {} GB", filename, bytes_expected/1073741824, 1 )); // @suppress("Invalid arguments")

	bytes_read = read(fd, dataptr + len_start, bytes_expected);

	if (addRoot)
		addRootElement(root_start, root_end, len_start, len_end, dataptr, bytes_read);

	if (bytes_read != bytes_expected){
		logger.error(format("ERROR: Reading file {} bytes_read {} bytes_expected {}", filename, bytes_read, bytes_expected)); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}

	fileIn->fileptr = dataptr;
	fileIn->size = bytes_expected + len_start + len_end;
	int len = strlen(filename)+1;
	char* fname = (char*)malloc(len);
	strncpy(fname, filename, len);
	fileIn->filename = fname;
	return fileIn;
}

bool fileExists(char* filepath){ //SO 12774207
	if (FILE *file = fopen(filepath, "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}

void addRootElement(const char* root_start, const char* root_end, int len_start, int len_end,
		unsigned char* dataptr, int bytes_read){

	strncpy((char*)dataptr, root_start, len_start);
	strcpy((char*)dataptr+bytes_read + len_start, root_end);
	logger.debug(format("file read and rooted:\n{}\n", (char*)dataptr)); // @suppress("Invalid arguments")

}
