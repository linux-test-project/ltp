/*
 *	@(#)op_ren.c	1.4 2003/12/30 Connectathon Testsuite
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
#include "../tests.h"
#endif

#ifndef DOSorWIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(SVR3) || defined(SVR4)
#include <fcntl.h>
#else
#include <sys/file.h>
#endif
#include <errno.h>
#endif /* DOSorWIN32 */

#define TBUFSIZ 100
static char wbuf[TBUFSIZ], rbuf[TBUFSIZ];
#define TMSG "This is a test message written to the target file\n"

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
	char *taname;
	char *tbname;
	int errcount = 0;
	long lret;

#ifdef DOSorWIN32
	/* 
	 * This test is too Unix oriented to be useful on a Dos or 
	 * Windows platform. Thus, we'll skip this test, for now.
	 */
	fprintf(stderr, "This Test Not Executable on DOS or Windows\n");
	exit(0);
#endif

#ifdef SUNOS4X
	/*
	 * SunOS 4.x has the bug for which I assume this is testing,
	 * i.e. it doesn't realize that rename(foo, bar) may remove
	 * "bar" if it exists, and therefore doesn't first rename it
	 * to ".nfsXXXX" if it's currently open.
	 *
	 * Thus, we'll skip this test, for now.
	 */
	printf("Test skipped because we know SunOS 4.x client has the bug\n");
	exit(0);
#endif

	setbuf(stdout, NULL);

#ifdef MACOSX
	/* make sure TMPDIR isn't set */
	unsetenv("TMPDIR");
#endif
	if ((taname = tempnam(".", "nfsa")) == NULL) {
		fprintf(stderr, "can't construct a temporary filename: ");
		xxit("tempnam");
	}

	if ((fd = creat(taname, 0777)) < 0) {
		fprintf(stderr, "can't create %s: ", taname);
		xxit("creat");
	}
	close(fd);

	if ((tbname = tempnam(".", "nfsb")) == NULL) {
		fprintf(stderr, "can't construct a temporary filename: ");
		xxit("tempnam");
	}

#ifdef O_RDWR
	if ((fd = open(tbname, O_CREAT|O_TRUNC|O_RDWR, 0777)) < 0) {
		fprintf(stderr, "can't create %s: ", tbname);
		(void) unlink(taname);
		xxit("open");
	}
#else
	if ((fd = creat(tbname, 0777)) < 0) {
		fprintf(stderr, "can't create %s: ", tbname);
		(void) unlink(taname);
		xxit("creat");
	}
	close(fd);
	if ((fd = open(tbname, 2)) < 0) {
		fprintf(stderr, "can't reopen %s: ", tbname);
		(void) unlink(taname);
		(void) unlink(tbname);
		xxit("open");
	}
#endif /* O_RDWR */

	printf("nfsjunk files before rename:\n  ");
	system("ls -al .nfs*");
	ret = rename(taname, tbname);
	printf("%s open; rename ret = %d\n", tbname, ret);
	if (ret)
		xxit(" unlink");
	printf("nfsjunk files after rename:\n  ");
	system("ls -al .nfs*");
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

	if (unlink(tbname) < 0) {
		errcount++;
		perror("unlink");
	}

	if (ret = close(fd)) {
		errcount++;
		perror("close");
	}

	printf("nfsjunk files after close:\n  ");
	system("ls -al .nfs*");

	if ((ret = close(fd)) == 0) {
		errcount++;
		fprintf(stderr, "second close didn't return error!??\n");
	}

	if (errcount == 0)
		printf("test completed successfully.\n");
	exit(errcount);
}

