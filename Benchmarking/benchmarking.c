void benchmark(struct RawCircuit *rawInputCircuit, char *ipAddress, char *portNumStr,
			int testID, int partyID)
{
	if(0 == partyID)
	{
		if(4 == testID)
			benchmarkRawCommReceiver(ipAddress, portNumStr);
		else if(5 == testID)
			 benchmark_ECC_PointReceiver(ipAddress, portNumStr);
		else if(6 == testID)
			benchmark_OT_LP_CnC_Receiver(ipAddress, portNumStr);
		else if(7 == testID)
			benchmark_OT_L_CnC_Mod_Receiver(ipAddress, portNumStr);
		else if(8 == testID)
			benchmark_Symm_OT_NP_Receiver(ipAddress, portNumStr);

		printBytesSent();
		printBytesReceived();
	}
	else if(1 == partyID)
	{
		if(4 == testID)
			benchmarkRawCommSender(portNumStr);
		else if(5 == testID)
			benchmark_ECC_PointSender(portNumStr);
		else if(6 == testID)
			benchmark_OT_LP_CnC_Sender(portNumStr);
		else if(7 == testID)
			benchmark_OT_L_CnC_Mod_Sender(portNumStr);
		else if(8 == testID)
			benchmark_Symm_OT_NP_Sender(portNumStr);

		printBytesSent();
		printBytesReceived();
	}
	else if(2 == partyID)
	{
		printf("Running Exponentiation Tests.\n");
		
		benchmarkECC_Exponentiation();
		benchmarkECC_Doubling();
		benchmarkECC_GroupOp();
	}
}