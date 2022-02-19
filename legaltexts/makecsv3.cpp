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

// ([{'id1': '100000', 'folder': 'scotus', 'full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES', 'standard_reporter': 'U.S.', 'volume': '259', 'page_number': '188', 'year': '1922'}], '1922')
/*
If using Eclipse, properties, C/C++ Build, Settings, GCC C++ Compiler, Dialect, Language standard, you can select ISO C++11 (-std=c++0x).
This was on a MacAir macOS 10.13 Eclipse Oxygen 3a c++ 4.2.1 clang 900 –
*/
using namespace std;

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

extern "C"
TupplesYear fill_in_data_from_IE_outfile(char* IE_file, Tupple* values, int tupplelength, char* txt_file,bool trace=false){

	HashMap pydict;
	printf("values passed to and printed from c program: \n\n");
	for (int i=0; i<tupplelength; i++){
		printf("     %s : %s \n", values[i].key, values[i].value);
		pydict[values[i].key]=values[i].value;

	}
	printf("\n");
	return getValsYear(pydict);
}




