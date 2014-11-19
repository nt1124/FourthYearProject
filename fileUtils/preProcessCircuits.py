import sys




def populateGatesDict(fileStr, gatesDict, readingOrder, numGatesWire, numOutputs):
	invCount = 0
	fileStr.pop(0); fileStr.pop(0); fileStr.pop(0)
	
	for line in fileStr:
		if("INV" in line):
			gateID = int(line.split(" ")[3])
			temp = line.split(" ")
			if( gateID < (numGatesWire - numOutputs) ):
				invCount += 1
			readingOrder.append(gateID)
			gatesDict[gateID] = [0, "INV", int(temp[2])]

		if("AND" in line or "XOR" in line):
			temp = line.split(" ")
			gatesDict[int(temp[4])] = [0, temp[5], int(temp[2]), int(temp[3])]
			readingOrder.append(int(temp[4]))

	return invCount


def shiftOutInvGates(gatesDict, numGatesWire, numOutputs):
	tempInvCount = 0

	for gateID in gatesDict.keys():
		if("INV" in gatesDict[gateID][1]):
			if(gateID < numGatesWire - numOutputs):
				tempInvCount += 1
			else:
				gatesDict[gateID][0] = gateID - tempInvCount
		else:
			gatesDict[gateID][0] = gateID - tempInvCount


	for gateID in gatesDict.keys():
		if("INV" in gatesDict[gateID][1] and gateID < numGatesWire - numOutputs):
			if(gatesDict[gateID][2] >= numInputs):
				gatesDict[gateID][0] = gatesDict[gatesDict[gateID][2]][0]
				gatesDict[gateID][2] = gatesDict[gateID][0]
			else:
				gatesDict[gateID][0] = gatesDict[gateID][2]


def collapseInversionGates(gatesDict, outputGate, numGatesWire, numInputs, numOutputs):
	for gateID in gatesDict.keys():
		if("INV" not in gatesDict[gateID][1]):
			inputID1 = gatesDict[gateID][2]
			inputID2 = gatesDict[gateID][3]
			temp = [gatesDict[gateID][0], gatesDict[gateID][1], inputID1, inputID2, 1, 1]

			if(inputID1 >= numInputs):
				temp[2] = gatesDict[inputID1][0]
				if("INV" in gatesDict[inputID1][1]):
					if(gatesDict[inputID1][2] >= numInputs):
						temp[2] = gatesDict[ gatesDict[inputID1][2] ][0]
					temp[4] = -1

			if(inputID2 >= numInputs):
				temp[3] = gatesDict[inputID2][0]
				if("INV" in gatesDict[inputID2][1]):
					if(gatesDict[inputID2][2] >= numInputs):
						temp[3] = gatesDict[ gatesDict[inputID2][2] ][0]
					temp[5] = -1

			outputGate[gateID] = temp

		else:
			if(gateID >= numGatesWire - numOutputs):
				inputID = gatesDict[gateID][2]
				temp = [gatesDict[gateID][0], gatesDict[gateID][1], inputID, -1]
				outputGate[gateID] = temp


def writeGate(outs, outputGate, key, numGates, numOutputs):
	if("XOR" in outputGate[key][1]):
		writeXOR(outs, outputGate, key)

	elif("AND" in outputGate[key][1]):
		writeAND(outs, outputGate, key)

	elif("INV" in outputGate[key][1]):
		if(key >= numGates - numOutputs):
			outputStr = "2 1 " + str(outputGate[key][2]) + " " + str(outputGate[key][0]) + " INV\n"
			outs.write(outputStr)



def writeXOR(outs, outputGate, key):
	gateID = outputGate[key][0]
	inputID1 = outputGate[key][2]
	inputID2 = outputGate[key][3]
	outputStr = "2 1 " + str(inputID1) + " " + str(inputID2) + " " + str(gateID)

	if(1 == outputGate[key][4] * outputGate[key][5]):
		outs.write(outputStr + " XOR\n")
	else:
		outs.write(outputStr + " NXOR\n")


def writeAND(outs, outputGate, key):
	gateID = outputGate[key][0]
	outputStr = "2 1 " + str(outputGate[key][2]) + " " + str(outputGate[key][3]) + " " + str(gateID)

	if(1 == outputGate[key][4] and 1 == outputGate[key][5]):
		outputStr = outputStr + " AND1\n"
	elif(1 == outputGate[key][4] and -1 == outputGate[key][5]):
		outputStr = outputStr + " AND2\n"
	elif(-1 == outputGate[key][4] and 1 == outputGate[key][5]):
		outputStr = outputStr + " AND3\n"
	elif(-1 == outputGate[key][4] and -1 == outputGate[key][5]):
		outputStr = outputStr + " AND4\n"
	outs.write(outputStr)


def writeOutput(fileName, numGates, numGatesWire, numInputs1, numInputs2, numOutputs, outputGate, readingOrder, invCount):
	index = 0
	alreadyWritten = []
	for x in range(0, numInputs1 + numInputs2):
		alreadyWritten.append(x)

	outs = open( fileName, "w" )
	outs.write( str(numGates - invCount) + " " + str(numGatesWire - invCount) + "\n" )
	outs.write( str(numInputs1) + " " + str(numInputs2) + " " + str(numOutputs) + "\n\n" )

	for key in outputGate.keys():
		writeGate(outs, outputGate, key, numGates, numOutputs)

	# while(index < numGates):
	# 	writeGate(outs, outputGate, key, numGates, numOutputs)


	outs.close()




invCount = 0

ins = open( sys.argv[1], "r" )
fileStr = ins.read().split("\n")
ins.close()

temp = fileStr[0]
temp = temp.split(" ")
numGates = int( temp[0] )
numGatesWire = int( temp[1] )

temp = fileStr[1]
temp = temp.split(" ")
numInputs1 = int(temp[0])
numInputs2 = int(temp[1])
numInputs = int( temp[0] ) + int( temp[1] )
numOutputs = int( temp[4] )


gatesDict = {}
readingOrder = []
outputGate = {}

invCount = populateGatesDict(fileStr, gatesDict, readingOrder, numGatesWire, numOutputs)

shiftOutInvGates(gatesDict, numGatesWire, numOutputs)

collapseInversionGates(gatesDict, outputGate, numGatesWire, numInputs, numOutputs)

writeOutput(sys.argv[2], numGates, numGatesWire, numInputs1, numInputs2, numOutputs, outputGate, readingOrder, invCount)
