// define EXIT_FILE to override setting, for use of
// if(EXIT)exit(EXIT_FAILURE);
#ifndef EXIT_FILE
#define EXIT true==false
#endif

#ifndef ZZOFLAW_H_
#define ZZOFLAW_H_

// if(EXIT)exit(EXIT_FAILURE); - Interrupt program by exiting due to preceding error.
// Could even be added to logging.error macro


#ifndef strtok_r
#define strtok_r(str, delim, saveptr) strtok_s(str, delim, saveptr)
#endif

#include <errno.h>
using namespace std;

typedef  unordered_map<const char*, const char*> HashMap;
typedef struct tuppleware{const char* key; const char* value;} Tupple;

typedef struct mapYear {HashMap* valuesMap; string year;} MapYear;
typedef struct tupplesYear {struct tuppleware* tupples; int length; const char* year;} TupplesYear;

const char n = '\n';
const int pr = 0;

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

TupplesYear fillInForReal(char* IE_file, Tupple* values, int tupplelength, char* txt_file,bool trace=false);

//utils
bool fileExists(char* filepath);
off_t fileSize(const char* filename);
size_t castOffSize(off_t offsize, const char* source);
typedef struct FileInput {int size; const char* filename; unsigned char* fileptr;} FileIn;
FileInput* readFile(char* filename, FileIn* fileInput, bool addRoot=false);
void addRootElement(const char* root_start, const char* root_end, int len_start, int len_end, unsigned char* dataptr, int bytes_read);
char* process_line(char* file_read);

int footwo();

#endif /* ZZOFLAW_H_ */
