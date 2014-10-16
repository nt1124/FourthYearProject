/*
typedef struct outputEncRow
{
	struct outputEncRow *keyChoice[2];

	unsigned char outputValue;
	unsigned char *outputEncValue;
}outputEncRow;


typedef struct bitsGarbleKeys
{
	unsigned char key0[16];
	unsigned char key1[16];
}bitsGarbleKeys;


typedef struct gate
{
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	struct outputEncRow *outputTreeEnc;
	unsigned char *outputTable[16];

	struct bitsGarbleKeys **inputKeySet;
} gate;


typedef struct gateOrWire
{
	int G_ID;
	char typeTag;		// G = Gate, W = wire
	char wireValue;
	unsigned char *wireEncValue;
	char outputFlag;

	struct bitsGarbleKeys *outputGarbleKeys;
	struct gate *gate_data;
} gateOrWire;
*/