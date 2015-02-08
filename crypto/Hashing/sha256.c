// Note 1: All variables are 32 bit uintegers and addition is calculated modulo 2^32
// Note 2: For each round, there is one round constant k[i] and one entry in the message schedule array w[i], 0 ≤ i ≤ 63
// Note 3: The compression function uses 8 working variables, a through h
// Note 4: Big-endian convention is used when expressing the constants in this pseudocode,
//         and when parsing message block data from bytes to words, for example,
//         the first word of the input message "abc" after padding is 0x61626380

typedef unsigned int uint;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


const uint k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
					0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
					0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
					0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
					0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
					0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
					0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
					0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

unsigned char *reverseUIntShort(unsigned int input)
{
	unsigned char *before = (unsigned char*) calloc(sizeof(unsigned int), sizeof(unsigned char));	
	unsigned char *after = (unsigned char*) calloc(sizeof(unsigned int), sizeof(unsigned char));
	int i, j = sizeof(unsigned int) - 1;

	memcpy(before, &input, sizeof(unsigned int));

	for(i = 0; i < sizeof(unsigned int); i ++)
	{
		after[j--] = before[i];
	}

	free(before);

	return after;
}

unsigned char *reverseUInt(unsigned long int input)
{
	unsigned char *before = (unsigned char*) calloc(sizeof(unsigned long int), sizeof(unsigned char));	
	unsigned char *after = (unsigned char*) calloc(sizeof(unsigned long int), sizeof(unsigned char));
	int i, j = sizeof(unsigned long int) - 1;

	memcpy(before, &input, sizeof(unsigned long int));

	for(i = 0; i < sizeof(unsigned long int); i ++)
	{
		after[j--] = before[i];
	}

	free(before);

	return after;
}

unsigned char *reverseUChar(unsigned char *input, int inputLength)
{	
	unsigned char *after = (unsigned char*) calloc(inputLength, sizeof(unsigned char));
	int i, j = inputLength - 1;

	for(i = 0; i < inputLength; i ++)
	{
		after[j--] = input[i];
	}

	return after;
}


uint rotr(uint value, int shift)
{
	return (value >> shift) | (value << (32 - shift));
}


unsigned int rotl(unsigned int value, int shift) {
    return (value << shift) | (value >> (32 - shift));
}


uint ch(uint e, uint f, uint g, uint h, uint k_i, uint w_i)
{
	uint s_1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
	uint ch = (e & f) ^ ((!e) & g);
	uint temp1 = h + s_1 + ch + k_i + w_i;

	return temp1;
}

uint maj(uint a, uint b, uint c)
{
	uint s_0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
	uint maj = (a & b) ^ (a & c) ^ (b & c);
	uint temp2 = s_0 + maj;

	return temp2;
}


unsigned char *sha_256_hash(unsigned char *input, int inputLen)
{
	uint H[8] = {0x6a09e667, 0xbb67ae85,
				 0x3c6ef372, 0xa54ff53a,
				 0x510e527f, 0x9b05688c,
				 0x1f83d9ab, 0x5be0cd19};
	unsigned char *digest = (unsigned char*) calloc(32, sizeof(unsigned char));

	unsigned long int ul_length = (unsigned long int) 8*inputLen;
	uint *W = (uint*) calloc(64, sizeof(uint));;
	unsigned char *pre_proc_input;
	int requiredLength = (inputLen + 1);
	int index, blocks, i, j;
	uint s_0, s_1, maj, ch, temp1, temp2;
	uint a, b, c, d, e, f, g, h;

	while(56 != requiredLength % 64)
	{
		requiredLength ++;
	}

	blocks = 64;
	while(blocks < requiredLength)
	{
		requiredLength ++;
	}

	pre_proc_input = (unsigned char*) calloc(64, sizeof(unsigned char));


	memcpy(pre_proc_input, input, inputLen);
	pre_proc_input[inputLen] ^= 0x80;
	unsigned char *big_endian_length = reverseUInt(ul_length);
	memcpy(pre_proc_input + 56, big_endian_length, sizeof(unsigned long int));


	blocks /= 64;

	for(i = 0; i < blocks; i ++)
	{
		index = i*64;
		for(j = 0; j < 16; j ++)
		{
			W[j] = (pre_proc_input[index+j*4]<<24) | (pre_proc_input[index+j*4+1]<<16) | 
                    (pre_proc_input[index+j*4+2]<<8) | (pre_proc_input[index+j*4+3]);
		}

		// Extend the first 16 words into the remaining 48 words w[16..63] of the message schedule array:
		for(j = 16; j < 64; j ++)
		{
			s_0 = ( rotr(W[j - 15], 7) ^ rotr(W[j - 15], 18) ^ (W[j - 15] >> 3) );
			s_1 = ( rotr(W[j - 2], 17) ^ rotr(W[j - 2], 19) ^ (W[j - 2] >> 10) );
			W[j] = W[j - 16] + s_0 + W[j - 7] + s_1;
		}


		a = H[0];		b = H[1];
		c = H[2];		d = H[3];
		e = H[4];		f = H[5];
		g = H[6];		h = H[7];

		for(j = 0; j < 64; j ++)
		{
			s_1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
			ch = (e & f) ^ (~(e) & g);
			temp1 = h + s_1 + ch + k[j] + W[j];

			s_0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
			maj = (a & b) ^ (a & c) ^ (b & c);
			temp2 = s_0 + maj;

			h = g;
			g = f;
			f = e;
			e = d + temp1;
			d = c;
			c = b;
			b = a;
			a = temp1 + temp2;
		}			
		H[0] += a;
		H[1] += b;
		H[2] += c;
		H[3] += d;
		H[4] += e;
		H[5] += f;
		H[6] += g;
		H[7] += h;
	}

	unsigned char *tempChars;
	for(i = 0; i < 8; i ++)
	{
		tempChars = reverseUIntShort(H[i]);
		memcpy(digest + i * sizeof(uint), tempChars, sizeof(uint));

	}

	return digest;
}
