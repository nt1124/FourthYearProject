import random

f_builder = open('inputs/randomAES.builder.input', 'w')

for i in range(128):
	toWrite = "Builder " + str(i) + " " + str(random.randint(0, 1)) + "\n"
	f_builder.write(toWrite)

f_builder.close()


f_executor = open('inputs/randomAES.executor.input', 'w')

for i in range(128, 256):
	toWrite = "Executor " + str(i) + " " + str(random.randint(0, 1)) + "\n" 
	f_executor.write(toWrite)

f_executor.close()