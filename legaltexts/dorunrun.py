from runmakecsv3 import runDMC
from dataheader import tendata
from dataheader import hundreddata
from dataheader import thousanddata
from dataheader import oneseventysix
from utils.timerecorder import Trex as t

runDMC(thousanddata)
t.prRex()
print(t)
t.prNice()
t.prDetails()