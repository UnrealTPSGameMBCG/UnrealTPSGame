# This script is designed to replace backslashes ("\") to forward slashes ("/") in file pathes in a Doxygen config file (Doxyfile)
# Back slashes are used in Windows paths. Forward slashes are used in Linux paths. 
# Use this script if you run Doxygen on Linux

# Usage: python replace-backslashes.py <file_to_process>
# Example of usage: 
# python replace_backslashes.py Doxyfile

## Example of usage in GitHub Workflow:
## replace-backslashes.py and Doxyfile are assumed to be in devops/docs
#      - name: Replace back slashes
#        run: |
#          python replace-backslashes.py ${{ env.file_to_process }}
#        env:
#          file_to_process: Doxyfile
#        working-directory: devops/docs

import sys
import os
import re

def process_line(line):
    # Check if the line is a comment
    if line.startswith('#'):
        return line
    
    # Replace "\" with "/" except in specified cases
    line = re.sub(r'(?<!\\)\\(?!["\s]|$)', '/', line)
    line = re.sub(r'(?<=\S)\\(?=\s|$)', '/', line)
    return line

def process_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    with open(file_path, 'w') as file:
        for line in lines:
            file.write(process_line(line))

if __name__ == "__main__": 
    if len(sys.argv) != 2: 
        print("Expected usage: python replace-backslashes.py <file_to_process>")
        sys.exit(1)

    file_path = sys.argv[1]
    # Construct the absolute path based on the current working directory
    file_path = os.path.abspath(file_path)
    process_file(file_path)
