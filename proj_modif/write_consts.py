import sys
import os
import subprocess
import time

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

if __name__ == "__main__":
    lines = readFile("tou_consts.h")
    parameters = readFile("parameters.txt")
    for i in range(len(parameters)) : 
        if (i == int(sys.argv[1])) :
            for j in range(int(sys.argv[2])) :
                parameters[i]= str((j+1)*int(sys.argv[3]))
                
                toFile = os.path.join('./src','tou_consts.h')
                writeFile (toFile, lines, parameters)

                bashCommand = "make -C ./build"
                process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
                process.wait()
                print("passed once")
