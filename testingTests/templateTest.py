import os
import sys


if 5 != len(sys.argv):
    print "Not enough arguments."


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
	baseInputFile = baseInputFile + ".builder.input"
else:
	baseInputFile = baseInputFile + ".executor.input"

# print ipAddress
# print portNum
# print partyID
# print circuitFile
# print baseInputFile

# os.system("ls ../ > temp.txt")



for i in range(0, numTests):
	inputFile = baseInputFile
	outputFile = baseOutputFile + str(i)
	command = "./a.out " + circuitFile + " " + ipAddress + " " + portNum + " " + inputFile + " " + str(partyID) + " > " + outputFile 

	print command
