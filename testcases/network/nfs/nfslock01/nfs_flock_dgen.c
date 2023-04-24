/*
 * This program generates data for testing file locking
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i, j, k, nlines, nchars, ctype;
	char c, buf[BUFSIZ];
	FILE *fp;

	if (argc != 5) {
		printf
		    ("usage: <nfs_flock_dgen> <file> <char/line> <lines> <ctype>\n");
		exit(2);
	}

	fp = fopen(argv[1], "w");

	nchars = atoi(argv[2]);
	if (nchars > BUFSIZ) {
		printf("Exceeded the maximum limit of the buffer (%d)\n",
		       BUFSIZ);
		exit(3);
	}
	nlines = atoi(argv[3]);
	ctype = atoi(argv[4]);

	k = 0;
	for (i = 1; i <= nlines; i++) {

		if (ctype)
			c = ((i % 2) ? '1' : '0');
		else
			c = 'A' + k;

		for (j = 0; j < nchars; j++)

			buf[j] = c;

		fprintf(fp, "%s\n", buf);

		if (!ctype) {
			if (i != 1 && i % 26 == 0)
				k = 0;
			else
				k++;
		}

	}

	fclose(fp);

	return 0;
}
