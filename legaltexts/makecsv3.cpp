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

#include "logging.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"

#include "zzoflaw.h"

static spdlog::logger logger = getLog();

/* Used in catch statements, log error and continue or log and rethrow exception to end execution.
}catch(const std::exception& e) {
	logger.debug(format("Error {} {}", "some error info", e.what() ));
	if(EXIT)
		throw;
	return -1;
} */
bool EXIT=0;

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

//auto logger = *spdlog::get("log");logger = 	logger = *spdlog::stdout_color_mt("log");


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

//std::tuple<double, char, std::string> get_student(int id)

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
		logger.error(format("file {} does not exist", IE_file)); // @suppress("Invalid arguments")
		if(EXIT)
			throw;// @suppress("Invalid arguments")
	}
	unsigned char* file_read = readFile(IE_file);

	return getValsYear();
}

// TODO: move to header


//TODO: add declarations to header and move util functions to a separate file

// https://stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
//https://stackoverflow.com/questions/238603/how-can-i-get-a-files-size-in-c
//https://unix.stackexchange.com/questions/621157/why-is-the-type-of-stat-st-size-not-unsigned-int
//https://stackoverflow.com/questions/62900964/how-to-properly-check-that-an-off-t-value-wont-overflow-when-converted-to-size
//man sysexits


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
		logger.error(format("Error casting to size_t for {}, (off_t) {}, SIZE_MAX= {}", source, (uintmax_t)offsize, SIZE_MAX )); // @suppress("Invalid arguments")
		if (EXIT) exit(-1);
	}
	return castoff;
}
//https://www.gnu.org/software/libc/manual/html_node/Error-Messages.html
unsigned char* readFile(char* filename){


	off_t file_off_t = fileSize(filename);
	size_t bytes_read, bytes_expected = castOffSize(file_off_t, filename);

	int fd;
	if ((fd = open(filename,O_RDONLY)) < 0){
		logger.error(format("Error opening file {} fd {}; {} {}", filename, fd, errno, strerror(errno))); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}
	unsigned char* dataptr;
	if ((dataptr = (unsigned char*)malloc(bytes_expected)) == NULL){
		// printf("%s %zu","data malloc for file ", filename, bytes_expected );
		logger.error(format("malloc file {} size_t {}; {} {}", filename, bytes_expected, errno, strerror(errno))); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}
	if (bytes_expected >= 1073741824) //1073741824 GB 1048576 MB 1024 kb
		logger.warn(format("Warning: reading large file size > 1 GB filename {} {:.{}f} GB", filename, bytes_expected/1073741824, 1 )); // @suppress("Invalid arguments")
	bytes_read = read(fd, dataptr, bytes_expected);
	if (bytes_read != bytes_expected){
		logger.error(format("Reading file {} bytes_read {} bytes_expected {}", filename, bytes_read, bytes_expected)); // @suppress("Invalid arguments")
		if (EXIT) exit(EXIT_FAILURE);
	}

	setDataPtr(dataptr); //getDataPtr() used to free
	return dataptr;
}

bool fileExists(char* filepath){ //SO 12774207
	if (FILE *file = fopen(filepath, "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}
}
