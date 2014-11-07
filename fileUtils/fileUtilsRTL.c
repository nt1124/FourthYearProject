
/*
int getNumGW(char *line)
{
	int strIndex = 0;
	int toReturn = 0;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] == ' ' ) {}

	while( line[strIndex] != ' ')
	{
		toReturn *= 10;
		toReturn += atoi(line[strIndex++]);
	}

	return toReturn;
}
*/


// RTL means the function deals with the Smart/Tillich style input.


struct gate *processGateRTL(char* line, int strIndex, struct gateOrWire **circuit,
						struct gateOrWire *curGate)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int tempIndex, outputTableSize = 1;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] != ' ' ) {}
	tempIndex = ++ strIndex;
	while( line[strIndex] != ' ' ) { strIndex ++; }

	char *tempString = (char*) calloc((strIndex - tempIndex + 1), sizeof(char));
	strncpy(tempString, line + tempIndex, (strIndex - tempIndex));
	toReturn -> numInputs = atoi(tempString);

	for(tempIndex = 0; tempIndex < toReturn -> numInputs; tempIndex ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	toReturn -> rawOutputTable = parseOutputTable(line, &strIndex, toReturn);
	toReturn -> inputIDs = parseInputTable(line, toReturn -> numInputs, &strIndex);
	toReturn -> encOutputTable = recursiveOutputTable(toReturn);
	
	free(tempString);

	return toReturn;
}


struct gateOrWire *processGateOrWireRTL(char *line, int idNum, int *strIndex, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);

	if( 'i' == line[*strIndex] )
	{
		toReturn -> gatePayload = NULL;
		toReturn -> outputWire -> wireMask = 0xF0;
	}
	else
	{
		if('o' == line[*strIndex])
		{
			toReturn -> outputWire -> wireMask = 0x0F;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}

		toReturn -> gatePayload = processGateRTL(line, *strIndex, circuit, toReturn);
		encWholeOutTable(toReturn, circuit);
	}

	return toReturn;
}


struct gateOrWire *processGateLineRTL(char *line, struct gateOrWire **circuit)
{
	int strIndex = 0, idNum;

	while( line[strIndex] != ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	char *idString = (char*) calloc(strIndex + 1, sizeof(char));
	strncpy(idString, line, strIndex);
	idNum = atoi(idString);
	free(idString);

	while( line[strIndex] == ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	return processGateOrWireRTL(line, idNum, &strIndex, circuit);
}


struct gateOrWire *initialiseInputWire(int idNum, unsigned char owner)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0xF0;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


struct gateOrWire **readInCircuiRTL(char* filepath, int *numGates)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; /* or other suitable maximum line size */
	int numInputs1, numInputs2, numOutputs;	int gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit;

	if ( file != NULL )
	{
		if(NULL != fgets ( line, sizeof(line), file ))
			sscanf(line, "%d %d", &numInputs1, numGates);
		else
			return NULL;

		if(NULL != fgets ( line, sizeof(line), file ))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &numOutputs);
		else
			return NULL;

		if(NULL == fgets ( line, sizeof(line), file ))
			return NULL;

	 	circuit = (struct gateOrWire**) calloc(*numGates, sizeof(struct gateOrWire*));

		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			tempGateOrWire = processGateLineRTL(line, circuit);
			if( NULL != tempGateOrWire )
			{
				*(circuit + gateIndex) = tempGateOrWire;
				gateIndex ++;
			}
		}
		fclose ( file );
	}

	return circuit;
}