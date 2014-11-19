import sys

invCount = 0
tempInvCount = 0


ins = open( sys.argv[1], "r" )
fileStr = ins.read().split("\n")
ins.close()

temp = fileStr[0]
temp = temp.split(" ")
numGates = int( temp[0] )
numGatesWire = int( temp[1] )

temp = fileStr[1]
temp = temp.split(" ")
numInputs = int( temp[0] ) + int( temp[1] )
numOutputs = int( temp[4] )


gatesDict = {}
readingOrder = []

fileStr.pop(0); fileStr.pop(0); fileStr.pop(0)

for line in fileStr:
	if("INV" in line):
		gateID = int(line.split(" ")[3])
		temp = line.split(" ")
		if( gateID < (numGatesWire - numOutputs) ):
			invCount += 1
		readingOrder.append(gateID)
		gatesDict[gateID] = [0, "INV", int(temp[2]), int(temp[3])]

	if("AND" in line or "XOR" in line):
		temp = line.split(" ")
		gatesDict[int(temp[4])] = [0, temp[5], int(temp[2]), int(temp[3])]
		readingOrder.append(int(temp[4]))



outputGate = {}

for gateID in gatesDict.keys():
	if("INV" in gatesDict[gateID][1]):
		tempInvCount += 1
		gatesDict[gateID][0] = gateID - tempInvCount
	else:
		gatesDict[gateID][0] = gateID - tempInvCount


for gateID in gatesDict.keys():
	if("INV" not in gatesDict[gateID][1]):
		inputID1 = gatesDict[gateID][2]
		inputID2 = gatesDict[gateID][3]
		temp = [gatesDict[gateID][0], gatesDict[gateID][1], inputID1, inputID2]

		if(inputID1 >= numInputs):
			temp[2] = gatesDict[inputID1][0]
			if("INV" in gatesDict[inputID1][1]):
				if(gatesDict[inputID1][2] >= numInputs):
					temp[2] = gatesDict[ gatesDict[inputID1][2] ][0]

				temp[2] *= -1

		if(inputID2 >= numInputs):
			temp[3] = gatesDict[inputID2][0]
			if("INV" in gatesDict[inputID2][1]):
				if(gatesDict[inputID2][2] >= numInputs):
					temp[2] = gatesDict[ gatesDict[inputID2][2] ][0]

				temp[3] *= -1

		outputGate[gateID] = temp



for key in outputGate.keys():
	print str(key) + "  -  " + str(outputGate[key])


'''
outs = open( sys.argv[2], "w" )
outs.write( str(numGates - invCount) + " " + str(numGatesWire - invCount) )
for line in fileStr:
	if("INV" not in line):
		outs.write(line)


outs.close()
'''