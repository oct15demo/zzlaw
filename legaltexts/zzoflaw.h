#ifndef ZZOFLAW_H_
#define ZZOFLAW_H_

using namespace std;

typedef  unordered_map<const char*, const char*> HashMap;
typedef struct tuppleware{const char* key; const char* value;} Tupple;

typedef struct mapYear {HashMap* valuesMap; string year;} MapYear;
typedef struct tupplesYear {struct tuppleware* tupples; int length; const char* year;} TupplesYear;

const char n = '\n';
const int pr = 1;

static int mapLen;

static Tupple tuppledMaps [1];

// see above typedef for HashMap, code here initializes a std::unordered_map
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

Tupple* map2tupples(HashMap realMap);

extern "C"
Tupple* getVals();

extern "C"
TupplesYear getValsYear(HashMap pydict=valHash);

#endif /* ZZOFLAW_H_ */
