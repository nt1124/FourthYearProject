#ifndef L_2013_HKE
#define L_2013_HKE


#include "../../fileUtils/createRawCheckCircuit.h"


/*
struct secCompBuilderOutput_HKE
{
	struct public_builderPRS_Keys *public_inputs;
	struct eccPoint **builderInputs;

	unsigned char *J_set;
	int J_setSize;
} secCompBuilderOutput_HKE;


struct secCompExecutorOutput_HKE
{
	struct publicInputsWithGroup *pubInputGroup;
	struct eccPoint **builderInputs;

	unsigned char *J_set;
	int J_setSize;

	unsigned char *output;
} secCompExecutorOutput_HKE;
*/


#include "L_2013_HKE_Utils.c"

// #include "L_2013_HKE_ECC_Builder.c"
// #include "L_2013_HKE_ECC_Executor.c"

#include "L_2013_HKE_Check_Builder.c"
#include "L_2013_HKE_Check_Executor.c"

#include "L_2013_HKE.c"

#endif
