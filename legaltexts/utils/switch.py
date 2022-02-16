import sys

# timerecorder tests for existence, so not brittle? 
c_openTXT=False
c_lines=False
c_xml_trees=False

def main():
    current_module = sys.modules[__name__]
    print(str(current_module.__dict__))
    if 'c_openTXT' in current_module.__dict__:
        print("here")
    
if __name__ == '__main__':
    main() 
