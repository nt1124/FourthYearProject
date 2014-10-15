#include "crypto/OT/otBase.cpp"
#include "crypto/gmpUtils.cpp"

#include <gmp.h>

/*	
Send runs the following protocol:
	WAIT for message (h0,h1) from R
	SAMPLE a random value r in  [0, . . . , q-1] 
	COMPUTE:
		+	u = g^r
		+	k0 = h0^r
		+	v0 = x0 XOR KDF(|x0|,k0) - in byteArray scenario.
				OR x0*k0			 - in GroupElement scenario.
		+	k1 = h1^r
		+	v1 = x1 XOR KDF(|x1|,k1) - in byteArray scenario.
				OR x1*k1 			 - in GroupElement scenario.
	SEND (u,v0,v1) to R
	OUTPUT nothing
*/

class otSemiHonestSender : public otBaseSender
{
		void 	preCompute();
		int  	send(unsigned char *buffer, int bufferSize);	// Will need to also pass a socket in future
		int 	listen(unsigned char *buffer, int bufferSize);	// Ditto
		int 	transfer();

		mpz_t dlogGroupOrder;
		mpz_t gen;

		mpz_t qMinusOne;
		gmp_randstate_t state;

		private:
			void otSemiHonestSender::computeU(mpz_t *u, mpz_t r);
			void otSemiHonestSender::computeK0(mpz_t *k0, mpz_t r, mpz_t message);
			void otSemiHonestSender::computeK1(mpz_t *k1, mpz_t r, mpz_t message);
};


void preCompute()
{
	mpz_init(dlogGroupOrder);
	mpz_init(gen);
	mpz_init(qMinusOne);

	getPrimeGMP(dlogGroupOrder, state, 1024);
	mpz_sub_ui(qMinusOne, dlogGroupOrder, 1);
	mpz_urandomm(gen, state, qMinusOne);
}


int otSemiHonestSender::send(unsigned char *buffer, int bufferSize)
{

}


int otSemiHonestSender::transfer(mpz_t message)
{
	//WAIT for message (h0,h1) from R
	
	//SAMPLE a random value r in  [0, . . . , q-1] 
	mpz_t r, u, k0, k1;
	mpz_init(r);
	mpz_init(u);
	mpz_init(k0);
	mpz_init(k1);

	mpz_urandomm(r, state, qMinusOne);
	
	//Compute u, k0, k1
	computeU(u, r);
	computeK0(k0, r, message);
	computeK1(k1, r, message);
	
	// OTSMsg messageToSend = computeTuple(input, u, k0, k1);
	// sendTupleToReceiver(channel, messageToSend);
	// Send our reply!
}


void otSemiHonestSender::computeU(mpz_t *u, mpz_t r)
{
	//Calculate u = g^r.
	mpz_powm(*u, gen, r, dlogGroupOrder);
}

void otSemiHonestSender::computeK0(mpz_t *k0, mpz_t r, mpz_t h0)
{
	//Calculate k0 = h0^r.
	mpz_powm(*k0, h0, r, dlogGroupOrder);
}


void otSemiHonestSender::computeK1(mpz_t *k1, mpz_t r, mpz_t h1) 
{	
	//Calculate k1 = h1^r.
	mpz_powm(*k1, h1, r, dlogGroupOrder);
}


int otSemiHonestSender::otSemiHonestSender()
{
    unsigned long int seed = time(NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, seed);

    preCompute();
}


// OTSMsg
void computeTuple(OTSInput input, mpz_t u, mpz_t k0, mpz_t k1)
{
	//If input is not instance of OTSOnByteArrayInput, throw Exception.
	if(!(input instanceof OTOnByteArraySInput))
	{
		throw new IllegalArgumentException("x0 and x1 should be binary strings.");
	}
	
	unsigned char *x0 = 
	unsigned char *x1 = 
	// ((OTOnByteArraySInput) input).getX0();
	// ((OTOnByteArraySInput) input).getX1();

	//If x0, x1 are not of the same length, throw Exception.
	if(x0.length != x1.length)
	{
		throw new IllegalArgumentException("x0 and x1 should be of the same length.");
	}
	
	//Calculate v0:
	byte[] k0Bytes = dlog.mapAnyGroupElementToByteArray(k0);
	
	int len = x0.length;
	byte[] v0 = kdf.deriveKey(k0Bytes, 0, k0Bytes.length, len).getEncoded();
	
	//Xores the result from the kdf with x0.
	for(int i = 0; i < len; i ++)
	{
		v0[i] = (byte) (v0[i] ^ x0[i]);
	}
	
	//Calculate v1:
	byte[] k1Bytes = dlog.mapAnyGroupElementToByteArray(k1);
	byte[] v1 = kdf.deriveKey(k1Bytes, 0, k1Bytes.length, len).getEncoded();
	
	//Xores the result from the kdf with x1.
	for(int i = 0; i < len; i ++)
	{
		v1[i] = (byte) (v1[i] ^ x1[i]);
	}
	
	//Create and return sender message.
	return new OTSemiHonestDDHOnByteArraySenderMsg(u.generateSendableData(), v0, v1);
}


/*	
Receive runs the following protocol:
 	SAMPLE random values alpha in Zq and h in the DlogGroup 
	COMPUTE h0,h1 as follows:
		1.	If sigma = 0 then h0 = g^alpha  and h1 = h
		2.	If sigma = 1 then h0 = h and h1 = g^alpha 
	SEND (h0,h1) to S
	WAIT for the message (u, v0,v1) from S
	COMPUTE kSigma = (u)^alpha							- in byte array scenario
		 OR (kSigma)^(-1) = u^(-alpha)					- in GroupElement scenario
	OUTPUT  xSigma = vSigma XOR KDF(|cSigma|,kSigma)	- in byte array scenario
		 OR xSigma = vSigma * (kSigma)^(-1) 			- in GroupElement scenario
*/