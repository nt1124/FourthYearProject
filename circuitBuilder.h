#ifndef CIRCUIT_BUILDER
#define	CIRCUIT_BUILDER

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"
#include "fileUtils/fileUtilsFP.h"
#include "fileUtils/fileUtilsRTL.h"
#include "comms/sockets.h"



void readInputLinesBuilder(char *line, struct gateOrWire **inputCircuit);
void readInputDetailsFileBuilder(char *filepath, struct gateOrWire **inputCircuit);
void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int sockfd, mpz_t N );
void sendGate(struct gateOrWire *inputGW, int writeSocket);
void sendCircuit(int writeSocket, struct gateOrWire **inputCircuit, int numGates, int *execOrder);



#include "circuitBuilder.c"


#endif

