#ifndef L_2013_HKE
#define L_2013_HKE


#include "../../fileUtils/createRawCheckCircuit.h"


struct secCompBuilderOutput_HKE
{
	unsigned char *J_set;
	int J_setSize;
} secCompBuilderOutput_HKE;

struct secCompExecutorOutput_HKE
{
	unsigned char *J_set;
	int J_setSize;

	unsigned char *output;
} secCompExecutorOutput_HKE;


// #include "L_2013_HKE_ECC_Builder.c"
// #include "L_2013_HKE_ECC_Executor.c"

// #include "L_2013_HKE_Check_Builder.c"
// #include "L_2013_HKE_Check_Executor.c"


// #include "L_2013_HKE.c"

#endif
