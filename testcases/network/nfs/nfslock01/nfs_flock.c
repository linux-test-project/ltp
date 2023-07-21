// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2001-2023
 * Copyright (c) International Business Machines Corp., 2001
 */

/*\
 * [Description]
 *
 * Program for testing file locking. The original data file consists of
 * characters from 'A' to 'Z'. The data file after running this program
 * would consist of lines with 1's in even lines and 0's in odd lines.
 *
 * Used in nfslock01.sh.
 */

#include "nfs_flock.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i, fd, mac, nchars, nlines;
	int offset = 0;
	char buf[BUFSIZ];

	if (argc != 5) {
		fprintf(stderr, "Usage: %s <mac num> <file name> <nchars> <nlines>\n",
				argv[0]);
		exit(2);
	}

	fd = open(argv[2], O_RDWR);
	if (fd < 0) {
		perror("opening a file");
		exit(1);
	}

	mac = atoi(argv[1]);
	nchars = atoi(argv[3]);
	nlines = atoi(argv[4]);

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

	/*
	 * Replace a line of characters by 1's if it is process one
	 * else with 0's. Number of charcters in any line are nchars-1,
	 * the last character being a newline character.
	 */
	for (i = 0; i < nchars - 1; i++) {
		if (mac == 1)
			buf[i] = '1';
		else
			buf[i] = '0';
	}
	buf[nchars - 1] = '\n';

	for (i = 0; i < nlines; i++) {
		if (mac == 1) {	/* Set the offset to even lines */
			if ((i % 2) == 0) {
				if (i == 0)
					offset = 0;
				else
					offset += 2 * nchars;
			} else
				continue;
		} else {	/* Set the offset to odd lines */
			if ((i % 2) == 1) {
				if (i == 1)
					offset = nchars;
				else
					offset += 2 * nchars;
			} else
				continue;
		}

		if (writeb_lock(fd, offset, SEEK_SET, nchars) < 0) {
			fprintf(stderr, "failed in writeb_lock, errno = %d\n", errno);
			exit(1);
		}

		lseek(fd, offset, SEEK_SET);

		if (write(fd, buf, nchars) < 0) {
			fprintf(stderr, "failed to write buffer to test file, errno = %d\n", errno);
			exit(1);
		}

		if (unb_lock(fd, offset, SEEK_SET, nchars) < 0) {
			fprintf(stderr, "failed in unb_lock, errno = %d\n", errno);
			exit(1);
		}
	}
	exit(0);
}
