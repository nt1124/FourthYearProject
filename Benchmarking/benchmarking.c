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
		if(4 == testID)
			benchmarkECC_Exponentiation();
		else if(5 == testID)
			benchmarkECC_Doubling();
		else if(6 == testID)
			benchmarkECC_GroupOp();
		else if(7 == testID)
			benchmark_LP_2010_CircuitBuilding(rawInputCircuit);
		else if(8 == testID)
			benchmark_L_2013_CircuitBuilding(rawInputCircuit);
		else if(9 == testID)
			benchmark_HKE_2013_CircuitBuilding(rawInputCircuit);
	}
}