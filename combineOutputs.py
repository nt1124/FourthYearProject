from collections import defaultdict
import re
import numpy


def intWithCommas(x):
    if type(x) not in [type(0), type(0L)]:
        raise TypeError("Parameter must be an integer.")
    if x < 0:
        return '-' + intWithCommas(-x)
    result = ''
    while x >= 1000:
        x, r = divmod(x, 1000)
        result = ",%03d%s" % (r, result)
    return "%d%s" % (x, result)


def writeLatexTableRowToFile(f, curKey, CPU_Dict, Wall_Dict, Bytes_Sent_Dict, Bytes_Received_Dict):
	listMean = numpy.mean(CPU_Dict[curKey])
	f.write("& $%.2f$ " % listMean)

	listMean = numpy.mean(Wall_Dict[curKey])
	f.write("& $%.2f$ " % listMean)

	listMean = numpy.mean(Bytes_Sent_Dict[curKey])
	f.write("& $" + intWithCommas(int(listMean)) + "$ ")

	listMean = numpy.mean(Bytes_Received_Dict[curKey])
	f.write("& $" + intWithCommas(int(listMean)) + "$ \\\\\n")


def getMeasurementFromString(inputStr):
	return float(re.split(':|=',inputStr)[-1])


def fileWriteStats(f, listToStat):
	listMean = numpy.mean(listToStat)
	listStd = numpy.std(listToStat)


	f.write("Mean    : %.2f\n" % listMean)
	f.write("Std Dev : %.2f\n" % listStd)


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


def processOneSet(protocolNum, circuitNum, partyID):
	numTests = 100
	ordering = []
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



			if(fileAsArray[j + 3][0] in commTags):
				if truncatedLine not in CPU_Dict:
					ordering.append(truncatedLine)
				CPU_Dict[truncatedLine].append( float(fileAsArray[j + 1][16:]) )
				Wall_Dict[truncatedLine].append( float(fileAsArray[j + 2][16:]) )
				Bytes_Sent_Dict[truncatedLine].append(getMeasurementFromString(fileAsArray[j + 3]))
				Bytes_Received_Dict[truncatedLine].append(getMeasurementFromString(fileAsArray[j + 4]))
				incrementBy = 5
			else:
				incrementBy = 3

			j += incrementBy


	outputFile = "./StatResults/Stats_" + circuitName + "_" + str(partyID) + "_" + protocolName + ".txt"

	f = open(outputFile, 'w')

	for key in ordering:
		f.write(key + "\n")
		'''
		f.write("CPU Time\n")
		fileWriteStats(f, CPU_Dict[key])
		f.write("\nWall Time\n")
		fileWriteStats(f, Wall_Dict[key])
		f.write("\nBytes Sent\n")
		fileWriteStats(f, Bytes_Sent_Dict[key])
		f.write("\nBytes Received\n")
		fileWriteStats(f, Bytes_Received_Dict[key])
		f.write("\n")
		'''
		writeLatexTableRowToFile(f, key, CPU_Dict, Wall_Dict, Bytes_Sent_Dict, Bytes_Received_Dict)
		f.write("\n\n")


	f.close()



commTags = ['#', ':']
infoTags = ['#', ':', '+']
protocolNameList = ["LP_2010", "L_2013", "HKE_2013", "CHIMERA_2013"]
circuitNameList = ["adder_32bit", "multiplication_32bit", "AES-non-expanded"]

protocolNum = 2
circuitNum = 0
partyID = 0

for partyID in range(0, 2):
	for circuitNum in range(0, 3):
		processOneSet(protocolNum, circuitNum, partyID)


