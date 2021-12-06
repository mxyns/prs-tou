import subprocess
import sys
import statistics
import os
import csv
import re

from subprocess import Popen, PIPE, STDOUT

def get_value(filename, regex):
    with open(filename) as f:
        lines = f.readlines()
        length = len(lines)

        for i in range (length-30, length):
            match = re.search(regex, lines[i])
            if (match is not None):
                #print(str(match.group(1)))
                return_value = str(match.group(1))

    return(return_value)
    
def parse_logs (directory, regex, num_tests):
    f = open(sys.argv[1], 'w')
    writer = csv.writer(f)
    test=0;
    row = [None]*(num_tests+1)
    for files in os.listdir(directory):
        value = get_value(directory+'/'+files,regex)
        row[test%num_tests] = float(value)
        if ((test%num_tests) == (num_tests-1)) :
            row[num_tests] = statistics.mean(row[0:num_tests])
            writer.writerow(row)
        #print(value)
        test += 1
    f.close()

if __name__ == "__main__":
    #get_value("logs/logs2/stdout0_1.txt",'(?:double estimated_throughput = )([\d\.]+)')
    parse_logs('logs/logs2', '(?:double estimated_throughput = )([\d\.]+)', 5)