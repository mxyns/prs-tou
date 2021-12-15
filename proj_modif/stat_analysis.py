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
    for files in sorted(os.listdir(directory)):
        res_reg = re.search("([0-9]+)_([0-9]+)_([0-9]+)", files);
        print(res_reg)
        print(res_reg.groups())
        batch_index = int(res_reg.group(3))
        print(batch_index)

        value = get_value(directory+'/'+files,regex)
        print(value)
        print(float(value))
        row[batch_index%num_tests] = float(value)
        print(row)
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


def parse_logs_single (directory, regex, num_tests):
    f = open(sys.argv[1], 'w')
    writer = csv.writer(f)
    test=0;
    row = [None]*(num_tests+1)
    xs=list()
    means=list()
    for files in sorted(os.listdir(directory)):
        value = get_value(directory+'/'+files,regex)
        row[test%num_tests] = float(value)
        if ((test%num_tests) == (num_tests-1)) :
            row[num_tests] = statistics.mean(row[0:num_tests])
            writer.writerow(row)
            means.append(row[num_tests])
            xs.append((test + 1)/num_tests)

        #print(value)
        test += 1
    f.close()
    return xs, means

if __name__ == "__main__":
    #get_value("logs/logs2/stdout0_1.txt",'(?:double estimated_throughput = )([\d\.]+)')

    xsingle, ysingle = parse_logs_single('logs/logs8', '(?:double estimated_throughput = )([\d\.]+)', 5)
    print(xsingle)
    print(ysingle)
    graph.graph(xsingle, ysingle)

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
