/*
mpz_t *getRecombinationVector(mpz_t *shares, int numShares, int index, struct DDH_Group *group)
{
	mpz_t *vector, *secret, tempMPZ, top, bottom, bottomInv;
	int i, j;

	mpz_init(tempMPZ);
	mpz_init(top);
	mpz_init(bottom);
	mpz_init(bottomInv);

	vector = (mpz_t *) calloc(numShares, sizeof(mpz_t));
	secret = (mpz_t *) calloc(1, sizeof(mpz_t));

	for(i = 0; i < numShares; i ++)
	{
		mpz_set_ui(tempMPZ, 1);
		mpz_init(vector[i]);

		for(j = 0; j < numShares; j ++)
		{
			if(i != j)
			{
				mpz_ui_sub(top, 0, shares[j]);
				mpz_sub(bottom, shares[i], shares[j]);
				mpz_invert(bottomInv, bottom, group -> p);
				mpz_mul(bottom, top, bottomInv);

				mpz_mod(top, bottom, group -> p);
				mpz_mul(tempMPZ, vector[i], top);
			}
		}

		mpz_mod(vector[i], tempMPZ, group -> p);
	}

	mpz_clear(tempMPZ);
	mpz_clear(top);
	mpz_clear(bottom);
	mpz_clear(bottomInv);
}
*/


struct pubVSS_Box *init_VSS_PublicBox(int t, int n)
{
	struct pubVSS_Box *pub = (struct pubVSS_Box *) calloc(1, sizeof(struct pubVSS_Box));
	int i;


	pub -> t = t;
	pub -> n = n;

	pub -> commitments = (mpz_t *) calloc(t + 1, sizeof(mpz_t));
	for(i = 0; i < t + 1; i ++)
	{
		mpz_init(pub -> commitments[i]);
	}

	return pub;
}


struct pubVSS_Box *init_VSS_PublicBoxWithCoeffs(int t, int n, struct Fq_poly *poly, struct DDH_Group *group)
{
	struct pubVSS_Box *pub = (struct pubVSS_Box *) calloc(1, sizeof(struct pubVSS_Box));
	int i;


	pub -> t = t;
	pub -> n = n;

	pub -> commitments = (mpz_t *) calloc(t + 1, sizeof(mpz_t));
	for(i = 0; i < t + 1; i ++)
	{
		mpz_init(pub -> commitments[i]);
		mpz_powm(pub -> commitments[i], group -> g, poly -> coeffs[i], group -> p);
	}


	return pub;
}


struct sharingScheme *init_VSS_SharingScheme(int t, int n)
{
	struct sharingScheme *scheme = (struct sharingScheme *) calloc(1, sizeof(struct sharingScheme));
	int i;


	mpz_init(scheme -> secret);

	scheme -> shares = (mpz_t *) calloc(n, sizeof(mpz_t));

	for(i = 0; i < n; i ++)
	{
		mpz_init(scheme -> shares[i]);
	}


	return scheme;
}


struct Fq_poly *getPolyForScheme(int t, mpz_t secret, gmp_randstate_t state, struct DDH_Group *group)
{
	struct Fq_poly *toReturn = (struct Fq_poly*) calloc(1, sizeof(struct Fq_poly));
	int i;


	toReturn -> degree = t;
	toReturn -> coeffs = (mpz_t*) calloc(t + 1, sizeof(mpz_t));

	mpz_init_set(toReturn -> coeffs[0], secret);
	for(i = 1; i <= t; i ++)
	{
		mpz_init(toReturn -> coeffs[i]);
		mpz_urandomm(toReturn -> coeffs[i], state, group -> q);
	}


	return toReturn;
}


// IMPORTANT NOTE : You need t + 1 many shares to uncover the secret.
struct sharingScheme *VSS_Share(mpz_t secret, int t, int n, gmp_randstate_t state, struct DDH_Group *group)
{
	struct sharingScheme *scheme = init_VSS_SharingScheme(t, n);
	mpz_t *tempMPZ, secret_index;
	int i;


	mpz_init(secret_index);
	scheme -> poly = getPolyForScheme(t, secret, state, group);
	scheme -> pub = init_VSS_PublicBoxWithCoeffs(t, n, scheme -> poly, group);

	for(i = 0 ; i < n; i ++)
	{
		mpz_set_ui(secret_index, i + 1);
		tempMPZ = evalutePoly(scheme -> poly, secret_index, group -> q);
		mpz_set(scheme -> shares[i], *tempMPZ);
		free(tempMPZ);
	}

	mpz_init_set(scheme -> secret, secret);

	return scheme;
}


/*
// IMPORTANT NOTE : You need t + 1 many shares to uncover the secret.
struct sharingScheme *VSS_Share_Backwards(int t, int n, gmp_randstate_t state, struct DDH_Group *group)
{
	struct sharingScheme *scheme = init_VSS_SharingScheme(t, n);
	mpz_t *shares, *tempMPZ, secret_index, twoPow256;
	int *indices, i;


	shares = (mpz_t *) calloc(n, sizeof(mpz_t));
	indices = (int *) calloc(n, sizeof(int));

	mpz_init(twoPow256);
	mpz_ui_pow_ui(twoPow256, 2, 120);

	for(i = 0; i < n; i ++)
	{
		mpz_urandomm(scheme -> shares[i], state, twoPow256);
		indices[i] = i + 1;
	}

	scheme -> poly = getPolyFromCodewords(scheme -> shares, indices, t, group -> q);
	scheme -> pub = init_VSS_PublicBoxWithCoeffs(t, n, scheme -> poly, group);

	mpz_init_set_ui(secret_index, 0);
	tempMPZ = evalutePoly(scheme -> poly, secret_index, group -> q);
	mpz_init_set(scheme -> secret, *tempMPZ);

	// printPoly(scheme -> poly);
	// gmp_printf("%Zd\n\n", scheme -> secret);

	for(i = 0 ; i < n; i ++)
	{
		mpz_set_ui(secret_index, i + 1);
		tempMPZ = evalutePoly(scheme -> poly, secret_index, group -> q);
		if(0 != mpz_cmp(scheme -> shares[i], *tempMPZ))
		{
			gmp_printf("%d >>> %d \n", i, scheme -> poly -> degree);
		}
	}


	mpz_clear(twoPow256);
	mpz_clear(*tempMPZ);
	free(tempMPZ);

	return scheme;
}
*/


int VSS_Verify(struct pubVSS_Box *pub, mpz_t shareToVerify, unsigned int index, struct DDH_Group *group)
{
	mpz_t indexMPZ, indexMPZPowT, gPowShare, c_iProduct, c_iTemp, temp;
	int i, verified = 0;


	mpz_init(temp);
	mpz_init(c_iTemp);
	mpz_init_set(c_iProduct, pub -> commitments[0]);
	mpz_init(gPowShare);
	mpz_init_set_ui(indexMPZ, index);
	mpz_init_set(indexMPZPowT, indexMPZ);


	mpz_mod(temp, shareToVerify, group -> q);
	mpz_powm(gPowShare, group -> g, temp, group -> p);

	for(i = 1; i <= pub -> t; i ++)
	{
		mpz_powm_ui(indexMPZPowT, indexMPZ, i, group -> q);
		mpz_powm(c_iTemp, pub -> commitments[i], indexMPZPowT, group -> p);

		mpz_mul(temp, c_iProduct, c_iTemp);
		mpz_mod(c_iProduct, temp, group -> p);
	}

	verified = mpz_cmp(c_iProduct, gPowShare);


	return verified;
}


mpz_t *VSS_Recover(mpz_t *shares, int *indices, int length, struct DDH_Group *group)
{
	struct Fq_poly *secretPoly = getPolyFromCodewords(shares, indices, length, group -> q);
	mpz_t zeroMPZ, *secret;


	mpz_init_set_ui(zeroMPZ, 0);
	secret = evalutePoly(secretPoly, zeroMPZ, group -> q);


	return secret;
}


unsigned char *serialisePubBoxes(struct sharingScheme **schemes, int numSchemes, int *bufferLength)
{
	unsigned char *outputBuffer;
	int i, j, tempOffset = 0;//2 * sizeof(int);


	*bufferLength = 0;//2 * sizeof(int);

	for(i = 0; i < numSchemes; i ++)
	{
		for(j = 0; j < (schemes[i] -> pub -> t + 1); j ++)
		{
			*bufferLength += sizeOfSerialMPZ(schemes[i] -> pub -> commitments[j]);
		}
	}

	outputBuffer = (unsigned char *) calloc(*bufferLength, sizeof(unsigned char));
	// memcpy(outputBuffer, &(schemes[i] -> pub -> t), sizeof(int));
	// memcpy(outputBuffer + sizeof(int), &(schemes[i] -> pub -> n), sizeof(int));

	for(i = 0; i < numSchemes; i ++)
	{
		for(j = 0; j < (schemes[i] -> pub -> t + 1); j ++)
		{
			serialiseMPZ(schemes[i] -> pub -> commitments[j], outputBuffer, &tempOffset);
		}
	}


	return outputBuffer;
}


struct sharingScheme **deserialisePubBoxes(unsigned char *inputBuffer, int t, int n, int numSchemes, int *inputOffset)
{
	struct sharingScheme **schemes = (struct sharingScheme **) calloc(numSchemes, sizeof(struct sharingScheme *));
	int i, j, tempOffset = *inputOffset;
	mpz_t *tempMPZ;



	for(i = 0; i < numSchemes; i ++)
	{
		schemes[i] = (struct sharingScheme *) calloc(1, sizeof(struct sharingScheme));
		schemes[i] -> pub = init_VSS_PublicBox(t, n);

		for(j = 0; j < t + 1; j ++)
		{
			tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
			mpz_set(schemes[i] -> pub -> commitments[j], *tempMPZ);

			mpz_clear(*tempMPZ);
			free(tempMPZ);
		}
	}


	return schemes;
}



void testVSS()
{
	struct DDH_Group *group;
	struct sharingScheme *scheme;
	mpz_t secret, *tempMPZ;
	gmp_randstate_t *state;
	unsigned int i;
	int verified = 0, *tempIndices = (int *) calloc(10, sizeof(int));

	state = seedRandGen();
	// group = getSchnorrGroup(1024, *state);
	group = generateGroup(128, *state);

	mpz_init(secret);
	mpz_urandomm(secret, *state, group -> q);

	scheme = VSS_Share(secret, 9, 10, *state, group);


	for(i = 0; i < 10; i ++)
	{
		tempIndices[i] = i + 1;
	}



	for(i = 2; i <= 10; i ++)
	{
		tempMPZ = VSS_Recover(scheme -> shares, tempIndices, i, group);
		printf("%d  =>  %d\n", i, mpz_cmp(*tempMPZ, secret));
	}

	printf("\n\n");


	for(i = 0; i < 10; i ++)
	{
		verified = VSS_Verify( scheme -> pub, scheme -> shares[i], i + 1, group);
		printf("%d  =>  %d\n", i + 1, verified);
	}


	mpz_t *sharesTemp = (mpz_t *) calloc(6, sizeof(mpz_t));
	for(i = 0; i < 5; i ++)
	{
		mpz_init_set(sharesTemp[i], scheme -> shares[i + 2]);
	}
	mpz_init_set(sharesTemp[i], scheme ->shares[0]);

	for(i = 0; i < 5; i ++)
	{
		tempIndices[i] = i + 3;
	}
	tempIndices[5] = 1;

	tempMPZ = VSS_Recover(sharesTemp, tempIndices, 6, group);
	printf("\n%d  =>  %d\n", i, mpz_cmp(*tempMPZ, secret));
}







