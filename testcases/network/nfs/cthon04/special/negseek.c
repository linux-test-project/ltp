/*
 *	@(#)negseek.c	1.2 98/10/26 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 * test seek to negative offset
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#include "../tests.h"
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
	int fd;
	long i;
	char buf[8192];

#ifdef WIN32
	/* 
	 * The Windows _llseek command returns invalid parameter when this
	 * test is run. However, it appears to work correctly under DOS.
	 */
	fprintf(stderr, "This Test Not Executable on Windows\n");
	exit(1); 
#endif

	if (argc != 2) {
		fprintf(stderr, "usage: negseek filename\n");
		exit(1);
	}
	fd = open(argv[1], O_CREAT|O_RDONLY, 0666);
	if (fd == -1) {
		perror(argv[1]);
		exit(1);
	}

	for (i = 0L; i > -10240L; i -= 1024L) {
		if (lseek(fd, i, 0) == -1L) {
			perror("lseek");
			close(fd);
			unlink(argv[1]);
			exit(0);
		}
		if (read(fd, buf, sizeof buf) == -1) {
			perror("read");
			close(fd);
			unlink(argv[1]);
			exit(0);
		}
	}
	close(fd);
	unlink(argv[1]);
	exit(1);
}
