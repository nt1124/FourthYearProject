from collections import defaultdict
import re
import numpy




def getMeasurementFromString(inputStr):
	return float(re.split(':|=',inputStr)[-1])


def printMeanAndStdDev(listToStat):
	listMean = numpy.mean(listToStat)
	listStd = numpy.std(listToStat)

	print "Mean    : " + str(listMean)
	print "Std Dev : " + str(listStd) + "\n"


def getFileAsStringArray(filepath):
	toReturn = []

	with open(filepath, 'r') as content_file:
		content = content_file.read()

	splitFile = content.split("\n")
	filteredReturn = filter(None, splitFile)

	for line in filteredReturn:
		if(line[0] in infoTags):
			toReturn.append(line)

	return toReturn



protocolNameList = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
circuitNameList = ["adder_32bit", "multiplication_32bit", "AES-non-expanded"]

protocolNum = 0
circuitNum = 0
numTests = 100
partyID = 1

commTags = ['#', ':']
infoTags = ['#', ':', '+']
protocolName = protocolNameList[protocolNum]
circuitName = circuitNameList[circuitNum]

CPU_Dict = defaultdict(list)
Wall_Dict = defaultdict(list)
Bytes_Sent_Dict = defaultdict(list)
Bytes_Received_Dict = defaultdict(list)


for i in range(0, numTests):
	outputNumber = str(i).zfill(4)
	OutputFile = "./TestResults/Output_" + circuitName + "_" + str(partyID) + "_" + protocolName + "_" + outputNumber + ".txt"

	fileAsArray = getFileAsStringArray(OutputFile)
	
	j = 0
	while( j < len(fileAsArray)):
		incrementBy = 1
		line = fileAsArray[j]
		truncatedLine = line[4:]

		CPU_Dict[truncatedLine].append( float(fileAsArray[j + 1][16:]) )
		Wall_Dict[truncatedLine].append( float(fileAsArray[j + 2][16:]) )

		if(fileAsArray[j + 3][0] in commTags):
			Bytes_Sent_Dict[truncatedLine].append(getMeasurementFromString(fileAsArray[j + 3]))
			Bytes_Received_Dict[truncatedLine].append(getMeasurementFromString(fileAsArray[j + 4]))
			incrementBy = 5
		else:
			incrementBy = 3

		j += incrementBy


for key in CPU_Dict:
	print "+++ " + key

	printMeanAndStdDev(CPU_Dict[key])
	printMeanAndStdDev(Wall_Dict[key])
	printMeanAndStdDev(Bytes_Sent_Dict[key])
	printMeanAndStdDev(Bytes_Received_Dict[key])







