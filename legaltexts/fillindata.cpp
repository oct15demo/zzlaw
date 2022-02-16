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

// ([{'id1': '100000', 'folder': 'scotus', 'full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES', 'standard_reporter': 'U.S.', 'volume': '259', 'page_number': '188', 'year': '1922'}], '1922')
/*
If using Eclipse, properties, C/C++ Build, Settings, GCC C++ Compiler, Dialect, Language standard, you can select ISO C++11 (-std=c++0x).
This was on a MacAir macOS 10.13 Eclipse Oxygen 3a c++ 4.2.1 clang 900 –
*/
using namespace std;


typedef  unordered_map<const char*, const char*> HashMap;
typedef struct tuppleware{const char* key; const char* value;} Tupple;

typedef struct mapYear {HashMap* valuesMap; string year;} MapYear;
typedef struct tupplesYear {struct tuppleware* tupples; int length; const char* year;} TupplesYear;

char n = '\n';
int pr = 1;

int mapLen;

Tupple* map2tupples(HashMap realMap){

	mapLen = realMap.size();
	Tupple* mapList = (Tupple*)malloc(mapLen * sizeof(Tupple));
	Tupple* mapZero = mapList;

	for (pair<const char*, const char*> entry: realMap){
		if(pr)cout<<entry.first<<n;
		int flen = (strlen(entry.first))+1;
		int slen = (strlen(entry.second))+1;
		char* fptr = (char*)malloc(flen*sizeof(char));
		char* sptr = (char*)malloc(slen*sizeof(char));
		strcpy(fptr,entry.first);
		strcpy(sptr,entry.second);
		fptr[flen-1]='\0';
		sptr[slen-1]='\0';
		if(pr)cout<<fptr<<n;
		mapList->key = fptr;
		mapList->value = sptr;
		if(pr)cout<<mapList[0].key<<n;
		mapList++;
		//printf("%s %s\n", mapList->key, mapList->value);
	}
	//cout<<mapList[0].key;
	return mapZero;
}

static HashMap valHash = {
		{ "id1", "100000"},
		{ "folder", "scotus"},
		{ "full_name", "Supreme Court of the United States"},
		{ "casename", "morrisdale coal co v united states"},
		{ "party1", "MORRISDALE COAL CO"},
		{ "party2", "UNITED STATES"},
		{ "standard_reporter", "U.S."},
		{ "volume", "259"},
		{ "page_number", "188"},
		{ "year", "1922"}
    };



Tupple tuppledMaps [1];



extern "C"
Tupple* getVals(){
	if(pr)cout<<"hello from getVals"<<n;
	Tupple* tuppledMap = map2tupples(valHash);

	return tuppledMap;
}

extern "C"
TupplesYear getValsYear(){
	if(pr)cout<<"hello from getValsYear"<<n;
	Tupple* tuppledMap = map2tupples(valHash);
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

int main(int argc, char**argv){
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
