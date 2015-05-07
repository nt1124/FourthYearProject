University of Bristol - MEng. Dissertation
Mathematics and Computer Science.

Author : Nicholas James Tutte - nt1124
Supervisor : Prof. Nigel Smart

Title : Secure Two Party Computation - A practical comparison of recent protocols.

Here we give a short summary of each folder/file included in our implementation in alphabetic order.
+ indicates a folder.
- indicates a file.

+ Root
	- circuitEvaluator.c : The main source code file.


	+ circuitFiles : Contains the input files representing the circuits to be evaluated


	+ circuits : Folder containing source code relating to circuits.

		- circuitUtils.h : Header binding all the circuit/circuit building/crypto code together.

		- gateOrWire.(c/h) : Source code for the gates/wires of a Yao Garbled Circuit. Also has the data structure for a circuit made from these gates/wires.

		- circuitUtils.c : Source code for utility functions, e.g. Printing output of circuit.

		- circuitCommunications.c : Source code for sending and receiving circuits.

		- circuitCorrectnessChecks : Source code for checking if two circuits are the same, used for checking correctness.

		+ LP_2010 : Folder containing all the source code directly relating to the Lindell-Pinkas 2010 implementation

			- LP_S2C_CnC_OT.h : Header file binding all the Lindell-Pinkas 2010 source code together.

			- LP_S2C_CnC_OT_Builder_ECC.c : Source code file, mostly utils relating to the builder. For example code to run all the OTs required, and to build the circuits.

			- LP_S2C_CnC_OT_Executor_ECC.c : Source code file, mostly utils relating to the Executor.

			- LP_S2C_CnC_OT.c : Source code file, main functions for Builder and Executor in the LP-2010 protocol.


		+ L_2013 : Folder containing all the source code directly relating to the implementation of the  Lindell 2013 protocol

			- L_CnC_OT.h : Header file binding all the Lindell 2013 source code together.

			- L_CnC_OT_Check_Builder.c : Source code file, functions for running the sub-computation for the Builder.

			- L_CnC_OT_ECC_Builder.c : Source code file, mostly utils relating to the main computation for the Builder. For example code to run all the OTs required, and to build the circuits. Calls the main of the L_CnC_OT_Check_Builder.c.

			- L_CnC_OT_Check_Executor.c : Source code file, functions for running the sub-computation for the Executor.

			- L_CnC_OT_ECC_Executor.c : Source code file, mostly utils relating to the Executor. Calls the main of the L_CnC_OT_Check_Executor.c.

			- L_CnC_OT.c : Source code file, main functions for Builder and Executor in the L-2013 protocol.


		+ HKE_2013 : Folder containing all the source code directly relating to the implementation of the Huang-Katz-Evans 2013 protocol.

			-  : 


		+ L_2013_HKE : Folder containing all the source code directly relating to the implementation of the modification of the Lindell 2013 protcol to use the Huang-Katz-Evans protocol for the sub-computation.




	+ comms : Folder containing the source code for the communication between parties.


	+ crypto : Folder containing source code relating to cryptographic primitives (e.g. OTs, AES etc).

		+ OT : Folder containing all source code for the Oblivious Transfer primitives.

		+ SecretSharing : Folder containing all source code for Secret scharing schemes (including F_q Polynomials and lagrange interpolation).

		+ 


	+ fileUtils : Folder containing the source code for reading input/circuit files, but also for converting a binary circuit read from file to a Yao Garbled Circuit.


	+ inputs : Folder contain the input files representing the input of the parties to the computations.





