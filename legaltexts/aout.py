#possible bugs
#1) later_date determination

from ctypes import *
from utils.timerecorder import Trex as tr
import ctypes
from utils import logger
from utils.timerecorder import Trex as t

# Parameters to pass to function fill_in_data_from_IE_outfile
class Entry(Structure):
    _fields_ = ("key", c_char_p),("value", c_char_p)

class TxtLocationArray(Structure):
    _fields_ = ('entries', c_short),('map_array', POINTER(Entry))

# The return structures for function fill_in_data_from_IE_outfile
class XMLValue(Structure):
    _fields_ = [("key", c_char_p),("value", c_char_p)]

# Maps of either values or elements.
# Values are the metadata passed to the function fill_in_data_from_IE_outfile
# which may be modified, appended to, in the function.
# Elements are the attributes of XML citation elements from case10 files
# The maps are return as lists of key value tupples 
class valsOrEls(Structure):
    _fields_ = [("valuesMap", POINTER(XMLValue)), ("mapLength", c_int)]

# A list of maps and a year
class valsElsYear(Structure):
    _fields_ = [("valuesMapVector", POINTER(POINTER(valsOrEls))), ("vectorLength", c_int),("year", c_char_p)]

"""py_print_array = lib.print_array
py_print_array.argtypes = [ctl.ndpointer(np.float64, 
                                         flags='aligned, c_contiguous'), 
                           ctypes.c_int]
A = np.array([1.4,2.6,3.0], dtype=np.float64)"""


c_fillindata = "../Debug/zzoflaw"
makecsv3c = CDLL(c_fillindata)
fill_in_c = makecsv3c.fillInDataFromIEOutfile
fill_in_c.restype =valsElsYear
fill_in_c.argtypes = [c_char_p, POINTER(Entry), c_int, c_char_p]


IE_file = b'scotus/100000.case10'
#IE_file = b'noroot.case10'
#IE_file = b'/Users/charles/root_100000.case10' 
valdict= {'id1':'100000', 'folder':'scotus','full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES'}
vallength = len(valdict)
tupplelist = [Entry(bytes(item[0],'utf-8'),bytes(item[1],'utf-8')) for item in valdict.items()]


values = (Entry * vallength)(*tupplelist,)#unpack, trailing comma makes it a tupple
#values = (Entry * 2)( Entry(b'id1', b'100000'), Entry(b'folder',b'scotus'))#, ('full_name', 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES'] 
txt_file = b'scotus/100000.txt'

t.start("fill_in_c")
values=fill_in_c(IE_file, values, vallength, txt_file)
t.end("fill_in_c")

print("\n           Values returned from c code\n\
____________________________________________________\
\n    Note: Default value for year is 'False'\n")
print(f'{"values.year.decode()":1>20}: {values.year.decode()}')
print(f'{"values.vectorLength":>20}: {values.vectorLength}')

print("\n    Part returned as array of tuple structures:\n")
#print(f'{values.valuesMapVector[0][0].valuesMap[0].key.decode("utf-8"):>20}')
#print(f'{values.valuesMapVector[0][0].valuesMap[0].value.decode("utf-8"):>20}')
#print(f'{values.valuesMapVector[0][0].mapLength:>20}')

# Note:The second subscript to access valuesMap is due valuesMapVector holding an array of pointers to arrays of Tupples.
# See function getValsYear in fillindata.cpp for longer explanation of data structures used.

for i in range(values.vectorLength):
   for j in range(values.valuesMapVector[0][0].mapLength):
       print(f'{values.valuesMapVector[i][0].valuesMap[j].key.decode("utf-8"):>20} : {values.valuesMapVector[i][0].valuesMap[j].value.decode("utf-8"):>20}')

valuesDict={}

print("\nTurned into dictionary:\n",valuesDict)

print("\n")
t.prRex()
print(t)
t.prNice()
t.prDetails()    
