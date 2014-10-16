// Send
// Receive
// Pre-Compute
// Constructor to Init


class otBaseSender
{
	public:
		virtual void 	preCompute() = 0;
		virtual int  	send(unsigned char *buffer, int bufferSize) = 0;	// Will need to also pass a socket in future
		virtual int 	listen(unsigned char *buffer, int bufferSize) = 0;	// Ditto
		virtual int 	transfer() = 0;
};



class otBaseReceiver
{
	public:
		virtual void 	preCompute() = 0;
		virtual int  	send(unsigned char *buffer, int bufferSize) = 0;	// Will need to also pass a socket in future
		virtual int 	listen(unsigned char *buffer, int bufferSize) = 0;	// Ditto
		virtual int 	transfer() = 0;
};

