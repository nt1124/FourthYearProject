#include <math.h>
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
		HMAC hmac;
};

/*
#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>

    
    // Be careful of the length of string with the choosen hash engine. SHA1 needed 20 characters.
    // Change the length accordingly with your choosen hash engine.     
    unsigned char* result;
    unsigned int len = 20;

    result = (unsigned char*)malloc(sizeof(char) * len);

    HMAC_CTX ctx;
    HMAC_CTX_init(&ctx);

    // Using sha1 hash engine here.
    // You may use other hash engines. e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
    HMAC_Init_ex(&ctx, key, strlen(key), EVP_sha1(), NULL);
    HMAC_Update(&ctx, (unsigned char*)&data, strlen(data));
    HMAC_Final(&ctx, result, &len);
    HMAC_CTX_cleanup(&ctx);

    printf("HMAC digest: ");
    for (int i = 0; i != len; i++)
        printf("%02x", (unsigned int)result[i]);
    printf("\n");
*/

int KDF::initialise()
{
	// The secret key for hashing
	const char key[] = "012345678";
	int key_len = 9;
	char data[] = "hello world";
	
	HMAC_CTX_init(ctx);
	HMAC_Init_ex(ctx, key, key_len, EVP_sha512, NULL);

}


void KDF::nextRound(int outLen, int ivLength, unsigned char *iv, int hmacLength, unsigned char *outBytes, unsigned char *intermediateOutBytes)
{
	int rounds = (int) ceil((float)outLen/(float)hmacLength); //the smallest number so that  hmacLength * rounds >= outLen
		
		int currentInBytesSize;	//the size of the CTXInfo and also the round;
		
		if(iv != NULL)
			currentInBytesSize = hmacLength + ivLength + 1;//the size of the CTXInfo and also the round;
		else
			currentInBytesSize = hmacLength + 1;//the size without the CTXInfo and also the round;
		
		//the result of the current computation
		unsigned char[] currentInBytes = new unsigned char[currentInBytesSize];
		
		
		int roundIndex;
		//for rounds 2 to t 
		if(iv!= NULL)
		{
			// In case we have an iv. puts it (ctxInfo after the K from the previous round at position hmacLength).
			strncpy( (currentInBytes + hmacLength), iv, ivLength);
		}		
		
		for(int i = 2; i <= rounds; i++){
			
			roundIndex = i; //creates the round integer for the data
			
			//copies the output of the last results
			strncpy(currentInBytes, intermediateOutBytes, hmacLength);
				
			//copies the round integer to the data array
			currentInBytes[currentInBytesSize - 1] = roundIndex & 0x000000FF;

			//operates the hmac to get the round output 
			hmac.computeBlock(currentInBytes, 0, currentInBytes.length, intermediateOutBytes, 0);
			
			if(i == rounds)
			{
				//We fill the rest of the array with a portion of the last result.
				//Copies the results to the output array
				strncpy( (outBytes + hmacLength*(i-1)), intermediateOutBytes, outLen - hmacLength*(i-1) );
			}
			else
			{
				//copies the results to the output array
				strncpy( (outBytes + hmacLength*(i-1)), intermediateOutBytes, hmacLength );
			}				
		}

}