struct idAndValue *new_idAndValue(int id, unsigned char value)
{
	struct idAndValue *newElement = (struct idAndValue *) calloc(1, sizeof(struct idAndValue));

	newElement -> id = id;
	newElement -> value = value;

	return newElement;
}


void free_idAndValueChain(struct idAndValue *start)
{
	struct idAndValue *current = start;
	struct idAndValue *last = NULL;

	while(NULL != current)
	{
		last = current;
		current = current -> next;
		free(last);
	}
}


struct idAndValue *readInputLine_Alt(char *line)
{	
	struct idAndValue *newElement;
	int strIndex = 0, gateID = 0, outputLength = 0, i, j;
	unsigned char value;

	while( ' ' != line[strIndex++] ){}
	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		gateID += line[strIndex] - 48;
		strIndex ++;
	}
	strIndex ++;

	if( '1' == line[strIndex] )
		value = 0x01;
	else if( '0' == line[strIndex] )
		value = 0x00;

	newElement = new_idAndValue(gateID, value);

	return newElement;
}


// Read the file containing our input data.
struct idAndValue *readInputDetailsFile_Alt(char *filepath)
{
	FILE *file = fopen( filepath, "r" );
	struct idAndValue *start = (struct idAndValue*) calloc(1, sizeof(struct idAndValue));
	struct idAndValue *current = start;


	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			current -> next = readInputLine_Alt(line);
			current = current -> next;
		}
		fclose ( file );
	}

	return start;
}


void setCircuitsInputs_Values(struct idAndValue *start, struct Circuit *inputCircuit, unsigned char ownerFlag)
{	
	struct idAndValue *current = start -> next;
	struct wire *outputWire;

	while(NULL != current)
	{
		outputWire = inputCircuit -> gates[current -> id] -> outputWire;
		outputWire -> wireOwner = ownerFlag;
		outputWire -> wirePermedValue = (current -> value) ^ (outputWire -> wirePerm & 0x01);
		current = current -> next;
	}
}


void setCircuitsInputs_Hardcode(struct idAndValue *start, struct Circuit *inputCircuit, unsigned char ownerFlag)
{	
	struct idAndValue *current = start -> next;
	struct wire *outputWire;

	while(NULL != current)
	{
		outputWire = inputCircuit -> gates[current -> id] -> outputWire;
		outputWire -> wireOwner = ownerFlag;

		if( 0x01 == current -> value )
		{
			outputWire -> wirePermedValue = 0x01 ^ (0x01 & outputWire -> wirePerm);
		}
		else if( 0x00 == current -> value )
		{
			outputWire -> wirePermedValue = (0x01 & outputWire -> wirePerm);
		}

		current = current -> next;
	}
}


