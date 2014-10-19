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

/*
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
*/

/*
HMAC_CTX *initialiseHMAC()
{
	// The secret key for hashing
	const char key[] = "012345678";
	int key_len = 9;
	HMAC_CTX *ctx;
	
	HMAC_CTX_init(ctx);
	HMAC_Init_ex(ctx, key, key_len, EVP_sha512(), NULL);

	unsigned char *meh = HMAC(EVP_sha512(), key, key_len, const unsigned char *d, int n,
               				unsigned char *md, unsigned int *md_len);

	return ctx;
}


unsigned char *computeBlock(HMAC_CTX *ctx, unsigned char *currentInBytes, unsigned int curLength,
								unsigned char *outputBytes, unsigned int *outputLength)
{

    HMAC_Update(ctx, currentInBytes, curLength);
    HMAC_Final(ctx, outputBytes, outputLength);
}

int clearup(HMAC_CTX *ctx)
{
    HMAC_CTX_cleanup(ctx);
}
*/

// private void nextRounds(int outLen, byte[] iv, int hmacLength, byte[] outBytes, byte[] intermediateOutBytes)
void nextRound(unsigned char *outBytes, unsigned int outLen,
				unsigned char *iv, unsigned int ivLength, 
				unsigned char *intermediateOutBytes, unsigned int intermediateLength, int hmacLength,
				const char *key, unsigned int keyLength)
{
	// The smallest number so that  hmacLength * rounds >= outLen
	int rounds = (int) ceil((float)outLen/(float)hmacLength);
	int i;		
	int currentInBytesLength;
	//the size of the CTXInfo and also the round;
	
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
	
	for(i = 2; i <= rounds; i++)
	{
		roundIndex = i; //creates the round integer for the data

		// Copies the output of the last results
		strncpy( (char*)currentInBytes, (char*)intermediateOutBytes, hmacLength );
			
		// Copies the round integer to the data array
		currentInBytes[currentInBytesLength - 1] = roundIndex & 0x000000FF;

		// Operates the hmac to get the round output 
		HMAC(EVP_sha512(), key, keyLength, currentInBytes, currentInBytesLength, outBytes, &outputLen);

		if(i == rounds)
		{
			// We fill the rest of the array with a portion of the last result. Copies the results to the output array
			strncpy( (char*)(outBytes + hmacLength*(i-1)), (char*)intermediateOutBytes, outLen - hmacLength*(i-1) );
		}
		else
		{
			// Copies the results to the output array
			strncpy( (char*)(outBytes + hmacLength*(i-1)), (char*)intermediateOutBytes, hmacLength );
		}				
	}
}

// void firstRound(byte[] outBytes, byte[] iv, byte[] intermediateOutBytes, int outLength)
void firstRound(unsigned char *outBytes, unsigned int outLength,
					unsigned char *iv, unsigned int ivLength,
					unsigned char *intermediateOutBytes, unsigned int intermediateLength,
					const char *key, unsigned int keyLength)
{
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
	// computeBlock(ctx, firstRoundInput, firstRoundLength, intermediateOutBytes, &intermediateLength);
	// HMAC(EVP_sha512(), key, key_len, currentInBytes, curLength, outputBytes, &outputLength);
	
	//copies the results to the output array
	strncpy((char*)intermediateOutBytes, (char*)outBytes, outLength);
}


unsigned char *deriveKey(unsigned char *entropySource, int entropySourceLength,
						unsigned int inOff, unsigned int inLen, unsigned int outputLen,
						unsigned char *iv, unsigned int ivLength)
{
	const char key[] = "012345678";
	unsigned int keyLength = 9;
	//checks that the offset and length are correct
	if( (inOff > entropySourceLength) || (inOff + inLen > entropySourceLength) )
		exit(1);
	
	//Sets the hmac object with a fixed key that was randomly generated once. This is done every time a new derived key is requested otherwise the result of deriving
	//a key from the same entropy source will be different in subsequent calls to this function (as long as the same instance of HKDF is used). 
	// hmac.setKey(new SecretKeySpec(Hex.decode("606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeaf"), ""));

	unsigned int hmacLength = 512;                           //the size of the output of the hmac.
	unsigned int currentInBytesLength = 512;
	unsigned char *outBytes = (unsigned char*) calloc(outputLen, sizeof(unsigned char));	//the output key
	unsigned char *roundKey = (unsigned char*) calloc(hmacLength, sizeof(unsigned char));	//PRK from the pseudocode
	unsigned char *intermediateOutBytes = (unsigned char*) calloc(hmacLength, sizeof(unsigned char)); //round result K(i) in the pseudocode
	unsigned char *currentInBytes = (unsigned char*) calloc(currentInBytesLength, sizeof(unsigned char)); 
	
	
	//first computes the new key. The new key is the result of computing the hmac function.
	// computeBlock(ctx,  currentInBytes, curLength, outputBytes, &outputLength);

	HMAC(EVP_sha512(), key, keyLength, currentInBytes, currentInBytesLength, outBytes, &outputLen);
	//init the hmac with the new key. From now on this is the key for all the rounds.
	// hmac.setKey(new SecretKeySpec(roundKey, "HKDF"));
	
	//calculates the first round
	//K(1) = HMAC(PRK,(CTXinfo,1)) [key=PRK, data=(CTXinfo,1)]

	/*
	void firstRound(unsigned char *outBytes, unsigned int outLength,
					unsigned char *iv, unsigned int ivLength,
					unsigned char *intermediateOutBytes, unsigned int intermediateLength,
					unsigned char *key, unsigned int keyLength)
	*/
	if (outputLen < hmacLength)
		firstRound(outBytes, outputLen, iv, ivLength, intermediateOutBytes, outputLen, key, keyLength);
	else
		firstRound(outBytes, outputLen, iv, ivLength, intermediateOutBytes, hmacLength, key, keyLength);
	// firstRound( outBytes, iv, intermediateOutBytes, hmacLength);
	
	//calculates the next rounds
	//FOR i = 2 TO t
	//K(i) = HMAC(PRK,(K(i-1),CTXinfo,i)) [key=PRK, data=(K(i-1),CTXinfo,i)]
	nextRound( iv, ivLength, outBytes, outputLen, intermediateOutBytes, hmacLength, hmacLength, key, keyLength );
	
	//creates the secret key from the generated bytes
	return outBytes;

}


int main()
{
	// HMAC_CTX *ctx = initialiseHMAC();

	return 0;
}