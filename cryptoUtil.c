#include <openssl/aes.h>
#include <openssl/sha.h>
#include "cryptoUtil.h"
#include <gcrypt.h>


#define GCRY_CIPHER GCRY_CIPHER_SERPENT128   // Pick the cipher here
#define GCRY_C_MODE GCRY_CIPHER_MODE_ECB // Pick the cipher mode here
#define GCRY_CIPHER_ASYM GCRY_CIPHER_SERPENT128   // Pick the cipher here

void aesTestGCrypt(void)
{
    gcry_error_t     gcryError;
    gcry_cipher_hd_t gcryCipherHd;
    size_t           index;
 
    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    size_t blkLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER);
    char * txtBuffer = "123456789 abcdefghijklmnopqrstuvwzyz ABCDEFGHIJKLMNOPQRSTUVWZYZ";
    size_t txtLength = strlen(txtBuffer)+1; // string plus termination
    char * encBuffer = malloc(txtLength);
    char * outBuffer = malloc(txtLength);
    char * aesSymKey = "one test AES key"; // 16 bytes
    char * iniVector = "a test ini value"; // 16 bytes
 
    gcryError = gcry_cipher_open(
        &gcryCipherHd, // gcry_cipher_hd_t *
        GCRY_CIPHER,   // int
        GCRY_C_MODE,   // int
        0);            // unsigned int
    if (gcryError)
    {
        printf("gcry_cipher_open failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_open    worked\n");
 
    gcryError = gcry_cipher_setkey(gcryCipherHd, aesSymKey, keyLength);
    if (gcryError)
    {
        printf("gcry_cipher_setkey failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setkey  worked\n");
 
    gcryError = gcry_cipher_setiv(gcryCipherHd, iniVector, blkLength);
    if (gcryError)
    {
        printf("gcry_cipher_setiv failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setiv   worked\n");
 
    gcryError = gcry_cipher_encrypt(
        gcryCipherHd, // gcry_cipher_hd_t
        encBuffer,    // void *
        txtLength,    // size_t
        txtBuffer,    // const void *
        txtLength);   // size_t
    if (gcryError)
    {
        printf("gcry_cipher_encrypt failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_encrypt worked\n");
 
    gcryError = gcry_cipher_setiv(gcryCipherHd, iniVector, blkLength);
    if (gcryError)
    {
        printf("gcry_cipher_setiv failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setiv   worked\n");
 
    gcryError = gcry_cipher_decrypt(
        gcryCipherHd, // gcry_cipher_hd_t
        outBuffer,    // void *
        txtLength,    // size_t
        encBuffer,    // const void *
        txtLength);   // size_t
    if (gcryError)
    {
        printf("gcry_cipher_decrypt failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_decrypt worked\n");
 
    printf("keyLength = %d\n", keyLength);
    printf("blkLength = %d\n", blkLength);
    printf("txtLength = %d\n", txtLength);
    printf("aesSymKey = %s\n", aesSymKey);
    printf("iniVector = %s\n", iniVector);
    printf("txtBuffer = %s\n", txtBuffer);
 
    printf("encBuffer = ");
    for (index = 0; index<txtLength; index++)
        printf("%02X", (unsigned char)encBuffer[index]);
    printf("\n");
 
    printf("outBuffer = %s\n", outBuffer);
 
    // clean up after ourselves
    gcry_cipher_close(gcryCipherHd);
    free(encBuffer);
    free(outBuffer);
}



void asymTest(void)
{
    gcry_error_t     gcryError;
    gcry_cipher_hd_t gcryCipherHd;
    size_t           index;
 
    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER_ASYM);
    size_t blkLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER_ASYM);
    char * txtBuffer = "123456789 abcdefghijklmnopqrstuvwzyz ABCDEFGHIJKLMNOPQRSTUVWZYZ";
    size_t txtLength = strlen(txtBuffer)+1; // string plus termination
    char * encBuffer = malloc(txtLength);
    char * outBuffer = malloc(txtLength);
    char * aesSymKey = "one test AES key"; // 16 bytes
    char * iniVector = "a test ini value"; // 16 bytes
 
    gcryError = gcry_cipher_open(
        &gcryCipherHd, // gcry_cipher_hd_t *
        GCRY_CIPHER_ASYM,   // int
        GCRY_C_MODE,   // int
        0);            // unsigned int
    if (gcryError)
    {
        printf("gcry_cipher_open failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_open    worked\n");
 
    gcryError = gcry_cipher_setkey(gcryCipherHd, aesSymKey, keyLength);
    if (gcryError)
    {
        printf("gcry_cipher_setkey failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setkey  worked\n");
 
    gcryError = gcry_cipher_setiv(gcryCipherHd, iniVector, blkLength);
    if (gcryError)
    {
        printf("gcry_cipher_setiv failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setiv   worked\n");
 
    gcryError = gcry_cipher_encrypt(
        gcryCipherHd, // gcry_cipher_hd_t
        encBuffer,    // void *
        txtLength,    // size_t
        txtBuffer,    // const void *
        txtLength);   // size_t
    if (gcryError)
    {
        printf("gcry_cipher_encrypt failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_encrypt worked\n");
 
    gcryError = gcry_cipher_setiv(gcryCipherHd, iniVector, blkLength);
    if (gcryError)
    {
        printf("gcry_cipher_setiv failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_setiv   worked\n");
 
    gcryError = gcry_cipher_decrypt(
        gcryCipherHd, // gcry_cipher_hd_t
        outBuffer,    // void *
        txtLength,    // size_t
        encBuffer,    // const void *
        txtLength);   // size_t
    if (gcryError)
    {
        printf("gcry_cipher_decrypt failed:  %s/%s\n",
               gcry_strsource(gcryError),
               gcry_strerror(gcryError));
        return;
    }
    printf("gcry_cipher_decrypt worked\n");
 
    printf("keyLength = %d\n", keyLength);
    printf("blkLength = %d\n", blkLength);
    printf("txtLength = %d\n", txtLength);
    printf("aesSymKey = %s\n", aesSymKey);
    printf("iniVector = %s\n", iniVector);
    printf("txtBuffer = %s\n", txtBuffer);
 
    printf("encBuffer = ");
    for (index = 0; index<txtLength; index++)
        printf("%02X", (unsigned char)encBuffer[index]);
    printf("\n");
 
    printf("outBuffer = %s\n", outBuffer);
 
    // clean up after ourselves
    gcry_cipher_close(gcryCipherHd);
    free(encBuffer);
    free(outBuffer);
}


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
