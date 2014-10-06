#ifndef FORMAT_UTILS
#define	FORMAT_UTILS


typedef struct formatItem
{
	char owner;
	char inputOrOutput;

	int bitLength;
	int *idList;
} formatItem;


void printFormatItem(struct formatItem *input);
int countWires(char* line, int strIndex);

struct formatItem *readFormatLine(char *line);
struct formatItem **readFormatFile( char *filepath, int numInputOutputs);

#endif
