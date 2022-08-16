#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// for file read
#include <err.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>
#include <time.h>
#include <unordered_map>
#include <iostream>

#include "zzoflaw.h"

#include <spdlog/spdlog.h>
#include "spdlog/sinks/basic_file_sink.h" // support for basic file logging
#include "spdlog/sinks/ostream_sink.h"
#include "logging.h"

#include <sstream>
#include <functional>
#include <streambuf>
#include <ostream>
#include <functional>
#include <string>
#include <memory>
#include <iostream>

// ([{'id1': '100000', 'folder': 'scotus', 'full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES', 'standard_reporter': 'U.S.', 'volume': '259', 'page_number': '188', 'year': '1922'}], '1922')
/*
If using Eclipse, properties, C/C++ Build, Settings, GCC C++ Compiler, Dialect, Language standard,
you can select ISO C++11 (-std=c++0x), but better to enter on input field below
     Other dialect flags      -std=c++2a
which then uses C++ 20, assuming you have it installed.
This was on a MacAir macOS 10.13 Eclipse Oxygen 3a c++ 4.2.1 clang 900 –
*/

static spdlog::logger logger = getLog();


/******************************************************
 *
 *                Meat and Potatoes
 *
 ******************************************************/


Tupple* charMap2tupples(std::unordered_map<char*,char*> realMap){

	int mapLen = realMap.size();
	Tupple* mapList = (Tupple*)malloc(mapLen * sizeof(Tupple));
	Tupple* mapZero = mapList;

	for (pair<const char*, const char*> entry: realMap){
		if(pr)cout<<entry.first<<", ";
		int flen = (strlen(entry.first))+1;
		int slen = (strlen(entry.second))+1;
		char* fptr = (char*)malloc(flen*sizeof(char));
		char* sptr = (char*)malloc(slen*sizeof(char));
		strcpy(fptr,entry.first);
		strcpy(sptr,entry.second);
		fptr[flen-1]='\0';
		sptr[slen-1]='\0';
		if(pr)cout<<fptr<<", ";
		mapList->key = fptr;
		mapList->value = sptr;
		if(pr)cout<<mapList[0].key<<n;
		mapList++;
		//printf("%s %s\n", mapList->key, mapList->value);
	}
	//cout<<mapList[0].key;
	return mapZero;
}

Tupples* strMap2tupples(std::unordered_map<std::string,std::string>*realMap){

	int mapLen = realMap->size();
	Tupple* tuppleArray = new Tupple[mapLen];
	Tupple* tupplePtr = tuppleArray; //
	Tupples* tupples = new Tupples; //Tupples holds Tupple* and length, like std::array
	int len=0;
	for (pair<std::string, std::string> entry: *realMap){
		int flen = (entry.first.length())+1;
		int slen = (entry.second.length())+1;
		char* fptr = (char*)malloc(flen*sizeof(char));
		char* sptr = (char*)malloc(slen*sizeof(char));
		strcpy(fptr,entry.first.c_str());
		strcpy(sptr,entry.second.c_str());
		fptr[flen-1]='\0';
		sptr[slen-1]='\0';
		tupplePtr->key = fptr;
		tupplePtr->value = sptr;
		logger.trace(format("{} : {}",tupplePtr->key, tupplePtr->value)); // @suppress("Invalid arguments")
		tupplePtr++;
		len++;
	}
	tupples->tuppledKeyVals = tuppleArray;
	tupples->length = len;
	return tupples;
}
//TODO: shallowStr2tupples used to test if malloc is big performance hit
Tupple* shallowChar2tupples(std::unordered_map<char*,char*> realMap){

	mapLen = realMap.size();
	Tupple* mapList = (Tupple*)malloc(mapLen * sizeof(Tupple));
	Tupple* mapZero = mapList;

	for (pair<const char*, const char*> entry: realMap){
		if(pr)cout<<entry.first<<", ";

		if(pr)cout<<entry.first<<", ";
		mapList->key = entry.first;
		mapList->value = entry.second;
		if(pr)cout<<entry.second<<n;
		mapList++;
		//printf("%s %s\n", mapList->key, mapList->value);
	}
	//cout<<mapList[0].key;
	return mapZero;
}

/*
 * The ctypes python-to-c interface cannot use C++ data containers, only C compatible
 * structures and arrays. Below is an outline of the nested structures used.
 * The original python code returned a list of dictionaries and a year
 * [{key:val,key:val},{key:val}], year
 *
 * To reproduce the same, there are three structures and two arrays of those structures.
 * The structures are declared in zzoflaw.h
 * The innermost is an array of Tupple structures, each Tupple contains two char*
 * Each array of Tupple structures is contained in a Tupples structure which also
 * contains the length of the array of Tupples. An array of Tupples structures is
 * contained in the TupplesYear structure which has three fields,
 * the Tupples array, length of the Tupples array, and char* for the year.
 * It can be pictured as:
 *
 * TupplesYear{ Tupples[]{Tupple[]{char* key,char* val},int length }, int length, char* year }
 *
 * The year field is char*, not int nor date because the original python program uses
 * "False" as a default value.
 *
 * In C++ it might look like this:
 *
 * pair<vector<unordered_map<string, string>>,string>
 *
 */

extern "C"
TupplesYear getValsYear(std::vector<std::unordered_map<std::string, std::string>*>  elements, std::string yearStr){

	logger.debug(format("number of maps: {}, yearStr = {}",std::to_string(elements.size()), yearStr)); // @suppress("Invalid arguments")
	int numOfElements = elements.size();
	int tupplesIndex = 0;
	Tupples** tupples = new Tupples*[tupplesIndex];
	Tupples** tupplesPtr = tupples;

	for(std::unordered_map<std::string, std::string>* element: elements){
		tupplesPtr[tupplesIndex] = strMap2tupples(element);
		//tupples[tupplei] = nextTupples;
		tupplesIndex++;
	}
	TupplesYear* valsYear = (TupplesYear*)malloc(sizeof(TupplesYear));
	valsYear->tupples=tupples;
	const char* year = yearStr.c_str();
	valsYear->year=year;
	valsYear->length=numOfElements;
	return *valsYear;
}

#include <clocale>

int main(int argc, char**argv){
	std::setlocale(LC_ALL, "en_US.UTF-8");

	TupplesYear valsyear = getValsYear();

	logger.debug(valsyear.year);
	logger.debug(valsyear.length);

    logger.log(spdlog::source_loc{"/Users/charles/Documents/eclipse-workspace/zzoflaw/legaltexts/fillindata.cpp", 186, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, " [%H:%M:%S %z] [thread %t] %v %s %# ");

    return 1;

}


