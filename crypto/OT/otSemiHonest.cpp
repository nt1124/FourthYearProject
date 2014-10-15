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