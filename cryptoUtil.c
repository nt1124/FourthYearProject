#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <string.h>
#include <time.h>
#include "cryptoUtil.h"


unsigned char *encryptMultiple(unsigned char **keyList, int numKeys, const unsigned char *toEncrypt)
{
	AES_KEY enc_key;
	unsigned char *ciphertext = (unsigned char*) calloc(16, sizeof(unsigned char));
	int i;

	AES_set_encrypt_key( keyList[0], 128, &enc_key );
	AES_encrypt(toEncrypt, ciphertext, &enc_key);

	for(i = 1; i < numKeys; i ++)
	{
		AES_set_encrypt_key( keyList[i], 128, &enc_key );
		AES_encrypt(ciphertext, ciphertext, &enc_key);
	}

	return ciphertext;
}


unsigned char *decryptMultiple(unsigned char **keyList, int numKeys, const unsigned char *toDecrypt)
{
	AES_KEY dec_key;
	unsigned char *ciphertext = (unsigned char*) calloc(16, sizeof(unsigned char));
	int i;

	AES_set_decrypt_key( keyList[numKeys - 1], 128, &dec_key );
	AES_decrypt(toDecrypt, ciphertext, &dec_key);

	for(i = 1; i < numKeys; i ++)
	{
		AES_set_decrypt_key( keyList[numKeys - i - 1], 128, &dec_key );
		AES_decrypt(ciphertext, ciphertext, &dec_key);
	}

	return ciphertext;
}


void generateRandomAESKeys(AES_KEY *enc_key, AES_KEY *dec_key)
{
	unsigned char *rawKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	RAND_bytes(rawKey, 16);

	AES_set_encrypt_key(rawKey, 128, enc_key);
	AES_set_decrypt_key(rawKey, 128, dec_key);
}


void testAES()
{
	int i;
	unsigned char text[] = "hello world!";
	unsigned char *enc_out = (unsigned char*) malloc(80 * sizeof(char)); 
	unsigned char *dec_out = (unsigned char*) malloc(80 * sizeof(char));

	AES_KEY enc_key, dec_key;

	generateRandomAESKeys(&enc_key, &dec_key);
	AES_encrypt(text, enc_out, &enc_key);  
	AES_decrypt(enc_out, dec_out, &dec_key);

	printf("Original:\t");
	for(i = 0; i < 16; i ++)
		printf("%02X ",*(text+i));

	printf("\nEncrypted:\t");
	for(i = 0; i < 16; i ++)
		printf("%02X ",*(enc_out+i));

	printf("\nDecrypted:\t");
	for(i = 0; i < 16; i ++)
		printf("%02X ",*(dec_out+i));
	printf("\n");

	free(enc_out);
	free(dec_out);
}



//Used in the 2009 paper before introduction of AES-NI made AES faster
unsigned char *sha256Digest(unsigned char* msgStr, int msgLength)
{
	SHA256_CTX sha256Ctx;
	SHA256_Init(&sha256Ctx);
	unsigned char *msgDigest = (unsigned char *) calloc(SHA256_DIGEST_LENGTH, sizeof(unsigned char));

	SHA256_Update(&sha256Ctx, msgStr, msgLength);

	SHA256_Final(msgDigest, &sha256Ctx);

	return msgDigest;
}


unsigned char *kdf2009Smart(unsigned char* key1, unsigned char* key2, unsigned char* salt,
							int keyLength, int msgLength, int saltLength)
{
	int i;
	unsigned char *key1Salt = (unsigned char *) calloc(keyLength + saltLength, sizeof(unsigned char));
	unsigned char *key2Salt = (unsigned char *) calloc(keyLength + saltLength, sizeof(unsigned char));
	unsigned char *tempMsg1;
	unsigned char *tempMsg2;
	unsigned char *output = (unsigned char *) calloc(msgLength, sizeof(unsigned char));

	strncpy((char*)key1Salt, (char*)key1, keyLength);
	strncpy((char*)(key1Salt + keyLength), (char*)salt, saltLength);
	tempMsg1 = sha256Digest(key1Salt, keyLength + saltLength);

	strncpy((char*)key2Salt, (char*)key2, keyLength);
	strncpy((char*)(key2Salt + keyLength), (char*)salt, saltLength);
	tempMsg2 = sha256Digest(key2Salt, keyLength + saltLength);

	for(i = 0; i < msgLength; i ++)
		output[i] = tempMsg1[i] ^ tempMsg2[i];

	return output;
}


unsigned char *enc2009Smart(unsigned char* key1, unsigned char* key2, unsigned char* salt, unsigned char *msg,
							int keyLength, int msgLength, int saltLength)
{
	unsigned char *kdfOutput = kdf2009Smart(key1, key2, salt, keyLength, msgLength, saltLength);
	unsigned char *output = (unsigned char *) calloc(msgLength, sizeof(unsigned char));
	int i;

	for(i = 0; i < msgLength; i ++)
		output[i] = kdfOutput[i] ^ msg[i];

	return output;
}
