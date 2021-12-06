import subprocess
import sys
import time
import os
import logging

from subprocess import Popen, PIPE, STDOUT

if __name__ == "__main__":
    bashCommand = "mkdir logsclient"
    process = Popen(bashCommand.split(), stdout=PIPE)
    process.wait()
    
    bashCommand = "../situation/"+sys.argv[1]+" 10.43.9.17 2000 lorem_10M 0"
    for i in range (int(sys.argv[2])*5):
        with open("logsclient/stdout{}.txt".format(i), "wb") as out, open("logsclient/stderr{}.txt".format(i),"wb") as err:
            process = subprocess.Popen(bashCommand.split(),stdout=out, stderr=out)
            process.wait()
            time.sleep(0.001)