#ifndef SERIALISATION_UTILS
#define SERIALISATION_UTILS

#include "gateOrWire.h"



int serialisedSizeInputWire(struct gateOrWire *inputGW)
{
	int serialisedSize = 0;

	if(0x00 == inputGW -> outputWire -> wireOwner)
	{
		serialisedSize ++; 	// One for the permutation.
	}
	else if(0xFF == inputGW -> outputWire -> wireOwner)
	{
		serialisedSize ++;		// One for the wirePermedValue.
		serialisedSize += 16;	// 16 Bytes for the hardcoded output key.
								// As we own this wire we give executor the correct key.
	}

	return serialisedSize;
}


int serialisedSizeGate(struct gate *inputGate)
{
	int serialisedSize = 0;

	serialisedSize ++;		// 1 Byte for the number of inputs.
	serialisedSize += 2;	// 2 Bytes for size of output table.
	serialisedSize ++;		// 1 Byte for the gateType.

	// 4 * numInputs many bytes for the input wire IDs
	serialisedSize = serialisedSize + (4 * inputGate -> numInputs);

	// 32 * outputTableSize many bytes for the garbled output table.
	serialisedSize = serialisedSize + (32 * inputGate -> outputTableSize);

	return serialisedSize;
}


// Get the size of the character buffer we need to store the input gateOrWire in.
int getSerialiseSizeAlt(struct gateOrWire *inputGW)
{
	int outputTableSize, numInputs;
	int serialisedSize = 7;

	if(NULL != inputGW -> gatePayload)
	{
		outputTableSize = inputGW -> gatePayload -> outputTableSize;
		numInputs = inputGW -> gatePayload -> numInputs;
	}


	if(0x01 == (0x0F & inputGW -> outputWire -> wireMask))
	{
		serialisedSize += serialisedSizeInputWire(inputGW);
	}
	else 
	{
		if(NULL != inputGW -> gatePayload)
		{
			serialisedSize += serialisedSizeGate(inputGW -> gatePayload);
		}

		// If this is an output gate then we need the wirePerm.
		if(0x02 == (0x0F & inputGW -> outputWire -> wireMask))
		{
			serialisedSize ++;
		}
	}

	return serialisedSize;
}


// Does the actual serialisation of an input wire into a buffer of uchars. Returns length of
// the buffer.
int serialiseInputWireAlt(struct gateOrWire *inputGW, unsigned char *toReturn, int offset)
{
	int curIndex = offset;

	if(0x00 == inputGW -> outputWire -> wireOwner)
	{
		// The executor is the owner of this input wire so needs the wirePerm.
		toReturn[curIndex ++] = inputGW -> outputWire -> wirePerm;
	}
	else if(0xFF == inputGW -> outputWire -> wireOwner)
	{
		toReturn[curIndex ++] = inputGW -> outputWire -> wirePermedValue;
		memcpy(toReturn + curIndex, inputGW -> outputWire -> wireOutputKey, 16);
		curIndex += 16;
	}

	return curIndex;
}


// Handles the serialisation of the gate part of a gateOrWire. Returns length of buffer.
int serialiseGateAlt(struct gate *inputGate, unsigned char *toReturn, int offset)
{
	int i, j, curIndex = offset;

	// One byte for numInputs 
	toReturn[curIndex++] = inputGate -> numInputs;

	// 2 bytes for the outputTableSize
	memcpy(toReturn + curIndex, &inputGate -> outputTableSize, 2);
	curIndex += 2;

	// For each inputID
	for(i = 0; i < inputGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(toReturn + j, inputGate -> inputIDs + i, 4);
	}

	for(i = 0; i < inputGate -> outputTableSize; i ++)
	{
		j = curIndex + (4 * inputGate -> numInputs) + (32 * i);
		memcpy(toReturn + j, inputGate -> encOutputTable[i], 32);
	}

	curIndex = curIndex + (4 * inputGate -> numInputs)
						+ (32 * inputGate -> outputTableSize);

	return curIndex;
}


// Handles the serialisation of a gateOrWire. Returns length of buffer.
int serialiseGateOrWireAlt(struct gateOrWire *inputGW, unsigned char *toReturn, int offset)
{
	int outputTableSize = 0, numInputs = 0;
	int serialisedSize = 0, curIndex = offset + 7;

	if(NULL != inputGW -> gatePayload)
	{
		outputTableSize = inputGW -> gatePayload -> outputTableSize;
		numInputs = inputGW -> gatePayload -> numInputs;
	}

	// Copy the G_ID.
	memcpy(toReturn + offset, &(inputGW -> G_ID), 4);

	// Then does this have a gate?
	if(NULL == inputGW -> gatePayload)
		toReturn[offset + 4] = 0x00;
	else
		toReturn[offset + 4] = 0xFF;

	// wireMask and wireOwner
	toReturn[offset + 5] = inputGW -> outputWire -> wireMask;
	toReturn[offset + 6] = inputGW -> outputWire -> wireOwner;

	// If it's an input wire
	if(0x01 == (0x0F & inputGW -> outputWire -> wireMask))
	{
		curIndex = serialiseInputWireAlt(inputGW, toReturn, curIndex);
	}
	else 
	{
		// If there is a gate to serialise.
		if(NULL != inputGW -> gatePayload)
		{
			curIndex = serialiseGateAlt(inputGW -> gatePayload, toReturn, curIndex);
		}

		// If this is an output gate then we need the wirePerm.
		if(0x02 == (0x0F & inputGW -> outputWire -> wireMask))
		{
			toReturn[curIndex] = inputGW -> outputWire -> wirePerm;
			curIndex ++;
		}
	}

	return curIndex;
}


// Deserialise a unsigned char buffer into an input gateOrWire
int deserialiseInputWire(struct gateOrWire *outputGW, unsigned char *serialGW, int offset)
{
	int curIndex = offset;

	if(0x00 == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePerm = serialGW[curIndex ++];
		outputGW -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
		
		outputGW -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(outputGW -> outputWire -> outputGarbleKeys -> key0, serialGW + curIndex, 16);
		curIndex += 16;

		outputGW -> outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(outputGW -> outputWire -> outputGarbleKeys -> key1, serialGW + curIndex, 16);
		curIndex += 16;
	}
	else if(0xFF == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePermedValue = serialGW[curIndex ++];
		memcpy(outputGW -> outputWire -> wireOutputKey, serialGW + curIndex, 16);
		curIndex += 16;
	}

	return curIndex;
}


// Deserialise a section of an (offset) uchar buffer into a gate.
int deserialiseGate(struct gate **outputGate, unsigned char *serialGW, int offset)
{
	int i, j, curIndex = offset;
	struct gate *tempGate = (struct gate*) calloc(1, sizeof(struct gate));

	// Get the number of input and the size of output table.
	tempGate -> numInputs = serialGW[curIndex++];
	memcpy(&tempGate -> outputTableSize, serialGW + curIndex, 2);
	curIndex += 2;

	// Calloc space for inputIDs table, then get inputIDs from serialisation.
	tempGate -> inputIDs = (int*) calloc(tempGate -> numInputs, sizeof(int));
	for(i = 0; i < tempGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(&tempGate -> inputIDs[i], serialGW + j, 4);
	}
	curIndex += (4 * tempGate -> numInputs);

	tempGate -> encOutputTable = (unsigned char **) calloc(tempGate -> outputTableSize, sizeof(unsigned char*));
	for(i = 0; i < tempGate -> outputTableSize; i ++)
	{
		tempGate -> encOutputTable[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
		j = curIndex + (32 * i);
		memcpy(tempGate -> encOutputTable[i], serialGW + j, 32);
	}

	curIndex += (32 * tempGate -> outputTableSize);

	// Export the output gate, then NULL tempGate for the heck of it.
	*outputGate = tempGate;
	tempGate = NULL;

	// Return the index in the serialisation string at end of reading.
	return curIndex;
}


// Function to deserialise an unsigned chars array to a gateOrWire struct.
struct gateOrWire *deserialiseGateOrWire(unsigned char *serialGW)
{
	unsigned char **tempEncOut;
	struct gateOrWire *toReturn;
	struct gate *tempGate = NULL;
	int i, curIndex = 7;

	// Calloc space for gateOrWire, and for the outputWire.
	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = (struct wire*) calloc(1, sizeof(struct wire));
	toReturn -> gatePayload = NULL;

	// Copy G_ID, wireMask, wireOwner from string. Initialise wirePerm to zero.
	memcpy(&(toReturn -> G_ID), serialGW, 4);
	toReturn -> outputWire -> wireMask = serialGW[5];
	toReturn -> outputWire -> wireOwner = serialGW[6];
	toReturn -> outputWire -> wirePerm = 0;
	
	// Calloc space for the wireOutputKey. 
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// If the wire is an input wire.
	if(0x01 == (0x0F & toReturn -> outputWire -> wireMask))
	{
		curIndex = deserialiseInputWire(toReturn, serialGW, curIndex);
	}
	else 
	{
		if(0xFF == serialGW[4])
		{
			curIndex = deserialiseGate(&tempGate, serialGW, curIndex);
		}

		if(0x02 == (0x0F & toReturn -> outputWire -> wireMask))
		{
			toReturn -> outputWire -> wirePerm = serialGW[curIndex];
			curIndex ++;
		}
	}

	// Assign the tempGate to the gatePayload of the gateOrWire to return.
	toReturn -> gatePayload = tempGate;
	tempGate = NULL;

	return toReturn;
}


unsigned char *serialiseCircuit(struct Circuit *inputCircuit, int *bufferLength)
{
	int i = 0, offset = 0, tempSerialiseSize = 0;
	unsigned char *toReturn;

	*bufferLength = 0;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		(*bufferLength) += getSerialiseSizeAlt(inputCircuit -> gates[i]);
		(*bufferLength) += 4;
	}

	toReturn = (unsigned char*) calloc(*bufferLength, sizeof(unsigned char));

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		tempSerialiseSize = getSerialiseSizeAlt(inputCircuit -> gates[i]);
		memcpy(toReturn + offset, &tempSerialiseSize, 4);
		offset = offset + 4;

		serialiseGateOrWireAlt(inputCircuit -> gates[i], toReturn, offset);
		offset = offset + tempSerialiseSize;
	}

	return toReturn;
}


struct gateOrWire **deserialiseCircuit(unsigned char *inputBuffer, int numGates)
{
	int i, offset, tempSerialiseSize;
	struct gateOrWire **outputCircuit = (struct gateOrWire **) calloc(numGates, sizeof(struct gateOrWire*));

	offset = 0;
	for(i = 0; i < numGates; i ++)
	{
		tempSerialiseSize = 0;
		memcpy(&tempSerialiseSize, inputBuffer + offset, 4);

		offset += 4;
		outputCircuit[i] = deserialiseGateOrWire(inputBuffer + offset);
		offset += tempSerialiseSize;
	}

	return outputCircuit;
}



#endif
