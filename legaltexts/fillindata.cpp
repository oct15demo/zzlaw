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
If using Eclipse, properties, C/C++ Build, Settings, GCC C++ Compiler, Dialect, Language standard, you can select ISO C++11 (-std=c++0x).
This was on a MacAir macOS 10.13 Eclipse Oxygen 3a c++ 4.2.1 clang 900 –
*/

static spdlog::logger logger = getLog();


typedef std::function<void(std::string)> function_type;

class functionbuf : public std::streambuf {
	private:

	typedef std::streambuf::traits_type traits_type;

    function_type d_function;

    char  d_buffer[1024];

    int overflow(int c) {
        if (!traits_type::eq_int_type(c, traits_type::eof())) {
            *this->pptr() = traits_type::to_char_type(c);
            this->pbump(1);
        }
        return this->sync()? traits_type::not_eof(c): traits_type::eof();
    }

    int sync() {
        if (this->pbase() != this->pptr()) {
            this->d_function(std::string(this->pbase(), this->pptr()));
            this->setp(this->pbase(), this->epptr());
        }
        return 0;
    }

	public:

    functionbuf(function_type const& function): d_function(function) {
        this->setp(this->d_buffer, this->d_buffer + sizeof(this->d_buffer) - 1);
    }
};

class ofunctionstream : private virtual functionbuf, public std::ostream {
	public:

    ofunctionstream(function_type const& function): functionbuf(function), std::ostream(static_cast<std::streambuf*>(this)) {
        this->flags(std::ios_base::unitbuf);
    }
};


/******************************************************
 *
 *                Meat and Potatoes
 *
 ******************************************************/


Tupple* charMap2tupples(std::unordered_map<char*,char*> realMap){

	mapLen = realMap.size();
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

Tupple* strMap2tupples(std::unordered_map<std::string,std::string> realMap){

	mapLen = realMap.size();
	Tupple* mapList = (Tupple*)malloc(mapLen * sizeof(Tupple));
	Tupple* mapZero = mapList;

	for (pair<std::string, std::string> entry: realMap){
		logger.trace(entry.first);
		int flen = (entry.first.length())+1;
		int slen = (entry.second.length())+1;
		char* fptr = (char*)malloc(flen*sizeof(char));
		char* sptr = (char*)malloc(slen*sizeof(char));
		strcpy(fptr,entry.first.c_str());
		strcpy(sptr,entry.second.c_str());
		fptr[flen-1]='\0';
		sptr[slen-1]='\0';
		if(pr)cout<<fptr<<", ";
		mapList->key = fptr;
		mapList->value = sptr;
		logger.trace(mapList[0].key);
		mapList++;
		//printf("%s %s\n", mapList->key, mapList->value);
	}
	//cout<<mapList[0].key;
	return mapZero;
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

extern "C"
Tupple* getVals(){
	if(pr)cout<<"hello from getVals"<<n;
	Tupple* tuppledMap = strMap2tupples(valHash);
	//logger.info("hello from getVals");

	return tuppledMap;
}

extern "C"
TupplesYear getValsYear(std::unordered_map<std::string, std::string>  pydict){
	if(pr)cout<<"hello from getValsYear"<<n;
	Tupple* tuppledMap = strMap2tupples(pydict);
	TupplesYear* valsYear = (TupplesYear*)malloc(sizeof(TupplesYear));
	valsYear->tupples=tuppledMap;
	char* year = (char*)malloc(sizeof(char)*5);
	strncpy(year, "False", 5);
	year[5]='\0';
	if(pr)cout<<year<<n;
	valsYear->year=year;
	valsYear->length=mapLen;
	return *valsYear;
}

void some_function(std::string const& value) {
    std::cout << "some_function(" << value << ")\n";
}

int streamcallsfunc() {
    ofunctionstream out(&some_function);
    out << "hello" << ',' << " world: " << 42 << "\n";
    printf("%s", "calling all cars\n");

    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt> (out);
    auto streamlogger = std::make_shared<spdlog::logger>("my_logger", ostream_sink);
    streamlogger->info("Welcome to spdlog going through out, ostream_sink, some_function!");

    out << std::nounitbuf << "not" << " as " << "many" << " calls\n" << std::flush;
    return 1;
}

/*
template<class _Tp, class ..._Args>
inline _LIBCPP_INLINE_VISIBILITY
typename enable_if
<
    !is_array<_Tp>::value,
    shared_ptr<_Tp>
>::type logger<spdlog::logger>;*/
int foo(){
	logger.debug("hello foo");
	return 0;
}
#include <clocale>

int main(int argc, char**argv){
	std::setlocale(LC_ALL, "en_US.UTF-8");
	//spdlog::logger logger = *spdlog::stdout_color_mt("log");
    ofunctionstream out(&some_function);
    //auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt> (out);
    //logptr = std::make_shared<spdlog::logger>("my_logger", ostream_sink);
    logger.warn("I warned you already");

    //spdlog::set_pattern("[%H:%M:%S %z] [thread %t] %v with the at %@");
    cout << typeid(logger).name() << endl;
	if(pr)cout<<"hello from fillindata main"<<n;
	TupplesYear valsyear = getValsYear();
	Tupple* vals = getVals();
	if(pr){
		printf("%s\n",vals[5].key);
		cout<<valsyear.year<<n;
		cout<<valsyear.length<<n;
	}
	//logger.set_pattern(">>>>>>>>> %H:%M:%S %z %v <<<<<<<<<");
	//logger.set_pattern(">>>>>>>>> [%H:%M:%S %z] [thread %t] %v %s %# <<<<<<<<<");
	//malloc error if left in : spdlog::set_default_logger((std::shared_ptr<spdlog::logger>)&logger);
    SPDLOG_INFO("this work?");
    logger.info("Welcome to spdlog going through out, ostream_sink, some_function!");

    logger.log(spdlog::source_loc{"/Users/charles/Documents/eclipse-workspace/zzoflaw/legaltexts/fillindata.cpp", 186, static_cast<const char *>(__FUNCTION__)}, spdlog::level::info, ">>>>>>>>> [%H:%M:%S %z] [thread %t] %v %s %# <<<<<<<<<");

    foo();
    footwo();
	//string s =fmt::format("malloc file {} size_t {}", "filename", 4 );
	//spdlog::info("/Users/charles/Documents/eclipse-workspace/zzoflaw(199):this is the test of VS format");
    return 1;

}


