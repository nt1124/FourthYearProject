#!/bin/bash

echo Started adder_32bit

echo 0
./a.out ./circuitFiles/adder_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/adder_32bit.executor.input.0 0 1 > ./TestResults/Output_adder_32bit_1_LP_2010_0000.txt
echo 1
./a.out ./circuitFiles/adder_32bit.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/adder_32bit.executor.input.1 0 1 > ./TestResults/Output_adder_32bit_1_LP_2010_0001.txt
echo 2
./a.out ./circuitFiles/adder_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/adder_32bit.executor.input.2 0 1 > ./TestResults/Output_adder_32bit_1_LP_2010_0002.txt
echo 3
./a.out ./circuitFiles/adder_32bit.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/adder_32bit.executor.input.3 0 1 > ./TestResults/Output_adder_32bit_1_LP_2010_0003.txt
echo 4
./a.out ./circuitFiles/adder_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/adder_32bit.executor.input.4 0 1 > ./TestResults/Output_adder_32bit_1_LP_2010_0004.txt
echo Finished adder_32bit


echo Started multiplication_32bit

echo 0
./a.out ./circuitFiles/multiplication_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/multiplication_32bit.executor.input.0 0 1 > ./TestResults/Output_multiplication_32bit_1_LP_2010_0000.txt
echo 1
./a.out ./circuitFiles/multiplication_32bit.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/multiplication_32bit.executor.input.1 0 1 > ./TestResults/Output_multiplication_32bit_1_LP_2010_0001.txt
echo 2
./a.out ./circuitFiles/multiplication_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/multiplication_32bit.executor.input.2 0 1 > ./TestResults/Output_multiplication_32bit_1_LP_2010_0002.txt
echo 3
./a.out ./circuitFiles/multiplication_32bit.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/multiplication_32bit.executor.input.3 0 1 > ./TestResults/Output_multiplication_32bit_1_LP_2010_0003.txt
echo 4
./a.out ./circuitFiles/multiplication_32bit.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/multiplication_32bit.executor.input.4 0 1 > ./TestResults/Output_multiplication_32bit_1_LP_2010_0004.txt
echo Finished multiplication_32bit


echo Started AES-non-expanded

echo 0
./a.out ./circuitFiles/AES-non-expanded.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/AES-non-expanded.executor.input.0 0 1 > ./TestResults/Output_AES-non-expanded_1_LP_2010_0000.txt
echo 1
./a.out ./circuitFiles/AES-non-expanded.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/AES-non-expanded.executor.input.1 0 1 > ./TestResults/Output_AES-non-expanded_1_LP_2010_0001.txt
echo 2
./a.out ./circuitFiles/AES-non-expanded.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/AES-non-expanded.executor.input.2 0 1 > ./TestResults/Output_AES-non-expanded_1_LP_2010_0002.txt
echo 3
./a.out ./circuitFiles/AES-non-expanded.rtl.circuit 192.168.0.2 6792 ./inputs/RandomTestInputs/AES-non-expanded.executor.input.3 0 1 > ./TestResults/Output_AES-non-expanded_1_LP_2010_0003.txt
echo 4
./a.out ./circuitFiles/AES-non-expanded.rtl.circuit 192.168.0.2 6789 ./inputs/RandomTestInputs/AES-non-expanded.executor.input.4 0 1 > ./TestResults/Output_AES-non-expanded_1_LP_2010_0004.txt
echo Finished AES-non-expanded


