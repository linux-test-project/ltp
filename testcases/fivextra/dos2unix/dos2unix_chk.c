#include <stdio.h>
#include <string.h>

/*
 * A function to check the output of dos2unix.
 */

int main (int argc, char *argv[])
{
	FILE * input = NULL;
	int cr = 0, nl = 0, TempChar;

	if (argc < 3)
	{
		fprintf(stderr, "\nUsage: %s mode inputfile\n\n", argv[0]);
		return -1;
	}

	if ((input=fopen(argv[2], "r")) == NULL)
	{
		 fprintf(stderr, "Unable to open file %s.\n", argv[2]);
                 return -1;
	}

	while ((TempChar = getc(input)) != EOF)
	{
		if ( TempChar == '\r' )
		{
			cr++;
			break;
		}

		if ( TempChar == '\n' )
                        nl++;
	}

	if (strcmp (argv[1],"dos") == 0)
		return cr;
	else if (strcmp (argv[1],"mac") == 0)
	{
		if ( nl != 9 )
			return nl;
		
		return cr;
	}
	else
		return -1;
}
