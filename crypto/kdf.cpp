#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/hmac.h>

/*
unsigned char *HMAC(const EVP_MD *evp_md, const void *key,
               int key_len, const unsigned char *d, int n,
               unsigned char *md, unsigned int *md_len);
void HMAC_CTX_init(HMAC_CTX *ctx);
int HMAC_Init(HMAC_CTX *ctx, const void *key, int key_len,
              const EVP_MD *md);
int HMAC_Init_ex(HMAC_CTX *ctx, const void *key, int key_len,
                  const EVP_MD *md, ENGINE *impl);
int HMAC_Update(HMAC_CTX *ctx, const unsigned char *data, int len);
int HMAC_Final(HMAC_CTX *ctx, unsigned char *md, unsigned int *len);
void HMAC_CTX_cleanup(HMAC_CTX *ctx);
void HMAC_cleanup(HMAC_CTX *ctx);
*/

class KDF
{
	private:
		HMAC_CTX *ctx;
	public:
		int initialise();
		int clearup();
		unsigned char *computeBlock(unsigned char *currentInBytes, unsigned int curLength, unsigned char *outputBytes, unsigned int *outputLength);
		void nextRound(unsigned char *outBytes, unsigned int outLen,
							unsigned char *iv, unsigned int ivLength, 
							unsigned char *intermediateOutBytes, unsigned int intermediateLength, int hmacLength);
		void firstRound(unsigned char *outBytes, unsigned int outLength,
						unsigned char *iv, unsigned int ivLength,
						unsigned char *intermediateOutBytes, unsigned int intermediateLength);
};

/*
result = (unsigned char*)malloc(sizeof(char) * len);

HMAC_CTX ctx;
HMAC_CTX_init(&ctx);

HMAC_Init_ex(&ctx, key, strlen(key), EVP_sha1(), NULL);
HMAC_Update(&ctx, (unsigned char*)&data, strlen(data));
HMAC_Final(&ctx, result, &len);
HMAC_CTX_cleanup(&ctx);
*/

int KDF::initialise()
{
	// The secret key for hashing
	const char key[] = "012345678";
	int key_len = 9;
	
	HMAC_CTX_init(ctx);
	HMAC_Init_ex(ctx, key, key_len, EVP_sha512(), NULL);
}


unsigned char *KDF::computeBlock(unsigned char *currentInBytes, unsigned int curLength,
								unsigned char *outputBytes, unsigned int *outputLength)
{
    HMAC_Update(ctx, currentInBytes, curLength);
    HMAC_Final(ctx, outputBytes, outputLength);
}


int KDF::clearup()
{
    HMAC_CTX_cleanup(ctx);
}


void KDF::nextRound(unsigned char *outBytes, unsigned int outLen,
					unsigned char *iv, unsigned int ivLength, 
					unsigned char *intermediateOutBytes, unsigned int intermediateLength, int hmacLength)
{
	// The smallest number so that  hmacLength * rounds >= outLen
	int rounds = (int) ceil((float)outLen/(float)hmacLength);
		
	int currentInBytesLength;	//the size of the CTXInfo and also the round;
	
	if(iv != NULL)
		currentInBytesLength = hmacLength + ivLength + 1;//the size of the CTXInfo and also the round;
	else
		currentInBytesLength = hmacLength + 1;//the size without the CTXInfo and also the round;
	
	// The result of the current computation
	unsigned char *currentInBytes = (unsigned char*) calloc( currentInBytesLength, sizeof(unsigned char) );
	int roundIndex;

	// For rounds 2 to t
	if(iv != NULL)
	{
		// In case we have an iv. puts it (ctxInfo after the K from the previous round at position hmacLength).
		strncpy( (char*)(currentInBytes + hmacLength), (char*)iv, ivLength );
	}
	
	for(int i = 2; i <= rounds; i++)
	{
		roundIndex = i; //creates the round integer for the data
		
		// Copies the output of the last results
		strncpy( (char*)currentInBytes, (char*)intermediateOutBytes, hmacLength );
			
		// Copies the round integer to the data array
		currentInBytes[currentInBytesLength - 1] = roundIndex & 0x000000FF;

		// Operates the hmac to get the round output 
		computeBlock(currentInBytes, currentInBytesLength, intermediateOutBytes, &intermediateLength);
		
		if(i == rounds)
		{
			// We fill the rest of the array with a portion of the last result.
			// Copies the results to the output array
			strncpy( (char*)(outBytes + hmacLength*(i-1)), (char*)intermediateOutBytes, outLen - hmacLength*(i-1) );
		}
		else
		{
			// Copies the results to the output array
			strncpy( (char*)(outBytes + hmacLength*(i-1)), (char*)intermediateOutBytes, hmacLength );
		}				
	}
}


void KDF::firstRound(unsigned char *outBytes, unsigned int outLength,
					unsigned char *iv, unsigned int ivLength,
					unsigned char *intermediateOutBytes, unsigned int intermediateLength)
{
	//round 1
	unsigned char *firstRoundInput;
	unsigned int firstRoundLength;

	if(iv != NULL)
		firstRoundLength = ivLength + 1;
	else
		firstRoundLength = 1;

	firstRoundInput = (unsigned char*) calloc( firstRoundLength, sizeof(unsigned char) );
	
	//copies the CTXInfo - iv
	if(iv != NULL)
		strncpy( (char*)iv, (char*)firstRoundInput, ivLength);

	//copies the integer with zero to the data array
	firstRoundInput[firstRoundLength - 1] = 0x01;
		
	//first computes the new key. The new key is the result of computing the hmac function.
	computeBlock(firstRoundInput, firstRoundLength, intermediateOutBytes, &intermediateLength);
	
	//copies the results to the output array
	strncpy((char*)intermediateOutBytes, (char*)outBytes, outLength);
}


unsigned char *KDF::deriveKey(unsigned char *entropySource, int inOff, unsigned int inLen, unsigned int outputLen,
						unsigned char *iv)
{
	//checks that the offset and length are correct
	if( (inOff > entropySource.length) || (inOff+inLen > entropySource.length) )
		throw new ArrayIndexOutOfBoundsException("wrong offset for the given input buffer");
	
	//Sets the hmac object with a fixed key that was randomly generated once. This is done every time a new derived key is requested otherwise the result of deriving
	//a key from the same entropy source will be different in subsequent calls to this function (as long as the same instance of HKDF is used). 
	hmac.setKey(new SecretKeySpec(Hex.decode("606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeaf"), ""));

	int hmacLength = hmac.getBlockSize();                           //the size of the output of the hmac.
	unsigned char *outBytes = (unsigned char*) calloc(outLen, sizeof(unsigned char));	//the output key
	unsigned char *roundKey = (unsigned char*) calloc(hmacLen, sizeof(unsigned char));	//PRK from the pseudocode
	unsigned char *intermediateOutBytes = (unsigned char*) calloc(hmacLen, sizeof(unsigned char)); //round result K(i) in the pseudocode
	
	
	//first computes the new key. The new key is the result of computing the hmac function.
	hmac.computeBlock(entropySource, 0, entropySource.length, roundKey, 0);
	computeBlock(currentInBytes, curLength,
				outputBytes, &outputLength)
	//init the hmac with the new key. From now on this is the key for all the rounds.
	// hmac.setKey(new SecretKeySpec(roundKey, "HKDF"));
	
	//calculates the first round
	//K(1) = HMAC(PRK,(CTXinfo,1)) [key=PRK, data=(CTXinfo,1)]
	if (outLen < hmacLength)
		firstRound(outBytes, outLen, iv, intermediateOutBytes );
	else
		firstRound(outBytes, iv, intermediateOutBytes, hmacLength);
	
	//calculates the next rounds
	//FOR i = 2 TO t
	//K(i) = HMAC(PRK,(K(i-1),CTXinfo,i)) [key=PRK, data=(K(i-1),CTXinfo,i)]
	nextRounds(iv, ivLength, outBytes, outLen,
			intermediateOutBytes, intermediateLength, hmacLength);
	
	//creates the secret key from the generated bytes
	return outBytes;

}


int main()
{
	KDF tempKDF;

	tempKDF.initialise();

	return 0;
}