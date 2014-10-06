#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.c"
#include <openssl/aes.h>
static const unsigned char key[] = {
  	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

void aesTest()
{
    int i;
    unsigned char text[]="hello world!";
    unsigned char * enc_out = malloc(80*sizeof(char)); 
    unsigned char * dec_out = malloc(80*sizeof(char));

    AES_KEY enc_key, dec_key;

    AES_set_encrypt_key(key, 128, &enc_key);
    AES_encrypt(text, enc_out, &enc_key);  


    AES_set_decrypt_key(key,128,&dec_key);
    AES_decrypt(enc_out, dec_out, &dec_key);


    printf("Original:\t");
    for(i=0;*(text+i)!=0x00;i++)
        printf("%X ",*(text+i));

    printf("\nEncrypted:\t");
    for(i=0;*(enc_out+i)!=0x00;i++)
        printf("%X ",*(enc_out+i));

    printf("\nDecrypted:\t");
    for(i=0;*(dec_out+i)!=0x00;i++)
        printf("%X ",*(dec_out+i));
    printf("\n");

    free(enc_out);
    free(dec_out);

}


void runCircuit( struct gateOrWire **inputCircuit, int numGates )
{
	int i, j, tempIndex, numInputs;
	char outputTableIndex, tempValue;
	struct gate *currentGate;

	for(i = 0; i < numGates; i ++)
	{
		if( 'G' == inputCircuit[i] -> typeTag )
		{
			outputTableIndex = 0;
			currentGate = inputCircuit[i] -> gate_data;
			numInputs = currentGate -> numInputs;

			for(j = 0; j < numInputs; j ++)
			{
				tempIndex = currentGate -> inputIDs[numInputs - j - 1];
				outputTableIndex <<= 1;
				outputTableIndex += inputCircuit[tempIndex] -> wireValue;

				inputCircuit[i] -> wireValue = currentGate -> outputTable[outputTableIndex];
			}
		}
	}
}


int main(int argc, char *argv[])
{
	int i;
	srand( time(NULL) );

	if( 2 != argc )
	{
		char *circuitFilepath = argv[1];
		char *formatFilepath = argv[2];

		int numGates = count_lines_of_file(circuitFilepath);
		int numInputOutputs = count_lines_of_file(formatFilepath);

		struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
		struct formatItem **inputFormats = readFormatFile(formatFilepath, numInputOutputs);

		char tempAlice[] = "And.alice.input";
		char tempBob[] = "And.bob.input";
		readInputDetailsFile( tempAlice, inputCircuit );
		readInputDetailsFile( tempBob, inputCircuit );

		runCircuit( inputCircuit, numGates );
		printAllOutput(inputCircuit, numGates);

		aesTest();
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}