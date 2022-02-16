'''
Created on Feb 28, 2020

@author: charles
'''

import logging.config
import os

# conf is a directory created to keep configuration files
# Logging requires configuration, this setup using a file conf/logging.conf
# The import statement below imports the conf/__init__.py file, used to locate logging.conf

import conf

LOGGING_CONFIG = 'logging.conf'   # Indirection in case you want to switch later

log_path = os.path.join(conf.dir_path, LOGGING_CONFIG)

logging.config.fileConfig(log_path)

def getLogger(name):
    '''
    Wrapper around logging.getLogger to format for IDE source link
    
    Transforms variable __file__  passed to getLogger from
       /a/b/c.py
    to
       c.py 
    needed for source link in IDE
    
    Import this file, then call this getLogger wrapper with
    
       logger = logger.getLogger(__file__)
       
    Use as you would a regular logger
    
       logger.debug('Hello debug logging')
    
    :param string file name passed using __file__
    :return: python logger
    '''
    
    name = os.path.basename(name)
    return logging.getLogger(name)

# ------ Some Test Code ------ #

logger = getLogger(__file__)

if __name__ == '__main__':
    logger.debug("Test link to source")

# Example configuration provided below.
# If using that logging configuration, test output will appear as    
# 2020-06-30 11:45:44,895 | DEBUG | logger.py | <module> | Test link to source            | "logger.py:52"    
# logger.py:50 will appear as a link to line 52 where the debug statement appears, in Eclipse with pydev or Pycharm
# Further discussion here of logging with source link can be found here: 
# https://stackoverflow.com/questions/42457296/pydev-source-file-hyperlinks-in-eclipse-console/57779755#57779755    
'''
# Example of logging configuration file
# format must include 
# "%(name)s:%(lineno)s"
# as that is what provides the source link in Eclipse pydev or PyCharm


[loggers]
keys=root

[handlers]
keys=consoleHandler

[formatters]
keys=customFormatter

[logger_root]
level=DEBUG
handlers=consoleHandler

[handler_consoleHandler]
level=DEBUG
class=StreamHandler
formatter=customFormatter
args=(sys.stdout,)

[formatter_customFormatter]
format=%(asctime)s | %(levelname)s | %(name)s | %(funcName)s | %(message)-30s | "%(name)s:%(lineno)s"
'''