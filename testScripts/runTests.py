import os
import sys



def runTests(ipAddress, portNum, partyID, protocolNum, circuitName, f):
	circuitFile = "./circuitFiles/" + circuitName + ".rtl.circuit"
	baseInputFile = "./inputs/RandomTestInputs/" + circuitName
	baseOutputFile = "./TestResults/Output_" + circuitName + "_" + str(partyID) + "_" + protocolNames[protocolNum] + "_"


	if '1' == partyID:
		baseInputFile = baseInputFile + ".builder.input."
	else:
		baseInputFile = baseInputFile + ".executor.input."


	f.write('echo Started ' + circuitName + '\n\n')

	for i in range(0, numTests):
		outputNumber = str(i).zfill(4)
		inputFile = baseInputFile + str(i)

		localPortNum = str( (portNum + 3 * (i % 2)) )
		outputFile = baseOutputFile + outputNumber + ".txt"

		command = "./a.out " + circuitFile + " " + ipAddress + " " + localPortNum + " " + inputFile + " " + str(protocolNum) + " " + str(partyID) + " > " + outputFile 

		f.write("echo " + str(i))
		f.write('\n')
		f.write(command)
		f.write('\n')

	f.write('echo Finished ' + circuitName + '\n\n\n')




if 4 != len(sys.argv):
    print "Not enough arguments."
    print "IP Port PartyID"
    exit()


protocolNames = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
circuitNamesList = ["adder_32bit", "multiplication_32bit", "AES-non-expanded"]


ipAddress = sys.argv[1]
portNum = 6789
numTests = int(sys.argv[2])
partyID = int(sys.argv[3])

for protocolNum in range(0, len(protocolNames)):
	if 1 == partyID:
		scriptName = "Test_" + protocolNames[protocolNum] + ".builder.sh"
	else:
		scriptName = "Test_" + protocolNames[protocolNum] + ".executor.sh"


	f = open(scriptName, 'w')
	f.write("#!/bin/bash\n\n")

	for k in range(0, len(circuitNamesList)):
		runTests(ipAddress, portNum, partyID, protocolNum, circuitNamesList[k], f)

	f.close()
	os.chmod(scriptName, 0744)


# ++++++++++++++++++++++++++++++++++++++++++++++++
# LOCAL   : python runTests.py 127.0.0.1 5
# DIFFIE  : python runTests.py 192.168.0.2 100
# HELLMAN : python runTests.py 192.168.0.1 100
# ++++++++++++++++++++++++++++++++++++++++++++++++


