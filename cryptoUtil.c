#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/bn.h>
#include "cryptoUtil.h"


void aesTest()
{
	const unsigned char testKey[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
									0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	int i;
	unsigned char text[] = "hello world!";
	unsigned char * enc_out = malloc(80 * sizeof(char)); 
	unsigned char * dec_out = malloc(80 * sizeof(char));

	AES_KEY enc_key, dec_key;

	AES_set_encrypt_key(testKey, 128, &enc_key);
	AES_encrypt(text, enc_out, &enc_key);  


	AES_set_decrypt_key(testKey,128,&dec_key);
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


unsigned char *sha256Digest(unsigned char* msgStr, int msgLength)
{
	SHA256_CTX sha256Ctx;
	SHA256_Init(&sha256Ctx);
	unsigned char *msgDigest = calloc(SHA256_DIGEST_LENGTH, sizeof(unsigned char));

	SHA256_Update(&sha256Ctx, msgStr, msgLength);

	SHA256_Final(msgDigest, &sha256Ctx);

	return msgDigest;
}


unsigned char *kdf2009Smart(unsigned char* key1, unsigned char* key2, unsigned char* salt,
							int keyLength, int msgLength, int saltLength)
{
	int i;
	unsigned char *key1Salt = calloc(keyLength + saltLength, sizeof(unsigned char));
	unsigned char *key2Salt = calloc(keyLength + saltLength, sizeof(unsigned char));
	unsigned char *tempMsg1;
	unsigned char *tempMsg2;
	unsigned char *output = calloc(msgLength, sizeof(unsigned char));

	strncpy(key1Salt, key1, keyLength);
	strncpy(key1Salt + keyLength, salt, saltLength);
	tempMsg1 = sha256Digest(key1Salt, keyLength + saltLength);

	strncpy(key2Salt, key2, keyLength);
	strncpy(key2Salt + keyLength, salt, saltLength);
	tempMsg2 = sha256Digest(key2Salt, keyLength + saltLength);

	for(i = 0; i < msgLength; i ++)
		output[i] = tempMsg1[i] ^ tempMsg2[i];

	return output;
}


unsigned char *enc2009Smart(unsigned char* key1, unsigned char* key2, unsigned char* salt, unsigned char *msg,
							int keyLength, int msgLength, int saltLength)
{
	unsigned char *kdfOutput = kdf2009Smart(key1, key2, salt, keyLength, msgLength, saltLength);
	unsigned char *output = calloc(msgLength, sizeof(unsigned char));
	int i;

	for(i = 0; i < msgLength; i ++)
		output[i] = kdfOutput[i] ^ msg[i];

	return output;
}
