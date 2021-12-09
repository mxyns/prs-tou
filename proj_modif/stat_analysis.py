import subprocess
import sys
import statistics
import os
import csv
import re

from subprocess import Popen, PIPE, STDOUT

import graph_utils as graph

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
    #f = open(sys.argv[1], 'w')
    #writer = csv.writer(f)
    test=0;
    v_dic = dict();
    row = [None]*(num_tests)
    for files in os.listdir(directory):
        res_reg = re.search("([0-9]+)_([0-9]+)_([0-9]+)", files);

        batch_index = int(res_reg.group(3))

        value = get_value(directory+'/'+files,regex)
        row[batch_index%num_tests] = float(value)
        if ((batch_index%num_tests) == (num_tests-1)) :
            x = int(res_reg.group(1))
            y = int(res_reg.group(2))
            v_dic [(x,y)] = statistics.mean(row)
            #writer.writerow(row)
        #print(value)
        #test += 1
    #f.close()
    print(v_dic)
    return(v_dic);


if __name__ == "__main__":
    #get_value("logs/logs2/stdout0_1.txt",'(?:double estimated_throughput = )([\d\.]+)')
    pts_mean_map = parse_logs('logs/logs2_10', '(?:double estimated_throughput = )([\d\.]+)', 5)

    Xs, Ys, data = graph.convert_points_map_to_arrays(pts_mean_map)

    datamax = graph.maxof(Xs, Ys, data)

    for mx in datamax :
        print(mx)

    graph.scaled_surface(Xs, Ys, data)
    
    # list dir
    #   for each file : extract (x, y)
    #       append x to scalex
    #       append y to scaley
    #       map (x, y) to mean
