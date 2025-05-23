#!/usr/bin/python

import subprocess
import json
import os
import sys
import time
import signal
print(sys.argv)
#saveAsOracle = sys.argv.count("--oracle") == 1
saveAsOracle = False

scores = {"compiles":0}
try:
    print("Calling make")
    result = subprocess.check_output('make',stderr=subprocess.STDOUT)
    scores['compiles']=1
    if(result.count(b" warning: ") == 0 and scores['compiles'] == 1):
        scores['compiles'] = 2
    print(result)
    for file in sorted(os.listdir("inputs")):
         if os.path.isfile("inputs/"+file):
             print("Trying input file: " + file)
             p1 = subprocess.Popen(args=["./nodemngr < inputs/"+file], shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,  stdin=subprocess.PIPE, universal_newlines=True)
             try:
                output= p1.communicate(timeout=8)[0]
             except Exception as e:
                print("============>>>>>ERROR Test timed out!<<<<<<=============")
                print("     Make sure to check for End of file (EOF) in your code or your code segfaulted against the test case :(")                
                scores[file] = 0
                continue
             print("Your Output:")
             print(output)
             if(saveAsOracle):
                 with open("outputs/"+file,"wb") as outfile:
                     outfile.write(output)
             else:
                 with open("outputs/"+file,"r") as basefile:
                     goodOutput=basefile.read()
                     if(goodOutput == output):
                         print("============OK!=============")
                         scores[file] = 1
                     else:
                         print("============>>>>>ERROR Expected output was!<<<<<<=============")
                         print(goodOutput)
                         scores[file] = 0
                 print(">Trying again with Valgrind:")
                 p1 = subprocess.Popen(args=["valgrind --leak-check=full ./nodemngr < inputs/"+file], shell=True, stdout=subprocess.PIPE,stderr=subprocess.STDOUT,  stdin=subprocess.PIPE)
                 output= p1.communicate()[0]
                 print("Your Output:")
                 print(output)
                 if output.count(b"LEAK SUMMARY") > 0:
                     print("===========>>>>>ERROR, MEMORY LEAK!<<<<<=============")
                 elif scores[file] == 1:
                     scores[file] = 2
except subprocess.CalledProcessError as e:
    print(e)

print(json.dumps( {"scores": scores} ))
os.system("make clean")
os.system("killall nodemngr")
