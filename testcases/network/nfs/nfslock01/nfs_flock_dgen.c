// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2023
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * [Description]
 *
 * Tool to generate data for testing file locking.
 * Used in nfslock01.sh.
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
		fprintf(stderr, "usage: <nfs_flock_dgen> <file> <char/line> <lines> <ctype>\n");
		exit(2);
	}

	fp = fopen(argv[1], "w");

	nchars = atoi(argv[2]);
	nlines = atoi(argv[3]);
	ctype = atoi(argv[4]);

	if (nchars > BUFSIZ) {
		fprintf(stderr, "Exceeded the maximum limit of the buffer (%d)\n", BUFSIZ);
		exit(3);
	}

	if (nchars < 1) {
		fprintf(stderr, "<char/line> must be > 0\n");
		exit(3);
	}

	if (nlines < 1) {
		fprintf(stderr, "<lines> must be > 0\n");
		exit(3);
	}

	k = 0;
	for (i = 1; i <= nlines; i++) {
		if (ctype)
			c = ((i % 2) ? '1' : '0');
		else
			c = 'A' + k;

		for (j = 0; j < nchars - 1; j++)
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
