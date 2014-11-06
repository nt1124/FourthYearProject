#ifndef _Data
#define _Data

#include <string.h>


typedef unsigned char octet;

#define BROADCAST 0
#define ROUTE     1
#define TERMINATE 2
#define GO        3

void encode_length(octet *buff,int len);
int  decode_length(octet *buff);

#endif
