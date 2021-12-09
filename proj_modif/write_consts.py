import subprocess
import sys
import os
import logging
import string
import copy

from pathlib import Path
from functools import partial

from subprocess import Popen, PIPE, STDOUT

BATCH_SIZE=5

def readFile(filename):
    with open(filename) as f:
        lines = f.readlines()

    return(lines)

def writeFile(filename, lines, parameters):
    file = open(filename, "w")

    file.write("#ifndef TOU_CONSTS_H\n#define TOU_CONSTS_H\n")
    for i in range(len(lines)) :
        file.write(lines[i].rstrip("\n")+str(parameters[i])+"\n")
    
    file.write("\n#endif")

def log_subprocess_output(pipe):
    for line in iter(pipe.readline, b''): # b'\n'-separated lines
        logging.info('got line from subprocess: %r', line)

def launch_bash_command(command):
    print(command)
    process = Popen(command.split())
    process.wait()


def build_project(iter_n) :

    print(f"building #{iter_n}")
    bashCommand = "make -j{} -s -C ./build".format(8)
    process = Popen(bashCommand.split())
    process.wait()

def run_batch(n, parameter_index, batch_index, logdst=None) :

    for k in range(n) :
        print(f"running #{batch_index} run {k}")

        dst = logdst
        if dst is None : 
            dst = "logs/logs{}/stdout{}_{}.txt".format(parameter_index,batch_index,k)
        else :
            dst = logdst(batch=k)

        Path(dst).parent.mkdir(parents=True, exist_ok=True)

        with open(dst,"wb") as out:
            bashCommand = "./build/src/xserver"
            process = subprocess.Popen(bashCommand.split(),stdout=out)
            exitcode = process.wait()


def run_config(lines, parameters, batch_index=-1, parameter_index=-1, logdst=None, dst_dir="./src", dst_file="tou_consts.h"):
    toFile = os.path.join(dst_dir,dst_file)
    writeFile(toFile, lines, parameters)

    build_project(batch_index)
    print("\n".join([lines[i] + str(parameters[i]) for i in range(len(lines))]))
    run_batch(BATCH_SIZE, parameter_index, batch_index, logdst=logdst)


def int_range_param(parameters, lines, parameter_index) : 
    for batch_index in range(int(sys.argv[2])) :
        parameters[parameter_index] = str(int(sys.argv[3])+(batch_index*int(sys.argv[4])))
        
        toFile = os.path.join('./src','tou_consts.h')
        writeFile (toFile, lines, parameters)
        
        build_project(batch_index)
        run_batch(BATCH_SIZE, parameter_index, batch_index)


def retransmit_param(paramters, lines, parameter_index) :
    options = [
        "tou_retransmit_n(conn, {})",
        "tou_retransmit_all(conn)",
        "tou_retransmit_expired(conn)",
        "tou_retransmit_id(conn, dropped_id)",
        "tou_retransmit_pkt(conn, expired_pkt)"
    ]

    option_id = int(sys.argv[2])
    if len([x[1] for x in string.Formatter().parse(options[option_id]) if x[1] is not None]) : # if has format
        batch_count = int(sys.argv[3])
        initial_value = int(sys.argv[4])
        step_size = int(sys.argv[5])

        for batch_index in range(batch_count) :
            parameters[parameter_index] = options[option_id].format(initial_value+(batch_index*step_size))

            toFile = os.path.join('./src','tou_consts.h')
            writeFile (toFile, lines, parameters)
            
            build_project(batch_index)
            print(parameters[parameter_index])
            run_batch(BATCH_SIZE, parameter_index, batch_index)

    else :
        parameters[parameter_index] = options[option_id]
        toFile = os.path.join('./src','tou_consts.h')
        writeFile (toFile, lines, parameters)
        
        build_project(0)
        print(parameters[parameter_index])
        run_batch(BATCH_SIZE, parameter_index, 0)



if __name__ == "__main__":
    lines = readFile("tou_consts.h")
    parameters = readFile("parameters.txt")

    if (str(os.path.exists('logs')) == 'False') : 
        bashCommand = "rm -r -d -f logs/ ; rm -r -d -f logsclient/ ; mkdir logs"
        launch_bash_command(bashCommand)

    if len(sys.argv) > 1 :
        print("running arg parse mode")

        funcmap = dict()
        funcmap[-1] = int_range_param
        funcmap[10] = retransmit_param

        for i in range(len(parameters)) :
            if (i == int(sys.argv[1])) :

                if (str(os.path.exists('logs/logs{}'.format(i))) == 'False') : 
                    bashCommand = "mkdir -p logs/logs{}".format(i)
                    launch_bash_command(bashCommand)

                            
                func = funcmap.get(i) if funcmap.get(i) is not None else funcmap.get(-1)
                print(f"func : {func.__name__}")
                func(parameters, lines, i)
    else :
        print("running prog mode")

        configs = list()

        window_id = 2
        retransmit_id = 10

        # make configs
        for window in range(0, 100 + 1, 30) :
            window = max(1, 100 - window) # make <= 0 be 1 instead
            for retransmit in range(0, 100 + 1, 30) :
                # retransmit = max(1, retransmit) # make <= 0 be 1 instead
                conf = copy.deepcopy(parameters)

                conf[window_id] = window
                conf[retransmit_id] = "tou_retransmit_n(conn, {})".format(retransmit)

                configs.append(((window, retransmit), conf))
        
        for batch_index in range(len(configs)) : 
            coord = configs[batch_index][0]
            config = configs[batch_index][1]
            print(f"running config : \n{str(config)}")
            run_config( \
                lines, \
                config, \
                batch_index=batch_index, \
                logdst=partial("logs/logs{logid}/stdout_{x}_{y}_{batch}.txt".format, \
                    logid=f"{window_id}_{retransmit_id}", \
                    x=coord[0], \
                    y=coord[1] \
                )
            )


