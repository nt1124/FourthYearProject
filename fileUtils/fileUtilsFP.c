int *parseOutputTable(char* line, int *strIndex, struct gate *curGate)
{
	int tableIndex = 0;
	int *tableToParse = (int*) calloc(curGate -> outputTableSize, sizeof(int));

	while( line[++ *strIndex] != '[' ) {}
	
	while( line[++ *strIndex] != ']' )
	{
		if( '1' == line[*strIndex] )
			tableToParse[tableIndex ++] = 1;
		else if( '0' == line[*strIndex] )
			tableToParse[tableIndex ++] = 0;
	}

	return tableToParse;
}


int *parseInputTable(char* line, int tableSize, int *strIndex)
{
	int tableIndex = -1;
	int *tableToReturn = (int*) calloc(tableSize, sizeof(int));
	char *curCharStr = (char*) calloc(2, sizeof(char));

	while( line[++ *strIndex] != '[' ) {}

	while( line[++ *strIndex] != ']' )
	{
		if( ' ' == line[*strIndex] )
			tableIndex ++;
		else
		{
			tableToReturn[tableIndex] *= 10;
			curCharStr[0] = line[*strIndex];
			tableToReturn[tableIndex] += atoi(curCharStr);
		}
	}

	free(curCharStr);

	return tableToReturn;
}


int count_lines_of_file(char * filepath)
{
    FILE *file = fopen ( filepath, "r" );
    int line_count = 0;

	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			if( (48 <= line[0] && line[0] < 58) ||
				( 'A' == line[0] || 'B' == line[0] ) )
			{
				line_count ++;	
			}
		}
		fclose ( file );
		return line_count;
	}

    return -1;
}


struct gate *processGateFP(char* line, int strIndex, struct gateOrWire **circuit,
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
	toReturn -> encOutputTable = createOutputTable(toReturn);
	
	free(tempString);

	return toReturn;
}


// FP stands for Fairplay
struct gateOrWire *processGateOrWireFP(char *line, int idNum, int *strIndex, struct gateOrWire **circuit)
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

		toReturn -> gatePayload = processGateFP(line, *strIndex, circuit, toReturn);
		encWholeOutTable(toReturn, circuit);
	}

	return toReturn;
}


struct gateOrWire *processGateLineFP(char *line, struct gateOrWire **circuit)
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

	return processGateOrWireFP(line, idNum, &strIndex, circuit);
}


struct gateOrWire **readInCircuitFP(char* filepath, int *numGates, int **execOrder)
{
	*numGates = count_lines_of_file(filepath);
	int execIndex = 0, i;
	int gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(*numGates, sizeof(struct gateOrWire*));
	
	*execOrder = (int*) calloc(*numGates, sizeof(int));

	for(i = 0; i < *numGates; i ++)
	{
		(*execOrder)[i] = i;
	}

	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			tempGateOrWire = processGateLineFP(line, circuit);
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