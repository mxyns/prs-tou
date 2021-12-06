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

def launch_bash_command(command):
    process = Popen(command.split())
    process.wait()


if __name__ == "__main__":
    lines = readFile("tou_consts.h")
    parameters = readFile("parameters.txt")

    if (str(os.path.exists('logs')) == 'False') : 
        bashCommand = "mkdir logs"
        launch_bash_command(bashCommand)

    for i in range(len(parameters)) : 
        if (i == int(sys.argv[1])) :

            if (str(os.path.exists('logs/logs{}'.format(i))) == 'False') : 
                bashCommand = "mkdir logs/logs{}".format(i)
                launch_bash_command(bashCommand)

            bashCommand = "make -j{} -C ./build".format(8)
            process = Popen(bashCommand.split(), stdout=PIPE)
            process.wait()
            
            for j in range(int(sys.argv[2])) :
                parameters[i]= str(int(sys.argv[3])+(j*int(sys.argv[4])))
                
                toFile = os.path.join('./src','tou_consts.h')
                writeFile (toFile, lines, parameters)

                for k in range(5) :
                    with open("logs/logs{}/stdout{}_{}.txt".format(i,j,k),"wb") as out:
                        bashCommand = "./build/src/xserver"
                        process = subprocess.Popen(bashCommand.split(),stdout=out)
                        exitcode = process.wait()