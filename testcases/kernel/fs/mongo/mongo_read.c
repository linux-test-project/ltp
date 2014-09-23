/*
 * Copyright 2000 by Hans Reiser, licensing governed by reiserfs/README
 */

/*
 * MONGO READ  - simple possible program to read a number of given files
 *               suitable for benchmarking FS read performance
 */

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd, rd, i;
	char *buf;
	int bufsize = 4096;

	if (argc < 2) {
		printf("\nUsage: %s filename [,filename2 [,...] ] ]\n\n",
		       argv[0]);
		return 0;
	}

	buf = malloc(bufsize);
	if (buf == 0) {
		printf("Malloc failed on %d\n", bufsize);
		return 0;
	}

	/* Read all given files */
	for (i = 1; i < argc; i++) {

		/* open the file */
		fd = open(argv[i], O_RDONLY);
		if (fd == -1) {
			printf("Open failed (%s)\n", strerror(errno));
			return 0;
		}

		/* read the file */
		while ((rd = read(fd, buf, bufsize)) == bufsize) ;
		if (rd == -1) {
			printf("Read failed (%s)\n", strerror(errno));
			return 0;
		}
		close(fd);
	}

	free(buf);
	return 0;
}
