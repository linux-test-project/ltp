/*
 *	@(#)op_unlk.c	1.4 2003/12/30 Connectathon Testsuite
 *	1.3 Lachman ONC Test Suite source
 *
 *  tests operation on open file which has been unlinked.
 *  steps taken:
 *	1.  create file
 *	2.  open for read/write
 *	3.  unlink file
 *	4.  write data
 *	5.  rewind
 *	6.  read data back
 */

#if defined (DOS) || defined (WIN32)
#define DOSorWIN32
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#if defined(SVR3) || defined(SVR4)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#endif /* DOSorWIN32 */

#include "../tests.h"

#define TBUFSIZ 100
static char wbuf[TBUFSIZ], rbuf[TBUFSIZ];
#define TMSG "This is a test message written to the unlinked file\n"

static void
xxit(s)
	char *s;
{
	perror(s);
	exit(1);
}

/*ARGSUSED*/
int
main(argc, argv)
	int argc;
	char *argv[];
{
	int fd, ret;
	char *tname;
	int errcount = 0;
	long lret;
	int oflags;			/* open flags */

	setbuf(stdout, NULL);

#ifdef MACOSX
	/* make sure TMPDIR isn't set */
	unsetenv("TMPDIR");
#endif
	if ((tname = tempnam(".", "nfs")) == NULL) {
		fprintf(stderr, "can't construct a temporary filename: ");
		xxit("tempnam");
	}

	oflags = O_CREAT|O_TRUNC|O_RDWR;
#ifdef DOSorWIN32
	oflags |= O_BINARY;
#endif
	if ((fd = open(tname, oflags, CHMOD_RW)) < 0) {
		fprintf(stderr, "can't create %s: ", tname);
		xxit("open");
	}

#ifndef WIN32
	/* For WIN you can not delete the file if it is open */
	printf("nfsjunk files before unlink:\n  ");
	system("ls -al .nfs*");
	ret = unlink(tname);
	printf("%s open; unlink ret = %d\n", tname, ret);
	if (ret)
		xxit(" unlink");
	printf("nfsjunk files after unlink:\n  ");
	system("ls -al .nfs*");
#endif
	strcpy(wbuf, TMSG);
	if ((ret = write(fd, wbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "write ret %d; expected %d\n", ret, TBUFSIZ);
		if (ret < 0)
			perror(" write");
		exit(1);
	}
	if ((lret = lseek(fd, 0L, 0)) != 0L) {
		fprintf(stderr, "lseek ret %ld; expected 0\n", lret);
		if (lret < 0)
			perror(" lseek");
		exit(1);
	}
	if ((ret = read(fd, rbuf, TBUFSIZ)) != TBUFSIZ) {
		fprintf(stderr, "read ret %d; expected %d\n", ret, TBUFSIZ);
		if (ret < 0)
			perror(" read");
		exit(1);
	}
	if (strcmp(wbuf, rbuf) != 0) {
		errcount++;
		printf("read data not same as written data\n");
		printf(" written: '%s'\n read:    '%s'\n", wbuf, rbuf);
	} else {
		printf("data compare ok\n");
	}

	if (unlink(tname) == 0) {
		errcount++;
		printf("Error: second unlink succeeded!??\n");
	} else {
		int expected;

#ifdef WIN32
		expected = EACCES;
#else
		expected = ENOENT;
#endif
		if (errno != expected) {
			errcount++;
			perror("unexpected error on second unlink");
		}
	}

	if (ret = close(fd)) {
		errcount++;
		perror("error on close");
	}

#ifndef WIN32
	printf("nfsjunk files after close:\n  ");
	system("ls -al .nfs*");
#endif

	if ((ret = close(fd)) == 0) {
		errcount++;
		fprintf(stderr, "second close didn't return error!??\n");
	}

#ifdef DOSorWIN32
	/* 
	 * XXX return of 0 indicates success.  Why does the error msg say
	 * "failed"? 
	 */
	if ((ret = unlink(tname)) == 0) {
		errcount++;
		fprintf(stderr, "second unlink failed!??\n");
	}
#endif

	if (errcount == 0)
		printf("test completed successfully.\n");
	exit(errcount);
}

