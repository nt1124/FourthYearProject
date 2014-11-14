#include <string.h>
#include <stdlib.h>
#include <stdio.h>


int main(int argc, char *argv[])
{
	int i;
	int numInputsBuilder = atoi(argv[2]);
	int numInputsExec = atoi(argv[3]);

	FILE *fileExec, *fileBuilder;
	char *builderInputFilepath = (char*) calloc(strlen(argv[1]) + 15, sizeof(char));
	char *executorInputFilepath = (char*) calloc(strlen(argv[1]) + 16, sizeof(char));

	strncpy(builderInputFilepath, argv[1], strlen(argv[1]));
	strncat(builderInputFilepath, ".builder.input", 15);
	strncpy(executorInputFilepath, argv[1], strlen(argv[1]));
	strncat(executorInputFilepath, ".executor.input", 16);

	fileBuilder = fopen(builderInputFilepath, "w");
	fileExec = fopen(executorInputFilepath, "w");

	for(i = 0; i < numInputsBuilder; i ++)
		fprintf(fileBuilder, "Builder %d 0\n", i);

	for(i = 0; i < numInputsExec; i ++)
		fprintf(fileExec, "Executor %d 0\n", i + numInputsBuilder);

	fclose(fileExec);
	fclose(fileBuilder);
}