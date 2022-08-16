from ctypes import *
from utils.timerecorder import Trex as tr
import ctypes

#
# The return value from fill_in_data_from_IE_outfile actually is
# a list of maps and a year.
#
# The python ctypes library doesn't work with C++ containers, only C structures.
# To work around this limitation, the python containers passed as parameters
# to C/C++ functions are converted to C compatible data structures, namely
# structures or arrays of structures. For example, python dictionaries
# are mapped to arrays of a two field structure object, each object holding
# a key and a value. It is in that form that the dictionary is sent to 
# the C++ program. Inside the C++, the array can be converted to a proper
# C++ container like an unordered map. Returning an unordered_map reverses
# the process. The unordered map is turned into an array of key value structures 
# inside the C++, and once back inside python, the resulting list of 
# key value objects can be turned back into a dictionary. Rube Goldberg school of
# code design, yes. 
# 
# Alternatives are use of Cython which uses specialized syntax within 
# python to call C code, or pybind11 which allows C++ container use
# without the manual code transforms cytypes requires as outlined above.
# A survey of these alternatives can be found here:
# http://blog.behnel.de/posts/cython-pybind11-cffi-which-tool-to-choose.html
# Note that cffi has some performance drawbacks absent in ctypes.    
#


# Parameters to pass to C/C++ function

class Entry(Structure):
    _fields_ = ("key", c_char_p),("value", c_char_p)
    
#class TxtLocationArray(Structure):
#    _fields_ = ('entries', c_short),('map_array', POINTER(Entry))

################################################################################# 
# Below are structures returned from the C/C++ method fill_in_data_from_IE_outfile
# called by the python code. It follows from the structures returned by the original
# python version of the method. 
# In python it's a list of dictionaries, and a single field representing a date.
# This becomes in C an array of an array of key value structure objects and a 
# char field to hold the date, since the python field is represented by a 
# string literal "False" by default. It purposefully doesn't use 0 or boolean False, 
# nor date objects, and date strings not necessarily validated and may appear in 
# various formats.
# python method returned [{key:value, key:value},{key:value, key:value}], date_string
# C/C++ method returns
#     struct{char* key, char* value}**, char*           
# which was   
#     vector<undordered_map<string, string> , string    
# in C/C++ before transformed to C only data structures. 

class XMLValue(Structure):
    _fields_ = [("key", c_char_p),("value", c_char_p)]

#This is the array of the key value structure objects 
class mapArray(Stucture):
    _fields_ = ("valuesMap", POINTER(XMLValue) )

class valArrayYear(Structure):
    _fields_ = [("valuesMapArray", POINTER(mapArray)), ("mapLength", c_int),("year", c_char_p)]

"""py_print_array = lib.print_array
py_print_array.argtypes = [ctl.ndpointer(np.float64, 
                                         flags='aligned, c_contiguous'), 
                           ctypes.c_int]
A = np.array([1.4,2.6,3.0], dtype=np.float64)"""


c_fillindata = "../Debug/zzoflaw"
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