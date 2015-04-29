def getFileAsStringArray(filepath):
	with open(filepath, 'r') as content_file:
		content = content_file.read()

	toReturn = content.split("\n")

	return filter(None, toReturn)




protocolNameList = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
circuitNameList = ["adder_32bit", "multiplication_32bit", "AES-non-expanded"]

protocolNum = 2
circuitNum = 0
numTests = 5
partyID = 0

protocolName = protocolNameList[protocolNum]
circuitName = circuitNameList[circuitNum]



for i in range(0, numTests):
	outputNumber = str(i).zfill(4)
	OutputFile = "./TestResults/Output_" + circuitName + "_" + str(partyID) + "_" + protocolName + "_" + outputNumber + ".txt"

	fileAsArray = getFileAsStringArray(OutputFile)
	print fileAsArray
