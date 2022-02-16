from makecsv3 import fill_in_data_from_IE_outfile
from utils.timerecorder import Trex as tr

IE_file = "scotus/100000.case10" 
values = {'id1': '100000', 'folder': 'scotus', 'full_name': 'Supreme Court of the United States', 'casename': 'morrisdale coal co v united states', 'party1': 'MORRISDALE COAL CO', 'party2': 'UNITED STATES'} 
txt_file = "scotus/100000.txt"
defaulttuple=(IE_file,values,txt_file)
loop = True

def runDMC(makecsv3params=[defaulttuple]):
    tr.start("fill_IE")
    if loop:
        for _ in range(1):
            for mktuple in makecsv3params:
                valsyear = fill_in_data_from_IE_outfile(mktuple[0], mktuple[1], mktuple[2])
    else:
        for mktuple in makecsv3params:
            valsyear = fill_in_data_from_IE_outfile(mktuple[0], mktuple[1], mktuple[2])
    tr.end("fill_IE")
    
    n='\n'
    print("return values:")
    print(valsyear)
    tr.prNice()
    
def main():
    runDMC()
    
if __name__ == '__main__':
    main()