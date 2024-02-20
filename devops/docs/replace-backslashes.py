# This script replaces backslashes ("\") to forward slashes ("/") in file pathes in a Doxygen config file (Doxyfile)
# The script uses a parameter as a path to file
# Example of use: 
# python replace_backslashes.py Doxyfile

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
        print("Usage: python replace-backslashes.py <file_to_process>")
        sys.exit(1)

    file_path = sys.argv[1]
    # Construct the absolute path based on the current working directory
    file_path = os.path.abspath(file_path)
    process_file(file_path)
