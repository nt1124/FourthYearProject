#ifndef L_2013_CnC_OT
#define L_2013_CnC_OT


#include "../../fileUtils/createRawCheckCircuit.h"


struct secCompBuilderOutput
{
	struct public_builderPRS_Keys *public_inputs;
	struct eccPoint **builderInputs;

	unsigned char *J_set;
	int J_setSize;
} secCompBuilderOutput;

struct secCompExecutorOutput
{
	struct publicInputsWithGroup *pubInputGroup;
	struct eccPoint **builderInputs;

	unsigned char *J_set;
	int J_setSize;

	unsigned char *output;
} secCompExecutorOutput;


#include "L_S2C_CnC_OT_ECC_Builder.c"
#include "L_S2C_CnC_OT_ECC_Executor.c"

#include "L_S2C_CnC_OT_Check_Builder.c"
#include "L_S2C_CnC_OT_Check_Executor.c"


#include "L_S2C_CnC_OT.c"

#endif
