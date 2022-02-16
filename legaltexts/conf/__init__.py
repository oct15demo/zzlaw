import os

dir_path = os.path.abspath(os.path.dirname(__file__))

# dir_path is imported where the path to a configuration file is needed
# Example below is for using logging.conf in a utils/logging.py file
#
# import os
# import logging
#
# import conf
# LOGGING_CONFIG = 'logging.conf'
#
# log_path = os.path.join(conf.dir_path, LOGGING_CONFIG)
# logging.config.fileConfig(log_path)
#
# more info here: https://stackoverflow.com/questions/40416072/reading-file-using-relative-path-in-python-project\