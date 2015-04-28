import os
import sys



def runTests(ipAddress, portNum, partyID, circuitName):
	circuitFile = "./circuitFiles/" + circuitName + ".rtl.circuit"
	baseInputFile = "./inputs/RandomTestInputs/" + circuitName
	baseOutputFile = "./TestResults/Output_" + circuitName + "_" + partyID + "_" + protocolNames[protocolNum] + "_"


	if '1' == partyID:
		baseInputFile = baseInputFile + ".builder.input."
	else:
		baseInputFile = baseInputFile + ".executor.input."

	# os.chdir('..')

	for i in range(0, numTests):
		outputNumber = str(i).zfill(4)
		inputFile = baseInputFile + str(i)
		outputFile = baseOutputFile + outputNumber + ".txt"
		command = "./a.out " + circuitFile + " " + ipAddress + " " + portNum + " " + inputFile + " " + str(protocolNum) + " " + str(partyID) + " > " + outputFile 


		open(outputFile, 'w').close()

		print command
		os.system(command)
		print "Done"



if 4 != len(sys.argv):
    print "Not enough arguments."
    print "IP Port PartyID"
    exit()


numTests = 3
protocolNames = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
circuitNamesList = ["adder_32bit", "multiplication_32bit", "AES-non-expanded"]
protocolNum = 2


ipAddress = sys.argv[1]
portNum = sys.argv[2]
partyID = sys.argv[3]


for i in range(0, len(circuitNamesList)):
	runTests(ipAddress, portNum, partyID, circuitNamesList[i])



# python runTests.py 127.0.0.1 6789 1

