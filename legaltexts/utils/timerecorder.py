'''
Created on Dec 8, 2021

@author: charles
'''
import time
from collections import defaultdict
import utils.switch as c_py_switch
import sys


"""
Wraps defaultdict to profile lines within a function
Useful for long methods, line profiling, and aggregate profile 
of looped lines or lines executed from repeated method calls. In 
this context, profiling is only measurement of time (no mem or cpu).

Example:
import time
from timerecorder import Trex as t

t("rex1")
time.sleep(4)
t.end("rex1")
t.prRex()

"""
    
# Time records (Trex)
class Trex:
    
    # skipTags,  do not print because times appeared insignificant
    skipTags = {'before read','processed_f.read','read_in_csv_file','write cit graph strs','write glob cits'}
    recTimes = defaultdict(float)
    recStarts = {}
    recCount = defaultdict(int)
    cumTotal = 0.0
    # TypedDict from python 3.8, not used here
    # backport of TypedDict, but not used https://pypi.org/project/typing-extensions/
    detailTimes = defaultdict(list)
    
    @staticmethod
    def start(name:str):
        Trex.recStarts[name]=time.time()
        
    @staticmethod
    def end(name:str,total=True)->float:
        elapsed = time.time() - Trex.recStarts[name]
        rounded = round(elapsed,5)
        Trex.recTimes[name] += elapsed
        Trex.recCount[name] += 1
        if total:
            Trex.cumTotal += rounded
        Trex.detailTimes[name].append(rounded)
        return rounded
        
    @staticmethod
    def prRex():
        print(Trex.recTimes)
    
    @staticmethod
    def prNice():
        keys = Trex.recTimes.keys()
        for key in keys:
            if key in Trex.skipTags:
                continue
            print(f'{key:<3}{round(Trex.recTimes[key],3):<3}')
        print(Trex.cumTotal)
        
    @staticmethod
    def prDetails():
        keys = Trex.detailTimes.keys()
        for key in keys:
            if key in Trex.skipTags:
                continue
            details=Trex.detailTimes[key]
            details.sort(reverse=True)

            ckey = "c_" + key
            if ckey in c_py_switch.__dict__:
                if c_py_switch.__dict__[ckey]:
                    print("C  ",end="")
                else:
                    print("py ",end="")
            else:
                print("py ",end="")
                

            print(f'{key:<20}{round(sum(details),2):<4}{str(details):<20}')
            

        