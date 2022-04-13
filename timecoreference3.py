__author__ = 'halley and meyers'
import xml.etree.ElementTree as ET
import os
import get_elements
import wol_utilities
import math
from wol_utilities import *
from makecsv3 import *
from wol_utilities import standardize
from utils import logger
from utils.timerecorder import Trex as t
from ctypes import *
from utils.switch import c_openTXT

prev_so_file = "fakefileread.so" #previous test, now unused
so_file = "textfileread.so"
fileReadInC = CDLL(so_file)

class TxtLoc(Structure):
    _fields_ = ("word", c_char_p),("position", c_int)

class TxtLocationArray(Structure):
    _fields_ = ('elements', c_short),('STRUCT_ARRAY', POINTER(TxtLoc))

logger = logger.getLogger(__file__)

## global_citations = {}
## global_versions = {}
global_citation_id = 0
dummy_file_id = 0

## filename_to_global_citation_dict = {}

#make a line cleaner
def cleanup(line):
    new_chars = []
    for character in line:
        if character.isalnum() or (character in '.,;:/?><!~`+-_=&!\\@$%^&*()[]{}|\n "') or (character in "'"):
            ## without the parens, python often gets scope of operators wrong
            ## but why remove non-ascii?
            new_chars.append(character)
    return ''.join(new_chars)

#get xml tree
def getXML(attribs, text, tag):
    if text == None:
        text = ''
    string = '<' + tag + ' '
    for key, value in attribs.items():
        string += key + '="' + wol_escape(str(value)) + '" '
    string += '>' + wol_escape(text) + '</' + tag + '>'
    return string

def increment_version(number):
    if type(number) in [float,int]:
        new_number = round((number+.001),3)
    elif type(number) == str:
        new_number = round((float(number)+.001),3)
    else:
        logger.debug('number in increment_version is type'+ str(type(number)))
        raise(Exception)
    return(str(new_number))

def increment_dummy_file(info_dict):
    global dummy_file_id
    global global_citation_id
    previous_id1 = False
    if ('lookup_key' in info_dict) and (info_dict['lookup_key'] in global_citations):
        previous_entry = global_citations[info_dict['lookup_key']]
        for key in ['id1','citation_global_level_id']:
            if key == 'citation_global_level_id':
                if (key in info_dict) and (info_dict[key] != previous_entry[key]) and (info_dict[key] in global_versions):
                    global_versions.pop(info_dict[key])
            if key in previous_entry:
                info_dict[key]=previous_entry[key]
        if 'id1' in info_dict:
            previous_id1 = True
    elif 'alternative_keys' in info_dict:
        for key in info_dict['alternative_keys']:
            if key in global_citations:
                alt_info = global_citations[key]
                if (not previous_id1) and ('id1' in alt_info):
                    previous_id1 = True
                    info_dict['id1']= alt_info['id1']
                    for key2 in ['no_current_file']:
                        if key2 in alt_info:
                            info_dict[key2] = alt_info[key2]
                    global_version = increment_version(list_of_globals[-1])
                    if ('citation_global_level_id' in alt_info) and (alt_info['citation_global_level_id'] in global_versions):
                        global_versions.pop(alt_info['citation_global_level_id'])
                        alt_info['citation_global_level_id'] = str(global_version)
                        global_versions[global_version] = alt_info
                    break
    if not previous_id1:                    
        dummy_file_id += 1
        new_id1 = 'Dummy_File_'+str(dummy_file_id)
        info_dict['id1']=new_id1
        if not 'citation_global_level_id' in info_dict:
            global_version = increment_version(global_citation_id)
            filename_to_global_citation_dict[new_id1] = [str(global_citation_id),global_version]
            info_dict['citation_global_level_id'] = str(global_version)
            info_dict['no_current_file']=True
            global_versions[global_version]=info_dict
            global_citation_id = str(int(global_citation_id) + 1)
        if 'lookup_key' in info_dict:
            global_citations[info_dict['lookup_key']] = info_dict

    
def probably_coreferential_citations(citation_attributes,citation_lookup):
    for key in ['id1','lookup_key']:
        if (key in citation_attributes) and (key in citation_lookup) and (citation_attributes[key]==citation_lookup[key]):
            return(True)
    if ('alternative_keys' in citation_attributes) and ('lookup_key' in citation_lookup):
        if citation_lookup['lookup_key'] in citation_attributes['alternative_keys']:
            return(True)

def update_global_citations(info_dict,id1=False,trace=False):
    ## logger.debug(1,info_dict)
    if id1:
        if ('id1' in info_dict) and (info_dict['id1']==id1) and \
          ('citation_global_level_id' in info_dict) and \
          (info_dict['citation_global_level_id'] in filename_to_global_citation_dict[id1]):
            pass
        else:
            info_dict['id1']=id1
            if 'lookup_key' in info_dict:
                lookup_key = info_dict['lookup_key']
                old_info_dict = global_citations[lookup_key]
            else:
                lookup_key = False
            if 'citation_global_level_id' in old_info_dict:
                old_global_version = old_info_dict['citation_global_level_id']
            else:
                old_global_version = False
            if trace and old_global_version:
                logger.debug('Warning: Changing Coreference properties of:' + str(lookup_key))
                logger.debug('Previous global id:' + str(old_global_version))
            new_dict = info_dict.copy()
            list_of_globals = filename_to_global_citation_dict[info_dict['id1']]
            global_version = increment_version(list_of_globals[-1])
            if trace:
                logger.debug('new global id:' + str(global_version))
            if 'no_current_file' in info_dict:
                new_dict['no_current_file']=True
            if trace and old_global_version:
                input('pause 1')
            new_dict['citation_global_level_id'] = str(global_version)
            global_citations[lookup_key] = new_dict
            global_versions[global_version] = new_dict
            if old_global_version and (old_global_version in global_versions):
                global_versions.pop(old_global_version)
    else:
        if 'lookup_key' in info_dict:
            lookup_key = info_dict['lookup_key']
        else:
            lookup_key = False
        if 'citation_global_level_id' in info_dict:
            global_id = info_dict['citation_global_level_id']
        else:
            global_id = False
        if not ((global_id and (global_id in global_versions)) \
                or (lookup_key and (lookup_key in global_citations))): 
            if global_id:
                global_versions[global_id] = info_dict
            if lookup_key:
                global_citations[lookup_key] = info_dict
        else:
            if not global_id in global_versions:
                global_versions[global_id] = info_dict
            if lookup_key and (not lookup_key in global_citations):
                global_citations[lookup_key] = info_dict
            if ((not global_id) or (global_id not in global_versions)) and trace:
                logger.debug(info_dict)
            entry = global_versions[global_id]
            if ('entry_type' in info_dict):
                entry_type = info_dict['entry_type']
            else:
                entry_type = False
            for key in ['global_id','id1', 'party1', 'party2', 'party1_short','party2_short','case_name', 'standard_reporter','volume',\
                    'page_number','lookup_key','entry_type','citation_global_level_id','no_current_file']:
                if (key in info_dict) and (not key in entry):
                    entry[key]=info_dict[key]
        if 'alternative_keys' in info_dict:
            list_of_globals = filename_to_global_citation_dict[info_dict['id1']]
            alternative_key_copy =  info_dict['alternative_keys'][:]
            alternative_key_copy.append(lookup_key)
            id1 = info_dict['id1']
            if 'no_current_file' in info_dict:
                no_current_file = True
            uncovered_keys = []
            new_id1 = False
            ignore_new_id1 = False
            file_anchored_coref = []
            for key in info_dict['alternative_keys']:
                if (not key in global_citations) or (not 'id1' in global_citations[key]):
                    new_dict = {}
                    new_dict['lookup_key'] = key
                    new_dict['alternative_keys'] = []
                    for alt_key in alternative_key_copy:
                        if alt_key != key:
                            new_dict['alternative_keys'].append(alt_key)
                    if (not id1) and ('id1' in info_dict):
                        id1 = info_dict['id1']
                    new_dict['id1']=id1
                    if 'no_current_file' in info_dict:
                        new_dict['no_current_file']=True                        
                    global_version = increment_version(list_of_globals[-1])
                    list_of_globals.append(global_version)
                    if ('citation_global_level_id' in new_dict) and \
                      (new_dict['citation_global_level_id'] != global_version):
                        old_version = new_dict['citation_global_level_id']
                        if old_version in global_versions:
                            global_versions.pop(old_version)
                    new_dict['citation_global_level_id'] = str(global_version)
                    global_citations[key] = new_dict
                    global_versions[global_version] = new_dict
                elif (not 'no_current_file' in global_citations[key]) and (global_citations[key]['id1'] != id1):
                    if new_id1 and (new_id1 !=global_citations[key]['id1']):
                        ignore_new_id1 = True
                    elif not new_id1:
                        new_id1 = global_citations[key]['id1']
                    file_anchored_coref.append(key)
                elif not global_citations[key]['id1'] == id1:
                    new_dict = global_citations[key]
                    if not 'alternative_keys' in new_dict:
                        new_dict['alternative_keys']=[]
                    for alt_key in new_dict['alternative_keys']:
                        if (not alt_key in alternative_key_copy) and (not alt_key in uncovered_keys):
                            uncovered_keys.append(alt_key)
                            ## these would keys that would need to be covered next if we
                            ## were to make this change recursive ## **57**                            
                    for alt_key in alternative_key_copy:
                        if (alt_key != key) and (not alt_key in new_dict['alternative_keys']):
                            new_dict['alternative_keys'].append(alt_key)
                    next_version = increment_version(list_of_globals[-1])
                    list_of_globals.append(next_version)
                    if ('citation_global_level_id' in new_dict) and (new_dict['citation_global_level_id'] in global_versions):
                        global_versions.pop(new_dict['citation_global_level_id'])
                    new_dict['citation_global_level_id']=next_version
                    new_dict['id1']=id1
                    for k in ['no_current_file']:
                        if k in info_dict:
                            new_dict[k]=info_dict[k]
                    global_versions[next_version]=new_dict
                    global_citations[key]=new_dict
            if len(uncovered_keys)>0:
                if trace:
                    logger.debug('Warning possibly uncovered coreferential items')
                    logger.debug('1' + str(info_dict))
                    logger.debug(uncovered_keys)
                    input('pause 2')
            if (len(file_anchored_coref)>0):
                if trace:
                    logger.debug('Warning file_anchored_coref -- probably an error')
                    logger.debug('1' + str(info_dict))
                    logger.debug('2' + str(file_anchored_coref))
                    input('pause 3')

def add_global_citation(info_dict,trace=False,matching_citations=False):
    global global_citations
    global global_versions
    global global_citation_id
    if 'lookup_key' in info_dict:
        lookup_key = info_dict['lookup_key']
    else:
        lookup_key = False
    added_id1 = False
    if 'id1' in info_dict:
        id1 = info_dict['id1']
    else:
        id1 = False
    if lookup_key and (lookup_key in global_citations) and ('citation_global_level_id' in global_citations[lookup_key]):
        previous_entry = global_citations[lookup_key]
        if ('citation_global_level_id' in info_dict) and (info_dict['citation_global_level_id'] != previous_entry['citation_global_level_id']) \
          and (info_dict['citation_global_level_id'] in global_versions):
            global_versions.pop(info_dict['citation_global_level_id'])
        info_dict['citation_global_level_id'] = previous_entry['citation_global_level_id']
        update_global_citations(info_dict,trace=trace)
        if ('id1' in previous_entry) and (not 'id1' in info_dict):
            id1 = previous_entry['id1']
            info_dict['id1']=previous_entry['id1']
        added_id1 = True
    elif id1:
        if not info_dict['id1'] in filename_to_global_citation_dict:
            global_version = increment_version(global_citation_id)
            filename_to_global_citation_dict[info_dict['id1']] = [global_citation_id,global_version]
        else:
            list_of_globals = filename_to_global_citation_dict[info_dict['id1']]
            global_version = increment_version(list_of_globals[-1])
            filename_to_global_citation_dict[info_dict['id1']].append(global_version)
        if ('citation_global_level_id' in info_dict) and (info_dict['citation_global_level_id'] != global_version) \
          and (info_dict['citation_global_level_id'] in global_versions):
            global_versions.pop(info_dict['citation_global_level_id'])
        info_dict['citation_global_level_id'] = global_version
        update_global_citations(info_dict,trace=trace)
    elif 'alternative_keys' in info_dict:
        mergeable = []
        merging_problem = False
        problems = []
        orphans = []
        alt_id1s = []
        for key in info_dict['alternative_keys']:
            if key in global_citations:
                if not 'id1' in global_citations[key]:
                    global_citations[key]['no_current_file']=True
                if ('no_current_file' in global_citations[key]):
                    mergeable.append(global_citations[key])
                    if ('id1' in global_citations[key]) and (not global_citations[key]['id1'] in alt_id1s):
                        alt_id1s.append(global_citations[key]['id1'])
                elif (not id1) and ('id1' in  global_citations[key]):
                    id1 = global_citations[key]['id1']
                    mergeable.append(global_citations[key])
                elif ('id1' in global_citations[key]) and (global_citations[key]['id1'] == id1):
                    mergeable.append(global_citations[key])
                else:
                    merging_problem = True
                    problems.append(global_citations[key])
            else:
                ## this is a coreferential citation that has not been processed yet
                ## we will process it now, but do a Dyanmic programming thing
                ## so we don't reprocess it later
                orphans.append(key)
        if len(problems)>0:
            if trace:
                logger.debug('coreference merging issue')
                logger.debug('current_info:'  + str(info_dict))
                logger.debug('matching items')
                for fnd in len(mergeable):
                    logger.debug(fnd+1,mergeable[fnd])
                logger.debug('problems')
                for prob in len(problems):
                    logger.debug(str(prob+1) + str(problems[prob]))
                input('pause 4')
        if (not id1) and (len(alt_id1s)>0):
            id1=alt_id1s[0]
        elif not id1:
            increment_dummy_file(info_dict)
            id1 = info_dict['id1']
            added_id1 = True
            update_global_citations(info_dict,trace=trace)
        for merge_item in mergeable:
            abc = merge_item.copy()
            update_global_citations(merge_item,id1=id1,trace=trace)
        if id1 and (not id1 in info_dict):
            info_dict['id1']=id1
        added_id1 = True
        if 'citation_global_level_id' in info_dict:
            old_global = info_dict['citation_global_level_id']
        else:
            old_global = False
        list_of_globals = filename_to_global_citation_dict[id1]
        global_version = increment_version(list_of_globals[-1])
        filename_to_global_citation_dict[info_dict['id1']].append(global_version)
        info_dict['citation_global_level_id'] = str(global_version)
        if old_global and global_version and (old_global in global_versions):
            global_versions.pop(old_global)
        global_versions[global_version] = info_dict
        update_global_citations(info_dict,trace=trace)
    else:
        increment_dummy_file(info_dict)
        added_id1 = True
        update_global_citations(info_dict,trace=trace)
    if matching_citations and (len(matching_citations)>0):
        ## these should only be here if they have no ID1
        ## or global_ids (yet)
        for matching_citation in matching_citations:
            for key in ['id1','no_current_file']:
                if (key in info_dict) and (not key in matching_citation):
                    matching_citation[key]=info_dict[key]
    if added_id1 and id1:
        return(id1)
    
def getCitation(xml, local_citation_dict,linked_citations, document_level_id, global_level_id, trees, local_names,trace=False):
    ## based on id_citations.py
    global global_citations
    global filename_to_global_citation_dict
    citation_attributes = xml.attrib
    citation_text = xml.text
    new_matching_citations = []
    local_id = citation_attributes['id']
    if 'id1' in citation_attributes:
        id1 = citation_attributes['id1']
    else:
        id1 = False
    if 'lookup_key' in citation_attributes:
        local_lookup = citation_attributes['lookup_key']
    else:
        local_lookup = False
    if local_lookup and (local_lookup in global_citations):
        previous_entry = global_citations[local_lookup]
        for key in ['id1','citation_global_level_id','no_current_file']:
            if [key in previous_entry] and not [key in citation_attributes]:
                citation_attributes[key] = previous_entry[key]
    if local_id in linked_citations:
        for matching_id in linked_citations[local_id][:]:
            ignore_item = False
            ## loop thru a copy of the list (so removing items has no effect on loop)
            if not matching_id in local_citation_dict:
                if trace:
                    logger.debug(matching_id)
                    logger.debug(citation_attributes)
                if trace:
                    input('pause 5')
                matching_citation_attributes = False
            else:
                matching_citation_attributes = local_citation_dict[matching_id]
            if matching_citation_attributes and \
              ('lookup_key' in matching_citation_attributes):
                matching_lookup = matching_citation_attributes['lookup_key']
            else:
                matching_lookup = False
            if local_lookup and matching_lookup:
                if ('alternative_keys' in citation_attributes):
                    if (not matching_lookup in citation_attributes['alternative_keys']):
                        citation_attributes['alternative_keys'].append(matching_lookup)
                else:
                    citation_attributes['alternative_keys'] = [matching_lookup]
                if 'alternative_keys' in matching_citation_attributes:
                    if not local_lookup in matching_citation_attributes['alternative_keys']:
                        matching_citation_attributes['alternative_keys'].append(local_lookup)
                else:
                    matching_citation_attributes['alternative_keys'] = [local_lookup]
            if not(matching_citation_attributes):
                ignore_item = True
            elif id1 and ('no_current_file' in matching_citation_attributes):
                matching_citation_attributes['id1']=id1
            elif id1 and ('id1' in matching_citation_attributes) and (id1 != matching_citation_attributes['id1']):
                linked_citations[local_id].remove(matching_id)
                ignore_item = True
            elif ('id1' in matching_citation_attributes) and (not id1):
                citation_attributes['id1'] = matching_citation_attributes['id1']
            else:
                new_matching_citations.append(citation_attributes)
            if not ignore_item:
                if trace and (not 'entry_type' in citation_attributes):
                    logger.debug(citation_attributes)
                entry_type = citation_attributes['entry_type']
                for feature in ['party1','party1_short','party2','party2_short','name','standard_reporter','volume','page_number']:
                    if (feature in matching_citation_attributes) and key_compatible_with_entry(entry_type,feature) and \
                      ((not feature in citation_attributes) or citation_attributes[feature] in ['None','Unknown']):
                        citation_attributes[feature] = matching_citation_attributes[feature]
                if 'lookup_key' in matching_citation_attributes:
                    alt_lookup = matching_citation_attributes['lookup_key']
                    if 'alternative_keys' in citation_attributes:
                        if not alt_lookup in citation_attributes['alternative_keys']:
                            citation_attributes['alternative_keys'].append(alt_lookup)
                    else:
                        citation_attributes['alternative_keys'] = [alt_lookup]
    found = False    
    coreferential_citations = []
    coreferential_local_ids = []
    coreferential_global_ids = []
    for local_id in local_citation_dict:
        citation_lookup = local_citation_dict[local_id]
        if probably_coreferential_citations(citation_attributes,citation_lookup):
            coreferential_citations.append(local_id)
            for key in ['id1','party1_local_name_id','party2_local_name_id']:
                if (not key in citation_attributes) and (key in citation_lookup):
                    citation_attributes[key] = citation_lookup[key]
            if ('citation_local_level_id' in citation_lookup) and (not citation_lookup['citation_local_level_id'] in coreferential_local_ids):
                coreferential_local_ids.append(citation_lookup['citation_local_level_id'])
            if 'citation_global_level_id' in citation_lookup  and (not citation_lookup['citation_global_level_id'] in coreferential_global_ids):
                coreferential_global_ids.append(citation_lookup['citation_local_level_id'])
    #get global id
    global_version = False
    if not 'lookup_key' in citation_attributes:
        if trace:
            logger.debug('no lookup_key')
            logger.debug(citation_attributes)
        if trace:
            input('pause 6')
    elif citation_attributes['lookup_key'] in global_citations:
        previous_entry = global_citations[citation_attributes['lookup_key']]
        for key in ['id1','citation_global_level_id']:
            if (key in previous_entry) and (not key in citation_attributes):
                citation_attributes[key] = previous_entry[key]
    if ('citation_global_level_id' not in citation_attributes) and ('lookup_key' in citation_attributes) and (citation_attributes['lookup_key'] in global_citations):
        if not 'citation_global_level_id' in (global_citations[citation_attributes['lookup_key']]):
            add_global_citation(citation_attributes)
        global_version = global_citations[citation_attributes['lookup_key']]['citation_global_level_id']
        if ('citation_global_level_id' in citation_attributes) and (citation_attributes['citation_global_level_id'] != global_version) and \
          (citation_attributes['citation_global_level_id'] in global_versions):
            global_versions.pop(citation_attributes['citation_global_level_id'])
        citation_attributes['citation_global_level_id'] = global_version
        update_global_citations(citation_attributes,trace=trace)
    else:
        new_id1 = add_global_citation(citation_attributes,trace=trace,matching_citations=new_matching_citations)
        if new_id1 and (not 'id1' in citation_attributes):
            citation_attributes['id1']=new_id1
    if 'citation_local_level_id' not in citation_attributes:
        citation_attributes['citation_local_level_id'] = document_level_id
    return citation_attributes

# def read_in_csv_file(csv_file,trace=False):
#     global global_citations
#     global global_versions
#     global global_citation_id
#     global filename_to_global_citation_dict
#     global_citations.clear()
#     global_citation_id = 0
#     global_citation_id_version = 0
#     with open(csv_file) as f_global_citations:
#         csv_lines = [i[:-1] for i in f_global_citations.readlines()]
#     first_line = csv_lines[0].strip().split('\t')
#     #get the rest from the csv file
#     for i in range(1, len(csv_lines)):
#         info = csv_lines[i].split('\t')
#         ## (standard_reporter, volume, page_number)
#         info_dict = {}        
#         for j in range(0, len(first_line)):
#             info_dict[first_line[j]] = info[j].strip()
#         if  info_dict['id1'] in filename_to_global_citation_dict:
#             global_citation_id_version = increment_version(filename_to_global_citation_dict[info_dict['id1']][-1])
#             info_dict['citation_global_level_id']=global_citation_id_version
#         else:
#             filename_to_global_citation_dict[info_dict['id1']] = [global_citation_id]
#             global_citation_id_version = increment_version(global_citation_id)
#             global_citation_id = global_citation_id+1
#             info_dict['citation_global_level_id']=global_citation_id_version
#         filename_to_global_citation_dict[info_dict['id1']].append(global_citation_id_version)
#         if 'lookup_key' in info_dict:
#             global_key = info_dict['lookup_key']
#             abc = info_dict.copy()
#             update_global_citations(info_dict,trace=trace)

def read_in_csv_file(csv_file,outfile_prefix,trace=False):
    global global_citations
    global global_versions
    global global_citation_id
    global filename_to_global_citation_dict
    global_versions,filename_to_global_citation_dict,global_citations,party_key_hash = create_global_shelves(outfile_prefix)
    ## global_citations.clear()
    global_citation_id = str(0)
    global_citation_id_version = str(0)
    with open(csv_file) as f_global_citations:
        csv_lines = [i[:-1] for i in f_global_citations.readlines()]
    first_line = csv_lines[0].strip().split('\t')
    #get the rest from the csv file
    for i in range(1, len(csv_lines)):
        info = csv_lines[i].split('\t')
        ## (standard_reporter, volume, page_number)
        info_dict = {}        
        for j in range(0, len(first_line)):
            info_dict[first_line[j]] = info[j].strip()
        if  info_dict['id1'] in filename_to_global_citation_dict:
            global_citation_id_version = increment_version(filename_to_global_citation_dict[info_dict['id1']][-1])
            info_dict['citation_global_level_id']=str(global_citation_id_version)
        else:
            filename_to_global_citation_dict[info_dict['id1']] = [global_citation_id]
            global_citation_id_version = increment_version(global_citation_id)
            global_citation_id = str(int(global_citation_id)+1)
            info_dict['citation_global_level_id']=global_citation_id_version
        filename_to_global_citation_dict[info_dict['id1']].append(global_citation_id_version)
        if 'lookup_key' in info_dict:
            global_key = info_dict['lookup_key']
            abc = info_dict.copy()
            update_global_citations(info_dict,trace=trace)

def write_global_IE_file(triples):
    global global_versions
    for out_file,xml_list,local_keys in triples:
        ## ignore local_keys
        with open(out_file,'w', encoding='utf-8') as outstream:
            all_str = ''
            for tree in xml_list:
                info = tree[0]
                if ('lookup_key' in info) and (info['lookup_key'] in global_citations):
                    global_info = global_citations[info['lookup_key']]
                    for key in ['id1','citation_global_level_id','no_current_file','alternative_keys']:
                        if (key in global_info) and ((not key in info) or (global_info[key] != info[key])):
                            info[key] = global_info[key]
                all_str += getXML(*tree) + '\n\n'
            outstream.write(all_str)

def write_global_citations(f_global_citations, trace=False):
    global global_versions
    global_id_list = list(global_versions.keys())
    global_id_list.sort()
    for global_id in global_id_list:
        citation = global_versions[global_id]
        new_str = ''
        new_str += str(citation['citation_global_level_id']) + '\t'
        if 'id1' in citation:
            new_str += str(citation['id1']) + '\t'
        else:
            new_str += 'None\t'
        if 'lookup_key' in citation:
            new_str +=str(citation['lookup_key']) + '\t'
        else:
            new_str += 'None\t'
        if 'entry_type' in citation:
            new_str +=str(citation['entry_type']) + '\t'
        else:
            new_str += 'None\t'
        if ('party1_local_name_id' in citation) and ('party2_local_name_id' in citation):
            if (not citation['party1_local_name_id'] in local_names) or (not citation['party2_local_name_id'] in local_names):
                if trace:
                    logger.debug('party1_local or party2_local not in local_names')
                    logger.debug(local_names)
                    logger.debug(TXT_file)
                    logger.debug(citation)
                    input('pause 8')
            else:
                new_str += ';'.join(local_names[citation['party1_local_name_id']]) + '\t' + ';'.join(local_names[citation['party2_local_name_id']]) + '\t'
        elif ('party1_local_name_id' in citation):
            if (not citation['party1_local_name_id'] in local_names):
                if trace:
                    logger.debug('party1_local not in local_names')
                    logger.debug(local_names)
                    logger.debug(TXT_file)
                    logger.debug(citation)
                    input('pause 9')
            else:
                new_str +=';'.join(local_names[citation['party1_local_name_id']]) + '\t'+'None\t'

        else:
            new_str += 'None\tNone\t'
        if 'party1' in citation:
            new_str += citation['party1']+'\t'
        else:
            new_str += 'None\t'
        if 'party2' in citation:
            new_str += citation['party2']+'\t'
        else:
            new_str += 'None\t'
        if 'party1_short' in citation:
            new_str += citation['party1_short']+'\t'
        elif 'party1' in citation:
            new_str += get_last_name_from_party(citation['party1']).upper()+'\t'
        else:
            new_str += 'None\t'
        if 'party2_short' in citation:
            new_str += citation['party2_short']+'\t'
        elif 'party2' in citation:
            new_str += get_last_name_from_party(citation['party2']).upper()+'\t'
        else:
            new_str += 'None\t'
        if 'name' in citation:
            new_str += wol_utilities.standardize(citation['name'])
            if 'alternative_case_names' in citation:
                for name in citation['alternative_case_names'].split(';'):
                    new_str += ';'+wol_utilities.standardize(name)
            new_str += '\t'
        else:
            new_str += 'None\t'
        if 'standard_reporter' in citation:
            new_str += citation['standard_reporter'] + '\t'
            new_str += citation['volume'] + '\t'
            if 'page_number' in citation:
                new_str += citation['page_number'] + '\t'
            else:
                new_str += 'None\t'
        else:
            new_str += 'None\tNone\tNone'
        new_str += '\n'
        f_global_citations.write(new_str)

def make_citation_graph_string(OUT_file,local_keys):
    ## lookup "current" id based on
    ## lookup key
    short_f = short_file(OUT_file)
    if short_f in filename_to_global_citation_dict:
        citation_graph_str = str(filename_to_global_citation_dict[short_f][0]) + ': '
        out_list = []
        for local_key in local_keys:
            if local_key in global_citations:
                citation = global_citations[local_key]
                if 'citation_global_level_id' in citation:
                    if not citation['citation_global_level_id'] in out_list:
                        out_list.append(citation['citation_global_level_id'])
        out_list.sort()
        for number in out_list:
            citation_graph_str += str(number) + ' '
        citation_graph_str += '\n'
        return(citation_graph_str)
    else:
        logger.debug('No citation graph for' + str(short_f))
        return('')
                           
def make_IE_out_file(TXT_file,IE_file,OUT_file,processed_f,processed,trace=False):
    ## bad_file = '695247' ## '104079'
    ## bad_file = '462626'
    ## ill-formed file: '3208272' (removed)
    bad_file = False
    global global_citations
    global global_citation_id
    global filename_to_global_citation_dict
    #logger.warn(TXT_file)
    
    # This block is the python version of creating the word location tuple list
    # These are the words in .txt files and their locations
    # The "while c_openTXT" c code version follows below 
    t.start('openTXT')
    while not c_openTXT:
        with open(TXT_file,encoding='utf-8') as instream:
            f_line = instream.read()
            words = [['',0]]
            for j in range(0, len(f_line)):
                if f_line[j] == ' ' or f_line[j] == '\n':
                    words.append(['', j + 1])
                else:
                    words[-1][0] += f_line[j]
                    #get all the words in the text (used later)
            words = list(filter(lambda i: len(i[0]) > 0, words))
        break
        print(str(words))


    #This block is the test of c code with dummy data
    while False:
        getGold = fileReadInC.getGoldilocs2
        getGold.restype = POINTER(TxtLoc)
        wordLocations = getGold()
    
        getLen = fileReadInC.getNumofwords2
        getGold.restype = c_int
        lenLocs = getLen()
        
        print(str(wordLocations))
        print(wordLocations.__dict__)
        for i in range(0,lenLocs):
            print(wordLocations[i].word.decode("utf-8"), wordLocations[i].position)
        break
    
    # This block calls c functions in textfileread.so
    # The while loop used for control and also for test purposes
    # A "while not c_openTXT:" block runs the python alternative
    # c_openTXT is set in utils/switch.py used as a pseudo config file 
    # Using 'while' vs 'if' allows inserting break to skip print diagnostics
    # when using logging and log levels would involve extra work for comparing
    # output (eg switch format to exclude timestamp and line number and create
    # custom log format and level for only this code block, if possible).
    while c_openTXT: 

        getGold = fileReadInC.getWordLocations

        getGold.restype = POINTER(TxtLoc)

        words = getGold(b'scotus/100000.txt')


        getLen = fileReadInC.getNumberOfWords
        getGold.restype = c_int
        lenLocs = getLen()
        break
        print(str(words))
        print(words.__dict__)

        for i in range(0,lenLocs):
            #https://stackoverflow.com/questions/53062552/unicodedecodeerror-utf-8-codec-cant-decode-byte-0xe2-in-position-1023-unexp
            print(words[i].word.decode('utf-8', 'ignore'), words[i].position)
        
        #break
    t.end('openTXT')
    
    all_str = ''
    logger.debug(IE_file)
    with open(IE_file, 'r', encoding='utf-8') as f:
        lines = [i[:-1] for i in f.readlines()]
    local_citations = []
    document_level_citation_id = 0
    local_names = {0:[]}
    other_names = []
    name_names = []
    trees = []
    processed_trees = []
    citation_names = []
    bad_lines = []
    #get a parsing of all trees
    local_citation_dict = {}
    ## linked_citations.clear()
    linked_citations = {}
    t.start('lines')
    for line in lines:
        try:
            xml_line = ET.fromstring(line) ## .replace('&', 'and'))
            trees.append(xml_line)
            if xml_line.tag == 'citation':
                local_id = xml_line.attrib['id']
                if bad_file and local_id.startswith(bad_file):
                    logger.debug(xml_line.attrib)
                    logger.debug('1-->' + str(end=''))
                # if local_id.startswith(bad_file):
                #     logger.debug('1-->',end='')
                # if local_id.startswith(bad_file):
                #     keyword_pairs = generate_key_words_from_case_record(xml_line.attrib,trace=True)
                if bad_file and local_id.startswith(bad_file):
                    logger.debug(xml_line.attrib)
                    logger.debug('1.1-->'  + str(end=''))
                    keyword_pairs = generate_key_words_from_case_record(xml_line.attrib,trace=True)
                else:
                    keyword_pairs = generate_key_words_from_case_record(xml_line.attrib)
                if bad_file and local_id.startswith(bad_file):
                    logger.debug(xml_line.attrib)
                    logger.debug('2-->'  + str(end=''))
                if len(keyword_pairs)==1:
                    xml_line.attrib['lookup_key']=keyword_pairs[0][1]
                    xml_line.attrib['entry_type']=keyword_pairs[0][0]
                    if not keyword_pairs[0][1] in global_citations:
                        global_citations[keyword_pairs[0][1]]=xml_line.attrib
                elif trace: 
                ## elif trace or True:
                    logger.debug(xml_line.attrib)
                    logger.debug(xml_line.text)
                    logger.debug('There should be exactly one keyword, but there are:'  + str(len(keywords)))
                    logger.debug('Keywords:'  + str(keywords))
                    input('pause 10')
                if bad_file and local_id.startswith(bad_file):
                    logger.debug(3)
                local_citation_dict[local_id]=xml_line.attrib
                if bad_file and local_id.startswith(bad_file):
                    logger.debug(3.1)
            elif (xml_line.tag == 'RELATION') and ('standard_case' in xml_line.attrib) and \
                (('X_vs_Y' in xml_line.attrib) or ('case_citation_other' in xml_line.attrib)):
                if bad_file and local_id.startswith(bad_file):
                    logger.debug('D'  + str(end=''))
                if 'standard_case' in xml_line.attrib:
                    standard = xml_line.attrib['standard_case']
                else:
                    standard = False
                if 'X_vs_Y' in xml_line.attrib:
                    X_v_Y = xml_line.attrib['X_vs_Y']
                else:
                    X_v_Y = False
                if 'case_citation_other' in xml_line.attrib:
                    other = xml_line.attrib['case_citation_other']
                else:
                    other = False
                if standard:
                    for coref in [other,X_v_Y]:
                        if coref:
                            if standard in linked_citations:
                                linked_citations[standard].append(coref)
                            else:
                                linked_citations[standard]=[coref]
                if X_v_Y:
                    for coref in [standard,other]:
                        if coref:
                            if X_v_Y in linked_citations:
                                linked_citations[X_v_Y].append(coref)
                            else:
                                linked_citations[X_v_Y]=[coref]
                if other:
                    for coref in [standard,X_v_Y]:
                        if coref:
                            if other in linked_citations:
                                linked_citations[other].append(coref)
                            else:
                                linked_citations[other]=[coref]
        except:
            # if local_id.startswith(bad_file):
            #     logger.debug(4)
            bad_lines.append(line)
            logger.debug('bad:'  + str(xml_line.attrib))
    t.end('lines')
    if (len(bad_lines)>0):
        logger.debug('Bad Lines found in file: '  + str(short_file(TXT_file)))
        if trace or (len(bad_lines) > 0):
            for line in bad_lines:
                logger.debug(line)
            if (len(bad_lines)>0):
                logger.debug('bad file: '  + str(TXT_file))
    for value_set in linked_citations.values():
        for local_id in value_set[:]:
            for local_id2 in value_set:
                if (local_id != local_id2) and (not local_id2 in linked_citations[local_id]):
                    linked_citations[local_id].append(local_id2)
        ## add one level of transtive relations:
        ## if Y is equiv to X and Z, mark X and Z as equiv to each other
    t.start('xml_trees')
    for xml_line in trees:
            if xml_line.tag == 'citation': #process citations
                if (not xml_line.attrib['id'] in local_citation_dict):
                    logger.debug('missing from local_citation_dict')
                    logger.debug(xml_line.attrib)
                    if trace:
                        input('pause 12')
                    local_citation_dict[xml_line.attrib['id']] = xml_line.attrib
                    ## fudge fix Aug 8, 2020 AM
                xml_element = getCitation(xml_line, local_citation_dict,linked_citations, document_level_citation_id, global_citation_id, trees, local_names,trace=trace)
                local_citations.append(xml_element)
                if xml_element['citation_local_level_id'] == document_level_citation_id: ## string or integer
                    document_level_citation_id += 1
                ## Note: party1_local_name_id and party2_local_name_id are not currently being handled
                ##       but this might be a future thing to do ** 57 ***
                if ('party1_local_name_id' in xml_element) and ('party1' in xml_element):
                    if xml_element['party1_local_name_id'] not in local_names: #add the name id to local_names
                        local_names[xml_element['party1_local_name_id']] = [wol_utilities.standardize(xml_element['party1'])]
                    else: #add this version of the name to the database
                        if wol_utilities.standardize(xml_element['party1']) not in local_names[xml_element['party1_local_name_id']]:
                            local_names[xml_element['party1_local_name_id']].append(wol_utilities.standardize(xml_element['party1']))
                    other_names.append((wol_utilities.standardize(xml_element['party1']), xml_element['party1_local_name_id'], 'from_citation'))
                    del xml_element['party1']
                if ('party2_local_name_id' in xml_element) and ('party2' in xml_element):
                    if xml_element['party2_local_name_id'] not in local_names:
                        local_names[xml_element['party2_local_name_id']] = [wol_utilities.standardize(xml_element['party2'])]
                    else:
                        if wol_utilities.standardize(xml_element['party2']) not in local_names[xml_element['party2_local_name_id']]:
                            local_names[xml_element['party2_local_name_id']].append(wol_utilities.standardize(xml_element['party2']))
                    other_names.append((wol_utilities.standardize(xml_element['party2']), xml_element['party2_local_name_id'], 'from_citation'))
                    del xml_element['party2']
                #also add alternate party names to list
                if 'alt_party1' in xml_element:
                    if wol_utilities.standardize(xml_element['alt_party1']) not in local_names[xml_element['party1_local_name_id']]:
                        local_names[xml_element['party1_local_name_id']].append(wol_utilities.standardize(xml_element['alt_party1']))
                    del xml_element['alt_party1']
                if 'alt_party2' in xml_element:
                    if wol_utilities.standardize(xml_element['alt_party2']) not in local_names[xml_element['party2_local_name_id']]:
                        local_names[xml_element['party2_local_name_id']].append(wol_utilities.standardize(xml_element['alt_party2']))
                    del xml_element['alt_party2']
                if (not 'party2' in xml_element) and (not 'alt_party2' in xml_element) and (not 'party2_local_name_id' in xml_element) and ('name' in xml_element):
                    if (xml_element['name'] in local_names) and (not wol_utilities.standardize(xml_element['name']) in local_names):
                        local_names[xml_element['name']].append(wol_utilities.standardize(xml_element['name']))
            elif xml_line.tag == 'NAME': #process names and add to local_names database
                xml_element = get_elements.getName(xml_line, local_citations, local_names)
                if xml_element['local_name_id'] not in local_names:
                    local_names[xml_element['local_name_id']] = [wol_utilities.standardize(xml_element['name'])]
                else:
                    if wol_utilities.standardize(xml_element['name']) not in local_names[xml_element['local_name_id']]:
                        local_names[xml_element['local_name_id']].append(wol_utilities.standardize(xml_element['name']))
                del xml_element['name']
                name_names.append(xml_element['local_name_id']) #used later to see what names are not tied to a party or profession
            elif xml_line.tag == 'LEGAL_ROLE': #process legal role
                xml_element = get_elements.getLegalRoleElement(words, trees, xml_line, local_names)
                if 'local_name_id' in xml_element:
                    if xml_element['local_name_id'] not in local_names:
                        local_names[xml_element['local_name_id']] = [wol_utilities.standardize(xml_element['name'])]
                    else:
                        if wol_utilities.standardize(xml_element['name']) not in local_names[xml_element['local_name_id']]:
                            local_names[xml_element['local_name_id']].append(wol_utilities.standardize(xml_element['name']))
                    other_names.append((wol_utilities.standardize(xml_element['name']), xml_element['local_name_id'], 'from_legal_role'))
            elif xml_line.tag == 'PROFESSION': #process professions
                xml_element = get_elements.getProfessionElement(trees, xml_line, local_names)
                if 'local_name_id' in xml_element:
                    if xml_element['local_name_id'] not in local_names:
                        local_names[xml_element['local_name_id']] = [wol_utilities.standardize(xml_element['party'])]
                    else:
                        if wol_utilities.standardize(xml_element['party']) not in local_names[xml_element['local_name_id']]:
                            local_names[xml_element['local_name_id']].append(wol_utilities.standardize(xml_element['party']))
                    other_names.append((wol_utilities.standardize(xml_element['name']), xml_element['local_name_id'], 'from_profession'))
            elif xml_line.tag == 'docket': #process docket element
                xml_element = get_elements.getDocketElement(trees, xml_line, local_citations)
            elif xml_line.tag == 'date': #process date element
                xml_element = get_elements.getDateElement(trees, xml_line, local_citations)
            else:
                xml_element = xml_line.attrib
            xml_text = ' '.join(xml_line.text.split()) if xml_line.text != None else '' #get text of new xml tree
            processed_trees.append((xml_element, xml_text, xml_line.tag.upper()))
    #prepare processed trees
    t.end('xml_trees')
    for processed_tree in processed_trees:
        if processed_tree[-1] == 'NAME':
            processed_tree[0]['all_names'] = '\t'.join(local_names[processed_tree[0]['local_name_id']])
        elif processed_tree[-1] == 'citation':
            if 'id1' in processed_tree[0]:
                if not processed_tree[0]['id1'].isdigit():
                    del processed_tree[0]['id1']
            if 'party1_local_name_id' in processed_tree[0]:
                processed_tree[0]['party1'] = local_names[processed_tree[0]['party1_local_name_id']][0]
            if 'party2_local_name_id' in processed_tree[0]:
                processed_tree[0]['party2'] = local_names[processed_tree[0]['party2_local_name_id']][0]

    #add to processed_trees the names not used in citations/professions
    other_names = filter(lambda i: i[1] not in name_names, other_names)
    for other_name in other_names:
        processed_trees.append(({'name':other_name[0], 'local_name_id':other_name[1], 'from':other_name[2]}, other_name[0], 'NAME'))
    if processed_f:
        processed_f.write(OUT_file + '\n') #confirm in processed_f that this file has been processed
    #write to the citation graph file
    processed_xml = []
    local_keys = []
    for triple in processed_trees:
        processed_xml = triple[0]
        if 'lookup_key' in processed_xml:
            local_keys.append(processed_xml['lookup_key'])
            ## currently for citations only
    output = [OUT_file,processed_trees,local_keys]
    return(output)

# def run_global_coreference(input_directories,root_directory,out_graph,global_csv_file,csv_file,txt_file_type='.txt',IE_infile_type='.case10',IE_out_file_type='.NYU_IE4',processed_file_list='processed.txt',initialize_csv=False,trace=False):
#     global global_citations
#     global global_citation_id
#     global processed_f
#     global filename_to_global_citation_dict
#     global party_key_hash
#     ## party_key_hash.clear()
#     ## global_citations.clear()
#     ## global_versions.clear()
#     ## filename_to_global_citation_dict.clear()
#     global_citation_id = 0
#     bad_lines = []
#     processed_files = []
#     if processed_file_list and os.path.isfile(processed_file_list):
#         with open(processed_file_list,'r') as processed_f:
#             processed = list(processed_f.read().split('\n'))
#     else:
#         processed = []
#     if processed_file_list:
#         processed_f = open(processed_file_list,'w')
#     else:
#         processed_f = False
#     ## either use an existing csv file or make
#     ## one base on the same files
#     if not (os.path.isfile(csv_file)) or initialize_csv:
#         create_csv_file(csv_file,root_directory,input_directories,IE_infile_type,trace=trace)
#     read_in_csv_file(csv_file,trace=trace)
#     ## input('pause for complete csv load')
#     ## logger.debug(global_citations)
#     with open(out_graph,'w') as f_citation_graph,open(global_csv_file,'w') as f_global_citations:
#         title_list = ['global_id','id1','lookup_key','entry_type','party1_id', 'party2_id', 'party1','party2','party1_short','party2_short','case_name', 'standard_reporter','volume','page_number']
#         title_string = title_list[0]
#         for title in title_list[1:]:
#             title_string = title_string+'\t'+title
#         title_string = title_string + '\n'
#         f_global_citations.write(title_string)
#         ## first line listing what columns do
#         output_tuples = []
#         for indir in input_directories:
#             full_dir =file_name_append(root_directory,indir)
#             logger.debug('processing',indir,'for graph info')
#             for infile in os.listdir(full_dir):
#                 if infile.endswith(IE_infile_type):
#                     if trace:
#                         logger.debug(infile)
#                     base_file = infile[:(-1*len(IE_infile_type))]
#                     IE_file = file_name_append(full_dir,infile)
#                     TXT_file = file_name_append(full_dir,base_file+txt_file_type)
#                     OUT_file = file_name_append(full_dir,base_file+IE_out_file_type)
#                     if not short_file(IE_file) in processed:
#                         output_tuples.append(make_IE_out_file(TXT_file,IE_file,OUT_file,processed_f,processed,trace=trace))
#                         processed.append(short_file(IE_file))
#         logger.debug('making graph')
#         for out_file,xml_list,local_keys in output_tuples:
#             ## ignoring xml_list
#             citation_graph_str=make_citation_graph_string(out_file,local_keys)
#             f_citation_graph.write(citation_graph_str)
#         logger.debug('writing global files')
#         write_global_citations(f_global_citations)
#     write_global_IE_file(output_tuples)
#     if processed_f:
#         processed_f.close()

def run_global_coreference2(input_directories,outfile_prefix,txt_file_type='.txt',IE_infile_type='.case10',IE_out_file_type='.NYU_IE4',processed_file_list='processed.txt',initialize_csv=True,trace=False):
    t.start('run_global_coref')
    t.start('before read')
    global global_citations
    global global_citation_id
    global processed_f
    global filename_to_global_citation_dict
    global party_key_hash
    out_graph = outfile_prefix + '.citation_graph'
    global_table_file = outfile_prefix + '_global_table_file.tsv'
    initial_table_file = outfile_prefix + '_initial_table_file.tsv'
    ## party_key_hash.clear()
    ## global_citations.clear()
    ## global_versions.clear()
    ## filename_to_global_citation_dict.clear()
    global_citation_id = str(0)
    bad_lines = []
    processed_files = []
    t.end('before read')
    if processed_file_list and os.path.isfile(processed_file_list):
        t.start('processed_f.read')
        with open(processed_file_list,'r') as processed_f:
            processed = list(processed_f.read().split('\n'))
        logger.warn(t.end('processed_f.read'))
    else:
        processed = []
    if processed_file_list:
        processed_f = open(processed_file_list,'w')
    else:
        processed_f = False
    ## either use an existing csv file or make
    ## one base on the same files
    t.start('create_csv')
    if not (os.path.isfile(initial_table_file)) or initialize_csv:
        create_csv_file(initial_table_file,input_directories,IE_infile_type,outfile_prefix,trace=trace)
        logger.debug('finish creating initial csv file')
    t.end('create_csv')
    t.start('read_csv_file')
    read_in_csv_file(initial_table_file,outfile_prefix,trace=trace)
    t.end('read_csv_file')
    ## input('pause for complete csv load')
    ## logger.debug(global_citations)
    with open(out_graph,'w') as f_citation_graph,open(global_table_file,'w', encoding='utf-8') as f_global_citations:
        title_list = ['global_id','id1','lookup_key','entry_type','party1_id', 'party2_id', 'party1','party2','party1_short','party2_short','case_name', 'standard_reporter','volume','page_number']
        title_string = title_list[0]
        for title in title_list[1:]:
            title_string = title_string+'\t'+title
        title_string = title_string + '\n'
        f_global_citations.write(title_string)
        ## first line listing what columns do
        output_tuples = []
        logger.debug('making graph')
        for indir in input_directories:
            logger.debug('directory:' + str(indir))
            full_dir =indir ## file_name_append(root_directory,indir)
            for infile in os.listdir(full_dir):
                if infile.endswith(IE_infile_type):
                    ## logger.debug(infile)
                    base_file = infile[:(-1*len(IE_infile_type))]
                    IE_file = file_name_append(full_dir,infile)
                    TXT_file = file_name_append(full_dir,base_file+txt_file_type)
                    OUT_file = file_name_append(full_dir,base_file+IE_out_file_type)
                    if not short_file(IE_file) in processed:
                        t.start('make_IE_out_file')
                        #output_tuples.append(make_IE_out_file(TXT_file,IE_file,OUT_file,processed_f,processed,trace=trace))
                        newtuple=make_IE_out_file(TXT_file,IE_file,OUT_file,processed_f,processed,trace=trace)
                        t.end('make_IE_out_file')
                        t.start('append_IE_out_file')
                        output_tuples.append(newtuple)
                        t.end('append_IE_out_file')
                        processed.append(short_file(IE_file))
                    else:
                        logger.warn(IE_file)
        t.start('write cit graph strs')
        for out_file,xml_list,local_keys in output_tuples:
            ## ignoring xml_list
            citation_graph_str=make_citation_graph_string(out_file,local_keys)
            f_citation_graph.write(citation_graph_str)
        t.end('write cit graph strs')
        t.start('write glob cits')
        write_global_citations(f_global_citations)
        t.end('write glob cits')
    t.start('write IE file')
    write_global_IE_file(output_tuples)
    t.end('write IE file')
    if processed_f:
        processed_f.close()
    t.end('run_global_coref',False)
    t.prRex()
    print(t)
    t.prNice()
    t.prDetails()

 
