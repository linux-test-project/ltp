/*	@(#)touchn.c	1.2 98/10/26 Connectathon Testsuite	*/
/*
 *  touch n files
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif /* DOSorWIN32 */

#include "../tests.h"

main(argc,argv)
char **argv;
{
	int n;
	char buf[1024];

	if (argc != 2) {
		printf("usage: %s count\n", argv[0]);
		exit(1);
	}
	n = atoi(argv[1]);
	for (; n; n--) {
		sprintf(buf, "name%d", n);
		close(creat(buf, 0666));
	}
	exit(0);
}
