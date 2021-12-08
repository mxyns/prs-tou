import subprocess
import sys
import time
import os
import logging
import psutil

from subprocess import Popen, PIPE, STDOUT

def await_for_process(targetname, timeout=0.2) :

    print(f"waiting for {targetname}")
    stop = False
    while stop == False :
        for proc in psutil.process_iter():
            pname = proc.as_dict()["name"]
            if targetname in pname :
                stop = True
    print(f"found {targetname}")
            
    time.sleep(timeout)

if __name__ == "__main__":
    bashCommand = "mkdir logsclient"
    process = Popen(bashCommand.split(), stdout=PIPE)
    process.wait()
    
    bashCommand = "../situation/"+sys.argv[1]+" 0.0.0.0 2000 lorem_1M 0"
    for i in range (int(sys.argv[2])*5):

        await_for_process("xserver", 0.05)
        time.sleep(.05)
        with open("logsclient/stdout{}.txt".format(i), "wb") as out, open("logsclient/stderr{}.txt".format(i),"wb") as err:
            process = subprocess.Popen(bashCommand.split(),stdout=out, stderr=out)
            process.wait()