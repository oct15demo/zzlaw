from ctypes import *
from utils.timerecorder import Trex as tr
import ctypes

# Parameters to pass to function
class Entry(Structure):
    _fields_ = ("key", c_char_p),("value", c_char_p)

class TxtLocationArray(Structure):
    _fields_ = ('entries', c_short),('map_array', POINTER(Entry))

# The return if going to end of function fill_in_data_from_IE_outfile
class XMLValue(Structure):
    _fields_ = [("key", c_char_p),("value", c_char_p)]

class valArrayYear(Structure):
    _fields_ = [("valuesMap", POINTER(XMLValue)), ("mapLength", c_int),("year", c_char_p)]

"""py_print_array = lib.print_array
py_print_array.argtypes = [ctl.ndpointer(np.float64, 
                                         flags='aligned, c_contiguous'), 
                           ctypes.c_int]
A = np.array([1.4,2.6,3.0], dtype=np.float64)"""


c_fillindata = "a.out"
makecsv3c = CDLL(c_fillindata)
fill_in_c = makecsv3c.fill_in_data_from_IE_outfile
fill_in_c.restype =valArrayYear
fill_in_c.argtypes = [c_char_p, POINTER(Entry), c_int, c_char_p]

IE_file = b'scotus/100000.case10' 
valdict= {'id1':'100000', 'folder':'scotus','full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES'}
vallength = len(valdict)
tupplelist = [Entry(bytes(item[0],'utf-8'),bytes(item[1],'utf-8')) for item in valdict.items()]


values = (Entry * vallength)(*tupplelist,)
#values = (Entry * 2)( Entry(b'id1', b'100000'), Entry(b'folder',b'scotus'))#, ('full_name', 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES'] 
txt_file = b'scotus/100000.txt'

values=fill_in_c(IE_file, values, vallength, txt_file)

print("\n           Values returned from c code\n\
____________________________________________________\
\n    Note: Default value for year is 'False'\n")
print(f'{"values.year.decode()":1>20}: {values.year.decode()}')
print(f'{"values.mapLength":>20}: {values.mapLength}')

print("\n    Part returned as array of tuple structures:\n")
for i in range(values.mapLength):
    print(f'{values.valuesMap[i].key.decode("utf-8"):>20}: {values.valuesMap[i].value.decode("utf-8")}')

valuesDict={}
for i in range(values.mapLength):
    valuesDict[values.valuesMap[i].key.decode("utf-8")]= values.valuesMap[i].value.decode("utf-8")

print("\nTurned into dictionary:\n",valuesDict)          
