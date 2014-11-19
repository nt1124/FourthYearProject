#ifndef CIRCUIT_EXECUTOR
#define	CIRCUIT_EXECUTOR

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"
#include "fileUtils/fileUtilsFP.h"
#include "fileUtils/fileUtilsRTL.h"
#include "comms/sockets.h"



void readInputLinesExec(int sockfd, char *line, struct gateOrWire **inputCircuit, gmp_randstate_t *state, struct rsaPrivKey *SKi);
void readInputDetailsFileExec(int sockfd, char *filepath, struct gateOrWire **inputCircuit,
											gmp_randstate_t *state, struct rsaPrivKey *SKi);
void runCircuitExec( struct gateOrWire **inputCircuit, int numGates, int sockfd, char *filepath, int *execOrder );
int receiveNumGates(int readSocket);
int *receiveExecOrder(int readSocket, int numGates);
struct gateOrWire **receiveCircuit(int numGates, int readSocket);


#include "circuitExecutor.c"

#endif