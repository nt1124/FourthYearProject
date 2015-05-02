#include <iostream>

using namespace std;

void error(const char *str)
{
    char err[1000];
    gethostname(err, 1000);
    strcat(err, " : ");
    strcat(err, str);
    perror(err);

    throw exception();
}


void error(const char *str1,const char *str2)
{
    char err[1000];
    gethostname(err, 1000);
    strcat(err, " : ");
    strcat(err, str1);
    strcat(err, str2);
    perror(err);

    throw exception();
}


// Main socket is the socket we use to set up the server and lsiten for connection attemps.
// consocket is the socket we assign for this conversation?
void set_up_server_socket(sockaddr_in& dest, int& consocket, int& main_socket, int Portnum)
{
    struct sockaddr_in serv; /* socket info about our server */
    int socksize = sizeof(struct sockaddr_in);

    memset(&dest, 0, sizeof(dest));    /* zero the struct before filling the fields */
    memset(&serv, 0, sizeof(serv));    /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;         /* set the type of connection to TCP/IP */
    serv.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
    serv.sin_port = htons(Portnum);    /* set the server port number */

    main_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(main_socket < 0)
    {
        error("set_up_socket:socket");
    }

    int one = 1;
    int fl = setsockopt(main_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&one,sizeof(int));
    if(fl < 0)
    {
        error("set_up_socket:setsockopt");
    }

    /* disable Nagle's algorithm */
    fl = setsockopt(main_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
    if(fl < 0)
    {
        error("set_up_socket:setsockopt");
    }

    /* Bind serv information to mysocket 
    *   - Just assume it will eventually wake up
    */
    fl = 1;
    while (fl != 0)
    {
        fl = bind(main_socket, (struct sockaddr *)&serv, sizeof(struct sockaddr));
        if (fl != 0)
        {
            cout << "Binding to socket failed, trying again in a second ..." << endl;
            sleep(1);
        }
    }

    if(fl < 0)
    {
        error("set_up_socket:bind");
    }

    /* start listening, allowing a queue of up to 1 pending connection */
    fl = listen(main_socket, 1);
    if(fl < 0)
    {
        error("set_up_socket:listen");
    }

    consocket = accept(main_socket, (struct sockaddr *)&dest, (socklen_t*) &socksize);

    if(consocket < 0)
    {
        error("set_up_socket:accept");
    }
}


void close_server_socket(int consocket, int main_socket)
{
    if (close(consocket))
    {
        error("close(socket)");
    }
    if (close(main_socket))
    {
        error("close(main_socket");
    }
}


void set_up_client_socket(int& mysocket, const char* hostname, int Portnum, sockaddr_in &dest)
{
    mysocket = socket(AF_INET, SOCK_STREAM, 0);
    if(mysocket < 0)
    {
        error("set_up_socket:socket");
    }

    /* disable Nagle's algorithm */
    int one = 1;
    int fl = setsockopt(mysocket, IPPROTO_TCP, TCP_NODELAY, (char*)&one,sizeof(int));
    if(fl < 0)
    {
        error("set_up_socket:setsockopt");
    }

    dest.sin_family = AF_INET;
    dest.sin_port = htons(Portnum);      // set destination port number 


    struct hostent *server;
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        error("set_up_socket:gethostbyname");
    }  
    bcopy((char *)server->h_addr, 
          (char *)&dest.sin_addr.s_addr,
           server->h_length);          // set destination IP number 

    struct addrinfo hints, *ai=NULL,*rp;
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_CANONNAME;

    int erp;

    for (int i = 0; i < 60; i++)
    {
        erp = getaddrinfo(hostname, NULL, &hints, &ai);
        if (erp == 0)
        {
            break;
        }
        else
        {
            cout << "getaddrinfo has returned " << gai_strerror(erp) <<
                    ", trying again in a second ..." << endl;
            if (ai)
                freeaddrinfo(ai);
            sleep(1);
        }
    }

    if (erp != 0)
    {
        error("set_up_socket:getaddrinfo"); 
    }

    for(rp = ai; rp != NULL; rp = rp -> ai_next)
    {
        const struct in_addr *addr4 = &((const struct sockaddr_in*)ai->ai_addr)->sin_addr;

        if (ai->ai_family == AF_INET)
        {
            memcpy((char *)&dest.sin_addr.s_addr, addr4, sizeof(in_addr));
            continue;
        }
    }
    freeaddrinfo(ai);


    do
    { 
        fl = 1;
        while (fl == 1 || errno == EINPROGRESS)
        {
            fl = connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr));
        }
    }
    while (fl == -1 && errno == ECONNREFUSED);


    if(fl < 0)
    {
        error("set_up_socket:connect:", hostname); 
    }
}



void close_client_socket(int socket)
{
  if (close(socket)) { error("close(socket)"); }
}



#include <iostream>
using namespace std;

unsigned long long sent_amount = 0, sent_counter = 0;



void sendBoth(int socket, octet *msg, int len)
{
    sendInt(socket, len);
    if (send(socket, msg, len, 0) != len)
    {
        error("Send error - 1 ");
    }

    byteSendCount += (sizeof(int) + len);
    totalSendCount += (sizeof(int) + len);
    subSendCount += (sizeof(int) + len);
    // get_ack(socket);
}


unsigned char *receiveBoth(int socket, int& len)
{
    int i = 0, j;
    len = receiveInt(socket);
    unsigned char *msg = (unsigned char*) calloc(len, sizeof(unsigned char));

    byteReceivedCount += (sizeof(int) + len);
    totalReceivedCount += (sizeof(int) + len);
    subReceivedCount += (sizeof(int) + len);

    while (len - i > 0)
    {
        j = recv(socket, msg + i, len - i, 0);
        if(j < 0)
        {
            error("Receiving error - 1");
        }
        i = i + j;
    }



    // send_ack(socket);
    return msg;
}


// Send an octet message given the length.
void send(int socket, octet *msg, int len)
{
    if (send(socket, msg, len, 0) != len)
    {
        error("Send error - 1 ");
    }

    sent_amount += len;
    sent_counter ++;
}


// Receives an octet message given it already knows the length.
void receive(int socket, octet *msg, int len)
{
    int i = 0, j;

    while (len - i > 0)
    {
        j = recv(socket, msg + i, len - i, 0);
        if(j < 0)
        {
            error("Receiving error - 1");
        }
        i = i + j;
    }
}


// Send a single byte.
void send(int socket, int a)
{
    unsigned char msg[1];
    msg[0] = a & 255;

    if(send(socket, msg, 1, 0) != 1)
    {
        error("Send error - 2 ");
    }
}


// Receive a single byte
void receive(int socket, int& a)
{
    unsigned char msg[1];
    int i = 0;

    while(i == 0)
    {
        i = recv(socket, msg, 1, 0);
        if(i < 0)
        {
            error("Receiving error - 2");
        }
    }

    a = msg[0];
}


void sendInt(int socket, int input)
{
    unsigned char *intBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));

    encode_length(intBuffer, input);

    if (send(socket, intBuffer, sizeof(int), 0) != sizeof(int))
    {
        error("Send error - 1 ");
    }

    sent_amount += 4;
    sent_counter ++;
    free(intBuffer);
}

// Receives an octet message given it already knows the length.
int receiveInt(int socket)
{
    unsigned char *intBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));
    int i = 0, j, len = 4, outputInt;

    while (len - i > 0)
    {
        j = recv(socket, intBuffer + i, len - i, 0);
        if(j < 0)
        {
            error("Receiving error - 1");
        }
        i = i + j;
    }

    outputInt = decode_length(intBuffer);
    // memcpy(&outputInt, intBuffer, 4);
    free(intBuffer);

    return outputInt;
}

void send_ack(int socket)
{
    char msg[]="OK";

    if(send(socket, msg, 2, 0) != 2)
    {
        error("Send Ack");
    }
}


int get_ack(int socket)
{
    char msg[] = "OK";
    char msg_r[2];
    int i = 0, j;

    while(2 - i > 0)
    {
        j = recv(socket, msg_r + i, 2 - i, 0);
        i = i + j;
    }

    if(msg_r[0] != msg[0] || msg_r[1] != msg[1])
    {
        return 1;
    }

    return 0;
}


// This function is an addition of mine.
unsigned char *sendReceiveExchange(int writeSocket, int readSocket, unsigned char *commBufferWrite,
                                int commBufferWriteLen, int *commBufferReadLen, int partyID)
{
    unsigned char *commBufferRead;
    int localBufferLen = 0;

    if(0 == partyID)
    {
        sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
        commBufferRead = receiveBoth(readSocket, localBufferLen);
    }
    else if(1 == partyID)
    {
        commBufferRead = receiveBoth(readSocket, localBufferLen);
        sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
    }

    *commBufferReadLen = localBufferLen;

    return commBufferRead;
}


//  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ++++----FUNCTIONS BELOW HERE ARE FOR MEASURING THE BYTES RECEIVED AND SENT----++++
//  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



void printBytesSent()
{
    printf("Bytes Sent = %llu\n", byteSendCount);
    fflush(stdout);
}


void printBytesReceived()
{
    printf("Bytes Received = %llu\n", byteReceivedCount);
    fflush(stdout);
}


void zeroSentCounter()
{
    byteSendCount = 0;
}

void zeroReceivedCounter()
{
    byteReceivedCount = 0;
}

void setSentCounter(long long unsigned int newCount)
{
    byteSendCount = newCount;
}

void setReceivedCounter(long long unsigned int newCount)
{
    byteReceivedCount = newCount;
}


long long unsigned int getSentCounter()
{
    return byteSendCount;
}

long long unsigned int getReceivedCounter()
{
    return byteReceivedCount;
}


void printAndZeroSentCounter()
{
    printf("::: Bytes Sent = %llu\n", byteSendCount);
    fflush(stdout);
    byteSendCount = 0;
}

void printAndZeroReceivedCounter()
{
    printf("::: Bytes Received = %llu\n", byteReceivedCount);
    fflush(stdout);

    byteReceivedCount = 0;
}


void printAndZeroBothCounters()
{
    printf("::: Bytes Sent = %llu\n", byteSendCount);
    fflush(stdout);
    byteSendCount = 0;

    printf("::: Bytes Received = %llu\n\n", byteReceivedCount);
    fflush(stdout);

    byteReceivedCount = 0;
}


void printBothTotalCounters()
{
    printf("### Total Bytes Sent = %llu\n", totalSendCount);
    fflush(stdout);

    printf("### Total Bytes Received = %llu\n\n", totalReceivedCount);
    fflush(stdout);
}


void zeroBothSubCounters()
{
    subSendCount = 0;
    subReceivedCount = 0;
}


void printZeroBothSubCounters()
{
    printf("£££ Bytes Sent = %llu\n", subSendCount);
    fflush(stdout);
    printf("£££ Bytes Received = %llu\n\n", subReceivedCount);
    fflush(stdout);

    subSendCount = 0;
    subReceivedCount = 0;
}