#include "data.h"
// #include "Exceptions/Exceptions.h"


// Don't be fooled by the name, this converts ints to octets.
void encode_length(octet *buff, int len)
{
    /*
    if(len < 0)
    {
        throw invalid_length();
    }
    */
    buff[0] = len & 255;
    buff[1] = (len >>  8) & 255;
    buff[2] = (len >> 16) & 255;
    buff[3] = (len >> 24) & 255;
}

int  decode_length(octet *buff)
{
    int len = buff[0] + 256 * buff[1] + 65536 * buff[2] + 16777216 * buff[3];

    /*
    if(len < 0)
    {
        throw invalid_length();
    }
    */
    return len;
}
