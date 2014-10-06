#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS


typedef struct outputRow
{
	struct outputRow* zeroPointer;
	struct outputRow* onePointer;
	
	unsigned char outputValue[16];
} outputRow;


typedef struct gate
{
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	int *outputTable;
} gate;


typedef struct gateOrWire
{
	int G_ID;
	char typeTag;		// G = Gate, W = wire
	char wireValue;
	char outputFlag;

	struct gate *gate_data;
} gateOrWire;


void printGate(struct gate *input);
void printGateOrWire(struct gateOrWire *input);

struct gate *processGate(char* line, int strIndex);
struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex);
struct gateOrWire *processGateLine(char *line);
struct gateOrWire **readInCircuit(char* filepath, int numGates);

int count_lines_of_file(char * filepath);

#endif

