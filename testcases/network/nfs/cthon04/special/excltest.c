/*
 *	@(#)excltest.c	1.2 98/10/26 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * test exclusive create
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#include "../tests.h"
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#if defined(SVR3) || defined(SVR4)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#endif /* DOSorWIN32 */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	char *testfile = "exctest.file";
	int count;
	int res, i;

	if (argc > 2) {
		fprintf(stderr, "usage: %s [count]\n", argv[0]);
		exit(1);
	}
	if (argc == 2) 
		count = atoi(argv[1]);
	else
		count = 2;

	unlink(testfile);
	for (i = 0; i < count; i++) {
		res = open(testfile, O_CREAT | O_EXCL, 0777);
		if (i == 0) {
			if (res < 0) {
				perror(testfile);
				exit(1);
			}
		} else {
			if (res >= 0) {
				fprintf(stderr, "exclusive create succeeded\n");
				exit(1);
			} else if (errno != EEXIST) {
				perror(testfile);
				exit(1);
			}
		}
	}

	exit(0);
}
