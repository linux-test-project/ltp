/*	@(#)truncate.c	1.2 98/10/26 Connectathon Testsuite	*/
/*
 * Test to see whether the server can handle extending a file via
 * a setattr request.
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#include "../tests.h"
#endif

#ifndef DOSorWIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#endif /* DOSorWIN32 */

main()
{
	int fd;
	struct stat statb;

#ifdef DOSorWIN32
	fprintf(stderr, "This Test Not Executable on DOS or Windows\n");
	exit(1);
#else
	if ((fd = creat("testfile", 0644)) < 0) {
		perror("creat");
		exit(1);
	}

	if (ftruncate(fd, 0) < 0) {
		perror("ftruncate1");
		exit(1);
	}
	if (stat("testfile", &statb) < 0) {
		perror("stat1");
		exit(1);
	}
	if (statb.st_size != 0L) {
		fprintf(stderr,
	"truncate: testfile not zero length, but no error from ftruncate\n");
		exit(1);
	}

	if (ftruncate(fd, 10) < 0) {
		perror("ftruncate2");
		exit(1);
	}
	if (stat("testfile", &statb) < 0) {
		perror("stat1");
		exit(1);
	}
	if (statb.st_size != 10L) {
		fprintf(stderr,
"truncate: testfile length not set correctly, but no error from ftruncate\n");
		exit(1);
	}

	close(fd);
	(void) unlink("testfile");

	printf("truncate succeeded\n");

	exit(0);
#endif /* DOSorWIN32 */
}
