#include <stdio.h>

/*
 * A function to check the output of unix2dos.
 */

int main (int argc, char *argv[])
{
	FILE * input = NULL;
	int cr = 0, nl = 0, TempChar;

	if (argc < 2)
	{
		fprintf(stderr, "\nUsage: %s mode inputfile\n\n", argv[0]);
		return -1;
	}

	if ((input=fopen(argv[1], "r")) == NULL)
	{
		 fprintf(stderr, "Unable to open file %s.\n", argv[1]);
                 return -1;
	}

	while ((TempChar = getc(input)) != EOF)
	{
		if ( TempChar == '\r' )
			cr++;

		if ( TempChar == '\n' )
                        nl++;
	}

	if ( cr == nl)
		return 0;
	else
		return 1;
}
