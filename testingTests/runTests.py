# python runTests.py 127.0.0.1 6789 1 adder_32bit

import os
import sys


if 5 != len(sys.argv):
    print "Not enough arguments."
    print "IP Port PartyID circuitName"
    exit()


numTests = 10
protocolNames = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
protocolNum = 2


ipAddress = sys.argv[1]
portNum = sys.argv[2]
partyID = sys.argv[3]
circuitFile = "./circuitFiles/" + sys.argv[4] + ".rtl.circuit"
baseInputFile = "./inputs/" + sys.argv[4]
baseOutputFile = "./TestResults/Output_" + partyID + "_" + protocolNames[protocolNum] + "_"


if 1 == partyID:
	baseInputFile = baseInputFile + ".builder.input."
else:
	baseInputFile = baseInputFile + ".executor.input."



os.chdir('..')
os.system("ls")



for i in range(0, numTests):
	inputFile = baseInputFile + str(i)
	outputFile = baseOutputFile + str(i)
	command = "./a.out " + circuitFile + " " + ipAddress + " " + portNum + " " + inputFile + " " + str(partyID) + " > " + outputFile 

	print command
