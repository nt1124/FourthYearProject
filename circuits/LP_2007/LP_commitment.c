

/*	+++------------(-----------------------)------------+++
	+++------------(-----------------------)------------+++
	+++------------( W_Boxes house keeping )------------+++
	+++-------------------------------------------------+++
	+++-------------------------------------------------+++	*/

struct commit_W_Boxes *init_W_Boxes(int length_W)
{
	int i;
	struct commit_W_Boxes *toReturn = (struct commit_W_Boxes*) calloc(1, sizeof(struct commit_W_Boxes));


	toReturn -> length = length_W;

	toReturn -> b_Box = init_commit_box();
	toReturn -> k_Boxes = (struct elgamal_commit_box**) calloc(length_W, sizeof(struct elgamal_commit_box*));

	toReturn -> b_dot_Box = init_commit_box();
	toReturn -> k_dot_Boxes = (struct elgamal_commit_box**) calloc(length_W, sizeof(struct elgamal_commit_box*));

	for(i = 0; i < length_W; i ++)
	{
		toReturn -> k_Boxes[i] = init_commit_box();
		toReturn -> k_dot_Boxes[i] = init_commit_box();
	}

	return toReturn;
}

void free_W_Boxes(struct commit_W_Boxes *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free(toFree -> k_Boxes[i]);
		free(toFree -> k_dot_Boxes[i] );
	}

	free(toFree -> b_Box);
	free(toFree -> b_dot_Box);
	free(toFree -> k_Boxes);
	free(toFree -> k_dot_Boxes);

	free(toFree);
}

struct commit_Circuit_Boxes *init_Circuit_Boxes(int length_Circuit, int length_W)
{
	int i;
	struct commit_Circuit_Boxes *toReturn = (struct commit_Circuit_Boxes*) calloc(1, sizeof(struct commit_Circuit_Boxes));


	toReturn -> length = length_Circuit;
	toReturn -> rows = (struct commit_W_Boxes**) calloc(length_Circuit, sizeof(struct commit_W_Boxes*));

	for(i = 0; i < length_Circuit; i ++)
	{
		toReturn -> rows[i] = init_W_Boxes(length_W);
	}

	return toReturn;
}

void free_Circuit_Boxes(struct commit_Circuit_Boxes *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free_W_Boxes(toFree -> rows[i]);
	}

	free(toFree -> rows);

	free(toFree);
}

struct commit_Wire_Boxes *init_Wire_Boxes(int length_Wire, int length_Circuit, int length_W)
{
	int i;
	struct commit_Wire_Boxes *toReturn = (struct commit_Wire_Boxes*) calloc(1, sizeof(struct commit_Wire_Boxes));


	toReturn -> length = length_Wire;
	toReturn -> columns = (struct commit_Circuit_Boxes**) calloc(length_Wire, sizeof(struct commit_Circuit_Boxes*));

	for(i = 0; i < length_Wire; i ++)
	{
		toReturn -> columns[i] = init_Circuit_Boxes(length_Circuit, length_W);
	}

	return toReturn;
}

void free_Wire_Box(struct commit_Wire_Boxes *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free_Circuit_Boxes(toFree -> columns[i]);
	}

	free(toFree -> columns);

	free(toFree);
}


/*	+++------------(----------------------)------------+++
	+++------------(----------------------)------------+++
	+++------------( W_Keys house keeping )------------+++
	+++------------------------------------------------+++
	+++------------------------------------------------+++	*/


struct commit_W_Keys *init_W_Keys(int length_W)
{
	int i;
	struct commit_W_Keys *toReturn = (struct commit_W_Keys*) calloc(1, sizeof(struct commit_W_Keys));


	toReturn -> length = length_W;

	toReturn -> b_Key = init_commit_key();
	toReturn -> k_Keys = (struct elgamal_commit_key**) calloc(length_W, sizeof(struct elgamal_commit_key*));

	toReturn -> b_dot_Key = init_commit_key();
	toReturn -> k_dot_Keys = (struct elgamal_commit_key**) calloc(length_W, sizeof(struct elgamal_commit_key*));

	for(i = 0; i < length_W; i ++)
	{
		toReturn -> k_Keys[i] = init_commit_key();
		toReturn -> k_dot_Keys[i] = init_commit_key();
	}

	return toReturn;
}

void free_W_Key(struct commit_W_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free(toFree -> k_Keys[i]);
		free(toFree -> k_dot_Keys[i] );
	}

	free(toFree -> b_Key);
	free(toFree -> b_dot_Key);
	free(toFree -> k_Keys);
	free(toFree -> k_dot_Keys);

	free(toFree);
}

struct commit_Circuit_Keys *init_Circuit_Keys(int length_Circuit, int length_W)
{
	int i;
	struct commit_Circuit_Keys *toReturn = (struct commit_Circuit_Keys*) calloc(1, sizeof(struct commit_Circuit_Keys));


	toReturn -> length = length_Circuit;
	toReturn -> rows = (struct commit_W_Keys**) calloc(length_Circuit, sizeof(struct commit_W_Keys*));

	for(i = 0; i < length_Circuit; i ++)
	{
		toReturn -> rows[i] = init_W_Keys(length_W);
	}

	return toReturn;
}

void free_Circuit_Keys(struct commit_Circuit_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free_W_Key(toFree -> rows[i]);
	}

	free(toFree -> rows);

	free(toFree);
}


struct commit_Wire_Keys *init_Wire_Keys(int length_Wire, int length_Circuit, int length_W)
{
	int i;
	struct commit_Wire_Keys *toReturn = (struct commit_Wire_Keys*) calloc(1, sizeof(struct commit_Wire_Keys));


	toReturn -> length = length_Wire;
	toReturn -> columns = (struct commit_Circuit_Keys**) calloc(length_Wire, sizeof(struct commit_Circuit_Keys*));

	for(i = 0; i < length_Wire; i ++)
	{
		toReturn -> columns[i] = init_Circuit_Keys(length_Circuit, length_W);
	}

	return toReturn;
}

void free_Wire_Box(struct commit_Wire_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> length; i ++)
	{
		free_Circuit_Keys(toFree -> columns[i]);
	}

	free(toFree -> columns);

	free(toFree);
}








struct commit_W_Keys *LP_2007_Circuit_commit_C(struct Circuit **circuitsArray, int securityParam, int wireID)
{
	struct commit_W_Keys *toReturn = (struct commit_W_Keys*) calloc(1, sizeof(struct commit_W_Keys));
	unsigned char b = getPermutation();
	int i, j;

	for(i = 0; i < securityParam; i ++)
	{

	}
}