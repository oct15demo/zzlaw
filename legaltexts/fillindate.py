from ctypes import *
from utils.timerecorder import Trex as tr
import ctypes
from config import *
#https://stackoverflow.com/questions/4691039/making-xerces-parse-a-string-instead-of-a-file
# https://stackoverflow.com/questions/20773602/returning-struct-from-c-dll-to-python
tr.start("fillin")

fakeFillInData = CDLL(c_fillIn)
 
class XMLValue(Structure):
    _fields_ = [("key", c_char_p),("value", c_char_p)]

class valArrayYear(Structure):
    _fields_ = [("valuesMap", POINTER(XMLValue)), ("mapLength", c_int),("year", c_char_p)]

if False:
    getVals = fakeFillInData.getVals
    getVals.restype = POINTER(XMLValue)
    vals = getVals()

getValsYear = fakeFillInData.getValsYear
getValsYear.restype = valArrayYear

n="\n"

pr = False
if pr:
    print("start vals \n\n")
    for i in range(10000):
        print(vals[i].key.decode("utf-8"), "\t",
              vals[i].value.decode("utf-8"))

for nothing in range(100):
    valsYear = getValsYear() 
    tupvals = valsYear.valuesMap
    if pr:
        print("start embeddedvals \n\n")
    casevals = {}
    if pr:
        print(valsYear.mapLength)
    for i in range(valsYear.mapLength):
        if pr:
            print(tupvals[i].key.decode("utf-8"),tupvals[i].value.decode("utf-8"))
        casevals[tupvals[i].key.decode("utf-8")]=tupvals[i].value.decode("utf-8")
    
    if pr:
        print(casevals)

if False:
    print(casevals)
    print(valsYear.year.decode("utf-8"))
tr.end("fillin")
tr.prNice()

""" 
https://stackoverflow.com/questions/17101845/python-ctypes-array-of-structs

/* https://www.journaldev.com/31907/calling-c-functions-from-python
   $ cc -fPIC -shared -o my_functions.so my_functions.c

   from ctypes import *
   so_file = "/Users/pankaj/my_functions.so"
   my_functions = CDLL(so_file)

   my_functions.square(10)
   100
*/

char* fakeOpenFile(char* fileName){
    return "text from file";
}

https://stackoverflow.com/questions/25840293/ctypes-and-array-of-structs

from ctypes import *

class Particle(Structure):
    _fields_ = [("x", c_double),("y", c_double)]

getp = cdll.x.getParticles
getp.restype = POINTER(Particle)
particles = getp()
for i in range(3):
    print(particles[i].x,particles[i].y)

"""

