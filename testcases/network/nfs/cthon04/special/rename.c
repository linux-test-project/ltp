/*
 *	@(#)rename.c	1.2 98/10/26 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * rename a file n times
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#include ../"tests.h"
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if defined(SVR3) || defined(SVR4)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#endif /* DOSorWIN32 */

main(argc, argv)
	int argc;
	char *argv[];
{
	int count;
	int i;
	int fd;

	if (argc != 2) {
		fprintf(stderr, "usage: %s <count>\n", argv[0]);
		exit(1);
	}
	if ((fd = open("rename1", O_CREAT, 0666)) < 0) {
		perror("rename1");
		exit(1);
	}
	close(fd);

	count = atoi(argv[1]);
	for (i=0; i<count; i++) {
		if (rename("rename1", "rename2") < 0) {
			perror("rename rename1 to rename2");
			fprintf(stderr, "%d of %d\n", i, count);
			exit(1);
		}
		if (rename("rename2", "rename1") < 0) {
			perror("rename rename2 to rename1");
			fprintf(stderr, "%d of %d\n", i, count);
			exit(1);
		}
	}
cleanup:
	unlink("rename1");
	unlink("rename2");
	exit(0);
}
