# python generateRandomTestInput.py 100 32 32 adder_32bit

import random
import sys


if 5 != len(sys.argv):
    print "Wrong number of arguments."
    exit()


numToGen = int(sys.argv[1])
numInputs_P1 = int(sys.argv[2])
numInputs_P2 = int(sys.argv[3])
circuitName = sys.argv[4]


for i in range(0, numToGen):
	f_builder = open('./inputs/RandomTestInputs/' + circuitName + '.builder.input.' + str(i), 'w')

	for j in range(0, numInputs_P1):
		toWrite = "Builder " + str(j) + " " + str(random.randint(0, 1)) + "\n"
		f_builder.write(toWrite)

	f_builder.close()


	f_executor = open('./inputs/RandomTestInputs/' + circuitName + '.executor.input.' + str(i), 'w')

	for j in range(numInputs_P1, numInputs_P1 + numInputs_P2):
		toWrite = "Executor " + str(j) + " " + str(random.randint(0, 1)) + "\n" 
		f_executor.write(toWrite)

	f_executor.close()
