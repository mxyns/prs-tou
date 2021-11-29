import subprocess
import sys
import os
import logging

from subprocess import Popen, PIPE, STDOUT

def readFile(filename):
    with open(filename) as f:
        lines = f.readlines()

    return(lines)

def writeFile(filename, lines, parameters):
    file = open(filename, "w")

    file.write("#ifndef TOU_CONSTS_H\n#define TOU_CONSTS_H\n")
    for i in range(len(lines)) :
        file.write(lines[i].rstrip("\n")+parameters[i]+"\n")
    
    file.write("\n#endif")

def log_subprocess_output(pipe):
    for line in iter(pipe.readline, b''): # b'\n'-separated lines
        logging.info('got line from subprocess: %r', line)

if __name__ == "__main__":
    lines = readFile("tou_consts.h")
    parameters = readFile("parameters.txt")

    bashCommand = "mkdir logs"
    process = Popen(bashCommand.split(), stdout=PIPE)
    process.wait()

    for i in range(len(parameters)) : 
        if (i == int(sys.argv[1])) :
            bashCommand = "mkdir logs/logs{}".format(i)
            process = Popen(bashCommand.split(), stdout=PIPE)
            process.wait()
            for j in range(int(sys.argv[2])) :
                parameters[i]= str((j+1)*int(sys.argv[3]))
                
                toFile = os.path.join('./src','tou_consts.h')
                writeFile (toFile, lines, parameters)

                bashCommand = "make -C ./build"
                process = Popen(bashCommand.split(), stdout=PIPE)
                process.wait()

                with open("logs/logs{}/stdout{}.txt".format(i,j),"wb") as out, open("logs/logs{}/stderr{}.txt".format(i,j),"wb") as err:
                    bashCommand = "./build/src/xserver"
                    process = subprocess.Popen(bashCommand.split(),stdout=out, stderr=out)
                    exitcode = process.wait()